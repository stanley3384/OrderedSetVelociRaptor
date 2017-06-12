
/*
    Test code for drawing squiggles.

    gcc -Wall squiggles1.c -o squiggles1 `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<time.h>
#include<math.h>

static void button_clicked(GtkWidget *button, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_squiggles(GtkWidget *da, cairo_t *cr, gpointer data);

static GRand *rand1=NULL;
static GArray *array1=NULL;

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Squiggles");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    rand1=g_rand_new_with_seed(time(NULL));
    array1=g_array_new(FALSE, FALSE, sizeof(gdouble));

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

    GtkWidget *button1=gtk_button_new_with_label("Redraw");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, FALSE);
    g_signal_connect(button1, "clicked", G_CALLBACK(button_clicked), da);
  
    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);    
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    gtk_main();

    g_rand_free(rand1);
    g_array_free(array1, TRUE);

    return 0;
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

    g_array_remove_range(array1, 0, array1->len);

    gint i=0;
    cairo_set_line_width(cr, 3.0);
    gdouble x1=0;
    gdouble y1=0;
    gdouble diff=0;
    gdouble back_x=width;
    gdouble back_y=height;
    gdouble start_x=0.0;
    gdouble start_y=height;
    gboolean loop=0.0;
    for(i=0;i<201;i++)
      {
        loop=TRUE;
        while(loop)
          {
            x1=g_rand_double(rand1)*width;
            //g_print("x1 %f start %f back %f\n", x1, start_x, back_x);
            //Check if point is inside previous rectangle.
            if((start_x>back_x)&&(x1>start_x)) loop=FALSE;
            else if(x1<back_x) loop=FALSE;
            else loop=TRUE;
            //Keep the size of the squiggle fragment small.
            if(fabs(x1-start_x)/width<0.3&&!loop) break;
            else loop=TRUE;
          }
        loop=TRUE;
        while(loop)
          {
            y1=g_rand_double(rand1)*height;
            if((start_y>back_y)&&(y1>start_y)) loop=FALSE;
            else if(y1<back_y) loop=FALSE;
            else loop=TRUE;
            if(fabs(y1-start_y)/height<0.3&&!loop) break;
            else loop=TRUE;           
          }
        //g_print("%f %f\n", x1, y1);
        g_array_append_val(array1, x1);
        g_array_append_val(array1, y1);
        back_x=start_x;
        back_y=start_y;
        start_x=x1;
        start_y=y1;        
      }

    //Draw the squiggley.
    gdouble red=0.0;
    gdouble green=0.0;
    gdouble blue=1.0;
    gint top=200;
    for(i=0;i<top;i+=2)
      {
        start_x=g_array_index(array1, gdouble, i);
        start_y=g_array_index(array1, gdouble, i+1);
        x1=g_array_index(array1, gdouble, i+2);
        y1=g_array_index(array1, gdouble, i+3);
        diff=start_y-y1;

        //Test rectangles with only a few curves. top=3.
        //cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        //cairo_rectangle(cr, start_x, start_y, (x1-start_x), (y1-start_y));
        //cairo_stroke(cr);
 
        red+=2.0/top;
        blue-=2.0/top;
        cairo_set_source_rgb(cr, red, green, blue);
        cairo_move_to(cr, start_x, start_y);
        //Curve with the control points at the corners of the rectangle.
        cairo_curve_to(cr, start_x, start_y-diff, x1, y1+diff, x1, y1);
        cairo_stroke(cr);
      } 
  }

