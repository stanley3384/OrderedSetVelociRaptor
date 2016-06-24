
/*

    Draw a mandelbrot set onto a GDK pixbuf then use cairo to draw a couple of bug eyes.
 
    Try both of the following to see if there is a difference in the drawing. There is more green
with the -O2 flag on my test computer.

    gcc -Wall -Werror mandelbrot1.c -o mandelbrot1 `pkg-config --cflags --libs gtk+-3.0`
    gcc -Wall -Werror -O2 mandelbrot1.c -o mandelbrot1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/
  
#include <gtk/gtk.h>

//The status of the mandelbrot drawing. It can take some time.
static gint status=0;

//Draw the mandelbrot set on the pixbuf on a seperate thread.
static gpointer draw_mandelbrot(GdkPixbuf *pixbuf);
//Use cairo to draw a couple of bug eyes.
static gboolean draw_mandelbrot_bug(GtkWidget *da, cairo_t *cr, GdkPixbuf *pixbuf);
//Check to see if the drawing is done. If it is, put it in the drawing area.
static gboolean check_pixbuf_status(GtkWidget *widgets[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot Bug");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GdkPixbuf *pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 500, 500);

    g_thread_new("thread1", (GThreadFunc)draw_mandelbrot, pixbuf);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 500, 500);
    g_signal_connect(da, "draw", G_CALLBACK(draw_mandelbrot_bug), pixbuf);

    GtkWidget *statusbar=gtk_statusbar_new();
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), 0, "Drawing Mandelbrot Bug");
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), statusbar, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    GtkWidget *widgets[]={da, statusbar};
    g_timeout_add(500, (GSourceFunc)check_pixbuf_status, widgets);

    gtk_widget_show_all(window);                  
    gtk_main();

    g_object_unref(pixbuf);

    return 0;
  }
static gpointer draw_mandelbrot(GdkPixbuf *pixbuf)
  {
    GTimer *timer=g_timer_new();
    gdouble x1=0.0;
    gdouble y1=0.0;
    gdouble x2=0.0;
    gdouble y2=0.0;
    gint iteration;
    gint max_iteration=25;
    gdouble scale=1.0;
    gdouble temp1=0.0;
    gdouble temp2=0.0;
    gdouble max_value=0.0;
    gint i=0;
    gint j=0;
    gint width=gdk_pixbuf_get_width(pixbuf);
    gint height=gdk_pixbuf_get_height(pixbuf);
    gint step=gdk_pixbuf_get_rowstride(pixbuf);
    gint channels=gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels, *p;
    pixels=gdk_pixbuf_get_pixels(pixbuf);

    for(i=0;i<width;i++)
      {
        for(j=0;j<height;j++)
          {
            //Get some mandelbrot values. Change things around.
            x1=0.0;
            y1=0.0;
            //Scale
            x2=(((gdouble)i)/((gdouble)width)-0.5)/scale*3.0-0.7;
            y2=(((gdouble)j)/((gdouble)height)-0.5)/scale*3.0;
            //Test different starting points.
            for(iteration=2;iteration<max_iteration;iteration++)
              {
                temp1=x1*x1-y1*y1+x2;
                y1=2.0*x1*y1+y2;
                x1=temp1;                
              }

            //Fill in the pixbuf.
            temp2=x1*x1+y1*y1;
            if(temp2>max_value) max_value=temp2;
            p=pixels+j*step+i*channels;
            if(temp2<100.0)
              {
                if(temp2<0.2)
                  {
                    p[0]=(gint)(255.0-(255.0*temp2/0.2));
                    p[1]=0;
                    p[2]=(gint)(0.0+(255.0*temp2/0.2));
                  }
                else
                  {
                    p[0]=0;
                    p[1]=0;
                    p[2]=255;
                  }
              }
            else
              {
                p[0]=0;
                p[1]=(gint)(255.0-(255.0*temp2/max_value));
                p[2]=0;
              }
          }
      }
    g_atomic_int_set(&status, 1);
    g_print("Mandelbrot Drawing Time %f\n", g_timer_elapsed(timer, NULL));
    g_timer_destroy(timer);
    return NULL;
  }
static gboolean draw_mandelbrot_bug(GtkWidget *da, cairo_t *cr, GdkPixbuf *pixbuf)
  {
    if(g_atomic_int_get(&status)==1)
      {
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_paint(cr);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_arc(cr, 185, 235, 8, 0, 2*G_PI);
        cairo_fill(cr);
        cairo_stroke(cr);
        cairo_arc(cr, 185, 265, 8, 0, 2*G_PI);
        cairo_fill(cr);
        cairo_stroke(cr);
      }
    return FALSE;
  }
static gboolean check_pixbuf_status(GtkWidget *widgets[])
  {
    gint i=0;
    static gint j=1;
    if(g_atomic_int_get(&status)==1)
      {
        gtk_statusbar_push(GTK_STATUSBAR(widgets[1]), j, "Drawing Done");
        gtk_widget_queue_draw(widgets[0]);
        return FALSE;
      }
    else 
      {
        GString *string=g_string_new("Drawing Mandelbrot Bug");
        for(i=0;i<j;i++) g_string_append_c(string, '.');
        gtk_statusbar_push(GTK_STATUSBAR(widgets[1]), j, string->str);
        g_string_free(string, TRUE);
        j++;
        return TRUE;
      }
  }
