
/*
    Move text around in a drawing area and draw a rectangle around the text based on the size
of the text. Draw a purple point on the shifted text that would print off screen and a blue 
point for text that isn't shifted. 

    gcc -Wall draw_characters1.c -o draw_characters1 -lm `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static guint timer_id=0;

static gboolean draw_characters(GtkWidget *widget, cairo_t *cr, gpointer *data);
static gboolean start_drawing(gpointer drawing);
static void close_program(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Cairo Characters");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    GRand *r_number=g_rand_new();
    GArray *xy_array=g_array_new(FALSE, FALSE, sizeof(gint));
    gpointer dataset[]={r_number, xy_array};

    GtkWidget *drawing=gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing, TRUE);
    gtk_widget_set_vexpand(drawing, TRUE);
    g_signal_connect(drawing, "draw", G_CALLBACK(draw_characters), dataset);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), drawing, 0, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window); 

    timer_id=g_timeout_add(800, start_drawing, drawing); 

    gtk_main();
    g_rand_free(r_number);
    g_array_free(xy_array, TRUE);

    return 0;   
  }
static gboolean draw_characters(GtkWidget *widget, cairo_t *cr, gpointer *data)
  {
    gint i=0;
    cairo_text_extents_t extents; 
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);
    gdouble rand_n1=g_rand_double((GRand*)data[0]);
    gdouble rand_n2=g_rand_double((GRand*)data[0]);
    gint rand_x=(gint)(rand_n1*(gdouble)width);
    gint rand_y=(gint)(rand_n2*(gdouble)height);

    //Array for accumulating random points. Reset back to 0 at 100 points.
    guint array_len=((GArray*)data[1])->len;
    if(array_len>100) g_array_remove_range((GArray*)data[1], 0, array_len);
 
    //Mearsure text.
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30);
    cairo_text_extents(cr, "Measure Text", &extents);

    //To keep text from going off screen right. Breaks the randomness of points on screen.
    if(rand_x+(gint)ceil(extents.width)>width)
      {
        //Mark shifted number with a negative sign.
        rand_x=-(rand_x-(gint)ceil(extents.width));
      }
    g_array_append_val((GArray*)data[1], rand_x);
    g_array_append_val((GArray*)data[1], rand_y);
     
    //Draw text.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, ABS(rand_x), rand_y); 
    cairo_show_text(cr, "Measure Text");
    cairo_stroke(cr);
    cairo_restore(cr);

    //Draw rectangle based on size of text.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);  
    cairo_rectangle(cr, ABS(rand_x), rand_y-(gint)ceil(extents.height), (gint)ceil(extents.width), (int)ceil(extents.height));
    cairo_stroke(cr); 
    cairo_restore(cr);

    //Draw point to check randomness.
    cairo_save(cr);
    array_len=((GArray*)data[1])->len;
    gint x_temp=0;
    for(i=0;i<array_len;i+=2)
      {  
        x_temp=g_array_index((GArray*)data[1], gint, i);
        if(x_temp<0)
          {
            cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
            cairo_arc(cr, ABS(x_temp), g_array_index((GArray*)data[1], gint, i+1), 4, 0, 2*G_PI);
          }
        else
          {
            cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
            cairo_arc(cr, x_temp, g_array_index((GArray*)data[1], gint, i+1), 4, 0, 2*G_PI);
          }
        cairo_fill(cr);
        cairo_stroke(cr); 
      }
    cairo_restore(cr);

    return TRUE;
  }
static gboolean start_drawing(gpointer drawing)
  {
    gtk_widget_queue_draw(GTK_WIDGET(drawing));
    return TRUE;
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    //Remove timer before quiting GTK or else GTK might issue an error.
    g_source_remove(timer_id);
    gtk_main_quit();
  }





