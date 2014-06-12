
/*

Simple animation with GTK+ and Cairo. Click to start movement. Testing things out.

Compile with; gcc DinoAnimate.c `pkg-config --cflags --libs gtk+-3.0` -Wall -o dino

C. Eric Cashon

*/

#include <gtk/gtk.h>

int move=3001;

static void close_window(GtkWidget *widget, gpointer data);
static void start_drawing(GtkWidget *widget, gpointer data);
static void click_drawing(GtkWidget *widget, gpointer data);
static void realize_drawing(GtkWidget *widget, gpointer data);
static void draw_veloci_raptor(GtkWidget *widget, int move);

int main (int argc, char *argv[])
 {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
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
static void start_drawing(GtkWidget *widget, gpointer data)
  {
    move+=40;

    if(move<3000)
      {
        draw_veloci_raptor(widget, move);
        gtk_widget_queue_draw_area(widget, 0, 0, 800, 400);
      }
   
    else
      {
        draw_veloci_raptor(widget, 0);        
      }
        
     g_usleep(10000);
      
  }
static void realize_drawing(GtkWidget *widget, gpointer data)
  {
    printf("Drawing Realized\n");
  }
static void click_drawing(GtkWidget *widget, gpointer data)
  {
    move=0;
    gtk_widget_queue_draw_area(widget, 0, 0, 800, 400);  
  }
static void draw_veloci_raptor(GtkWidget *widget, int move)
  {    
    GdkWindow *DefaultWindow=NULL;
    cairo_t *cr1;
    cairo_t *cr2;
    cairo_t *cr3;
    cairo_t *cr4;
    cairo_t *raptor;
    cairo_pattern_t *pattern;
    gint i=0;
    gint j=0;
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
    DefaultWindow=gtk_widget_get_window(GTK_WIDGET(widget));

    //Built in pieces. Should probably put together into one context.
    cr1 = gdk_cairo_create(DefaultWindow);
    cr2 = gdk_cairo_create(DefaultWindow);
    cr3 = gdk_cairo_create(DefaultWindow);
    cr4 = gdk_cairo_create(DefaultWindow);
    raptor = gdk_cairo_create(DefaultWindow);

    //Scaled from a 1024x576 screen. Original graphic.
    ScaleWidth=250/1024.0;
    ScaleHeight=250/576.0;

    //Draw raptor points and fill in green.
    cairo_scale(raptor, ScaleWidth, ScaleHeight);

    //Draw point to point.
    for(j = 0; j < 20; j++)
       {
         cairo_line_to(raptor, points[j][0]+move, points[j][1]);
       }

    //Draw curve at nose.
    cairo_move_to (raptor, 860+move, 310);
    cairo_curve_to(raptor, 900+move, 380, 900+move, 380, 860+move, 420);

    cairo_close_path(raptor);
    cairo_set_source_rgb(raptor, 0, 1, 0);
    cairo_fill(raptor);

     //Set up black ellipses.
    cairo_scale(cr1, ScaleWidth, ScaleHeight);
    cairo_set_source_rgba(cr1, 0, 0, 0, 1);
    cairo_set_line_width(cr1, 5.0);
    cairo_translate(cr1, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr1, move, 0);
    cairo_arc(cr1, 0, 0, 60, 0, 2 * G_PI);
    cairo_save(cr1);
  
    //Set up red ellipses.
    cairo_scale(cr2, ScaleWidth, ScaleHeight);
    cairo_set_source_rgba(cr2, 1, 0, 0, 1);
    cairo_set_line_width(cr2, 2.0);
    cairo_translate(cr2, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr2, move, 0);
    cairo_arc(cr2, 0, 0, 60, 0, 2 * G_PI);
    cairo_save(cr2);

    //Set up center ellipse of the eye.
    cairo_scale(cr3, ScaleWidth, ScaleHeight);
    cairo_set_source_rgb(cr3, 0, 0, 0);
    cairo_set_line_width(cr3, 3);
    cairo_translate(cr3, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr3, move, 0);
    cairo_save(cr3);
  
    //Set up round center of the eye.
    cairo_scale(cr4, ScaleWidth, ScaleHeight);
    cairo_set_source_rgb(cr4, 0, 0, 0);
    cairo_set_line_width(cr4, 3);
    cairo_translate(cr4, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr4, move, 0);
    cairo_save(cr4);

    //Rotated ellipses for the eye.
    for ( i = 0; i < 36; i++)
      {
         if(i==0||i%2==0)
           {
            cairo_rotate(cr1, i*G_PI/36);
            cairo_scale(cr1, 0.3, 1);
            cairo_arc(cr1, 0, 0, 60, 0, 2 * G_PI);
            cairo_restore(cr1);
            cairo_stroke(cr1);
            cairo_save(cr1);
           }
         else
           {
            cairo_rotate(cr2, i * G_PI/36);
            cairo_scale(cr2, 0.3, 1);
            cairo_arc(cr2, 0, 0, 60, 0, 2 * G_PI);
            cairo_restore(cr2);
            cairo_stroke(cr2);
            cairo_save(cr2);
           }
       }

    //Pattern for the center eye ellipse.
    pattern = cairo_pattern_create_linear(-120.0, 30.0, 120.0, 30.0);
    cairo_pattern_add_color_stop_rgb(pattern, 0.1, 0, 0, 0);
    cairo_pattern_add_color_stop_rgb(pattern, 0.5, 0, 0.5, 1);
    cairo_pattern_add_color_stop_rgb(pattern, 0.9, 0, 0, 0);

    //Draw center elipse of eye.
    cairo_rotate(cr3, 18 * G_PI/36);
    cairo_scale(cr3, 0.3, 1);
    cairo_arc(cr3, 0, 0, 60, 0, 2 * G_PI);
    cairo_close_path(cr3);
    //Blink eye.
    if(g_random_double()>0.97){cairo_fill(cr3);}
    else{cairo_set_source(cr3, pattern);}
    cairo_fill(cr3);

    //Draw center circle for the eye.
    cairo_rotate(cr4, 18*G_PI/36);
    cairo_scale(cr4, 0.3, 1);
    cairo_arc(cr4, 0, 0, 15, 0, 2 * G_PI);
    cairo_close_path(cr4);
    cairo_fill(cr4);
  
    cairo_destroy(cr1);
    cairo_destroy(cr2);
    cairo_destroy(cr3);
    cairo_destroy(cr4);
    cairo_destroy(raptor);
    cairo_pattern_destroy(pattern);

  }

