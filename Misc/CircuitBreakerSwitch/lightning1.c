/*
    Draw a lightning icon for text replacement in the circuit breaker switch widget.

    gcc -Wall lightning1.c -o lightning1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
*/

#include<gtk/gtk.h>

gboolean lightning_break=FALSE;

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void toggle_button(GtkToggleButton *button, GtkWidget *da);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Lightning Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *button=gtk_toggle_button_new_with_label("Toggle Break");
   gtk_widget_set_hexpand(button, TRUE);
   g_signal_connect(button, "toggled", G_CALLBACK(toggle_button), da);

   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static void toggle_button(GtkToggleButton *button, GtkWidget *da)
 {
   if(lightning_break) lightning_break=FALSE;
   else lightning_break=TRUE;
   gtk_widget_queue_draw(da);
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_paint(cr);

   cairo_set_line_width(cr, 1.0);

   //Lightning bolt.
   cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
   cairo_move_to(cr, 7.0*width/10.0, 1.0*height/10.0);
   cairo_line_to(cr, 4.5*width/10.0, 4.5*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_line_to(cr, 3.0*width/10.0, 6.0*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_line_to(cr, 4.5*width/10.0, 5.5*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_line_to(cr, 3.0*width/10.0, 9.0*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_line_to(cr, 5.5*width/10.0, 5.5*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_line_to(cr, 7.0*width/10.0, 4.0*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_line_to(cr, 5.5*width/10.0, 4.5*height/10.0);
   cairo_stroke_preserve(cr);
   cairo_close_path(cr);
   cairo_fill(cr);
   cairo_stroke(cr);

   //Lightning break.
   if(lightning_break)
     {
       cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
       cairo_set_line_width(cr, 10.0);
       cairo_move_to(cr, 3.0*width/10.0, 5.15*height/10.0);
       cairo_line_to(cr, 3.5*width/10.0, 4.85*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 4.0*width/10.0, 5.15*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 4.5*width/10.0, 4.85*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.0*width/10.0, 5.15*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.5*width/10.0, 4.85*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 6.0*width/10.0, 5.15*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 6.5*width/10.0, 4.85*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 7.0*width/10.0, 5.15*height/10.0);
       cairo_stroke(cr);
     }

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

   return FALSE;
}








