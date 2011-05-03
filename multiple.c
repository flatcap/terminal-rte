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

#define OFFSET_X	1920
#define OFFSET_Y	0

char *font_face  = "monospace";
int   font_point = 11;

int   font_width  = 9;
int   font_height = 18;

/**
 * file_read
 */
void
file_read (VIEW *view, char *file)
{
	int buf_size = 10240;
	char *buffer = NULL;
	char *ptr = NULL;
	int fd = -1;
	if (!view || !file)
		return;

	fd = open (file, O_RDONLY);
	if (fd < 0)
		return;

	buffer = malloc (buf_size);
	if (!buffer) {
		close (fd);
		printf ("%s: failed\n", __FUNCTION__);
		return;
	}

	buf_size = read (fd, buffer, buf_size);
	printf ("read %d bytes\n", buf_size);

	ptr = strtok (buffer, "\n");
	while (ptr) {
		//printf ("LINE: %s\n", ptr);
		view_add_line (view, ptr);
		ptr = strtok (NULL, "\n");
	}

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
 * event_button_press
 */
gboolean
event_button_press (GtkWidget *widget, GdkEventButton *button, VIEW *v)
{
	static int count = 0;
	char buffer[64];
	GdkRectangle rect = { 0, 0, NUM_COLS*font_width, NUM_ROWS*font_height };
	GdkRegion *region;

	sprintf (buffer, "(%d) button press %d at (%.0f,%.0f) at %u", count++, button->button, button->x, button->y, button->time);
	view_add_line (v, buffer);

	region = gdk_region_rectangle (&rect);
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_region_destroy (region);

	return FALSE;
}

/**
 * event_expose
 */
gboolean
event_expose (GtkWidget * widget, GdkEventExpose * event, VIEW *v)
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

	lines = view_get_length (v);
	if (lines > NUM_ROWS) {
		offset = lines - NUM_ROWS;
	} else {
		offset = 0;
	}
	printf ("offset = %d\n", offset);

	for (i = 0; i < NUM_ROWS; i++) {
		text = view_get_line (v, i + offset);
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
event_key_press (GtkWidget *widget, GdkEventKey *key, VIEW *v)
{
	if (key->keyval == GDK_Escape)
		gtk_main_quit();

	if (key->keyval == GDK_n) {
		// Create a new window
		GtkWidget *window = NULL;
		GtkWidget *drawing_area = NULL;
		GtkWidget *hbox = NULL;
		GtkWidget *vscr = NULL;
		GtkObject *adj = NULL;
		VIEW *v = NULL;

		window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
		gtk_widget_add_events (window, GDK_BUTTON_PRESS_MASK);

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

		v = view_new (NUM_COLS, NUM_ROWS);

		g_signal_connect_after (window,       "destroy",            G_CALLBACK (gtk_main_quit),   NULL);
		g_signal_connect       (drawing_area, "expose-event",       G_CALLBACK (event_expose),    v);
		g_signal_connect       (window,       "button-press-event", G_CALLBACK (event_button_press), v);
		g_signal_connect       (window,       "key-press-event",    G_CALLBACK (event_key_press),    v);

		gtk_window_set_title (GTK_WINDOW (window), "main window");
		gtk_widget_show_all (window);
		gtk_window_resize (GTK_WINDOW (window), NUM_COLS*font_width, NUM_ROWS*font_height);
		gtk_window_move (GTK_WINDOW (window), OFFSET_X - (2*NUM_COLS*font_width+15), OFFSET_Y);
	}

	if (key->keyval == GDK_f) {
		file_choose (GTK_WINDOW (widget), v);
	}

	//printf ("num = %ld\n", (long int) num);
	return FALSE;
}


/**
 * main
 */
int
main (int argc, char **argv)
{
	GtkWidget *window;
	GtkWidget *drawing_area;
	VIEW *v = NULL;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	drawing_area = gtk_drawing_area_new();
	gtk_container_add (GTK_CONTAINER (window), drawing_area);

	gtk_widget_add_events (window, GDK_BUTTON_PRESS_MASK);

	v = view_new (NUM_COLS, NUM_ROWS);
	file_read (v, "rte.c");

	g_signal_connect_after (window,       "destroy",            G_CALLBACK (gtk_main_quit),   NULL);
	g_signal_connect       (drawing_area, "expose-event",       G_CALLBACK (event_expose),    v);
	g_signal_connect       (window,       "button-press-event", G_CALLBACK (event_button_press), v);
	g_signal_connect       (window,       "key-press-event",    G_CALLBACK (event_key_press),    v);

	gtk_window_set_title (GTK_WINDOW (window), "main window");
	gtk_widget_show_all (window);
	gtk_window_resize (GTK_WINDOW (window), NUM_COLS*font_width, NUM_ROWS*font_height);
	gtk_window_move (GTK_WINDOW (window), OFFSET_X - (NUM_COLS*font_width), OFFSET_Y);

	gtk_main();

	view_free (v);
	return 0;
}

