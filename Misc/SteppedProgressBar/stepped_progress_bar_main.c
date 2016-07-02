
/*

    Work on making a stepped progress bar widget. It is currently under construction. This is
moving the da_progress1.c progress bar drawings into a widget. It isn't functional yet. Many things
still need to be connected and functions created.

    gcc -Wall -Werror stepped_progress_bar.c stepped_progress_bar_main.c -o stepped_progress `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu14.04, GTK3.10.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "stepped_progress_bar.h"

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
  
  stepped_progress_bar_set_color(STEPPED_PROGRESS_BAR(data[4]), color_rgba);
}
static void color_changed1(GtkWidget *widget, gpointer data)
{
  g_print("Color Changed\n");
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Stepped Progress Bar");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *stepped_progress_bar=stepped_progress_bar_new(0);
  gtk_widget_set_hexpand(stepped_progress_bar, TRUE);
  gtk_widget_set_vexpand(stepped_progress_bar, TRUE);
  g_signal_connect(stepped_progress_bar, "color-changed", G_CALLBACK(color_changed1), NULL);

  //Test with g_object_set and g_object_get.
  gdouble red=1.0, green=1.0, blue=1.0, alpha=1.0;
  g_object_set(stepped_progress_bar, "Red", 0.0, "Green", 1.0, "Blue", 0.0, "Alpha", 1.0, NULL);
  g_object_get(stepped_progress_bar, "Red", &red, "Green", &green, "Blue", &blue, "Alpha", &alpha, NULL);
  g_print("Test Color1 %f, %f, %f, %f\n", red, green, blue, alpha);

  //Test with and draw with set_color.
  gdouble color_rgba[4]={1.0, 0.0, 1.0, 1.0};
  stepped_progress_bar_set_color(STEPPED_PROGRESS_BAR(stepped_progress_bar), color_rgba);
  gchar *color=stepped_progress_bar_get_color(STEPPED_PROGRESS_BAR(stepped_progress_bar));
  g_print("Test Color2 %s\n", color);
  g_free(color);

  //Test the widget name.
  g_print("Widget Name: %s, Progress Name: %s\n", gtk_widget_get_name(stepped_progress_bar), stepped_progress_bar_get_name(STEPPED_PROGRESS_BAR(stepped_progress_bar)));
  stepped_progress_bar_set_name(STEPPED_PROGRESS_BAR(stepped_progress_bar), "Dave");
  g_print("Widget Name: %s, Progress Name: %s\n", gtk_widget_get_name(stepped_progress_bar), stepped_progress_bar_get_name(STEPPED_PROGRESS_BAR(stepped_progress_bar)));
  g_object_set(stepped_progress_bar, "ProgressName", "Good Drawing", NULL);
  g_print("%s\n", stepped_progress_bar_get_name(STEPPED_PROGRESS_BAR(stepped_progress_bar))); 

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

  gpointer entries[]={red_entry, green_entry, blue_entry, alpha_entry, stepped_progress_bar};
  g_signal_connect(button, "clicked", G_CALLBACK(change_color), entries);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), stepped_progress_bar, 0, 0, 8, 4);
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

