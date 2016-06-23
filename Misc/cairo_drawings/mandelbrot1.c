
/*

    Draw a mandelbrot set onto a GDK pixbuf.
 
    gcc -Wall mandelbrot1.c -o mandelbrot1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/
  
#include <gtk/gtk.h>

static void draw_mandelbrot(GdkPixbuf *pixbuf);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbuf *pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, 500, 500);
    draw_mandelbrot(pixbuf);
    GtkWidget *image=gtk_image_new_from_pixbuf(pixbuf);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);                  
    gtk_main();

    g_object_unref(pixbuf);

    return 0;
  }
static void draw_mandelbrot(GdkPixbuf *pixbuf)
  {
    gdouble x1=0.0;
    gdouble y1=0.0;
    gdouble x2=0.0;
    gdouble y2=0.0;
    gint iteration;
    gint max_iteration=255;
    gdouble scale=1.0;
    gdouble temp1=0;
    gdouble temp2=0;
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
            x1 = 0.0;
            y1 = 0.0;
            //Scale
            x2 = (((gdouble)i)/((gdouble)width)-0.5)/scale*3.0-0.7;
            y2 = (((gdouble)j)/((gdouble)height)-0.5)/scale*3.0;
            //Test different starting points.
            for(iteration=3;iteration<max_iteration;iteration++)
              {
                temp1=x1*x1-y1*y1+x2;
                y1=2.0*x1*y1+y2;
                x1 = temp1;
                if(x1*x1+y1*y1>255.0) iteration=999999;
              }

            //Fill in the pixbuf.
            temp2=x1*x1+y1*y1;
            p=pixels+j*step+i*channels;
            if(iteration<99999)
              {
                if(temp2<0.02)
                  {
                    p[0]=255;
                    p[1]=0;
                    p[2]=255;
                  }
                else
                  {
                    p[0]=(gint)(255.0-(255.0*temp2/0.02));
                    p[1]=0;
                    p[2]=(gint)(0+(255.0*temp2/0.02));
                  }
              }
            else
              {
                p[0]=0;
                p[1]=200;
                p[2]=255;
              }
          }
      }
  }

