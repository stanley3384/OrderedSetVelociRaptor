
/*

    Another test program for the adjustable gauge widget.

    gcc -Wall -Werror adjustable_gauge.c adjustable_gauges.c -o gauges `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>
#include "adjustable_gauge.h"

static GRand *rand;

static gboolean time_draw(GtkWidget *widgets[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    rand=g_rand_new();

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    gtk_window_set_title(GTK_WINDOW(window), "Gauges");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
   
    GtkWidget *gauge1=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge1, TRUE);
    gtk_widget_set_vexpand(gauge1, TRUE);

    GtkWidget *gauge2=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge2, TRUE);
    gtk_widget_set_vexpand(gauge2, TRUE);
  
    GtkWidget *gauge3=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge3, TRUE);
    gtk_widget_set_vexpand(gauge3, TRUE);
   
    GtkWidget *gauge4=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge4, TRUE);
    gtk_widget_set_vexpand(gauge4, TRUE);
    
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), gauge1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gauge2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gauge3, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gauge4, 1, 1, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);
  
    GtkWidget *widgets[]={gauge1, gauge2, gauge3, gauge4};    
    g_timeout_add(1000, (GSourceFunc)time_draw, widgets);

    gtk_widget_show_all(window);                  
    gtk_main();

    g_rand_free(rand);

    return 0;
  }
static gboolean time_draw(GtkWidget *widgets[])
  {
    gint i=0;
    gdouble needle=100*g_rand_double(rand);
    for(i=0;i<4;i++)
      {
        adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle);
        needle=100*g_rand_double(rand);
      }
    
    return TRUE;
  }

