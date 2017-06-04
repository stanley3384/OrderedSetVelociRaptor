
/*
    Draw a few gears and give them a spin. Then try to synchonize them.

    gcc -Wall gears1.c -o gears1 `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gint drawing_id=0;
static gdouble rotate1=G_PI/2.0;
static gdouble rotate2=G_PI/2.0;
static gdouble rotate3=G_PI/2.0;
static gdouble rotate4=G_PI/2.0+G_PI/6.0;
//Synchronization for three gears drawing.
static gdouble sync2=0;
static gdouble sync3=0;
static gdouble sync4=0;
//ID for the frame clock.
static guint tick_id=0;

static void combo_changed(GtkComboBox *combo_box, gpointer data);
static gboolean animate_gears(GtkWidget *da, GdkFrameClock *frame_clock, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_gears(GtkWidget *da, cairo_t *cr);
static void gear(cairo_t *cr, gdouble w1, gdouble inside_radius, gdouble outside_radius, gint teeth, gdouble bevel, gboolean fill);
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1);
static void gears2(cairo_t *cr, gdouble width, gdouble height, gdouble w1);
static void gears3(cairo_t *cr, gdouble width, gdouble height, gdouble w1);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gears");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "Gear");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "Gears");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "Sync Gears");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_widget_set_hexpand(combo, TRUE);
    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), da);  
  
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
  
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
static void combo_changed(GtkComboBox *combo_box, gpointer data)
  {
    //Reset rotation.
    rotate1=G_PI/2.0;
    rotate2=G_PI/2.0;
    rotate3=G_PI/2.0;
    rotate4=G_PI/2.0+G_PI/6.0;
    gint combo_id=gtk_combo_box_get_active(combo_box);
    if(combo_id==0)
      {
        drawing_id=0;
        if(tick_id!=0)
          {
            gtk_widget_remove_tick_callback(GTK_WIDGET(data), tick_id);
            tick_id=0;
          }
        gtk_widget_queue_draw(GTK_WIDGET(data));
      }
    else if(combo_id==1) drawing_id=1;
    else drawing_id=2;

    if(drawing_id>0)
      {
        gtk_widget_remove_tick_callback(GTK_WIDGET(data), tick_id);
        tick_id=gtk_widget_add_tick_callback(GTK_WIDGET(data), (GtkTickCallback)animate_gears, NULL, NULL);
      }
  }
static gboolean animate_gears(GtkWidget *da, GdkFrameClock *frame_clock, gpointer data)
  {
    //Check frame rate.
    gint64 frame=gdk_frame_clock_get_frame_counter(frame_clock);
    if(frame%60==0&&frame>0)
      {
        gint64 current_time=gdk_frame_clock_get_frame_time(frame_clock);
        gint64 start = gdk_frame_clock_get_history_start(frame_clock);
        gint64 history_len=frame-start;
        GdkFrameTimings *previous_timings=gdk_frame_clock_get_timings(frame_clock, frame-history_len);
        gint64 previous_frame_time=gdk_frame_timings_get_frame_time(previous_timings);
        g_print("Frame %lld, %f fps\n", frame, (gdouble)(history_len)*G_USEC_PER_SEC/(gdouble)(current_time-previous_frame_time));
      }
   
    if(drawing_id==1)
      {
        rotate1+=G_PI/256.0;
        rotate2+=G_PI/128.0;
        rotate3-=G_PI/64.0;
        rotate4+=G_PI/64.0;
      }
    else
      {
        rotate2+=G_PI/128.0*sync2;
        rotate3-=G_PI/128.0*sync3;
        rotate4+=G_PI/128.0*sync4;
      }
   
    gtk_widget_queue_draw(GTK_WIDGET(da));
    return G_SOURCE_CONTINUE;
  }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    //GTimer *timer=g_timer_new();
    draw_gears(da, cr);
    //g_print("%f\n", g_timer_elapsed(timer, NULL));
    //g_timer_destroy(timer); 
    return FALSE;
  }
static void draw_gears(GtkWidget *da, cairo_t *cr)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Scale.
    if(w1>h1)
      {
        w1=h1;
      }
  
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //Layout axis for drawing.
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, width/10.0, height/10.0, 8.0*width/10.0, 8.0*height/10.0);
    cairo_stroke(cr);
    cairo_move_to(cr, 1.0*width/10.0, 5.0*height/10.0);
    cairo_line_to(cr, 9.0*width/10.0, 5.0*height/10.0);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*width/10.0, 1.0*height/10.0);
    cairo_line_to(cr, 5.0*width/10.0, 9.0*height/10.0);
    cairo_stroke(cr); 

    //Draw the gear or gears.
    if(drawing_id==0) gears1(cr, width, height, w1); 
    else if(drawing_id==1) gears2(cr, width, height, w1);
    else gears3(cr, width, height, w1);
 
  }
static void gear(cairo_t *cr, gdouble w1, gdouble inside_radius, gdouble outside_radius, gint teeth, gdouble bevel, gboolean fill)
  {
    gint i=0;
    gdouble step=-G_PI*2.0/teeth;

    //Outside points.
    gdouble outside_start1=-G_PI/2.0-bevel;
    gdouble outside_cos1=0;
    gdouble outside_sin1=0;
    gdouble outside_start2=-G_PI/2.0+bevel;
    gdouble outside_cos2=0;
    gdouble outside_sin2=0;
    //Inside points.
    gdouble inside_start1=-G_PI/2.0+G_PI/teeth-bevel;
    gdouble inside_cos1=0;
    gdouble inside_sin1=0;
    gdouble inside_start2=-G_PI/2.0+G_PI/teeth+bevel;
    gdouble inside_cos2=0;
    gdouble inside_sin2=0;
    //Draw the gear teeth.
    for(i=0;i<teeth;i++)
      {
        outside_cos1=cos(outside_start1-(step*i))*outside_radius;
        outside_sin1=sin(outside_start1-(step*i))*outside_radius;                
        if(i==0) cairo_move_to(cr, outside_cos1, outside_sin1);
        else
          {
            cairo_line_to(cr, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);
          }

        outside_cos2=cos(outside_start2-(step*i))*outside_radius;
        outside_sin2=sin(outside_start2-(step*i))*outside_radius;                
        cairo_line_to(cr, outside_cos2, outside_sin2);
        cairo_stroke_preserve(cr);   
         
        inside_cos1=cos(inside_start1-(step*i))*inside_radius;
        inside_sin1=sin(inside_start1-(step*i))*inside_radius;
        cairo_line_to(cr, inside_cos1, inside_sin1);
        cairo_stroke_preserve(cr); 

        inside_cos2=cos(inside_start2-(step*i))*inside_radius;
        inside_sin2=sin(inside_start2-(step*i))*inside_radius;
        cairo_line_to(cr, inside_cos2, inside_sin2);
        cairo_stroke_preserve(cr);                          
     }
    cairo_close_path(cr);
    if(fill) cairo_fill(cr); 
    else cairo_stroke(cr);
  }
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1)
  {
    //Some gear variables for the gear.
    gdouble inside_radius=3.2*w1;
    gdouble outside_radius=4.0*w1;
    gint teeth=12;
    gdouble bevel=G_PI/36;
    gboolean fill=FALSE;

    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, width/2.0, height/2.0); 
    gear(cr, w1, inside_radius, outside_radius, teeth, bevel, fill);
    cairo_arc(cr, 0.0, 0.0, 2.5*w1, 0, 2*G_PI);
    cairo_stroke(cr); 
    cairo_restore(cr);
  }
static void gears2(cairo_t *cr, gdouble width, gdouble height, gdouble w1)
  {
    //Some gear variables for the gears.
    gdouble inside_radius=3.2*w1;
    gdouble outside_radius=4.0*w1;
    gint teeth=12;
    gdouble bevel=G_PI/36;
    gboolean fill=FALSE;

    //Draw some gears.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 3.0*width/10.0, 3.0*height/10.0);
    cairo_rotate(cr, rotate1);
    cairo_scale(cr, 0.5, 0.5); 
    gear(cr, w1, inside_radius, outside_radius, teeth, bevel, fill);
    cairo_arc(cr, 0.0, 0.0, 2.5*w1, 0, 2*G_PI);
    cairo_stroke(cr); 
    cairo_restore(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 7.0*width/10.0, 3.0*height/10.0);
    cairo_rotate(cr, rotate2);
    cairo_scale(cr, 0.5, 0.5); 
    gear(cr, w1, inside_radius, outside_radius, teeth, bevel, fill);
    cairo_arc(cr, 0.0, 0.0, 2.5*w1, 0, 2*G_PI);
    cairo_stroke(cr); 
    cairo_restore(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 3.0*width/10.0, 7.0*height/10.0);
    cairo_rotate(cr, rotate3);
    cairo_scale(cr, 0.5, 0.5); 
    gear(cr, w1, inside_radius, outside_radius, teeth, bevel, fill);
    cairo_arc(cr, 0.0, 0.0, 2.5*w1, 0, 2*G_PI);
    cairo_stroke(cr); 
    cairo_restore(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 7.0*width/10.0, 7.0*height/10.0);
    cairo_rotate(cr, rotate4);
    cairo_scale(cr, 0.5, 0.5); 
    gear(cr, w1, inside_radius, outside_radius, teeth, bevel, fill);
    cairo_arc(cr, 0.0, 0.0, 2.5*w1, 0, 2*G_PI);
    cairo_stroke(cr); 
    cairo_restore(cr);     
  }
static void gears3(cairo_t *cr, gdouble width, gdouble height, gdouble w1)
  {
    //Some gear variables for the gears.
    gdouble inside_radius=3.2*w1;
    gdouble outside_radius=4.0*w1;

    //Sync the gears.
    sync2=1.2;
    sync3=1.0;
    sync4=2.0;

    //Draw some different sized gears.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 3.0*w1, 3.7*w1);
    cairo_rotate(cr, rotate2);
    cairo_scale(cr, 0.375, 0.375); 
    gear(cr, w1, 3.0*w1, outside_radius, 10, G_PI/30.0, TRUE);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, 0.0, 0.0, 1.0*w1, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 0.0, 1.0*w1);
    cairo_line_to(cr, 0.0, outside_radius);
    cairo_stroke(cr); 
    cairo_restore(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 3.0*w1, 7.0*w1);
    cairo_rotate(cr, rotate3);
    cairo_scale(cr, 0.5, 0.5); 
    gear(cr, w1, inside_radius, outside_radius, 12, G_PI/48, TRUE);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, 0.0, 0.0, 1.5*w1, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, 0.0, 1.5*w1);
    cairo_line_to(cr, 0.0, outside_radius);
    cairo_stroke(cr);  
    cairo_restore(cr);

    cairo_save(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, 5.75*w1, 7.0*w1);
    cairo_rotate(cr, rotate4);
    cairo_scale(cr, 0.25, 0.25); 
    gear(cr, w1, 2.8*w1, outside_radius, 6, G_PI/16.0, TRUE);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_arc(cr, 0.0, 0.0, 1.5*w1, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0); 
    cairo_move_to(cr, 0.0, 1.5*w1);
    cairo_line_to(cr, 0.0, outside_radius);
    cairo_stroke(cr); 
    cairo_restore(cr); 
  }
