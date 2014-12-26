
/*

    Test a GtkImage. The images need to be supplied and file names changed accordingly. Use a
viewport and scrolled window to scroll the image. 

gcc -Wall image1.c -o image1 `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon
*/

#include<gtk/gtk.h>

static void open_button1_image(GtkWidget *button, GtkWidget *image)
 {
    g_print("Button1 Clicked\n");
    gtk_image_clear(GTK_IMAGE(image));
    gtk_image_set_from_file(GTK_IMAGE(image), "test1.svg");
 }
static void open_button2_image(GtkWidget *button, GtkWidget *image)
 {
    g_print("Button2 Clicked\n");
    gtk_image_clear(GTK_IMAGE(image));
    gtk_image_set_from_file(GTK_IMAGE(image), "test2.png");
 }
int main(int argc, char **argv)
 {
   GtkWidget *window, *button1, *button2, *scrolled_win, *view_port, *image, *grid;

   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Get Image");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   image=gtk_image_new();
   gtk_widget_set_vexpand(image, TRUE);
   gtk_widget_set_hexpand(image, TRUE);

   view_port=gtk_viewport_new(NULL, NULL);
   gtk_container_add(GTK_CONTAINER(view_port), image);
   scrolled_win=gtk_scrolled_window_new(NULL, NULL);
   gtk_container_add(GTK_CONTAINER(scrolled_win), view_port);

   button1=gtk_button_new_with_label("Open Button1 Image");
   gtk_widget_set_hexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(open_button1_image), image); 

   button2=gtk_button_new_with_label("Open Button2 Image");
   gtk_widget_set_hexpand(button2, TRUE);
   g_signal_connect(button2, "clicked", G_CALLBACK(open_button2_image), image); 

   grid=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), scrolled_win, 0, 2, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;
  }
   
