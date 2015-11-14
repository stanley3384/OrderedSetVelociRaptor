
/*
    Copy, cut and paste values from one textview to another with a clipboard. You can use the keyboard
shortcut keys, ctrl-x, ctrl-c or ctrl-v or the buttons.

    gcc -Wall clipboard1.c -o clipboard1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static void get_button(GtkWidget *button, gpointer *data)
  {
    const gchar *button_name=gtk_widget_get_name(button);
    GtkTextBuffer *text_buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data[0]));
    gboolean selected=gtk_text_buffer_get_has_selection(text_buffer1);
   
    if(selected||g_strcmp0(button_name, "b3")==0)
      {
        GtkTextBuffer *text_buffer2=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data[1]));
        if(g_strcmp0(button_name, "b1")==0)
          {
            gtk_text_buffer_copy_clipboard(text_buffer1, GTK_CLIPBOARD(data[2]));
          }
        else if(g_strcmp0(button_name, "b2")==0)
          {
            gtk_text_buffer_cut_clipboard(text_buffer1, GTK_CLIPBOARD(data[2]), TRUE);
          }
        else
          {
            gtk_text_buffer_paste_clipboard(text_buffer2, GTK_CLIPBOARD(data[2]), NULL, TRUE);
          }
       }
     else g_print("Select Some Text\n");
  }
int main(int argc, char *argv[])
  {   
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Copy, Cut and Paste");
    gtk_container_set_border_width(GTK_CONTAINER(window),10);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    g_signal_connect(window,"destroy",G_CALLBACK(gtk_main_quit), NULL);

    GdkDisplay *display=gdk_display_get_default();
    GtkClipboard *clipboard=gtk_clipboard_get_for_display(display, GDK_SELECTION_CLIPBOARD);

    GtkWidget *textview1=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview1), GTK_WRAP_CHAR);
    gtk_widget_set_vexpand(textview1, TRUE);
    GtkTextBuffer *text_buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1));
    gtk_text_buffer_set_text(text_buffer, "Select to copy or cut this text.", -1);

    GtkWidget *textview2=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview2), GTK_WRAP_CHAR);
    gtk_widget_set_vexpand(textview2, TRUE);

    gpointer textviews[]={textview1, textview2, clipboard};

    GtkWidget *button1=gtk_button_new_with_label("Copy");
    gtk_widget_set_name(button1, "b1");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(GTK_BUTTON(button1), "clicked", G_CALLBACK(get_button), textviews);

    GtkWidget *button2=gtk_button_new_with_label("Cut");
    gtk_widget_set_name(button2, "b2");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(GTK_BUTTON(button2), "clicked", G_CALLBACK(get_button), textviews);

    GtkWidget *button3=gtk_button_new_with_label("Paste");
    gtk_widget_set_name(button3, "b3");
    gtk_widget_set_hexpand(button3, TRUE);
    g_signal_connect(GTK_BUTTON(button3), "clicked", G_CALLBACK(get_button), textviews);

    GtkWidget *grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), textview1, 0, 0, 3, 2);
    gtk_grid_attach(GTK_GRID(grid), textview2, 0, 2, 3, 2);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button3, 2, 4, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);
    gtk_main();

    return 0;
  }
