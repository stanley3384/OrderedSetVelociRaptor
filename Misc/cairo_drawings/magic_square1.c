
/*

    Simple animation with GTK+ and Cairo. Click to start movement. Testing things out.
Roll a square, build a cycloid "bridge" wave, sine wave and cosine wave. Have the
square disappear and reappear. Magic Square. Change around as needed.

    gcc -Wall `pkg-config --cflags gtk+-3.0` magic_square1.c -o magic_square1 `pkg-config --libs gtk+-3.0` -lm

    C. Eric Cashon

*/

#include <gtk/gtk.h>
#include <math.h>

#define move_ulimit 1000
int move=0;
gboolean moving=FALSE;
gint timer_id=0;
gint block_id=0;
gboolean click1=TRUE;

static void close_window(GtkWidget *widget, gpointer data);
static gboolean start_drawing(gpointer widget);
static void click_drawing(GtkWidget *widget, gpointer data);
static void realize_drawing(GtkWidget *widget, gpointer data);
static gboolean draw_square(GtkWidget *widget, cairo_t *cr, gpointer data);

int main (int argc, char *argv[])
 {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 950, 350);
    gtk_window_set_title(GTK_WINDOW(window), "Magic Square");
    g_signal_connect(window, "destroy", G_CALLBACK(close_window), NULL);

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 950, 350);
    gtk_widget_set_events(drawing_area, GDK_BUTTON_PRESS_MASK);
    g_signal_connect(drawing_area, "button_press_event", G_CALLBACK(click_drawing), NULL); 
    block_id=g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_square), NULL); 
    g_signal_connect(drawing_area, "realize", G_CALLBACK(realize_drawing), NULL); 

    gtk_container_add(GTK_CONTAINER(window), drawing_area); 

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
  }
static void close_window(GtkWidget *widget, gpointer data)
  {
    //Remove timer on exit.
    if(moving)
      {
        if(timer_id!=0) g_source_remove(timer_id);
      }
    gtk_main_quit();
  }
static gboolean start_drawing(gpointer widget)
  {
    GtkAllocation allocation;
    GdkWindow *win=gtk_widget_get_window(GTK_WIDGET(widget));
    gtk_widget_get_allocation(GTK_WIDGET(widget), &allocation);
    if(!click1) gdk_window_invalidate_rect(win, &allocation, FALSE);

    if(move>move_ulimit) return FALSE;
    else return TRUE;
      
  }
static void realize_drawing(GtkWidget *widget, gpointer data)
  {
    printf("Drawing Realized\n");
  }
static void click_drawing(GtkWidget *widget, gpointer data)
  {
    //If animation is running, don't start a new timer.
    if(!moving)
      {
        timer_id=g_timeout_add(30, start_drawing, widget);         
        moving=TRUE;
      }

    if(click1) gtk_widget_queue_draw_area(widget, 0, 0, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget)); 

    //Toggle start and stop animation.
    if(click1)
      {
        click1=FALSE;
        if(move>0) timer_id=g_timeout_add(30, start_drawing, widget);
      }
    else
      {
        click1=TRUE;
        g_source_remove(timer_id);
        timer_id=0;
      } 
  }
