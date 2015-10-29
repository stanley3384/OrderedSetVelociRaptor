
/*
    Test code for putting a simple cairo mesh on a rotating rectangle in 3d using cairo.

    gcc -Wall rotate_rectangle3.c -o rotate_rectangle3 -lm `pkg-config --cflags --libs gtk+-3.0`

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
        GdkScreen *screen=gtk_widget_get_screen(window);  
        GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
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

    timer_id=g_timeout_add(100, start_drawing, drawing); 

    gtk_main();

    return 0;   
  }
static gboolean rotate_rectangle(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    static gint i=1;
    gdouble angle=i*G_PI/64.0;
    gdouble scale_x=sin(angle);
    gdouble scale_x_inv=1.0/scale_x;
    //g_print("scale_x %f\n", scale_x);
    i++;
    
    //Get the current time.
    GTimeZone *time_zone=g_time_zone_new_local();
    GDateTime *date_time=g_date_time_new_now(time_zone);
    gchar *string=g_date_time_format(date_time, "%I:%M:%S");
    g_time_zone_unref(time_zone);
    g_date_time_unref(date_time);

    //Get drawing area size.
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);

    //Paint the background as clear.
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(cr);

    //The oval behind the rectangle.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_set_line_width(cr, 4);
    cairo_scale(cr, 1.0, 0.20);
    cairo_translate(cr, 0.0, 800.0);
    cairo_arc(cr, width/2.0, height/2.0, 170.0, G_PI, 2.0*G_PI);
    cairo_stroke(cr);
    cairo_arc(cr, width/2.0, height/2.0, 130.0, G_PI, 2.0*G_PI);
    cairo_stroke(cr);
    cairo_restore(cr);

    //x-axis behind rectangle.
    if(scale_x<0)
      {
        cairo_save(cr);
        cairo_set_line_width(cr, 4);
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_move_to(cr, width/2.0, height/2.0);
        cairo_line_to(cr, width/2.0-(150.0*cos(angle)), height/2.0+30.0*sin(angle));
        cairo_stroke(cr);
        cairo_restore(cr);
      }

    //y-axis
    cairo_save(cr);
    cairo_set_line_width(cr, 4);
    cairo_set_source_rgb(cr, 0.8, 0.0, 0.8);
    cairo_move_to(cr, width/2.0, height/2.0);
    cairo_line_to(cr, width/2.0, -height/2.0);
    cairo_stroke(cr);
    cairo_restore(cr);

    //The mesh.
    cairo_save(cr);
    cairo_scale(cr, scale_x, 1.0);
    cairo_translate(cr, scale_x_inv*(width/2.0)-100, height/2.0-150);
    cairo_pattern_t *pattern = cairo_pattern_create_mesh();
    cairo_mesh_pattern_begin_patch(pattern);
    cairo_mesh_pattern_move_to(pattern, 0.0, 0.0);
    cairo_mesh_pattern_line_to(pattern, 200.0, 0.0);
    cairo_mesh_pattern_line_to(pattern, 200.0, 300.0);
    cairo_mesh_pattern_line_to(pattern, 0.0, 300.0);
    cairo_mesh_pattern_line_to(pattern, 0.0, 0.0);
    if(scale_x>0)
      {
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 0, 1, 0, 1, 0.7);
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 1, 1, 1, 0, 0.7);
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 2, 1, 0, 1, 0.7);
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 3, 1, 1, 0, 0.7);
      }
    else
      {
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 0, 1, 1, 0, 0.7);
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 1, 0, 0, 1, 0.7);
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 2, 1, 1, 0, 0.7);
        cairo_mesh_pattern_set_corner_color_rgba(pattern, 3, 0, 0, 1, 0.7);
      }
    cairo_mesh_pattern_end_patch(pattern);
    //g_print("Pattern Status %i\n", cairo_pattern_status(pattern));
    cairo_set_source(cr, pattern);
    cairo_rectangle(cr, 0.0, 0.0, 200.0, 300.0);
    cairo_fill(cr);
    cairo_pattern_destroy(pattern);
    cairo_restore(cr);

    //Rectangle and font.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_save(cr);
    cairo_scale(cr, scale_x, 1.0);
    cairo_translate(cr, scale_x_inv*(width/2.0), height/2.0);
    cairo_rectangle(cr, -100.0, -150.0, 200.0, 300.0);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30);
    gint j=0;
    int n_glyphs=g_utf8_strlen(string, -1);
    cairo_glyph_t glyphs[n_glyphs];
    /*
      Problem code. Adjust Courier font for glyphs. If cairo_show_text() is used the text is jumpy.
      Number shifted to asci code page decimal value. Subtract out the control chars? 
    */
    for(j=0;j<n_glyphs;j++)
      {
        glyphs[j]=(cairo_glyph_t){(gulong)string[j]-31, -100.0+20.0*(double)j + 20.0, 100.0};
        //g_print("%i %c %d %i| ", j, string[j], (int)string[j], (int)(string[j]-31));
      }
    //g_print("\n");
    cairo_show_glyphs(cr, glyphs, n_glyphs);
    cairo_stroke(cr);
    cairo_restore(cr);

    //The oval in front the rectangle.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_set_line_width(cr, 4);
    cairo_scale(cr, 1.0, 0.20);
    cairo_translate(cr, 0.0, 800.0);
    cairo_arc(cr, width/2, height/2, 170.0, 0.0, G_PI);
    cairo_stroke(cr);
    cairo_arc(cr, width/2, height/2, 130.0, 0.0, G_PI);
    cairo_stroke(cr);
    cairo_restore(cr);

    //x-axis in front of rectangle.
    if(scale_x>=0)
      {
        cairo_save(cr);
        cairo_set_line_width(cr, 4);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_move_to(cr, width/2.0, height/2.0);
        cairo_line_to(cr, width/2.0-(150.0*cos(angle)), height/2.0+30.0*sin(angle));
        cairo_stroke(cr);
        cairo_restore(cr);
      }

    g_free(string);
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






