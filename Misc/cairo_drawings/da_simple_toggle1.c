
/*

    Draw a simple headlight toggle switch. 

    gcc -Wall da_simple_toggle1.c -o da_simple_toggle1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>

static gboolean draw_circuit_breaker_horizontal(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean click_drawing(GtkWidget *widget, GdkEvent *event, gpointer data);

static gint state=0;

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);  

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 100);
    gtk_window_set_title(GTK_WINDOW(window), "Simple Toggle Switch");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    gtk_widget_set_events(da1, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(da1, "button_press_event", G_CALLBACK(click_drawing), NULL); 
    g_signal_connect(da1, "draw", G_CALLBACK(draw_circuit_breaker_horizontal), NULL);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean draw_circuit_breaker_horizontal(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);

    //Position toggle gradients.
    gint button_start=0;
    gint button_left=width/8;
    gint button_middle=width/2;
    gint button_right=7*width/8;

    //Paint background.    
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);
     
    //The far left button gradient and rectangle.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(button_start, 0.0, button_left, 0.0);
    if(state==1)
      {  
        cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.0, 0.0, 0.0, 1.0); 
        cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.6, 0.6, 0.6, 1.0); 
        cairo_set_source(cr, pattern1);    
        cairo_rectangle(cr, button_start, 0.0, button_left, height);
        cairo_fill(cr);
      }

    //The left gradient and rectangle.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(button_left, 0.0, button_middle, 0.0);
    if(state==1)
      {
        cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0);
 
        cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.4, 0.4, 0.4, 1.0); 
        cairo_set_source(cr, pattern2);     
        cairo_rectangle(cr, button_left, 0, button_middle-button_left, height);
        cairo_fill(cr);
      }

    //The button is off. Draw left part a solid color.
    if(state==0)
      {
        cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
        cairo_rectangle(cr, button_start, 0, button_middle, height);
        cairo_fill(cr);
      }

    //The lit part of the switch.
    if(state==0) cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
    else cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, button_middle, 0, width-button_middle, height);
    cairo_fill(cr);

    //The right gradient.
    cairo_pattern_t *pattern3=cairo_pattern_create_linear(button_right, 0.0, width, 0.0);
    if(state==0)
      {
        cairo_pattern_add_color_stop_rgba(pattern3, 0.0, 1.0, 0.0, 0.0, 1.0); 
        cairo_pattern_add_color_stop_rgba(pattern3, 1.0, 0.0, 0.0, 0.0, 1.0); 
        cairo_set_source(cr, pattern3);    
        cairo_rectangle(cr, button_right, 0, width-button_right, height);
        cairo_fill(cr);
      }

    //The outside rectangle.
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 8.0);
    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_stroke(cr);

    //The headlight drawing.
    if(state==0)
      {
        cairo_set_source_rgba(cr, 0.8, 0.8, 0.0, 1.0);
        cairo_scale(cr, 0.9, 1.0);
        cairo_translate(cr, 0.05*width, 0.0); 
      }
    else
      {
        cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
      }
    cairo_set_line_width(cr, 3.0);
    cairo_move_to(cr, 6.0*width/8.0, height/4.0);
    cairo_curve_to(cr, width/2.0, 3.0*height/8.0, width/2.0, 5.0*height/8.0, 6.0*width/8.0, 3.0*height/4.0);
    cairo_stroke(cr);
    //The headlight ellipse.
    cairo_save(cr);
    cairo_set_line_width(cr, 6.0);
    cairo_scale(cr, 0.2, 1.0);
    cairo_translate(cr, 4.0*6.0*width/8.0, 0.0);
    cairo_arc(cr, 6.0*width/8.0, height/2.0, height/4.0, 0.0, 2*G_PI);
    cairo_restore(cr);
    cairo_stroke(cr);
    //The 5 headlight rays.
    cairo_move_to(cr, 6.0*width/8.0, 5.0*height/16.0);
    cairo_line_to(cr, 7.0*width/8.0, 3.0*height/16.0);
    cairo_stroke(cr); 
    cairo_move_to(cr, 6.0*width/8.0, 6.5*height/16.0);
    cairo_line_to(cr, 7.0*width/8.0, 5.25*height/16.0);
    cairo_stroke(cr); 
    cairo_move_to(cr, 6.0*width/8.0, 8.0*height/16.0);
    cairo_line_to(cr, 7.0*width/8.0, 8.0*height/16.0);
    cairo_stroke(cr); 
    cairo_move_to(cr, 6.0*width/8.0, 9.25*height/16.0);
    cairo_line_to(cr, 7.0*width/8.0, 11.0*height/16.0);
    cairo_stroke(cr); 
    cairo_move_to(cr, 6.0*width/8.0, 11.0*height/16.0);
    cairo_line_to(cr, 7.0*width/8.0, 13.0*height/16.0);
    cairo_stroke(cr); 
    
    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);
    cairo_pattern_destroy(pattern3);

    return FALSE;
  }
static gboolean click_drawing(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(widget);

    if(state==0&&(event->button.x)>width/2) 
      {
        state=1;
        gtk_widget_queue_draw(widget);
      }
     if(state==1&&(event->button.x)<width/2) 
      {
        state=0;
        gtk_widget_queue_draw(widget);
      }
    return FALSE;
  }


