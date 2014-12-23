
/*

  Animate the windows on some buttons. One with a linear gradient and another with
a radial gradient. There is a png button but it needs a png file. It is commented out in
main.

  gcc -Wall icon.c -o icon `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon

*/

#include<gtk/gtk.h>

gint timer_id1=0;
gint timer_id2=0;
cairo_surface_t *surface=NULL;

static gboolean draw_radial_color(gpointer data)
 {
   static guint radius=10;
   guint width=gtk_widget_get_allocated_width(GTK_WIDGET(data));
   guint height=gtk_widget_get_allocated_height(GTK_WIDGET(data));
   GdkWindow *button_window=gtk_button_get_event_window(GTK_BUTTON(data));
   cairo_t *cr=gdk_cairo_create(button_window);
   cairo_pattern_t *radial1; 
   cairo_text_extents_t extents; 

   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_paint(cr);

   cairo_translate(cr, width/2, height/2);

   //Draw a radial gradient.
   if(radius>600) radius=10;
   radial1 = cairo_pattern_create_radial(0, 0, 1, 0, 0, radius);  
   cairo_pattern_add_color_stop_rgb(radial1, 0.3, 0.0, 0.0, 1.0);
   cairo_pattern_add_color_stop_rgb(radial1, 0.0, 1.0, 1.0, 0.0);
   cairo_set_source(cr, radial1);
   cairo_arc(cr, 0, 0, 300, 0, G_PI * 2);
   cairo_fill(cr); 

   radius+=20;    
  
   //Draw purple rectangle around the button.
   cairo_translate(cr, -width/2, -height/2);
   cairo_set_line_width(cr, 4.0);
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_rectangle(cr, 0, 0, width, height);
   cairo_stroke_preserve(cr); 

   //Add some text.
   cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 20);
   cairo_text_extents(cr, "Animation Button", &extents);
   cairo_move_to(cr, width/2 - extents.width/2, height/2 + extents.height/2); 
   cairo_show_text(cr, "Animation Button");  

   cairo_destroy(cr);     
   cairo_pattern_destroy(radial1);
   return TRUE;
 }
static gboolean draw_linear_color(gpointer data)
 {
   static gdouble shift=1.0;
   guint width=gtk_widget_get_allocated_width(GTK_WIDGET(data));
   guint height=gtk_widget_get_allocated_height(GTK_WIDGET(data));
   GdkWindow *button_window=gtk_button_get_event_window(GTK_BUTTON(data));
   cairo_t *cr=gdk_cairo_create(button_window);
   cairo_pattern_t *linear1; 

   //Draw a linear gradients.
   if(shift>50.0) shift=1.0;
   linear1 = cairo_pattern_create_linear(0, 0, width, 0);
   cairo_pattern_add_color_stop_rgb(linear1, shift/100, 1.0, 0.0, 0.0);
   cairo_pattern_add_color_stop_rgb(linear1, (shift+20)/100, 1.0, 1.0, 1.0);
   cairo_pattern_add_color_stop_rgb(linear1, (shift+30)/100, 0.0, 1.0, 0.0);
   cairo_pattern_add_color_stop_rgb(linear1, (shift+40)/100, 0.0, 1.0, 1.0);
   cairo_pattern_add_color_stop_rgb(linear1, (shift+50)/100, 0.0, 0.0, 1.0);
   cairo_rectangle(cr, 0, 0, width, height);
   cairo_set_source(cr, linear1);
   cairo_fill(cr); 

   shift+=1.0;    
  
   //Draw arrow
   cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
   cairo_rectangle(cr, width/3, height/2.5, width/4, height/5);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);
   cairo_move_to(cr, width/3+width/4, height/2.5); 
   cairo_line_to(cr, width/3+width/4+width/10, height/2);
   cairo_line_to(cr, width/3+width/4, height/2.5+height/5);
   cairo_close_path(cr);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);

   //Draw purple rectangle around the button.
   cairo_set_line_width(cr, 4.0);
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_rectangle(cr, 0, 0, width, height);
   cairo_stroke_preserve(cr); 

   cairo_destroy(cr);     
   cairo_pattern_destroy(linear1);
   return TRUE;
 }
static gboolean start_animation(GtkWidget *button, GdkEvent *event, gpointer data)
 {
   g_print("Start Animation %i\n", (int)GPOINTER_TO_SIZE(data));
   gint timer=(int)GPOINTER_TO_SIZE(data);
   if(timer==1)
     {
       timer_id1=g_timeout_add(50, draw_radial_color, button);
     }
   else
     {
       timer_id2=g_timeout_add(50, draw_linear_color, button);
     }
   return TRUE;
 }
static gboolean stop_animation(GtkWidget *button, GdkEvent *event, gpointer data)
 {
   g_print("Stop Animation %i\n", (int)GPOINTER_TO_SIZE(data));
   gint timer=(int)GPOINTER_TO_SIZE(data);
   
   if(timer==1)
     {
       g_source_remove(timer_id1);
     }
   else
     {
       g_source_remove(timer_id2);
     }
 
   GdkWindow *win = gtk_widget_get_window(GTK_WIDGET(button));
   if(win)
     {
       GtkAllocation allocation;
       gtk_widget_get_allocation(GTK_WIDGET(button), &allocation);
       gdk_window_invalidate_rect(win, &allocation, FALSE);
     }
   return TRUE;
 }
