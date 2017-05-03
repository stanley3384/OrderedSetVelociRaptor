
/*
   
    Test getting some Bezier points with some gradients. Needs to be kept square.

    gcc -Wall circular_gradient1.c -o circular_gradient1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Bezier Points");
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
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //background.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr);
   
   //gdouble points=(4.0/3.0*tan(G_PI/(2.0*8.0)));
   //g_print("Bezier Point %f\n", points);
   gdouble start=0.0;
   gdouble next_second=-G_PI/4.0;
   gdouble line_radius1=0;
   gdouble line_radius2=0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   gdouble prev_cos1=2.5*w1;
   gdouble prev_sin1=0.0;
   gdouble prev_cos2=4.0*w1;
   gdouble prev_sin2=0.0;
   gdouble control_points[32];
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_line_width(cr, 2.0);
   for(i=0;i<8;i++)
     {
       temp_cos1=cos(start-(next_second*(i+1)));
       temp_sin1=sin(start-(next_second*(i+1)));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       //The polar form of the equation for an ellipse to get the radius.
       line_radius1=((2.5*w1)*(2.5*h1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin1*temp_sin1) + ((2.5*h1)*(2.5*h1)*temp_cos1*temp_cos1));
       line_radius2=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin1*temp_sin1) + ((4.0*h1)*(4.0*h1)*temp_cos1*temp_cos1));
       //g_print("radius1 %f radius2 %f\n", line_radius1, line_radius2);

       temp_cos1=temp_cos1*line_radius1;
       temp_sin1=temp_sin1*line_radius1;
       temp_cos2=temp_cos2*line_radius2;
       temp_sin2=temp_sin2*line_radius2;
       //g_print("Corners %f %f %f %f\n", prev_cos2, prev_sin2, temp_cos2, temp_sin2);

       /*
         Draw Bezier control points. 
         https://en.wikipedia.org/wiki/Composite_B%C3%A9zier_curve#Approximating_circular_arcs 
       */   
       control_points[4*i+2]=(4.0*line_radius2-line_radius2*cos(G_PI/8.0))/3.0;
       control_points[4*i+3]=((line_radius2-line_radius2*cos(G_PI/8.0))*(3.0*line_radius2-line_radius2*cos(G_PI/8.0)))/(3.0*line_radius2*sin(G_PI/8.0));
       control_points[4*i]=control_points[4*i+2];
       control_points[4*i+1]=-control_points[4*i+3];      
       cairo_save(cr);
       cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
       cairo_rotate(cr, -(i+1)*G_PI/4.0+G_PI/8.0);
       cairo_move_to(cr, control_points[4*i], control_points[4*i+1]);           
       cairo_show_text(cr, "X");
       cairo_move_to(cr, control_points[4*i+2], control_points[4*i+3]);
       cairo_show_text(cr, "X");
       //g_print("Controls %f %f %f %f\n", control_points[4*i], control_points[4*i+1], control_points[4*i+2], control_points[4*i+3]);
       cairo_device_to_user_distance(cr, &control_points[4*i], &control_points[4*i+1]);
       cairo_device_to_user_distance(cr, &control_points[4*i+2], &control_points[4*i+3]);
       cairo_restore(cr);               
       //g_print("Controls1 %f %f %f %f\n", control_points[4*i], control_points[4*i+1], control_points[4*i+2], control_points[4*i+3]);
        
       //Draw some gradients.        
       cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
       cairo_mesh_pattern_begin_patch(pattern1);
       cairo_mesh_pattern_move_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_curve_to(pattern1, control_points[4*i], control_points[4*i+1],  control_points[4*i+2], control_points[4*i+3], temp_cos2, temp_sin2);
       cairo_mesh_pattern_line_to(pattern1, temp_cos1, temp_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos1, prev_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 0.0, 1.0, 0.0);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 0.0, 0.0, 1.0);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 0.0, 1.0);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 0.0, 1.0, 0.0);
       cairo_mesh_pattern_end_patch(pattern1);
       cairo_set_source(cr, pattern1);
       cairo_paint(cr);
       cairo_pattern_destroy(pattern1);          

       //Polygon
       cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); 
       cairo_move_to(cr, prev_cos1, prev_sin1);
       cairo_line_to(cr, temp_cos1, temp_sin1);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, temp_cos2, temp_sin2);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, prev_cos2, prev_sin2);
       cairo_close_path(cr);
       cairo_stroke(cr);   
      
       //Save previous values.
       prev_cos1=temp_cos1;
       prev_sin1=temp_sin1;
       prev_cos2=temp_cos2;
       prev_sin2=temp_sin2;
     }
 
   return FALSE;
 }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }








