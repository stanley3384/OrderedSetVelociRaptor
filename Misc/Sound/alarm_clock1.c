
/*
    Put the circular_gradient_clock1.c and gstreamer_test2.c together to make an alarm clock.
The alarm sounds are .ogg files in the program folder. The sounds are loaded at startup. You can
get the barnyard making noise with this one. When the sounds play, a green rooster will appear
in the clock that you can click to stop the sounds.

    For some .ogg sounds to test out
    http://www.bigsoundbank.com 

    gcc -Wall alarm_clock1.c -o alarm_clock1 `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<gst/gst.h>
#include<math.h>

static GtkWidget *window;
//The sound files to play put in an array.
static GPtrArray *ogg_files;
static GArray *play_index;
static GMutex mutex;
static gint sounds_left=0;
static gint num_sounds=0;
static gchar *pool_string=NULL;
//For the alarm.
static gint alarm_hour=1;
static gint alarm_minute=1;
static gboolean alarm_am=FALSE;
static gboolean block_alarm=FALSE;
static gboolean alarm_set=FALSE;
static gboolean dialog_active=FALSE;

struct s_pipeline
{
  gpointer pool_id;
  GstElement *pipeline;
  GstElement *decoder;
  gint array_index;
  guint bus_watch_id;
  guint pipeline_id;
}; 

static GstTaskPool *pool;
static GtkWidget *da;
static GtkWidget *button1; 

//For clock display
static gboolean time_redraw(gpointer sounds);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_clock(cairo_t *cr, gdouble width, gdouble height);
static void draw_rooster(cairo_t *cr, gdouble width, gdouble height);
static gboolean click_rooster_drawing(GtkWidget *widget, GdkEvent *event, gpointer sounds);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void set_alarm_dialog(GtkWidget *widget, gpointer *data);
//For alarm setup dialog and sounds.
static void close_dialog(GtkDialog *dialog, gint response_id, gpointer data);
static gboolean draw_background_dialog(GtkWidget *widget, cairo_t *cr, gpointer data);
static void set_alarm_active(GtkWidget *widget, gpointer data);
static void set_alarm_spin1(GtkSpinButton *spin_button, gpointer data);
static void set_alarm_spin2(GtkSpinButton *spin_button, gpointer data);
static void set_alarm_check(GtkWidget *widget, gpointer data);
static void add_sound_to_pool(GtkWidget *combo, gpointer data);
static void stop_sounds(GtkWidget *button, gpointer *sounds);
static void clear_pool(GtkWidget *button, gpointer data);
static gint load_sounds_array();
static void load_sounds_combo(GtkWidget *combo);
static void play_sound(GtkWidget *button, gpointer *sounds);
static void sound_pipeline(struct s_pipeline *p1);
static gboolean bus_call(GstBus *bus, GstMessage *msg, struct s_pipeline *p1);
static void on_pad_added(GstElement *element, GstPad *pad, struct s_pipeline *p1);
static gboolean draw_pool(GtkWidget *widget, cairo_t *cr, gpointer data);
//About dialog and icon.
static void about_dialog(GtkWidget *widget, gpointer data);
static GdkPixbuf* draw_icon();

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);
   gst_init(&argc, &argv);
   g_mutex_init(&mutex);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Circular Gradient Clock");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
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

   //Draw the program icon.
   GdkPixbuf *icon=draw_icon();
   gtk_window_set_default_icon(icon);

   //The found .ogg file names
   ogg_files=g_ptr_array_new_full(10, g_free);
   //Index of sounds to play.
   play_index=g_array_new(FALSE, FALSE, sizeof(gint));

   //Load the .ogg files into the combobox from the local file to start with.
   num_sounds=load_sounds_array();

   //Set up the pipeline structs for the sounds.
   gint i=0;
   g_print("Sounds %i\n", num_sounds);
   gpointer sounds[num_sounds];
   for(i=0;i<num_sounds;i++)
     {
       sounds[i]=g_new(struct s_pipeline, 1);
       ((struct s_pipeline *)(sounds[i]))->array_index=i;
       ((struct s_pipeline *)(sounds[i]))->pipeline=NULL;
     } 

   //A pool for playing the sounds all at once.
   pool=gst_task_pool_new();
   gst_task_pool_prepare(pool, NULL);

   da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   gtk_widget_set_events(da, GDK_BUTTON_PRESS_MASK);
   g_signal_connect(da, "button_press_event", G_CALLBACK(click_rooster_drawing), sounds); 
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *menu1=gtk_menu_new();
   GtkWidget *menu1item1=gtk_menu_item_new_with_label("Set Alarm");
   gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
   GtkWidget *title1=gtk_menu_item_new_with_label("Settings");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);

   GtkWidget *menu2=gtk_menu_new();
   GtkWidget *menu2item1=gtk_menu_item_new_with_label("Circular Gradient Clock");
   gtk_menu_shell_append(GTK_MENU_SHELL(menu2), menu2item1);
   GtkWidget *title2=gtk_menu_item_new_with_label("About");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(title2), menu2);

   GtkWidget *menu_bar=gtk_menu_bar_new();
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title2);

   g_signal_connect(menu1item1, "activate", G_CALLBACK(set_alarm_dialog), sounds);
   g_signal_connect(menu2item1, "activate", G_CALLBACK(about_dialog), NULL);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   g_timeout_add_seconds(5, (GSourceFunc)time_redraw, sounds);

   gtk_widget_show_all(window);

   gtk_main();

   g_object_unref(icon);
   gst_object_unref(pool);
   g_mutex_clear(&mutex);
   g_ptr_array_free(ogg_files, TRUE);
   for(i=0;i<num_sounds;i++)
     {
       g_free(sounds[i]);
     }
   g_array_free(play_index, TRUE); 

   return 0;  
 }
static gboolean time_redraw(gpointer sounds)
 {
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gint hour=g_date_time_get_hour(date_time);
   gint minute=g_date_time_get_minute(date_time);
   if(hour>12&&!alarm_am) hour=hour-12;
   g_print("Time %i %i Alarm %i %i %i Set %i\n", hour, minute, alarm_hour, alarm_minute, (gint)alarm_am, (gint)alarm_set);
   if(alarm_set&&alarm_hour==hour&&alarm_minute==minute&&!block_alarm)
     {
       play_sound(NULL, sounds);
       block_alarm=TRUE;
     };
   if(minute!=alarm_minute) block_alarm=FALSE;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);

   gtk_widget_queue_draw(da);
   return TRUE;
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   draw_clock(cr, width, height);
   return FALSE;
 }
static void draw_clock(cairo_t *cr, gdouble width, gdouble height)
 {
   /*
     From http://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves
     So for 4 points it is (4/3)*tan(pi/8) = 4*(sqrt(2)-1)/3 = 0.552284749831
   */
   gdouble points=0.552284749831;

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Transparent background.
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);

   //Blue outside ring.
   cairo_save(cr);
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 8.3*w1/2.0, 8.3*h1/2.0);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_fill(cr);
   cairo_restore(cr);

   //Gray inside.
   cairo_save(cr);
   cairo_set_source_rgb(cr, 0.8, 0.8, 1.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 8.0*w1/2.0, 8.0*h1/2.0);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_fill(cr);
   cairo_restore(cr);

   //Color circle pyramid.
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_set_line_width(cr, 4.0);
   //purple
   cairo_arc(cr, 5.0*w1, 2.95*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //yellow
   cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
   cairo_arc(cr, 4.85*w1, 3.25*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //cyan
   cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
   cairo_arc(cr, 5.15*w1, 3.25*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //red
   cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
   cairo_arc(cr, 4.70*w1, 3.55*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //green
   cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
   cairo_arc(cr, 5.0*w1, 3.55*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //blue
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_arc(cr, 5.3*w1, 3.55*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   
   //Quadrant 1 arc.
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, (5.0*w1)+(points*4.0*width/10), 1.0*h1,  9.0*w1,  (5.0*h1)-(points*4.0*height/10), 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 8.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 8.0*w1, (5.0*h1)-(points*3.0*height/10),  (5.0*w1)+(points*3.0*w1), 2.0*h1, 5.0*w1, 2.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch(pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);

   //Quadrant 2 arc.
   cairo_pattern_t *pattern2=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern2);
   cairo_mesh_pattern_move_to(pattern2, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 1.0*w1, (5.0*h1)-(points*4.0*height/10), (5.0*w1)-(points*4.0*width/10), 1.0*h1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 5.0*w1, 2.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, (5.0*w1)-(points*3.0*width/10), 2.0*h1,  2.0*w1, (5.0*h1)-(points*3.0*h1), 2.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 2, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 3, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern2);
   cairo_set_source(cr, pattern2);
   cairo_paint(cr);

   //Quadrant 3 arc.
   cairo_pattern_t *pattern3=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern3);
   cairo_mesh_pattern_move_to(pattern3, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, (5.0*w1)-(points*4.0*width/10), 9.0*h1, 1.0*w1, (5.0*h1)+(points*4.0*height/10), 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 2.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 2.0*w1, (5.0*h1)+(points*3.0*height/10),  (5.0*w1)-(points*3.0*width/10), 8.0*h1, 5.0*w1, 8.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 0, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 1, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 2, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch (pattern3);
   cairo_set_source(cr, pattern3);
   cairo_paint(cr);

   //Quadrant 4 arc.
   cairo_pattern_t *pattern4=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern4);
   cairo_mesh_pattern_move_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 9.0*w1, (5.0*h1)+(points*4.0*height/10), (5.0*w1)+(points*4.0*width/10),  9.0*h1, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 5.0*w1, 8.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, (5.0*w1)+(points*3.0*width/10), 8.0*h1,  8.0*w1, (5.0*h1)+(points*3.0*height/10), 8.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 0, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 1, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 2, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 3, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern4);
   cairo_set_source(cr, pattern4);
   cairo_paint(cr);
   
   cairo_pattern_destroy(pattern1);
   cairo_pattern_destroy(pattern2);
   cairo_pattern_destroy(pattern3);
   cairo_pattern_destroy(pattern4);

   //Set the clock text.
   cairo_save(cr);
   gint i=0;
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CGC"};
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   //Start at 12 or radius with just the y component.
   gdouble hour_radius=3.5*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 20);
   cairo_move_to(cr, 0.0, 0.0);
   cairo_translate(cr, width/2.0, height/2.0);
   //Color 12 with blue for contrast with yellow. Other numbers are white.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
       cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
     }

   //Put the clock name on the clock.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_text_extents(cr, hours[12], &tick_extents);
   cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
   cairo_show_text(cr, hours[12]);

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   hour=hour+minute/60.0;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   if(hour>12) hour=hour-12;

   //Hour hand.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=((1.75*w1)*(1.75*h1))/sqrt(((1.75*w1)*(1.75*w1)*temp_sin*temp_sin) + ((1.75*h1)*(1.75*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=((2.75*w1)*(2.75*h1))/sqrt(((2.75*w1)*(2.75*w1)*temp_sin*temp_sin) + ((2.75*h1)*(2.75*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);
   cairo_restore(cr);

   if(g_atomic_int_get(&sounds_left)>0)
     {
       cairo_scale(cr, 0.25, 0.25);
       cairo_translate(cr, 31.0*w1, 31.0*h1);
       draw_rooster(cr, width, height);
     }
 }
static void draw_rooster(cairo_t *cr, gdouble width, gdouble height)
  {
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Draw the rooster body.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_move_to(cr, 8.0*w1, 3.0*h1);
    cairo_curve_to(cr, 9.0*w1, 4.0*h1, 9.0*w1, 7.5*h1, 5.0*w1, 7.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 4.0*w1, 7.0*h1, 3.0*w1, 6.0*h1, 3.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.0*w1, 4.0*h1, 2.0*w1, 4.0*h1, 2.0*w1, 4.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 3.5*h1, 2.5*w1, 3.25*h1, 2.75*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 3.0*h1, 1.5*w1, 3.25*h1, 1.5*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 1.5*w1, 2.5*h1, 2.0*w1, 2.5*h1, 2.5*w1, 3.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 2.0*h1, 1.5*w1, 2.0*h1, 1.0*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 1.5*w1, 1.0*h1, 2.5*w1, 1.0*h1, 4.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.0*w1, 5.0*h1, 6.0*w1, 4.0*h1, 6.5*w1, 2.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.5*w1, 2.5*h1, 5.5*w1, 2.25*h1, 6.5*w1, 2.25*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.75*w1, 2.0*h1, 5.75*w1, 1.75*h1, 6.75*w1, 2.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.25*w1, 1.5*h1, 6.25*w1, 1.25*h1, 7.0*w1, 1.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.0*w1, 1.0*h1, 7.25*w1, 1.0*h1, 7.5*w1, 1.8*h1);
    cairo_stroke_preserve(cr);    
    cairo_curve_to(cr, 7.7*w1, 1.6*h1, 8.0*w1, 1.4*h1, 8.5*w1, 1.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.0*w1, 1.6*h1, 7.7*w1, 1.8*h1, 7.75*w1, 2.0*h1);
    cairo_stroke_preserve(cr);

    cairo_curve_to(cr, 7.7*w1, 2.0*h1, 8.0*w1, 1.8*h1, 8.5*w1, 2.15*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.25*w1, 2.15*h1, 8.15*w1, 2.05*h1, 7.85*w1, 2.15*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.4*w1, 2.5*h1, 8.5*w1, 2.7*h1, 8.0*w1, 3.0*h1);
    cairo_close_path(cr);
    cairo_fill(cr);

    //First leg and foot.
    cairo_move_to(cr, 5.0*w1, 7.0*h1);
    cairo_line_to(cr, 5.5*w1, 8.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 5.0*w1, 8.5*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5*w1, 8.5*h1);
    cairo_line_to(cr, 6.0*w1, 8.3*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5*w1, 8.5*h1);
    cairo_line_to(cr, 6.2*w1, 8.6*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5*w1, 8.5*h1);
    cairo_line_to(cr, 6.0*w1, 8.9*h1);
    cairo_stroke(cr);
    //Second leg and foot.
    cairo_move_to(cr, 5.5*w1, 7.0*h1);
    cairo_line_to(cr, 6.75*w1, 8.25*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 6.25*w1, 8.25*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.75*w1, 8.25*h1);
    cairo_line_to(cr, 7.25*w1, 8.05*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.75*w1, 8.25*h1);
    cairo_line_to(cr, 7.45*w1, 8.35*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.75*w1, 8.25*h1);
    cairo_line_to(cr, 7.25*w1, 8.65*h1);
    cairo_stroke(cr);
    
    //Draw the eye.
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_arc(cr, 7.25*w1, 1.85*h1, 0.07*h1, 0.0, 2.0*G_PI);
    cairo_fill(cr);  
  }
static gboolean click_rooster_drawing(GtkWidget *widget, GdkEvent *event, gpointer sounds)
{
  gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
  gdouble height=(gdouble)gtk_widget_get_allocated_width(widget);
  
  if((event->button.x)>8.0*width/10.0&&(event->button.y)>8.0*height/10.0) 
    {
      g_print("Rooster Clicked\n");
      if(g_atomic_int_get(&sounds_left)>0)
        {
          stop_sounds(NULL, sounds);
          gtk_widget_queue_draw(widget);
        }
    }
  
  return FALSE;
}
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }
static void set_alarm_dialog(GtkWidget *widget, gpointer *sounds)
 {    
   GtkWidget *dialog=gtk_dialog_new_with_buttons("Clock Alarm", GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, "OK", GTK_RESPONSE_NONE, NULL);
   gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 600);
   GtkWidget *content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   g_signal_connect(dialog, "response", G_CALLBACK(close_dialog), NULL);

   gtk_widget_set_app_paintable(dialog, TRUE);
   //Try to set transparency.
   if(gtk_widget_is_composited(dialog))
     {
       GdkScreen *screen=gtk_widget_get_screen(dialog);  
       GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
       gtk_widget_set_visual(dialog, visual);
     }
   else g_print("Can't set window transparency.\n");
   g_signal_connect(dialog, "draw", G_CALLBACK(draw_background_dialog), NULL);

   G_GNUC_BEGIN_IGNORE_DEPRECATIONS 
   GtkWidget *action=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
   G_GNUC_END_IGNORE_DEPRECATIONS
   g_object_set(G_OBJECT(action), "halign", GTK_ALIGN_CENTER, NULL);

   GtkWidget *combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);

   //Load the .ogg files into the combobox from the local file to start with.
   load_sounds_combo(combo1);

   //button1 is global.
   button1=gtk_button_new_with_label("Play Sound Pool");
   if(g_atomic_int_get(&sounds_left)==0) gtk_widget_set_sensitive(button1, TRUE);
   else gtk_widget_set_sensitive(button1, FALSE);
   gtk_widget_set_hexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(play_sound), sounds);

   GtkWidget *label1=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='yellow' size='x-large'>Add Sounds to Pool</span>");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *label2=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label2), "<span foreground='yellow' size='x-large'>Sound Pool</span>");
   gtk_widget_set_hexpand(label2, TRUE);

   GtkWidget *label3=gtk_label_new("");
   if(pool_string!=NULL) gtk_label_set_markup(GTK_LABEL(label3), pool_string);
   gtk_widget_set_hexpand(label3, TRUE);
   gtk_widget_set_vexpand(label3, TRUE);

   g_signal_connect(combo1, "changed", G_CALLBACK(add_sound_to_pool), label3);

   GtkWidget *event_box=gtk_event_box_new();
   gtk_widget_set_hexpand(event_box, TRUE);
   gtk_widget_set_vexpand(event_box, TRUE);
   gtk_container_add(GTK_CONTAINER(event_box), label3);
   g_signal_connect(event_box, "draw", G_CALLBACK(draw_pool), NULL);

   GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
   gtk_widget_set_hexpand(scroll, TRUE);
   gtk_widget_set_vexpand(scroll, TRUE);
   gtk_container_add(GTK_CONTAINER(scroll), event_box);

   GtkWidget *button2=gtk_button_new_with_label("Stop Sound Pool");
   gtk_widget_set_hexpand(button2, TRUE);
   g_signal_connect(button2, "clicked", G_CALLBACK(stop_sounds), sounds);

   GtkWidget *button3=gtk_button_new_with_label("Clear Sound Pool");
   gtk_widget_set_hexpand(button3, TRUE);
   g_signal_connect(button3, "clicked", G_CALLBACK(clear_pool), label3);

   GtkWidget *label4=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label4), "<span foreground='yellow' size='x-large'>Alarm Time</span>");
   gtk_widget_set_hexpand(label4, TRUE);
  
   GtkWidget *label5=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label5), "<span foreground='yellow' size='x-large'>Hour</span>");

   GtkAdjustment *adj1=gtk_adjustment_new(1.0, 1.0, 12.0, 1.0, 0.0, 0.0);
   GtkWidget *spin1=gtk_spin_button_new(adj1, 1.0, 0);

   GtkWidget *label6=gtk_label_new("Minute");
   gtk_label_set_markup(GTK_LABEL(label6), "<span foreground='yellow' size='x-large'>Minute</span>");

   GtkAdjustment *adj2=gtk_adjustment_new(1.0, 1.0, 60.0, 1.0, 0.0, 0.0);
   GtkWidget *spin2=gtk_spin_button_new(adj2, 1.0, 0);

   GtkWidget *check1=gtk_check_button_new_with_label("");
   GtkWidget *check1_label=gtk_bin_get_child(GTK_BIN(check1));
   gtk_label_set_markup(GTK_LABEL(check1_label), "<span foreground='yellow' size='x-large'>AM</span>");

   GtkWidget *check2=gtk_check_button_new_with_label("Set Alarm");
   GtkWidget *check2_label=gtk_bin_get_child(GTK_BIN(check2));
   gtk_label_set_markup(GTK_LABEL(check2_label), "<span foreground='yellow' size='x-large'> Set Alarm</span>");
   gtk_widget_set_hexpand(check2, TRUE);
   gtk_widget_set_halign(check2, GTK_ALIGN_CENTER);
   g_signal_connect(check2, "clicked", G_CALLBACK(set_alarm_active), NULL);

   //Update the alarm when the spin buttons or am/pm is clicked.
   g_signal_connect(spin1, "value-changed", G_CALLBACK(set_alarm_spin1), NULL);
   g_signal_connect(spin2, "value-changed", G_CALLBACK(set_alarm_spin2), NULL);
   g_signal_connect(check1, "clicked", G_CALLBACK(set_alarm_check), NULL);
 
   GtkWidget *grid=gtk_grid_new();
   gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), label2, 0, 2, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), scroll, 0, 3, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 4, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 5, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), button3, 0, 6, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), label4, 0, 7, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), label5, 0, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), spin1, 1, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label6, 2, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), spin2, 3, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), check1, 4, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), check2, 0, 9, 5, 1);

   gtk_container_add(GTK_CONTAINER(content_area), grid);

   dialog_active=TRUE;
   gtk_widget_show_all(dialog);
 }
