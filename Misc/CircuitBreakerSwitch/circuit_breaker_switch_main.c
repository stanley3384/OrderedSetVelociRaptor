
/*

    A basic circuit breaker switch panel for testing. The da_circuit_breaker1.c in the cairo_drawings
folder was the start for this widget.

    gcc -Wall -Werror circuit_breaker_switch.c circuit_breaker_switch_main.c -o circuit_breaker `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "circuit_breaker_switch.h"

static gboolean cursor_click=FALSE;

static gboolean click_drawing(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  //Just change between states.
  gint state=circuit_breaker_switch_get_state(CIRCUIT_BREAKER_SWITCH(widget));
  if(state==BREAKER_ON) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_BREAK);
  else if(state==BREAKER_STARTING) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_ON);
  else if(state==BREAKER_OFF) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_STARTING);
  else if(state==BREAKER_BREAK) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_OFF);
  else circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_OFF);

  return FALSE;
}
static gboolean click_drawing_cbs1(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  //Same as above but check for cursor position.
  if(cursor_click)
    {
      gint state=circuit_breaker_switch_get_state(CIRCUIT_BREAKER_SWITCH(widget));
      if(state==BREAKER_ON) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_BREAK);
      else if(state==BREAKER_STARTING) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_ON);
      else if(state==BREAKER_OFF) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_STARTING);
      else if(state==BREAKER_BREAK) circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_OFF);
      else circuit_breaker_switch_set_state(CIRCUIT_BREAKER_SWITCH(widget), BREAKER_OFF);
      cursor_click=FALSE;
    }

  return FALSE;
}
static gboolean move_over_cbs1(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  //Test some cursors in the first switch.
  gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
  gint state=circuit_breaker_switch_get_state(CIRCUIT_BREAKER_SWITCH(widget));
  GdkWindow *window=gtk_widget_get_window(widget);

  if(state==2&&(event->button.x)<width/6.0) 
    {
      GdkDisplay *display=gdk_window_get_display(window);
      GdkCursor *cursor=gdk_cursor_new_from_name(display, "e-resize");
      gdk_window_set_cursor(window, cursor);
      g_object_unref(cursor);
      cursor_click=TRUE;
    }
  else if(state==0&&(event->button.x)>5.0*width/6.0) 
    {
      GdkDisplay *display=gdk_window_get_display(window);
      GdkCursor *cursor=gdk_cursor_new_from_name(display, "w-resize");
      gdk_window_set_cursor(window, cursor);
      g_object_unref(cursor);
      cursor_click=TRUE;
    }
  else if(state==3&&(event->button.x)>3.5*width/6.0&&(event->button.x)<4.5*width/6.0) 
    {
      GdkDisplay *display=gdk_window_get_display(window);
      GdkCursor *cursor=gdk_cursor_new_from_name(display, "w-resize");
      gdk_window_set_cursor(window, cursor);
      g_object_unref(cursor);
      cursor_click=TRUE;
    }
  else if(state==1)
    {
      gdk_window_set_cursor(window, NULL);
      cursor_click=TRUE;
    }
  else
    {
      gdk_window_set_cursor(window, NULL);
      cursor_click=FALSE;
    }
  
  return FALSE;
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
  circuit_breaker_switch_set_icon(CIRCUIT_BREAKER_SWITCH(cbs1), FALSE);
  gtk_widget_set_hexpand(cbs1, TRUE);
  gtk_widget_set_vexpand(cbs1, TRUE);
  gtk_widget_set_events(cbs1, GDK_BUTTON_PRESS_MASK|GDK_POINTER_MOTION_MASK);
  g_signal_connect(cbs1, "button_press_event", G_CALLBACK(click_drawing_cbs1), NULL);
  g_signal_connect(cbs1, "motion-notify-event", G_CALLBACK(move_over_cbs1), NULL);

  GtkWidget *cbs2=circuit_breaker_switch_new();
  circuit_breaker_switch_set_background_off(CIRCUIT_BREAKER_SWITCH(cbs2), "rgba(150, 150, 150, 1.0)");
  circuit_breaker_switch_set_background_starting(CIRCUIT_BREAKER_SWITCH(cbs2), "rgba(0, 200, 200, 1.0)");
  circuit_breaker_switch_set_background_on(CIRCUIT_BREAKER_SWITCH(cbs2), "rgba(200, 200, 0, 1.0)");
  circuit_breaker_switch_set_background_break(CIRCUIT_BREAKER_SWITCH(cbs2), "rgba(0, 0, 0, 1.0)");
  circuit_breaker_switch_set_foreground(CIRCUIT_BREAKER_SWITCH(cbs2), "rgba(255, 255, 255, 1.0)");
  g_print("%s\n%s\n%s\n%s\n%s\n", circuit_breaker_switch_get_background_off(CIRCUIT_BREAKER_SWITCH(cbs2)), circuit_breaker_switch_get_background_starting(CIRCUIT_BREAKER_SWITCH(cbs2)), circuit_breaker_switch_get_background_on(CIRCUIT_BREAKER_SWITCH(cbs2)), circuit_breaker_switch_get_background_break(CIRCUIT_BREAKER_SWITCH(cbs2)), circuit_breaker_switch_get_foreground(CIRCUIT_BREAKER_SWITCH(cbs2)));
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
  circuit_breaker_switch_set_icon(CIRCUIT_BREAKER_SWITCH(cbs4), FALSE);
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

