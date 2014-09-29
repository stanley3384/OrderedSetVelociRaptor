
/*

Test code for trying out some CSS with GTK. A couple of color gradients on buttons, a colored event box under text and a drawing area with some animation. Add a worker thread for testing what it takes to bind up the UI on heavy draws. 

gcc -Wall css1.c -o css1 `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<stdlib.h>

gint timer_id=0;
gint busy=0;
guint radius_t=100;
guint sleep_value=500000;

//Use the threaded code path or not.
gboolean use_thread_code=TRUE;
//Use a worker thread or not in the threaded code path.
gboolean thread_code_path=TRUE;

//For threading.
cairo_surface_t *cairo_surface1=NULL;
GTimer *timer2=NULL;
guint width_t=0;
guint height_t=0;

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

   GTimer *timer1=g_timer_new();
   //Block UI for testing.
   g_usleep(sleep_value);
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

   radius+=20;    
       
   cairo_pattern_destroy(radial1);

   g_timer_stop(timer1);
   g_print("Draw Timer %f\n", g_timer_elapsed(timer1, NULL));  
   g_timer_destroy(timer1);
   
   return TRUE;
 }

//Drawing code with a worker thread.
static gboolean end_thread(GThread *thread)
 {
   g_thread_join(thread);
   return FALSE;
 }
static void *draw_radial_color_t2(cairo_surface_t *cairo_surface1)
 {
   //g_print("Start Drawing\n");
   //Cairo context to draw with.
   cairo_t *cr2=cairo_create(cairo_surface1);
  
   //Slow the drawing thread down. Cairo can draw very fast.
   g_usleep(g_atomic_int_get(&sleep_value));
   //Draw on surface.
   cairo_pattern_t *radial1;    
   cairo_set_source_rgba(cr2, 0.0, 0.0, 1.0, 1.0);
   cairo_paint(cr2);
   cairo_translate(cr2, width_t/2, height_t/2);  
   if(radius_t>1500) radius_t=100;
   radial1 = cairo_pattern_create_radial(0, 0, 1, 0, 0, radius_t);  
   cairo_pattern_add_color_stop_rgb(radial1, 0.3, 0.0, 0.0, 1.0);
   cairo_pattern_add_color_stop_rgb(radial1, 0.0, 1.0, 0.0, 0.0);
   cairo_set_source(cr2, radial1);
   cairo_arc(cr2, 0, 0, 300, 0, G_PI * 2);
   cairo_fill(cr2); 
   cairo_translate(cr2, width_t*2, height_t*2);
   radius_t+=20;           
   cairo_pattern_destroy(radial1);
   cairo_paint(cr2);

   //g_print("End Drawing\n");
   cairo_destroy(cr2);
   g_atomic_int_set(&busy, 0);
   if(thread_code_path) gdk_threads_add_idle((GSourceFunc)end_thread, g_thread_self());
   g_timer_stop(timer2);
   g_print("Thread Draw Timer %f\n", g_timer_elapsed(timer2, NULL));  
   return NULL;
 }
gboolean draw_radial_color_t1(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   static guint drawing=0;
   static guint check_drawing=0;
   gint width=gtk_widget_get_allocated_width(widget);
   gint height=gtk_widget_get_allocated_height(widget);

   //Reallocate surface when screen size changes.
   if(drawing!=0)
     {
       if(width!=width_t||height!=height_t)
         {
           width_t=width;
           height_t=height;
           cairo_surface_destroy(cairo_surface1);
           cairo_surface1=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width_t, height_t);
         }
     }

   //Initialize a independent drawing surface.
   if(drawing==0)
     {
       width_t=width;
       height_t=height;
       cairo_surface1=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width_t, height_t);
       timer2=g_timer_new();
     }

    //A new drawing is ready.
    if(check_drawing>drawing&&g_atomic_int_get(&busy)==0)
     {
       //g_print("Update Surface\n");  
       cairo_set_source_surface(cr, cairo_surface1, 0, 0);
       cairo_paint(cr);  
       drawing++;
       check_drawing=drawing;
     }

   //Start a drawing thread and send the surface to be drawn on.
   if(drawing>0&&g_atomic_int_get(&busy)==0)
     {
       g_timer_start(timer2);
       g_atomic_int_set(&busy, 1);
       //g_print("Drawing %i\n", drawing);
       if(thread_code_path)
         { 
           //g_print("Start Thread\n");
         }
       if(thread_code_path) g_thread_new("drawing_thread", (GThreadFunc)draw_radial_color_t2, cairo_surface1);
       else draw_radial_color_t2(cairo_surface1);
       check_drawing++;
     }
   else //g_print("Skip Drawing\n");
     
   //Draw blue background on first draw.
   if(drawing==0)
     {
       drawing++;
       cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
       cairo_paint(cr);
     }
  
   return TRUE;
 }
static gboolean animate_drawing_area(gpointer data)
 {  
   //g_print("Timer Fired\n");
   gtk_widget_queue_draw_area(GTK_WIDGET(data), 0, 0, gtk_widget_get_allocated_width(GTK_WIDGET(data)), gtk_widget_get_allocated_height(GTK_WIDGET(data)));
      
   return TRUE;
 }
gboolean select_draw_path(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   if(!use_thread_code&&g_atomic_int_get(&busy)==0)
     {
       gtk_widget_set_double_buffered(widget, TRUE);
       draw_radial_color(widget, cr, data);
     }    
   else
     {
       gtk_widget_set_double_buffered(widget, FALSE);
       draw_radial_color_t1(widget, cr, data);
     }
   return TRUE;
 }
static void set_threading_path(GtkWidget *combo1, gpointer *data)
 {
   if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo1)))==0) 
     {
       use_thread_code=FALSE;
     }
   else if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo1)))==1) 
     {
       use_thread_code=TRUE;
       thread_code_path=FALSE;
     }
   else if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo1)))==2) 
     {
       use_thread_code=TRUE;
       thread_code_path=TRUE;
     }
 }
static void set_sleep(GtkWidget *combo2, gpointer *data)
 {
   if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo2)))==0) 
     {
       g_atomic_int_set(&sleep_value, 0);
     }
   else if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo2)))==1) 
     {
       g_atomic_int_set(&sleep_value, 500000);
     }
   else if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo2)))==2) 
     {
       g_atomic_int_set(&sleep_value, 1000000);
     }
   else
     {
       g_atomic_int_set(&sleep_value, 2000000);
     }    
 }
static void close_program()
 {
   if(use_thread_code) g_timer_destroy(timer2);
   cairo_surface_destroy(cairo_surface1);
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   printf("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   GtkWidget *window, *combo1, *combo2, *label1, *label2, *button1, *button2, *button3, *button4, *button_label1, *button_label2, *button_label3, *button_label4, *event_box1, *drawing_area1, *grid1;
   gchar css_string[]="GtkButton{background: blue}\n\
                       GtkLabel{color: black}\n\
                       GtkComboBoxText{color: white}\n\
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
   gtk_window_set_title(GTK_WINDOW(window), "Thread Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
   gtk_container_set_border_width(GTK_CONTAINER(window), 10);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   combo1=gtk_combo_box_text_new();     
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "Single Thread");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "Single Thread T_path");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "Worker Thread T_path");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 2);
   g_signal_connect(combo1, "changed", G_CALLBACK(set_threading_path), NULL);

   combo2=gtk_combo_box_text_new();     
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "0", "Timer 0 Second");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "1", "Timer 1/2 Second");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "2", "Timer 1 Seconds");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "3", "Timer 2 Seconds");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 1);
   g_signal_connect(combo2, "changed", G_CALLBACK(set_sleep), NULL);
     
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
   g_signal_connect(G_OBJECT(drawing_area1), "draw", G_CALLBACK(select_draw_path), NULL);

   //if(use_thread_code) gtk_widget_set_double_buffered(drawing_area1, FALSE);
   //if(use_thread_code) g_signal_connect(G_OBJECT(drawing_area1), "draw", G_CALLBACK(draw_radial_color_t1), NULL);
   //else g_signal_connect(G_OBJECT(drawing_area1), "draw", G_CALLBACK(draw_radial_color), NULL);
   
   grid1=gtk_grid_new();
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_grid_attach(GTK_GRID(grid1), combo1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), combo2, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), event_box1, 0, 3, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 4, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button2, 0, 5, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button3, 0, 6, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button4, 0, 7, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), drawing_area1, 0, 8, 3, 1);
   
   provider = gtk_css_provider_new();
   display = gdk_display_get_default();
   screen = gdk_display_get_default_screen(display);
   gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
   gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
   if(css_error!=NULL) g_print("CSS loader error %s\n", css_error->message);
   g_object_unref(provider);

   gtk_widget_show_all(window);

   timer_id=gdk_threads_add_timeout(100, animate_drawing_area, drawing_area1);

   gtk_main();
   return 0;
}
