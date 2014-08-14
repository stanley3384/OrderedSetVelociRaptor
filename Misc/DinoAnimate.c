
/*

Simple animation with GTK+ and Cairo. Click to start movement. Testing things out.

Compile with; gcc -Wall `pkg-config --cflags gtk+-3.0`DinoAnimate.c -o dino `pkg-config --libs gtk+-3.0` 

C. Eric Cashon

*/

#include <gtk/gtk.h>

int move=4001;

static void close_window(GtkWidget *widget, gpointer data);
static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data);
static void click_drawing(GtkWidget *widget, gpointer data);
static void realize_drawing(GtkWidget *widget, gpointer data);
static gboolean draw_veloci_raptor(GtkWidget *widget, cairo_t *cr, int move);

int main (int argc, char *argv[])
 {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 900, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

    GtkWidget *RaptorDrawing = gtk_drawing_area_new();
    gtk_widget_set_size_request (RaptorDrawing, 800, 400);
    gtk_widget_set_events(RaptorDrawing, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(RaptorDrawing), "button_press_event", G_CALLBACK(click_drawing), NULL); 
    g_signal_connect(G_OBJECT(RaptorDrawing), "draw", G_CALLBACK(start_drawing), NULL); 
    g_signal_connect(G_OBJECT(RaptorDrawing), "realize", G_CALLBACK(realize_drawing), NULL); 

    gtk_container_add(GTK_CONTAINER(window), RaptorDrawing); 

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
  }
static void close_window(GtkWidget *widget, gpointer data)
  {
    gtk_main_quit ();
  }
static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    move+=20;
    GtkAllocation allocation;
    GdkWindow *win=gtk_widget_get_window(widget);
    gtk_widget_get_allocation(GTK_WIDGET(widget), &allocation);

    if(move<4000)
      {
        draw_veloci_raptor(widget, cr, move);
        gdk_window_invalidate_rect(win, &allocation, FALSE);
      }   
    else
      {
        draw_veloci_raptor(widget, cr, 0);        
      }
     
     return TRUE;
      
  }
static void realize_drawing(GtkWidget *widget, gpointer data)
  {
    printf("Drawing Realized\n");
  }
static void click_drawing(GtkWidget *widget, gpointer data)
  {
    move=0;
    gtk_widget_queue_draw_area(widget, 0, 0, 900, 400);  
  }
static gboolean draw_veloci_raptor(GtkWidget *widget, cairo_t *cr, int move)
  {        
    cairo_pattern_t *pattern;
    gint i=0;
    int height=250;
    int width=250;
    double ScaleWidth=0;
    double ScaleHeight=0;
    int points[21][2] = { 
      { 40, 85 }, 
      { 105, 75 }, 
      { 140, 10 }, 
      { 165, 75 }, 
      { 490, 100 },
      { 790, 225 },
      { 860, 310 }, 
      //{ 900, 380 }, curve nose
      { 860, 420 },
      { 820, 380 },
      { 780, 420 },
      { 740, 380 },
      { 700, 420 },
      { 660, 380 },
      { 650, 385 },
      { 810, 520 }, 
      { 440, 540 },
      { 340, 840 },
      { 240, 840 },
      { 140, 200 },
      { 90, 125 },
      { 40, 85 } 
  };
    g_print("Draw Dino %i\n", move);
    
    //Scaled from a 1024x576 screen. Original graphic.
    ScaleWidth=250/1024.0;
    ScaleHeight=250/576.0;

    //Clear the surface.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_restore(cr);
    
    cairo_save(cr);
    //Draw raptor points and fill in green.
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    //Draw point to point.
    for(i=0; i<20; i++)
       {
         cairo_line_to(cr, points[i][0]+move, points[i][1]);
       }
    //Draw curve at nose.
    cairo_move_to(cr, 860+move, 310);
    cairo_curve_to(cr, 900+move, 380, 900+move, 380, 860+move, 420);
    cairo_close_path(cr);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_fill(cr);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Set up rotated black ellipses.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgba(cr, 0, 0, 0, 1);
    cairo_set_line_width(cr, 7.0);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    for( i=0; i<36; i+=2)
      {
        cairo_save(cr);
        cairo_rotate(cr, i*G_PI/36);
        cairo_scale(cr, 0.3, 1);
        cairo_arc(cr, 0, 0, 60, 0, 2 * G_PI);
        cairo_stroke(cr);
        cairo_restore(cr);
      }
    cairo_restore(cr);

    //Set up rotated purple ellipses.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgba(cr, 1, 0, 1.0, 1);
    cairo_set_line_width(cr, 3.0);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    for(i=1; i<36; i+=2)
      {
        cairo_save(cr);
        cairo_rotate(cr, i*G_PI/36);
        cairo_scale(cr, 0.3, 1);
        cairo_arc(cr, 0, 0, 60, 0, 2 * G_PI);
        cairo_stroke(cr);
        cairo_restore(cr);
      }
    cairo_restore(cr);

    //Pattern for the center eye ellipse.
    pattern = cairo_pattern_create_linear(-120.0, 30.0, 120.0, 30.0);
    cairo_pattern_add_color_stop_rgb(pattern, 0.1, 0, 0, 0);
    cairo_pattern_add_color_stop_rgb(pattern, 0.5, 0, 0.5, 1);
    cairo_pattern_add_color_stop_rgb(pattern, 0.9, 0, 0, 0);

    //Draw center elipse of eye.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    cairo_rotate(cr, 18 * G_PI/36);
    cairo_scale(cr, 0.3, 1);
    cairo_arc(cr, 0, 0, 60, 0, 2 * G_PI);
    cairo_close_path(cr);
    //Blink eye.
    if(g_random_double()>0.97&&move!=0){cairo_fill(cr);}
    else{cairo_set_source(cr, pattern);}
    cairo_fill(cr);
    cairo_restore(cr);

    //Draw center circle for the eye.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    cairo_rotate(cr, 18*G_PI/36);
    cairo_scale(cr, 0.3, 1);
    cairo_arc(cr, 0, 0, 15, 0, 2 * G_PI);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);
  
    cairo_pattern_destroy(pattern);

    return TRUE;

  }

