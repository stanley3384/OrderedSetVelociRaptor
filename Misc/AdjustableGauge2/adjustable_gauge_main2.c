
/*

    An adjustable gauge widget. The threaded version. It uses an extra thread and surface to
speed up redraws with a frame clock. The gauge drawing reuses a background drawing as long as the
window size and properties outside the needle position stay the same. This will make moving
the middle gradient fluidly not work very well. Otherwise it will draw much more efficiently.
    There are two types of gauges. One is called a voltage gauge and the other is a speedometer 
gauge. The range is not enforced.  They can go above 10,000 or below -10,000 but the number formatting
in the speedometer gets out of place. Also the speedometer numbers are displayed as ints and
the voltage gauge is displayed as a float with two decimal places. These things can easily be 
changed if need be.
    The gauges can be set with a gradient or with solid colors. The draw time for gradients is
longer so they might not be suitable for using with a frame clock.
    The start for this widget can be found in da_gauge1.c, da_speedometer1.c and circular_gradient2.c
in the cairo_drawings folder. When setting properties, make sure to set the top and
bottom range to accomadate the other property values.

    gcc -Wall -Werror adjustable_gauge2.c adjustable_gauge_main2.c -o gauge2 `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "adjustable_gauge2.h"
#include<stdlib.h>

static void change_settings(GtkWidget *button, GtkWidget *widgets[]);
static void animate(GtkWidget *widget, GtkWidget *widgets[]);
static gboolean animate_needle(GtkWidget *widgets[]);

//For animation.
static gdouble needle_pos=0;

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Adjustable Gauge Test");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *gauge=adjustable_gauge_new();
  adjustable_gauge_set_background(ADJUSTABLE_GAUGE(gauge), "rgba(255, 255, 255, 1.0)");
  adjustable_gauge_set_text_color(ADJUSTABLE_GAUGE(gauge), "rgba(0, 0, 0, 1.0)");
  adjustable_gauge_set_needle_color(ADJUSTABLE_GAUGE(gauge), "rgba(0, 0, 0, 1.0)");
 
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

  GtkWidget *combo1=gtk_combo_box_text_new();
  gtk_widget_set_hexpand(combo1, TRUE);
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Inside Radius 1.0");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Inside Radius 2.0");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Inside Radius 3.0");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 1);

  GtkWidget *combo2=gtk_combo_box_text_new();
  gtk_widget_set_hexpand(combo2, TRUE);
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Outside Radius 4.0");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Outside Radius 4.5");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Outside Radius 5.0");
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);

  GtkWidget *combo3=gtk_combo_box_text_new();
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "VOLTAGE_GAUGE");
  gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "SPEEDOMETER_GAUGE");
  gtk_widget_set_hexpand(combo3, TRUE);  
  gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);

  GtkWidget *start_label=gtk_label_new("Start Color");
  gtk_widget_set_hexpand(start_label, TRUE);

  GtkWidget *mid_label=gtk_label_new("Mid Color");
  gtk_widget_set_hexpand(mid_label, TRUE);

  GtkWidget *end_label=gtk_label_new("End Color");
  gtk_widget_set_hexpand(end_label, TRUE);

  GtkWidget *start_entry=gtk_entry_new();
  gtk_widget_set_hexpand(start_entry, TRUE);
  gtk_entry_set_text(GTK_ENTRY(start_entry), "rgba(0, 255, 0, 1.0)");

  GtkWidget *mid_entry=gtk_entry_new();
  gtk_widget_set_hexpand(mid_entry, TRUE);
  gtk_entry_set_text(GTK_ENTRY(mid_entry), "rgba(255, 255, 0, 1.0)");

  GtkWidget *end_entry=gtk_entry_new();
  gtk_widget_set_hexpand(end_entry, TRUE);
  gtk_entry_set_text(GTK_ENTRY(end_entry), "rgba(255, 0, 0, 1.0)");

  GtkWidget *check1=gtk_check_button_new_with_label("Color Gradient");
  gtk_widget_set_halign(check1, GTK_ALIGN_CENTER);

  GtkWidget *button1=gtk_button_new_with_label("Change Settings");
  gtk_widget_set_hexpand(button1, TRUE);

  GtkWidget *widgets[]={gauge, cutoff1_entry, cutoff2_entry, needle_entry, bottom_entry, top_entry, start_entry, mid_entry, end_entry, combo1, combo2, combo3, check1};
  g_signal_connect(button1, "clicked", G_CALLBACK(change_settings), widgets);

  GtkWidget *button2=gtk_button_new_with_label("Animate 0-100");
  gtk_widget_set_halign(button2, GTK_ALIGN_CENTER);
  gtk_widget_set_hexpand(button2, FALSE);
  GtkWidget *widgets2[]={button2, gauge};
  g_signal_connect(button2, "clicked", G_CALLBACK(animate), widgets2);

  GtkWidget *grid=gtk_grid_new();
  gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
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
  gtk_grid_attach(GTK_GRID(grid), start_label, 0, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), start_entry, 1, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), mid_label, 0, 6, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), mid_entry, 1, 6, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), end_label, 0, 7, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), end_entry, 1, 7, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), combo1, 0, 8, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), combo2, 0, 9, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), combo3, 0, 10, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), check1, 0, 11, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), button1, 0, 12, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), button2, 0, 13, 2, 1);

  GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
  gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
  gtk_paned_pack2(GTK_PANED(paned1), gauge, TRUE, TRUE);
  gtk_paned_set_position(GTK_PANED(paned1), 300);
   
  gtk_container_add(GTK_CONTAINER(window), paned1);

  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}
static void change_settings(GtkWidget *button, GtkWidget *widgets[])
{
  //Test some functions and properties of the AdjustableGauge widget.
  gdouble cutoff1=atof(gtk_entry_get_text(GTK_ENTRY(widgets[1])));
  gdouble cutoff2=atof(gtk_entry_get_text(GTK_ENTRY(widgets[2])));
  gdouble needle=atof(gtk_entry_get_text(GTK_ENTRY(widgets[3])));
  gdouble bottom=atof(gtk_entry_get_text(GTK_ENTRY(widgets[4])));
  gdouble top=atof(gtk_entry_get_text(GTK_ENTRY(widgets[5])));
  gchar *start=g_strdup(gtk_entry_get_text(GTK_ENTRY(widgets[6])));
  gchar *mid=g_strdup(gtk_entry_get_text(GTK_ENTRY(widgets[7])));
  gchar *end=g_strdup(gtk_entry_get_text(GTK_ENTRY(widgets[8])));
  gint inside=gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[9]));
  gint outside=gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[10]));
  gint type=gtk_combo_box_get_active(GTK_COMBO_BOX(widgets[11]));
  gboolean check=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[12]));

  //Check the combos.
  if(inside==0) adjustable_gauge_set_inside_radius(ADJUSTABLE_GAUGE(widgets[0]), 1.0);
  else if(inside==1) adjustable_gauge_set_inside_radius(ADJUSTABLE_GAUGE(widgets[0]), 2.0);
  else adjustable_gauge_set_inside_radius(ADJUSTABLE_GAUGE(widgets[0]), 3.0);

  if(outside==0) adjustable_gauge_set_outside_radius(ADJUSTABLE_GAUGE(widgets[0]), 4.0);
  else if(outside==1) adjustable_gauge_set_outside_radius(ADJUSTABLE_GAUGE(widgets[0]), 4.5);
  else adjustable_gauge_set_outside_radius(ADJUSTABLE_GAUGE(widgets[0]), 5.0);

  if(type==0) adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(widgets[0]), VOLTAGE_GAUGE);
  else adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(widgets[0]), SPEEDOMETER_GAUGE);

  //Set colors.
  adjustable_gauge_set_arc_color1(ADJUSTABLE_GAUGE(widgets[0]), start);
  adjustable_gauge_set_arc_color2(ADJUSTABLE_GAUGE(widgets[0]), mid);
  adjustable_gauge_set_arc_color3(ADJUSTABLE_GAUGE(widgets[0]), end);
  g_free(start);
  g_free(mid);
  g_free(end); 

  //Then set scale.
  adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(widgets[0]), bottom);
  adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(widgets[0]), top);
  //Then set needle and cutoffs.  
  adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(widgets[0]), cutoff1);
  adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(widgets[0]), cutoff2);
  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[0]), needle);
  adjustable_gauge_set_draw_gradient(ADJUSTABLE_GAUGE(widgets[0]), check);  
}
static void animate(GtkWidget *widget, GtkWidget *widgets[])
  {
    gtk_widget_set_sensitive(widget, FALSE);
    needle_pos=0;
    g_timeout_add(100, (GSourceFunc)animate_needle, widgets);
  }
static gboolean animate_needle(GtkWidget *widgets[])
 {
   if(needle_pos<100)
     {
       needle_pos++;
       adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[1]), needle_pos);
       gtk_widget_queue_draw(widgets[1]);
       return TRUE;
     }
   else
     {
       adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(widgets[1]), 0);
       gtk_widget_queue_draw(widgets[1]);
       gtk_widget_set_sensitive(widgets[0], TRUE);
       return FALSE;
     }
 }

