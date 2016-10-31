
/*

    A basic stepped progress bar widget. This is the result of moving the da_progress1.c progress
bar drawings into a widget.

    gcc -Wall -Werror stepped_progress_bar.c stepped_progress_bar_main.c -o stepped_progress `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "stepped_progress_bar.h"
#include<stdlib.h>

static void change_settings(GtkWidget *button, GtkWidget *widgets[])
{
  //Test some functions and properties of the SteppedProgressBar widget.
  gint direction=gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[0]));
  gint steps=atoi(gtk_entry_get_text(GTK_ENTRY(widgets[1])));
  gint step_stop=atoi(gtk_entry_get_text(GTK_ENTRY(widgets[2])));

  g_print("Change Settings %i %i\n", direction, steps);
  stepped_progress_bar_set_progress_direction(STEPPED_PROGRESS_BAR(widgets[3]), direction);
  stepped_progress_bar_set_steps(STEPPED_PROGRESS_BAR(widgets[3]), steps);  
  stepped_progress_bar_set_step_stop(STEPPED_PROGRESS_BAR(widgets[3]), step_stop);

  gint a1=stepped_progress_bar_get_progress_direction(STEPPED_PROGRESS_BAR(widgets[3]));
  gint a2=stepped_progress_bar_get_steps(STEPPED_PROGRESS_BAR(widgets[3]));
  gint a3=stepped_progress_bar_get_step_stop(STEPPED_PROGRESS_BAR(widgets[3]));
  g_print("Settings directon %i, steps %i, step_stop %i\n", a1, a2, a3);

  //stepped_progress_bar_set_background_rgba1(STEPPED_PROGRESS_BAR(widgets[3]), "rgba(0, 0, 0, 255)");
  //stepped_progress_bar_set_background_rgba2(STEPPED_PROGRESS_BAR(widgets[3]), "rgba(0, 0, 0, 255)");
  //stepped_progress_bar_set_foreground_rgba1(STEPPED_PROGRESS_BAR(widgets[3]), "rgba(0, 255, 255, 255)");
  //stepped_progress_bar_set_foreground_rgba2(STEPPED_PROGRESS_BAR(widgets[3]), "rgba(0, 0, 255, 255)");

  const gchar *background_rgba1=stepped_progress_bar_get_background_rgba1(STEPPED_PROGRESS_BAR(widgets[3]));
  const gchar *background_rgba2=stepped_progress_bar_get_background_rgba2(STEPPED_PROGRESS_BAR(widgets[3]));
  const gchar *foreground_rgba1=stepped_progress_bar_get_foreground_rgba1(STEPPED_PROGRESS_BAR(widgets[3]));
  const gchar *foreground_rgba2=stepped_progress_bar_get_foreground_rgba2(STEPPED_PROGRESS_BAR(widgets[3]));
  g_print("Colors %s %s %s %s\n", background_rgba1, background_rgba2, foreground_rgba1, foreground_rgba2);
}
static void step_changed(GtkWidget *widget, gpointer data)
{
  g_print("Step Changed\n");
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Stepped Progress Bar");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *stepped_progress_bar=stepped_progress_bar_new();
  gtk_widget_set_hexpand(stepped_progress_bar, TRUE);
  gtk_widget_set_vexpand(stepped_progress_bar, TRUE);
  g_signal_connect(stepped_progress_bar, "step-changed", G_CALLBACK(step_changed), NULL);

  GtkWidget *direction_combo=gtk_combo_box_text_new();
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(direction_combo), 0, "1", "HORIZONTAL_RIGHT");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(direction_combo), 1, "2", "VERTICAL_UP");
  gtk_widget_set_hexpand(direction_combo, TRUE);  
  gtk_combo_box_set_active(GTK_COMBO_BOX(direction_combo), 1);

  GtkWidget *steps_label=gtk_label_new("Steps");
  gtk_widget_set_hexpand(steps_label, TRUE);

  GtkWidget *steps_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(steps_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(steps_entry), "20");

  GtkWidget *step_stop_label=gtk_label_new("Step Stop");
  gtk_widget_set_hexpand(step_stop_label, TRUE);

  GtkWidget *step_stop_entry=gtk_entry_new();
  gtk_entry_set_width_chars(GTK_ENTRY(step_stop_entry), 4);
  gtk_entry_set_text(GTK_ENTRY(step_stop_entry), "10");

  GtkWidget *button=gtk_button_new_with_label("Change Settings");
  gtk_widget_set_hexpand(button, TRUE);

  GtkWidget *widgets[]={direction_combo, steps_entry, step_stop_entry, stepped_progress_bar};
  g_signal_connect(button, "clicked", G_CALLBACK(change_settings), widgets);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), stepped_progress_bar, 0, 0, 6, 4);
  gtk_grid_attach(GTK_GRID(grid), direction_combo, 0, 4, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), steps_label, 2, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), steps_entry, 3, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), step_stop_label, 4, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), step_stop_entry, 5, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 5, 6, 1);

  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

