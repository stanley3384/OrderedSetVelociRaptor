
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
static gdouble needle=13.0;
static gdouble scale_bottom=13;
static gdouble scale_top=23;

static gboolean draw_gage(GtkWidget *da, cairo_t *cr, gpointer data);
static void change_yellow(GtkComboBox *combo, GtkWidget *da);
static void change_red(GtkComboBox *combo, GtkWidget *da);
static void change_needle(GtkComboBox *combo, GtkWidget *da);
static void change_bottom(GtkComboBox *combo, GtkWidget *da);
static void change_top(GtkComboBox *combo, GtkWidget *da);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
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
    g_signal_connect(combo1, "changed", G_CALLBACK(change_yellow), da1);
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
    g_signal_connect(combo2, "changed", G_CALLBACK(change_red), da1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 7);

    GtkWidget *combo3=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "n0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "n5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "n10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 3, "4", "n13");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 4, "5", "n50");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 5, "6", "n100");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 6, "7", "n150");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 7, "8", "n200");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 8, "9", "n250");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 9, "10", "n300");  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 3);
    g_signal_connect(combo3, "changed", G_CALLBACK(change_needle), da1);

    GtkWidget *combo4=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 0, "1", "b0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 1, "2", "b10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 2, "3", "b13");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 3, "4", "b20");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 4, "5", "b30");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo4), 2);
    g_signal_connect(combo4, "changed", G_CALLBACK(change_bottom), da1);

    GtkWidget *combo5=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 0, "1", "t23");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 1, "2", "t50");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 2, "3", "t100");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 3, "4", "t150");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 4, "5", "t200");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo5), 0);
    g_signal_connect(combo5, "changed", G_CALLBACK(change_top), da1);
   
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo4, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo5, 1, 2, 1, 1);

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

    //Needle line between 1 and 11. Standardize needle on this scale.
    gdouble diff=scale_top-scale_bottom;
    gdouble standard_needle=(((needle-scale_bottom)/diff)*10.0)+1.0;
    //g_print("N %f St %f T %f B %f diff %f\n", needle, standard_needle, scale_top, scale_bottom, diff);
    if(needle>scale_top)
      {
        g_print("Gauge overload %f!\n", needle);
        standard_needle=11.0;
      }
    if(needle<scale_bottom)
      {
        g_print("Gauge underload %f!\n", needle);
        standard_needle=1.0;
      }
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, -(cos(standard_needle*G_PI/12.0)*150), -sin(standard_needle*G_PI/12.0)*150);
    cairo_stroke(cr);

    //Text for needle value.
    cairo_text_extents_t extents1;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 28);
    gchar *string1=g_strdup_printf("%3.2f", needle);
    cairo_text_extents(cr, string1, &extents1); 
    cairo_move_to(cr, -extents1.width/2, 30.0+extents1.height/2-10);  
    cairo_show_text(cr, string1);
    g_free(string1);

    //Text for bottom end scale value.
    cairo_text_extents_t extents2;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    gchar *string2=g_strdup_printf("%3.2f", scale_bottom);
    cairo_text_extents(cr, string2, &extents2); 
    cairo_move_to(cr, -extents2.width/2-125, extents2.height/2-10);  
    cairo_show_text(cr, string2);
    g_free(string2);

    //Text for top end scale value.
    cairo_text_extents_t extents3;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 16);
    gchar *string3=g_strdup_printf("%3.2f", scale_top);
    cairo_text_extents(cr, string3, &extents3); 
    cairo_move_to(cr, -extents3.width/2+125, extents3.height/2-10);  
    cairo_show_text(cr, string3);
    g_free(string3);

    return FALSE;
  }
static void change_yellow(GtkComboBox *combo, GtkWidget *da)
  {
    gint cutoff=gtk_combo_box_get_active(combo);
    //g_print("Yellow %f\n", 13.0+(gdouble)cutoff-0.001);
    yellow_cutoff1=13.0+(gdouble)cutoff-0.001;
    gtk_widget_queue_draw(da);
  }
static void change_red(GtkComboBox *combo, GtkWidget *da)
  {
    gint cutoff=gtk_combo_box_get_active(combo);
    //g_print("Red %f\n", 13.0+(gdouble)cutoff-0.001);
    red_cutoff1=13.0+(gdouble)cutoff-0.001;
    gtk_widget_queue_draw(da);
  }
static void change_needle(GtkComboBox *combo, GtkWidget *da)
  {
    gint active=gtk_combo_box_get_active(combo);
    if(active==0) needle=0.0;
    else if(active==1) needle=5.0;
    else if(active==2) needle=10.0;
    else if(active==3) needle=13.0;
    else if(active==4) needle=50.0;
    else if(active==5) needle=100.0;
    else if(active==6) needle=150.0;
    else if(active==7) needle=200.0;
    else if(active==8) needle=250.0;
    else if(active==9) needle=300.0;
    else scale_bottom=13.0;
    gtk_widget_queue_draw(da);
  }
static void change_bottom(GtkComboBox *combo, GtkWidget *da)
  {
    gint active=gtk_combo_box_get_active(combo);
    if(active==0) scale_bottom=0.0;
    else if(active==1) scale_bottom=10.0;
    else if(active==2) scale_bottom=13.0;
    else if(active==3) scale_bottom=20.0;
    else if(active==4) scale_bottom=30.0;
    else scale_bottom=13.0;
    gtk_widget_queue_draw(da);
  }
static void change_top(GtkComboBox *combo, GtkWidget *da)
  {
    gint active=gtk_combo_box_get_active(combo);
    if(active==0) scale_top=23.0;
    else if(active==1) scale_top=50.0;
    else if(active==2) scale_top=100.0;
    else if(active==3) scale_top=150.0;
    else if(active==4) scale_top=200.0;
    else scale_top=23.0;
    gtk_widget_queue_draw(da);
  }








