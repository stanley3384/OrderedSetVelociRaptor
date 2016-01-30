
/*  
    Original code at: https://developer.gnome.org/gtk3/3.0/gtk-getting-started.html
in the drawing section.

    This version changes squares to lines to smooth out the drawing. You can also save the 
drawing to the current working directory.

    Tested with GTK3.10 and Ubuntu14.04

    gcc -Wall speedy_scribble.c -o speedy_scribble `pkg-config --cflags --libs gtk+-3.0`
*/

#include <gtk/gtk.h>

/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;
static gdouble prev_x = 0;
static gdouble prev_y = 0;

static void clear_surface (void);
static gboolean configure_event_cb (GtkWidget *widget, GdkEventConfigure *event, gpointer data);
static gboolean draw_cb (GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_brush (GtkWidget *widget, gdouble x, gdouble y);
static gboolean button_press_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean button_release_event_cb (GtkWidget *widget, GdkEventButton *event, gpointer data);
static gboolean motion_notify_event_cb (GtkWidget *widget, GdkEventMotion *event, gpointer data);
static void close_window (void);
static void clear_cairo_surface(GtkWidget *widget, gpointer data);
static void write_to_png(GtkWidget *widget, gpointer data);

int
main (int   argc,
      char *argv[])
{
  gtk_init (&argc, &argv);

  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Speedy Scribble");
  gtk_window_set_default_size(GTK_WINDOW (window), 500, 500);
  gtk_window_set_position(GTK_WINDOW (window), GTK_WIN_POS_CENTER);

  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  GtkWidget *frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);

  GtkWidget *da = gtk_drawing_area_new ();
  gtk_widget_set_hexpand(da, TRUE);
  gtk_widget_set_vexpand(da, TRUE);

  gtk_container_add (GTK_CONTAINER (frame), da);

  /* Signals used to handle the backing surface */
  g_signal_connect (da, "draw",
            G_CALLBACK (draw_cb), NULL);
  g_signal_connect (da,"configure-event",
            G_CALLBACK (configure_event_cb), NULL);

  /* Event signals */
  g_signal_connect (da, "motion-notify-event",
            G_CALLBACK (motion_notify_event_cb), NULL);
  g_signal_connect (da, "button-press-event",
            G_CALLBACK (button_press_event_cb), NULL);
  g_signal_connect (da, "button-release-event",
            G_CALLBACK (button_release_event_cb), NULL);

  /* Ask to receive events the drawing area doesn't normally
   * subscribe to. In particular, we need to ask for the
   * button press and motion notify events that want to handle.
   */
  gtk_widget_set_events (da, gtk_widget_get_events (da)
                     | GDK_BUTTON_PRESS_MASK
                     | GDK_BUTTON_RELEASE_MASK
                     | GDK_POINTER_MOTION_MASK
                     | GDK_POINTER_MOTION_HINT_MASK);

  GtkWidget *button1 = gtk_button_new_with_label("Clear");
  gtk_widget_set_hexpand(button1, TRUE);
  g_signal_connect(button1, "clicked", G_CALLBACK(clear_cairo_surface), da);

  GtkWidget *button2 = gtk_button_new_with_label("Save Drawing");
  gtk_widget_set_hexpand(button2, TRUE);

  GtkWidget *entry1 = gtk_entry_new();
  gtk_widget_set_hexpand (entry1, TRUE);
  gtk_entry_set_text (GTK_ENTRY(entry1), "scribble1.png");

  g_signal_connect(button2, "clicked", G_CALLBACK(write_to_png), entry1);

  GtkWidget *grid = gtk_grid_new ();
  gtk_container_set_border_width (GTK_CONTAINER(grid), 10);
  gtk_grid_attach (GTK_GRID(grid), frame, 0, 0, 3, 5);
  gtk_grid_attach (GTK_GRID(grid), button1, 0, 6, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), button2, 1, 6, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), entry1, 2, 6, 1, 1);

  gtk_container_add (GTK_CONTAINER(window), grid);

  //Add CSS for some background color.
  GError *css_error = NULL;
  gchar css_string[] = "GtkWindow{background: #7700ff;} GtkButton{background: #7700aa; color: #ffff00} GtkEntry{background: #0044aa; color: #ffff00}";
  GtkCssProvider *provider = gtk_css_provider_new();
  GdkDisplay *display = gdk_display_get_default();
  GdkScreen *screen = gdk_display_get_default_screen(display);
  gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
  if(css_error!=NULL)
    {
      g_print("CSS loader error %s\n", css_error->message);
      g_error_free(css_error);
    }
  g_object_unref(provider);

  gtk_widget_show_all (window);

  gtk_main ();

  return 0;
}
static void
clear_surface (void)
{
  cairo_t *cr;

  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 1, 1, 1);
  cairo_paint (cr);

  cairo_destroy (cr);
}

