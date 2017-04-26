
/*
    Test a pattern with Gouraud shading. Look at some different patterns to maybe put in
the center of the circular_gradient_clock1.c.

    gcc -Wall gouraud_mesh3.c -o gouraud_mesh3 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean time_redraw(gpointer da);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Gouraud Mesh3");
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

   g_timeout_add(1000, (GSourceFunc)time_redraw, da);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static gboolean time_redraw(gpointer da)
 {
   gtk_widget_queue_draw(GTK_WIDGET(da));
   return TRUE;
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   static gint counter=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble advance=counter%60;
   counter++;

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Set background as transparent.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
   cairo_paint(cr);

   //A circle to clip the mesh in.
   cairo_arc(cr, width/2.0, height/2.0, 3.5*h1, 0.0, 2.0*G_PI);
   cairo_clip(cr);

   cairo_rectangle(cr, 1.0*w1, 1.0*h1, 9.0*w1, 9.0*h1);
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_fill(cr);

   //The sweeping line.
   gdouble hour_start=-G_PI/2.0;
   gdouble next_second=-G_PI/30.0;
   gdouble hour_radius=4.0*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   gdouble prev_cos=0;
   gdouble prev_sin=0;
   cairo_move_to(cr, 0.0, 0.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0); 
   prev_cos=cos(hour_start-(next_second*(advance-5.0)));
   prev_sin=sin(hour_start-(next_second*(advance-5.0)));
   temp_cos=cos(hour_start-(next_second*advance));
   temp_sin=sin(hour_start-(next_second*advance));
   //The polar form of the equation for an ellipse to get the radius.
   hour_radius=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin*temp_sin) + ((4.0*h1)*(4.0*h1)*temp_cos*temp_cos));
   prev_cos=prev_cos*hour_radius;
   prev_sin=prev_sin*hour_radius;
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, prev_cos, prev_sin);
   cairo_mesh_pattern_line_to(pattern1, temp_cos, temp_sin);
   cairo_mesh_pattern_line_to(pattern1, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, 0.0, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, 0.0, 1.0, 1.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, 0.0, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch(pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);   
   cairo_pattern_destroy(pattern1);
        
   return FALSE;
 }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }








