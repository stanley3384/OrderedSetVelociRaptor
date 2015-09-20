
/*
   Test code for comparing running a gstreamer sound with g_spawn and from code. The program needs
an ogg sound file. The sound should be less than or around a second long and in the same folder
as the program. Used a test ogg file from the following.

   http://rpg.hamsterrepublic.com/ohrrpgce/Free_Sound_Effects

   Ubuntu14.04 with GTK3.10

   gcc -Wall gstreamer_test1.c -o gstreamer_test1 `pkg-config --cflags --libs gtk+-3.0 gstreamer-0.10`

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<gst/gst.h>
#include<stdio.h>

//The sound file to test. Change ogg char strings for testing
static gchar *ogg_file="Metal_Hit.ogg";
static gchar *ogg_file_uri="playbin2 uri=file:///home/owner/eric/Rectangle2/Metal_Hit.ogg";
static gchar *ogg_file_spawn="gst-launch-1.0 filesrc location=Metal_Hit.ogg ! decodebin ! pulsesink";
/*
  The timer doesn't give a total time for async since it returns right away. Gstreamer will
return the play time of the sound file for sync, async and system. The program timer gives
the total time for gstreamer, gstreamer_playbin, sync and system.
*/
static GTimer *timer=NULL;
//gstreamer variables
static GstElement *pipeline=NULL;
static guint bus_watch_id=0;

static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data);
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data);
static void play_sound(GtkWidget *button, gpointer data);
static void sound_pipeline(GtkWidget *button);
static void sound_pipeline_playbin(GtkWidget *button);
static void spawn_sound(gpointer data);
static void click_button(GtkWidget *button, gpointer data);

int main(int argc, char *argv[])
 {
   gtk_init(&argc, &argv);
   gst_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Sound");
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *combo=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "gstreamer");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "gstreamer playbin");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "sync");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 3, "4", "async");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 4, "5", "system");
   gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), "1");

   GtkWidget *button1=gtk_button_new_with_label("Play Sound");
   gtk_widget_set_hexpand(button1, TRUE);
   gtk_widget_set_vexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(play_sound), combo);
  
   GtkWidget *button2=gtk_button_new_with_label("Click Button");
   gtk_widget_set_hexpand(button2, TRUE);
   gtk_widget_set_vexpand(button2, TRUE);
   g_signal_connect(button2, "clicked", G_CALLBACK(click_button), NULL);

   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), combo, 0, 2, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;
 }
static gboolean bus_call(GstBus *bus, GstMessage *msg, gpointer data)
 {
   GError *error=NULL;
   switch(GST_MESSAGE_TYPE(msg))
    {
      case GST_MESSAGE_EOS:
        //g_print ("End of Stream\n");
        gst_element_set_state (pipeline, GST_STATE_NULL);
        //Unreference the pipeline and other element objects stored in the pipeline.
        gst_object_unref(GST_OBJECT(pipeline));
        pipeline=NULL;
        g_source_remove(bus_watch_id);
        bus_watch_id=0;
        g_timer_stop(timer);
        g_print("Time %f\n", g_timer_elapsed(timer, NULL));
        g_timer_destroy(timer);
        timer=NULL;
        gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
        break;
      case GST_MESSAGE_ERROR:
        gst_message_parse_error(msg, &error, NULL);
        g_print("Error: %s\n", error->message);
        g_error_free(error);
        break;
      default:
        break;
    }
    return TRUE;
  }
static void on_pad_added(GstElement *element, GstPad *pad, gpointer data)
  {
    GstPad *sinkpad;
    GstElement *decoder=(GstElement*)data;  
    sinkpad=gst_element_get_static_pad(decoder, "sink");
    gst_pad_link(pad, sinkpad);
    gst_object_unref(sinkpad);   
  }
static void play_sound(GtkWidget *button, gpointer data)
  {
    timer=g_timer_new(); 
    //The set sensitive won't prevent the button from queuing up clicks in sync and system.      
    gtk_widget_set_sensitive(button, FALSE);
    //set_sensitive changed and timer stopped at end of stream(EOS) in sound_pipeline() and sound_pipeline_playbin.
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==0) sound_pipeline(button);
    else if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==1) sound_pipeline_playbin(button);
    else
      {
        spawn_sound(data);
        g_timer_stop(timer);
        g_print("Time %f\n", g_timer_elapsed(timer, NULL));
        g_timer_destroy(timer);
        timer=NULL;
        gtk_widget_set_sensitive(button, TRUE);
      }
  }
static void sound_pipeline(GtkWidget *button)
  {
    pipeline=gst_pipeline_new("audio-player");
    GstElement *source=gst_element_factory_make("filesrc", "file-source");
    GstElement *demuxer=gst_element_factory_make("oggdemux", "ogg-demuxer");
    GstElement *decoder=gst_element_factory_make("vorbisdec", "vorbis-decoder");
    GstElement *conv=gst_element_factory_make("audioconvert", "converter");
    GstElement *sink=gst_element_factory_make ("autoaudiosink", "audio-output");

    if(!pipeline || !source || !demuxer || !decoder || !conv || !sink)
      {
        g_print("Error, One element could not be created.\n");
      }

    g_object_set(G_OBJECT(source), "location", ogg_file, NULL);

    GstBus *bus=gst_pipeline_get_bus(GST_PIPELINE(pipeline));
    bus_watch_id=gst_bus_add_watch(bus, bus_call, button);
    gst_object_unref(bus);

    gst_bin_add_many(GST_BIN(pipeline), source, demuxer, decoder, conv, sink, NULL);

    gst_element_link(source, demuxer);
    gst_element_link_many(decoder, conv, sink, NULL);
    g_signal_connect(demuxer, "pad-added", G_CALLBACK(on_pad_added), decoder);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);
  }
static void sound_pipeline_playbin(GtkWidget *button)
  {
    //The short version of sound_pipeline().
    GstBus *bus=NULL;
    GstMessage *msg=NULL;
    pipeline=gst_parse_launch(ogg_file_uri, NULL);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);
   
    bus=gst_element_get_bus(pipeline);
    msg=gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE, GST_MESSAGE_ERROR | GST_MESSAGE_EOS);
   
    if(msg!=NULL)
      {
        if(GST_MESSAGE_TYPE(msg)==GST_MESSAGE_ERROR)
          {
            GError *error=NULL;
            gst_message_parse_error(msg, &error, NULL);
            g_print("Error: %s\n", error->message);
            g_error_free(error);
          }
        gst_message_unref(msg);
      }
    gst_object_unref(bus);
    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);
    pipeline=NULL;
    g_timer_stop(timer);
    g_print("Time %f\n", g_timer_elapsed(timer, NULL));
    g_timer_destroy(timer);
    timer=NULL;
    gtk_widget_set_sensitive(button, TRUE);
  }
static void spawn_sound(gpointer data)
  {
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==2)
      {
        g_spawn_command_line_sync(ogg_file_spawn, NULL, NULL, NULL, NULL);
      }
    else if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==3)
      {
        g_spawn_command_line_async(ogg_file_spawn, NULL);
      }
    else
      {
        system(ogg_file_spawn);
      }
  }
static void click_button(GtkWidget *button, gpointer data)
  {
    g_print("Button Clicked\n");
  }


