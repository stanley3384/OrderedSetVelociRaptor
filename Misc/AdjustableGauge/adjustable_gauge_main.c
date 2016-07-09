
/*

    An adjustable gauge widget. There are two cutoff points that can be set along with the 
range of the scale. No colors are set with funcitons but you can change them in the code
easy enough. This needs some more work and testing but the basic functionality is working.
    The start for this widget can be found in da_gauge.c in the cairo_drawings folder.

    gcc -Wall -Werror adjustable_gauge.c adjustable_gauge_main.c -o gauge `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu14.04, GTK3.10.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "adjustable_gauge.h"
#include<stdlib.h>

static void change_settings(GtkWidget *button, GtkWidget *widgets[])
{
  //Test some functions and properties of the AdjustableGauge widget.
  gdouble cutoff1=atof(gtk_entry_get_text(GTK_ENTRY(widgets[1])));
  gdouble cutoff2=atof(gtk_entry_get_text(GTK_ENTRY(widgets[2])));
  gdouble needle=atof(gtk_entry_get_text(GTK_ENTRY(widgets[3])));
  gdouble bottom=atof(gtk_entry_get_text(GTK_ENTRY(widgets[4])));
  gdouble top=atof(gtk_entry_get_text(GTK_ENTRY(widgets[5])));

  g_print("%f %f %f %f %f\n", cutoff1, cutoff2, needle, bottom, top);

  adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(widgets[0]), cutoff1);
  adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(widgets[0]), cutoff2);
  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[0]), needle);
  adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(widgets[0]), bottom);
  adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(widgets[0]), top);  
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Adjustable Gauge");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *adjustable_gauge=adjustable_gauge_new();
  gtk_widget_set_hexpand(adjustable_gauge, TRUE);
  gtk_widget_set_vexpand(adjustable_gauge, TRUE);

  GtkWidget *cutoff1_label=gtk_label_new("First Cutoff");
  gtk_widget_set_hexpand(cutoff1_label, TRUE);

  GtkWidget *cutoff1_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(cutoff1_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(cutoff1_entry), "80");

  GtkWidget *cutoff2_label=gtk_label_new("Second Cutoff");
  gtk_widget_set_hexpand(cutoff2_label, TRUE);

  GtkWidget *cutoff2_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(cutoff2_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(cutoff2_entry), "90");

  GtkWidget *needle_label=gtk_label_new("Needle");
  gtk_widget_set_hexpand(needle_label, TRUE);

  GtkWidget *needle_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(needle_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(needle_entry), "0");

  GtkWidget *bottom_label=gtk_label_new("Bottom");
  gtk_widget_set_hexpand(bottom_label, TRUE);

  GtkWidget *bottom_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(bottom_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(bottom_entry), "0");

  GtkWidget *top_label=gtk_label_new("Top");
  gtk_widget_set_hexpand(top_label, TRUE);

  GtkWidget *top_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(top_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(top_entry), "100");

  GtkWidget *button=gtk_button_new_with_label("Change Settings");
  gtk_widget_set_hexpand(button, TRUE);

  GtkWidget *widgets[]={adjustable_gauge, cutoff1_entry, cutoff2_entry, needle_entry, bottom_entry, top_entry};
  g_signal_connect(button, "clicked", G_CALLBACK(change_settings), widgets);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), adjustable_gauge, 0, 0, 4, 4);
  gtk_grid_attach(GTK_GRID(grid), cutoff1_label, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), cutoff1_entry, 1, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), cutoff2_label, 2, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), cutoff2_entry, 3, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), needle_label, 0, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), needle_entry, 1, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), bottom_label, 0, 6, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), bottom_entry, 1, 6, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), top_label, 2, 6, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), top_entry, 3, 6, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 7, 6, 1);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

