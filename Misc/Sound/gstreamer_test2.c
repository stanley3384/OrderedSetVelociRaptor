
/*
   Test code for pooling sounds in gstreamer. Working at it.

   The program needs some short ogg sound files. Used test ogg files from the following.

   http://rpg.hamsterrepublic.com/ohrrpgce/Free_Sound_Effects

   Ubuntu16.04 with GTK3.18

   gcc -Wall gstreamer_test2.c -o gstreamer_test2 `pkg-config --cflags --libs gtk+-3.0 gstreamer-1.0`

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<gst/gst.h>

//The sound files to play put in an array.
static GPtrArray *ogg_files;
static gint array_len=0;
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

static gint load_sounds(GtkWidget *combo, gpointer data);
static void play_sound(GtkWidget *button, gpointer *sounds);
static void sound_pipeline(struct s_pipeline *p1);
static gboolean bus_call(GstBus *bus, GstMessage *msg, struct s_pipeline *p1);
static void on_pad_added(GstElement *element, GstPad *pad, struct s_pipeline *p1);
  
int main(int argc, char *argv[])
 {
   gtk_init(&argc, &argv);
   gst_init(&argc, &argv);
   g_mutex_init(&mutex);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Sounds");
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   //The found .ogg file names
   ogg_files=g_ptr_array_new_full(10, g_free);

   GtkWidget *combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);

   //Load the .ogg files into the combobox to start with.
   gint num_sounds=load_sounds(combo1, NULL);
   array_len=num_sounds;

   //Set up the pipeline structs.
   gint i=0;
   g_print("Sounds %i\n", num_sounds);
   gpointer sounds[num_sounds];
   for(i=0;i<num_sounds;i++)
     {
       sounds[i]=g_new(struct s_pipeline, 1);
       ((struct s_pipeline *)(sounds[i]))->array_index=i;
     } 

   //A pool for the sounds.
   pool=gst_task_pool_new();
   gst_task_pool_prepare(pool, NULL);

   button1=gtk_button_new_with_label("Play .ogg Sounds");
   gtk_widget_set_hexpand(button1, TRUE);
   gtk_widget_set_vexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(play_sound), sounds);
  
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
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

   return 0;
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

    gtk_widget_set_sensitive(button, FALSE);
    sounds_left=array_len;

    //Start sound threads in pool.
    for(i=0;i<array_len;i++)
      {
        ((struct s_pipeline *)(sounds[i]))->pipeline_id=id;
        ((struct s_pipeline *)(sounds[i]))->pool_id=gst_task_pool_push(pool, (GstTaskPoolFunction)sound_pipeline, (struct s_pipeline *)sounds[i], &error);
        if(error!=NULL) g_print("Error: %s\n", error->message);
      }
    
    id++;        
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


