
/*
    Put a flat rotating clock in 3-space. 

    Tested on Ubuntu16.04 with GTK3.18.

    gcc -Wall space_clock.c -o space_clock -lm `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static guint tick_id=0;

static gboolean rotate_rectangle(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean start_drawing(GtkWidget *drawing, GdkFrameClock *frame_clock, gpointer data);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void quit_program(GtkWidget *widget, GtkWidget *da);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Cairo Space Clock");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_widget_set_app_paintable(window, TRUE);
    //Try to set transparency of main window.
    if(gtk_widget_is_composited(window))
      {
        GdkScreen *screen=gtk_widget_get_screen(window);  
        GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(window, visual);
      }
    else g_print("Can't set window transparency.\n");
    g_signal_connect(window, "draw", G_CALLBACK(draw_background), NULL);

    GtkWidget *drawing=gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing, TRUE);
    gtk_widget_set_vexpand(drawing, TRUE);
    g_signal_connect(drawing, "draw", G_CALLBACK(rotate_rectangle), NULL);
    g_signal_connect(window, "destroy", G_CALLBACK(quit_program), drawing);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), drawing, 0, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    tick_id=gtk_widget_add_tick_callback(drawing, (GtkTickCallback)start_drawing, NULL, NULL); 

    gtk_main();

    return 0;   
  }
static gboolean rotate_rectangle(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gint i=0;
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Keep the drawing circular.
    if(w1>h1) w1=h1;

    //Paint the background as clear.
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(cr);
  
    //Get the current time.
    GTimeZone *time_zone=g_time_zone_new_local();
    GDateTime *date_time=g_date_time_new_now(time_zone);
    gdouble hour=(gdouble)g_date_time_get_hour(date_time);
    gdouble minute=(gdouble)g_date_time_get_minute(date_time);
    gint second=g_date_time_get_second(date_time);
    hour=hour+minute/60.0;
    g_time_zone_unref(time_zone);
    g_date_time_unref(date_time);
    if(hour>12) hour=hour-12;

    static gint j=0;
    gdouble angle=-j*G_PI/256.0;
    gdouble scale_x=cos(angle);
    gdouble scale_x_inv=1.0/scale_x;
    angle=angle+G_PI/2.0;
    j++;

    //The ring line pattern.
    cairo_pattern_t *pattern1;  
    pattern1=cairo_pattern_create_linear(width/2.0, (height/2.0)-(w1), width/2.0, (height/2.0)+(w1));
    cairo_pattern_add_color_stop_rgba(pattern1, 0.1, 0.0, 0.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.5, 0.0, 1.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.9, 1.0, 1.0, 0.0, 1.0);
    
    //The ring line behind the clock.
    cairo_save(cr);
    cairo_set_source(cr, pattern1);  
    cairo_set_line_width(cr, 20);
    cairo_scale(cr, 1.0, 0.20);
    cairo_translate(cr, 0.0,  2.0*height);
    cairo_arc(cr, width/2.0, height/2.0, 4.7*w1, G_PI, 2.0*G_PI);
    cairo_stroke(cr);
    cairo_restore(cr);

    cairo_save(cr);

    //The circle gradient.
    cairo_scale(cr, scale_x, 1.0);
    cairo_translate(cr, scale_x_inv*width/2.0, height/2.0);
    cairo_pattern_t *pattern2 = cairo_pattern_create_radial(0.0, 0.0, 2.0*w1, 0.0, 0.0, 4.0*w1);
    if(scale_x>0)
      {
        cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 1.0, 1.0, 0.0, 0.7);
        cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.0, 1.0, 1.0, 0.7);
      }
    else
      {
        cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 1.0, 1.0, 0.0, 0.7);
        cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.0, 0.0, 1.0, 0.7);
      }
    cairo_set_source(cr, pattern2);
    cairo_arc (cr, 0.0, 0.0, 4.0*w1, 0, 2.0*G_PI);
    cairo_fill(cr);
    cairo_pattern_destroy(pattern2);

    //Set the clock text.
    gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CSC"};
    gdouble hour_start=-G_PI/2.0;
    gdouble next_hour=-G_PI/6.0;
    //Start at 12 or radius with just the y component.
    gdouble hour_radius=3.5*h1;
    gdouble temp_cos=0;
    gdouble temp_sin=0;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_text_extents_t tick_extents;
    cairo_set_font_size(cr, 20*(w1/40.0));
    cairo_move_to(cr, 0.0, 0.0);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    for(i=0;i<12;i++)
      {
        temp_cos=cos(hour_start-(next_hour*i));
        temp_sin=sin(hour_start-(next_hour*i));
        hour_radius=3.5*w1;
        cairo_text_extents(cr, hours[i], &tick_extents);
        temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
        temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
        cairo_move_to(cr, temp_cos, temp_sin);
        cairo_show_text(cr, hours[i]);
      }

    //Put the clock name on the clock.
    cairo_text_extents(cr, hours[12], &tick_extents);
    cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
    cairo_show_text(cr, hours[12]);

    //Hour hand.
    cairo_set_line_width(cr, 6.0*w1/40.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, 0.0, 0.0);
    temp_cos=cos(hour_start-(next_hour*hour));
    temp_sin=sin(hour_start-(next_hour*hour));
    hour_radius=1.5*w1;
    temp_cos=temp_cos*hour_radius;
    temp_sin=temp_sin*hour_radius;
    cairo_line_to(cr, temp_cos, temp_sin);
    cairo_stroke(cr);

    //Minute hand.
    gdouble next_minute=-G_PI/(6.0*5.0);
    cairo_move_to(cr, 0.0, 0.0);
    temp_cos=cos(hour_start-(next_minute*minute));
    temp_sin=sin(hour_start-(next_minute*minute));
    hour_radius=2.5*w1;
    temp_cos=temp_cos*hour_radius;
    temp_sin=temp_sin*hour_radius;
    cairo_line_to(cr, temp_cos, temp_sin);
    cairo_stroke(cr);

    //Second hand.
    cairo_set_line_width(cr, 4.0*w1/40.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.3);
    gdouble start=-G_PI/2.0;
    gdouble next=-G_PI/30.0;
    temp_cos=0;
    temp_sin=0;
    temp_cos=cos(start-(next*second));
    temp_sin=sin(start-(next*second));
    hour_radius=3.0*w1;
    temp_cos=temp_cos*hour_radius;
    temp_sin=temp_sin*hour_radius;
    cairo_move_to(cr, 0.0, 0.0);
    cairo_line_to(cr, temp_cos, temp_sin);
    cairo_stroke(cr);

    cairo_restore(cr);
 
    //The ring line in front of the clock.
    cairo_save(cr);
    cairo_set_source(cr, pattern1);  
    cairo_set_line_width(cr, 20);
    cairo_scale(cr, 1.0, 0.20); 
    cairo_translate(cr, 0.0,  2.0*height);
    cairo_arc(cr, width/2.0, height/2.0, 4.7*w1, 0.0, G_PI);
    cairo_stroke(cr);
    cairo_restore(cr);

    cairo_pattern_destroy(pattern1);
    return FALSE;
  }
static gboolean start_drawing(GtkWidget *drawing, GdkFrameClock *frame_clock, gpointer data)
  {
    gtk_widget_queue_draw(GTK_WIDGET(drawing));
    return G_SOURCE_CONTINUE;
  }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    cairo_paint(cr);
    return FALSE;
  }
static void quit_program(GtkWidget *widget, GtkWidget *da)
  {
    if(tick_id!=0) gtk_widget_remove_tick_callback(da, tick_id);
    gtk_main_quit();
  }






