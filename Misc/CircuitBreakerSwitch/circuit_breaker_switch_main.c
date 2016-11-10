
/*

    A basic circuit breaker switch panel for testing. The da_circuit_breaker1.c in the cairo_drawings
folder was the start for this widget.

    gcc -Wall -Werror circuit_breaker_switch.c circuit_breaker_switch_main.c -o circuit_breaker `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "circuit_breaker_switch.h"

static void click_drawing(GtkWidget *widget, gpointer data)
{
  //Just change between states.
  gint state=circuit_breaker_switch_get_state(CIRCUIT_BREAKER_SWITCH(widget));
  if(state==0) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), 3);
  else if(state==1) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), 0);
  else if(state==2) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), 1);
  else if(state==3) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), 2);
  else circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), 2);
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Circuit Breaker Panel");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *cbs1=circuit_breaker_switch_new();
  gtk_widget_set_hexpand(cbs1, TRUE);
  gtk_widget_set_vexpand(cbs1, TRUE);
  gtk_widget_set_events(cbs1, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cbs1, "button_press_event", G_CALLBACK(click_drawing), NULL);

  GtkWidget *cbs2=circuit_breaker_switch_new();
  gtk_widget_set_hexpand(cbs2, TRUE);
  gtk_widget_set_vexpand(cbs2, TRUE);
  gtk_widget_set_events(cbs2, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cbs2, "button_press_event", G_CALLBACK(click_drawing), NULL); 

  GtkWidget *cbs3=circuit_breaker_switch_new();
  gtk_widget_set_hexpand(cbs3, TRUE);
  gtk_widget_set_vexpand(cbs3, TRUE);
  gtk_widget_set_events(cbs3, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cbs3, "button_press_event", G_CALLBACK(click_drawing), NULL); 

  GtkWidget *cbs4=circuit_breaker_switch_new();
  circuit_breaker_switch_set_direction(CIRCUIT_BREAKER_SWITCH(cbs4), BREAKER_VERTICAL_UP);
  gtk_widget_set_hexpand(cbs4, TRUE);
  gtk_widget_set_vexpand(cbs4, TRUE);
  gtk_widget_set_events(cbs4, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cbs4, "button_press_event", G_CALLBACK(click_drawing), NULL); 

  GtkWidget *cbs5=circuit_breaker_switch_new();
  circuit_breaker_switch_set_direction(CIRCUIT_BREAKER_SWITCH(cbs5), BREAKER_VERTICAL_UP);
  gtk_widget_set_hexpand(cbs5, TRUE);
  gtk_widget_set_vexpand(cbs5, TRUE);
  gtk_widget_set_events(cbs5, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cbs5, "button_press_event", G_CALLBACK(click_drawing), NULL); 

  GtkWidget *cbs6=circuit_breaker_switch_new();
  circuit_breaker_switch_set_direction(CIRCUIT_BREAKER_SWITCH(cbs6), BREAKER_VERTICAL_UP);
  gtk_widget_set_hexpand(cbs6, TRUE);
  gtk_widget_set_vexpand(cbs6, TRUE);
  gtk_widget_set_events(cbs6, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(cbs6, "button_press_event", G_CALLBACK(click_drawing), NULL); 
  
  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), cbs1, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), cbs2, 0, 1, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), cbs3, 0, 2, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), cbs4, 2, 0, 1, 3);
  gtk_grid_attach(GTK_GRID(grid), cbs5, 3, 0, 1, 3);
  gtk_grid_attach(GTK_GRID(grid), cbs6, 4, 0, 1, 3);
  
  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}
