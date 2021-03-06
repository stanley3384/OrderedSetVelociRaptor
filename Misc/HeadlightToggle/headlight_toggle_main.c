
/*

    A headlight toggle widget for testing. The da_simple_toggle1.c in the cairo_drawings
folder was the start for this widget. You can draw different icons for the toggle switch
and add them to the code.
    This should probably be called the rocker switch widget. Might have to rename a few
things or add some toggle drawings to it eventually. 
    The vertical toggle drawings are translated and scaled. This might not fit the drawing
exactly where it needs to be on the toggle. Depending what the needs are you can add or
delete drawings and draw with or without translating and scaling.

    gcc -Wall -Werror headlight_toggle.c headlight_toggle_main.c -o headlight_toggle `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "headlight_toggle.h"

static gboolean click_horizontal_drawing(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  //Need to move the location of the two sides of the toggle inside the widget.
  gint width=gtk_widget_get_allocated_width(widget);
  gint state=headlight_toggle_get_state(HEADLIGHT_TOGGLE(widget));

  if(state==0&&(event->button.x)>width/2) 
    {
      headlight_toggle_set_state(HEADLIGHT_TOGGLE(widget), HEADLIGHT_TOGGLE_ON);
      gtk_widget_queue_draw(widget);
    }
  if(state==1&&(event->button.x)<width/2) 
    {
      headlight_toggle_set_state(HEADLIGHT_TOGGLE(widget), HEADLIGHT_TOGGLE_OFF);
      gtk_widget_queue_draw(widget);
    }

  return FALSE;
}
static gboolean click_vertical_drawing(GtkWidget *widget, GdkEvent *event, gpointer data)
{
  //Need to move the location of the two sides of the toggle inside the widget.
  gint height=gtk_widget_get_allocated_height(widget);
  gint state=headlight_toggle_get_state(HEADLIGHT_TOGGLE(widget));

  if(state==0&&(event->button.y)<height/2) 
    {
      headlight_toggle_set_state(HEADLIGHT_TOGGLE(widget), HEADLIGHT_TOGGLE_ON);
      gtk_widget_queue_draw(widget);
    }
  if(state==1&&(event->button.y)>height/2) 
    {
      headlight_toggle_set_state(HEADLIGHT_TOGGLE(widget), HEADLIGHT_TOGGLE_OFF);
      gtk_widget_queue_draw(widget);
    }

  return FALSE;
}
int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Toggles");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *ht1=headlight_toggle_new();
  gtk_widget_set_hexpand(ht1, TRUE);
  gtk_widget_set_vexpand(ht1, TRUE);
  gtk_widget_set_events(ht1, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht1, "button_press_event", G_CALLBACK(click_horizontal_drawing), NULL);

  GtkWidget *ht2=headlight_toggle_new();
  headlight_toggle_set_icon(HEADLIGHT_TOGGLE(ht2), EMERGENCY_LIGHT_ICON);
  headlight_toggle_set_dim_color(HEADLIGHT_TOGGLE(ht2), "rgba(0, 200, 0, 1.0)");
  headlight_toggle_set_lit_color(HEADLIGHT_TOGGLE(ht2), "rgba(0, 255, 0, 1.0)");
  gtk_widget_set_hexpand(ht2, TRUE);
  gtk_widget_set_vexpand(ht2, TRUE);
  gtk_widget_set_events(ht2, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht2, "button_press_event", G_CALLBACK(click_horizontal_drawing), NULL); 

  GtkWidget *ht3=headlight_toggle_new();
  headlight_toggle_set_dim_color(HEADLIGHT_TOGGLE(ht3), "rgba(0, 0, 200, 1.0)");
  headlight_toggle_set_lit_color(HEADLIGHT_TOGGLE(ht3), "rgba(0, 0, 255, 1.0)");
  headlight_toggle_set_icon(HEADLIGHT_TOGGLE(ht3), FAN_ICON);
  gtk_widget_set_hexpand(ht3, TRUE);
  gtk_widget_set_vexpand(ht3, TRUE);
  gtk_widget_set_events(ht3, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht3, "button_press_event", G_CALLBACK(click_horizontal_drawing), NULL); 

  GtkWidget *ht4=headlight_toggle_new();
  headlight_toggle_set_icon(HEADLIGHT_TOGGLE(ht4), HEATER_ICON);
  gtk_widget_set_hexpand(ht4, TRUE);
  gtk_widget_set_vexpand(ht4, TRUE);
  gtk_widget_set_events(ht4, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht4, "button_press_event", G_CALLBACK(click_horizontal_drawing), NULL); 

  //Vertical drawings.
  GtkWidget *ht5=headlight_toggle_new();
  headlight_toggle_set_direction(HEADLIGHT_TOGGLE(ht5), VERTICAL_UP);
  headlight_toggle_set_dim_color(HEADLIGHT_TOGGLE(ht5), "rgba(200, 0, 200, 1.0)");
  headlight_toggle_set_lit_color(HEADLIGHT_TOGGLE(ht5), "rgba(255, 0, 255, 1.0)");
  gtk_widget_set_hexpand(ht5, TRUE);
  gtk_widget_set_vexpand(ht5, TRUE);
  gtk_widget_set_events(ht5, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht5, "button_press_event", G_CALLBACK(click_vertical_drawing), NULL);

  GtkWidget *ht6=headlight_toggle_new();
  headlight_toggle_set_direction(HEADLIGHT_TOGGLE(ht6), VERTICAL_UP);
  headlight_toggle_set_icon(HEADLIGHT_TOGGLE(ht6), EMERGENCY_LIGHT_ICON);
  gtk_widget_set_hexpand(ht6, TRUE);
  gtk_widget_set_vexpand(ht6, TRUE);
  gtk_widget_set_events(ht6, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht6, "button_press_event", G_CALLBACK(click_vertical_drawing), NULL); 

  GtkWidget *ht7=headlight_toggle_new();
  headlight_toggle_set_direction(HEADLIGHT_TOGGLE(ht7), VERTICAL_UP);
  headlight_toggle_set_icon(HEADLIGHT_TOGGLE(ht7), FAN_ICON);
  gtk_widget_set_hexpand(ht7, TRUE);
  gtk_widget_set_vexpand(ht7, TRUE);
  gtk_widget_set_events(ht7, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht7, "button_press_event", G_CALLBACK(click_vertical_drawing), NULL); 

  GtkWidget *ht8=headlight_toggle_new();
  headlight_toggle_set_direction(HEADLIGHT_TOGGLE(ht8), VERTICAL_UP);
  headlight_toggle_set_dim_color(HEADLIGHT_TOGGLE(ht8), "rgba(0, 200, 200, 1.0)");
  headlight_toggle_set_lit_color(HEADLIGHT_TOGGLE(ht8), "rgba(0, 255, 255, 1.0)");
  headlight_toggle_set_icon_dim_color(HEADLIGHT_TOGGLE(ht8), "rgba(200, 200, 200, 1.0)");
  headlight_toggle_set_icon_lit_color(HEADLIGHT_TOGGLE(ht8), "rgba(255, 255, 255, 1.0)");
  headlight_toggle_set_icon(HEADLIGHT_TOGGLE(ht8), HEATER_ICON);
  gtk_widget_set_hexpand(ht8, TRUE);
  gtk_widget_set_vexpand(ht8, TRUE);
  gtk_widget_set_events(ht8, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(ht8, "button_press_event", G_CALLBACK(click_vertical_drawing), NULL); 
  
  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), ht1, 0, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), ht2, 0, 1, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), ht3, 2, 0, 2, 1);
  gtk_grid_attach(GTK_GRID(grid), ht4, 2, 1, 2, 1);

  gtk_grid_attach(GTK_GRID(grid), ht5, 0, 2, 1, 2);
  gtk_grid_attach(GTK_GRID(grid), ht6, 1, 2, 1, 2);
  gtk_grid_attach(GTK_GRID(grid), ht7, 2, 2, 1, 2);
  gtk_grid_attach(GTK_GRID(grid), ht8, 3, 2, 1, 2);
  
  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();
  return 0;
}

