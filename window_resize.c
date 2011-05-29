#include <string.h>
#include <gtk/gtk.h>

int   font_width  = 9;
int   font_height = 18;
char *font_name   = "monospace 72";

/**
 * event_expose
 */
gboolean
event_expose (GtkWidget *drawing_area, GdkEventExpose *event, void *data)
{
	cairo_t *cr;
	PangoLayout *layout;
	int cols = 0;
	int rows = 0;
	int i;
	GtkAllocation da_alloc;
	PangoFontDescription *font_desc = NULL;
	char buffer[20];

	cr = gdk_cairo_create (gtk_widget_get_window (drawing_area));

	cairo_set_source_rgba (cr, 0.0, 0.0, 0.0, 1.0);
	cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint (cr);

	layout = pango_cairo_create_layout (cr);

	font_desc = pango_font_description_from_string (font_name);
	pango_layout_set_font_description (layout, font_desc);

	gtk_widget_get_allocation (drawing_area, &da_alloc);
	pango_layout_set_wrap  (layout, PANGO_WRAP_CHAR);
	pango_layout_set_width (layout, pango_units_from_double (da_alloc.width));
	pango_layout_set_spacing (layout, pango_units_from_double (0));

	cols = da_alloc.width / font_width;
	if (cols > sizeof (buffer))
		cols = sizeof (buffer);

	rows = da_alloc.height / font_height;

	memset (buffer, 0, sizeof (buffer));
	for (i = 0; i < cols; i++) {
		buffer[i] = 'A' + (i % 26);
	}

	for (i = 0; i < rows; i++) {
		pango_layout_set_text (layout, buffer, -1);

		cairo_move_to (cr, 0, i*font_height);
		cairo_set_source_rgb (cr, 1.0, 1.0, 1.0);
		pango_cairo_update_layout (cr, layout);
		pango_cairo_show_layout (cr, layout);
	}

	pango_font_description_free (font_desc);
	g_object_unref (layout);
	cairo_stroke (cr);
	cairo_destroy (cr);
	return FALSE;
}

/**
 * get_font_size
 */
void
get_font_size (GtkWidget *drawing_area)
{
	PangoFontDescription *font_desc = NULL;
	cairo_t *cr;
	PangoLayout *layout;
	PangoRectangle logical;

	if (!drawing_area)
		return;

	cr = gdk_cairo_create (gtk_widget_get_window (drawing_area));
	layout = pango_cairo_create_layout (cr);

	font_desc = pango_font_description_from_string (font_name);
	pango_layout_set_font_description (layout, font_desc);

	pango_layout_set_text (layout, "A", -1);
	pango_layout_get_extents (layout, NULL, &logical);

	font_width  = PANGO_PIXELS_CEIL (logical.width);
	font_height = PANGO_PIXELS_CEIL (logical.height);

	pango_font_description_free (font_desc);
	g_object_unref (layout);
	cairo_destroy (cr);
}

/**
 * window_create
 */
GtkWidget *
window_create (int cols, int rows)
{
	GtkWidget *window       = NULL;
	GtkWidget *drawing_area = NULL;
	GtkWidget *hbox         = NULL;
	GtkWidget *vscr         = NULL;
	GtkAdjustment *adj      = NULL;
	GtkAllocation alloc;
	GdkGeometry geom;
	GdkWindowHints hints;
	int width  = 800;
	int height = 500;

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

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

	gtk_widget_show_all  (window);

	get_font_size (drawing_area);

	gtk_widget_get_allocation (vscr, &alloc);

	if ((cols > 0) && (rows > 0)) {
		width  = cols * font_width + alloc.width;	// + scrollbar
		height = rows * font_height;
		gtk_window_resize    (GTK_WINDOW (window), width, height);
	}

	geom.base_width  = alloc.width;
	geom.min_width   = font_width  *  cols;
	geom.max_width   = font_width  * (cols + 5);
	geom.width_inc   = font_width;

	geom.base_height = 0;
	geom.min_height  = font_height *  rows;
	geom.max_height  = font_height * (rows + 3);
	geom.height_inc  = font_height;

	hints = GDK_HINT_MIN_SIZE | GDK_HINT_MAX_SIZE | GDK_HINT_BASE_SIZE | GDK_HINT_RESIZE_INC;

	gtk_window_set_geometry_hints (GTK_WINDOW (window), NULL, &geom, hints);

	g_signal_connect_after (window,       "destroy", G_CALLBACK (gtk_main_quit), NULL);
	g_signal_connect       (drawing_area, "draw",    G_CALLBACK (event_expose),  NULL);

	return window;
}


/**
 * main
 */
int
main (int argc, char **argv)
{
	gtk_init (&argc, &argv);

	window_create (10, 5);

	gtk_main();

	return 0;
}

