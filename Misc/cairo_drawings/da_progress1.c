
/*

    Draw a progress bar with a cairo linear pattern on a GTK drawing area.

    gcc -Wall -Werror da_progress1.c -o da_progress1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>

static gint p_width=0;

static gboolean time_draw(GtkWidget *widgets[])
  {
    g_print("Timer %i\n", p_width);
    if(p_width<10)
      {
        p_width++;
        gtk_widget_queue_draw(widgets[1]);
        return TRUE;
      }
    else  
      {
        gtk_widget_set_sensitive(widgets[0], TRUE);
        return FALSE;
      }   
  }
static void start_drawing(GtkWidget *button, GtkWidget *widgets[])
  {
    g_print("Click\n");
    p_width=0;
    gtk_widget_set_sensitive(button, FALSE);
    g_timeout_add(300, (GSourceFunc)time_draw, widgets);
  }
static gboolean draw_progress(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);
    gint i=0;
    
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0,  width, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i/200.0), 0.0, 1.0, 1.0); 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i+10.0)/200.0, 0.0, 0.0, 1.0); 
      }
    cairo_set_source(cr, pattern1);
     
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0,  width, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i/200.0), 1.0, 1.0, 0.0); 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i+10.0)/200.0, 1.0, 0.0, 0.0); 
      }
    cairo_set_source(cr, pattern2);
     
    cairo_rectangle(cr, 0, 0, (p_width/10.0)*width, height);
    cairo_fill(cr);

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);
    return FALSE;
  }
int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 50);
    gtk_window_set_title(GTK_WINDOW(window), "Progress Bar");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 300, 20);
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(draw_progress), NULL);
   
    GtkWidget *button=gtk_button_new_with_label("progress");
    gtk_widget_set_hexpand(button, TRUE);
    GtkWidget *widgets[]={button, da};
    g_signal_connect(button, "clicked", G_CALLBACK(start_drawing), widgets);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