static gboolean draw_square(GtkWidget *widget, cairo_t *cr, gpointer data)
  {  
    int i=0;
    double x1=0;
    double y1=0;
    double y2=0;
    static double last_x1=0;
    static double last_y1=0;
    static double last_x2=0;
    static double last_y2=0;
    double current_x1=0;
    double current_y1=0;
    double current_x2=0;
    double current_y2=0;

    //g_print("Draw Square %i\n", move);

    //Clear the surface.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_restore(cr);
    
    cairo_save(cr);
    //Rotate around center of square so move there.
    cairo_translate(cr, 30+move+50, 130+50);
    //Rotate the square. Tip the square on end with pi/4 to start. 
    cairo_rotate(cr, (((move/10)*G_PI/25.0)+G_PI/4.0));
    //Move back to to the corner of the square and draw the square.
    cairo_translate(cr, -50, -50);
    cairo_set_source_rgb(cr, 0.5, 0, 0.5);
    cairo_rectangle(cr, 0, 0, 100.0, 100.0);
    cairo_fill(cr);

    //Add a circle around the square.
    cairo_translate(cr, 50, 50);
    cairo_set_source_rgb(cr, 1, 1, 0);
    cairo_set_line_width(cr, 4.0);
    cairo_arc(cr, 0, 0, sqrt(5000.0), 0, 2*M_PI);
    cairo_close_path(cr);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Put a line in for the square to roll on.
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_move_to(cr, 0, 130+50+sqrt(5000.0));
    cairo_line_to(cr, 1000, 130+50+sqrt(5000.0));
    cairo_set_line_width(cr, 3.0);
    cairo_stroke(cr);

    //Make a line in the square.
    cairo_set_source_rgb(cr, 0, 1.0, 0);
    cairo_set_line_width(cr, 2.0);
    cairo_move_to(cr,30+50+move, 130+50);
    cairo_arc(cr, 30+50+move, 130+50, sqrt(5000.0), (((move/10)*G_PI/25.0)+G_PI/2.0), (((move/10)*G_PI/25.0)+G_PI/2.0));
    cairo_line_to(cr, 30+50+move, 130+50);
    cairo_stroke(cr);

    //Set line colors and width.
    for(i=0;i<move;i+=10)
       {
         //The bridge.
         cairo_set_line_width(cr, 2.0);
         cairo_set_source_rgb(cr, 0, 1.0, 0);
         cairo_move_to(cr,30+50+i, 130+50);
         cairo_arc(cr, 30+50+i, 130+50, sqrt(5000.0), (((i/10)*G_PI/25.0)+G_PI/2.0), (((i/10)*G_PI/25.0)+G_PI/2.0));
         cairo_get_current_point(cr, &current_x1, &current_y1);
         cairo_line_to(cr, 30+50+i, 130+50);
         cairo_stroke(cr);
         cairo_move_to(cr, current_x1, current_y1);
         if(i!=0) cairo_line_to(cr, last_x1, last_y1);
         cairo_stroke(cr);
         last_x1=current_x1;
         last_y1=current_y1;
         //The sine wave.
         cairo_set_source_rgb(cr, 0, 0, 1.0);
         y1=(sqrt(5000.0))*sin(((i/10.0)*G_PI/25.0)+G_PI/2.0);
         y2=(sqrt(5000.0))*sin((((i+10.0)/10.0)*G_PI/25.0)+G_PI/2.0);
         cairo_move_to(cr, 30+50+i, 130+50);
         cairo_line_to(cr, 30+50+i, 130+50+y1);
         cairo_stroke_preserve(cr);
         cairo_line_to(cr, 30+50+(i+10), 130+50+y2); 
         cairo_stroke(cr);
         //The cosine wave. Rotate slightly off the x-axis.
         cairo_set_source_rgb(cr, 1, 0, 0);
         x1=(sqrt(5000.0))*cos(((i/10.0)*G_PI/25.0)+G_PI/2.0);
         if(x1>=0)
           {
             cairo_arc(cr, 30+50+i, 130+50, (int)floor(x1), G_PI/8, G_PI/8);
             cairo_get_current_point(cr, &current_x2, &current_y2);
           }
         else
           {
             cairo_arc(cr, 30+50+i, 130+50, (int)floor(fabs(x1)), G_PI+G_PI/8, G_PI+G_PI/8);
             cairo_get_current_point(cr, &current_x2, &current_y2);
           }
         cairo_line_to(cr, 30+50+i, 130+50);
         cairo_stroke(cr);
         cairo_move_to(cr, current_x2, current_y2);
         if(i!=0) cairo_line_to(cr, last_x2, last_y2);
         cairo_stroke(cr);
         last_x2=current_x2;
         last_y2=current_y2;
         //The sine cosine color component line.
         cairo_set_line_width(cr, 40.0); 
         cairo_set_source_rgb(cr, fabs(x1)/sqrt(5000), 0, fabs(y1)/sqrt(5000)); 
         cairo_move_to(cr, 30+50+i, 130+50+110);
         cairo_line_to(cr, 30+50+i+10, 130+50+110);
         cairo_stroke(cr);       
       }
  
    if(moving) move+=5;

    //Cancel timer.
    if(move>move_ulimit)
      {
        g_source_remove(timer_id);
        moving=FALSE;
        move=0;
        click1=TRUE;
        gtk_widget_queue_draw_area(widget, 0, 0, gtk_widget_get_allocated_width(widget), gtk_widget_get_allocated_height(widget));  
      }

    return TRUE;

  }


