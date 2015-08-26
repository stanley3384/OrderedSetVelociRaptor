
/*
   Test code for plugs and sockets with interprocess communication. For it to work, both the
plug2.c and socket2.c files need to be compiled and then run socket2.

   gcc -Wall socket2.c -o socket2 `pkg-config --cflags --libs gtk+-3.0`

   C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<gtk/gtkx.h>
#include<stdlib.h>

static gboolean plug_embedded=FALSE;
static GPid child_pid=0;

static void plug_added(GtkSocket *socket, gpointer data);
static gboolean plug_removed(GtkSocket *socket, gpointer data);
static void add_plug(GtkWidget *widget, gpointer data);
static gboolean watch_out_channel(GIOChannel *channel, GIOCondition cond, gpointer data);
static void quit_program(GtkWidget *widget, gpointer data);
 
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Socket");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(quit_program), NULL);

    GtkWidget *grid=gtk_grid_new();
    gtk_container_add(GTK_CONTAINER(window), grid);

    GtkWidget *socket=gtk_socket_new();
    gtk_widget_set_size_request(socket, 200, 175);
    gtk_grid_attach(GTK_GRID(grid), socket, 0, 0, 1, 1);
    g_signal_connect(socket, "plug-added", G_CALLBACK(plug_added), NULL);
    g_signal_connect(socket, "plug-removed", G_CALLBACK(plug_removed), NULL); 

    GtkWidget *button1=gtk_button_new_with_label("Get Plug");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);
    g_signal_connect(button1, "clicked", G_CALLBACK(add_plug), socket);
    
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
static void add_plug(GtkWidget *widget, gpointer data)
  {
    g_print("Add Plug\n");
    gboolean retval;
    gchar *cmd = "plug2";
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
static gboolean watch_out_channel(GIOChannel *channel, GIOCondition cond, gpointer data)
  {
    g_print("Watch Out\n");
    if(cond==G_IO_HUP)
      {
        g_print("IO_HUP\n");
        g_io_channel_unref(channel);
        return FALSE;
      } 
    else
      {
        gsize size;
        gchar *string=NULL;
        g_io_channel_read_line(channel, &string, &size, NULL, NULL );
        g_print("%s", string);
        if(atoi(string)>0&&!plug_embedded)
         {
           gtk_socket_add_id(GTK_SOCKET(data), (Window)atoi(string));
           plug_embedded=TRUE;
         }
        g_free(string);
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
    gtk_main_quit();
  }
