#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtk.h>

#include "view.h"
#include "debug.h"
#include "options.h"

#define NUM_ROWS	20
#define NUM_COLS	50

char *font_face  = "monospace";
int   font_point = 11;

int   font_width  = 9;
int   font_height = 18;

GtkWidget * window_create     (int cols, int rows, int x, int y, VIEW *view);
void        window_invalidate (GtkWidget *window, VIEW *view);

/**
 * list_invalidate
 */
void
list_invalidate (GtkWidget *window, VIEW *view)
{
	window_invalidate (window, view);
}

/**
 * char_block
 */
void
char_block (VIEW *view)
{
	char *buffer;
	int r;
	int c;

	if (!view)
		return;

	buffer = malloc (view->cols + 1);
	buffer[view->cols] = 0;

	for (r = 0; r < view->rows; r++) {
		for (c = 0; c < view->cols; c++) {
			buffer[c] = 'A' + ((r % 26) + (c / 50)) % 26;
		}
		view_add_line (view, buffer);
	}

	free (buffer);
}


/**
 * file_read
 */
void
file_read (VIEW *view, char *file)
{
	int   buf_size = 10240;
	int   fd       = -1;
	char *buffer   = NULL;
	char *start    = NULL;
	char *end      = NULL;
	char *line     = NULL;

	if (!view || !file)
		return;

	fd = open (file, O_RDONLY);
	if (fd < 0)
		return;

	buffer = calloc (1, buf_size);
	if (!buffer) {
		close (fd);
		printf ("%s: failed\n", __FUNCTION__);
		return;
	}

	buf_size = read (fd, buffer, buf_size);
	printf ("read %d bytes\n", buf_size);

	start = buffer;
	end = strchr (start, '\n');
	do {
		line = strndup (start, (int) (end - start));
		//printf ("LINE: %s\n", line);
		//printf ("bytes processed: %d\n", (int) (ptr - buffer));
		view_add_line (view, line);
		free (line);
		start = end + 1;
		end = strchr (start, '\n');
	} while (*start);

	Tprintf ("%s (%p,\"%s\")\n", __FUNCTION__, view, file);
	close (fd);
	free (buffer);
}

/**
 * file_choose
 */
void
file_choose (GtkWindow *window, VIEW *view)
{
	GtkWidget *dialog;
	char *filename;

	dialog = gtk_file_chooser_dialog_new ("Open File", window, GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));
		file_read (view, filename);
		g_free (filename);
	}
	gtk_widget_destroy (dialog);
}


/**
 * event_frame
 */
gboolean
event_frame (GtkWidget *drawing_area, GdkEvent *event, VIEW *view)
{
	int w;
	int h;

	w = event->configure.width;
	h = event->configure.height;

	if (1) {
		char buf[20];
		GtkWidget *hbox = NULL;
		GtkWidget *window = NULL;

		hbox   = gtk_widget_get_parent (drawing_area);
		window = gtk_widget_get_parent (hbox);

		sprintf (buf, "%d, %d", w, h);
		gtk_window_set_title (GTK_WINDOW (window), buf);
	}

	view_set_size (view, w/font_width, h/font_height);

	return FALSE;
}

/**
 * event_button_press
 */
gboolean
event_button_press (GtkWidget *widget, GdkEventButton *button, VIEW *view)
{
	static int count = 0;
	char buffer[64];

	sprintf (buffer, "(%d) button press %d at (%.0f,%.0f) at %u", count++, button->button, button->x, button->y, button->time);
	view_add_line (view, buffer);

	g_list_foreach (view->windows, (GFunc) list_invalidate, view);
	return FALSE;
}

/**
 * event_expose
 */
