#include <string.h>
#include <stdlib.h>
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
 * on_button_press
 */
gboolean
on_button_press (GtkWidget *widget, GdkEventButton *button, VIEW *v)
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
 * on_key_press
 */
gboolean
on_key_press (GtkWidget *widget, GdkEventKey *key, VIEW *v)
{
	if (key->keyval == GDK_Escape)
		gtk_main_quit();

	//printf ("num = %ld\n", (long int) num);
	return FALSE;
}

/**
 * expose_event
 */
gboolean
expose_event (GtkWidget * widget, GdkEventExpose * event, VIEW *v)
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

	g_signal_connect_after (window,       "destroy",            G_CALLBACK (gtk_main_quit),   NULL);
	g_signal_connect      (drawing_area, "expose-event",       G_CALLBACK (expose_event),    v);
	g_signal_connect      (window,       "button-press-event", G_CALLBACK (on_button_press), v);
	g_signal_connect      (window,       "key-press-event",    G_CALLBACK (on_key_press),    v);

	gtk_window_set_title (GTK_WINDOW (window), "main window");
	gtk_widget_show_all (window);
	gtk_window_resize (GTK_WINDOW (window), NUM_COLS*font_width, NUM_ROWS*font_height);
	gtk_window_move (GTK_WINDOW (window), OFFSET_X- (NUM_COLS*font_width), OFFSET_Y);

	gtk_main();

	view_free (v);
	return 0;
}

