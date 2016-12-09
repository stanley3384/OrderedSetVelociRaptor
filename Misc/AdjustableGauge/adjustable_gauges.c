
/*

    Another test program for the adjustable gauge widget. Test four gauges with some numbers.

    gcc -Wall -Werror adjustable_gauge.c adjustable_gauges.c -o gauges `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>
#include "adjustable_gauge.h"

//Save the needle position between calls.
static gdouble needle1=0.0;
static gdouble needle2=0.0;
static gdouble needle3=20.0;
static gdouble needle4=0.0;
//The top of each scale.
static const gdouble top1=100.0;
static const gdouble top2=200.0;
static const gdouble top3=200.0;
static const gdouble top4=500.0;

static gboolean time_draw(GtkWidget *widgets[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    gtk_window_set_title(GTK_WINDOW(window), "Gauges");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
   
    //Default gauge.
    GtkWidget *gauge1=adjustable_gauge_new();
    adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(gauge1), SPEEDOMETER_GAUGE);
    gtk_widget_set_name(gauge1, "g1");
    gtk_widget_set_hexpand(gauge1, TRUE);
    gtk_widget_set_vexpand(gauge1, TRUE); 

    GtkWidget *gauge2=adjustable_gauge_new();
    adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(gauge2), SPEEDOMETER_GAUGE);
    gtk_widget_set_hexpand(gauge2, TRUE);
    gtk_widget_set_vexpand(gauge2, TRUE);
    //Set scale values before cutoff values.
    adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(gauge2), 0.0);
    adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(gauge2), top2);
    adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(gauge2), 150.0);
    adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(gauge2), 180.0); 
  
    GtkWidget *gauge3=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge3, TRUE);
    gtk_widget_set_vexpand(gauge3, TRUE);
    adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(gauge3), 20.0);
    adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(gauge3), top3);
    adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(gauge3), 120.0);
    adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(gauge3), 170.0); 
   
    GtkWidget *gauge4=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge4, TRUE);
    gtk_widget_set_vexpand(gauge4, TRUE);
    adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(gauge4), 0.0);
    adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(gauge4), top4);
    adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(gauge4), 450.0);
    adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(gauge4), 480.0); 
    
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), gauge1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gauge2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gauge3, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), gauge4, 1, 1, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);
  
    GtkWidget *widgets[]={gauge1, gauge2, gauge3, gauge4};    
    g_timeout_add(100, (GSourceFunc)time_draw, widgets);

    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean time_draw(GtkWidget *widgets[])
  {
    gint i=0;

    needle1++;
    needle2++;
    needle3++;
    needle4++;
    for(i=0;i<4;i++)
      {
        switch(i)
          {
            case 0:
              if(needle1<top1)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle1);
                }
              else
                {
                  g_print("Reset Gauge1\n");
                  needle1=0.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle1);
                }
              break;
            case 1:
              if(needle2<top2)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle2);
                }
              else
                {
                  g_print("Reset Gauge2\n");
                  needle2=0.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle2);
                }
              break;
            case 2:
              if(needle3<top3)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle3);
                }
              else
                {
                  g_print("Reset Gauge3\n");
                  needle3=20.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle3);
                }
              break;
            case 3:
              if(needle4<top4)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle4);
                }
              else
                {
                  g_print("Reset Gauge4\n");
                  needle4=0.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[i]), needle4);
                }
          }        
      }
    
    return TRUE;
  }

