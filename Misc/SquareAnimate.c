
/*

Simple animation with GTK+ and Cairo. Click to start movement. Testing things out.
Roll a square, build a cycloid "bridge" wave, sine wave and cosine wave. Have the
square disappear and reappear. Magic Square.

Compile with; gcc SquareAnimate.c `pkg-config --cflags --libs gtk+-3.0` -lm -Wall -o square

C. Eric Cashon

*/

#include <gtk/gtk.h>
#include <math.h>

int move=1001;

static void close_window(GtkWidget *widget, gpointer data);
static void start_drawing(GtkWidget *widget, gpointer data);
static void click_drawing(GtkWidget *widget, gpointer data);
static void realize_drawing(GtkWidget *widget, gpointer data);
static void draw_square(GtkWidget *widget, int move);

int main (int argc, char *argv[])
 {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 950, 350);
    gtk_window_set_title(GTK_WINDOW(window), "Magic Square");
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

    GtkWidget *SquareDrawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(SquareDrawing, 800, 350);
    gtk_widget_set_events(SquareDrawing, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(G_OBJECT(SquareDrawing), "button_press_event", G_CALLBACK(click_drawing), NULL); 
    g_signal_connect(G_OBJECT(SquareDrawing), "draw", G_CALLBACK(start_drawing), NULL); 
    g_signal_connect(G_OBJECT(SquareDrawing), "realize", G_CALLBACK(realize_drawing), NULL); 

    gtk_container_add(GTK_CONTAINER(window), SquareDrawing); 

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
    int i=0;
    move+=5;

    //Move drawing area to reduce re-draw area.
    if(move<640){i=move-5;}
    else if(move>=640&&move<=1000){i=640;}
    else{i=0;}

    if(move<=1000)
      {
        if(move!=1000)
          {
            draw_square(widget, move);
            gtk_widget_queue_draw_area(widget, i, 30, 170, 250);
          }
        else
          {
            draw_square(widget, 0);
            gtk_widget_queue_draw_area(widget, 0, 0, 800, 350);
          }
      }  
    else
      {
        draw_square(widget, 0);        
      }
        
     g_usleep(20000);
      
  }
static void realize_drawing(GtkWidget *widget, gpointer data)
  {
    printf("Drawing Realized\n");
  }
static void click_drawing(GtkWidget *widget, gpointer data)
  {
    move=0;
    gtk_widget_queue_draw_area(widget, 0, 0, 800, 350);  
  }
static void draw_square(GtkWidget *widget, int move)
  {  
    int i=0;
    double x1=0;
    double y1=0;
    GdkWindow *DefaultWindow=NULL;
    cairo_t *square=NULL;
    cairo_t *circle=NULL;
    cairo_t *line1=NULL;
    cairo_t *line2=NULL;
    cairo_t *line3=NULL;
    cairo_t *line4=NULL;
    cairo_t *line5=NULL;

    g_print("Draw Square %i\n", move);
    DefaultWindow=gtk_widget_get_window(GTK_WIDGET(widget));
    square = gdk_cairo_create(DefaultWindow);
    circle = gdk_cairo_create(DefaultWindow);
    line1 = gdk_cairo_create(DefaultWindow);
    line2 = gdk_cairo_create(DefaultWindow);
    line3 = gdk_cairo_create(DefaultWindow);
    line4 = gdk_cairo_create(DefaultWindow);
    line5 = gdk_cairo_create(DefaultWindow);        

    //Rotate around center of square so move there.
    cairo_translate(square, 30+move+50, 130+50);
    //Rotate the square. Tip the square on end with pi/4 to start. 
    cairo_rotate(square, (((move/10)*G_PI/25.0)+G_PI/4.0));
    //Move back to to the corner of the square and draw the square.
    cairo_translate(square, -50, -50);
    cairo_set_source_rgb(square, 0.5, 0, 0.5);
    cairo_rectangle(square, 0, 0, 100.0, 100.0);
    cairo_fill(square);
    //Add a circle around the square.
    cairo_translate(circle, 30+move+50, 130+50);
    cairo_set_source_rgb(circle, 1, 1, 0);
    cairo_set_line_width(circle, 4.0);
    cairo_arc(circle, 0, 0, sqrt(5000.0), 0, 2*M_PI);
    cairo_close_path(circle);
    cairo_stroke_preserve(circle);
    //Put a line in for the square to roll on.
    cairo_set_source_rgb(line1, 0, 0, 0);
    cairo_move_to(line1, 0, 130+50+sqrt(5000.0));
    cairo_line_to(line1, 1000, 130+50+sqrt(5000.0));
    cairo_set_line_width(line1, 3.0);
    cairo_stroke(line1);
    //Make a line in the square.
    cairo_set_source_rgb(line2, 0, 1.0, 0);
    cairo_set_line_width(line2, 2.0);
    cairo_move_to(line2,30+50+move, 130+50);
    cairo_arc(line2, 30+50+move, 130+50, sqrt(5000.0), (((move/10)*G_PI/25.0)+G_PI/2.0), (((move/10)*G_PI/25.0)+G_PI/2.0));
    cairo_line_to(line2, 30+50+move, 130+50);
    cairo_stroke_preserve(line2);
    //Set line colors and width.
    cairo_set_line_width(line3, 2.0);
    cairo_set_source_rgb(line3, 0, 1.0, 0);
    cairo_set_line_width(line4, 2.0);
    cairo_set_source_rgb(line4, 0, 0, 1.0);
    cairo_set_line_width(line5, 0.5);
    cairo_set_source_rgb(line5, 1, 0, 0);
    for(i=0;i<move;i+=10)
       {
         //The bridge.
         cairo_move_to(line3,30+50+i, 130+50);
         cairo_arc(line3, 30+50+i, 130+50, sqrt(5000.0), (((i/10)*G_PI/25.0)+G_PI/2.0), (((i/10)*G_PI/25.0)+G_PI/2.0));
         cairo_line_to(line3, 30+50+i, 130+50);
         cairo_stroke_preserve(line3);
         //The sine wave.
         y1=(sqrt(5000.0))*sin(((i/10.0)*G_PI/25.0)+G_PI/2.0);
         cairo_move_to(line4, 30+50+i, 130+50);
         cairo_line_to(line4, 30+50+i, 130+50+(int)y1);
         cairo_stroke_preserve(line4);
         //The cosine wave. Rotate slightly off the x-axis.
         x1=(sqrt(5000.0))*cos(((i/10.0)*G_PI/25.0)+G_PI/2.0);
         if(x1>=0)
           {
             cairo_arc(line5, 30+50+i, 130+50, (int)floor(x1), G_PI/8, G_PI/8);
           }
         else
           {
             cairo_arc(line5, 30+50+i, 130+50, (int)floor(fabs(x1)), G_PI+G_PI/8, G_PI+G_PI/8);
           }
         cairo_line_to(line5, 30+50+i, 130+50);
         cairo_stroke_preserve(line5);        
       }

    //clean-up.
    cairo_destroy(square);
    cairo_destroy(circle);
    cairo_destroy(line1);
    cairo_destroy(line2);
    cairo_destroy(line3);
    cairo_destroy(line4);
    cairo_destroy(line5);

  }






