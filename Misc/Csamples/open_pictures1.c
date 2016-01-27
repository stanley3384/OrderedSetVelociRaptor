
/*
    Test out opening a series of png files tiled across the screen. This program loads a
combo box with files ending in .png within the local directory. This works on Ubuntu14.04
but a window manager might not comply with the move window request. 

    gcc -Wall open_pictures1.c -o open_pictures1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include <gtk/gtk.h>

static void load_pictures(GtkWidget *combo);
static void show_picture_dialog(GtkWidget *widget, gpointer *data);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    gtk_window_set_title(GTK_WINDOW(window), "Open Pictures");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *combo1 = gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_widget_set_vexpand(combo1, TRUE);
    //Load the .png files into the combo.
    load_pictures(combo1); 

    GtkWidget *button1 = gtk_toggle_button_new_with_label("Open");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, TRUE);

    gpointer win_combo[] = {window, combo1};
    g_signal_connect(button1, "clicked", G_CALLBACK(show_picture_dialog), win_combo);
      
    GtkWidget *grid = gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);                  
    gtk_main();
    return 0;
  }
static void load_pictures(GtkWidget *combo)
  {
    GError *dir_error = NULL;
    const gchar *file_temp = NULL;
    gint i = 0;
    gchar *file_type = g_strdup(".png");

    GDir *directory = g_dir_open("./", 0, &dir_error);
    if(dir_error != NULL)
      {
        g_print("dir Error %s\n", dir_error->message);
        g_error_free(dir_error);
        gtk_widget_set_sensitive(combo, FALSE);
      }
    else
      {
        file_temp = g_dir_read_name(directory);
        while(file_temp != NULL)
          {
            if(file_temp != NULL && g_str_has_suffix(file_temp, file_type))
              {
                //g_print("%s\n", file_temp);
                gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(combo), file_temp);
                i++;
              }
            file_temp = g_dir_read_name(directory);
          }
        if(i > 0)
          {
            gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
            gtk_widget_set_sensitive(combo, TRUE);
          }
        else gtk_widget_set_sensitive(combo, FALSE);
        g_dir_close(directory);
      }

    g_free(file_type);
  }
static void show_picture_dialog(GtkWidget *widget, gpointer *data)
  {
    //Non modal dialog. Open several dialog windows from the main window.
    static gint i = 1;
    gchar *file_name = gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(data[1]));

    GtkWidget *dialog = gtk_dialog_new_with_buttons(file_name, GTK_WINDOW(data[0]), GTK_DIALOG_DESTROY_WITH_PARENT, "Close", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 200, 200);
                                 
    GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));

    GtkWidget *image = gtk_image_new_from_file(file_name);

    GtkWidget *view = gtk_viewport_new(NULL, NULL);
    gtk_widget_set_hexpand(view, TRUE);
    gtk_widget_set_vexpand(view, TRUE);
    gtk_container_add(GTK_CONTAINER(view), image);
    GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    gtk_container_add(GTK_CONTAINER(content_area), scroll);
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
    gtk_widget_show_all(dialog); 

    //Move the windows. This might not work with all window managers. See GTK+ documentation.
    gint width = gdk_screen_width();
    gint height = gdk_screen_height();
    gint sections_x = width / 100;
    gint sections_y = height / 100;
    gint s1 = (i - 1) % (sections_x * sections_y);
    gint move_x = (s1 % sections_x) * 100;
    gint move_y = (s1 / sections_x) * 100;    
    g_print("Move %i %i %i %i %i %i\n", width, height, sections_x, sections_y, move_x, move_y);
    gtk_window_move(GTK_WINDOW(dialog), move_x, move_y);
 
    i++;
    g_free(file_name);        
  }






