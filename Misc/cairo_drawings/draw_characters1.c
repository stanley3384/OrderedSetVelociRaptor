
/*
    Move text around in a drawing area and draw a rectangle around the text based on the size
of the text. 

    gcc -Wall draw_characters1.c -o draw_characters1 -lm `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static guint timer_id=0;

static gboolean draw_characters(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean start_drawing(gpointer drawing);
static void close_program(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Cairo Characters");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    GRand *r_number=g_rand_new();

    GtkWidget *drawing=gtk_drawing_area_new();
    gtk_widget_set_hexpand(drawing, TRUE);
    gtk_widget_set_vexpand(drawing, TRUE);
    g_signal_connect(drawing, "draw", G_CALLBACK(draw_characters), r_number);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), drawing, 0, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
    gtk_widget_show_all(window); 

    timer_id=g_timeout_add(800, start_drawing, drawing); 

    gtk_main();
    g_rand_free(r_number);

    return 0;   
  }
static gboolean draw_characters(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_text_extents_t extents; 
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);
    gdouble rand_n1=g_rand_double((GRand*)data);
    gdouble rand_n2=g_rand_double((GRand*)data);
    gint rand_x=(gint)(rand_n1*(gdouble)width);
    gint rand_y=(gint)(rand_n2*(gdouble)height);

    //Mearsure text.
    cairo_select_font_face(cr, "Courier", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30);
    cairo_text_extents(cr, "Measure Text", &extents);

    //To keep text from going off screen right. Breaks the randomness.
    if(rand_x+(gint)ceil(extents.width)>width) rand_x=rand_x-(gint)ceil(extents.width);

    //Draw text.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, rand_x, rand_y); 
    cairo_show_text(cr, "Measure Text");
    cairo_stroke(cr);
    cairo_restore(cr);

    //Draw rectangle based on size of text.
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);  
    cairo_rectangle(cr, rand_x, rand_y-(gint)ceil(extents.height), (gint)ceil(extents.width), (int)ceil(extents.height));
    cairo_stroke(cr); 
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





