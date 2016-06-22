
/*

    A non-modal dialog in GTK+. There is also an example of this in the Python folder that is named
similarly. 

    gcc -Wall dialog_nonmodal1.c -o dialog_nonmodal1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/

#include <gtk/gtk.h>

static void open_dialog(GtkWidget *widget, gpointer data);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    gtk_window_set_title(GTK_WINDOW(window), "Non Modal Dialogs");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *button1=gtk_toggle_button_new_with_label("Show Dialog");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(open_dialog), window);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);                  
    gtk_main();
    return 0;
  }
static void open_dialog(GtkWidget *widget, gpointer data)
  {
    //Non modal dialog. Open several dialog windows from the main window.
    static gint i=1;
    gchar *dialog_label=g_strdup_printf("Dialog Window%i", i);
    GtkWidget *dialog=gtk_dialog_new_with_buttons("Dialog", GTK_WINDOW(data), GTK_DIALOG_DESTROY_WITH_PARENT, "Close", GTK_RESPONSE_CLOSE, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 200, 100);
                                 
    GtkWidget *box=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    GtkWidget *label=gtk_label_new(dialog_label);
    gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0);
    
    g_signal_connect_swapped(dialog, "response", G_CALLBACK(gtk_widget_destroy), dialog);
    gtk_widget_show_all(dialog);

    //Move the windows.
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
    g_free(dialog_label);        
  }

