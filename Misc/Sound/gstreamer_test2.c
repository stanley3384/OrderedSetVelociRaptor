
/*
   Test code for pooling sounds in gstreamer. Add sounds to the pool and then play. All
the sounds will be played at once. Working towards an alarm for circular_gradient_clock1.c.

   The program needs some short ogg sound files in the same folder as the program. Used
test ogg files from the following.

   http://rpg.hamsterrepublic.com/ohrrpgce/Free_Sound_Effects

   Ubuntu16.04 with GTK3.18

   gcc -Wall gstreamer_test2.c -o gstreamer_test2 `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<gst/gst.h>

//The sound files to play put in an array.
static GPtrArray *ogg_files;
static GArray *play_index;
static GMutex mutex;
static gint sounds_left=0;

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
static GtkWidget *button1; 

static void add_sound_to_pool(GtkWidget *combo, gpointer data);
static void clear_pool(GtkWidget *button, gpointer data);
static gint load_sounds(GtkWidget *combo, gpointer data);
static void play_sound(GtkWidget *button, gpointer *sounds);
static void sound_pipeline(struct s_pipeline *p1);
static gboolean bus_call(GstBus *bus, GstMessage *msg, struct s_pipeline *p1);
static void on_pad_added(GstElement *element, GstPad *pad, struct s_pipeline *p1);
static gboolean draw_pool(GtkWidget *widget, cairo_t *cr, gpointer data);
  
int main(int argc, char *argv[])
 {
   gtk_init(&argc, &argv);
   gst_init(&argc, &argv);
   g_mutex_init(&mutex);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Clock Alarm");
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   //The found .ogg file names
   ogg_files=g_ptr_array_new_full(10, g_free);
   //Index of sounds to play.
   play_index=g_array_new(FALSE, FALSE, sizeof(gint));

   GtkWidget *combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);

   //Load the .ogg files into the combobox from the local file to start with.
   gint num_sounds=load_sounds(combo1, NULL);

   //Set up the pipeline structs for the sounds.
   gint i=0;
   g_print("Sounds %i\n", num_sounds);
   gpointer sounds[num_sounds];
   for(i=0;i<num_sounds;i++)
     {
       sounds[i]=g_new(struct s_pipeline, 1);
       ((struct s_pipeline *)(sounds[i]))->array_index=i;
     } 

   //A pool for playing the sounds all at once.
   pool=gst_task_pool_new();
   gst_task_pool_prepare(pool, NULL);

   button1=gtk_button_new_with_label("Play Sound Pool");
   gtk_widget_set_hexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(play_sound), sounds);

   GtkWidget *label1=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='Blue' size='x-large'>Add Sounds to Pool</span>");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *label2=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label2), "<span foreground='Blue' size='x-large'>Sound Pool</span>");
   gtk_widget_set_hexpand(label2, TRUE);

   GtkWidget *label3=gtk_label_new("");
   gtk_widget_set_hexpand(label3, TRUE);
   gtk_widget_set_vexpand(label3, TRUE);

   g_signal_connect(combo1, "changed", G_CALLBACK(add_sound_to_pool), label3);

   GtkWidget *event_box=gtk_event_box_new();
   gtk_widget_set_hexpand(event_box, TRUE);
   gtk_widget_set_vexpand(event_box, TRUE);
   gtk_container_add(GTK_CONTAINER(event_box), label3);
   g_signal_connect(event_box, "draw", G_CALLBACK(draw_pool), NULL);

   GtkWidget *button2=gtk_button_new_with_label("Clear Sound Pool");
   gtk_widget_set_hexpand(button2, TRUE);
   g_signal_connect(button2, "clicked", G_CALLBACK(clear_pool), label3);

   GtkWidget *label4=gtk_label_new("");
   gtk_label_set_markup(GTK_LABEL(label4), "<span foreground='Blue' size='x-large'>Alarm Time</span>");
   gtk_widget_set_hexpand(label4, TRUE);
  
   GtkWidget *label5=gtk_label_new("Hour");

   GtkAdjustment *adj1=gtk_adjustment_new(1.0, 1.0, 12.0, 1.0, 0.0, 0.0);
   GtkWidget *spin1=gtk_spin_button_new(adj1, 1.0, 0);

   GtkWidget *label6=gtk_label_new("Minute");

   GtkAdjustment *adj2=gtk_adjustment_new(1.0, 1.0, 60.0, 1.0, 0.0, 0.0);
   GtkWidget *spin2=gtk_spin_button_new(adj2, 1.0, 0);

   GtkWidget *check1=gtk_check_button_new_with_label("AM");

   GtkWidget *check2=gtk_check_button_new_with_label("Set Alarm");
   GtkWidget *check_label=gtk_bin_get_child(GTK_BIN(check2));
   gtk_label_set_markup(GTK_LABEL(check_label), "<span foreground='purple' size='x-large'> Set Alarm</span>");
   gtk_widget_set_hexpand(check2, TRUE);
   gtk_widget_set_halign(check2, GTK_ALIGN_CENTER);
 
   GtkWidget *grid=gtk_grid_new();
   gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), label2, 0, 2, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), event_box, 0, 3, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 4, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 5, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), label4, 0, 6, 5, 1);
   gtk_grid_attach(GTK_GRID(grid), label5, 0, 7, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), spin1, 1, 7, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label6, 2, 7, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), spin2, 3, 7, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), check1, 4, 7, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), check2, 0, 8, 5, 1);

   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

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
        gtk_label_set_text(GTK_LABEL(data), label_string);
        g_free(label_string);
        g_free(file_name);
      }
    else g_print("The sound is already in the pool.\n");
  }
static void clear_pool(GtkWidget *button, gpointer data)
  {
    gtk_label_set_text(GTK_LABEL(data), "");
    gint length=play_index->len;
    g_array_remove_range(play_index, 0, length);
  }
static gint load_sounds(GtkWidget *combo, gpointer data)
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
        gtk_widget_set_sensitive(combo, FALSE);
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
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), file_temp);
                i++;
              }
            file_temp=g_dir_read_name(directory);
          }
        if(i>0)
          {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
            gtk_widget_set_sensitive(combo, TRUE);
          }
        else gtk_widget_set_sensitive(combo, FALSE);
        g_dir_close(directory);
      }

    if(file_type!=NULL) g_free(file_type);

    return i;
  }
static void play_sound(GtkWidget *button, gpointer *sounds)
  { 
    gint i=0;
    static gint id=0;
    GError *error=NULL;

    //Start sound threads in pool.
    gint length=play_index->len;
    sounds_left=length;
    if(sounds_left!=0) gtk_widget_set_sensitive(button, FALSE);
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
    GstElement *sink=gst_element_factory_make ("autoaudiosink", s5);

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
   switch(GST_MESSAGE_TYPE(msg))
    {
      case GST_MESSAGE_EOS:
        g_mutex_lock(&mutex);
        g_print("%s Done\n", (gchar*)g_ptr_array_index(ogg_files, p1->array_index));
        g_mutex_unlock(&mutex);
        gst_element_set_state(p1->pipeline, GST_STATE_NULL);
        //Unreference the pipeline and other element objects stored in the pipeline.
        gst_object_unref(GST_OBJECT(p1->pipeline));
        g_source_remove(p1->bus_watch_id);
        sounds_left--;
        gst_task_pool_join(pool, p1->pool_id);
        break;
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &error, NULL);
        g_print("Error: %s\n", error->message);
        g_error_free(error);
        break;
      default:
        break;
    }
   
    if(sounds_left==0) gtk_widget_set_sensitive(button1, TRUE);
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
    cairo_set_source_rgb(cr, 0.0, 0.7, 1.0);
    cairo_paint(cr);
    return FALSE;
  }


