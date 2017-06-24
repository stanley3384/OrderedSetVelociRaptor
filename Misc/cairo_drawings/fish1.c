
/* 
    Test drawing with generated control points. Draw a fish and a spiral. Getting the control
points to draw the curves makes it much easier to draw something like a spiral or animating
a spiral.

    gcc -Wall fish1.c -o fish1 `pkg-config --cflags --libs gtk+-3.0` -lm

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
static gdouble translate=1;

static void combo_changed(GtkComboBox *combo, gpointer data);
static gboolean animate_spiral(gpointer data);
static gboolean draw_fish(GtkWidget *da, cairo_t *cr, gpointer data);
static GArray* control_points_from_coords2(const GArray *dataPoints);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Draw Fish");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(draw_fish), NULL);

   GtkWidget *combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);
   gtk_widget_set_vexpand(combo1, FALSE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Draw Fish");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Draw Spiral");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Animate Spiral");
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

   if(combo_row==2)
     {
       timer_id=g_timeout_add(100, (GSourceFunc)animate_spiral, data);
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
static gboolean animate_spiral(gpointer data)
 {   
   static gboolean backwards=FALSE;
   if(translate>21) backwards=TRUE;
   if(translate<1) backwards=FALSE;
   gtk_widget_queue_draw(GTK_WIDGET(data));  
   if(backwards)translate--; 
   else translate++;  
   return TRUE;
 }
static gboolean draw_fish(GtkWidget *da, cairo_t *cr, gpointer data)
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

   //Sketch a few points for a fish.
   struct point p1;
   GArray *coords=g_array_new(FALSE, FALSE, sizeof(struct point));
   cairo_set_line_width(cr, 3.0);
   if(combo_row==0)
     {
       cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
       p1.x=9.0*w1;
       p1.y=5.0*h1;
       g_array_append_val(coords, p1);
       p1.x=6.0*w1;
       p1.y=3.0*h1;
       g_array_append_val(coords, p1);
       p1.x=2.5*w1;
       p1.y=4.5*h1;
       g_array_append_val(coords, p1);
       p1.x=1.0*w1;
       p1.y=3.0*h1;
       g_array_append_val(coords, p1);
       p1.x=1.25*w1;
       p1.y=5.0*h1;
       g_array_append_val(coords, p1);
       p1.x=1.0*w1;
       p1.y=7.0*h1;
       g_array_append_val(coords, p1);
       p1.x=2.5*w1;
       p1.y=5.5*h1;
       g_array_append_val(coords, p1);
       p1.x=6.0*w1;
       p1.y=7.0*h1;
       g_array_append_val(coords, p1);
       p1.x=9.0*w1;
       p1.y=5.0*h1;
       g_array_append_val(coords, p1);
     }
   else if(combo_row==1) //Some points for a spiral.
     {
       cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
       cairo_translate(cr, width/2.0, height/2.0);
       gdouble temp_x=0;
       gdouble temp_y=0;
       gdouble radius=4.0;
       gdouble circle_points[]={0, G_PI/4.0, G_PI/2.0, 3.0*G_PI/4.0, G_PI, 5.0*G_PI/4.0, 3.0*G_PI/2.0, 7.0*G_PI/4.0};
       gdouble spirals=10;
       for(i=0;i<spirals;i++)
         {
           for(j=0; j<8; j++)
             {
               temp_x=radius*cos(circle_points[j]);
               temp_y=radius*sin(circle_points[j]);
               p1.x=temp_x*w1;
               p1.y=temp_y*h1;
               g_array_append_val(coords, p1);
               radius-=.05;
             }
         }
     }
   else //Animate the spiral.
     {
       cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
       cairo_move_to(cr, 1.0*w1, 9.0*h1);
       cairo_line_to(cr, (1.0+translate/3.0)*w1, 9.0*h1);
       cairo_stroke(cr);
       cairo_translate(cr, (1.0+translate/3.0)*w1, 8.0*h1);
       gdouble temp_x=0;
       gdouble temp_y=0;
       gdouble radius=1.0;
       gdouble circle_points[]={-3.0*G_PI/2.0, -7.0*G_PI/4.0, 0, -G_PI/4.0, -G_PI/2.0, -3.0*G_PI/4.0, -G_PI, -5.0*G_PI/4.0};
       gdouble spirals=3;
       for(i=0;i<spirals;i++)
         {           
           for(j=0;j<8;j++)
             {
               temp_x=radius*cos(circle_points[j]);
               temp_y=radius*sin(circle_points[j]);
               p1.x=temp_x*w1;
               p1.y=temp_y*h1;
               g_array_append_val(coords, p1);
               radius-=.05;
              }
         }
      
     }

   //Get the control points.
   GArray *controlPoints=control_points_from_coords2(coords);

   //Draw the curves.
   struct point d1;
   struct point d2;
   struct controls c1; 
   gint length=coords->len-1;
   if(combo_row==2)
     {
       length=length-translate;
     }
   for(i=0;i<length;i++)
     {
       d1=g_array_index(coords, struct point, i);
       d2=g_array_index(coords, struct point, i+1);
       c1=g_array_index(controlPoints, struct controls, i);
       cairo_move_to(cr, d1.x, d1.y);
       cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
       cairo_stroke(cr);
     }

   if(combo_row==0)
     {
       cairo_arc(cr, 7.5*w1, 4.25*h1, 0.10*h1, 0.0, 2.0*G_PI);
       cairo_fill(cr);
     }  
   
   g_array_free(coords, TRUE);
   g_array_free(controlPoints, TRUE);

   //g_print("Time %f\n", g_timer_elapsed(timer, NULL));
   //g_timer_destroy(timer);

   return FALSE;
 }
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=dataPoints->len-1;
    gdouble *fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
    gdouble *sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count==1)
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

    g_free(fCP);
    g_free(sCP);
    return controlPoints;
  }



