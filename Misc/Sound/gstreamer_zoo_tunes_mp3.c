
/*
    Test code for playing some mp3's with gstreamer and pulsesink. 

    The program uses three mp3 files called chicken.mp3, elephant.mp3 and cougar.mp3.
Change the combo box and file names for the sounds that you might want to test. Caution, 
you might want to keep the volume down if you are at the library.

    http://www.noiseaddicts.com/free-samples-mp3/?category_name=Animals

    Tested on Ubuntu14.04 and GTK3.10
    gcc gstreamer_zoo_tunes_mp3.c -o zoo_tunes `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>

static void play_sound(GtkWidget *button, gpointer data)
  {
    gchar *critter=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(data));
    gchar *string=g_strdup_printf("gst-launch-1.0 filesrc location=%s ! mad ! pulsesink", critter); 
    if(g_file_test(critter, G_FILE_TEST_EXISTS))
      {
        if(g_strcmp0(critter, "chicken.mp3")==0||g_strcmp0(critter, "elephant.mp3")==0||g_strcmp0(critter, "cougar.mp3")==0)
          {
            //Doesn't kill the process at the end of the program if it is still playing.  
            g_spawn_command_line_async(string, NULL);
          }
        else
          {
            g_print("Couldn't find a valid critter mp3.\n");
          }
      }
    else
      {
        g_print("Couldn't find the file %s.\n", critter);
      }
    g_free(critter);
    g_free(string);
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Zoo Tunes");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *button1=gtk_button_new_with_label("Play Sound");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, TRUE);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "chicken.mp3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "elephant.mp3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "cougar.mp3");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo), "1");
    g_signal_connect(button1, "clicked", G_CALLBACK(play_sound), combo);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;   
  }

