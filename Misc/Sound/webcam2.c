
/*
    Test a gstreamer webcam video split to the output window and to a single image rendered to an 
image widget. Remove a color channel before rendering.

    First, put together a gst-launch-1.0 to get a workable pipeline. Get an idea of how it might work.
Output frames to a folder.

    gst-launch-1.0 -e v4l2src ! tee name=t t. ! queue ! videoconvert ! videoscale ! video/x-raw, width=320, height=240 ! xvimagesink t. ! queue ! videoconvert ! videorate ! video/x-raw, framerate=1/1 ! pngenc ! multifilesink location="/home/owner/eric/Rectangle4/pictures/frame%d.png"

    Test outputting to appsink since that is what the program needs to do.

    gst-launch-1.0 -e v4l2src ! tee name=t t. ! queue ! videoconvert ! videoscale ! video/x-raw, width=320, height=240 ! xvimagesink t. ! queue ! videoconvert ! videorate ! video/x-raw, framerate=1/1 ! pngenc ! appsink

    compile with:
    gcc -Wall -fopenmp webcam2.c -o webcam2 `pkg-config gstreamer-1.0 gtk+-3.0 gstreamer-video-1.0 gstreamer-app-1.0 --cflags --libs`

    Tested on Ubuntu 14.04, GTK 3.10, GCC 4.8.4 and GStreamer 1.2.4

    C. Eric Cashon 
*/
   
#include<gtk/gtk.h>
#include<gst/gst.h>
#include<gst/video/videooverlay.h> 
#include<gst/app/gstappsink.h>  
#include<gdk/gdk.h>
#include<gdk/gdkx.h>

static void get_xid(GtkWidget *widget, gpointer data);
static gboolean refresh_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void on_message(GstBus *bus, GstMessage *message, gpointer *data);
static void on_sync_message(GstBus *bus, GstMessage *message, gpointer data);
static gboolean get_pixbuf(gpointer *data);
static void set_color(GtkComboBox *widget, gpointer data);
static void remove_color(GdkPixbuf *pixbuf, gint width, gint height);

static gint combo_color=0;

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "View");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 500);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //The pipeline and source.
    GstElement *pipeline=gst_pipeline_new("split_video");
    GstElement *src=gst_element_factory_make("v4l2src", NULL);
    GstElement *tee=gst_element_factory_make("tee", NULL);

    //First branch in pipeline.
    GstElement *q1 = gst_element_factory_make("queue", NULL);
    GstElement *video_convert1=gst_element_factory_make("videoconvert", NULL);
    GstElement *video_scale1=gst_element_factory_make("videoscale", NULL);
    GstElement *caps_filter1=gst_element_factory_make ("capsfilter", NULL); 
    GstElement *sink=gst_element_factory_make("xvimagesink", NULL);

    //Second branch in pipeline.
    GstElement *q2 = gst_element_factory_make ("queue", NULL);
    GstElement *video_convert2=gst_element_factory_make("videoconvert", NULL); 
    GstElement *caps_filter2=gst_element_factory_make ("capsfilter", NULL);    
    GstElement *png=gst_element_factory_make("pngenc", NULL);
    GstElement *app_sink=gst_element_factory_make("appsink", NULL);

    //Set the capabilities for the video stream.
    GstCaps *caps1=gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "YUV", "framerate", GST_TYPE_FRACTION, 30, 1, "width", G_TYPE_INT, 320, "height", G_TYPE_INT, 240, NULL);

    gst_app_sink_set_max_buffers(GST_APP_SINK(app_sink), 1);
    gst_app_sink_set_drop(GST_APP_SINK(app_sink), TRUE);
    g_object_set(G_OBJECT(q1), "leaky", 2, NULL);
    g_object_set(G_OBJECT(q2), "leaky", 2, "max-size-buffers", 1, NULL);

    gst_bin_add_many(GST_BIN(pipeline), src, tee, q1, video_convert1, video_scale1, caps_filter1, sink, q2, video_convert2, caps_filter2, png, app_sink, NULL);
    
    g_print("Add Filters\n");
    if(!gst_element_link_filtered(video_scale1, caps_filter1, caps1)) g_print("filter1 linked\n");
    else g_print("Warning: filter1 not linked\n");
    if(!gst_element_link_filtered(video_convert2, caps_filter2, caps1)) g_print("filter2 linked\n");
    else g_print("Warning: filter2 not linked\n");

    g_print("Link Elements\n");
    if(gst_element_link_many(src, tee, NULL)) g_print("tee linked\n");
    else g_print("Warning: tee not linked.\n");
  
    if(gst_element_link_many(tee, q1, video_convert1, caps_filter1, video_scale1, sink, NULL)) g_print("group1 linked\n");
    else g_print("Warning: group1 not linked.\n");
    
    if(gst_element_link_many(tee, q2, video_convert2, caps_filter2, png, app_sink, NULL)) g_print("group2 linked\n");
    else g_print("Warning: group2 not linked.\n");

    GtkWidget *da1 = gtk_drawing_area_new();
    gtk_widget_set_size_request(da1, 320, 240);
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "realize", G_CALLBACK(get_xid), sink);
    g_signal_connect(da1, "draw", G_CALLBACK(refresh_background), pipeline);

    GtkWidget *image = gtk_image_new();
    gtk_widget_set_size_request(image, 320, 240);
    gtk_widget_set_hexpand(image, TRUE);
    gtk_widget_set_vexpand(image, TRUE);

    GtkWidget *combo1 = gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "remove red");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "remove green");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "remove blue");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "no remove");
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
    g_signal_connect(combo1, "changed", G_CALLBACK(set_color), NULL);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 4, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 8, 4, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    //Setup a timer to get the frames.
    gpointer update[]={image, app_sink};
    g_timeout_add(500, (GSourceFunc)get_pixbuf, update);

    gtk_widget_show_all(window);

    GstBus *bus=gst_element_get_bus(pipeline);
    gst_bus_add_signal_watch(bus);
    gst_bus_enable_sync_message_emission(bus);

    g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(on_message), pipeline);
    g_signal_connect(G_OBJECT(bus), "sync-message::element", G_CALLBACK(on_sync_message), da1);
    gst_object_unref(bus);
    gst_element_set_state(GST_ELEMENT(pipeline), GST_STATE_PLAYING);

    gtk_main();

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
  }
static void get_xid(GtkWidget *widget, gpointer data)
  {
    GdkWindow *window=gtk_widget_get_window(widget);
    guintptr window_handle;
 
    if(!gdk_window_ensure_native(window))
      {
        g_print("Couldn't create native window needed for GstXOverlay!\n");
      }
 
    window_handle=GDK_WINDOW_XID(window);

    g_print("WindowID %i\n", (int)window_handle);
    gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(data), window_handle);     
  }
static gboolean refresh_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    if(GST_STATE((GstElement*)data)<GST_STATE_PAUSED)
      {
        GtkAllocation allocation;
        gtk_widget_get_allocation(widget, &allocation);
        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_rectangle(cr, 0, 0, allocation.width, allocation.height);
        cairo_fill(cr);
      }
   
    return FALSE;
  }
static void on_message(GstBus *bus, GstMessage *message, gpointer *data)
  {
    if(GST_MESSAGE_TYPE(message)==GST_MESSAGE_EOS)
      {
        gst_element_set_state((GstElement*)data, GST_STATE_NULL);
      }
    if(GST_MESSAGE_TYPE(message)==GST_MESSAGE_ERROR)
      {
        GError *error=NULL;
        gchar *debug=NULL;
        gst_message_parse_error(message, &error, &debug);
        g_print("Gst Error: %s, Debug: %s\n", error->message, debug);
        g_error_free(error);
        g_free(debug);
        gst_element_set_state((GstElement*)data, GST_STATE_NULL);
      }
  }
static void on_sync_message(GstBus *bus, GstMessage *message, gpointer data)
  {    
    if(gst_is_video_overlay_prepare_window_handle_message(message))
      {
         GdkWindow *window=gtk_widget_get_window(GTK_WIDGET(data));
         guintptr window_handle;
         window_handle = GDK_WINDOW_XID(window);
         gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)),
     window_handle);
      }    
  }
static gboolean get_pixbuf(gpointer *data)
  {
    GTimer *timer=g_timer_new();
    GstSample *sample=NULL;
    sample=gst_app_sink_pull_sample(GST_APP_SINK(data[1]));
    if(sample==NULL) g_print("Sample==NULL\n");
    gint width=0;
    gint height=0;
    GstCaps *caps = gst_sample_get_caps (sample);  
    GstStructure *s = gst_caps_get_structure (caps, 0);  
    gst_structure_get_int (s, "width", &width); 
    gst_structure_get_int (s, "height", &height);
    GstBuffer *buffer=gst_sample_get_buffer(sample);
    GstMapInfo map;
    GdkPixbuf *pixbuf=NULL;
    if(gst_buffer_map(buffer, &map, GST_MAP_READ)) 
      {
        //g_print("Gst Map size %i maxsize %i\n", map.size, map.maxsize);
        GInputStream *stream=g_memory_input_stream_new_from_data(map.data, map.maxsize, NULL);
        pixbuf=gdk_pixbuf_new_from_stream(stream, NULL, NULL); 
        gst_buffer_unmap(buffer, &map);
        g_object_unref(stream);
      }
    else g_print("Map Failed\n");

    if(combo_color!=3) remove_color(pixbuf, width, height);

    GdkPixbuf *scale=gdk_pixbuf_scale_simple(pixbuf, 320, 240, GDK_INTERP_BILINEAR);
    gtk_image_set_from_pixbuf(GTK_IMAGE(data[0]), scale);

    if(sample!=NULL) gst_sample_unref(sample);
    if(pixbuf!=NULL) g_object_unref(pixbuf);
    if(scale!=NULL) g_object_unref(scale);  

    gdouble elapsed_time=g_timer_elapsed(timer, NULL);
    g_print("Get Pixbuf Time %f\n", elapsed_time);
    g_timer_destroy(timer);

    return TRUE;
  }
static void set_color(GtkComboBox *widget, gpointer data)
  {
    gint color=gtk_combo_box_get_active(widget);
    combo_color=color;
  }
static void remove_color(GdkPixbuf *pixbuf, gint width, gint height)
  {
    gint i=0;
    gint j=0;
    gint step=gdk_pixbuf_get_rowstride(pixbuf);
    gint channels=gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels;
    guint length=0;
    pixels=gdk_pixbuf_get_pixels_with_length(pixbuf, &length);

    if(combo_color==0)
      {
        #pragma omp parallel for collapse(2)
        for(i=0;i<width;i++)
          {
            for(j=0;j<height;j++)
              {
                pixels[j*step+i*channels]=0;
              }
          }
      }
    else if(combo_color==1)
      {
        #pragma omp parallel for collapse(2)
        for(i=0;i<width;i++)
          {
            for(j=0;j<height;j++)
              {
                pixels[j*step+i*channels+1]=0;
              }
          }
      }
    else
      {
        #pragma omp parallel for collapse(2)
        for(i=0;i<width;i++)
          {
            for(j=0;j<height;j++)
              {
                pixels[j*step+i*channels+2]=0;
              }
          }
      }
  }