static void close_dialog(GtkDialog *dialog, gint response_id, gpointer data)
 {
   dialog_active=FALSE;
   gtk_widget_destroy(GTK_WIDGET(dialog));
 }
static gboolean draw_background_dialog(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   cairo_set_source_rgba(cr, 0.0, 0.7, 1.0, 0.9);
   cairo_paint(cr);
   return FALSE;
 }
static void set_alarm_active(GtkWidget *widget, gpointer data)
 {
   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) alarm_set=TRUE;
   else alarm_set=FALSE;     
 }
static void set_alarm_spin1(GtkSpinButton *spin_button, gpointer data)
 {
   gint spin1=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_button));
   alarm_hour=spin1;
   block_alarm=FALSE;
 }
static void set_alarm_spin2(GtkSpinButton *spin_button, gpointer data)
 {
   gint spin2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_button));
   alarm_minute=spin2;
   block_alarm=FALSE;
 }
static void set_alarm_check(GtkWidget *widget, gpointer data)
 {
   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget))) alarm_am=TRUE;
   else alarm_am=FALSE;
   block_alarm=FALSE; 
 }
static void add_sound_to_pool(GtkWidget *combo, gpointer data)
  {
    gint index=gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
    gint i=0;
    gboolean add_sound=TRUE;
    gint length=play_index->len;
   
    //Can only have one instance of the sound in the pool.
    for(i=0;i<length;i++)
      {
        if(index==g_array_index(play_index, gint, i)) add_sound=FALSE;
      }

    if(add_sound)
      {
        g_array_append_val(play_index, index);
        gchar *file_name=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
        gchar *label_string=g_strdup_printf("%s%s\n", gtk_label_get_text(GTK_LABEL(data)), file_name);
        gchar *label_string2=g_strdup_printf("<span foreground='yellow'>%s</span>", label_string);
        gtk_label_set_markup(GTK_LABEL(data), label_string2);
        if(pool_string!=NULL) g_free(pool_string);
        pool_string=g_strdup(label_string2);
        g_free(label_string);
        g_free(label_string2);
        g_free(file_name);
      }
    else g_print("The sound is already in the pool.\n");
  }
