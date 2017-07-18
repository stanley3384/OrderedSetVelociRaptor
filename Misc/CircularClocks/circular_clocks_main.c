
/*

    A clock widget. Five different types of clocks to choose from. This is making a widget out of the clocks1.c code in the cairo_drawings folder.

    gcc -Wall circular_clocks.c circular_clocks_main.c -o circular_clocks `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "circular_clocks.h"

static void combo1_changed(GtkComboBox *combo1, gpointer clock)
{
  gint combo_id=gtk_combo_box_get_active(combo1);
  if(combo_id==0) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GRADIENT_CLOCK);
  else if(combo_id==1) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), WAVE_CLOCK);
  else if(combo_id==2) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), BRAID_CLOCK);
  else if(combo_id==3) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GEAR_CLOCK);
  else circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GOURAUD_MESH_CLOCK);  
}
static void combo2_changed(GtkComboBox *combo2, gpointer clock)
{
  gint combo_id=gtk_combo_box_get_active(combo2);
  if(combo_id==0) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GRADIENT_CLOCK);
  else if(combo_id==1) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), WAVE_CLOCK);
  else if(combo_id==2) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), BRAID_CLOCK);
  else if(combo_id==3) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GEAR_CLOCK);
  else circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GOURAUD_MESH_CLOCK);  
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Circular Clocks");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 425);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *clock1=circular_clocks_new();
  gtk_widget_set_hexpand(clock1, TRUE);
  gtk_widget_set_vexpand(clock1, TRUE);

  GtkWidget *clock2=circular_clocks_new();
  circular_clocks_set_clock(CIRCULAR_CLOCKS(clock2), WAVE_CLOCK);
  gtk_widget_set_hexpand(clock2, TRUE);
  gtk_widget_set_vexpand(clock2, TRUE);

  GtkWidget *combo1=gtk_combo_box_text_new();
  gtk_widget_set_hexpand(combo1, TRUE);
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Gradient Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Wave Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Braid Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Gear Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Gouraud Mesh Clock");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
  g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), clock1);

  GtkWidget *combo2=gtk_combo_box_text_new();
  gtk_widget_set_hexpand(combo1, TRUE);
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Gradient Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Wave Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Braid Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Gear Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 4, "5", "Gouraud Mesh Clock");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 1);
  g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), clock2);
  
  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), clock1, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), clock2, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo2, 1, 1, 1, 1);
  
  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

