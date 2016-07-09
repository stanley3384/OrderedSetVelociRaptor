
/*

    Draw a mandelbrot set onto a GDK pixbuf then use cairo to draw a couple of bug eyes.
 
    Try both of the following to see if there is a difference in the drawing. There is more green
with the -O2 flag on my test computer.

    gcc -Wall -Werror mandelbrot1.c -o mandelbrot1 `pkg-config --cflags --libs gtk+-3.0`
    gcc -Wall -Werror -O2 mandelbrot1.c -o mandelbrot1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/
  
#include <gtk/gtk.h>

//How big the mandelbrot drawing should be.
#define PICTURE_ROWS 500
#define PICTURE_COLUMNS 500

//The status of the mandelbrot drawing. It can take some time.
static gint status=0;
//For the progress bar.
static gint columns_done=0;
//Iterations for the mandelbrot drawing.
static gint max_iteration=25;

static void start_drawing_thread(gpointer widgets_pixbuf[]);
//Draw the mandelbrot set on the pixbuf on a seperate thread.
static gpointer draw_mandelbrot(GdkPixbuf *pixbuf);
//Use cairo to draw a couple of bug eyes.
static gboolean draw_mandelbrot_bug(GtkWidget *da, cairo_t *cr, GdkPixbuf *pixbuf);
//Check to see if the drawing is done. If it is, put it in the drawing area.
static gboolean check_pixbuf_status(gpointer widgets_pixbuf[]);
static void combo_changed(GtkComboBox *combo_box, gpointer data);
static void redraw_bug(GtkWidget *button, gpointer widgets_pixbuf[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 550);
    gtk_window_set_title(GTK_WINDOW(window), "Mandelbrot Bug");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    GdkPixbuf *pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, PICTURE_ROWS, PICTURE_COLUMNS);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 400, 400);
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(draw_mandelbrot_bug), pixbuf);

    GtkWidget *view=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view), da);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "MaxIteration=5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "MaxIteration=10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "MaxIteration=15");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 3, "4", "MaxIteration=20");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 4, "5", "MaxIteration=25");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 5, "6", "MaxIteration=30");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 6, "7", "MaxIteration=35");    
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 4);
    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), NULL);
    gtk_widget_set_sensitive(combo, FALSE);

    GtkWidget *button=gtk_button_new_with_label("Redraw Bug");
    gtk_widget_set_sensitive(button, FALSE);

    GtkWidget *progress=gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);

    GtkWidget *statusbar=gtk_statusbar_new();
    gtk_statusbar_push(GTK_STATUSBAR(statusbar), 0, "Drawing Mandelbrot Bug");
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), progress, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), statusbar, 0, 3, 2, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
    
    //Draw the initial mandelbrot bug and hook up button click for re-draws.
    gpointer widgets_pixbuf[]={da, progress, statusbar, combo, button, pixbuf};
    g_signal_connect(button, "clicked", G_CALLBACK(redraw_bug), widgets_pixbuf);
    start_drawing_thread(widgets_pixbuf);

    gtk_widget_show_all(window);                  
    gtk_main();

    g_object_unref(pixbuf);

    return 0;
  }
static void start_drawing_thread(gpointer widgets_pixbuf[])
  {
    g_timeout_add(300, (GSourceFunc)check_pixbuf_status, widgets_pixbuf);
    g_thread_new("thread1", (GThreadFunc)draw_mandelbrot, widgets_pixbuf[5]);
  }
static gpointer draw_mandelbrot(GdkPixbuf *pixbuf)
  {
    GTimer *timer=g_timer_new();
    gdouble x1=0.0;
    gdouble y1=0.0;
    gdouble x2=0.0;
    gdouble y2=0.0;
    gint iteration;
    gdouble scale=1.0;
    gdouble temp1=0.0;
    gdouble temp2=0.0;
    gdouble max_value=0.0;
    gint i=0;
    gint j=0;
    gint width=gdk_pixbuf_get_width(pixbuf);
    gint height=gdk_pixbuf_get_height(pixbuf);
    gint step=gdk_pixbuf_get_rowstride(pixbuf);
    gint channels=gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels, *p;
    pixels=gdk_pixbuf_get_pixels(pixbuf);

    for(i=0;i<width;i++)
      {
        g_atomic_int_set(&columns_done, i);
        for(j=0;j<height;j++)
          {
            //Get some mandelbrot values. Change things around.
            x1=0.0;
            y1=0.0;
            //Scale
            x2=(((gdouble)i)/((gdouble)width)-0.5)/scale*3.0-0.7;
            y2=(((gdouble)j)/((gdouble)height)-0.5)/scale*3.0;
            //Test different starting points.
            for(iteration=2;iteration<max_iteration;iteration++)
              {
                temp1=x1*x1-y1*y1+x2;
                y1=2.0*x1*y1+y2;
                x1=temp1;                
              }

            //Fill in the pixbuf.
            temp2=x1*x1+y1*y1;
            if(temp2>max_value) max_value=temp2;
            p=pixels+j*step+i*channels;
            if(temp2<100.0)
              {
                if(temp2<0.2)
                  {
                    p[0]=(gint)(255.0-(255.0*temp2/0.2));
                    p[1]=0;
                    p[2]=(gint)(0.0+(255.0*temp2/0.2));
                  }
                else
                  {
                    p[0]=0;
                    p[1]=0;
                    p[2]=255;
                  }
              }
            else
              {
                p[0]=0;
                p[1]=(gint)(255.0-(255.0*temp2/max_value));
                p[2]=0;
              }
          }
      }
    g_atomic_int_set(&status, 1);
    g_print("Mandelbrot Drawing Time %f\n", g_timer_elapsed(timer, NULL));
    g_timer_destroy(timer);
    return NULL;
  }
static gboolean draw_mandelbrot_bug(GtkWidget *da, cairo_t *cr, GdkPixbuf *pixbuf)
  {
    if(g_atomic_int_get(&status)==1)
      {
        gdk_cairo_set_source_pixbuf(cr, pixbuf, 0, 0);
        cairo_paint(cr);
        cairo_scale(cr, (gdouble)PICTURE_ROWS/500.0, (gdouble)PICTURE_COLUMNS/500.0);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_arc(cr, 185, 235, 8, 0, 2*G_PI);
        cairo_fill(cr);
        cairo_stroke(cr);
        cairo_arc(cr, 185, 265, 8, 0, 2*G_PI);
        cairo_fill(cr);
        cairo_stroke(cr);
      }
    else
      {
        cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_paint(cr);
        cairo_text_extents_t extents;
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 30.0);
        cairo_text_extents(cr, "Drawing...", &extents); 
        cairo_move_to(cr, PICTURE_ROWS/2-extents.width/2, PICTURE_COLUMNS/2-extents.height/2);  
        cairo_show_text(cr, "Drawing...");
      }
    return FALSE;
  }
static gboolean check_pixbuf_status(gpointer widgets_pixbuf[])
  {
    gint i=0;
    static gint j=1;
    if(g_atomic_int_get(&status)==1)
      {
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets_pixbuf[1]), (gdouble)g_atomic_int_get(&columns_done)/(gdouble)PICTURE_ROWS);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets_pixbuf[1]), "Drawing Done");
        gtk_statusbar_pop(GTK_STATUSBAR(widgets_pixbuf[2]), 0);
        gtk_statusbar_push(GTK_STATUSBAR(widgets_pixbuf[2]), 0, "Drawing Done");
        gtk_widget_queue_draw(widgets_pixbuf[0]);
        gtk_widget_set_sensitive(widgets_pixbuf[3], TRUE);
        gtk_widget_set_sensitive(widgets_pixbuf[4], TRUE);
        j=1;
        return FALSE;
      }
    else 
      {
        GString *string=g_string_new("Drawing Mandelbrot Bug");
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets_pixbuf[1]), (gdouble)g_atomic_int_get(&columns_done)/(gdouble)PICTURE_COLUMNS);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(widgets_pixbuf[1]), string->str);
        for(i=0;i<j;i++) g_string_append_c(string, '.');
        gtk_statusbar_pop(GTK_STATUSBAR(widgets_pixbuf[2]), 0);
        gtk_statusbar_push(GTK_STATUSBAR(widgets_pixbuf[2]), 0, string->str);
        g_string_free(string, TRUE);
        j++;
        return TRUE;
      }
  }
static void combo_changed(GtkComboBox *combo_box, gpointer data)
  {
    gint combo_id=gtk_combo_box_get_active(combo_box);
    switch(combo_id)
      {
        case 0:
          max_iteration=5;
          break;
        case 1:
          max_iteration=10;
          break;
        case 2:
          max_iteration=15;
          break;
        case 3:
          max_iteration=20;
          break;
        case 4:
          max_iteration=25;
          break;
        case 5:
          max_iteration=30;
          break;
        case 6:
          max_iteration=35;
          break;
        default:
          max_iteration=25;
      }
  }
static void redraw_bug(GtkWidget *button, gpointer widgets_pixbuf[])
  {
    gtk_widget_set_sensitive(button, FALSE);
    gtk_widget_set_sensitive(widgets_pixbuf[3], FALSE);
    g_atomic_int_set(&status, 0);
    start_drawing_thread(widgets_pixbuf);
  }



