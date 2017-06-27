
/*
   Test code for plugs and sockets with interprocess communication. For it to work, both the
plug2.c and socket2.c files need to be compiled and then run socket2.

   gcc -Wall socket2.c -o socket2 `pkg-config --cflags --libs gtk+-3.0`

   Tested on Ubuntu16.04 and GTK3.18

   C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<stdlib.h>

static gboolean plug_embedded=FALSE;
static GPid child_pid=0;

static void plug_added(GtkSocket *socket, gpointer data);
static gboolean plug_removed(GtkSocket *socket, gpointer data);
static void add_plug(GtkWidget *widget, gpointer *data);
static gboolean watch_out_channel(GIOChannel *channel, GIOCondition cond, gpointer *data);
static void quit_program(GtkWidget *widget, gpointer data);
static void change_color(GtkComboBox *combo, gpointer data);

static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 0.5);
    cairo_paint(cr);
    return FALSE;
  } 
static gboolean draw_background2(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 0.5);
    cairo_paint(cr);

    cairo_set_line_width(cr, 3.0);
    cairo_set_source_rgba(cr, 1.0, 0.0, 1.0, 1.0);
    cairo_rectangle(cr, width/10.0, height/10.0, 8.0*width/10.0, 8.0*height/10.0);
    cairo_stroke(cr);
    
    return FALSE;
  } 
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Socket");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 300);

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

    GtkWidget *grid=gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);

    GtkWidget *socket=gtk_socket_new();
    gtk_widget_set_size_request(socket, 200, 175);
    gtk_grid_attach(GTK_GRID(grid), socket, 0, 0, 1, 1);
    g_signal_connect(socket, "plug-added", G_CALLBACK(plug_added), NULL);
    g_signal_connect(socket, "plug-removed", G_CALLBACK(plug_removed), NULL);

    gtk_widget_set_app_paintable(socket, TRUE);
    //Try to set transparency of main window.
    if(gtk_widget_is_composited(socket))
      {
        GdkScreen *screen=gtk_widget_get_screen(socket);  
        GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(socket, visual);
      }
    else g_print("Can't set window transparency.\n");
    g_signal_connect(socket, "draw", G_CALLBACK(draw_background2), NULL); 

    GString *plug_color=g_string_new("red");
    g_signal_connect(window, "destroy", G_CALLBACK(quit_program), plug_color);

    GtkWidget *button1=gtk_button_new_with_label("Get Plug");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);
    gpointer data[]={button1, socket, plug_color};
    g_signal_connect(button1, "clicked", G_CALLBACK(add_plug), data);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "red plug label");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "green plug label");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "blue plug label");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), "1");
    g_signal_connect(combo, "changed", G_CALLBACK(change_color), plug_color);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 2, 1, 1);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;   
  }
static void plug_added(GtkSocket *socket, gpointer data)
  {
    g_print("A plug was added\n");
  }
static gboolean plug_removed(GtkSocket *socket, gpointer data)
  {
    g_print("A Plug was removed\n");
    return TRUE;
  }
static void add_plug(GtkWidget *widget, gpointer *data)
  {
    g_print("Add Plug\n");
    gboolean retval;
    gchar *cmd = g_strdup_printf("plug2 %s", ((GString*)data[2])->str);
    gchar **arg_v = NULL;
    gint std_out=0;
    GError *error=NULL;
    GIOChannel *std_out_ch;

    g_shell_parse_argv(cmd, NULL, &arg_v, NULL);
    retval = g_spawn_async_with_pipes(NULL, arg_v, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &child_pid, NULL, &std_out, NULL, &error);

    g_strfreev(arg_v);

    if(retval)
      {
        gtk_widget_set_sensitive(widget, FALSE);
        std_out_ch=g_io_channel_unix_new(std_out);
        g_io_add_watch(std_out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)watch_out_channel, data);
        g_print("Channel Added\n");
      }
    else 
      {
        g_print("Couldn't start plug. %s\n", error->message);
        g_error_free(error);
      }
  }
static gboolean watch_out_channel(GIOChannel *channel, GIOCondition cond, gpointer *data)
  {
    g_print("Watch Out\n");
    if(cond==G_IO_HUP)
      {
        g_print("Unreference Channel\n");
        g_io_channel_unref(channel);
        return FALSE;
      } 
    else
      {
        gsize size;
        gchar *string1=NULL;
        g_io_channel_read_line(channel, &string1, &size, NULL, NULL );
        g_print("%s", string1);
        if(atoi(string1)>0&&!plug_embedded)
          {
            gtk_socket_add_id(GTK_SOCKET(data[1]), (Window)atoi(string1));
            plug_embedded=TRUE;
          }
        if(g_strcmp0("dis\n", string1)==0)
          {
            plug_embedded=FALSE;
            gchar *string2=g_strdup_printf("kill %i", (gint)child_pid);
            g_print("%s\n", string2);
            g_spawn_command_line_async(string2, NULL); 
            g_spawn_close_pid(child_pid);
            g_free(string2);
            gtk_widget_set_sensitive(GTK_WIDGET(data[0]), TRUE);
          }
        g_free(string1);
        return TRUE;
      }
  }
static void quit_program(GtkWidget *widget, gpointer data)
  {
    if(child_pid!=0) 
      {
        gchar *string=g_strdup_printf("kill %i", (gint)child_pid);
        g_print("%s\n", string);
        g_spawn_command_line_async(string, NULL); 
        g_spawn_close_pid(child_pid);
        g_free(string);
      }
    g_string_free((GString*)data, TRUE);
    gtk_main_quit();
  }
static void change_color(GtkComboBox *combo, gpointer data)
  {
    gchar *color=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo));
    g_string_truncate((GString*)data, 0);
    g_string_append((GString*)data, color);
    g_print("GString Color %s\n", ((GString*)data)->str);
    g_free(color);
  }
