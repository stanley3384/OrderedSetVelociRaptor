
/*  
    Original code at: https://developer.gnome.org/gtk3/3.0/gtk-getting-started.html
in the drawing section.

    This version changes squares to lines to smooth out the drawing. You can also save the 
drawing to the current working directory along with changing the pen drawing area cursor
to the selected color. The program icon is a rgb random walk. 

    Tested with GTK3.10 and Ubuntu14.04

    gcc -Wall speedy_scribble.c -o speedy_scribble `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include <gtk/gtk.h>

/* Surface to store current scribbles */
static cairo_surface_t *surface = NULL;
static gdouble prev_x = 0;
static gdouble prev_y = 0;
static gdouble pen_color[] = {0.0, 0.0, 1.0};
//Drawing pen icon.
static GdkPixbuf *pen = NULL;

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
static void set_pen_color(GtkWidget *widget, gpointer *data);
static void error_message(const gchar *string);
static void about_dialog(GtkWidget *widget, gpointer data);
static GdkPixbuf* draw_icon();
static GdkPixbuf* draw_pen_cursor();

int
main (int   argc,
      char *argv[])
{
  gtk_init (&argc, &argv);

  GtkWidget *window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window), "Speedy Scribble");
  gtk_window_set_default_size (GTK_WINDOW (window), 700, 500);
  gtk_window_set_position (GTK_WINDOW (window), GTK_WIN_POS_CENTER);
  GdkPixbuf *icon = draw_icon();
  gtk_window_set_default_icon (icon);

  g_signal_connect (window, "destroy", G_CALLBACK (close_window), NULL);

  gtk_container_set_border_width (GTK_CONTAINER (window), 8);

  GtkWidget *da = gtk_drawing_area_new ();
  gtk_widget_set_hexpand(da, TRUE);
  gtk_widget_set_vexpand(da, TRUE);

  GtkWidget *view = gtk_viewport_new(NULL, NULL);
  gtk_widget_set_hexpand(view, TRUE);
  gtk_widget_set_vexpand(view, TRUE);
  gtk_container_add(GTK_CONTAINER(view), da);
  GtkWidget *scroll = gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(scroll), view);

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

  GtkWidget *button3 = gtk_button_new_with_label("Set Pen Color(RGB)");
  gtk_widget_set_hexpand(button3, TRUE);

  GtkWidget *entry1 = gtk_entry_new();
  gtk_widget_set_hexpand (entry1, TRUE);
  gtk_entry_set_text (GTK_ENTRY(entry1), "scribble1.png");

  g_signal_connect(button2, "clicked", G_CALLBACK(write_to_png), entry1);

  GtkWidget *entry2 = gtk_entry_new();
  gtk_entry_set_text (GTK_ENTRY(entry2), "0.0");

  GtkWidget *entry3 = gtk_entry_new();
  gtk_entry_set_text (GTK_ENTRY(entry3), "0.0");

  GtkWidget *entry4 = gtk_entry_new();
  gtk_entry_set_text (GTK_ENTRY(entry4), "1.0");

  gpointer entries[] = {entry2, entry3, entry4, da};
  g_signal_connect(button3, "clicked", G_CALLBACK(set_pen_color), entries);

  GtkWidget *menu1=gtk_menu_new();
  GtkWidget *menu1item1=gtk_menu_item_new_with_label("Speedy Scribble");
  gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
  GtkWidget *title1=gtk_menu_item_new_with_label("About");
  gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
  GtkWidget *menu_bar=gtk_menu_bar_new();
  gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
  g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

  GtkWidget *grid = gtk_grid_new ();
  gtk_container_set_border_width (GTK_CONTAINER(grid), 10);
  gtk_grid_attach (GTK_GRID(grid), scroll, 0, 0, 4, 5);
  gtk_grid_attach (GTK_GRID(grid), button1, 0, 6, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), button2, 2, 6, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), entry1, 3, 6, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), button3, 0, 7, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), entry2, 1, 7, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), entry3, 2, 7, 1, 1);
  gtk_grid_attach (GTK_GRID(grid), entry4, 3, 7, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 8, 1, 1);

  gtk_container_add (GTK_CONTAINER(window), grid);

  //Add CSS for some background color.
  GError *css_error = NULL;
  gchar css_string[] = "GtkWindow, GtkDialog{background: #7700ff; color: #ffff00} GtkButton{background: #7700aa; color: #ffff00} GtkEntry{background: #0044aa; color: #ffff00}";
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
  gint widget_width = gtk_widget_get_allocated_width (widget);
  gint widget_height = gtk_widget_get_allocated_height (widget);
  static gint prev_width = 0;
  static gint prev_height = 0;
  static gint max_width = 0;
  static gint max_height = 0;

  //If drawing area size increases redraw to new size. If the size decreases, have it scroll.
  if (surface && (max_width < widget_width || max_height < widget_height))
    {
      cairo_surface_t *old_surface = NULL;

      old_surface = cairo_surface_create_for_rectangle (surface, 0, 0, prev_width, prev_height);

      cairo_surface_destroy (surface);  
      surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                                   CAIRO_CONTENT_COLOR,
                                                   widget_width,
                                                   widget_height);
      clear_surface();
      
      cairo_t *cr = cairo_create (surface);
      cairo_set_source_surface (cr, old_surface, (widget_width - prev_width) / 2, (widget_height - prev_height) / 2);
      cairo_rectangle (cr, (widget_width - prev_width) / 2, (widget_height - prev_height) / 2, prev_width, prev_height);
      cairo_fill (cr);
      cairo_destroy (cr);
      cairo_surface_destroy (old_surface);                            
    }

  if (!surface)
    {
      surface = gdk_window_create_similar_surface (gtk_widget_get_window (widget),
                                                   CAIRO_CONTENT_COLOR,
                                                   widget_width,
                                                   widget_height);
      clear_surface();
    }

  if(widget_width > max_width) max_width = widget_width;
  if(widget_height > max_height) max_height = widget_height;
  gtk_widget_set_size_request(widget, max_width, max_height);
  prev_width = widget_width;
  prev_height = widget_height;

  //The pen cursor is also changed in set_pen_color.
  if(pen == NULL) pen = draw_pen_cursor ();
  GdkWindow *win = gtk_widget_get_window (widget);
  GdkDisplay *display = gdk_window_get_display (win);
  GdkCursor *cursor = gdk_cursor_new_from_pixbuf (display, pen, 2, 58);
  gdk_window_set_cursor(win, cursor);
  g_object_unref(cursor); 

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

  cairo_set_source_rgb (cr, pen_color[0], pen_color[1], pen_color[2]);
  cairo_set_line_width (cr, 6);
  cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);

  if (prev_x != 0)
    {
      //g_print ("Move %f %f %f %f\n", prev_x, prev_y, x, y); 
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
  //g_print("Button Release\n");
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
static void
set_pen_color(GtkWidget *widget, gpointer *data)
{
  g_print("Set Pen Color\n");
  const gchar *red_string = gtk_entry_get_text(GTK_ENTRY(data[0]));
  const gchar *green_string = gtk_entry_get_text(GTK_ENTRY(data[1]));
  const gchar *blue_string = gtk_entry_get_text(GTK_ENTRY(data[2]));
  gchar *message = NULL;

  gdouble red = g_ascii_strtod(red_string, NULL);
  gdouble green = g_ascii_strtod(green_string, NULL);
  gdouble blue = g_ascii_strtod(blue_string, NULL);
  g_print("%f %f %f\n", red, green, blue);

  //Should check for g_ascii_strtod returning 0 on error.
  if(red >= 0.0 && red <= 1.0) pen_color[0] = red;
  else
    {
      message = g_strdup ("Valid red values: 0.0 <= red <= 1.0");
      error_message (message);
      g_free (message);
      message = NULL;
    }
  if(green >= 0.0 && green <= 1.0) pen_color[1] = green;
  else
    {
      message = g_strdup ("Valid green values: 0.0 <= green <= 1.0");
      error_message (message);
      g_free (message);
      message = NULL;
    }
  if(blue >= 0.0 && blue <= 1.0) pen_color[2] = blue;
  else
    {
      message = g_strdup ("Valid blue values: 0.0 <= blue <= 1.0");
      error_message (message);
      g_free (message);
      message = NULL;
    }

  //Reset pen tip color. This is changed in the configure event also.
  if(pen != NULL)
    {
      g_object_unref(pen);
      pen = NULL;
    }
  pen = draw_pen_cursor ();
  GdkWindow *win = gtk_widget_get_window (GTK_WIDGET(data[3]));
  GdkDisplay *display = gdk_window_get_display (win);
  GdkCursor *cursor = gdk_cursor_new_from_pixbuf (display, pen, 1, 60);
  gdk_window_set_cursor(win, cursor);
  g_object_unref(cursor); 
}
static void error_message(const gchar *string)
{
  GtkWidget *dialog=gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", string);
  gtk_dialog_run(GTK_DIALOG(dialog));
  gtk_widget_destroy(dialog);
}
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Speedy Scribble");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A program for creating fine artwork.");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
//The diving and swimming pool notes.
static GdkPixbuf* draw_icon()
  {
    gint i = 0;
    gdouble current_x = 0;
    gdouble current_y = 0;
    gdouble rnd_prev_x = 50.0;
    gdouble rnd_prev_y = 50.0;
    
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface_icon = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr = cairo_create (surface_icon);
    
    //Paint the background.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
   
    //Draw red random lines.
    cairo_set_source_rgb (cr, 1.0, 0.0, 0.0);
    cairo_set_line_width (cr, 4);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    for(i = 0; i < 25; i++)
      {
        current_x = 256.0 * g_random_double();
        current_y = 256.0 * g_random_double();
        cairo_move_to (cr, rnd_prev_x, rnd_prev_y);
        cairo_line_to (cr, current_x, current_y);
        cairo_stroke (cr);
        rnd_prev_x = current_x;
        rnd_prev_y = current_y;
      }
    
    //Draw green random lines.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);    
    rnd_prev_x = 100.0;
    rnd_prev_y = 100.0;
    for(i = 0; i < 25; i++)
      {
        current_x = 256.0 * g_random_double();
        current_y = 256.0 * g_random_double();
        cairo_move_to (cr, rnd_prev_x, rnd_prev_y);
        cairo_line_to (cr, current_x, current_y);
        cairo_stroke (cr);
        rnd_prev_x = current_x;
        rnd_prev_y = current_y;
      }

    //Draw blue random lines.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);    
    rnd_prev_x = 150.0;
    rnd_prev_y = 150.0;
    for(i = 0; i < 25; i++)
      {
        current_x = 256.0 * g_random_double();
        current_y = 256.0 * g_random_double();
        cairo_move_to (cr, rnd_prev_x, rnd_prev_y);
        cairo_line_to (cr, current_x, current_y);
        cairo_stroke (cr);
        rnd_prev_x = current_x;
        rnd_prev_y = current_y;
      }
    
    GdkPixbuf *icon = gdk_pixbuf_get_from_surface(surface_icon, 0, 0, 256, 256);

    cairo_destroy (cr);
    cairo_surface_destroy (surface_icon); 
    return icon;
  }
//The diving and swimming pool notes.
static GdkPixbuf* draw_pen_cursor()
  { 
    //Create a surface to draw on. 
    cairo_surface_t *surface_pen = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 64, 64);
    cairo_t *cr = cairo_create (surface_pen);
    
    //Paint the background transparent.
    cairo_set_source_rgba (cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint (cr);

    //y=mx+b, y=3/1x
    cairo_set_source_rgb (cr, 0.0, 0.0, 0.0);
    cairo_set_line_width (cr, 5.0);
    cairo_move_to (cr, 14.0, 22.0);
    cairo_line_to (cr, 4.0, 52.0);
    cairo_stroke (cr);
    cairo_set_source_rgb (cr, pen_color[0], pen_color[1], pen_color[2]);
    cairo_move_to (cr, 4.0, 52.0);
    cairo_line_to (cr, 2.0, 58.0);
    cairo_stroke (cr);
    
    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface_pen, 0, 0, 64, 64);

    cairo_destroy(cr);
    cairo_surface_destroy(surface_pen); 
    return icon;
  }








