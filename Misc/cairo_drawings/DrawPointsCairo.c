
/*
  gcc -Wall  `pkg-config --cflags gtk+-3.0` DrawPointsCairo.c -o DrawPointsCairo `pkg-config --libs gtk+-3.0`

From discussion on GTK+ Forum.

*/

#include <gtk/gtk.h>
#include <gdk/gdk.h>

#define numbers 1000
static double test_numbers[numbers];
//If draw or the timer is called for a re-draw.
static gboolean timer_func=FALSE;

static void get_random_numbers(void)
  {
    int i=0;
    GRand *rand1=g_rand_new();
    for(i=0;i<numbers;i++) test_numbers[i]=200*g_rand_double(rand1);
    g_rand_free(rand1);
  }
static gboolean draw_points(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    int i=0;
    //Clear the surface.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_restore(cr);

    cairo_save(cr);
    cairo_translate(cr, 250, 250);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 3.0);
    cairo_move_to(cr, -200, 0);
    cairo_line_to(cr, 200, 0);
    cairo_move_to(cr, 0, -200);
    cairo_line_to(cr, 0, 200);
    cairo_stroke(cr);
    cairo_restore(cr);
    
    if(timer_func==FALSE) g_print("Draw Signal\n");
    if(timer_func==TRUE)
      {
        cairo_translate(cr, 250, 50);
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 5.0);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        for(i=0;i<numbers;i+=2)
           {
             cairo_move_to(cr, test_numbers[i], test_numbers[i+1]);
             cairo_line_to(cr, test_numbers[i], test_numbers[i+1]);
           }
        cairo_stroke(cr);
      }

    timer_func=FALSE;
    return TRUE;
  }

static gboolean invalidate_drawing(gpointer data)
  {
    GdkWindow *win;

    g_print("Timer Signal\n");
    timer_func=TRUE;
    get_random_numbers();

    win = gtk_widget_get_window(GTK_WIDGET(data));
    if (win) {
        GtkAllocation allocation;

        gtk_widget_get_allocation(GTK_WIDGET(data), &allocation);
        gdk_window_invalidate_rect(win, &allocation, FALSE);
    }
   
    return TRUE;
  }

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Points");
    g_signal_connect_swapped(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 500, 500);
   
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_points), NULL);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    //For testing. Get and draw the points every 3 seconds.
    //g_timeout_add(3000, invalidate_drawing, drawing_area);
    gdk_threads_add_timeout_full(G_PRIORITY_DEFAULT_IDLE,3000,invalidate_drawing,drawing_area,NULL);

    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
  }
