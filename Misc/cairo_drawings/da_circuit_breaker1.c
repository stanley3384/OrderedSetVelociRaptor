
/*

    Draw a circuit breaker switch.

    gcc -Wall da_circuit_breaker1.c -o da_circuit_breaker1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>

static gboolean draw_custom_progress_horizontal(GtkWidget *da, cairo_t *cr, gpointer data);
static void combo_changed(GtkComboBox *combo, gpointer data);

gint state=0;

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 100);
    gtk_window_set_title(GTK_WINDOW(window), "Circuit Breaker Switch");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_custom_progress_horizontal), NULL);
    
    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "ON");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "OFF");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "BREAK");
    gtk_widget_set_hexpand(combo, TRUE);  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), da1);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 12, 10, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean draw_custom_progress_horizontal(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);

    gint button_start=width/2.0;
    gint button_mid1=button_start+button_start/3.0;
    gint button_mid2=button_start+2.0*button_start/3.0;
    gint button_end=width;

    if(state==1)
      {
        button_mid1=button_mid1-button_start;
        button_mid2=button_mid2-button_start;
        button_end=button_end-button_start;
        button_start=0;
      }
    if(state==2)
      {
        button_mid1=button_mid1-button_start/2.0;
        button_mid2=button_mid2-button_start/2.0;
        button_end=button_end-button_start/2.0;
        button_start=width/4.0;
      }

    //Paint background.
    if(state==0)
      {
        cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
        cairo_paint(cr);
      }
    if(state==1)
      {
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_paint(cr);
      }
    if(state==2)
      {
        cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
        cairo_paint(cr);
      }
    
    //The left button gradient and rectangle.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(button_start, 0.0, button_mid1, 0.0);  
    cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.6, 0.6, 0.6, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 0.0, 0.0, 1.0); 
    cairo_set_source(cr, pattern1);    
    cairo_rectangle(cr, button_start, 0, button_mid1, height);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    //The middle of the button.
    cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 1.0);
    cairo_rectangle(cr, button_mid1, 0, button_mid2-button_mid1, height);
    cairo_fill(cr);

    //The right button gradient and rectangle.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(button_mid2, 0.0, button_end, 0.0);
    cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.6, 0.6, 0.6, 1.0); 
    cairo_set_source(cr, pattern2);     
    cairo_rectangle(cr, button_mid2, 0, button_end-button_mid2, height);
    cairo_fill(cr);

    cairo_text_extents_t extents1;
    cairo_text_extents_t extents2;
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    if(state==0)
      {
        cairo_text_extents(cr, "ON", &extents1);
        cairo_move_to(cr, width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
        cairo_show_text(cr, "ON");
      }
    if(state==1)
      {
        cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
        cairo_text_extents(cr, "OFF", &extents1);
        cairo_move_to(cr, 3.0*width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
        cairo_show_text(cr, "OFF");
      }
    if(state==2)
      {
        cairo_text_extents(cr, "CIRCUIT", &extents1);
        cairo_move_to(cr, width/8.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
        cairo_show_text(cr, "CIRCUIT");
        cairo_text_extents(cr, "BREAK", &extents2);
        cairo_move_to(cr, 7.0*width/8.0 - extents1.width/2.0, height/2.0 + extents2.height/2.0);
        cairo_show_text(cr, "BREAK");  
      }    

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);

    return FALSE;
  }
static void combo_changed(GtkComboBox *combo, gpointer data)
  {
    state=gtk_combo_box_get_active(combo);
    gtk_widget_queue_draw(GTK_WIDGET(data));    
  }

