
/*
Animate the window on a GtkButton.

  gcc -Wall icon.c -o icon `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon
*/
#include<gtk/gtk.h>

gint timer_id=0;

static gboolean draw_radial_color(gpointer data)
 {
   static guint radius=10;
   guint width=gtk_widget_get_allocated_width(GTK_WIDGET(data));
   guint height=gtk_widget_get_allocated_height(GTK_WIDGET(data));
   GdkWindow *button_window=gtk_button_get_event_window(GTK_BUTTON(data));
   cairo_t *cr=gdk_cairo_create(button_window);
   cairo_pattern_t *radial1;  
  
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_paint(cr);

   cairo_translate(cr, width/2, height/2);

   if(radius>600) radius=10;
   radial1 = cairo_pattern_create_radial(0, 0, 1, 0, 0, radius);  
   cairo_pattern_add_color_stop_rgb(radial1, 0.3, 0.0, 0.0, 1.0);
   cairo_pattern_add_color_stop_rgb(radial1, 0.0, 1.0, 1.0, 0.0);
   cairo_set_source(cr, radial1);
   cairo_arc(cr, 0, 0, 300, 0, G_PI * 2);
   cairo_fill(cr); 

   radius+=20;    
  
   cairo_translate(cr, -width/2, -height/2);
   cairo_set_line_width(cr, 4.0);
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_rectangle(cr, 0, 0, width, height);
   cairo_stroke_preserve(cr); 

   cairo_destroy(cr);     
   cairo_pattern_destroy(radial1);
   return TRUE;
 }
static gboolean start_animation(GtkWidget *button, GdkEvent *event, gpointer data)
 {
   g_print("Start Animation\n");
   timer_id=g_timeout_add(50, draw_radial_color, button);
   return TRUE;
 }
static gboolean stop_animation(GtkWidget *button, GdkEvent *event, gpointer data)
 {
   g_print("Stop Animation\n");
   g_source_remove(timer_id);
   GdkWindow *win = gtk_widget_get_window(GTK_WIDGET(button));
   if(win)
     {
       GtkAllocation allocation;
       gtk_widget_get_allocation(GTK_WIDGET(button), &allocation);
       gdk_window_invalidate_rect(win, &allocation, FALSE);
     }
   return TRUE;
 }
static void button_clicked(GtkWidget *widget, gpointer data)
 {
   g_print("Clicked\n");
 }

static void close_program(GtkWidget *widget, gpointer data)
 {
   printf("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   GtkWidget *window, *button1, *grid;

   gtk_init(&argc, &argv);

   gchar css_string[]="GtkButton{background: blue}\n\
                       GtkWindow{background: green}";
   GtkCssProvider *provider;
   GdkDisplay *display;
   GdkScreen *screen;
   GError *css_error=NULL;
   provider = gtk_css_provider_new();
   display = gdk_display_get_default();
   screen = gdk_display_get_default_screen(display);
   gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
   if(css_error!=NULL) g_print("CSS loader error %s\n", css_error->message);
   g_object_unref(provider);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Animation Button");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 100);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);
   g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

   button1=gtk_button_new();
   gtk_widget_set_hexpand(button1, TRUE);
   gtk_widget_set_vexpand(button1, TRUE);
   g_signal_connect(button1, "enter-notify-event", G_CALLBACK(start_animation), NULL);
   g_signal_connect(button1, "leave-notify-event", G_CALLBACK(stop_animation), NULL);
   g_signal_connect(button1, "button-press-event", G_CALLBACK(button_clicked), NULL); 

   grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();
   return 0;
  }
   


