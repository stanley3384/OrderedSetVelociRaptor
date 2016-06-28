
/*

    Draw progress bars with a cairo linear pattern on a GTK drawing area. Draw custom horizontal
and vertical progress bars along with the standard GTK progress bar.

    gcc -Wall -Werror da_progress1.c -o da_progress1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>

//The progress of the bars.
static gint p_width=0;

static gboolean time_draw(GtkWidget *widgets[]);
static void start_drawing(GtkWidget *button, GtkWidget *widgets[]);
static gboolean draw_custom_progress_horizontal(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_custom_progress_vertical(GtkWidget *da, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Progress Bars");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    //Custom horizontal progress bar.    
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_size_request(da1, 300, 30);
    gtk_widget_set_hexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_custom_progress_horizontal), NULL);

    //Custom vertical progress bars.   
    GtkWidget *da2=gtk_drawing_area_new();
    gtk_widget_set_name(da2, "da2");
    gtk_widget_set_size_request(da2, 30, 300);
    gtk_widget_set_vexpand(da2, TRUE);
    gtk_widget_set_hexpand(da2, FALSE);
    g_signal_connect(da2, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da3=gtk_drawing_area_new();
    gtk_widget_set_name(da3, "da3");
    gtk_widget_set_size_request(da3, 30, 300);
    gtk_widget_set_vexpand(da3, TRUE);
    gtk_widget_set_hexpand(da3, FALSE);
    g_signal_connect(da3, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da4=gtk_drawing_area_new();
    gtk_widget_set_name(da4, "da4");
    gtk_widget_set_size_request(da4, 30, 300);
    gtk_widget_set_vexpand(da4, TRUE);
    gtk_widget_set_hexpand(da4, FALSE);
    g_signal_connect(da4, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da5=gtk_drawing_area_new();
    gtk_widget_set_name(da5, "da5");
    gtk_widget_set_size_request(da5, 30, 300);
    gtk_widget_set_vexpand(da5, TRUE);
    gtk_widget_set_hexpand(da5, FALSE);
    g_signal_connect(da5, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da6=gtk_drawing_area_new();
    gtk_widget_set_name(da6, "da6");
    gtk_widget_set_size_request(da6, 30, 300);
    gtk_widget_set_vexpand(da6, TRUE);
    gtk_widget_set_hexpand(da6, FALSE);
    g_signal_connect(da6, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    //Standard progress bar.
    GtkWidget *progress=gtk_progress_bar_new();
   
    GtkWidget *button=gtk_button_new_with_label("progress");
    gtk_widget_set_hexpand(button, TRUE);
    GtkWidget *widgets[]={button, progress, da1, da2, da3, da4, da5, da6};
    g_signal_connect(button, "clicked", G_CALLBACK(start_drawing), widgets);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), da2, 0, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da3, 2, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da4, 4, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da5, 6, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da6, 8, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), progress, 0, 11, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 12, 10, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean time_draw(GtkWidget *widgets[])
  {
    g_print("Timer %i\n", p_width);
    if(p_width<10)
      {
        p_width++;
        gtk_widget_queue_draw(widgets[2]);
        gtk_widget_queue_draw(widgets[3]);
        gtk_widget_queue_draw(widgets[4]);
        gtk_widget_queue_draw(widgets[5]);
        gtk_widget_queue_draw(widgets[6]);
        gtk_widget_queue_draw(widgets[7]);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets[1]), (gdouble)p_width/10.0);
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
static gboolean draw_custom_progress_horizontal(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);
    gint i=0;
    
    //The cyan blue gradient.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i/200.0), 0.0, 1.0, 1.0); 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i+10.0)/200.0, 0.0, 0.0, 1.0); 
      }
    cairo_set_source(cr, pattern1);
     
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    //The yellow red gradient.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i/200.0), 1.0, 1.0, 0.0); 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i+10.0)/200.0, 1.0, 0.0, 0.0); 
      }
    cairo_set_source(cr, pattern2);
     
    cairo_rectangle(cr, 0, 0, (p_width/10.0)*width, height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);
    return FALSE;
  }
static gboolean draw_custom_progress_vertical(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da)*10;
    gint height=gtk_widget_get_allocated_height(da);
    gint i=0;
    gint bars=0;
    
    //Test some offset counters.
    if(g_strcmp0(gtk_widget_get_name(da), "da2")==0&&p_width>1) bars=2;
    else if(g_strcmp0(gtk_widget_get_name(da), "da3")==0&&p_width>3) bars=4;
    else if(g_strcmp0(gtk_widget_get_name(da), "da4")==0&&p_width>5) bars=6;
    else if(g_strcmp0(gtk_widget_get_name(da), "da5")==0&&p_width>7) bars=8;
    else bars=p_width;

    cairo_save(cr);
    cairo_rotate(cr, G_PI/2.0);
    cairo_translate(cr, 0.0, -height);

    //The cyan blue gradient.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0, height, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i/200.0), 0.0, 1.0, 1.0); 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i+10.0)/200.0, 0.0, 0.0, 1.0); 
      }
    cairo_set_source(cr, pattern1);
     
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    //The yellow red gradient.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, height, 0.0);
    for(i=0;i<=200;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i/200.0), 1.0, 1.0, 0.0); 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i+10.0)/200.0, 1.0, 0.0, 0.0); 
      }
    cairo_set_source(cr, pattern2);
     
    cairo_rectangle(cr, (1.0-(bars/10.0))*height, 0, (bars/10.0)*height, height);
    cairo_fill(cr);

    cairo_restore(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, width/10, height);
    cairo_stroke(cr);

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);
    return FALSE;
  }

