
/*

    Layer two drawing areas with an overlay. Draw rectangles on the top layer and add them
to the bottom layer. This way you can add shapes to a cairo surface and clear them.

    gcc -Wall draw_rectangle1.c -o draw_rectangle1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/

#include <gtk/gtk.h>

//Bottom drawing area for the green rectangle.
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);
//Top drawing area for the red rectangle.
static gboolean draw_foreground(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data);
//Add the rectangle from the top layer to the bottom layer.
static void add_rectangle(GtkWidget *widget, gpointer data);
static void clear_rectangles(GtkWidget *widget, gpointer *data);


//For blocking motion signal. Block when not drawing top rectangle
gint motion_id=0;
//Coordinates for the drawing rectangle.
gdouble rect[]={0.0, 0.0, 0.0, 0.0};

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    GtkWidget *window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Rectangle");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    //Save some rectangles for drawing in da1.
    GArray *rectangles=g_array_new(FALSE, FALSE, sizeof(gdouble));

    //Bottom drawing area.
    GtkWidget *da1 = gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_background), (gpointer)rectangles);

    GtkWidget *overlay=gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);
    gtk_container_add(GTK_CONTAINER(overlay), da1);

    //Top drawing area
    GtkWidget *da2 = gtk_drawing_area_new();
    gtk_widget_set_hexpand(da2, TRUE);
    gtk_widget_set_vexpand(da2, TRUE);
    //Add some extra events to the top drawing area.
    gtk_widget_add_events(da2, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK);
    g_signal_connect(da2, "draw", G_CALLBACK(draw_foreground), NULL);
    g_signal_connect(da2, "button-press-event", G_CALLBACK(start_press), NULL);
    g_signal_connect(da2, "button-release-event", G_CALLBACK(stop_press), NULL);
    motion_id=g_signal_connect(da2, "motion-notify-event", G_CALLBACK(cursor_motion), NULL);
    g_signal_handler_block(da2, motion_id);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), da2);

    GtkWidget *button1=gtk_button_new_with_label("Add Rectangle");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(add_rectangle), (gpointer)rectangles);

    gpointer clear[]={rectangles, da1};
    GtkWidget *button2=gtk_button_new_with_label("Clear Rectangles");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(button2, "clicked", G_CALLBACK(clear_rectangles), clear);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), overlay, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 2, 4, 2, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    //Get drawing area size.
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);

    //Paint the background blue.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    //Put a green rectangle in the background.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 20, 20, width-40, height-40);
    cairo_stroke(cr);

    //Draw the stored rectangles
    gint i=0;
    gint len=((GArray*)data)->len/4;
    gdouble x1=0;
    gdouble y1=0;
    gdouble x2=0;
    gdouble y2=0;
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    for(i=0;i<len;i++)
      {
        x1=g_array_index((GArray*)data, gdouble, 4*i);
        y1=g_array_index((GArray*)data, gdouble, 4*i+1);
        x2=g_array_index((GArray*)data, gdouble, 4*i+2);
        y2=g_array_index((GArray*)data, gdouble, 4*i+3);
        cairo_rectangle(cr, x1, y1, x2-x1, y2-y1);
        cairo_stroke(cr);
      }
    

    return TRUE;
  }
static gboolean draw_foreground(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    //Paint the foreground transparent.
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    //Draw a red rectangle.
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, rect[0], rect[1], rect[2]-rect[0], rect[3]-rect[1]);
    cairo_stroke(cr);

    return TRUE;
  }
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_print("Button Pressed\n");
    g_signal_handler_unblock(widget, motion_id);
    GdkWindow *win=gtk_widget_get_window(widget);
    cairo_t *cr=gdk_cairo_create(win);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_destroy(cr);

    rect[0]=event->button.x;
    rect[1]=event->button.y;

    return TRUE;
  }
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_print("Button Released\n");
    g_signal_handler_block(widget, motion_id);
    return TRUE;
  }
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    rect[2]=event->button.x;
    rect[3]=event->button.y;
    g_print("%f %f %f %f\n", rect[0], rect[1], rect[2], rect[3]);
    gtk_widget_queue_draw(widget);
    return TRUE;
  }
static void add_rectangle(GtkWidget *widget, gpointer data)
  {
    g_print("Add Rectangle\n");
    g_array_append_val((GArray*)data, rect[0]);
    g_array_append_val((GArray*)data, rect[1]);
    g_array_append_val((GArray*)data, rect[2]);
    g_array_append_val((GArray*)data, rect[3]);
    g_print("%f %f %f %f\n", rect[0], rect[1], rect[2], rect[3]);
    
    g_print("Rectangles\n");
    gint i=0;
    gint len=((GArray*)data)->len/4;
    for(i=0;i<len;i++)
      {
        g_print("%i %f %f %f %f\n", len, g_array_index((GArray*)data, gdouble, 4*i), g_array_index((GArray*)data, gdouble, 4*i+1), g_array_index((GArray*)data, gdouble, 4*i+2), g_array_index((GArray*)data, gdouble, 4*i+3));
      }
  }
static void clear_rectangles(GtkWidget *widget, gpointer *data)
  {
    rect[0]=0.0;
    rect[1]=0.0;
    rect[2]=0.0;
    rect[3]=0.0;
    g_array_remove_range((GArray*)data[0], 0, ((GArray*)data[0])->len);
    gtk_widget_queue_draw(GTK_WIDGET(data[1]));
  }






