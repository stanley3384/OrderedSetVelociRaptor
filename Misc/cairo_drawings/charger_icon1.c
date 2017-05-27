
/*
    Draw a charger icon with cairo.

    gcc -Wall charger_icon1.c -o charger_icon1 `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gint drawing_id=0;

static void combo_changed(GtkComboBox *combo_box, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_charger(GtkWidget *da, cairo_t *cr);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Charger Icon");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "Draw Charger Icon");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    gtk_widget_set_hexpand(combo, TRUE);
    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), da);  
  
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
  
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
static void combo_changed(GtkComboBox *combo_box, gpointer data)
  {
    gint combo_id=gtk_combo_box_get_active(combo_box);

    if(combo_id==0) drawing_id=0;
    else drawing_id=1;

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    if(drawing_id==0) draw_charger(da, cr);
 
    return FALSE;
  }
static void draw_charger(GtkWidget *da, cairo_t *cr)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;
    gdouble scale_line=width/400.0;
  
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

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
    
    //Draw the base
    cairo_set_line_width(cr, 3.0*scale_line);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_move_to(cr, 2.5*w1, 8.5*h1);
    cairo_line_to(cr, 7.5*w1, 8.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.75*w1, 8.65*h1, 7.75*w1, 8.85*h1, 7.5*w1, 9.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 2.5*w1, 9.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.25*w1, 8.85*h1, 2.25*w1, 8.65*h1, 2.5*w1, 8.5*h1);
    cairo_stroke_preserve(cr);
    cairo_close_path(cr);
    cairo_fill(cr);

    //Draw the middle rectangle that is filled.
    cairo_set_line_width(cr, 14.0*scale_line);
    cairo_move_to(cr, 3.0*w1, 8.5*h1);
    cairo_line_to(cr, 3.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.0*w1, 8.5*h1);
    cairo_stroke_preserve(cr);
    cairo_close_path(cr);
    cairo_fill(cr);

    //Top rectangle background.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 1.0);
    cairo_move_to(cr, 3.0*w1, 4.0*h1);
    cairo_line_to(cr, 3.0*w1, 1.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.1*w1, 1.1*h1, 3.4*w1, 1.0*h1, 3.5*w1, 1.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 6.5*w1, 1.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.6*w1, 1.0*h1, 6.9*w1, 1.1*h1, 7.0*w1, 1.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.0*w1, 4.0*h1);
    cairo_close_path(cr);
    cairo_fill(cr);

    //Gradient in the top rectangle.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(3.0*w1, 1.0*h1, 3.0*w1, 4.0*h1); 
    cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.0, 0.0, 1.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 0.1, 1.0, 1.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.2, 0.0, 0.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.3, 1.0, 1.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.4, 0.0, 0.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.5, 1.0, 1.0, 0.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 0.6, 0.0, 0.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.7, 1.0, 1.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.8, 0.0, 0.0, 1.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.9, 1.0, 1.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 0.0, 1.0, 1.0);      
    cairo_set_source(cr, pattern1);    
    cairo_rectangle(cr, 3.3*w1, 1.0*h1, 0.5*w1, 3.0*h1);
    cairo_fill(cr);
    cairo_rectangle(cr, 3.9*w1, 1.0*h1, 0.5*w1, 3.0*h1);
    cairo_fill(cr);
    cairo_rectangle(cr, 4.5*w1, 1.0*h1, 0.5*w1, 3.0*h1);
    cairo_fill(cr);
    cairo_rectangle(cr, 5.1*w1, 1.0*h1, 0.5*w1, 3.0*h1);
    cairo_fill(cr);
    cairo_rectangle(cr, 5.7*w1, 1.0*h1, 0.5*w1, 3.0*h1);
    cairo_fill(cr);
    cairo_rectangle(cr, 6.3*w1, 1.0*h1, 0.5*w1, 3.0*h1);
    cairo_fill(cr);
    cairo_pattern_destroy(pattern1);

    //Draw the top rectangle outline.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 14.0*scale_line);
    cairo_move_to(cr, 3.0*w1, 3.5*h1);
    cairo_line_to(cr, 3.0*w1, 1.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.1*w1, 1.1*h1, 3.4*w1, 1.0*h1, 3.5*w1, 1.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 6.5*w1, 1.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.6*w1, 1.0*h1, 6.9*w1, 1.1*h1, 7.0*w1, 1.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.0*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.9*w1, 3.9*h1, 6.6*w1, 4.0*h1, 6.5*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 3.5*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.4*w1, 4.0*h1, 3.1*w1, 3.9*h1, 3.0*w1, 3.5*h1);
    cairo_stroke(cr);

    //fill gap between bottom and top rectangle.
    cairo_move_to(cr, 3.0*w1, 4.0*h1);
    cairo_line_to(cr, 3.0*w1, 3.5*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 7.0*w1, 4.0*h1);
    cairo_line_to(cr, 7.0*w1, 3.5*h1);
    cairo_stroke(cr);

    //Rectangle on the side of the battery.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 1.0);
    cairo_rectangle(cr, 6.75*w1, 2.25*h1, 0.25*w1, 0.5*h1);
    cairo_fill(cr);

    //Draw the hose.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 14.0*scale_line);
    cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
    cairo_move_to(cr, 7.0*w1, 4.5*h1);
    cairo_curve_to(cr, 7.75*w1, 4.5*h1, 7.75*w1, 4.5*h1, 7.75*w1, 7.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.75*w1, 8.5*h1, 8.75*w1, 8.5*h1, 8.75*w1, 7.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.5*w1, 6.0*h1, 8.5*w1, 5.0*h1, 8.75*w1, 3.25*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.75*w1, 2.25*h1);
    cairo_stroke(cr);

    //Draw the handle.
    cairo_set_line_width(cr, 8.0*scale_line);
    cairo_move_to(cr, 8.75*w1, 3.25*h1);
    cairo_curve_to(cr, 8.0*w1, 3.5*h1, 8.0*w1, 3.5*h1, 8.25*w1, 2.75*h1);
    cairo_stroke(cr);

    //The lightning bolt.
    cairo_scale(cr, 0.5, 0.5);
    cairo_translate(cr,  width/2.0, height/2.0+height/4.0);
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
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

