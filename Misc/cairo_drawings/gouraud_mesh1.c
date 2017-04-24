
/*
    Test a pattern with Gouraud shading. Look at some different patterns to maybe put in
the center of the circular_gradient_clock1.c.

    gcc -Wall gouraud_mesh1.c -o gouraud_mesh1 `pkg-config --cflags --libs gtk+-3.0` -lm

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
   gtk_window_set_title(GTK_WINDOW(window), "Gouraud Mesh");
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

   g_timeout_add(100, (GSourceFunc)time_redraw, da);

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
   gint i=0;
   static gint counter=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Set background as transparent.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
   cairo_paint(cr);

   //A circle to clip the mesh in.
   cairo_arc(cr, width/2.0, height/2.0, 3.5*h1, 0.0, 2.0*G_PI);
   cairo_clip(cr);

   //Adjust the color gradient slightly with the timer.
   gdouble advance=counter%24;
   gdouble red=0.0;
   gdouble green=0.0;
   gdouble blue=1.0;
   if(advance<12)
     {
       red=0.0+(advance*0.0833);
       green=0.0+(advance*0.0833);
       blue=1.0-(advance*0.0833);
     }
   else
     {
       red=1.0-((advance-12)*0.0833);
       green=1.0-((advance-12)*0.0833);
       blue=0.0+((advance-12)*0.0833);
     } 
   //g_print("%i %f %f %f\n", counter, advance, red, blue);
   counter++;

   //Draw 12 gouraud shaded triangles in a circle.
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   gdouble rotation=G_PI/24.0*advance;
   gdouble hour_radius=4.0*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   gdouble prev_cos=0;
   gdouble prev_sin=0;
   cairo_move_to(cr, 0.0, 0.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);  
   for(i=0;i<13;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i)+rotation);
       temp_sin=sin(hour_start-(next_hour*i)+rotation);
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin*temp_sin) + ((4.0*h1)*(4.0*h1)*temp_cos*temp_cos));
       temp_cos=temp_cos*hour_radius;
       temp_sin=temp_sin*hour_radius;
       
       if(i>0)
         {
           cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
           cairo_mesh_pattern_begin_patch(pattern1);
           cairo_mesh_pattern_move_to(pattern1, prev_cos, prev_sin);
           cairo_mesh_pattern_line_to(pattern1, temp_cos, temp_sin);
           cairo_mesh_pattern_line_to(pattern1, 0.0, 0.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, 0.0, 1.0, 1.0, 1.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, red, green, blue, 1.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, 0.0, 0.0, 1.0, 1.0);
           cairo_mesh_pattern_end_patch(pattern1);
           cairo_set_source(cr, pattern1);
           cairo_paint(cr);   
           cairo_pattern_destroy(pattern1);
         }
       prev_cos=temp_cos;
       prev_sin=temp_sin;
     }
     
   return FALSE;
 }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }








