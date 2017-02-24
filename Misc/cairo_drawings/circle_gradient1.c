
/*
    Try drawing a circular gradient with cairo and a couple of tensor-product patch meshes.
Draw four arc patches of a circle. Then add the rest to make a clock.
   
    gcc -Wall circle_gradient1.c -o circle_gradient1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean time_redraw(gpointer da);
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

   g_timeout_add_seconds(5, (GSourceFunc)time_redraw, da);

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
   /*
     From http://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves
     So for 4 points it is (4/3)*tan(pi/8) = 4*(sqrt(2)-1)/3 = 0.552284749831
   */
   gdouble points=0.552284749831;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Quadrant 1 arc.
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, (5.0*w1)+(points*4.0*width/10), 1.0*h1,  9.0*w1,  (5.0*h1)-(points*4.0*height/10), 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 8.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 8.0*w1, (5.0*h1)-(points*3.0*height/10),  (5.0*w1)+(points*3.0*w1), 2.0*h1, 5.0*w1, 2.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);

   //Quadrant 2 arc.
   cairo_pattern_t *pattern2=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern2);
   cairo_mesh_pattern_move_to(pattern2, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 1.0*w1, (5.0*h1)-(points*4.0*height/10), (5.0*w1)-(points*4.0*width/10), 1.0*h1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 5.0*w1, 2.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, (5.0*w1)-(points*3.0*width/10), 2.0*h1,  2.0*w1, (5.0*h1)-(points*3.0*h1), 2.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 2, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 3, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern2);
   cairo_set_source(cr, pattern2);
   cairo_paint(cr);

   //Quadrant 3 arc.
   cairo_pattern_t *pattern3=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern3);
   cairo_mesh_pattern_move_to(pattern3, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, (5.0*w1)-(points*4.0*width/10), 9.0*h1, 1.0*w1, (5.0*h1)+(points*4.0*height/10), 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 2.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 2.0*w1, (5.0*h1)+(points*3.0*height/10),  (5.0*w1)-(points*3.0*width/10), 8.0*h1, 5.0*w1, 8.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 0, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 1, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 2, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch (pattern3);
   cairo_set_source(cr, pattern3);
   cairo_paint(cr);

   //Quadrant 4 arc.
   cairo_pattern_t *pattern4=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern4);
   cairo_mesh_pattern_move_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 9.0*w1, (5.0*h1)+(points*4.0*height/10), (5.0*w1)+(points*4.0*width/10),  9.0*h1, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 5.0*w1, 8.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, (5.0*w1)+(points*3.0*width/10), 8.0*h1,  8.0*w1, (5.0*h1)+(points*3.0*height/10), 8.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 0, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 1, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 2, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 3, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern4);
   cairo_set_source(cr, pattern4);
   cairo_paint(cr);
   
   //Layout axis for drawing.
   cairo_set_line_width(cr, 1.0);
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
   cairo_stroke(cr);
   cairo_move_to(cr, 1.0*w1, 5.0*h1);
   cairo_line_to(cr, 9.0*w1, 5.0*h1);
   cairo_stroke(cr);
   cairo_move_to(cr, 5.0*w1, 1.0*h1);
   cairo_line_to(cr, 5.0*w1, 9.0*h1);
   cairo_stroke(cr);

   cairo_pattern_destroy(pattern1);
   cairo_pattern_destroy(pattern2);
   cairo_pattern_destroy(pattern3);
   cairo_pattern_destroy(pattern4);

   //Set the clock text.
   gint i=0;
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11"};
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   //Start at 12 or radius with just the y component.
   gdouble hour_radius=3.5*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 20);
   cairo_move_to(cr, 0.0, 0.0);
   cairo_translate(cr, width/2.0, height/2.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
     }

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   hour=hour+minute/60.0;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   if(hour>12) hour=hour-12;

   //Hour hand.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=((2.0*w1)*(2.0*h1))/sqrt(((2.0*w1)*(2.0*w1)*temp_sin*temp_sin) + ((2.0*h1)*(2.0*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=((3.0*w1)*(3.0*h1))/sqrt(((3.0*w1)*(3.0*w1)*temp_sin*temp_sin) + ((3.0*h1)*(3.0*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);
  
   return FALSE;
}








