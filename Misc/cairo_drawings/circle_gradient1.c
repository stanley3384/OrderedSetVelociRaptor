
/*
    Try drawing a circular gradient.
   
    gcc -Wall circle_gradient1.c -o circle_gradient1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Circular Gradient");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

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
   //Use 0.522 to approximate circle with Bezier control points.
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Quadrant 1.
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, 5.0*width/10.0, 1.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern1, (5.0*width/10.0)+(0.522*4.0*width/10), 1.0*height/10.0,  9.0*width/10.0,  (5.0*height/10.0)-(0.522*4.0*height/10), 9.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern1, 8.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern1, 8.0*width/10.0, (5.0*height/10.0)-(0.522*3.0*height/10),  (5.0*width/10.0)+(0.522*3.0*width/10.0), 2.0*height/10.0, 5.0*width/10.0, 2.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern1, 5.0*width/10.0, 1.0*height/10.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);

   //Quadrant 2.
   cairo_pattern_t *pattern2=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern2);
   cairo_mesh_pattern_move_to(pattern2, 1.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern2, 1.0*width/10.0, (5.0*height/10.0)-(0.522*4.0*height/10), (5.0*width/10.0)-(0.522*4.0*width/10), 1.0*height/10.0, 5.0*width/10.0, 1.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern2, 5.0*width/10.0, 2.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern2, (5.0*width/10.0)-(0.522*3.0*width/10), 2.0*height/10.0,  2.0*width/10.0, (5.0*height/10.0)-(0.522*3.0*height/10.0), 2.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern2, 1.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 2, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 3, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern2);
   cairo_set_source(cr, pattern2);
   cairo_paint(cr);

   //Quadrant 3.
   cairo_pattern_t *pattern3=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern3);
   cairo_mesh_pattern_move_to(pattern3, 5.0*width/10.0, 9.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern3, (5.0*width/10.0)-(0.522*4.0*width/10), 9.0*height/10.0, 1.0*width/10.0, (5.0*height/10.0)+(0.522*4.0*height/10), 1.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern3, 2.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern3, 2.0*width/10.0, (5.0*height/10.0)+(0.522*3.0*height/10),  (5.0*width/10.0)-(0.522*3.0*width/10), 8.0*height/10.0, 5.0*width/10.0, 8.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern3, 5.0*width/10.0, 9.0*height/10.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 0, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 1, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 2, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch (pattern3);
   cairo_set_source(cr, pattern3);
   cairo_paint(cr);

   //Quadrant 4.
   cairo_pattern_t *pattern4=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern4);
   cairo_mesh_pattern_move_to(pattern4, 9.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern4, 9.0*width/10.0, (5.0*height/10.0)+(0.522*4.0*height/10), (5.0*width/10.0)+(0.522*4.0*width/10),  9.0*height/10.0, 5.0*width/10.0, 9.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern4, 5.0*width/10.0, 8.0*height/10.0);
   cairo_mesh_pattern_curve_to(pattern4, (5.0*width/10.0)+(0.522*3.0*width/10), 8.0*height/10.0,  8.0*width/10.0, (5.0*height/10.0)+(0.522*3.0*height/10), 8.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_line_to(pattern4, 9.0*width/10.0, 5.0*height/10.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 0, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 1, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 2, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 3, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch (pattern4);
   cairo_set_source(cr, pattern4);
   cairo_paint(cr);
   
   //Layout axis for drawing.
   cairo_set_line_width(cr, 1.0);
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_rectangle(cr, width/10.0, height/10.0, 8.0*width/10.0, 8.0*height/10.0);
   cairo_stroke(cr);
   cairo_move_to(cr, 1.0*width/10.0, 5.0*height/10.0);
   cairo_line_to(cr, 9.0*width/10.0, 5.0*height/10.0);
   cairo_stroke(cr);
   cairo_move_to(cr, 5.0*width/10.0, 1.0*height/10.0);
   cairo_line_to(cr, 5.0*width/10.0, 9.0*height/10.0);
   cairo_stroke(cr);

   cairo_pattern_destroy(pattern1);
   cairo_pattern_destroy(pattern2);
   cairo_pattern_destroy(pattern3);
   cairo_pattern_destroy(pattern4);

   return FALSE;
}








