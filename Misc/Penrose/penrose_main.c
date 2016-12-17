
/*

    See about setting up CSS with a GTK+ widget. This started from the gtkmm tutorial for 
custom widgets which is very good. Look at setting background and foreground colors with
CSS or accessor functions.

    https://developer.gnome.org/gtkmm-tutorial/stable/sec-custom-widgets.html.en

    Since the GTK drawing area is a widget, with the functions that you need to override when 
deriving from a widget already taken care of, it makes the code simpler to test the CSS.
 
    gcc -Wall penrose.c penrose_main.c -o penrose `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "penrose.h"

/*
    Check what gtk_style_context_get_background_color() returns. It is deprecated but it
looks like it will return a solid color for the widget if one is present. Otherwise it 
returns 0's for the color if GtkDrawingArea isn't set in CSS.
*/
static void get_colors(GtkWidget *button, GtkWidget **widgets)
{
  gint i=0;
  GdkRGBA color;
  GtkStyleContext *context;
  for(i=0;widgets[i]!=NULL;i++)
    {
      //Reset colors.
      color.red=1.0; color.green=1.0; color.blue=1.0; color.alpha=1.0;
      context=gtk_widget_get_style_context(widgets[i]);
      G_GNUC_BEGIN_IGNORE_DEPRECATIONS
      gtk_style_context_get_background_color(context, GTK_STATE_FLAG_NORMAL, &color);
      G_GNUC_END_IGNORE_DEPRECATIONS
      g_print("penrose%i %f %f %f %f\n", i+1, color.red, color.green, color.blue, color.alpha);
    }
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Penrose Triangle Drawing");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  //Color with CSS with the parent GtkDrawingArea widget.
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

  //Set color with accessor functions instead of CSS.
  GtkWidget *penrose4=penrose_triangle_drawing_new();
  gtk_widget_set_hexpand(penrose4, TRUE);
  gtk_widget_set_vexpand(penrose4, TRUE);
  penrose_triangle_drawing_set_background(PENROSE_TRIANGLE_DRAWING(penrose4), "rgba(0, 255, 0, 1.0)");
  penrose_triangle_drawing_set_foreground(PENROSE_TRIANGLE_DRAWING(penrose4), "rgba(0, 0, 255, 1.0)");
  g_print("penrose4 background %s\n", penrose_triangle_drawing_get_background(PENROSE_TRIANGLE_DRAWING(penrose4)));
  gchar *string=NULL;
  g_object_get(penrose4, "foreground", &string, NULL);
  g_print("penrose4 foreground %s\n", string);
  g_free(string);

  GtkWidget *button=gtk_button_new_with_label("Get background colors");
  gtk_widget_set_hexpand(button, TRUE);
  GtkWidget *widgets[]={penrose1, penrose2, penrose3, penrose4, NULL};
  g_signal_connect(button, "clicked", G_CALLBACK(get_colors), widgets);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), penrose1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), penrose2, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), penrose3, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), penrose4, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 4, 1, 1);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