gboolean
event_expose (GtkWidget *drawing_area, GdkEventExpose *event, VIEW *view)
{
	cairo_t *cr;
	PangoFontDescription *desc;
	char buffer[128];
	PangoLayout *layout;
	char *text = NULL;
	int lines = 0;
	int cols = 0;
	int rows = 0;
	int start_row = 0;
	int i;
	int offset = 0;
	int len;
	int line_count = 0;
	GtkAllocation da_alloc;
	int wrap_count = 0;

	cr = gdk_cairo_create (gtk_widget_get_window (drawing_area));

	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);

	layout = pango_cairo_create_layout (cr);

	sprintf (buffer, "%s %d", font_face, font_point);
	desc = pango_font_description_from_string (buffer);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	gtk_widget_get_allocation (drawing_area, &da_alloc);
	pango_layout_set_wrap  (layout, PANGO_WRAP_CHAR);
	pango_layout_set_width (layout, pango_units_from_double (da_alloc.width));
	//pango_layout_set_spacing (layout, pango_units_from_double (30));
	//printf ("spacing = %d (%d)\n", pango_layout_get_spacing (layout), PANGO_PIXELS (pango_layout_get_spacing (layout)));

	cols = da_alloc.width / font_width;
	if (cols == 0)
		cols = 1;		// Alway print something

	rows = da_alloc.height / font_height;		// view->rows
	lines = view_get_length (view);
	if (lines > view->rows) {
		offset = lines - rows;
	} else {
		offset = 0;
	}

	/* notes:
	 *   always draw one character (prevent div by zero)
	 *   strlen won't work because we have tab chars
	 */

	/* options:
	 *   wrap on / off
	 *   follow on / off (i.e. is the window following the current output?)
	 *
	 *   if follow is on, start drawing from the bottom
	 *   if follow if off, start drawing from the top
	 *   follow=on if keypress, or scrollbar at bottom
	 */

	/* calculate wrap_count:
	 * how many more lines to skip due to wrapping of text
	 * the screen is ROWS high so look at the last ROWS of the view
	 * wrap_count = number of rows that need wrapping
	 * not quite
	 * number of rows it would take to display - number of rows in the view
	 * if wrap_count = 6, ignore that many lines from the top of the view.
	 */

	/* drawing (follow=on)
	 * start at the bottom
	 * for each row calculate how many lines it requires
	 * draw lines (start may be negative (off the top of the screen)
	 * continue until screen is full (or end of view)
	 */

	/* drawing (follow=off)
	 * start at the top
	 * for each row calculate how many lines it requires
	 * draw lines (last row may be truncated)
	 * continue until screen is full (or end of view)
	 */


	//printf ("view contains %d cols, %d rows\n", cols, lines);
	for (i = 0; i < view->rows; i++) {
		int tmp = 0;
		text = view_get_line (view, i + offset);
		if (!text)
			break;
		len = strlen (text);
		tmp = (len+cols-1) / cols; /* round up */
		wrap_count += tmp;
	}
	wrap_count -= lines;
	//printf ("offset = %d, wrap_count = %d\n", offset, wrap_count);
	start_row = rows - (lines + wrap_count);
	if (start_row > 0)
		start_row = 0;
	printf ("rows = %d, lines = %d, offset = %d, wrap_count = %d, start_row = %d\n", rows, lines, offset, wrap_count, start_row);
#if 0
	printf ("drawing area is %d lines high\n", da_alloc.height / font_height);
#endif

	line_count = start_row;
	for (i = 0; i < view->rows; i++) {
		text = view_get_line (view, i + offset);
		//printf ("buffer = %s\n", text);
		if (!text)
			break;

		len = strlen (text);
		if (len > view->cols) {
			//printf ("wrap: %s\nto:   %.*s\n", text, view->cols, text);
		}

		pango_layout_set_text (layout, text, -1);

		if (1) {
			PangoLayoutLine *pline = NULL;
			PangoRectangle ink;
			PangoRectangle logical;

			pline = pango_layout_get_line_readonly (layout, 0);
			pango_layout_line_get_pixel_extents (pline, &ink, &logical);
			//printf ("rect: ink (%d,%d,%d,%d), logical (%d,%d,%d,%d) - (%d,%d) - %d lines\n", ink.x, ink.y, ink.width, ink.height, logical.x, logical.y, logical.width, logical.height, logical.width / font_width, logical.height / font_height, pango_layout_get_line_count (layout));
			//rect: ink (2,-11,385,14), logical (0,-14,387,18)
			//int   font_width  = 9;
			//int   font_height = 18;
		}

		cairo_move_to (cr, 0, line_count*font_height);
		cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
		pango_cairo_update_layout (cr, layout);
		pango_cairo_show_layout (cr, layout);

		line_count += pango_layout_get_line_count (layout);
	}

	g_object_unref (layout);
	cairo_stroke (cr);
	cairo_destroy (cr);
	return FALSE;
}


