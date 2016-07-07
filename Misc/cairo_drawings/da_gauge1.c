
/*

    Test drawing for a gauge. 

    gcc -Wall -Werror da_gauge1.c -o da_gauge1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<math.h>

//Yellow and red cutoff between 13 and 23. Eleven lines from 11*pi/12 to pi/12. A scale of 11.
static gdouble yellow_cutoff1=17.0;
static gdouble red_cutoff1=20.0;
//Needle line between 1 and 11.
static gdouble needle=1.0;

static gboolean draw_gage(GtkWidget *da, cairo_t *cr, gpointer data);
static void change_yellow(GtkComboBox *combo, gpointer data);
static void change_red(GtkComboBox *combo, gpointer data);
static void change_needle(GtkComboBox *combo, gpointer data);
static void draw_gauge(GtkWidget *button, GtkWidget *da);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Gauge");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    //Gauge drawing.    
    GtkWidget *da1=gtk_drawing_area_new();
    //gtk_widget_set_size_request(da1, 400, 400);
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_gage), NULL);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "y1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "y2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "y3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "y4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "y5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "y6");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 6, "7", "y7");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 7, "8", "y8");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 8, "9", "y9");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 9, "10", "y10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 10, "11", "y11");  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
    g_signal_connect(combo1, "changed", G_CALLBACK(change_yellow), NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 5);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "r1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "r2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "r3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "r4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 4, "5", "r5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 5, "6", "r6");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 6, "7", "r7");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 7, "8", "r8");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 8, "9", "r9");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 9, "10", "r10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 10, "11", "r11");  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
    g_signal_connect(combo2, "changed", G_CALLBACK(change_red), NULL);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 7);

    GtkWidget *combo3=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "n1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "n2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "n3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 3, "4", "n4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 4, "5", "n5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 5, "6", "n6");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 6, "7", "n7");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 7, "8", "n8");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 8, "9", "n9");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 9, "10", "n10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 10, "11", "n11");  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);
    g_signal_connect(combo3, "changed", G_CALLBACK(change_needle), NULL);
   
    GtkWidget *button=gtk_button_new_with_label("Test Gauge");
    gtk_widget_set_hexpand(button, TRUE);
    g_signal_connect(button, "clicked", G_CALLBACK(draw_gauge), da1);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 3, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean draw_gage(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);
    gint center_x=width/2;
    gint center_y=height/2;
    //Original drawing 400x400.
    gdouble scale_y=(gdouble)height/400.0;
    
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_paint(cr);

    //transforms
    cairo_translate(cr, center_x, center_y+(50*scale_y));
    cairo_scale(cr, 1.2*scale_y, 1.2*scale_y);

    //Green underneath 
    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
    cairo_set_line_width(cr, 3.0);
    cairo_arc_negative(cr, 0, 0, 100, 23.0*G_PI/12.0, 13.0*G_PI/12.0);
    cairo_line_to(cr, -(cos(G_PI/12.0)*150), -sin(G_PI/12.0)*150);
    cairo_arc(cr, 0, 0, 150, 13.0*G_PI/12.0, 23.0*G_PI/12.0);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_stroke(cr);

    //Yellow next.
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_arc_negative(cr, 0, 0, 100, 23.0*G_PI/12.0, yellow_cutoff1*G_PI/12.0);
    cairo_arc(cr, 0, 0, 150, yellow_cutoff1*G_PI/12.0, 23.0*G_PI/12.0);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_stroke(cr);

    //Red top.
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    cairo_arc_negative(cr, 0, 0, 100, 23.0*G_PI/12.0, red_cutoff1*G_PI/12.0);
    cairo_arc(cr, 0, 0, 150, red_cutoff1*G_PI/12.0, 23.0*G_PI/12.0);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_stroke(cr);

    //Needle line between 1 and 11
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, -(cos(needle*G_PI/12.0)*150), -sin(needle*G_PI/12.0)*150);
    cairo_stroke(cr);

    //Text for needle value.
    cairo_text_extents_t extents;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 28);
    gchar *string=g_strdup_printf("%3.2f", needle);
    cairo_text_extents(cr, string, &extents); 
    cairo_move_to(cr, -extents.width/2, 30.0+extents.height/2);  
    cairo_show_text(cr, string);
    g_free(string);

    return FALSE;
  }
static void change_yellow(GtkComboBox *combo, gpointer data)
  {
    gint cutoff=gtk_combo_box_get_active(combo);
    //g_print("Yellow %f\n", 13.0+(gdouble)cutoff-0.001);
    yellow_cutoff1=13.0+(gdouble)cutoff-0.001;
  }
static void change_red(GtkComboBox *combo, gpointer data)
  {
    gint cutoff=gtk_combo_box_get_active(combo);
    //g_print("Red %f\n", 13.0+(gdouble)cutoff-0.001);
    red_cutoff1=13.0+(gdouble)cutoff-0.001;
  }
static void change_needle(GtkComboBox *combo, gpointer data)
  {
    needle=(gdouble)(gtk_combo_box_get_active(combo)+1);
  }
static void draw_gauge(GtkWidget *button, GtkWidget *da)
  {
    gtk_widget_queue_draw(da);
  }









