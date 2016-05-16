
/*

    Derive a simple object from the gtk drawing area widget. All it does is draw a smiley face that
you can change the color of. Put the drawing.h, drawing.c and drawing_main.c in the working folder
and compile. 

    gcc -Wall drawing.c drawing_main.c -o drawing_main `pkg-config gtk+-3.0 --cflags --libs`

    C. Eric Cashon

*/
#include<gtk/gtk.h>
#include "drawing.h"

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Smiley Drawing");
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *smiley_drawing=smiley_drawing_new();
  gtk_widget_set_hexpand(smiley_drawing, TRUE);
  gtk_widget_set_vexpand(smiley_drawing, TRUE);

  //Test with g_object_set and g_object_get.
  gdouble red=1.0, green=1.0, blue=1.0, alpha=1.0;
  g_object_set(smiley_drawing, "Red", 0.0, "Green", 1.0, "Blue", 0.0, "Alpha", 1.0, NULL);
  g_object_get(smiley_drawing, "Red", &red, "Green", &green, "Blue", &blue, "Alpha", &alpha, NULL);
  g_print("Test Color1 %f, %f, %f, %f\n", red, green, blue, alpha);

  //Test with set_color.
  gdouble color_rgba[4]={1.0, 0.0, 1.0, 1.0};
  smiley_drawing_set_color(SMILEY_DRAWING(smiley_drawing), color_rgba);
  gchar *color=smiley_drawing_get_color(SMILEY_DRAWING(smiley_drawing));
  g_print("Test Color2 %s\n", color);
  g_free(color);

  gtk_container_add(GTK_CONTAINER(window), smiley_drawing);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

