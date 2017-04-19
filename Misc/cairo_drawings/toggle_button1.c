
/*
    Test drawing on a toggle button. 

    gcc -Wall toggle_button1.c -o toggle_button1 `pkg-config --cflags --libs gtk+-3.0`

    Tested with GTK3.18 and Ubuntu16.04

    C. Eric Cashon
*/

#include <gtk/gtk.h>

gboolean draw_button1(GtkWidget *widget, cairo_t *cr, gpointer user_data)
  {
    guint width = gtk_widget_get_allocated_width(GTK_WIDGET(widget));
    guint height = gtk_widget_get_allocated_height(GTK_WIDGET(widget));

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      {
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_paint(cr);
      }
    else
      {
        cairo_set_source_rgb(cr, 0.1, 0.1, 0.1);
        cairo_paint(cr);
      }
    
    //Draw rectangle around the button.
    cairo_set_line_width(cr, 10.0);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr); 

    //Add the text.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_text_extents_t extents; 
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 20);
    cairo_text_extents(cr, "Cairo Toggle!", &extents);
    cairo_move_to(cr, width/2 - extents.width/2, height/2 + extents.height/2); 
    cairo_show_text(cr, "Cairo Toggle!");  

    return TRUE;
  }
int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    gtk_window_set_title(GTK_WINDOW(window), "Toggle Button");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *button1=gtk_toggle_button_new_with_label("button1");
    gtk_widget_set_app_paintable(button1, TRUE);
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, TRUE);
    g_signal_connect(button1, "draw", G_CALLBACK(draw_button1), NULL);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);                  
    gtk_main();
    return 0;
}
