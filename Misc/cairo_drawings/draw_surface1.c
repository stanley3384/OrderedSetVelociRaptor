
/*
    Test code for drawing a cairo surface on a separate thread. The gradient drawing in the
Adjustable Gauge widget can be slower than a frame clock .0167 sec. Look at only redrawing
the background when the window size changes and reusing the background surface. That way only
the needle needs to be drawn on each frame which should speed things up and make the animation
smooth.

    gcc -Wall draw_surface1.c -o draw_surface1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean tick_draw(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data);
static gpointer thread_draw_new_surface(gpointer data);
static gboolean thread_join(gpointer data);
static void resize_window(GtkContainer *container, gpointer data);

//Test variable that are going to be needed.
static GtkWidget *window=NULL;
static GtkWidget *da=NULL;
static gboolean resize=FALSE;
static gdouble rotate=G_PI;
static GThread *thread=NULL;
static cairo_surface_t *surface=NULL;
static gdouble width_height[2];
static gboolean thread_active=FALSE;

int main(int argc, char **argv)
  {
    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Surface");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    width_height[0]=400.0;
    width_height[1]=400.0;

    da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

    gtk_widget_add_tick_callback(da, (GtkTickCallback)tick_draw, NULL, NULL); 

    g_signal_connect(window, "check-resize", G_CALLBACK(resize_window), da);
   
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;  
  }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    width_height[0]=(gdouble)gtk_widget_get_allocated_width(da);
    width_height[1]=(gdouble)gtk_widget_get_allocated_height(da);

    if(resize&&!thread_active)
      {
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_paint(cr);
        if(thread!=NULL)
          {
            g_thread_unref(thread);
            thread_active=TRUE;
            resize=FALSE;
            thread=g_thread_new("thread1", (GThreadFunc)thread_draw_new_surface, NULL);
          }
        else
          {
            thread_active=TRUE;
            resize=FALSE;
            thread=g_thread_new("thread1", (GThreadFunc)thread_draw_new_surface, NULL);
          }
      }
    else
      {
        if(surface!=NULL&&!thread_active)
          {
            cairo_set_source_surface(cr, surface, 0, 0);
            cairo_paint(cr);
            gdouble w1=0;
            if(width_height[0]<width_height[1]) w1=4.0*width_height[0]/10.0;
            else w1=4.0*width_height[1]/10.0;
            cairo_translate(cr, width_height[0]/2.0, width_height[1]/2.0);
            cairo_rotate(cr, rotate);
            cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
            cairo_set_line_width(cr, 3.0);
            cairo_move_to(cr, 0.0, 0.0);
            cairo_line_to(cr, w1, 0.0);
            cairo_stroke(cr);
          }
        else
          {
            cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
            cairo_paint(cr);
          }
      }
   
    return FALSE;
  }
static gboolean tick_draw(GtkWidget *widget, GdkFrameClock *frame_clock, gpointer data)
  {
    rotate=rotate+G_PI/360.0;
    gtk_widget_queue_draw(da);
    return G_SOURCE_CONTINUE;
  }
static gpointer thread_draw_new_surface(gpointer data)
  {
    g_print("New Surface\n");

    if(surface!=NULL)
      {
        cairo_surface_destroy(surface); 
        surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width_height[0], width_height[1]);
      } 
    else
      {
        surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width_height[0], width_height[1]);
      }

    cairo_t *cr=cairo_create(surface); 
    gdouble w1=width_height[0]/10.0;
    gdouble h1=width_height[1]/10.0;
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //Layout axis for drawing.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 1.0*w1, 5.0*h1);
    cairo_line_to(cr, 9.0*w1, 5.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*w1, 1.0*h1);
    cairo_line_to(cr, 5.0*w1, 9.0*h1);
    cairo_stroke(cr); 

    //Slow the drawing thread down to test.
    g_usleep(500000); 

    cairo_destroy(cr);
    g_idle_add((GSourceFunc)thread_join, NULL);
    return NULL;
  }
static gboolean thread_join(gpointer data)
  {
    g_print("Thread Join\n");
    
    g_thread_join(thread);
    thread=NULL;
    thread_active=FALSE;

    gtk_widget_queue_draw(da);

    return FALSE;
  }
static void resize_window(GtkContainer *container, gpointer data)
  {
    g_print("Resize\n");
    resize=TRUE;
  }

