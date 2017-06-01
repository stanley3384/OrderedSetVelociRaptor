
/*
    Draw a few gears.

    gcc -Wall gears1.c -o gears1 `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gint drawing_id=0;

static void combo_changed(GtkComboBox *combo_box, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_gears(GtkWidget *da, cairo_t *cr);
static void gear(cairo_t *cr, gdouble w1);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gears");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "Gear");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "Gears");
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
    draw_gears(da, cr); 
    return FALSE;
  }
static void draw_gears(GtkWidget *da, cairo_t *cr)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Scale.
    if(w1>h1)
      {
        w1=h1;
      }
  
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

    if(drawing_id==0)
      {
        cairo_save(cr);
        cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        cairo_set_line_width(cr, 5.0);
        cairo_translate(cr, width/2.0, height/2.0); 
        gear(cr, w1);
        cairo_restore(cr);
      } 
    else
      {
        //Draw some gears.
        cairo_save(cr);
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_set_line_width(cr, 5.0);
        cairo_translate(cr, 3.0*w1, 3.0*h1);
        cairo_scale(cr, 0.5, 0.5); 
        gear(cr, w1);
        cairo_restore(cr);

        cairo_save(cr);
        cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        cairo_set_line_width(cr, 5.0);
        cairo_translate(cr, 7.0*w1, 3.0*h1);
        cairo_rotate(cr, G_PI/12.0);
        cairo_scale(cr, 0.5, 0.5); 
        gear(cr, w1);
        cairo_restore(cr);

        cairo_save(cr);
        cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        cairo_set_line_width(cr, 5.0);
        cairo_translate(cr, 3.0*w1, 7.0*h1);
        cairo_scale(cr, 0.5, 0.5); 
        gear(cr, w1);
        cairo_restore(cr);

        cairo_save(cr);
        cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
        cairo_set_line_width(cr, 5.0);
        cairo_translate(cr, 7.0*w1, 7.0*h1);
        cairo_rotate(cr, G_PI/12.0);
        cairo_scale(cr, 0.5, 0.5); 
        gear(cr, w1);
        cairo_restore(cr);       
      }   
  }
static void gear(cairo_t *cr, gdouble w1)
  {
    gint i=0;

    //Outside points.
    gdouble outside_start1=-G_PI/2.0-G_PI/36.0;
    gdouble outside_next1=-G_PI/6.0;
    gdouble outside_radius1=4.0*w1;
    gdouble outside_cos1=0;
    gdouble outside_sin1=0;
    gdouble outside_start2=-G_PI/2.0+G_PI/36.0;
    gdouble outside_next2=-G_PI/6.0;
    gdouble outside_radius2=4.0*w1;
    gdouble outside_cos2=0;
    gdouble outside_sin2=0;
    //Inside points.
    gdouble inside_start1=-G_PI/2.0+G_PI/12.0-G_PI/36.0;
    gdouble inside_next1=-G_PI/6.0;
    gdouble inside_radius1=3.2*w1;
    gdouble inside_cos1=0;
    gdouble inside_sin1=0;
    gdouble inside_start2=-G_PI/2.0+G_PI/12.0+G_PI/36.0;
    gdouble inside_next2=-G_PI/6.0;
    gdouble inside_radius2=3.2*w1;
    gdouble inside_cos2=0;
    gdouble inside_sin2=0;
    for(i=0;i<12;i++)
      {
        outside_cos1=cos(outside_start1-(outside_next1*i))*outside_radius1;
        outside_sin1=sin(outside_start1-(outside_next1*i))*outside_radius1;                
        if(i==0) cairo_move_to(cr, outside_cos1, outside_sin1);
        else
          {
            cairo_line_to(cr, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);
          }

        outside_cos2=cos(outside_start2-(outside_next2*i))*outside_radius2;
        outside_sin2=sin(outside_start2-(outside_next2*i))*outside_radius2;                
        cairo_line_to(cr, outside_cos2, outside_sin2);
        cairo_stroke_preserve(cr);   
         
        inside_cos1=cos(inside_start1-(inside_next1*i))*inside_radius1;
        inside_sin1=sin(inside_start1-(inside_next1*i))*inside_radius1;
        cairo_line_to(cr, inside_cos1, inside_sin1);
        cairo_stroke_preserve(cr); 

        inside_cos2=cos(inside_start2-(inside_next2*i))*inside_radius2;
        inside_sin2=sin(inside_start2-(inside_next2*i))*inside_radius2;
        cairo_line_to(cr, inside_cos2, inside_sin2);
        cairo_stroke_preserve(cr);                          
     }
    cairo_close_path(cr);
    cairo_stroke(cr);
    cairo_arc(cr, 0.0, 0.0, 2.5*w1, 0, 2*G_PI);
    cairo_stroke(cr);  
  }
