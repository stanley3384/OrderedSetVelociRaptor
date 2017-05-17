
/*

    An adjustable gauge widget. There are two cutoff points that can be set along with the 
range of the scale.
    There are two types of gauges. One is called a voltage gauge and the other is a speedometer 
gauge. The range is not enforced.  They can go above 10,000 or below -10,000 but the number formatting
in the speedometer gets out of place. Also the speedometer numbers are displayed as ints and
the voltage gauge is displayed as a float with two decimal places. These things can easily be 
changed if need be.
    The gauges can be set with a gradient or with solid colors. The draw time for gradients is
longer so they might not be suitable for using with a frame clock. 
    The start for this widget can be found in da_gauge1.c and da_speedometer1.c in the
cairo_drawings folder. When setting properties, make sure to set the range or top and
bottom values first.

    gcc -Wall -Werror adjustable_gauge.c adjustable_gauge_main.c -o gauge `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

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
  gboolean check=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[7]));

  g_print("%f %f %f %f %f\n", cutoff1, cutoff2, needle, bottom, top);

  //Set the gauge to draw.
  if(gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[6]))==0)
    {
      adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(widgets[0]), VOLTAGE_GAUGE);
    }
  else
    {
      adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(widgets[0]), SPEEDOMETER_GAUGE);
    }
  //Then set scale.
  adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(widgets[0]), bottom);
  adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(widgets[0]), top);
  //Then set needle and cutoffs.  
  adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(widgets[0]), cutoff1);
  adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(widgets[0]), cutoff2);
  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[0]), needle);
  adjustable_gauge_set_draw_gradient(ADJUSTABLE_GAUGE(widgets[0]), check);
  
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Adjustable Gauge");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *gauge=adjustable_gauge_new();
  adjustable_gauge_set_background(ADJUSTABLE_GAUGE(gauge), "rgba(255, 255, 255, 1.0)");
  adjustable_gauge_set_text_color(ADJUSTABLE_GAUGE(gauge), "rgba(0, 0, 0, 1.0)");
  adjustable_gauge_set_needle_color(ADJUSTABLE_GAUGE(gauge), "rgba(0, 0, 0, 1.0)");

  /*Test setting some colors in the gauge.
  adjustable_gauge_set_background(ADJUSTABLE_GAUGE(gauge), "rgba(255, 0, 255, 1.0)");
  adjustable_gauge_set_text_color(ADJUSTABLE_GAUGE(gauge), "rgba(255, 255, 0, 1.0)");
  adjustable_gauge_set_arc_color1(ADJUSTABLE_GAUGE(gauge), "rgba(0, 200, 0, 1.0)");
  adjustable_gauge_set_arc_color2(ADJUSTABLE_GAUGE(gauge), "rgba(200, 200, 0, 1.0)");
  adjustable_gauge_set_arc_color3(ADJUSTABLE_GAUGE(gauge), "rgba(200, 0, 0, 1.0)");
  adjustable_gauge_set_needle_color(ADJUSTABLE_GAUGE(gauge), "rgba(150, 150, 0, 1.0)");
  g_print("%s, %s, %s, %s, %s, %s\n", adjustable_gauge_get_background(ADJUSTABLE_GAUGE(gauge)), adjustable_gauge_get_text_color(ADJUSTABLE_GAUGE(gauge)), adjustable_gauge_get_arc_color1(ADJUSTABLE_GAUGE(gauge)), adjustable_gauge_get_arc_color2(ADJUSTABLE_GAUGE(gauge)), adjustable_gauge_get_arc_color3(ADJUSTABLE_GAUGE(gauge)), adjustable_gauge_get_needle_color(ADJUSTABLE_GAUGE(gauge)));
  */  
  gtk_widget_set_hexpand(gauge, TRUE);
  gtk_widget_set_vexpand(gauge, TRUE);

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

  GtkWidget *combo=gtk_combo_box_text_new();
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "VOLTAGE_GAUGE");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "SPEEDOMETER_GAUGE");
  gtk_widget_set_hexpand(combo, TRUE);  
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);

  GtkWidget *button=gtk_button_new_with_label("Change Settings");
  gtk_widget_set_hexpand(button, TRUE);

  GtkWidget *check1=gtk_check_button_new_with_label("Color Gradient");
  gtk_widget_set_halign(check1, GTK_ALIGN_CENTER);

  GtkWidget *widgets[]={gauge, cutoff1_entry, cutoff2_entry, needle_entry, bottom_entry, top_entry, combo, check1};
  g_signal_connect(button, "clicked", G_CALLBACK(change_settings), widgets);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), cutoff1_label, 0, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), cutoff1_entry, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), cutoff2_label, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), cutoff2_entry, 1, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), needle_label, 0, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), needle_entry, 1, 2, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), bottom_label, 0, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), bottom_entry, 1, 3, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), top_label, 0, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), top_entry, 1, 4, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo, 0, 5, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), check1, 0, 6, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), button, 0, 7, 2, 1);

  GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
  gtk_paned_pack2(GTK_PANED(paned1), gauge, TRUE, TRUE);
  gtk_paned_set_position(GTK_PANED(paned1), 300);
   
  gtk_container_add(GTK_CONTAINER(window), paned1);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