static void stop_sounds(GtkWidget *button, gpointer *sounds)
  {
    gint i=0;
    g_mutex_lock(&mutex);
    for(i=0;i<num_sounds;i++)
      {
        if(((struct s_pipeline *)(sounds[i]))->pipeline!=NULL)
          {
            g_print("Stop Pipeline %i\n", i);
            gst_element_set_state(((struct s_pipeline *)(sounds[i]))->pipeline, GST_STATE_PAUSED);
            gst_object_unref(GST_OBJECT(((struct s_pipeline *)(sounds[i]))->pipeline));
            ((struct s_pipeline *)(sounds[i]))->pipeline=NULL;
            g_source_remove(((struct s_pipeline *)(sounds[i]))->bus_watch_id);
            sounds_left--;
            gst_task_pool_join(pool, ((struct s_pipeline *)(sounds[i]))->pool_id);
          }
        if(button1!=NULL) gtk_widget_set_sensitive(button1, TRUE);
      }
    g_mutex_unlock(&mutex);
  }
static void clear_pool(GtkWidget *button, gpointer data)
  {
    gtk_label_set_text(GTK_LABEL(data), "");
    gint length=play_index->len;
    g_array_remove_range(play_index, 0, length);
    if(pool_string!=NULL)
      {
        g_free(pool_string);
        pool_string=NULL;
      }
  }
