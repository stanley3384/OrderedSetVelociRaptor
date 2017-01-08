/*
    Draw a lightning icon for replacing text in the circuit breaker switch widget.

    gcc -Wall lightning1.c -o lightning1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
*/

#include<gtk/gtk.h>

static gint drawing=0;
static gint translate=0;

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void button_clicked(GtkWidget *button, GtkWidget *da);
static void translate_drawing(GtkWidget *combo, GtkWidget *da);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Lightning Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *button=gtk_button_new_with_label("Toggle");
   gtk_widget_set_hexpand(button, TRUE);
   g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), da);

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
static void button_clicked(GtkWidget *button, GtkWidget *da)
 {
   if(drawing==0) drawing=1;
   else if(drawing==1) drawing=2;
   else if(drawing==2) drawing=0;
   else drawing=0;
   gtk_widget_queue_draw(da);
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

   cairo_set_line_width(cr, 1.0);
   
   if(translate==1) cairo_translate(cr, width/4.0, 0.0);
   if(translate==2) cairo_translate(cr, width/3.0, 0.0);

   //Lightning bolt.
   if(drawing==0||drawing==1)
     {
       cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
       cairo_move_to(cr, 6.0*width/10.0, 1.0*height/10.0);
       cairo_line_to(cr, 4.0*width/10.0, 5.5*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.0*width/10.0, 5.5*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 4.0*width/10.0, 9.0*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 6.0*width/10.0, 4.5*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_line_to(cr, 5.0*width/10.0, 4.5*height/10.0);
       cairo_stroke_preserve(cr);
       cairo_close_path(cr);
       cairo_fill(cr);
       cairo_stroke(cr);
     }

   //Lightning break.
   if(drawing==1)
     {
       cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
       cairo_set_line_width(cr, 10.0*height/400.0);
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

   //Charges.
   if(drawing==2)
     {
       gint i=3;
       cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
       cairo_set_line_width(cr, 14.0*height/400.0);       
       //Negatives
       for(i=2;i<9;i+=3)
         {
           cairo_move_to(cr, 5.5*width/10.0, (gdouble)i*height/10.0);
           cairo_line_to(cr, 6.5*width/10.0, (gdouble)i*height/10.0);
           cairo_stroke(cr);
         }
       //Positives
       for(i=2;i<9;i+=3)
         {
           cairo_move_to(cr, 3.5*width/10.0, (gdouble)i*height/10.0);
           cairo_line_to(cr, 4.5*width/10.0, (gdouble)i*height/10.0);
           cairo_stroke(cr);
         }
       for(i=2;i<9;i+=3)
         {
           cairo_move_to(cr, 4.0*width/10.0, ((gdouble)i-0.5)*height/10.0);
           cairo_line_to(cr, 4.0*width/10.0, ((gdouble)i+0.5)*height/10.0);
           cairo_stroke(cr);
         }
     }

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

   return FALSE;
}








