
/* 
    Make a circular braid clock out of braid1.c. Animate the seconds in the braid.
 
    gcc -Wall circular_braid_clock1.c -o circular_braid_clock1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

struct point{
  gdouble x;
  gdouble y;
}points;

struct controls{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
}controls;

static gboolean animate_circular_wave(gpointer data);
static gboolean draw_circular_wave(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_coords1(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second);
static void draw_coords2(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second);
static void draw_coords3(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second);
static GArray* control_points_from_coords2(const GArray *dataPoints);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Circular Braid Clock");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(draw_circular_wave), NULL);
   
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
static gboolean draw_circular_wave(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gint i=0;
   gint j=0;
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

   //Points for a circular braid clock.
   gint rotations=6;
   gint inc=10;
   struct point p1;
   GArray *coords1=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords2=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords3=g_array_new(FALSE, FALSE, sizeof(struct point));
   gdouble sind1=-0.75;
   gdouble sind2=0.0;
   for(i=0;i<rotations;i++)
     {
       for(j=0;j<inc;j++)
         {         
           p1.x=(3.0-sind1)*w1*cos((i*inc+j)*G_PI/30.0-G_PI/2.0);
           p1.y=(3.0-sind1)*h1*sin((i*inc+j)*G_PI/30.0-G_PI/2.0);
           g_array_append_val(coords1, p1);
           p1.x=(3.0+sind1)*w1*cos((i*inc+j)*G_PI/30.0-G_PI/2.0);
           p1.y=(3.0+sind1)*h1*sin((i*inc+j)*G_PI/30.0-G_PI/2.0);
           g_array_append_val(coords2, p1);
           p1.x=(3.0+sind2)*w1*cos((i*inc+j)*G_PI/30.0-G_PI/2.0);
           p1.y=(3.0+sind2)*h1*sin((i*inc+j)*G_PI/30.0-G_PI/2.0);
           g_array_append_val(coords3, p1);  
           sind1=(0.75)*sin(G_PI*2.0*((gdouble)j+1)/inc-G_PI/2.0);
           sind2=(0.75)*sin(G_PI*2.0*((gdouble)j+1)/inc);
         }
     } 
   p1.x=0.0;
   p1.y=-3.75*h1;
   g_array_append_val(coords1, p1);
   p1.x=0.0;
   p1.y=-2.25*h1;
   g_array_append_val(coords2, p1); 
   p1.x=0.0;
   p1.y=-3.0*h1;
   g_array_append_val(coords3, p1);     
  
   //Get the control points.
   GArray *control1=control_points_from_coords2(coords1);
   GArray *control2=control_points_from_coords2(coords2);
   GArray *control3=control_points_from_coords2(coords3);

   //Draw from the center.
   cairo_translate(cr, width/2.0, height/2.0);
    
   //Draw the curves.
   gint draw_order=-1;
   cairo_set_line_width(cr, 20.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   gint half1=2*rotations;
   gint half2=inc/2;    
   for(i=0;i<half1;i++)
     {
       draw_order++;
       if(draw_order>2) draw_order=0;
       //Braid order 1 to 3, 2 to 1, 3 to 2.
       switch(draw_order)
         {
           case 0:
             draw_coords1(cr, coords1, control1, i, half2, second);
             draw_coords2(cr, coords2, control2, i, half2, second);
             draw_coords3(cr, coords3, control3, i, half2, second);
             break;
           case 1:
             draw_coords3(cr, coords3, control3, i, half2, second);
             draw_coords1(cr, coords1, control1, i, half2, second);
             draw_coords2(cr, coords2, control2, i, half2, second);
             break;
           case 2:
             draw_coords2(cr, coords2, control2, i, half2, second);
             draw_coords3(cr, coords3, control3, i, half2, second);
             draw_coords1(cr, coords1, control1, i, half2, second);
             break;
         }          
     }
   
   g_array_free(coords1, TRUE);
   g_array_free(coords2, TRUE);
   g_array_free(coords3, TRUE);
   g_array_free(control1, TRUE);
   g_array_free(control2, TRUE);
   g_array_free(control3, TRUE);

   //Set the clock text.
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CBC"};
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
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
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
   hour_radius=((1.5*w1)*(1.5*h1))/sqrt(((1.5*w1)*(1.5*w1)*temp_sin*temp_sin) + ((1.5*h1)*(1.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=((2.5*w1)*(2.5*h1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin*temp_sin) + ((2.5*h1)*(2.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //g_print("Time %f\n", g_timer_elapsed(timer, NULL));
   //g_timer_destroy(timer);

   return FALSE;
 }
static void draw_coords1(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(second==index) cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
        d1=g_array_index(coords1, struct point, index);
        d2=g_array_index(coords1, struct point, index+1);
        c1=g_array_index(control1, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
static void draw_coords2(cairo_t *cr, GArray *coords2, GArray *control2, gint i, gint half2, gint second)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(second==index) cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
        d1=g_array_index(coords2, struct point, index);
        d2=g_array_index(coords2, struct point, index+1);
        c1=g_array_index(control2, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
static void draw_coords3(cairo_t *cr, GArray *coords3, GArray *control3, gint i, gint half2, gint second)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(second==index) cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        d1=g_array_index(coords3, struct point, index);
        d2=g_array_index(coords3, struct point, index+1);
        c1=g_array_index(control3, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
/*
    This is some exellent work done by Ramsundar Shandilya. Note the following for the original work
    and the rational behind it.
    
    https://medium.com/@ramshandilya/draw-smooth-curves-through-a-set-of-points-in-ios-34f6d73c8f9

    https://github.com/Ramshandilya/Bezier

    The MIT License (MIT)

    Copyright (c) 2015 Ramsundar Shandilya

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
  
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    This is a translation of the original Swift code to C. It makes the function easy to use with GTK+,
    cairo and glib.
*/
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=0;
    if(dataPoints!=NULL) count=dataPoints->len-1;
    gdouble *fCP=NULL;
    gdouble *sCP=NULL;

    if(count>0)
      {
        fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
      }
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count<1||dataPoints==NULL)
      {
        //Return NULL.
        controlPoints=NULL;
        g_print("Can't get control points from coordinates. NULL returned.\n");
      }
    else if(count==1)
      {
        struct point P0=g_array_index(dataPoints, struct point, 0);
        struct point P3=g_array_index(dataPoints, struct point, 1);

        //Calculate First Control Point
        //3P1 = 2P0 + P3
        struct point P1;
        P1.x=(2.0*P0.x+P3.x)/3.0;
        P1.y=(2.0*P0.y+P3.y)/3.0;

        *(fCP)=P1.x;
        *(fCP+1)=P1.y;

        //Calculate second Control Point
        //P2 = 2P1 - P0
        struct point P2;
        P2.x=(2.0*P1.x-P0.x);
        P2.y=(2.0*P1.y-P0.x);

        *(sCP)=P2.x;
        *(sCP+1)=P2.y;      
      }
    else
      {
        gdouble *rhs=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        gdouble *a=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *b=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *c=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble rhsValueX=0;
        gdouble rhsValueY=0;
        struct point P0;
        struct point P3;        
        gdouble m=0;
        gdouble b1=0;
        gdouble r2x=0;
        gdouble r2y=0;
        gdouble P1_x=0;
        gdouble P1_y=0;
   
        for(i=0;i<count;i++)
          {
            P0=g_array_index(dataPoints, struct point, i);
            P3=g_array_index(dataPoints, struct point, i+1);

            if(i==0)
              {
                *(a)=0.0;
                *(b)=2.0;
                *(c)=1.0;

                //rhs for first segment
                rhsValueX=P0.x+2.0*P3.x;
                rhsValueY=P0.y+2.0*P3.y;
              }
            else if(i==count-1)
              {
                *(a+i)=2.0;
                *(b+i)=7.0;
                *(c+i)=0.0;

                //rhs for last segment
                rhsValueX=8.0*P0.x+P3.x;
                rhsValueY=8.0*P0.y+P3.y;
              }
            else
              {
                *(a+i)=1.0;
                *(b+i)=4.0;
                *(c+i)=1.0;

                rhsValueX=4.0*P0.x+2.0*P3.x;
                rhsValueY=4.0*P0.y+2.0*P3.y;
              }
            *(rhs+i*2)=rhsValueX;
            *(rhs+i*2+1)=rhsValueY;
          }

        //Solve Ax=B. Use Tridiagonal matrix algorithm a.k.a Thomas Algorithm
        for(i=1;i<count;i++)
          {
            m=(*(a+i))/(*(b+i-1));

            b1=(*(b+i))-m*(*(c+i-1));
            *(b+i)=b1;

            r2x=(*(rhs+i*2))-m*(*(rhs+(i-1)*2));
            r2y=(*(rhs+i*2+1))-m*(*(rhs+(i-1)*2+1));

            *(rhs+i*2)=r2x;
            *(rhs+i*2+1)=r2y;
          }

        //Get First Control Points
        
        //Last control Point
        gdouble lastControlPointX=(*(rhs+2*count-2))/(*(b+count-1));
        gdouble lastControlPointY=(*(rhs+2*count-1))/(*(b+count-1));

        *(fCP+2*count-2)=lastControlPointX;
        *(fCP+2*count-1)=lastControlPointY;

        gdouble controlPointX=0;
        gdouble controlPointY=0;

        for(i=count-2;i>=0;--i)
          {
            controlPointX=(*(rhs+i*2)-(*(c+i))*(*(fCP+(i+1)*2)))/(*(b+i));
            controlPointY=(*(rhs+i*2+1)-(*(c+i))*(*(fCP+(i+1)*2+1)))/(*(b+i));

             *(fCP+i*2)=controlPointX;
             *(fCP+i*2+1)=controlPointY; 
          }

        //Compute second Control Points from first.
        for(i=0;i<count;i++)
          {
            if(i==count-1)
              {
                P3=g_array_index(dataPoints, struct point, i+1);
                P1_x=(*(fCP+i*2));
                P1_y=(*(fCP+i*2+1));

                controlPointX=(P3.x+P1_x)/2.0;
                controlPointY=(P3.y+P1_y)/2.0;

                *(sCP+count*2-2)=controlPointX;
                *(sCP+count*2-1)=controlPointY;
              }
            else
              {
                P3=g_array_index(dataPoints, struct point, i+1);                
                P1_x=(*(fCP+(i+1)*2));
                P1_y=(*(fCP+(i+1)*2+1));

                controlPointX=2.0*P3.x-P1_x;
                controlPointY=2.0*P3.y-P1_y;

                *(sCP+i*2)=controlPointX;
                *(sCP+i*2+1)=controlPointY;
              }

          }

        controlPoints=g_array_new(FALSE, FALSE, sizeof(struct controls));
        struct controls cp;
        for(i=0;i<count;i++)
          {
            cp.x1=(*(fCP+i*2));
            cp.y1=(*(fCP+i*2+1));
            cp.x2=(*(sCP+i*2));
            cp.y2=(*(sCP+i*2+1));
            g_array_append_val(controlPoints, cp);
          }

        g_free(rhs);
        g_free(a);
        g_free(b);
        g_free(c);
     }

    if(fCP!=NULL) g_free(fCP);
    if(sCP!=NULL) g_free(sCP);

    return controlPoints;
  }



