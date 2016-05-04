
/*
    A driver program using g_spawn_async_with_pipes(). This program starts a worker program(worker1.c).
Complile both programs in the same folder and start the driver program.

    Tested on Ubuntu14.04 with GTK3.10.

    gcc -Wall driver1.c -o driver1 `pkg-config gtk+-3.0 --cflags --libs`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<stdio.h>

//The path for the worker program. Probably in the same folder as the driver program for testing,
static const gchar path[]="/home/owner/eric/Rectangle5/worker1";
//Some file descriptors for std_out and std_error.
static gint fds[2];

static gboolean out_watch(GIOChannel *channel, GIOCondition cond, gpointer label)
  {
    gchar *string=NULL;
    gsize size;

    if(cond==G_IO_HUP)
      {
        g_print("Unref Out Channel\n");
        g_io_channel_unref(channel);
        return FALSE;
      }

    g_io_channel_read_line(channel, &string, &size, NULL, NULL);
    g_print("Out: %s", string);
    gtk_label_set_text(GTK_LABEL(label), string);
    g_free(string);

    return TRUE;
  }
static gboolean err_watch(GIOChannel *channel, GIOCondition cond, gpointer label)
  {
    gchar *string=NULL;
    gsize size;

    if(cond==G_IO_HUP)
      {
        g_print("Unref Error Channel\n");
        g_io_channel_unref(channel);
        return FALSE;
      }

    g_io_channel_read_line(channel, &string, &size, NULL, NULL);
    g_print("Err: %s", string);
    gtk_label_set_text(GTK_LABEL(label), string);
    g_free(string);

    return TRUE;
  }
static void child_watch(GPid  pid, gint status, GtkWidget **widgets)
  {
    g_print("Close Pipes and Exit\n");
    //Close std_out and std_error pipes and gnuplot.
    close(fds[0]);
    close(fds[1]);
    g_spawn_close_pid(pid);
    gtk_label_set_text(GTK_LABEL(widgets[1]), "Analysis Idle");
    gtk_widget_set_sensitive(widgets[0], TRUE);
  }
static void start_analysis(GtkWidget *button, GtkWidget **widgets)
  {
    gtk_widget_set_sensitive(button, FALSE);
    gchar *cmd=g_strdup_printf(path);
    gchar **arg_v=NULL;
    GError *error=NULL;
    GPid pid;
    gint std_out=0;
    gint std_error=0;

    g_shell_parse_argv(cmd, NULL, &arg_v, NULL);
    g_spawn_async_with_pipes(NULL, arg_v, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, NULL, &std_out, &std_error, &error); 
    
    if(error!=NULL)
      {
        g_print("%s\n", error->message);
        g_error_free(error);
      }
    else
      {
        fds[0]=std_out;
        fds[1]=std_error;
        GIOChannel *out_ch=g_io_channel_unix_new(std_out);
        GIOChannel *err_ch=g_io_channel_unix_new(std_error);
        g_io_add_watch(out_ch, G_IO_IN|G_IO_HUP, (GIOFunc)out_watch, widgets[1]);
        g_io_add_watch(err_ch, G_IO_IN|G_IO_HUP, (GIOFunc)err_watch, widgets[1]);
        guint event_id=0;
        event_id=g_child_watch_add(pid, (GChildWatchFunc)child_watch, widgets);
        g_print("Pipes %i %i %i %i\n", event_id, (int)pid, std_out, std_error);
      }

    g_free(cmd);
    g_strfreev(arg_v);   
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Driver Program");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *label=gtk_label_new("Analysis Idle");
    gtk_widget_set_vexpand(label, TRUE);
    gtk_widget_set_hexpand(label, TRUE);

    GtkWidget *button=gtk_button_new_with_label("Start Analysis");
    gtk_widget_set_vexpand(button, TRUE);
    gtk_widget_set_hexpand(button, TRUE);
    GtkWidget *widgets[]={button, label};
    g_signal_connect(button, "clicked", G_CALLBACK(start_analysis), widgets);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
