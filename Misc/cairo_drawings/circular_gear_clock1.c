
/* 
    Make a circular gear clock out of gear2.c. Keep this one sqaure. Don't deform with an ellipse.
 
    gcc -Wall circular_gear_clock1.c -o circular_gear_clock1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

struct gear_vars{
  gdouble inside_radius;
  gdouble outside_radius;
  gdouble spindle_radius;
  gint teeth;
  gdouble bevel;
  gboolean fill;
  gdouble fill_color[4];
  gdouble rotation;
  gboolean draw_bezier;
}gear_vars;

static gboolean animate_circular_wave(gpointer data);
static gboolean draw_circular_wave(GtkWidget *da, cairo_t *cr, struct gear_vars *g1);
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1, gint second, struct gear_vars *g1);
static void gear(cairo_t *cr, gdouble w1, struct gear_vars *g1);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Circular Gear Clock");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   struct gear_vars g1;
   g1.inside_radius=3.0;
   g1.outside_radius=4.0;
   g1.spindle_radius=2.0;
   g1.teeth=12;
   g1.bevel=G_PI/36.0;
   g1.fill=TRUE;
   g1.fill_color[0]=0.75;
   g1.fill_color[1]=0.75;
   g1.fill_color[2]=0.75;
   g1.fill_color[3]=1.0;
   g1.rotation=-G_PI/2.0; 
   g1.draw_bezier=TRUE;   

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(draw_circular_wave), &g1);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   g_timeout_add(300, (GSourceFunc)animate_circular_wave, da);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static gboolean animate_circular_wave(gpointer data)
 {   
   gtk_widget_queue_draw(GTK_WIDGET(data));  
   return TRUE;
 }
static gboolean draw_circular_wave(GtkWidget *da, cairo_t *cr, struct gear_vars *g1)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //GTimer *timer=g_timer_new();

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   gint second=g_date_time_get_second(date_time);
   hour=hour+minute/60.0;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   if(hour>12) hour=hour-12;

   //Paint background.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Cartesian coordinates for drawing.
   cairo_set_line_width(cr, 1.0);
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
   cairo_stroke(cr);
   cairo_move_to(cr, 1.0*w1, 5.0*h1);
   cairo_line_to(cr, 9.0*w1, 5.0*h1);
   cairo_stroke(cr);
   cairo_move_to(cr, 5.0*w1, 1.0*h1);
   cairo_line_to(cr, 5.0*w1, 9.0*h1);
   cairo_stroke(cr); 

   //Keep the gear drawing circular.
   if(w1>h1) w1=h1;
     
   //Draw the gear or gears.
   gears1(cr, width, height, w1, second, g1);

   //Circular coordinates with marks at each second or pi/30.
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_arc(cr, 0.0, 0.0, 4.0*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   gdouble start=-G_PI/2.0;
   gdouble next_second=-G_PI/30.0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   for(i=0;i<60;i++)
     {
       temp_cos1=cos(start-(next_second*i));
       temp_sin1=sin(start-(next_second*i));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       temp_cos1=temp_cos1*3.6*w1;
       temp_sin1=temp_sin1*3.6*w1;
       temp_cos2=temp_cos2*4.0*w1;
       temp_sin2=temp_sin2*4.0*w1;
       cairo_move_to(cr, temp_cos1, temp_sin1);
       cairo_line_to(cr, temp_cos2, temp_sin2);
       cairo_stroke(cr);
     }

   //Set the clock text.
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CGrC"};
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
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //Keep the gear clock square.
       hour_radius=3.5*w1;
       //The polar form of the equation for an ellipse to get the radius.
       //hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
     }

   //Put the clock name on the clock.
   cairo_text_extents(cr, hours[12], &tick_extents);
   cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
   cairo_show_text(cr, hours[12]);

   //Hour hand.
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=1.5*w1;
   //hour_radius=((1.5*w1)*(1.5*h1))/sqrt(((1.5*w1)*(1.5*w1)*temp_sin*temp_sin) + ((1.5*h1)*(1.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=2.5*w1;
   //hour_radius=((2.5*w1)*(2.5*h1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin*temp_sin) + ((2.5*h1)*(2.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //g_print("Time %f\n", g_timer_elapsed(timer, NULL));
   //g_timer_destroy(timer);

   return FALSE;
 }
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1, gint second, struct gear_vars *g1)
  {
    cairo_save(cr);
    cairo_set_source_rgb(cr, g1->fill_color[0], g1->fill_color[1], g1->fill_color[2]);
    cairo_set_line_width(cr, 8.0);
    cairo_translate(cr, width/2.0, height/2.0);
    cairo_rotate(cr, g1->rotation+second*G_PI/30);

    //Draw the gear. 
    gear(cr, w1, g1);

    //Draw the inside circle of the gear.
    cairo_arc(cr, 0.0, 0.0, g1->spindle_radius*w1, 0, 2*G_PI);
    if(g1->fill)
      {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_fill(cr);
      }
    else cairo_stroke(cr);
   
    //The timing mark at 0 radians on the unit circle.
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_move_to(cr, (g1->inside_radius*w1)+0.5*(g1->outside_radius*w1-g1->inside_radius*w1), 0.0);
    cairo_line_to(cr, g1->outside_radius*w1, 0.0);
    cairo_stroke(cr);  
    cairo_restore(cr);
  }
static void gear(cairo_t *cr, gdouble w1, struct gear_vars *g1)
  {
    gint i=0;
    gdouble step=-G_PI*2.0/(g1->teeth);

    //Outside points.
    gdouble outside_start1=-(g1->bevel);
    gdouble outside_cos1=0;
    gdouble outside_sin1=0;
    gdouble outside_start2=(g1->bevel);
    gdouble outside_cos2=0;
    gdouble outside_sin2=0;
    //Inside points.
    gdouble inside_start1=G_PI/(g1->teeth)-(g1->bevel);
    gdouble inside_cos1=0;
    gdouble inside_sin1=0;
    gdouble inside_start2=G_PI/(g1->teeth)+(g1->bevel);
    gdouble inside_cos2=0;
    gdouble inside_sin2=0;
    //Draw the gear teeth.
    outside_cos1=cos(outside_start1-(step*i))*(g1->outside_radius)*w1;
    outside_sin1=sin(outside_start1-(step*i))*(g1->outside_radius)*w1;                
    cairo_move_to(cr, outside_cos1, outside_sin1);
    if(!g1->draw_bezier)
      {
        for(i=0;i<(g1->teeth);i++)
          {
            outside_cos2=cos(outside_start2-(step*i))*(g1->outside_radius)*w1;
            outside_sin2=sin(outside_start2-(step*i))*(g1->outside_radius)*w1;                
            cairo_line_to(cr, outside_cos2, outside_sin2);
            cairo_stroke_preserve(cr);   
         
            inside_cos1=cos(inside_start1-(step*i))*(g1->inside_radius)*w1;
            inside_sin1=sin(inside_start1-(step*i))*(g1->inside_radius)*w1;
            cairo_line_to(cr, inside_cos1, inside_sin1);
            cairo_stroke_preserve(cr); 

            inside_cos2=cos(inside_start2-(step*i))*(g1->inside_radius)*w1;
            inside_sin2=sin(inside_start2-(step*i))*(g1->inside_radius)*w1;
            cairo_line_to(cr, inside_cos2, inside_sin2);
            cairo_stroke_preserve(cr); 

            outside_cos1=cos(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;
            outside_sin1=sin(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;   
            cairo_line_to(cr, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);                           
         }
      }
    else
      {
        for(i=0;i<(g1->teeth);i++)
          {              
            outside_cos2=cos(outside_start2-(step*i))*(g1->outside_radius)*w1;
            outside_sin2=sin(outside_start2-(step*i))*(g1->outside_radius)*w1;                
            cairo_line_to(cr, outside_cos2, outside_sin2);
            cairo_stroke_preserve(cr);   
         
            inside_cos1=cos(inside_start1-(step*i))*(g1->inside_radius)*w1;
            inside_sin1=sin(inside_start1-(step*i))*(g1->inside_radius)*w1;

            inside_cos2=cos(inside_start2-(step*i))*(g1->inside_radius)*w1;
            inside_sin2=sin(inside_start2-(step*i))*(g1->inside_radius)*w1;

            outside_cos1=cos(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;
            outside_sin1=sin(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;   
            cairo_curve_to(cr, inside_cos1, inside_sin1, inside_cos2, inside_sin2, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);                   
         }
     }
    cairo_close_path(cr);
    if(g1->fill) cairo_fill(cr); 
    else cairo_stroke(cr);
  }
