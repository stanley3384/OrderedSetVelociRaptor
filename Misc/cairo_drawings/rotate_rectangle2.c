
/*

    Test code for putting an image onto a rotating rectangle using cairo. Like flipping over a card in 3d.
    The code needs an image file in the working folder and in the function gdk_pixbuf_new_from_file_at_scale
for it to work.

    gcc -Wall rotate_rectangle2.c -o rotate_rectangle2 -lm `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<math.h>

static guint timer_id=0;

static gboolean rotate_rectangle(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean start_drawing(gpointer drawing);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void quit_program(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Rotate Rectangle");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_widget_set_app_paintable(window, TRUE);
    //Set transparency of main window.
    if(gtk_widget_is_composited(window))
      {
        GdkScreen *screen = gtk_widget_get_screen(window);
        GdkVisual *visual = gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(window, visual);
      }
    else g_print("Can't set window transparency.\n");
    g_signal_connect(window, "draw", G_CALLBACK(draw_background), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(quit_program), NULL);

    GError *error=NULL;
    GdkPixbuf *pixbuf=NULL;
    //Scale the image file.
    pixbuf=gdk_pixbuf_new_from_file_at_scale("dino2.png", 200, 300, FALSE, &error);
    if(error)
      {
        g_print("%s\n", error->message);
      }
    if(error!=NULL)g_error_free(error);
    cairo_surface_t *image=gdk_cairo_surface_create_from_pixbuf(pixbuf, 0, NULL);

    GtkWidget *drawing=gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing, TRUE);
    gtk_widget_set_vexpand(drawing, TRUE);
    g_signal_connect(drawing, "draw", G_CALLBACK(rotate_rectangle), image);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), drawing, 0, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    timer_id=g_timeout_add(50, start_drawing, drawing); 

    gtk_main();

    return 0;   
  }
static gboolean rotate_rectangle(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    static gint i=1;
    static gboolean rise=TRUE;
    gdouble scale_x=sin(i*G_PI/32);
    gdouble scale_x_inv=1.0/scale_x;
    i++;
    if((int)fabs(scale_x)==1) 
      {
        if(rise) rise=FALSE;
        else rise=TRUE;
      }

    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(cr);

    //x-axis behind rectangle.
    if(scale_x<0)
      {
        cairo_save(cr);
        cairo_set_line_width(cr, 4);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_move_to(cr, width/2, height/2);
        if(rise) cairo_line_to(cr, fabs(scale_x*width/2), height/2);
        else cairo_line_to(cr, (width/2)+(1.0+scale_x)*width/2, height/2);
        cairo_stroke(cr);
        cairo_restore(cr);
      }

    //y-axis
    cairo_save(cr);
    cairo_set_line_width(cr, 4);
    cairo_set_source_rgb(cr, 0.8, 0.0, 0.8);
    cairo_move_to(cr, width/2, height/2);
    cairo_line_to(cr, width/2, -height/2);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Draw the image.
    if(scale_x>0)
      {
        cairo_save(cr);
        cairo_scale(cr, scale_x, 1.0);
        cairo_translate(cr, scale_x_inv*(width/2), height/2);    
        cairo_surface_t *card=NULL;
        if(scale_x<0.5&&scale_x>-0.5)
          {
            card=cairo_surface_create_for_rectangle(cairo_get_target(cr), (width/2)+(1.0-fabs(scale_x)*100), height/2-150, 200, 300);
          }
        else
          {
            card=cairo_surface_create_for_rectangle(cairo_get_target(cr), (width/2-100)+(((1.0-fabs(scale_x))*100)), height/2-150, 200, 300);
          }
        cairo_t *cr2=cairo_create(card);
        cairo_scale(cr2, fabs(scale_x), 1.0);    
        cairo_set_source_surface(cr2, (cairo_surface_t*)data, 0, 0);
        cairo_paint(cr2);
        cairo_set_source_surface(cr, card, 0, 0);
        cairo_destroy(cr2);
        cairo_surface_destroy(card);
        cairo_restore(cr);
      }

    //Rectangle and font.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_save(cr);
    cairo_scale(cr, scale_x, 1.0);
    cairo_translate(cr, scale_x_inv*(width/2), height/2);
    cairo_rectangle(cr, -100, -150, 200, 300);
    if(scale_x<0) cairo_fill(cr);
    if(scale_x>0)
      {
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_move_to(cr, 75, 125);
        cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 20);
        cairo_show_text(cr, "7");  
        cairo_stroke(cr);
        cairo_move_to(cr, -75, -125);
        cairo_show_text(cr, "7");
      }  
    cairo_stroke(cr);
    cairo_restore(cr);

    //x-axis in front of rectangle.
    if(scale_x>=0)
      {
        cairo_save(cr);
        cairo_set_line_width(cr, 4);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_move_to(cr, width/2, height/2);
        if(rise) cairo_line_to(cr, scale_x*width/2, height/2);
        else cairo_line_to(cr, (width/2)+(1.0-scale_x)*width/2, height/2);
        cairo_stroke(cr);
        cairo_restore(cr);
      }

    return FALSE;
  }
static gboolean start_drawing(gpointer drawing)
  {
    gtk_widget_queue_draw(GTK_WIDGET(drawing));
    return TRUE;
  }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    cairo_paint(cr);
    return FALSE;
  }
static void quit_program(GtkWidget *widget, gpointer data)
  {
    //Remove the timer before quiting to prevent possible GTK+ error on exit.
    g_source_remove(timer_id);
    gtk_main_quit();
  }






