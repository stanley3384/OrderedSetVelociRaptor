
/*

Test code for trying out some CSS with GTK. A couple of color gradients on buttons, a colored event box under text and a drawing area with some animation.

//gcc -Wall css1.c -o css1 `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon
*/

#include<gtk/gtk.h>

gint timer_id=0;

static gboolean change_font_color_enter(GtkWidget *button, GdkEvent *event, GtkLabel *label)
 {
   gchar *string=g_strdup_printf( "<span foreground=\"white\">%s</span>", gtk_button_get_label(GTK_BUTTON(button)));
   gtk_label_set_markup(GTK_LABEL(label), string);
   g_free(string);
   return TRUE;
 }
static gboolean change_font_color_leave(GtkWidget *button, GdkEvent *event, GtkLabel *label)
 {
   gchar *string=g_strdup_printf( "<span foreground=\"black\">%s</span>", gtk_button_get_label(GTK_BUTTON(button)));
   gtk_label_set_markup(GTK_LABEL(label), string);
   g_free(string);
   return TRUE;
 }
static gboolean change_font_color_enter_event(GtkWidget *event_box, GdkEvent *event, GtkLabel *label)
 {
   gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"white\">label2</span>");
   return TRUE;
 }
static gboolean change_font_color_leave_event(GtkWidget *event_box, GdkEvent *event, GtkLabel *label)
 {
   gtk_label_set_markup(GTK_LABEL(label), "<span foreground=\"black\">label2</span>");
   return TRUE;
 }
static gboolean event_box_button_press(GtkWidget *event_box, GdkEvent *event, GtkLabel *label)
 {
   g_print("Click\n");
   return TRUE;
 }
gboolean draw_radial_color(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   static guint radius=100;

   guint width=gtk_widget_get_allocated_width(widget);
   guint height=gtk_widget_get_allocated_height(widget);
   cairo_pattern_t *radial1;  
  
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_paint(cr);
   cairo_translate(cr, width/2, height/2);
  
   if(radius>1500) radius=100;
   radial1 = cairo_pattern_create_radial(0, 0, 1, 0, 0, radius);  
   cairo_pattern_add_color_stop_rgb(radial1, 0.3, 0.0, 0.0, 1.0);
   cairo_pattern_add_color_stop_rgb(radial1, 0.0, 1.0, 0.0, 0.0);
   cairo_set_source(cr, radial1);
   cairo_arc(cr, 0, 0, 300, 0, G_PI * 2);
   cairo_fill(cr);     
         
   cairo_pattern_destroy(radial1);
   radius+=20;
   return TRUE;
 }
static gboolean animate_drawing_area(gpointer data)
 {  
   gtk_widget_queue_draw_area(GTK_WIDGET(data), 0, 0, gtk_widget_get_allocated_width(GTK_WIDGET(data)), gtk_widget_get_allocated_height(GTK_WIDGET(data)));  
   return TRUE;
 }
