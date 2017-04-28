
/*
    Test a pattern with Gouraud shading. Look at some different patterns to maybe put in
the center of the circular_gradient_clock1.c. Makes a good clock on its own.

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

   g_timeout_add(400, (GSourceFunc)time_redraw, da);

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
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   
   //The seconds value.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gint hours=g_date_time_get_hour(date_time);
   gint minutes=g_date_time_get_minute(date_time);
   gint seconds=g_date_time_get_second(date_time);
   gboolean pm=FALSE;
   if(hours>12)
     {
       hours=hours-12;
       pm=TRUE;
     }
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Set background as transparent.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
   cairo_paint(cr);

   //Black inside.
   cairo_save(cr);
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 4.0*w1, 4.0*h1);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_fill(cr);
   cairo_restore(cr);

   //The sweeping line and tick marks.
   cairo_save(cr);
   gdouble start=-G_PI/2.0;
   gdouble next_second=-G_PI/30.0;
   gdouble line_radius1=0;
   gdouble line_radius2=0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   gdouble prev_cos=0;
   gdouble prev_sin=0;
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_line_width(cr, 2.0);
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); 
   for(i=0;i<60;i++)
     {
       temp_cos1=cos(start-(next_second*i));
       temp_sin1=sin(start-(next_second*i));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       //The polar form of the equation for an ellipse to get the radius.
       line_radius1=((3.6*w1)*(3.6*h1))/sqrt(((3.6*w1)*(3.6*w1)*temp_sin1*temp_sin1) + ((3.6*h1)*(3.6*h1)*temp_cos1*temp_cos1));
       line_radius2=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin1*temp_sin1) + ((4.0*h1)*(4.0*h1)*temp_cos1*temp_cos1));
       temp_cos1=temp_cos1*line_radius1;
       temp_sin1=temp_sin1*line_radius1;
       temp_cos2=temp_cos2*line_radius2;
       temp_sin2=temp_sin2*line_radius2;
       cairo_move_to(cr, temp_cos1, temp_sin1);
       cairo_line_to(cr, temp_cos2, temp_sin2);
       cairo_stroke(cr);
       if(i==seconds)
         {
           //Extend outside the inside circle to fill the clip region. Cover with a outside ring.
           line_radius1=((4.1*w1)*(4.1*h1))/sqrt(((4.1*w1)*(4.1*w1)*temp_sin1*temp_sin1) + ((4.1*h1)*(4.1*h1)*temp_cos1*temp_cos1));
           prev_cos=prev_cos*line_radius1;
           prev_sin=prev_sin*line_radius1;
           temp_cos1=temp_cos1*line_radius1;
           temp_sin1=temp_sin1*line_radius1;
           cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
           cairo_mesh_pattern_begin_patch(pattern1);
           cairo_mesh_pattern_move_to(pattern1, prev_cos, prev_sin);
           cairo_mesh_pattern_line_to(pattern1, temp_cos1, temp_sin1);
           cairo_mesh_pattern_line_to(pattern1, 0.0, 0.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, 1.0, 0.0, 1.0, 0.7);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, 0.0, 0.0, 1.0, 1.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, 1.0, 0.0, 1.0, 0.7);
           cairo_mesh_pattern_end_patch(pattern1);
           cairo_set_source(cr, pattern1);
           cairo_paint(cr);   
           cairo_pattern_destroy(pattern1);
           cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
         }
       prev_cos=temp_cos1;
       prev_sin=temp_sin1;
    }
   cairo_restore(cr);

   //Black outside ring.
   cairo_save(cr);
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_set_line_width(cr, .08);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 4.0*w1, 4.0*h1);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   cairo_restore(cr);

   //Draw the seconds
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 25*width/400);
   gchar *string=NULL;
   if(seconds<10)
     {
       if(pm) string=g_strdup_printf("%i:%i:0%i PM", hours, minutes, seconds);
       else string=g_strdup_printf("%i:%i:0%i AM", hours, minutes, seconds);
     }
   else
     {
       if(pm) string=g_strdup_printf("%i:%i:%i PM", hours, minutes, seconds);
       else string=g_strdup_printf("%i:%i:%i AM", hours, minutes, seconds);
     }
   cairo_text_extents(cr, string, &tick_extents);
   cairo_move_to(cr, 0.0-tick_extents.width/2.0, 1.5*h1+tick_extents.height/2.0);
   cairo_show_text(cr, string);
   g_free(string);
        
   return FALSE;
 }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }








