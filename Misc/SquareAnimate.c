
/*

Simple animation with GTK+ and Cairo. Click to start movement. Testing things out.

Compile with; gcc SquareAnimate.c `pkg-config --cflags --libs gtk+-3.0` -Wall -o square

C. Eric Cashon

*/

#include <gtk/gtk.h>

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
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

    GtkWidget *SquareDrawing = gtk_drawing_area_new();
    gtk_widget_set_size_request(SquareDrawing, 800, 400);
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
    move+=5;

    if(move<1000)
      {
        draw_square(widget, move);
        gtk_widget_queue_draw_area(widget, 0, 0, 800, 400);
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
    gtk_widget_queue_draw_area(widget, 0, 0, 800, 400);  
  }
static void draw_square(GtkWidget *widget, int move)
  {    
    GdkWindow *DefaultWindow=NULL;
    cairo_t *square=NULL;

    g_print("Draw Square %i\n", move);
    DefaultWindow=gtk_widget_get_window(GTK_WIDGET(widget));
    square = gdk_cairo_create(DefaultWindow);

    //Rotate around center of square so move there.
    cairo_translate(square, 30+move+50, 130+50);
    cairo_rotate(square, (2*G_PI)*move/500);
    //Move back to to the corner of the square and draw the square.
    cairo_translate(square, -50, -50);
    cairo_set_source_rgb(square, 0.5, 0, 0.5);
    cairo_rectangle(square, 0, 0, 100.0, 100.0);
    cairo_fill(square);

    cairo_destroy(square);

  }










