
/*

   Test code for trying out GtkApplication along with putting an icon on the Ubuntu Unity Launcher.
The code needs a png file in order to work. Change the icon_test.png to a valid png.

   gcc -Wall gtk_app1.c -o gtk_app1 `pkg-config --cflags --libs gtk+-3.0`

   C. Eric Cashon
*/

#include<gtk/gtk.h>

static void activate(GtkApplication *app, gpointer user_data)
 {
   GtkWidget *window, *label1, *grid;
   GError *error = NULL;

   window=gtk_application_window_new(GTK_APPLICATION(app));
   gtk_window_set_title(GTK_WINDOW(window), "Application Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 100);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   //icon gets placed on the launcher. Need to supply a valid png in the working folder.
   GdkPixbuf *pixbuf=gdk_pixbuf_new_from_file("icon_test.png", &error);
   if(!pixbuf)
     {
       g_print("%s\n", error->message);
       g_error_free(error);
     }
   else
     {
       gtk_window_set_icon(GTK_WINDOW(window), pixbuf);
     }
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);

   label1=gtk_label_new("GTK+ Application with Icon\n in Launcher");
   gtk_widget_set_hexpand(label1, TRUE);

   grid=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);
 }
int main(int argc, char **argv)
 {
   GtkApplication *app;
   int status;
   app = gtk_application_new("app.example", G_APPLICATION_FLAGS_NONE);
   g_signal_connect(app, "activate", G_CALLBACK(activate), NULL);
   status=g_application_run(G_APPLICATION(app), argc, argv);
   g_object_unref(app);
   return(status);
  }
