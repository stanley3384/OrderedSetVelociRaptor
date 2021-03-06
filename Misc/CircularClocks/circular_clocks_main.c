
/*

    A clock widget. Seven different types of clocks to choose from. This is making a widget out of the clocks1.c code in the cairo_drawings folder along with a few others. There are only two accessor 
functions. One to change the  name of the clock and the other to change the background color. Could
always change clock colors and other properties in the code also.

    gcc -Wall circular_clocks.c circular_clocks_main.c -o circular_clocks `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "circular_clocks.h"

//Draw the main window transparent so the clock background can be transparent.
static gboolean draw_main_window(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
  cairo_paint(cr);
  return FALSE;
}
//Put the other widgets in an event box and draw the event box background.
static gboolean draw_event(GtkWidget *widget, cairo_t *cr, gpointer data)
{
  cairo_set_source_rgb(cr, 0.8, 0.8, 0.9);
  cairo_paint(cr);
  return FALSE;
}
static void combo1_changed(GtkComboBox *combo1, gpointer clock)
{
  gint combo_id=gtk_combo_box_get_active(combo1);
  if(combo_id==0) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GRADIENT_CLOCK);
  else if(combo_id==1) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), WAVE_CLOCK);
  else if(combo_id==2) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), BRAID_CLOCK);
  else if(combo_id==3) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GEAR_CLOCK);
  else if(combo_id==4) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GOURAUD_MESH_CLOCK);
  else if(combo_id==5) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GEM_CLOCK);
  else circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), SPACE_CLOCK);
}
static void combo2_changed(GtkComboBox *combo2, gpointer clock)
{
  gint combo_id=gtk_combo_box_get_active(combo2);
  if(combo_id==0) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GRADIENT_CLOCK);
  else if(combo_id==1) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), WAVE_CLOCK);
  else if(combo_id==2) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), BRAID_CLOCK);
  else if(combo_id==3) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GEAR_CLOCK);
  else if(combo_id==4) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GOURAUD_MESH_CLOCK);
  else if(combo_id==5) circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), GEM_CLOCK);
  else circular_clocks_set_clock(CIRCULAR_CLOCKS(clock), SPACE_CLOCK);  
}
static void change_background1(GtkWidget *button, gpointer *data[])
{
  circular_clocks_set_background_color(CIRCULAR_CLOCKS(data[0]), gtk_entry_get_text(GTK_ENTRY(data[1])));
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));
}
static void change_background2(GtkWidget *button, gpointer *data[])
{
  circular_clocks_set_background_color(CIRCULAR_CLOCKS(data[0]), gtk_entry_get_text(GTK_ENTRY(data[1])));
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Circular Clocks");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
  gtk_widget_set_app_paintable(window, TRUE);
  //Try to set transparency of main window.
  if(gtk_widget_is_composited(window))
    {
      GdkScreen *screen=gtk_widget_get_screen(window);  
      GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
      gtk_widget_set_visual(window, visual);
    }
  else g_print("Can't set window transparency.\n");
  g_signal_connect(window, "draw", G_CALLBACK(draw_main_window), NULL);

  GtkWidget *clock1=circular_clocks_new();
  gtk_widget_set_hexpand(clock1, TRUE);
  gtk_widget_set_vexpand(clock1, TRUE);

  GtkWidget *clock2=circular_clocks_new();
  circular_clocks_set_clock(CIRCULAR_CLOCKS(clock2), WAVE_CLOCK);
  circular_clocks_set_background_color(CIRCULAR_CLOCKS(clock2), "rgba(0, 255, 255, 1.0)");
  gtk_widget_set_hexpand(clock2, TRUE);
  gtk_widget_set_vexpand(clock2, TRUE);

  GtkWidget *combo1=gtk_combo_box_text_new();
  gtk_widget_set_hexpand(combo1, TRUE);
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Gradient Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Wave Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Braid Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Gear Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Gouraud Mesh Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "Gem Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 6, "7", "Space Clock");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
  g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), clock1);

  GtkWidget *combo2=gtk_combo_box_text_new();
  gtk_widget_set_hexpand(combo1, TRUE);
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Gradient Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Wave Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Braid Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Gear Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 4, "5", "Gouraud Mesh Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 5, "6", "Gem Clock");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 6, "7", "Space Clock");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 1);
  g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), clock2);

  GtkWidget *label1=gtk_label_new("Background");

  GtkWidget *label2=gtk_label_new("Background");

  GtkWidget *entry1=gtk_entry_new();
  gtk_widget_set_hexpand(entry1, TRUE);
  gtk_entry_set_text(GTK_ENTRY(entry1), "rgba(255, 255, 255, 1.0)");

  GtkWidget *entry2=gtk_entry_new();
  gtk_widget_set_hexpand(entry2, TRUE);
  gtk_entry_set_text(GTK_ENTRY(entry2), "rgba(0, 255, 255, 1.0)");

  GtkWidget *button1=gtk_button_new_with_label("Update Background1");
  gpointer widgets1[2]={clock1, entry1};
  g_signal_connect(button1, "clicked", G_CALLBACK(change_background1), widgets1);

  GtkWidget *button2=gtk_button_new_with_label("Update Background2");
  gpointer widgets2[2]={clock2, entry2};
  g_signal_connect(button2, "clicked", G_CALLBACK(change_background2), widgets2);

  GtkWidget *grid1=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid1), combo1, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid1), label1, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid1), entry1, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid1), button1, 0, 2, 2, 1);
  gtk_grid_attach(GTK_GRID(grid1), combo2, 2, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid1), label2, 2, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid1), entry2, 3, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid1), button2, 2, 2, 2, 1);

  GtkWidget *event1=gtk_event_box_new();
  gtk_container_add(GTK_CONTAINER(event1), grid1);
  g_signal_connect(event1, "draw", G_CALLBACK(draw_event), NULL);
  
  GtkWidget *grid2=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid2), clock1, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid2), clock2, 2, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid2), event1, 0, 1, 4, 1);
  
  gtk_container_add(GTK_CONTAINER(window), grid2);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

