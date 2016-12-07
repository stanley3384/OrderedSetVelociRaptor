
/*

    See about setting up CSS with a GTK+ widget. This started from the gtkmm tutorial for 
custom widgets which is very good.

    https://developer.gnome.org/gtkmm-tutorial/stable/sec-custom-widgets.html.en

    Since the GTK drawing area is a widget, with the functions that you need to override when 
deriving from a widget already taken care of, it makes the code simpler.
 
    gcc -Wall penrose.c penrose_main.c -o penrose `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "penrose.h"

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Penrose Triangle Drawing");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  //Default drawing. No CSS.
  GtkWidget *penrose1=penrose_triangle_drawing_new();
  gtk_widget_set_hexpand(penrose1, TRUE);
  gtk_widget_set_vexpand(penrose1, TRUE);

  //Color with CSS by name.
  GtkWidget *penrose2=penrose_triangle_drawing_new();
  gtk_widget_set_hexpand(penrose2, TRUE);
  gtk_widget_set_vexpand(penrose2, TRUE);
  gtk_widget_set_name(penrose2, "penrose2");

  GtkWidget *penrose3=penrose_triangle_drawing_new();
  gtk_widget_set_hexpand(penrose3, TRUE);
  gtk_widget_set_vexpand(penrose3, TRUE);
  gtk_widget_set_name(penrose3, "penrose3");

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), penrose1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), penrose2, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), penrose3, 0, 2, 1, 1);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

