
/*
    Test code for drawing squiggles. It will draw a continuous squiggle and a smooth and continuous
squiggle. 

    gcc -Wall squiggles1.c -o squiggles1 `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<time.h>
#include<math.h>

struct point{
  gdouble x;
  gdouble y;
}points;

static void check_smooth(GtkToggleButton *check1, gpointer data);
static void button_clicked(GtkWidget *button, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_squiggles(GtkWidget *da, cairo_t *cr, gpointer data);
static void control_points_from_coords(gdouble firstControlPoints[][2], gdouble secondControlPoints[][2]);

static GRand *rand1=NULL;
static GArray *coords=NULL;

gboolean draw_smooth=FALSE;

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Squiggles");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    rand1=g_rand_new_with_seed(time(NULL));
    coords=g_array_new(FALSE, FALSE, sizeof(struct point));

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

    GtkWidget *check1=gtk_check_button_new_with_label("Smooth and Continuous");
    gtk_widget_set_halign(check1, GTK_ALIGN_CENTER);
    g_signal_connect(check1, "toggled", G_CALLBACK(check_smooth), NULL);

    GtkWidget *button1=gtk_button_new_with_label("Redraw");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, FALSE);
    g_signal_connect(button1, "clicked", G_CALLBACK(button_clicked), da);
  
    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);    
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check1, 0, 2, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    gtk_main();

    g_rand_free(rand1);
    g_array_free(coords, TRUE);

    return 0;
  }
static void check_smooth(GtkToggleButton *check1, gpointer data)
  {
    if(gtk_toggle_button_get_active(check1)) draw_smooth=TRUE;
    else draw_smooth=FALSE;
  }
static void button_clicked(GtkWidget *button, gpointer data)
  {
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    draw_squiggles(da, cr, NULL);
    return FALSE;
  }
static void draw_squiggles(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //Cartesian coordinates for drawing.
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 1.0*w1, 5.0*h1);
    cairo_line_to(cr, 9.0*w1, 5.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*w1, 1.0*h1);
    cairo_line_to(cr, 5.0*w1, 9.0*h1);
    cairo_stroke(cr); 

    g_array_remove_range(coords, 0, coords->len);

    gint i=0;
    /*
      Can change the number of points here but keep it reasonable. Also the control point arrays
      are on the stack so don't abuse the privelege, too much.
    */
    gint points=100;
    gdouble diff=0;
    struct point p1;
    gdouble back_x=width;
    gdouble back_y=height;
    gdouble start_x=0.0;
    gdouble start_y=height;
    gboolean loop=0.0;
    for(i=0;i<points;i++)
      {
        loop=TRUE;
        while(loop)
          {
            p1.x=g_rand_double(rand1)*width;
            //g_print("x1 %f start %f back %f\n", x1, start_x, back_x);
            //Check if point is inside previous rectangle.
            if((start_x>back_x)&&(p1.x>start_x)) loop=FALSE;
            else if(p1.x<back_x) loop=FALSE;
            else loop=TRUE;
            //Keep the size of the squiggle fragment small.
            if(fabs(p1.x-start_x)/width<0.3&&!loop) break;
            else loop=TRUE;
          }
        loop=TRUE;
        while(loop)
          {
            p1.y=g_rand_double(rand1)*height;
            if((start_y>back_y)&&(p1.y>start_y)) loop=FALSE;
            else if(p1.y<back_y) loop=FALSE;
            else loop=TRUE;
            if(fabs(p1.y-start_y)/height<0.3&&!loop) break;
            else loop=TRUE;           
          }
        //g_print("%f %f\n", p1.x, p1.y);
        g_array_append_val(coords, p1);
        back_x=start_x;
        back_y=start_y;
        start_x=p1.x;
        start_y=p1.y;        
      }

    gint count=coords->len-1;
    gdouble firstControlPoints[count][2];
    gdouble secondControlPoints[count][2];
    control_points_from_coords(firstControlPoints, secondControlPoints);

    //Draw the squiggley.
    gdouble red=0.0;
    gdouble green=0.0;
    gdouble blue=1.0;
    struct point c1;
    struct point c2;
    cairo_set_line_width(cr, 3.0);
    for(i=0;i<points-1;i++)
      {
        c1=g_array_index(coords, struct point, i);
        c2=g_array_index(coords, struct point, i+1);
        diff=c1.y-c2.y;

        //Test rectangles with only a few curves. top=3.
        //cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        //cairo_rectangle(cr, start_x, start_y, (x1-start_x), (y1-start_y));
        //cairo_stroke(cr);
 
        red+=2.0/points;
        blue-=2.0/points;
        cairo_set_source_rgb(cr, red, green, blue);
        cairo_move_to(cr, c1.x, c1.y);
        if(draw_smooth) cairo_curve_to(cr, firstControlPoints[i][0], firstControlPoints[i][1], secondControlPoints[i][0], secondControlPoints[i][1], c2.x, c2.y);
        //Curve with the control points at the corners of the rectangle.
        else cairo_curve_to(cr, c1.x, c1.y-diff, c2.x, c2.y+diff, c2.x, c2.y);
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

    This is a translation of the original Swift code to C. First time looking at Swift for
    me so there are some things that should be done better. Just tried to get things working here.
    Need to go back through and get some arrays off of the stack and set up the function call better. 
*/
static void control_points_from_coords(gdouble firstControlPoints[][2], gdouble secondControlPoints[][2])
  {  
    gint i=0;      
    //Number of Segments
    gint count=coords->len-1;
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count==1)
      {
        struct point P0=g_array_index(coords, struct point, 0);
        struct point P3=g_array_index(coords, struct point, 1);

        //Calculate First Control Point
        //3P1 = 2P0 + P3
        struct point P1;
        P1.x=(2.0*P0.x+P3.x)/3.0;
        P1.y=(2.0*P0.y+P3.y)/3.0;

        firstControlPoints[0][0]=P1.x;
        firstControlPoints[0][1]=P1.y;

        //Calculate second Control Point
        //P2 = 2P1 - P0
        struct point P2;
        P2.x=(2.0*P1.x-P0.x);
        P2.y=(2.0*P1.y-P0.x);

        secondControlPoints[0][0]=P2.x;
        secondControlPoints[0][1]=P2.y;      
      }
    else
      {
        gdouble rhsArray[count][2];
        gdouble a[count];
        gdouble b[count];
        gdouble c[count];
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
            P0=g_array_index(coords, struct point, i);
            P3=g_array_index(coords, struct point, i+1);

            if(i==0)
              {
                a[0]=0.0;
                b[0]=2.0;
                c[0]=1.0;

                //rhs for first segment
                rhsValueX=P0.x+2.0*P3.x;
                rhsValueY=P0.y+2.0*P3.y;
              }
            else if(i==count-1)
              {
                a[i]=2.0;
                b[i]=7.0;
                c[i]=0.0;

                //rhs for last segment
                rhsValueX=8.0*P0.x+P3.x;
                rhsValueY=8.0*P0.y+P3.y;
              }
            else
              {
                a[i]=1.0;
                b[i]=4.0;
                c[i]=1.0;

                rhsValueX=4.0*P0.x+2.0*P3.x;
                rhsValueY=4.0*P0.y+2.0*P3.y;
              }
            rhsArray[i][0]=rhsValueX;
            rhsArray[i][1]=rhsValueY;
          }

        //Solve Ax=B. Use Tridiagonal matrix algorithm a.k.a Thomas Algorithm
        for(i=1;i<count;i++)
          {
            m=a[i]/b[i-1];

            b1=b[i]-m*c[i-1];
            b[i]=b1;

            r2x=rhsArray[i][0]-m*rhsArray[i-1][0];
            r2y=rhsArray[i][1]-m*rhsArray[i-1][1];

            rhsArray[i][0]=r2x;
            rhsArray[i][1]=r2y;
          }

        //Get First Control Points
        
        //Last control Point
        gdouble lastControlPointX=rhsArray[count-1][0]/b[count-1];
        gdouble lastControlPointY=rhsArray[count-1][1]/b[count-1];

        firstControlPoints[count-1][0]=lastControlPointX;
        firstControlPoints[count-1][1]=lastControlPointY;

        gdouble controlPointX=0;
        gdouble controlPointY=0;

        for(i=count-2;i>=0;--i)
          {
            controlPointX=(rhsArray[i][0]-c[i]*firstControlPoints[i+1][0])/b[i];
            controlPointY=(rhsArray[i][1]-c[i]*firstControlPoints[i+1][1])/b[i];

            firstControlPoints[i][0]=controlPointX;
            firstControlPoints[i][1]=controlPointY; 
          }
    
        //Compute second Control Points from first.
        for(i=0;i<count;i++)
          {
            if(i==count-1)
              {
                P3=g_array_index(coords, struct point, i+1);
                P1_x=firstControlPoints[0][0];
                P1_y=firstControlPoints[0][1];

                controlPointX=(P3.x+P1_x)/2.0;
                controlPointY=(P3.y+P1_y)/2.0;

                secondControlPoints[count-1][0]=controlPointX;
                secondControlPoints[count-1][1]=controlPointY;
              }
            else
              {
                P3=g_array_index(coords, struct point, i+1);                
                P1_x=firstControlPoints[i+1][0];
                P1_y=firstControlPoints[i+1][1];

                controlPointX=2.0*P3.x-P1_x;
                controlPointY=2.0*P3.y-P1_y;

                secondControlPoints[i][0]=controlPointX;
                secondControlPoints[i][1]=controlPointY;
              }

          }

     }
  }



