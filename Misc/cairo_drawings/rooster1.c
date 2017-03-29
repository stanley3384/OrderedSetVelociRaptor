
/*
    Draw a rooster for alarm_clock1.c. Maybe a dialog for when the alarm goes off? An easy
way to turn it off? 

    gcc -Wall rooster1.c -o rooster1 `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean draw_rooster(GtkWidget *da, cairo_t *cr, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Rooster Icon");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(draw_rooster), NULL);  
  
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
  
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
static gboolean draw_rooster(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;
  
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //Draw the rooster body.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_move_to(cr, 8.0*w1, 3.0*h1);
    cairo_curve_to(cr, 9.0*w1, 4.0*h1, 9.0*w1, 7.5*h1, 5.0*w1, 7.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 4.0*w1, 7.0*h1, 3.0*w1, 6.0*h1, 3.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.0*w1, 4.0*h1, 2.0*w1, 4.0*h1, 2.0*w1, 4.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 3.5*h1, 2.5*w1, 3.25*h1, 2.75*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 3.0*h1, 1.5*w1, 3.25*h1, 1.5*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 1.5*w1, 2.5*h1, 2.0*w1, 2.5*h1, 2.5*w1, 3.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.0*w1, 2.0*h1, 1.5*w1, 2.0*h1, 1.0*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 1.5*w1, 1.0*h1, 2.5*w1, 1.0*h1, 4.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.0*w1, 5.0*h1, 6.0*w1, 4.0*h1, 6.5*w1, 2.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.5*w1, 2.5*h1, 5.5*w1, 2.25*h1, 6.5*w1, 2.25*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.75*w1, 2.0*h1, 5.75*w1, 1.75*h1, 6.75*w1, 2.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.25*w1, 1.5*h1, 6.25*w1, 1.25*h1, 7.0*w1, 1.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.0*w1, 1.0*h1, 7.25*w1, 1.0*h1, 7.5*w1, 1.8*h1);
    cairo_stroke_preserve(cr);    
    cairo_curve_to(cr, 7.7*w1, 1.6*h1, 8.0*w1, 1.4*h1, 8.5*w1, 1.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.0*w1, 1.6*h1, 7.7*w1, 1.8*h1, 7.75*w1, 2.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.7*w1, 2.0*h1, 8.0*w1, 1.8*h1, 8.5*w1, 1.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.25*w1, 2.0*h1, 8.15*w1, 2.0*h1, 7.85*w1, 2.15*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 8.4*w1, 2.5*h1, 8.5*w1, 2.7*h1, 8.0*w1, 3.0*h1);
    //cairo_close_path(cr);
    cairo_stroke(cr);

    //First leg and foot.
    cairo_move_to(cr, 5.0*w1, 7.0*h1);
    cairo_line_to(cr, 5.5*w1, 8.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 5.0*w1, 8.5*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5*w1, 8.5*h1);
    cairo_line_to(cr, 6.0*w1, 8.3*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5*w1, 8.5*h1);
    cairo_line_to(cr, 6.2*w1, 8.6*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.5*w1, 8.5*h1);
    cairo_line_to(cr, 6.0*w1, 8.9*h1);
    cairo_stroke(cr);
    //Second leg and foot.
    cairo_move_to(cr, 5.5*w1, 7.0*h1);
    cairo_line_to(cr, 6.75*w1, 8.25*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 6.25*w1, 8.25*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.75*w1, 8.25*h1);
    cairo_line_to(cr, 7.25*w1, 8.05*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.75*w1, 8.25*h1);
    cairo_line_to(cr, 7.45*w1, 8.35*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 6.75*w1, 8.25*h1);
    cairo_line_to(cr, 7.25*w1, 8.65*h1);
    cairo_stroke(cr);
    
    //Draw the eye.
    cairo_arc(cr, 7.25*w1, 1.85*h1, 0.07*h1, 0.0, 2.0*G_PI);
    cairo_fill(cr);  

    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    //Layout axis for drawing.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 1.0*w1, 5.0*h1);
    cairo_line_to(cr, 9.0*w1, 5.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*w1, 1.0*h1);
    cairo_line_to(cr, 5.0*w1, 9.0*h1);
    cairo_stroke(cr);
 
    return FALSE;
 }