/* Create a new surface of the appropriate size to store our scribbles */
static gboolean
configure_event_cb (GtkWidget         *widget,
            GdkEventConfigure *event,
            gpointer           data)
{
  if (surface)
    cairo_surface_destroy (surface);

  surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                       CAIRO_CONTENT_COLOR,
                                       gtk_widget_get_allocated_width (widget),
                                       gtk_widget_get_allocated_height (widget));

  /* Initialize the surface to white */
  clear_surface ();

  /* We've handled the configure event, no need for further processing. */
  return TRUE;
}

/* Redraw the screen from the surface. Note that the ::draw
 * signal receives a ready-to-be-used cairo_t that is already
 * clipped to only draw the exposed areas of the widget
 */
static gboolean
draw_cb (GtkWidget *widget,
 cairo_t   *cr,
 gpointer   data)
{
  cairo_set_source_surface (cr, surface, 0, 0);
  cairo_paint (cr);

  return FALSE;
}

/* Draw a rectangle on the surface at the given position */
static void
draw_brush (GtkWidget *widget,
    gdouble    x,
    gdouble    y)
{
  cairo_t *cr;
  
  /* Paint to the surface, where we store our state */
  cr = cairo_create (surface);

  cairo_set_source_rgb (cr, 0.0, 0.0, 1.0);
  cairo_set_line_width (cr, 6);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

  if (prev_x != 0)
    {
      g_print ("Move %f %f %f %f\n", prev_x, prev_y, x, y); 
      cairo_move_to (cr, prev_x, prev_y);
      cairo_line_to (cr, x, y);
      cairo_stroke (cr);
    }

  prev_x = x;
  prev_y = y;
  cairo_destroy (cr);

  /* Now invalidate the affected region of the drawing area. */
  gtk_widget_queue_draw_area (widget, 0, 0, gtk_widget_get_allocated_width (widget), gtk_widget_get_allocated_height (widget));
}

/* Handle button press events by either drawing a rectangle
 * or clearing the surface, depending on which button was pressed.
 * The ::button-press signal handler receives a GdkEventButton
 * struct which contains this information.
 */
static gboolean
button_press_event_cb (GtkWidget      *widget,
               GdkEventButton *event,
               gpointer        data)
{
  /* paranoia check, in case we haven't gotten a configure event */
  if (surface == NULL)
    return FALSE;

  if (event->button == 1)
    {
      draw_brush (widget, event->x, event->y);
    }
  else if (event->button == 3)
    {
      clear_surface ();
      gtk_widget_queue_draw (widget);
    }

  /* We've handled the event, stop processing */
  return TRUE;
}
static gboolean
button_release_event_cb (GtkWidget      *widget,
               GdkEventButton *event,
               gpointer        data)
{
  g_print("Button Release\n");
  prev_x = 0;
  prev_y = 0;
  return TRUE;
}
/* Handle motion events by continuing to draw if button 1 is
 * still held down. The ::motion-notify signal handler receives
 * a GdkEventMotion struct which contains this information.
 */
static gboolean
motion_notify_event_cb (GtkWidget      *widget,
                GdkEventMotion *event,
                gpointer        data)
{
  int x, y;
  GdkModifierType state;

  /* paranoia check, in case we haven't gotten a configure event */
  if (surface == NULL)
    return FALSE;

  /* This call is very important; it requests the next motion event.
   * If you don't call gdk_window_get_pointer() you'll only get
   * a single motion event. The reason is that we specified
   * GDK_POINTER_MOTION_HINT_MASK to gtk_widget_set_events().
   * If we hadn't specified that, we could just use event->x, event->y
   * as the pointer location. But we'd also get deluged in events.
   * By requesting the next event as we handle the current one,
   * we avoid getting a huge number of events faster than we
   * can cope.
   */
  //gdk_window_get_pointer (event->window, &x, &y, &state);
  gdk_window_get_device_position(gtk_widget_get_window(widget), event->device, &x, &y, &state);

  if (state & GDK_BUTTON1_MASK)
    draw_brush (widget, x, y);

  /* We've handled it, stop processing */
  return TRUE;
}
static void
close_window (void)
{
  if (surface)
    cairo_surface_destroy (surface);

  gtk_main_quit ();
}
static void
clear_cairo_surface(GtkWidget *widget, gpointer data)
{
  g_print("Clear Surface\n");
  clear_surface();
  gtk_widget_queue_draw (GTK_WIDGET(data));
}
static void
write_to_png(GtkWidget *widget, gpointer data)
{
  g_print("Write Surface\n");
  cairo_status_t status;
  //Save to working directory.
  gchar *file_name = g_strdup_printf ("./%s", gtk_entry_get_text(GTK_ENTRY(data)));
  status = cairo_surface_write_to_png (surface, file_name);
  if(status != CAIRO_STATUS_SUCCESS) cairo_status_to_string(status);
  g_free(file_name);
}