static gint load_sounds_array()
  {
    GError *dir_error=NULL;
    const gchar *file_temp=NULL;
    gchar *file_type=g_strdup(".ogg");
    gint i=0;

    GDir *directory=g_dir_open("./", 0, &dir_error);
    if(dir_error!=NULL)
      {
        g_print("dir Error %s\n", dir_error->message);
        g_error_free(dir_error);
      }
    else
      {
        file_temp=g_dir_read_name(directory);
        while(file_temp!=NULL)
          {
            if(file_temp!=NULL&&g_str_has_suffix(file_temp, file_type))
              {
                g_print("%s\n", file_temp);
                g_ptr_array_add(ogg_files, g_strdup(file_temp));
                i++;
              }
            file_temp=g_dir_read_name(directory);
          }
        g_dir_close(directory);
      }

    if(file_type!=NULL) g_free(file_type);

    return i;
  }
static void load_sounds_combo(GtkWidget *combo)
  {
    gint i=0;
    gint length=ogg_files->len;

    if(length==0)
      {
        g_print("There are no .ogg files for the combobox.\n");
        gtk_widget_set_sensitive(combo, FALSE);
      }
    else
      {
        for(i=0;i<length;i++)
          {
            gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), (gchar*)g_ptr_array_index(ogg_files, i));
          }
        gtk_widget_set_sensitive(combo, TRUE);
      }

  }
static void play_sound(GtkWidget *button, gpointer *sounds)
  { 
    gint i=0;
    static gint id=0;
    GError *error=NULL;

    //Start sound threads in pool.
    gint length=play_index->len;
    sounds_left=length;
    if(sounds_left!=0&&dialog_active) gtk_widget_set_sensitive(button, FALSE);

    if(sounds_left>0) gtk_widget_queue_draw(da);

    for(i=0;i<length;i++)
      {
        ((struct s_pipeline *)(sounds[g_array_index(play_index, gint, i)]))->pipeline_id=id;
        ((struct s_pipeline *)(sounds[g_array_index(play_index, gint, i)]))->pool_id=gst_task_pool_push(pool, (GstTaskPoolFunction)sound_pipeline, (struct s_pipeline *)sounds[g_array_index(play_index, gint, i)], &error);
        if(error!=NULL) g_print("Error: %s\n", error->message);
        id++; 
      }
           
    if(error!=NULL) g_error_free(error);
  }
