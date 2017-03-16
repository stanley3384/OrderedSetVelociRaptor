
/*   
    Compare drawing with a window, layout and a drawing area. Put the layout and drawing area
in a scrolled window to get an idea of the differences.

    gcc -Wall layout_drawing1.c -o layout_drawing1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean window_background(GtkWidget *window, cairo_t *cr, gpointer data);
static gboolean layout_drawing(GtkWidget *layout, cairo_t *cr, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Drawing Comparison");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 40);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
   gtk_widget_set_app_paintable(window, TRUE);
   //Try to set transparency of main window.
   if(gtk_widget_is_composited(window))
     {
       GdkScreen *screen=gtk_widget_get_screen(window);  
       GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
       gtk_widget_set_visual(window, visual);
     }
   else g_print("Can't set window transparency.\n");
   g_signal_connect(window, "draw", G_CALLBACK(window_background), NULL);

   GtkWidget *label1=gtk_label_new("Label Widget and Rectangle");

   GtkWidget *layout=gtk_layout_new(NULL, NULL);
   gtk_widget_set_hexpand(layout, TRUE);
   gtk_widget_set_vexpand(layout, TRUE);
   gtk_layout_set_size(GTK_LAYOUT(layout), 1000, 1000);
   gtk_layout_put(GTK_LAYOUT(layout), label1, 100, 50);
   g_signal_connect(layout, "draw", G_CALLBACK(layout_drawing), NULL);

   GtkWidget *scroll1=gtk_scrolled_window_new(NULL, NULL);
   gtk_widget_set_hexpand(scroll1, TRUE);
   gtk_widget_set_vexpand(scroll1, TRUE);
   gtk_container_add(GTK_CONTAINER(scroll1), layout);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_size_request(da, 1000, 1000);
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *scroll2=gtk_scrolled_window_new(NULL, NULL);
   gtk_widget_set_hexpand(scroll2, TRUE);
   gtk_widget_set_vexpand(scroll2, TRUE);
   gtk_container_add(GTK_CONTAINER(scroll2), da);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), scroll1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), scroll2, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static gboolean window_background(GtkWidget *window, cairo_t *cr, gpointer data)
 {
   //Draw background window transparent blue.
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
   cairo_paint(cr);
   return FALSE;
 }
static gboolean layout_drawing(GtkWidget *layout, cairo_t *cr, gpointer data)
 {
   //Draw on layout background.
   cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
   cairo_paint(cr);
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_arc(cr, 50, 75, 20, 0.0, 2.0*G_PI);
   cairo_stroke(cr);

   //Draw on moving layout window.
   GdkWindow *win=gtk_layout_get_bin_window(GTK_LAYOUT(layout));
   cairo_t *cr2=gdk_cairo_create(win);
   cairo_set_source_rgb(cr2, 0.0, 1.0, 1.0);
   cairo_rectangle(cr2, 175, 50, 60, 30);
   cairo_fill(cr2);
   cairo_destroy(cr2);

   return FALSE;
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   //Draw on a drawing area.
   cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
   cairo_paint(cr);
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_arc(cr, 50, 75, 20, 0.0, 2.0*G_PI);
   cairo_stroke(cr);

   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
   cairo_set_font_size(cr, 16.0);
   cairo_move_to(cr, 100, 75);  
   cairo_show_text(cr, "Drawing Area Text");

   return FALSE;
 }

