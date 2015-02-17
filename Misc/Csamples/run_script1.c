
/*
  Test code for running a Bash script from a GTK+ program and getting return values.
  From discussion at GTK+ Forum, "System() call + ProgressBar update", with Shoaoby and Errol.

  A simple Bash script to use with the program saved as sleep.sh.
    #!/bin/bash
    echo "Start Bash Sleep"
    sleep 5s
    echo "End Bash Sleep"

  gcc -Wall run_script1.c -o run_script1 `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon
*/
#include <gtk/gtk.h>

typedef struct {
  GtkWidget *widget;
  GtkWidget *text;
  GtkWidget *pbar;
  guint timeout_id;
} Child_Info;

static gboolean watch_out_channel(GIOChannel *channel, GIOCondition cond, Child_Info *info)
 {
   gchar *string;
   gsize  size;

   if(cond==G_IO_HUP)
     {
       g_io_channel_unref(channel);
       return FALSE;
     }
   else
     {
       g_io_channel_read_line(channel, &string, &size, NULL, NULL );
       g_print("%s", string);
       gtk_label_set_text(GTK_LABEL(info->text), string);
       g_free(string);
       return TRUE;
     }
 }

static gint progress(gpointer data)
{  
  gtk_progress_bar_pulse(GTK_PROGRESS_BAR (data)); 
  return TRUE;
}

static void wait_for_child(GPid pid, gint script_retval, gpointer data)
{
  Child_Info *info = (Child_Info *)data;
  char message_to_display[256];

  g_source_remove(info->timeout_id);
  gtk_widget_set_sensitive(info->widget, TRUE);
  gtk_widget_set_sensitive(info->text, TRUE);
  g_spawn_close_pid(pid);

  if(script_retval == 0)
  {
    sprintf(message_to_display, "Script Completed");
  }
  else
  {
    sprintf(message_to_display, "Script Error Return %i", script_retval);
  }
  g_print("%s\n", message_to_display);
  gtk_widget_set_visible(info->pbar, FALSE);

  g_free(info);
}

static void on_click_Btn_Turn_on_trending_process( GtkWidget *widget, GArray *widgets )
{
  Child_Info *info;
  GtkWidget *text = g_array_index(widgets, GtkWidget*, 0);
  GtkWidget *pbar = g_array_index(widgets, GtkWidget*, 1);
  gboolean retval;
  GPid child_pid;
  char *cmd = "sleep.sh";
  gchar **arg_v = NULL;

  gtk_widget_set_sensitive(widget, FALSE); 
  g_shell_parse_argv(cmd, NULL, &arg_v, NULL);

  int std_out;
  GIOChannel *std_out_ch;
  retval = g_spawn_async_with_pipes(NULL, arg_v, NULL, G_SPAWN_DO_NOT_REAP_CHILD, NULL, NULL, &child_pid, NULL, &std_out, NULL, NULL);

  g_strfreev(arg_v);

  if(!retval) 
    {
     g_print("Could not start the executable.\n");
     return;
    }

  /* Execution of child has started */
  info = g_new(Child_Info, 1);

  gtk_widget_show(pbar);
  info->timeout_id = g_timeout_add(100, progress, pbar);
  info->widget = widget;
  info->text = text;
  info->pbar = pbar;

  std_out_ch=g_io_channel_unix_new(std_out);
  g_io_add_watch(std_out_ch, G_IO_IN | G_IO_HUP, (GIOFunc)watch_out_channel, info);
  g_child_watch_add(child_pid, wait_for_child, (gpointer) info);
}

int main(int argc, char **argv)
 {
   GtkWidget *window, *button1, *button2, *label1, *label2, *progress1, *progress2, *grid1;

   gtk_init(&argc, &argv);
   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Script Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
   gtk_container_set_border_width(GTK_CONTAINER(window), 10);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   button1=gtk_button_new_with_label("Run Script1");
   gtk_widget_set_hexpand(button1, TRUE);
   label1=gtk_label_new("Status1");
   gtk_widget_set_hexpand(label1, TRUE);
   progress1=gtk_progress_bar_new();
   gtk_widget_set_hexpand(progress1, TRUE);

   button2=gtk_button_new_with_label("Run Script2");
   gtk_widget_set_hexpand(button2, TRUE);
   label2=gtk_label_new("Status2");
   gtk_widget_set_hexpand(label2, TRUE);
   progress2=gtk_progress_bar_new();
   gtk_widget_set_hexpand(progress2, TRUE);

   GArray *widgets1=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
   g_array_append_val(widgets1, label1);
   g_array_append_val(widgets1, progress1);
   GArray *widgets2=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
   g_array_append_val(widgets2, label2);
   g_array_append_val(widgets2, progress2);
   g_signal_connect(button1, "clicked", G_CALLBACK(on_click_Btn_Turn_on_trending_process), widgets1);
   g_signal_connect(button2, "clicked", G_CALLBACK(on_click_Btn_Turn_on_trending_process), widgets2);

   grid1=gtk_grid_new();
   gtk_container_add(GTK_CONTAINER(window), grid1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), progress1, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button2, 0, 3, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label2, 0, 4, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), progress2, 0, 5, 1, 1);

   gtk_widget_show_all(window);
   gtk_widget_set_visible(progress1, FALSE);
   gtk_widget_set_visible(progress2, FALSE);
   gtk_main();
   return 0;
 }
