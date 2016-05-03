
/*

    Use gnuplot to output directly to a drawing area widget. This works with the x11 terminal. The 
drawing doesn't look as good as the pngcairo terminal but it is all done in memory.
    Try using g_spawn_async_with_pipes() function instead of popen() in gnuplot3. Check for possible
gnuplot errors. Check the program with "ps -x" in another terminal to see if it is closing gnuplot.

    gcc -Wall gnuplot3.c -o gnuplot3 `pkg-config gtk+-3.0 --cflags --libs`

    A listing of gnuplot terminals.
    http://www.bersch.net/gnuplot-doc/complete-list-of-terminals.html

    Tested with Ubuntu14.04 32bit and GTK3.10

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<stdio.h>
#include<string.h>

static GRand *rand=NULL;

static gboolean out_watch(GIOChannel *channel, GIOCondition cond, gpointer data)
  {
    gchar *string;
    gsize size;

    if(cond==G_IO_HUP)
      {
        g_print("Unref Out Channel\n");
        g_io_channel_unref(channel);
        return FALSE;
      }

    g_io_channel_read_line(channel, &string, &size, NULL, NULL);
    g_print("Out: %s", string);
    g_free(string);

    return TRUE;
  }
static gboolean err_watch(GIOChannel *channel, GIOCondition cond, gpointer data)
  {
    gchar *string;
    gsize size;

    if(cond==G_IO_HUP)
      {
        g_print("Unref Error Channel\n");
        g_io_channel_unref(channel);
        return FALSE;
      }

    g_io_channel_read_line(channel, &string, &size, NULL, NULL);
    g_print("Err: %s", string);
    g_free(string);

    return TRUE;
  }
static void child_watch(GPid  pid, gint status, gpointer data)
  {
    g_print("Close Pipes and Exit\n");
    //Close std_out and std_error pipes and gnuplot.
    close(((gint*)data)[0]);
    close(((gint*)data)[1]);
    g_spawn_close_pid(pid);
  }
static gboolean plot_data(gpointer da)
  {
    static gint i=1;
    g_print("\nPlot %i\n", i++);
    gdouble rand_num=g_rand_double(rand);

    GdkWindow *win=gtk_widget_get_window(GTK_WIDGET(da));
    Window win_id=gdk_x11_window_get_xid(win);
    g_print("WindowID %02x\n", (unsigned int)win_id);
    gchar *hex=g_strdup_printf("%02x", (unsigned int)win_id);
    gchar *cmd=g_strdup_printf("/usr/bin/gnuplot");
    gchar *script=g_strdup_printf("set terminal x11 window '%s'\nset xlabel \"sine wave\"\nset yrange [-1:1]\nset ylabel \"amplitude\"\nplot %f*sin(x)\n", hex, rand_num);
    g_print("%s\n", script);
    gchar **arg_v=NULL;
    GError *error=NULL;
    GPid pid;
    gint std_in=0;
    gint std_out=0;
    gint std_error=0;

    g_shell_parse_argv(cmd, NULL, &arg_v, NULL);
    g_spawn_async_with_pipes(NULL, arg_v, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &pid, &std_in, &std_out, &std_error, &error); 
    
    if(error!=NULL)
      {
        g_print("%s\n", error->message);
        g_error_free(error);
      }
    else
      {
        GIOChannel *out_ch=g_io_channel_unix_new(std_out);
        GIOChannel *err_ch=g_io_channel_unix_new(std_error);
        g_io_add_watch(out_ch, G_IO_IN|G_IO_HUP, (GIOFunc)out_watch, NULL);
        g_io_add_watch(err_ch, G_IO_IN|G_IO_HUP, (GIOFunc)err_watch, NULL);
        gint fds[]={std_out, std_error};
        guint event_id=0;
        event_id=g_child_watch_add(pid, (GChildWatchFunc)child_watch, fds);
        g_print("Pipes %i %i %i %i\n", event_id, (int)pid, std_out, std_error);

        write(std_in, script, strlen(script));
        /*
          Close std_in first. Close other pipes in child_watch(). Otherwise problem with closing
          gnuplot and unref channels. This also keeps the std_out and std_error messages.
        */
        close(std_in);
      }

    g_free(hex);
    g_free(cmd);
    g_free(script);
    g_strfreev(arg_v);   
    return TRUE;
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gnuplot GTK");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    rand=g_rand_new();

    GtkWidget *da = gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 400, 400);
    gtk_widget_set_double_buffered(da, FALSE);
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);

    GtkWidget *view=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view), da);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    //Add a timer to update the drawing.
    g_timeout_add(500, (GSourceFunc)plot_data, da);

    gtk_widget_show_all(window);

    gtk_main();

    g_rand_free(rand);

    return 0;
  }
