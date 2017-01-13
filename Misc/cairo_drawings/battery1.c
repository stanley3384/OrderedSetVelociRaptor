/*
    Test code for drawing a battery icon and animating it for charging.

    gcc -Wall battery1.c -o battery1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
*/

#include<gtk/gtk.h>

//Translate with combo box.
static gint translate=0;
static gdouble animate=1.0;
static gboolean charging=FALSE;
static guint timer_id=0;

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void button_clicked(GtkWidget *button, GtkWidget *widgets[]);
static gboolean draw_battery(GtkWidget *widgets[]);
static void translate_drawing(GtkWidget *combo, GtkWidget *da);
static void exit_program(GtkWidget *widget, gpointer user_data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Battery Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(exit_program), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *button=gtk_button_new_with_label("Charge");
   gtk_widget_set_hexpand(button, TRUE);
   GtkWidget *widgets[]={button, da};
   g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), widgets);

   GtkWidget *combo=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "Center");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "Translate +1/4");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "Translate +1/3");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
   g_signal_connect(combo, "changed", G_CALLBACK(translate_drawing), da);

   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), combo, 0, 2, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static void button_clicked(GtkWidget *button, GtkWidget *widgets[])
 {
   animate=9.0;
   charging=TRUE;
   gtk_widget_queue_draw(widgets[1]);
   gtk_widget_set_sensitive(button, FALSE);
   g_timeout_add(100, (GSourceFunc)draw_battery, widgets);
 }
static gboolean draw_battery(GtkWidget *widgets[])
 {
   if(animate>1.0)
     {
       animate-=0.05;
       gtk_widget_queue_draw(widgets[1]);
       return TRUE;
     }
   else
     {
       animate=1.0;
       gtk_widget_queue_draw(widgets[1]);
       g_print("Timer Done\n");
       charging=FALSE;
       gtk_widget_set_sensitive(widgets[0], TRUE);
       timer_id=0;
       return FALSE;
     }
 }
static void translate_drawing(GtkWidget *combo, GtkWidget *da)
 {
   gint row=gtk_combo_box_get_active(GTK_COMBO_BOX(combo));
   if(row==1) translate=1;
   else if(row==2) translate=2;
   else translate=0;
   gtk_widget_queue_draw(da);
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_paint(cr);
   
   if(translate==1) cairo_translate(cr, width/4.0, 0.0);
   if(translate==2) cairo_translate(cr, width/3.0, 0.0);

   //Battery background rectangle.
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_rectangle(cr, 3.0*width/10.0, height/10.0, 4.0*width/10.0, 8.0*height/10.0);
   cairo_fill(cr);

   //The red, yellow to green gradient.
   cairo_pattern_t *pattern1=cairo_pattern_create_linear(4.0*width/10.0, height/10.0, 4.0*width/10.0, 8.0*height/10.0); 
   cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.0, 1.0, 0.0, 1.0); 
   cairo_pattern_add_color_stop_rgba(pattern1, 0.5, 1.0, 1.0, 0.0, 1.0);
   cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 1.0, 0.0, 0.0, 1.0);  
   cairo_set_source(cr, pattern1);    
   cairo_rectangle(cr, 3.0*width/10.0, animate*height/10.0, 4.0*width/10.0, (9.0-animate)*height/10.0);
   cairo_fill(cr);

   //Battery outline rectangle.
   cairo_set_line_width(cr, 8.0);
   cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
   cairo_rectangle(cr, 3.0*width/10.0, height/10.0, 4.0*width/10.0, 8.0*height/10.0);
   cairo_stroke(cr);

   //Battery top.
   cairo_rectangle(cr, 4.5*width/10.0, height/20.0, 1.0*width/10.0, height/20.0);
   cairo_fill(cr);

   //Lightning bolt.
   if(charging)
     {
       cairo_set_line_width(cr, 1.0);
       cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
       cairo_move_to(cr, 6.0*width/10.0, 2.0*height/10.0);
       cairo_line_to(cr, 4.25*width/10.0, 5.25*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.0*width/10.0, 5.25*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 4.0*width/10.0, 8.0*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.75*width/10.0, 4.75*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.0*width/10.0, 4.75*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_close_path(cr);
       cairo_fill(cr);
       cairo_stroke(cr);
     }

   //Add percentage text.
   cairo_text_extents_t extents1;
   cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_set_font_size(cr, 16);
   gchar *percent=g_strdup_printf("%.2f", ((9.0-animate)/8.0)*100.0);
   cairo_text_extents(cr, percent, &extents1);
   cairo_move_to(cr, width/2.0 - extents1.width/2, 9.5*height/10.0 + extents1.height/2);  
   cairo_show_text(cr, percent);
   g_free(percent);

   if(translate==1) cairo_translate(cr, -width/4.0, 0.0);
   if(translate==2) cairo_translate(cr, -width/3.0, 0.0);

   //Layout axis for drawing.
   cairo_set_line_width(cr, 1.0);
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_rectangle(cr, width/10.0, height/10.0, 8.0*width/10.0, 8.0*height/10.0);
   cairo_stroke(cr);
   cairo_move_to(cr, 1.0*width/10.0, 5.0*height/10.0);
   cairo_line_to(cr, 9.0*width/10.0, 5.0*height/10.0);
   cairo_stroke(cr);
   cairo_move_to(cr, 5.0*width/10.0, 1.0*height/10.0);
   cairo_line_to(cr, 5.0*width/10.0, 9.0*height/10.0);
   cairo_stroke(cr);

   cairo_pattern_destroy(pattern1);

   return FALSE;
}
static void exit_program(GtkWidget *widget, gpointer user_data)
{
   if(timer_id!=0) g_source_remove(timer_id);
   gtk_main_quit();
}