static void sound_pipeline(struct s_pipeline *p1)
  {
    gchar *s0=g_strdup_printf("audio-player%i_%i", p1->pipeline_id, p1->array_index);
    gchar *s1=g_strdup_printf("file-source%i_%i", p1->pipeline_id, p1->array_index);
    gchar *s2=g_strdup_printf("ogg-demuxer%i_%i", p1->pipeline_id, p1->array_index);
    gchar *s3=g_strdup_printf("vorbis-decoder%i_%i", p1->pipeline_id, p1->array_index);
    gchar *s4=g_strdup_printf("converter%i_%i", p1->pipeline_id, p1->array_index);
    gchar *s5=g_strdup_printf("audio-output%i_%i", p1->pipeline_id, p1->array_index);

    p1->pipeline=gst_pipeline_new(s0);
    GstElement *source=gst_element_factory_make("filesrc", s1);
    GstElement *demuxer=gst_element_factory_make("oggdemux", s2);
    p1->decoder=gst_element_factory_make("vorbisdec", s3);
    GstElement *conv=gst_element_factory_make("audioconvert", s4);
    GstElement *sink=gst_element_factory_make("autoaudiosink", s5);

    g_print("Pipeline %s started\n", s0);
    g_free(s0);
    g_free(s1);
    g_free(s2);
    g_free(s3);
    g_free(s4);
    g_free(s5);

    if(!p1->pipeline || !source || !demuxer || !p1->decoder || !conv || !sink)
      {
        g_print("Error, One element could not be created.\n");
      }

    g_mutex_lock(&mutex);
    g_object_set(G_OBJECT(source), "location", (gchar*)g_ptr_array_index(ogg_files, p1->array_index), NULL);
    g_mutex_unlock(&mutex);

    GstBus *bus=gst_pipeline_get_bus(GST_PIPELINE(p1->pipeline));
    p1->bus_watch_id=gst_bus_add_watch(bus, (GstBusFunc)bus_call, p1);
    gst_object_unref(bus);

    gst_bin_add_many(GST_BIN(p1->pipeline), source, demuxer, p1->decoder, conv, sink, NULL);

    gst_element_link(source, demuxer);
    gst_element_link_many(p1->decoder, conv, sink, NULL);
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), p1);

    gst_element_set_state(p1->pipeline, GST_STATE_PLAYING);
  }
static gboolean bus_call(GstBus *bus, GstMessage *msg, struct s_pipeline *p1)
 {
   GError *error=NULL;
   //g_print("%s\n", gst_message_type_get_name(GST_MESSAGE_TYPE(msg)));
   switch(GST_MESSAGE_TYPE(msg))
    {
      case GST_MESSAGE_EOS:
        g_mutex_lock(&mutex);
        g_print("%s Done\n", (gchar*)g_ptr_array_index(ogg_files, p1->array_index));
        gst_element_set_state(p1->pipeline, GST_STATE_NULL);
        //Unreference the pipeline and other element objects stored in the pipeline.
        gst_object_unref(GST_OBJECT(p1->pipeline));
        p1->pipeline=NULL;
        g_source_remove(p1->bus_watch_id);
        sounds_left--;
        gst_task_pool_join(pool, p1->pool_id);
        g_mutex_unlock(&mutex);
        break;
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &error, NULL);
        g_print("Error: %s\n", error->message);
        g_error_free(error);
        break;
      default:
        break;
    }
   
    if(sounds_left==0&&dialog_active) gtk_widget_set_sensitive(button1, TRUE);
    return TRUE;
  }
static void on_pad_added(GstElement *element, GstPad *pad, struct s_pipeline *p1)
  {
    GstPad *sinkpad;  
    sinkpad=gst_element_get_static_pad(p1->decoder, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);   
  }
static gboolean draw_pool(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);
    return FALSE;
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(window));
    gtk_widget_set_app_paintable(dialog, TRUE);
    //Try to set transparency.
    if(gtk_widget_is_composited(dialog))
      {
        GdkScreen *screen=gtk_widget_get_screen(dialog);  
        GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(dialog, visual);
      }
    else g_print("Can't set window transparency.\n");
    g_signal_connect(dialog, "draw", G_CALLBACK(draw_background_dialog), NULL);

    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Circular Gradient Clock");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A colorful clock with a sound pool alarm.");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2017 C. Eric Cashon");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
