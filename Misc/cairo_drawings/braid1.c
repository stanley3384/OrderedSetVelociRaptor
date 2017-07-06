
/* 
    Test code for drawing a braid or a weave with three strands. Use a smoothing function to
get the control points for the sine and cosine curves to be braided. Draw a linear braid and
a circular braid.

    gcc -Wall braid1.c -o braid1 `pkg-config --cflags --libs gtk+-3.0` -lm

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

static gint combo_row=0;
static guint timer_id=0;
static gdouble translate=0;

static void combo_changed(GtkComboBox *combo, gpointer data);
static gboolean animate_braid(gpointer data);
static gboolean draw_braid(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_coords1(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2);
static void draw_coords2(cairo_t *cr, GArray *coords2, GArray *control2, gint i, gint half2);
static void draw_coords3(cairo_t *cr, GArray *coords3, GArray *control3, gint i, gint half2);
static GArray* control_points_from_coords2(const GArray *dataPoints);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Draw Braid");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(draw_braid), NULL);

   GtkWidget *combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);
   gtk_widget_set_vexpand(combo1, FALSE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Draw Braid");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Animate Braid");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Circular Braid");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Animate Circular Braid");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
   g_signal_connect(combo1, "changed", G_CALLBACK(combo_changed), da);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static void combo_changed(GtkComboBox *combo, gpointer data)
 {
   combo_row=gtk_combo_box_get_active(combo);

   if(combo_row==1||combo_row==3)
     {
       translate=0;
       if(timer_id==0)
         {
           timer_id=g_timeout_add(100, (GSourceFunc)animate_braid, data);
         }
     }
   else
     {
       if(timer_id!=0)
         {

           g_source_remove(timer_id); 
           timer_id=0;
           translate=1;
         }
     }
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static gboolean animate_braid(gpointer data)
 {   
   static gboolean backwards=FALSE;
   if(combo_row==1)
     {
       if(translate>22) backwards=TRUE;
       if(translate<1) backwards=FALSE;
     }
   else
     {
       backwards=FALSE;
       if(translate>46) translate=0;       
     }
   gtk_widget_queue_draw(GTK_WIDGET(data));  
   if(backwards)translate--; 
   else translate++;  
   return TRUE;
 }
static gboolean draw_braid(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gint i=0;
   gint j=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //GTimer *timer=g_timer_new();

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

   //Sketch a few points for a braid. The smoothing function will take care of the control points.
   gint rotations=3;
   gint inc=8;
   struct point p1;
   GArray *coords1=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords2=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords3=g_array_new(FALSE, FALSE, sizeof(struct point));
   //The linear braids.
   if(combo_row==0||combo_row==1)
     {
       gdouble sind1=1.0;
       gdouble sind2=0.0;
       for(i=0;i<rotations;i++)
         {
           for(j=0;j<inc;j++)
             {         
               p1.x=(((gdouble)(i*inc+j)/(gdouble)rotations)+1.0)*w1;
               p1.y=(5.0-sind1)*h1;
               g_array_append_val(coords1, p1);
               p1.y=(5.0+sind1)*h1;
               g_array_append_val(coords2, p1);
               p1.y=(5.0+sind2)*h1;
               g_array_append_val(coords3, p1);
               //g_print("%i %f %f %f\n", i*inc+j, p1.x/w1, p1.y/h1, sind);  
               sind1=(1.0+(gdouble)translate/23.0)*sin(G_PI*2.0*((gdouble)j+1)/inc+G_PI/2.0);
               sind2=(1.0+(gdouble)translate/23.0)*sin(G_PI*2.0*((gdouble)j+1)/inc);
             }
         } 
       p1.x=9.0*w1;
       p1.y=4.0*h1;
       g_array_append_val(coords1, p1);
       p1.x=9.0*w1;
       p1.y=6.0*h1;
       g_array_append_val(coords2, p1); 
       p1.x=9.0*w1;
       p1.y=5.0*h1;
       g_array_append_val(coords3, p1);       
     }  

   //Try a circular braid.
   if(combo_row==2||combo_row==3)
     {
       rotations=6;
       inc=8;
       gdouble sind1=1.0;
       gdouble sind2=0.0;
       for(i=0;i<rotations;i++)
         {
           for(j=0;j<inc;j++)
             {         
               p1.x=(3.0-sind1)*w1*cos((i*inc+j)*G_PI/24.0);
               p1.y=(3.0-sind1)*h1*sin((i*inc+j)*G_PI/24.0);
               g_array_append_val(coords1, p1);
               p1.x=(3.0+sind1)*w1*cos((i*inc+j)*G_PI/24.0);
               p1.y=(3.0+sind1)*h1*sin((i*inc+j)*G_PI/24.0);
               g_array_append_val(coords2, p1);
               p1.x=(3.0+sind2)*w1*cos((i*inc+j)*G_PI/24.0);
               p1.y=(3.0+sind2)*h1*sin((i*inc+j)*G_PI/24.0);
               g_array_append_val(coords3, p1);  
               sind1=(1.0+(gdouble)translate/47.0)*sin(G_PI*2.0*((gdouble)j+1)/inc+G_PI/2.0);
               sind2=(1.0+(gdouble)translate/47.0)*sin(G_PI*2.0*((gdouble)j+1)/inc);
             }
         } 
       p1.x=2.0*w1;
       p1.y=0.0;
       g_array_append_val(coords1, p1);
       p1.x=4.0*w1;
       p1.y=0.0;
       g_array_append_val(coords2, p1); 
       p1.x=3.0*w1;
       p1.y=0.0;
       g_array_append_val(coords3, p1);    
       cairo_translate(cr, width/2.0, height/2.0);
     }
   
   //Get the control points.
   GArray *control1=control_points_from_coords2(coords1);
   GArray *control2=control_points_from_coords2(coords2);
   GArray *control3=control_points_from_coords2(coords3);

   //Draw the curves.
   gint draw_order=-1;
   cairo_set_line_width(cr, 15.0);
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
             draw_coords1(cr, coords1, control1, i, half2);
             draw_coords2(cr, coords2, control2, i, half2);
             draw_coords3(cr, coords3, control3, i, half2);
             break;
           case 1:
             draw_coords3(cr, coords3, control3, i, half2);
             draw_coords1(cr, coords1, control1, i, half2);
             draw_coords2(cr, coords2, control2, i, half2);
             break;
           case 2:
             draw_coords2(cr, coords2, control2, i, half2);
             draw_coords3(cr, coords3, control3, i, half2);
             draw_coords1(cr, coords1, control1, i, half2);
             break;
         }          
     }
   
   g_array_free(coords1, TRUE);
   g_array_free(coords2, TRUE);
   g_array_free(coords3, TRUE);
   g_array_free(control1, TRUE);
   g_array_free(control2, TRUE);
   g_array_free(control3, TRUE);

   //g_print("Time %f\n", g_timer_elapsed(timer, NULL));
   //g_timer_destroy(timer);

   return FALSE;
 }
static void draw_coords1(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(translate==index&&(combo_row==1||combo_row==3)) cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
        else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        d1=g_array_index(coords1, struct point, index);
        d2=g_array_index(coords1, struct point, index+1);
        c1=g_array_index(control1, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
static void draw_coords2(cairo_t *cr, GArray *coords2, GArray *control2, gint i, gint half2)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(translate==index&&(combo_row==1||combo_row==3)) cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        d1=g_array_index(coords2, struct point, index);
        d2=g_array_index(coords2, struct point, index+1);
        c1=g_array_index(control2, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
static void draw_coords3(cairo_t *cr, GArray *coords3, GArray *control3, gint i, gint half2)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(translate==index&&(combo_row==1||combo_row==3)) cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        else cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
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
    gint count=dataPoints->len-1;
    gdouble *fCP=NULL;
    gdouble *sCP=NULL;

    if(count>0)
      {
        fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
      }
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count<1)
      {
        //Return NULL.
        controlPoints=NULL;
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



