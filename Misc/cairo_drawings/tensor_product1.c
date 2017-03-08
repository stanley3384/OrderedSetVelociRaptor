
/*
    Draw a simple tensor-product patch mesh to get an idea how it works.
   
    gcc -Wall tensor_product1.c -o tensor_product1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Tensor Product Patch Mesh");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
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
    g_signal_connect(window, "draw", G_CALLBACK(draw_background), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Transparent background.
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
   cairo_paint(cr);

   //Patch1
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, 1.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 4.0*w1, 2.0*h1, 6.0*w1, 4.0*h1, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 2.0*w1, 4.0*h1, 0.0*w1, 2.0*h1, 1.0*w1, 1.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 0.0, 0.0, 1.0);
   //Pull purple into the center with a control point.
   cairo_mesh_pattern_set_control_point(pattern1, 1, 0.0*w1, 9.0*h1);
   cairo_mesh_pattern_end_patch(pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);
   
   //Patch2
   cairo_pattern_t *pattern2=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern2);
   cairo_mesh_pattern_move_to(pattern2, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 9.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 8.0*w1, 2.0*h1, 10.0*w1, 4.0*h1, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 6.0*w1, 4.0*h1, 4.0*w1, 2.0*h1, 5.0*w1, 1.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 1, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 3, 0.0, 0.0, 1.0);
   //Pull blue up to the right with a control point.
   cairo_mesh_pattern_set_control_point(pattern2, 3, 9.0*w1, -4.0*h1);
   cairo_mesh_pattern_end_patch(pattern2);
   cairo_set_source(cr, pattern2);
   cairo_paint(cr);

   //Patch3
   cairo_pattern_t *pattern3=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern3);
   cairo_mesh_pattern_move_to(pattern3, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 4.0*w1, 6.0*h1, 6.0*w1, 8.0*h1, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 1.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 2.0*w1, 8.0*h1, 0.0*w1, 6.0*h1, 1.0*w1, 5.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch(pattern3);
   cairo_set_source(cr, pattern3);
   cairo_paint(cr);

   //Patch4
   cairo_pattern_t *pattern4=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern4);
   cairo_mesh_pattern_move_to(pattern4, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 8.0*w1, 6.0*h1, 10.0*w1, 8.0*h1, 9.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 6.0*w1, 8.0*h1, 4.0*w1, 6.0*h1, 5.0*w1, 5.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch(pattern4);
   cairo_set_source(cr, pattern4);
   cairo_paint(cr);
   
   cairo_pattern_destroy(pattern1);
   cairo_pattern_destroy(pattern2);
   cairo_pattern_destroy(pattern3);
   cairo_pattern_destroy(pattern4);

   cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
   cairo_set_line_width(cr, 4.0);

   //Yellow curve1.
   cairo_move_to(cr, 1.0*w1, 1.0*h1);
   cairo_curve_to(cr, 0.0*w1, 2.0*h1, 2.0*w1, 4.0*h1, 1.0*w1, 5.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 0.0*w1, 6.0*h1, 2.0*w1, 8.0*h1, 1.0*w1, 9.0*h1);
   cairo_stroke(cr);

   //Yellow curve2.
   cairo_move_to(cr, 5.0*w1, 1.0*h1);
   cairo_curve_to(cr, 4.0*w1, 2.0*h1, 6.0*w1, 4.0*h1, 5.0*w1, 5.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 4.0*w1, 6.0*h1, 6.0*w1, 8.0*h1, 5.0*w1, 9.0*h1);
   cairo_stroke(cr);

   //Yellow curve3.
   cairo_move_to(cr, 9.0*w1, 1.0*h1);
   cairo_curve_to(cr, 8.0*w1, 2.0*h1, 10.0*w1, 4.0*h1, 9.0*w1, 5.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 8.0*w1, 6.0*h1, 10.0*w1, 8.0*h1, 9.0*w1, 9.0*h1);
   cairo_stroke(cr);

   //Layout axis for drawing.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_rectangle(cr, width/10.0, height/10.0, 8.0*width/10.0, 8.0*height/10.0);
   cairo_stroke(cr);
   cairo_move_to(cr, 1.0*width/10.0, 5.0*height/10.0);
   cairo_line_to(cr, 9.0*width/10.0, 5.0*height/10.0);
   cairo_stroke(cr);
   cairo_move_to(cr, 5.0*width/10.0, 1.0*height/10.0);
   cairo_line_to(cr, 5.0*width/10.0, 9.0*height/10.0);
   cairo_stroke(cr);

   return FALSE;
 }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   //Draw background window transparent.
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }








