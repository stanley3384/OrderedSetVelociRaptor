
/*
    Test a simple gstreamer webcam. 

    gcc -Wall webcam1.c -o webcam1 `pkg-config gstreamer-1.0 gtk+-3.0 gstreamer-video-1.0 --cflags --libs`

    Tested on Ubuntu14.04 GTK3.10

    C. Eric Cashon 
*/
   
#include<gtk/gtk.h>
#include<gst/gst.h>
#include<gst/video/videooverlay.h>   
#include<gdk/gdk.h>
#include<gdk/gdkx.h>

static void get_xid(GtkWidget *widget, gpointer data);
static gboolean refresh_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void start_stop(GtkWidget *widget, gpointer *data);
static void on_message(GstBus *bus, GstMessage *message, gpointer *data);
static void on_sync_message(GstBus *bus, GstMessage *message, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
    gst_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "View");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 150, 150);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GstElement *pipeline=gst_pipeline_new("xvoverlay");
    GstElement *src=gst_element_factory_make("v4l2src", NULL);
    GstElement *sink=gst_element_factory_make("xvimagesink", NULL);
    gst_bin_add_many(GST_BIN(pipeline), src, sink, NULL);
    gst_element_link(src, sink);
   
    GtkWidget *da1 = gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "realize", G_CALLBACK(get_xid), sink);
    g_signal_connect(da1, "draw", G_CALLBACK(refresh_background), pipeline);
   
    gpointer dap[]={da1, pipeline};
    GtkWidget *button1=gtk_button_new_with_label("Stop");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(G_OBJECT(button1), "clicked", G_CALLBACK(start_stop), dap);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 4, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    GstBus *bus=gst_element_get_bus(pipeline);
    gst_bus_add_signal_watch(bus);
    gst_bus_enable_sync_message_emission(bus);

    gpointer vid[]={pipeline, button1};
    g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(on_message), vid);
    g_signal_connect(G_OBJECT(bus), "sync-message::element", G_CALLBACK(on_sync_message), da1);
    gst_object_unref(bus);

    gst_element_set_state(pipeline, GST_STATE_PLAYING);

    gtk_main();

    gst_element_set_state(pipeline, GST_STATE_NULL);
    gst_object_unref(pipeline);

    return 0;
  }
static void get_xid(GtkWidget *widget, gpointer data)
  {
    GdkWindow *window = gtk_widget_get_window(widget);
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
static void start_stop(GtkWidget *widget, gpointer *data)
  {
    if(g_strcmp0("Start", gtk_button_get_label(GTK_BUTTON(widget)))==0)
      {
        g_print("Start\n");
        gtk_button_set_label(GTK_BUTTON(widget), "Stop");
        gst_element_set_state((GstElement*)data[1], GST_STATE_PLAYING);
      }
    else
      {
        g_print("Stop\n");
        gst_element_set_state((GstElement*)data[1], GST_STATE_NULL);
        gtk_button_set_label(GTK_BUTTON(widget), "Start");
        gtk_widget_queue_draw(GTK_WIDGET(data[0]));
      }
  }
static void on_message(GstBus *bus, GstMessage *message, gpointer *data)
  {
    if(GST_MESSAGE_TYPE(message)==GST_MESSAGE_EOS)
      {
        gst_element_set_state((GstElement*)data[0], GST_STATE_NULL);
        gtk_button_set_label(GTK_BUTTON(data[1]), "Start");
      }
    if(GST_MESSAGE_TYPE(message)==GST_MESSAGE_ERROR)
      {
        GError *error=NULL;
        gchar *debug=NULL;
        gst_message_parse_error(message, &error, &debug);
        g_print("Gst Error: %s, Debug: %s\n", error->message, debug);
        g_error_free(error);
        g_free(debug);
        gst_element_set_state((GstElement*)data[0], GST_STATE_NULL);
        gtk_button_set_label(GTK_BUTTON(data[1]), "Start");
      }
  }
static void on_sync_message(GstBus *bus, GstMessage *message, gpointer data)
  {
    g_print("On Sync Message\n");
    
    if(gst_is_video_overlay_prepare_window_handle_message(message))
      {
         GdkWindow *window=gtk_widget_get_window(GTK_WIDGET(data));
         guintptr window_handle;
         window_handle = GDK_WINDOW_XID(window);
         gst_video_overlay_set_window_handle(GST_VIDEO_OVERLAY(GST_MESSAGE_SRC(message)),
     window_handle);
      }    
  }







