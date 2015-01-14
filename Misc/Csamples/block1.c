
/*

  Block repeated function calls from rapidly clicking on a button. Design problem with how
the UI responds.

  gcc -Wall block1.c -o block1 `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon

*/

#include<gtk/gtk.h>

static void button1_callback(GtkWidget *button1, gpointer data)
 {
   g_print("Button1 Clicked\n");
   guint signal_id=g_signal_lookup("clicked", GTK_TYPE_BUTTON);
   gulong handler_id=handler_id=g_signal_handler_find( (gpointer)button1, G_SIGNAL_MATCH_ID, signal_id, 0, NULL, NULL, NULL );
   g_print("Signal %i Handler %lu\n", signal_id, handler_id);
   g_signal_handler_block(button1, handler_id);
   
   g_print("Wait...\n");
   g_usleep(2000000);
   g_print("Return\n");

   while(gtk_events_pending ()) gtk_main_iteration ();
   g_signal_handler_unblock(button1, handler_id);
 }
static void button2_callback(GtkWidget *button1, gpointer data)
 {
   g_print("Button2 Clicked\n");
 }
int main(int argc, char **argv)
 {
   GtkWidget *window, *button1, *button2, *grid;

   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Block Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   button1=gtk_button_new_with_label("button1");
   gtk_widget_set_hexpand(button1, TRUE);
   gtk_widget_set_vexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(button1_callback), NULL);

   button2=gtk_button_new_with_label("button2");
   gtk_widget_set_hexpand(button2, TRUE);
   gtk_widget_set_vexpand(button2, TRUE);
   g_signal_connect(button2, "clicked", G_CALLBACK(button2_callback), NULL);

   grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();
   return 0;
  
 }
