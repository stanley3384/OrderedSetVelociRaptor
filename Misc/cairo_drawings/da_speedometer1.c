
/*

    Test drawing for a speedometer or tachometer gauge.

    gcc -Wall da_speedometer1.c -o da_speedometer1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<math.h>

static gdouble yellow_cutoff1=80.0;
static gdouble red_cutoff1=90.0;
static gdouble needle=0.0;
static gdouble scale_bottom=0.0;
static gdouble scale_top=100.0;

static gboolean draw_gage(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean time_draw(GtkWidget *da);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Speedometer Gauge");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    //Gauge drawing.    
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_gage), NULL);
   
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 3, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
   
    g_timeout_add(100, (GSourceFunc)time_draw, da1);

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
    gdouble scale_y=(gdouble)height/400.0;
    
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //transforms
    cairo_translate(cr, center_x, center_y+(scale_y));
    cairo_scale(cr, 1.10*scale_y, 1.10*scale_y);

    //Green underneath 
    cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
    cairo_set_line_width(cr, 3.0);
    cairo_arc_negative(cr, 0, 0, 100, -5.0*G_PI/3.0, -4.0*G_PI/3.0);
    cairo_line_to(cr, (cos(-4.0*G_PI/3.0)*150), sin(-4.0*G_PI/3.0)*150);
    cairo_arc(cr, 0, 0, 150, -4.0*G_PI/3.0, -5.0*G_PI/3.0);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_stroke(cr);

    gdouble diff=scale_top-scale_bottom;

    //Yellow next.
    gdouble standard_first_cutoff=(((yellow_cutoff1-scale_bottom)/diff)*(5.0*G_PI/3.0));
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_arc_negative(cr, 0, 0, 100, -5.0*G_PI/3.0, -4.0*G_PI/3.0+standard_first_cutoff);
    cairo_arc(cr, 0, 0, 150, -4.0*G_PI/3.0+standard_first_cutoff, -5.0*G_PI/3.0);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_stroke(cr);

    //Red top.
    gdouble standard_second_cutoff=(((red_cutoff1-scale_bottom)/diff)*(5.0*G_PI/3.0));
    cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
    cairo_arc_negative(cr, 0, 0, 100, -5.0*G_PI/3.0, -4.0*G_PI/3.0+standard_second_cutoff);
    cairo_arc(cr, 0, 0, 150, -4.0*G_PI/3.0+standard_second_cutoff, -5.0*G_PI/3.0);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_stroke(cr);

    //Set large tick marks.
    gint i=0;
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    gdouble tick_mark=(5.0*G_PI/3.0)/10.0;
    gdouble temp=0;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_text_extents_t tick_extents;
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 0, 0);
    for(i=0;i<11;i++)
      {
        temp=(gdouble)i*tick_mark;
        cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*140, -sin((4.0*G_PI/3.0)-temp)*140);
        cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*150, -sin((4.0*G_PI/3.0)-temp)*150);
        cairo_stroke(cr);
        //String values at bit tick marks.
        gchar *tick_string=g_strdup_printf("%i", i*10);
        cairo_text_extents(cr, tick_string, &tick_extents);
        cairo_move_to(cr, (cos((4.0*G_PI/3.0)-temp)*125)-tick_extents.width/2.0, (-sin((4.0*G_PI/3.0)-temp)*125)+tick_extents.height/2.0);
        cairo_show_text(cr, tick_string);
        g_free(tick_string);
        //Reset position to the center.
        cairo_move_to(cr, 0, 0);
      }

    //Set small tick marks.
    cairo_set_line_width(cr, 3.0);
    gdouble half_tick=tick_mark/2.0;
    cairo_move_to(cr, 0, 0);
    for(i=0;i<10;i++)
      {
        temp=(gdouble)i*tick_mark+half_tick;
        cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*145, -sin((4.0*G_PI/3.0)-temp)*145);
        cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*150, -sin((4.0*G_PI/3.0)-temp)*150);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, 0);
      }

    //The needle line.
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    gdouble standard_needle=(((needle-scale_bottom)/diff)*(5.0*G_PI/3.0));
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, cos((4.0*G_PI/3.0)-standard_needle)*110, -sin((4.0*G_PI/3.0)-standard_needle)*110);
    cairo_stroke(cr);
    
    //Text for needle value.
    cairo_text_extents_t extents1;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30);
    gchar *string1=g_strdup_printf("%3.2f", needle);
    cairo_text_extents(cr, string1, &extents1); 
    cairo_move_to(cr, -extents1.width/2, 140.0+extents1.height/2);  
    cairo_show_text(cr, string1);
    g_free(string1);

    return FALSE;
  }
static gboolean time_draw(GtkWidget *da)
  {
    static int i=0;
    g_print("Redraw Gauge %i\n", i);
    if(i==100) i=0;
    needle=i;
    gtk_widget_queue_draw(da);
    i++;
    return TRUE;
  }