static void close_program()
 {
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   printf("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   GtkWidget *window, *label1, *label2, *button1, *button2, *button3, *button4, *button_label1, *button_label2, *button_label3, *button_label4, *event_box1, *drawing_area1, *grid1;
   gchar css_string[]="GtkButton{background: blue}\n\
                       GtkLabel{color: black}\n\
                       GtkEventBox{background: yellow}\n\
                       GtkButton#css_button3{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,0,0,1)), color-stop(0.5,rgba(0,255,0,1)), color-stop(1.0,rgba(0,0,255,1)));}\n\
                       GtkButton#css_button4{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,0,255,1)), color-stop(1.0,rgba(255,255,0,1)));}\n\
                       GtkButton#css_button3:focused{background: purple}\n\
                       GtkButton#css_button4:focused{background: purple}\n\
                       GtkWindow{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,0,255,1)), color-stop(1.0,rgba(255,255,0,1)));}\n\
                       GtkButton:focused{background: purple}";
   GtkCssProvider *provider;
   GdkDisplay *display;
   GdkScreen *screen;
   GError *css_error=NULL;

   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Label and ButtonLabel");
   gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
   gtk_container_set_border_width(GTK_CONTAINER(window), 10);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   label1=gtk_label_new("label1");
   gtk_widget_set_hexpand(label1, TRUE);

   label2=gtk_label_new("label2");
   gtk_widget_set_hexpand(label2, TRUE);
   gtk_widget_set_name (GTK_WIDGET(label2), "css_label2"); 

   event_box1=gtk_event_box_new();
   gtk_widget_set_hexpand(event_box1, TRUE);
   gtk_widget_set_vexpand(event_box1, TRUE);
   gtk_container_add(GTK_CONTAINER(event_box1), label2);
   gtk_widget_set_events(event_box1, GDK_ENTER_NOTIFY_MASK);
   gtk_widget_set_events(event_box1, GDK_LEAVE_NOTIFY_MASK);
   gtk_widget_set_events(event_box1, GDK_BUTTON_PRESS_MASK);
   g_signal_connect(event_box1, "enter-notify-event", G_CALLBACK(change_font_color_enter_event), GTK_LABEL(label2));
   g_signal_connect(event_box1, "leave-notify-event", G_CALLBACK(change_font_color_leave_event), GTK_LABEL(label2)); 
   g_signal_connect(event_box1, "button-press-event", G_CALLBACK(event_box_button_press), GTK_LABEL(label2)); 

   button1=gtk_button_new_with_label("button1");
   gtk_widget_set_hexpand(button1, TRUE);
   button_label1=gtk_bin_get_child(GTK_BIN(button1));
   g_signal_connect(button1, "enter-notify-event", G_CALLBACK(change_font_color_enter), GTK_LABEL(button_label1));
   g_signal_connect(button1, "leave-notify-event", G_CALLBACK(change_font_color_leave), GTK_LABEL(button_label1));

   button2=gtk_button_new_with_label("button2");
   gtk_widget_set_hexpand(button2, TRUE);
   button_label2=gtk_bin_get_child(GTK_BIN(button2));
   g_signal_connect(button2, "enter-notify-event", G_CALLBACK(change_font_color_enter), GTK_LABEL(button_label2));
   g_signal_connect(button2, "leave-notify-event", G_CALLBACK(change_font_color_leave), GTK_LABEL(button_label2));

   button3=gtk_button_new_with_label("button3");
   gtk_widget_set_hexpand(button3, TRUE);
   button_label3=gtk_bin_get_child(GTK_BIN(button3));
   g_signal_connect(button3, "enter-notify-event", G_CALLBACK(change_font_color_enter), GTK_LABEL(button_label3));
   g_signal_connect(button3, "leave-notify-event", G_CALLBACK(change_font_color_leave), GTK_LABEL(button_label3));
   gtk_widget_set_name(GTK_WIDGET(button3), "css_button3"); 

   button4=gtk_button_new_with_label("button4");
   gtk_widget_set_hexpand(button4, TRUE);
   button_label4=gtk_bin_get_child(GTK_BIN(button4));
   g_signal_connect(button4, "enter-notify-event", G_CALLBACK(change_font_color_enter), GTK_LABEL(button_label4));
   g_signal_connect(button4, "leave-notify-event", G_CALLBACK(change_font_color_leave), GTK_LABEL(button_label4));
   gtk_widget_set_name(GTK_WIDGET(button4), "css_button4");    

   drawing_area1=gtk_drawing_area_new();
   gtk_widget_set_size_request (drawing_area1, 300, 150);
   g_signal_connect (G_OBJECT(drawing_area1), "draw", G_CALLBACK(draw_radial_color), NULL);
   
   grid1=gtk_grid_new();
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), event_box1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button2, 0, 3, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button3, 0, 4, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button4, 0, 5, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), drawing_area1, 0, 6, 3, 1);
   
   provider = gtk_css_provider_new();
   display = gdk_display_get_default();
   screen = gdk_display_get_default_screen(display);
   gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
   if(css_error!=NULL) g_print("CSS loader error %s\n", css_error->message);
   g_object_unref(provider);

   gtk_widget_show_all(window);

   timer_id=g_timeout_add(100, animate_drawing_area, drawing_area1);

   gtk_main();
   return 0;
}