//The diving and swimming pool notes.
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    
    //Paint the background.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    //The diving note.
    cairo_save(cr);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_translate(cr, 60, 60);
    cairo_rotate(cr, 3*G_PI/4);
    cairo_scale(cr, 1.0, 0.60);
    cairo_arc(cr, 0, 0, 30, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);

    //The diving note line.
    cairo_save(cr);
    cairo_set_line_width(cr, 6);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); 
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 80, 40);
    cairo_line_to(cr, 170, 130);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Two leg lines on diving note line.
    cairo_save(cr);
    cairo_set_line_width(cr, 6);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);  
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 170, 130);
    cairo_line_to(cr, 170, 90);
    cairo_stroke(cr); 
    cairo_move_to(cr, 150, 110);
    cairo_line_to(cr, 150, 70);
    cairo_stroke(cr);
    cairo_restore(cr);

    //The pool arc.
    cairo_save(cr);
    cairo_set_line_width(cr, 6);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.5);
    cairo_arc(cr, 128, 380, 240, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);

    //A note in the pool.
    cairo_save(cr);
    cairo_set_line_width(cr, 1);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_translate(cr, 70, 230);
    cairo_rotate(cr, G_PI);
    cairo_scale(cr, 0.60, 1.0);
    cairo_arc(cr, 0, 0, 15, 0, G_PI);
    cairo_fill(cr);
    cairo_stroke_preserve(cr);
    cairo_restore(cr);

    //Note flag legs sticking up in pool.
    cairo_save(cr);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 100, 180);
    cairo_line_to(cr, 105, 170);
    cairo_stroke(cr);
    cairo_move_to(cr, 105, 180);
    cairo_line_to(cr, 110, 170);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Note flag in pool.
    cairo_save(cr);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_move_to(cr, 50, 190);
    cairo_line_to(cr, 50, 170);
    cairo_stroke(cr);
    cairo_move_to(cr, 50, 170);
    cairo_line_to(cr, 43, 177);
    cairo_stroke(cr);
    cairo_move_to(cr, 50, 175);
    cairo_line_to(cr, 43, 182);
    cairo_stroke(cr);
    cairo_restore(cr);

    //A green border.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, 256, 256);
    cairo_stroke(cr);

    //The diving board.
    cairo_save(cr);
    cairo_matrix_t matrix;
    cairo_matrix_init(&matrix, 1.0, 0.5, 0.0, 1.0, 0.0, 0.0);
    cairo_transform(cr, &matrix);
    cairo_rectangle(cr, 148, 100, 140, 50);
    cairo_stroke_preserve(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    //Draw a jumping fish.
    cairo_save(cr);
    gdouble w1=256.0/10.0;
    gdouble h1=256.0/10.0;
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_set_line_width(cr, 2.0);
    cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_rotate(cr, -G_PI/6.0);
    cairo_scale(cr, 0.20, 0.12);
    cairo_translate(cr, 15.0*w1, 75.0*h1);
    cairo_move_to(cr, 1.0*w1, 3.0*h1);
    cairo_curve_to(cr, 2.0*w1, 3.0*h1, 2.0*w1, 3.0*h1, 3.0*w1, 4.5*h1);
    cairo_stroke_preserve(cr);  
    cairo_curve_to(cr, 4.0*w1, 2.0*h1, 4.0*w1, 2.0*h1, 6.0*w1, 2.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.0*w1, 2.0*h1, 8.0*w1, 2.0*h1, 9.0*w1, 5.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.0*w1, 8.0*h1, 8.0*w1, 8.0*h1, 6.0*w1, 8.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 4.0*w1, 8.0*h1, 4.0*w1, 8.0*h1, 3.0*w1, 5.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 7.0*h1, 2.0*w1, 7.0*h1, 1.0*w1, 7.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 6.0*h1, 2.0*w1, 6.0*h1, 2.0*w1, 5.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 4.0*h1, 2.0*w1, 4.0*h1, 1.0*w1, 3.0*h1);
    cairo_stroke_preserve(cr);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    //The clock.
    cairo_translate(cr, 165, 0);
    cairo_scale(cr, 0.35, 0.35);
    draw_clock(cr, 256.0, 256.0);

    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface, 0, 0, 256, 256);

    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    return icon;
  }










