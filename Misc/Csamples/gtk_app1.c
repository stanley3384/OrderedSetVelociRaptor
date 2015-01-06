
/*

   Test code for trying out a GtkApplication object with a menu along with putting an icon
in the Ubuntu Unity Launcher.

   gcc -Wall gtk_app1.c -o gtk_app1 `pkg-config --cflags --libs gtk+-3.0`

   C. Eric Cashon
*/

#include<gtk/gtk.h>

static void quit_program(GSimpleAction *action, GVariant *parameter, gpointer user_data)
 {
   g_application_quit(G_APPLICATION(user_data));
 }
static void test1_callback(GSimpleAction *action, GVariant *parameter, gpointer user_data)
 {
   g_print("Test1 Callback\n");
 }
static void app_startup(GtkApplication *app, gpointer user_data)
 {
   //Setup menu callbacks.
   GSimpleAction *test1;
   GSimpleAction *quit;
   test1 = g_simple_action_new("test1", NULL);
   g_signal_connect(test1, "activate", G_CALLBACK(test1_callback), app);
   g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(test1));
   g_object_unref(test1);
   quit = g_simple_action_new("quit", NULL);
   g_signal_connect(quit, "activate", G_CALLBACK(quit_program), app);
   g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(quit));
   g_object_unref(quit);

   //Setup menu.
   GMenu *menu;
   GMenu *submenu;  
   GMenu *section;  
   menu = g_menu_new();
   submenu = g_menu_new();
   g_menu_append_submenu(menu, "Application", G_MENU_MODEL(submenu));
   section = g_menu_new();
   g_menu_append_section(submenu, NULL, G_MENU_MODEL(section));
   g_menu_append(section, "Test1", "app.test1");
   g_menu_append(section, "Quit", "app.quit");
   g_object_unref(submenu);
   g_object_unref(section);

   gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menu));
   g_object_unref(menu);
 }
static void app_activate(GtkApplication *app, gpointer user_data)
 {
   GtkWidget *window, *label1, *grid;
   GError *error = NULL;

   window=gtk_application_window_new(GTK_APPLICATION(app));
   gtk_window_set_title(GTK_WINDOW(window), "Application Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 100);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);
   //icon gets placed on the launcher. Need to supply a picture file in the working folder.
   GdkPixbuf *pixbuf=gdk_pixbuf_new_from_file("dino.svg", &error);
   if(!pixbuf)
     {
       g_print("%s\n", error->message);
       g_error_free(error);
     }
   else
     {
       gtk_window_set_icon(GTK_WINDOW(window), pixbuf);
     }

   label1=gtk_label_new("GMenu with icon");
   gtk_widget_set_hexpand(label1, TRUE);

   grid=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);
 }
int main(int argc, char **argv)
 {
   GtkApplication *app;
   int status;
   app = gtk_application_new("app.example", G_APPLICATION_FLAGS_NONE);
   g_signal_connect(app, "startup", G_CALLBACK(app_startup), NULL);
   g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
   status=g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);
   return(status);
  }
