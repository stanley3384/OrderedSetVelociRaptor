
/*

    Draw a circuit breaker switch. Just test turning the switch on, off, starting and breaking
with random numbers and a timer. 

    There is also a GTK switch(switch1.py) in the python folder for a comparison.

    gcc -Wall da_circuit_breaker1.c -o da_circuit_breaker1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>

static gboolean draw_circuit_breaker_horizontal(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_circuit_breaker_vertical(GtkWidget *da, cairo_t *cr, gpointer data);
static void click_drawing(GtkWidget *widget, gpointer data);
static gboolean start_process(gpointer user_data);
static gboolean service_killed(gpointer user_data);

//Test drawing the switch horizontal=0 and vertical=1.
static gint draw_direction=0;
//States ON=0, STARTING=1 OFF=2 and BREAK=3. Start in the OFF position.
static gint state=2;
static GRand *rand;
static guint timeout_id=0;

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv); 

    rand=g_rand_new();  

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    if(draw_direction==0) gtk_window_set_default_size(GTK_WINDOW(window), 400, 50);
    else gtk_window_set_default_size(GTK_WINDOW(window), 50, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Circuit Breaker Switch");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
        
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    gtk_widget_set_events(da1, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(da1, "button_press_event", G_CALLBACK(click_drawing), NULL); 
    if(draw_direction==0) g_signal_connect(da1, "draw", G_CALLBACK(draw_circuit_breaker_horizontal), NULL);
    else g_signal_connect(da1, "draw", G_CALLBACK(draw_circuit_breaker_vertical), NULL);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    gtk_widget_show_all(window);                  
    gtk_main();

    g_rand_free(rand);

    return 0;
  }
static gboolean draw_circuit_breaker_horizontal(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);

    //Position slider button.
    gint button_start=width/2.0;
    gint button_mid1=button_start+button_start/3.0;
    gint button_mid2=button_start+2.0*button_start/3.0;
    gint button_end=width;
    if(state==2)
      {
        button_mid1=button_mid1-button_start;
        button_mid2=button_mid2-button_start;
        button_end=button_end-button_start;
        button_start=0;
      }
    if(state==3)
      {
        button_mid1=button_mid1-button_start/2.0;
        button_mid2=button_mid2-button_start/2.0;
        button_end=button_end-button_start/2.0;
        button_start=width/4.0;
      }

    //Paint background.
    if(state==0)
      {
        cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
        cairo_paint(cr);
      }
    if(state==1)
      {
        cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
        cairo_paint(cr);
      }
    if(state==2)
      {
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_paint(cr);
      }
    if(state==3)
      {
        cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
        cairo_paint(cr);
      }

    //Adjust the font size. 
    gint font_size=14;
    if(state==1||state==3)
      {
        if(height>width) font_size=(gint)(font_size*width/100.0);
        else font_size=(gint)(font_size*height/100.0);
      }
    else
      {
        if(height>width) font_size=(gint)(font_size*width/50.0);
        else font_size=(gint)(font_size*height/50.0);
      }
  
    //Draw the text.
    cairo_text_extents_t extents1;
    cairo_text_extents_t extents2;
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, font_size);
    if(state==0)
      {
        cairo_text_extents(cr, "ON", &extents1);
        cairo_move_to(cr, width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
        cairo_show_text(cr, "ON");
      }
    else if(state==1)
      {
        cairo_text_extents(cr, "STARTING", &extents1);
        cairo_move_to(cr, width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
        cairo_show_text(cr, "STARTING");
      }
    else if(state==2)
      {
        cairo_text_extents(cr, "OFF", &extents1);
        cairo_move_to(cr, 3.0*width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);   
        cairo_show_text(cr, "OFF");
      }
    else
      {
        cairo_text_extents(cr, "BREAK", &extents1);
        cairo_move_to(cr, width/8.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
        cairo_show_text(cr, "BREAK");
        cairo_text_extents(cr, "CONTACT", &extents2);
        cairo_move_to(cr, 7.0*width/8.0 - extents2.width/2.0, height/2.0 + extents2.height/2.0);
        cairo_show_text(cr, "CONTACT");  
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

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);

    return FALSE;
  }
static gboolean draw_circuit_breaker_vertical(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);

    //Position slider button.
    gint button_start=height/2.0;
    gint button_mid1=button_start+button_start/3.0;
    gint button_mid2=button_start+2.0*button_start/3.0;
    gint button_end=height;
    if(state==2)
      {
        button_mid1=button_mid1-button_start;
        button_mid2=button_mid2-button_start;
        button_end=button_end-button_start;
        button_start=0;
      }
    if(state==3)
      {
        button_mid1=button_mid1-button_start/2.0;
        button_mid2=button_mid2-button_start/2.0;
        button_end=button_end-button_start/2.0;
        button_start=height/4.0;
      }

    //Paint background.
    if(state==0)
      {
        cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
        cairo_paint(cr);
      }
    if(state==1)
      {
        cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
        cairo_paint(cr);
      }
    if(state==2)
      {
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
        cairo_paint(cr);
      }
    if(state==3)
      {
        cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
        cairo_paint(cr);
      }

    //Adjust the font size. 
    gint font_size=14;
    if(state==1||state==3)
      {
        if(height<width) font_size=(gint)(font_size*height/100.0);
        else font_size=(gint)(font_size*width/100.0);
      }
    else
      {
        if(height<width) font_size=(gint)(font_size*height/50.0);
        else font_size=(gint)(font_size*width/50.0);
      }
  
    //Draw the text.
    cairo_text_extents_t extents1;
    cairo_text_extents_t extents2;
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, font_size);
    if(state==0)
      {
        cairo_text_extents(cr, "ON", &extents1);
        cairo_move_to(cr, width/2.0 - extents1.width/2.0, height/4.0 + extents1.height/2.0);  
        cairo_show_text(cr, "ON");
      }
    else if(state==1)
      {
        cairo_text_extents(cr, "STARTING", &extents1);
        cairo_move_to(cr, width/2.0 - extents1.width/2.0, height/4.0 + extents1.height/2.0);  
        cairo_show_text(cr, "STARTING");
      }
    else if(state==2)
      {
        cairo_text_extents(cr, "OFF", &extents1);
        cairo_move_to(cr, width/2.0 - extents1.width/2.0, 3.0*height/4.0 + extents1.height/2.0);   
        cairo_show_text(cr, "OFF");
      }
    else
      {
        cairo_text_extents(cr, "BREAK", &extents1);
        cairo_move_to(cr, width/2.0 - extents1.width/2.0, height/8.0 + extents1.height/2.0);  
        cairo_show_text(cr, "BREAK");
        cairo_text_extents(cr, "CONTACT", &extents2);
        cairo_move_to(cr, width/2.0 - extents2.width/2.0, 7.0*height/8.0 + extents2.height/2.0);
        cairo_show_text(cr, "CONTACT");  
      }  
    
    //The left button gradient and rectangle.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, button_start, 0.0, button_mid1);  
    cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.6, 0.6, 0.6, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 0.0, 0.0, 1.0); 
    cairo_set_source(cr, pattern1);    
    cairo_rectangle(cr, 0, button_start, width, button_mid1);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);

    //The middle of the button.
    cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 1.0);
    cairo_rectangle(cr, 0, button_mid1, width, button_mid2-button_mid1);
    cairo_fill(cr);

    //The right button gradient and rectangle.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, button_mid2, 0.0, button_end);
    cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.6, 0.6, 0.6, 1.0); 
    cairo_set_source(cr, pattern2);     
    cairo_rectangle(cr, 0, button_mid2, width, button_end-button_mid2);
    cairo_fill(cr);

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);

    return FALSE;
  }
static void click_drawing(GtkWidget *widget, gpointer data)
  {
    g_print("Drawing Clicked State %i\n", state);
    //ON to OFF.
    if(state==0)
      {
        if(timeout_id>0)
          {
            //Remove the service checking timer if it is active.
            g_source_remove(timeout_id);
            timeout_id=0;
          }
        state=2;
        gtk_widget_queue_draw(widget);
      }
    //Can't click on state 1. The service is starting and the widget is insensitive.
    else if(state==1) {}
    //Off to Starting.
    else if(state==2)
      {
        state=1;
        gtk_widget_queue_draw(widget);
        gtk_widget_set_sensitive(GTK_WIDGET(widget), FALSE);
        g_timeout_add(1000, (GSourceFunc)start_process, widget); 
      }
    //Circuit Break to OFF.
    else
      {
        state=2;
        gtk_widget_queue_draw(widget); 
      }

  }
static gboolean start_process(gpointer user_data)
  {
    gdouble num=g_rand_double(rand);
    //Start or stop the process but delay it a little.
    g_print("Starting Service...\n");
    if(num<0.1||num>.90)
      {
        //Start the service more than failing to test.
        num=g_rand_double(rand);
        if(num<0.3)
          {
            gtk_widget_set_sensitive(GTK_WIDGET(user_data), TRUE);
            //Failed to start service. Switch to OFF
            state=2;
            gtk_widget_queue_draw(GTK_WIDGET(user_data)); 
            g_print("Service didn't Start! Number %f Return FALSE\n", num);
          }
        else
          {
            state=0;
            gtk_widget_queue_draw(GTK_WIDGET(user_data));
            gtk_widget_set_sensitive(GTK_WIDGET(user_data), TRUE);   
            g_print("Service Started! %f Return TRUE\n", num);
            //Start new timer to check if service continues to run.
            timeout_id=g_timeout_add(1000, (GSourceFunc)service_killed, GTK_WIDGET(user_data)); 
          }
        return FALSE;
     } 
    return TRUE;   
  }
static gboolean service_killed(gpointer user_data)
  {
    gdouble num=g_rand_double(rand);
    //Randomly kill the service.
    if(num<0.15)
      {
        g_print("Service Randomly Killed After Starting\n");
        state=3;
        gtk_widget_queue_draw(GTK_WIDGET(user_data));
        gtk_widget_set_sensitive(GTK_WIDGET(user_data), TRUE); 
        return FALSE;
      }
    else
      {
        g_print("Service Running\n");
        return TRUE;
      }
  }

