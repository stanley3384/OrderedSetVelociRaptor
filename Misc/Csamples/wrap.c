

/*

  Test some word wrapping. From GTK+ Forum.

  gcc -Wall wrap.c -o wrap `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon
*/

#include<gtk/gtk.h>

static void button_clicked(GtkWidget *widget, GArray *widgets)
 {
   g_print("Clicked\n");
   const gchar *string=gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 1)));
   gtk_label_set_text(GTK_LABEL(g_array_index(widgets, GtkWidget*, 0)), string);
 }
int main(int argc, char **argv)
 {
   GtkWidget *window, *entry1, *button1, *messageCrypte, *cryptTab;

   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Label Wrap");
   gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 10);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   messageCrypte=gtk_label_new("");
   //Use margin-start and margin-end for GTK >= 3.12
   gtk_widget_set_margin_right(messageCrypte, 200);
   gtk_widget_set_size_request(messageCrypte, 50, 100);
   gtk_label_set_line_wrap(GTK_LABEL(messageCrypte), TRUE);

   entry1=gtk_entry_new();
   gtk_widget_set_hexpand(entry1, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry1), "See if this text wraps.");

   GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
   g_array_append_val(widgets, messageCrypte);
   g_array_append_val(widgets, entry1);

   button1=gtk_button_new_with_label("Update Label");
   gtk_widget_set_hexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(button_clicked), widgets); 

   cryptTab=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(cryptTab), messageCrypte, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(cryptTab), entry1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(cryptTab), button1, 0, 2, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), cryptTab);

   gtk_widget_show_all(window);

   gtk_main();
   return 0;
  }
