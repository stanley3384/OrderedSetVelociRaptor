
/*

   Test using cairo to animate text over an image. The program needs an image file in the
working directory and in the program. A png or jpeg will work. Preferably the image size
should be small. Tested with png's and jpeg's of around 300kb. The pixbuf will get resized
on every draw so there is some overhead. Similar to the drawSVG.py script. From a question
on the GTK+ Forum titled "How do i render text to a pixbuf?"
 
   gcc -Wall draw_chars1.c -o draw_chars1 `pkg-config --cflags --libs gtk+-3.0`
  
   C. Eric Cashon
*/

#include <gtk/gtk.h>

static gboolean start_drawing(gpointer data);
static void reallocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data);
static gboolean draw_some_chars(GtkWidget *widget, cairo_t *cr, gpointer data);

int main (int argc, char *argv[])
 {
    gtk_init(&argc, &argv);
    
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GError *error=NULL;
    GdkPixbuf *pixbuf=NULL;
    //Change the image file.
    pixbuf=gdk_pixbuf_new_from_file_at_scale("Bob_iris.jpeg", 400, 400, FALSE, &error);
    if(error)
      {
        g_print("%s\n", error->message);
      }
    if(error!=NULL)g_error_free(error);
    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request (drawing_area, 400, 400); 
    g_signal_connect(drawing_area, "draw", G_CALLBACK(draw_some_chars), pixbuf); 
    g_signal_connect(drawing_area, "size_allocate", G_CALLBACK(reallocate), NULL); 

    gtk_container_add(GTK_CONTAINER(window), drawing_area); 

    g_timeout_add(50, start_drawing, drawing_area); 

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
  }
static gboolean start_drawing(gpointer widget)
  {
    gtk_widget_queue_draw_area(GTK_WIDGET(widget), 0, 0, gtk_widget_get_allocated_width(GTK_WIDGET(widget)), gtk_widget_get_allocated_height(GTK_WIDGET(widget)));
    return TRUE;      
  }
static void reallocate(GtkWidget *widget, GdkRectangle *allocation, gpointer data)
  {
    g_print("Reallocate\n");
    gint width=allocation->width;
    gint height=allocation->height; 
    g_print("width %i height %i\n", width, height);
  }
static gboolean draw_some_chars(GtkWidget *widget, cairo_t *cr, gpointer data)
  {  
    static gint rotate = 0;
    gint width = gtk_widget_get_allocated_width(GTK_WIDGET(widget)); 
    gint height = gtk_widget_get_allocated_height(GTK_WIDGET(widget));
    gint center_x = width/2;
    gint center_y = height/2;
    if(data!=NULL)
      {
        GdkPixbuf *new_pixbuf=gdk_pixbuf_scale_simple(GDK_PIXBUF(data), width, height, GDK_INTERP_BILINEAR);
        gdk_cairo_set_source_pixbuf(cr, new_pixbuf, 0, 0);
        g_object_unref(new_pixbuf);
      }   
    cairo_paint(cr);
    cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 35);    
    cairo_set_source_rgb(cr, 1.0 , 1.0 , 0.0);
    cairo_move_to(cr, center_x, center_y);
    cairo_rotate(cr, rotate*(G_PI/180));
    rotate++;
    cairo_show_text(cr, "Cairo Text");
    cairo_stroke(cr);    
    return TRUE;
  }


