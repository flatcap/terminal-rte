#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#include "view.h"

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
			buffer[c] = 'A' + ((r + c) % 26);
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
		printf ("LINE: %s\n", line);
		//printf ("bytes processed: %d\n", (int) (ptr - buffer));
		view_add_line (view, line);
		free (line);
		start = end + 1;
		end = strchr (start, '\n');
	} while (*start);

	printf ("%s (%p,\"%s\")\n", __FUNCTION__, view, file);
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
event_frame (GtkWindow *window, GdkEvent *event, VIEW *view)
{
	int x, y, w, h;
	char buf[20];
	x = event->configure.x;
	y = event->configure.y;
	w = event->configure.width;
	h = event->configure.height;
	sprintf (buf, "%d, %d + %d, %d", w, h, x, y);
	gtk_window_set_title (window, buf);

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
event_expose (GtkWidget *widget, GdkEventExpose *event, VIEW *view)
{
	cairo_t *cr;
	PangoFontDescription *desc;
	char buffer[128];
	PangoLayout *layout;
	char *text = NULL;
	int lines = 0;
	int i;
	int offset = 0;

	cr = gdk_cairo_create (widget->window);

	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);

	layout = pango_cairo_create_layout (cr);

	sprintf (buffer, "%s %d", font_face, font_point);
	desc = pango_font_description_from_string (buffer);
	pango_layout_set_font_description (layout, desc);
	pango_font_description_free (desc);

	lines = view_get_length (view);
	if (lines > view->rows) {
		offset = lines - view->rows;
	} else {
		offset = 0;
	}
	printf ("offset = %d\n", offset);

	for (i = 0; i < view->rows; i++) {
		text = view_get_line (view, i + offset);
		printf ("buffer = %s\n", text);
		if (!text)
			break;

		cairo_move_to (cr, 0, i*font_height);
		cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
		pango_layout_set_text (layout, text, -1);
		pango_cairo_update_layout (cr, layout);
		pango_cairo_show_layout (cr, layout);
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
		case GDK_Escape:
		case GDK_q:
			gtk_main_quit();
			break;
		case GDK_n:
			window_create (view->cols, view->rows, -1, -1, view);
			sprintf (buffer, "new window: %dx%d\n", view->cols, view->rows);
			view_add_line (view, buffer);
			g_list_foreach (view->windows, (GFunc) list_invalidate, view);
			break;
		case GDK_f:
		case GDK_o:
			file_choose (GTK_WINDOW (widget), view);
			g_list_foreach (view->windows, (GFunc) list_invalidate, view);
			break;
		case GDK_b:
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
	GdkRectangle rect = { 0, 0, view->cols*font_width, view->rows*font_height };
	GdkRegion *region;

	region = gdk_region_rectangle (&rect);
	gdk_window_invalidate_region (window->window, region, TRUE);
	gdk_region_destroy (region);
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
	GtkObject *adj          = NULL;

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
	g_signal_connect       (drawing_area, "expose-event",       G_CALLBACK (event_expose),       view);
	g_signal_connect       (drawing_area, "configure-event",    G_CALLBACK (event_frame),        view);

	if ((x > 0) && (y > 0))
		gtk_window_move (GTK_WINDOW (window), x, y);

	gtk_window_set_title (GTK_WINDOW (window), "rte");
	gtk_window_resize    (GTK_WINDOW (window), cols*font_width, rows*font_height);
	gtk_widget_show_all  (window);

	return window;
}


/**
 * main
 */
int
main (int argc, char **argv)
{
	gtk_init (&argc, &argv);

	window_create (NUM_COLS, NUM_ROWS, -1, -1, NULL);

	gtk_main();

	//view_free (v);	// XXX move to on_destroy_window
	return 0;
}

