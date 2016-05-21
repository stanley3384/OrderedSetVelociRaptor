
/*

    Derive a simple object from the gtk drawing area widget. All it does is draw a smiley face that
you can change the color of. Put the drawing.h, drawing.c and drawing_main.c in the working folder
and compile. 

    gcc -Wall drawing.c drawing_main.c -o drawing_main `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu14.04, GTK3.10.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "drawing.h"

static void change_color(GtkWidget *button, gpointer *data)
{
  gint i=0;
  gdouble color_rgba[4]={1.0, 0.0, 1.0, 1.0};
  color_rgba[0]=g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(data[0])), NULL);
  color_rgba[1]=g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(data[1])), NULL);
  color_rgba[2]=g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(data[2])), NULL);
  color_rgba[3]=g_ascii_strtod(gtk_entry_get_text(GTK_ENTRY(data[3])), NULL);
  
  for(i=0;i<4;i++) 
    {
      if(color_rgba[i]<0.0||color_rgba[i]>1.0) color_rgba[i]=0.0;
    }
  
  smiley_drawing_set_color(SMILEY_DRAWING(data[4]), color_rgba);
}
static void color_changed1(GtkWidget *widget, gpointer data)
{
  g_print("Color Changed\n");
}
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
  g_signal_connect(smiley_drawing, "color-changed", G_CALLBACK(color_changed1), NULL);

  //Test with g_object_set and g_object_get.
  gdouble red=1.0, green=1.0, blue=1.0, alpha=1.0;
  g_object_set(smiley_drawing, "Red", 0.0, "Green", 1.0, "Blue", 0.0, "Alpha", 1.0, NULL);
  g_object_get(smiley_drawing, "Red", &red, "Green", &green, "Blue", &blue, "Alpha", &alpha, NULL);
  g_print("Test Color1 %f, %f, %f, %f\n", red, green, blue, alpha);

  //Test with and draw with set_color.
  gdouble color_rgba[4]={1.0, 0.0, 1.0, 1.0};
  smiley_drawing_set_color(SMILEY_DRAWING(smiley_drawing), color_rgba);
  gchar *color=smiley_drawing_get_color(SMILEY_DRAWING(smiley_drawing));
  g_print("Test Color2 %s\n", color);
  g_free(color);

  //Test the widget name.
  g_print("Widget Name: %s, Smiley Name: %s\n", gtk_widget_get_name(smiley_drawing), smiley_drawing_get_name(SMILEY_DRAWING(smiley_drawing)));
  smiley_drawing_set_name(SMILEY_DRAWING(smiley_drawing), "Dave");
  g_print("Widget Name: %s, Smiley Name: %s\n", gtk_widget_get_name(smiley_drawing), smiley_drawing_get_name(SMILEY_DRAWING(smiley_drawing)));
  g_object_set(smiley_drawing, "SmileyName", "Good Drawing", NULL);
  g_print("%s\n", smiley_drawing_get_name(SMILEY_DRAWING(smiley_drawing))); 

  GtkWidget *red_label=gtk_label_new("Red");

  GtkWidget *red_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(red_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(red_entry), "1.0");

  GtkWidget *green_label=gtk_label_new("Green");

  GtkWidget *green_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(green_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(green_entry), "0.0");

  GtkWidget *blue_label=gtk_label_new("Blue");

  GtkWidget *blue_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(blue_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(blue_entry), "1.0");

  GtkWidget *alpha_label=gtk_label_new("Alpha");

  GtkWidget *alpha_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(alpha_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(alpha_entry), "1.0");

  GtkWidget *button=gtk_button_new_with_label("Change Color");

  gpointer entries[]={red_entry, green_entry, blue_entry, alpha_entry, smiley_drawing};
  g_signal_connect(button, "clicked", G_CALLBACK(change_color), entries);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), smiley_drawing, 0, 0, 8, 4);
  gtk_grid_attach(GTK_GRID(grid), red_label, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), red_entry, 1, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), green_label, 2, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), green_entry, 3, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), blue_label, 4, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), blue_entry, 5, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), alpha_label, 6, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), alpha_entry, 7, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button, 2, 5, 4, 1);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

