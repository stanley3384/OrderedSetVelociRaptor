
/*
    Rotate a rectangle using cairo to get a 3d drawing and animation. Test some transparency along
with a radial pattern.

    gcc -Wall rotate_rectangle1.c -o rotate_rectangle1 -lm `pkg-config --cflags --libs gtk+-3.0`

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

    GtkWidget *drawing=gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing, TRUE);
    gtk_widget_set_vexpand(drawing, TRUE);
    g_signal_connect(drawing, "draw", G_CALLBACK(rotate_rectangle), NULL);

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
    gint j=0;
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

    //Card drawn with radial pattern.
    cairo_save(cr);
    cairo_pattern_t *radial1;
    cairo_scale(cr, scale_x, 1.0);
    cairo_translate(cr, scale_x_inv*(width/2), height/2);
    j=1000-((i%40)*25);
    //g_print("I %i J %i\n", i, j);
    radial1 = cairo_pattern_create_radial(0, 0, 1, 0, 0, j);  
    cairo_pattern_add_color_stop_rgba(radial1, 0.3, 1.0, 0.0, 1.0, 0.5);
    cairo_pattern_add_color_stop_rgba(radial1, 0.0, 1.0, 1.0, 0.0, 0.7);
    cairo_set_source(cr, radial1);
    cairo_rectangle(cr, -100, -150, 200, 300);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 75, 125);
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_show_text(cr, "7");  
    cairo_stroke(cr);
    cairo_move_to(cr, -75, -125);
    cairo_show_text(cr, "7");  
    cairo_stroke(cr);
    cairo_pattern_destroy(radial1);
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