static void realize_and_draw(GtkWidget *button, gpointer data)
 {
   g_print("Realize\n");
   guint width=gtk_widget_get_allocated_width(GTK_WIDGET(button));
   guint height=gtk_widget_get_allocated_height(GTK_WIDGET(button));
   gtk_widget_queue_draw_area(button, 0, 0, width, height);
 }
static gboolean draw_arrow(GtkWidget *button, cairo_t *cr, gpointer data)
 {
   g_print("Draw Arrow\n");
 
   guint width=gtk_widget_get_allocated_width(GTK_WIDGET(button));
   guint height=gtk_widget_get_allocated_height(GTK_WIDGET(button));

   //Draw background
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_paint(cr);
   //Draw arrow
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_rectangle(cr, width/3, height/2.5, width/4, height/5);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);
   cairo_move_to(cr, width/3+width/4, height/2.5); 
   cairo_line_to(cr, width/3+width/4+width/10, height/2);
   cairo_line_to(cr, width/3+width/4, height/2.5+height/5);
   cairo_close_path(cr);
   cairo_stroke_preserve(cr);
   cairo_fill(cr);
   //Draw purple rectangle around the button.
   cairo_set_line_width(cr, 4.0);
   cairo_rectangle(cr, 0, 0, width, height);
   cairo_stroke(cr); 

   return TRUE;     
 }
static gboolean draw_png(GtkWidget *button, cairo_t *cr, gpointer data)
 {
   g_print("Draw PNG\n");

   cairo_text_extents_t extents; 
   guint png_width=0;
   guint png_height=0;
   guint button_width=gtk_widget_get_allocated_width(GTK_WIDGET(button));
   guint button_height=gtk_widget_get_allocated_height(GTK_WIDGET(button));

   if(surface!=NULL)
     {  
       png_width=cairo_image_surface_get_width(surface);
       png_height=cairo_image_surface_get_height(surface);   
       cairo_scale(cr, (float)button_width/(float)png_width, (float)button_height/(float)png_height);
       cairo_set_source_surface(cr, surface, 0, 0);
       cairo_paint(cr);
       cairo_scale(cr, (float)png_width/(float)button_width, (float)png_height/(float)button_height);
     }
   else
     {
       g_print("Need a PNG file.\n");
     }     

   //Add some text.
   cairo_set_source_rgb(cr, 0, 0, 0);
   cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 20);
   cairo_text_extents(cr, "PNG Button", &extents);
   cairo_move_to(cr, button_width/2 - extents.width/2, button_height/2 + extents.height/2); 
   cairo_show_text(cr, "PNG Button");  
   cairo_stroke(cr); 

   //Draw purple rectangle around the button.
   cairo_set_source_rgb(cr, 1.0, 0, 1.0);
   cairo_set_line_width(cr, 4.0);
   cairo_rectangle(cr, 0, 0, button_width, button_height);
   cairo_stroke(cr); 
   
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
   GtkWidget *window, *button1, *button2, *button3, *grid;

   gtk_init(&argc, &argv);

   gchar css_string[]="GtkButton#css_button1{background: blue; color: yellow;}\n\
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
   gtk_window_set_title(GTK_WINDOW(window), "Animation Buttons");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);
   g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

   button1=gtk_button_new_with_label("Animation Button");
   gtk_widget_set_hexpand(button1, TRUE);
   gtk_widget_set_vexpand(button1, TRUE);
   gtk_widget_set_name(GTK_WIDGET(button1), "css_button1"); 
   g_signal_connect(button1, "enter-notify-event", G_CALLBACK(start_animation), GSIZE_TO_POINTER(1));
   g_signal_connect(button1, "leave-notify-event", G_CALLBACK(stop_animation), GSIZE_TO_POINTER(1));
   g_signal_connect(button1, "button-press-event", G_CALLBACK(button_clicked), NULL); 

   button2=gtk_button_new();
   gtk_widget_set_hexpand(button2, TRUE);
   gtk_widget_set_vexpand(button2, TRUE);
   g_signal_connect(button2, "enter-notify-event", G_CALLBACK(start_animation), GSIZE_TO_POINTER(2));
   g_signal_connect(button2, "leave-notify-event", G_CALLBACK(stop_animation), GSIZE_TO_POINTER(2));
   g_signal_connect(button2, "button-press-event", G_CALLBACK(button_clicked), NULL); 
   g_signal_connect(button2, "realize", G_CALLBACK(realize_and_draw), NULL); 
   g_signal_connect(button2, "draw", G_CALLBACK(draw_arrow), NULL); 

   //surface=cairo_image_surface_create_from_png("mercator_projection.png");
   button3=gtk_button_new_with_label("Draw .png");
   gtk_widget_set_hexpand(button3, TRUE);
   gtk_widget_set_vexpand(button3, TRUE); 
   g_signal_connect(button3, "draw", G_CALLBACK(draw_png), NULL); 
   g_signal_connect(button3, "button-press-event", G_CALLBACK(button_clicked), NULL); 


   grid=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button3, 0, 2, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   cairo_surface_destroy(surface);
   return 0;
  }
   


