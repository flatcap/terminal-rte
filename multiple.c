#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>

#define NUM_WINDOWS	8
#define NUM_ROWS	5
#define NUM_COLS	50

#define OFFSET_X	990
#define OFFSET_Y	0

char *font_face  = "monospace";
int   font_point = 11;

int   font_width  = 9;
int   font_height = 18;

int   ring_size = NUM_ROWS;
int   ring_head[NUM_WINDOWS];
char *ring_buffer[NUM_WINDOWS][NUM_ROWS];

/**
 * ring_init
 */
void
ring_init(int index)
{
	int i;

	for (i = 0; i < ring_size; i++) {
		ring_buffer[index][i] = NULL;
	}

	ring_head[i] = 0;
}

/**
 * ring_dump
 */
void
ring_dump(int index)
{
	int i;
	int r;

	//printf ("ring_head[%d] = %d\n", index, ring_head[index]);
	for (i = 0; i < ring_size; i++) {
		r = (ring_head[i] + i) % ring_size;
		if (!ring_buffer[index][r])
			continue;
		printf ("ring%d: %s\n", i, ring_buffer[index][r]);
	}
}

/**
 * ring_insert
 */
void
ring_insert (int index, char *text)
{
	if (!text)
		return;

	//printf ("insert: %s\n", text);
	free (ring_buffer[index][ring_head[index]]);
	ring_buffer[index][ring_head[index]] = strdup (text);
	ring_head[index] = (ring_head[index]+1) % ring_size;

	//ring_dump();
	//printf ("\n");
}

/**
 * ring_free
 */
void
ring_free(int index)
{
	int i;

	for (i = 0; i < ring_size; i++) {
		free (ring_buffer[index][i]);
	}
}


/**
 * on_button_press
 */
gboolean
on_button_press(GtkWidget *widget, GdkEventButton *button, void *num)
{
	int index = (int)(long long) num;
	char buffer[NUM_COLS+1];
	GdkRectangle rect = { 0, 0, NUM_COLS*font_width, NUM_ROWS*font_height };
	GdkRegion *region;

	sprintf (buffer, "button press %d at (%.0f,%.0f) at %u", button->button, button->x, button->y, button->time);
	ring_insert (index, buffer);

	region = gdk_region_rectangle (&rect);
	gdk_window_invalidate_region (widget->window, region, TRUE);
	gdk_region_destroy (region);

	return FALSE;
}

/**
 * on_key_press
 */
gboolean
on_key_press (GtkWidget *widget, GdkEventKey *key, void *num)
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
expose_event(GtkWidget * widget, GdkEventExpose * event, void *num)
{
	int index = (int)(long long) num;
	cairo_t *cr;
	int i, r;
	PangoFontDescription *desc;
	char buffer[NUM_COLS+1];
	PangoLayout *layout;

	cr = gdk_cairo_create(widget->window);

	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);

	layout = pango_cairo_create_layout(cr);

	sprintf (buffer, "%s %d", font_face, font_point);
	desc = pango_font_description_from_string(buffer);
	pango_layout_set_font_description(layout, desc);
	pango_font_description_free(desc);

	for (i = 0; i < ring_size; i++) {
		r = (ring_head[index] + i) % ring_size;
		if (!ring_buffer[index][r])
			continue;
		cairo_move_to (cr, 0, i * font_height); 
		cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 1.0);
		pango_layout_set_text(layout, ring_buffer[index][r], -1);
		pango_cairo_update_layout(cr, layout);
		pango_cairo_show_layout(cr, layout);
	}

	g_object_unref(layout);
	cairo_stroke (cr);
	cairo_destroy(cr);
	return FALSE;
}


/**
 * main
 */
int
main(int argc, char **argv)
{
	int i;
	GtkWidget *window[NUM_WINDOWS];
	GtkWidget *drawing_area[NUM_WINDOWS];
	char title[64];

	gtk_init(&argc, &argv);

	for (i = 0; i < NUM_WINDOWS; i++) {
		ring_init(i);
		window[i] = gtk_window_new(GTK_WINDOW_TOPLEVEL);

		drawing_area[i] = gtk_drawing_area_new();
		gtk_container_add(GTK_CONTAINER(window[i]), drawing_area[i]);

		gtk_widget_add_events(window[i], GDK_BUTTON_PRESS_MASK);

		g_signal_connect_after(window[i],       "destroy",            G_CALLBACK(gtk_main_quit),   NULL);
		g_signal_connect      (drawing_area[i], "expose-event",       G_CALLBACK(expose_event),    (void*)(long long)i);
		g_signal_connect      (window[i],       "button-press-event", G_CALLBACK(on_button_press), (void*)(long long)i);
		g_signal_connect      (window[i],       "key-press-event",    G_CALLBACK(on_key_press),    (void*)(long long)i);

		sprintf (title, "multiple %d", i);
		gtk_window_set_title (GTK_WINDOW (window[i]), title);
		gtk_widget_show_all(window[i]);
		gtk_window_resize (GTK_WINDOW(window[i]), NUM_COLS*font_width, NUM_ROWS*font_height);
		gtk_window_move (GTK_WINDOW (window[i]), OFFSET_X, i*(NUM_ROWS*font_height+20)+OFFSET_Y);
	}

	gtk_main();

	ring_dump(0);

	for (i = 0; i < NUM_WINDOWS; i++) {
		ring_free(i);
	}

	return 0;
}

