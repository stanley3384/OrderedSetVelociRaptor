
/*

  A simple menu for comparison with gtk_app1.c

  gcc -Wall gtk_menu1.c -o gtk_menu1 `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon

*/

#include<gtk/gtk.h>

static void callback_test1(GtkWidget *menu, gpointer data)
 {
   g_print("Test1 Callback\n");
 }
static void callback_test2(GtkWidget *menu, gpointer data)
 {
   g_print("Test2 Callback\n");
 }
int main(int argc, char **argv)
 {
   GtkWidget *menu_bar, *data_item, *data_menu, *test1, *test2, *window, *label1, *grid;

   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Application Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 100);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   data_menu=gtk_menu_new();
   test1=gtk_menu_item_new_with_label("Test1");
   test2=gtk_menu_item_new_with_label("Test2");
   gtk_menu_shell_append(GTK_MENU_SHELL(data_menu), test1);
   gtk_menu_shell_append(GTK_MENU_SHELL(data_menu), test2);
   g_signal_connect(test1, "activate", G_CALLBACK(callback_test1), NULL);
   g_signal_connect(test2, "activate", G_CALLBACK(callback_test2), NULL);
   menu_bar=gtk_menu_bar_new();
   data_item=gtk_menu_item_new_with_label("Data");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(data_item), data_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), data_item);

   label1=gtk_label_new("GtkMenu");
   gtk_widget_set_hexpand(label1, TRUE);

   grid=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();
   return 0;
  
 }
