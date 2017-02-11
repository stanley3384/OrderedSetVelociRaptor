
/*
    Draw a animated graph with cairo.

    gcc -Wall draw_graph1.c -o draw_graph1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
   
    C. Eric Cashon
*/

#include <gtk/gtk.h>

#define numbers 50
static double test_numbers[numbers];
static GRand *rand1;
static const double dashed_line[]={20.0, 5.0};
static gint dashed_len=2;

static void get_random_numbers(void);
static gboolean redraw(gpointer data);
static gboolean draw_points(GtkWidget *widget, cairo_t *cr, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 300);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Graph");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    rand1=g_rand_new();
    get_random_numbers();

    GtkWidget *drawing_area = gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 500, 300);   
    g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_points), NULL);

    gtk_container_add(GTK_CONTAINER(window), drawing_area);

    g_timeout_add(1000, redraw, drawing_area);

    gtk_widget_show_all(window);
    
    gtk_main();

    g_rand_free(rand1);

    return 0;
  }
static gboolean redraw(gpointer data)
  {
    get_random_numbers();
    gtk_widget_queue_draw(GTK_WIDGET(data));
    return TRUE;
  }
static void get_random_numbers(void)
  {
    gint i=0;    
    for(i=0;i<numbers;i++) test_numbers[i]=g_rand_double(rand1);
  }
static gboolean draw_points(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gint i=0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);
     
    //Set a rgb pattern.   
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(width/20.0, 19.0*height/20.0, width/20.0, height/20.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.0, 1.0, 0.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 0.5, 1.0, 1.0, 0.0, 1.0);
    cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 1.0, 0.0, 0.0, 1.0);  
    cairo_set_source(cr, pattern1); 

    //Draw random points and lines.
    gdouble run=((18.0*width/20.0)/(numbers));
    gdouble x=(width/20.0);
    gdouble y=(height/20.0);
    cairo_move_to(cr, width/20.0, 19.0*height/20.0);
    for(i=0;i<numbers;i++)
       {
         cairo_line_to(cr, run*i+x, (18.0*height/20.0)*test_numbers[i]+y);
       }
    cairo_line_to(cr, 19.0*width/20.0, 19.0*height/20.0);
    cairo_close_path(cr);
    cairo_fill(cr);

    //Outside rectangle.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 5.0);
    cairo_move_to(cr, width/20.0, height/20.0);
    cairo_line_to(cr, width/20.0, 19.0*height/20.0);
    cairo_line_to(cr, 19.0*width/20.0, 19.0*height/20.0);
    cairo_line_to(cr, 19.0*width/20.0, height/20.0);
    cairo_close_path(cr);
    cairo_stroke(cr);

    //Horizontal dashed lines.
    cairo_set_line_width(cr, 1.0);
    cairo_set_dash(cr, dashed_line, dashed_len, 0);
    cairo_move_to(cr, width/20.0, 5.5*height/20.0);
    cairo_line_to(cr, 19.0*width/20.0, 5.5*height/20.0);
    cairo_stroke(cr);
    cairo_move_to(cr, width/20.0, 10.0*height/20.0);
    cairo_line_to(cr, 19.0*width/20.0, 10.0*height/20.0);
    cairo_stroke(cr);
    cairo_move_to(cr, width/20.0, 14.5*height/20.0);
    cairo_line_to(cr, 19.0*width/20.0, 14.5*height/20.0);
    cairo_stroke(cr);

    //Vertical dashed lines.
    cairo_move_to(cr, 5.5*width/20.0, height/20.0);
    cairo_line_to(cr, 5.5*width/20.0, 19.0*height/20.0);
    cairo_stroke(cr);
    cairo_move_to(cr, 10.0*width/20.0, height/20.0);
    cairo_line_to(cr, 10.0*width/20.0, 19.0*height/20.0);
    cairo_stroke(cr);
    cairo_move_to(cr, 14.5*width/20.0, height/20.0);
    cairo_line_to(cr, 14.5*width/20.0, 19.0*height/20.0);
    cairo_stroke(cr);

    cairo_pattern_destroy(pattern1);
     
    return TRUE;
  }