/**
 * event_key_press
 */
gboolean
event_key_press (GtkWidget *widget, GdkEventKey *key, VIEW *view)
{
	char buffer[64];

	switch (key->keyval) {
		case 0xFF1B:	// Escape
		case 'q':
			gtk_main_quit();
			break;
		case 'n':
			window_create (view->cols, view->rows, -1, -1, view);
			sprintf (buffer, "new window: %dx%d", view->cols, view->rows);
			view_add_line (view, buffer);
			g_list_foreach (view->windows, (GFunc) list_invalidate, view);
			break;
		case 'f':
		case 'o':
			file_choose (GTK_WINDOW (widget), view);
			g_list_foreach (view->windows, (GFunc) list_invalidate, view);
			break;
		case 'b':
			char_block (view);
			g_list_foreach (view->windows, (GFunc) list_invalidate, view);
			break;
	}

	return FALSE;
}


/**
 * window_invalidate
 */
void
window_invalidate (GtkWidget *window, VIEW *view)
{
	cairo_rectangle_int_t rect = { 0, 0, view->cols*font_width, view->rows*font_height };
	cairo_region_t *region = NULL;

	region = cairo_region_create_rectangle (&rect);
	gdk_window_invalidate_region (gtk_widget_get_window (window), region, TRUE);
	cairo_region_destroy (region);
}

/**
 * window_create
 */
GtkWidget *
window_create (int cols, int rows, int x, int y, VIEW *view)
{
	GtkWidget *window       = NULL;
	GtkWidget *drawing_area = NULL;
	GtkWidget *hbox         = NULL;
	GtkWidget *vscr         = NULL;
	GtkAdjustment *adj      = NULL;
	int        width        = 800;
	int        height       = 500;

	if ((cols < 0) || (rows < 0)) {
		gtk_window_get_size  (GTK_WINDOW (window), &width, &height);
	} else {
		width  = cols * font_width + 18;	// + scrollbar
		height = rows * font_height;
	}

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_widget_add_events (window, GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events (window, GDK_CONFIGURE);

	hbox = gtk_hbox_new (FALSE, 0);
	gtk_container_add (GTK_CONTAINER (window), hbox);

	drawing_area = gtk_drawing_area_new();
	gtk_container_add (GTK_CONTAINER (hbox), drawing_area);

	// (value, lower, upper, step_increment, page_increment, page_size);
	adj = gtk_adjustment_new (0.0, 0.0, 100.0, 1.0, 20, 20);
	vscr = gtk_vscrollbar_new (GTK_ADJUSTMENT (adj));

	gtk_container_add (GTK_CONTAINER (hbox), vscr);

	gtk_box_set_child_packing (GTK_BOX (hbox), drawing_area, TRUE,  TRUE,  0, GTK_PACK_START);
	gtk_box_set_child_packing (GTK_BOX (hbox), vscr,         FALSE, FALSE, 0, GTK_PACK_END);

	if (!view)
		view = view_new (cols, rows);
	view_add_window (view, window);

	g_signal_connect_after (window,       "destroy",            G_CALLBACK (gtk_main_quit),      NULL);
	g_signal_connect       (window,       "button-press-event", G_CALLBACK (event_button_press), view);
	g_signal_connect       (window,       "key-press-event",    G_CALLBACK (event_key_press),    view);
	g_signal_connect       (drawing_area, "draw",               G_CALLBACK (event_expose),       view);
	g_signal_connect       (drawing_area, "configure-event",    G_CALLBACK (event_frame),        view);

	if ((x > 0) && (y > 0))
		gtk_window_move (GTK_WINDOW (window), x, y);

	gtk_window_set_title (GTK_WINDOW (window), "rte");
	gtk_window_resize    (GTK_WINDOW (window), width, height);
	gtk_widget_show_all  (window);

	return window;
}


/**
 * main
 */
int
main (int argc, char **argv)
{
	if (!parse_options (argc, argv))
		return 1;

	dump_options (&opts);

	gtk_init (&argc, &argv);

	window_create (NUM_COLS, NUM_ROWS, -1, -1, NULL);

	gtk_main();

	//view_free (v);	// XXX move to on_destroy_window
	return 0;
}

