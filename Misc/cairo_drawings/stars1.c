
/*
    Put some transparent buttons over a drawing area in an overlay. The buttons are from
music_buttons.py. Check out the motion of the stars.

    gcc -Wall stars1.c -o stars1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/

#include <gtk/gtk.h>

static gboolean set_grid_width_height(GtkOverlay *overlay, GtkWidget *widget, GdkRectangle *allocation, gpointer data);
//Draw the animated stars.
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_stars(cairo_t *cr, gint width, gint height, gdouble *coord);
static gboolean start_drawing(gpointer drawing);
//Draw the widgets for putting in the overlay.
static gboolean draw_play(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean draw_stop(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean draw_forward(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean draw_backward(GtkWidget *widget, cairo_t *cr, gpointer data);
//Start and pause animation.
static void play_clicked(GtkWidget *widget, gpointer data);
//Draw the widgets white when pressed.
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean stop_release(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean stop_pressed=FALSE;  
static gboolean forward_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean forward_release(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean forward_pressed=FALSE;  
static gboolean backward_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean backward_release(GtkWidget *widget, GdkEvent *event, gpointer data);
gboolean backward_pressed=FALSE; 
//Show and hide the overlay widgets.
static void show_widget(GtkWidget *widget, gpointer data);

//The speed of moving stars.
gdouble speed=4.0;

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    GtkWidget *window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Stars");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    gint i=0;
    gdouble width=20.0;
    gdouble height=20.0;
    GRand *rand=g_rand_new();
    //Shouldn't have a problem getting 240 doubles but it might be good to check the return value.
    gdouble *coord=g_malloc(240*sizeof(gdouble));
    /*
       Initialize stars in four quadrants with the origin at the center. The points are stored
    in groups of four. The values are x coordinate, y coordinate, x slope, y slope. 240/4=60 points.
    The following loop adds four points. One point in each quadrant for each pass through the loop
    body.
    */      
    for(i=0;i<240;i+=16)
      {
        //Quadrant I
        coord[i]=width*g_rand_double(rand);
        coord[i+1]=height*g_rand_double(rand);
        coord[i+2]=g_rand_double(rand);
        coord[i+3]=g_rand_double(rand);
        //Quadrant II
        coord[i+4]=-width*g_rand_double(rand);
        coord[i+5]=height*g_rand_double(rand);
        coord[i+6]=-g_rand_double(rand);
        coord[i+7]=g_rand_double(rand);
        //Quadrant III
        coord[i+8]=-width*g_rand_double(rand);
        coord[i+9]=-height*g_rand_double(rand);
        coord[i+10]=-g_rand_double(rand);
        coord[i+11]=-g_rand_double(rand);
        //Quadrant IV
        coord[i+12]=width*g_rand_double(rand);
        coord[i+13]=-height*g_rand_double(rand);
        coord[i+14]=g_rand_double(rand);
        coord[i+15]=-g_rand_double(rand);
      }
    g_rand_free(rand);

    GtkWidget *da = gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(draw_background), coord);

    GtkWidget *overlay=gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);
    g_signal_connect(overlay, "get-child-position", G_CALLBACK(set_grid_width_height), NULL);
    gtk_container_add(GTK_CONTAINER(overlay), da);

    GtkWidget *toggle1=gtk_toggle_button_new();
    gtk_widget_set_hexpand(toggle1, TRUE);
    gtk_widget_set_vexpand(toggle1, TRUE);
    gtk_widget_set_app_paintable(toggle1, TRUE);
    g_signal_connect(toggle1, "draw", G_CALLBACK(draw_play), NULL);
    g_signal_connect(toggle1, "clicked", G_CALLBACK(play_clicked), NULL);

    GtkWidget *button1=gtk_button_new();
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_widget_set_vexpand(button1, TRUE);
    gtk_widget_set_app_paintable(button1, TRUE);
    g_signal_connect(button1, "draw", G_CALLBACK(draw_stop), NULL);
    g_signal_connect(button1, "button-press-event", G_CALLBACK(stop_press), toggle1);
    g_signal_connect(button1, "button-release-event", G_CALLBACK(stop_release), NULL);

    GtkWidget *button2=gtk_button_new();
    gtk_widget_set_hexpand(button2, TRUE);
    gtk_widget_set_vexpand(button2, TRUE);
    gtk_widget_set_app_paintable(button2, TRUE);
    g_signal_connect(button2, "draw", G_CALLBACK(draw_forward), NULL);
    g_signal_connect(button2, "button-press-event", G_CALLBACK(forward_press), NULL);
    g_signal_connect(button2, "button-release-event", G_CALLBACK(forward_release), NULL);

    GtkWidget *button3=gtk_button_new();
    gtk_widget_set_hexpand(button3, TRUE);
    gtk_widget_set_vexpand(button3, TRUE);
    gtk_widget_set_app_paintable(button3, TRUE);
    g_signal_connect(button3, "draw", G_CALLBACK(draw_backward), NULL);
    g_signal_connect(button3, "button-press-event", G_CALLBACK(backward_press), NULL);
    g_signal_connect(button3, "button-release-event", G_CALLBACK(backward_release), NULL);

    GtkWidget *grid1=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid1), button3, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), button2, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), toggle1, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), button1, 3, 0, 1, 1);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), grid1);

    GtkWidget *toggle2=gtk_toggle_button_new_with_label("Hide Widget");
    gtk_widget_set_hexpand(toggle2, TRUE);
    g_signal_connect(toggle2, "clicked", G_CALLBACK(show_widget), grid1);

    GtkWidget *grid2=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid2), overlay, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid2), toggle2, 0, 4, 4, 1);

    gtk_container_add(GTK_CONTAINER(window), grid2);

    g_timeout_add(100, start_drawing, da); 

    gtk_widget_show_all(window);

    gtk_main();

    g_free(coord);

    return 0;
  }
static gboolean set_grid_width_height(GtkOverlay *overlay, GtkWidget *widget, GdkRectangle *allocation, gpointer data)
  {
    gdouble width1=(gdouble)gtk_widget_get_allocated_width(GTK_WIDGET(overlay));
    gdouble height1=(gdouble)gtk_widget_get_allocated_height(GTK_WIDGET(overlay));

    gdouble allo_y=320.0*height1/400.0;
    gdouble allo_height=60.0*height1/400.0;

    allocation->x=0;
    allocation->y=allo_y;
    allocation->width=width1;
    allocation->height=allo_height;
    
    return TRUE;
  }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    //Get drawing area size.
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);

    //Paint the background.
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    //Draw the stars.
    if(!stop_pressed) draw_stars(cr, width, height, (gdouble*)data);

    return FALSE;
  }
static void draw_stars(cairo_t *cr, gint width, gint height, gdouble *coord)
  {
    gint i=0;
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 5);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND); 
    for(i=0;i<240;i+=4)
      {
        cairo_move_to(cr, coord[i]+((gdouble)width/2.0), coord[i+1]+((gdouble)height/2.0));
        cairo_line_to(cr, coord[i]+1.0+((gdouble)width/2.0), coord[i+1]+((gdouble)height/2.0));
        cairo_stroke(cr);
        coord[i]+=(speed*coord[i+2]);
        coord[i+1]+=(speed*coord[i+3]);
        if(ABS(coord[i])>(gdouble)width/2.0||ABS(coord[i+1])>(gdouble)height/2.0)
          {
            coord[i]=0;
            coord[i+1]=0;
          }
      }
  }
static gboolean start_drawing(gpointer drawing)
  {
    gtk_widget_queue_draw(GTK_WIDGET(drawing));
    return TRUE;
  }
static gboolean draw_play(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble scale_x=width/120.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      {
        if(!forward_pressed&&!backward_pressed) speed=0;
        cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
        cairo_paint(cr);
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_set_line_width(cr, 6);
        cairo_move_to(cr, center_x-6, center_y-15);
        cairo_line_to(cr, center_x-6, center_y+15);
        cairo_stroke(cr);
        cairo_move_to(cr, center_x+6, center_y-15);
        cairo_line_to(cr, center_x+6, center_y+15);
        cairo_stroke(cr);
      }
    else
      {
        if(!forward_pressed&&!backward_pressed) speed=4;
        cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
        cairo_paint(cr);
        cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
        cairo_set_line_width(cr, 6);
        cairo_move_to(cr, center_x-10, center_y-15);
        cairo_line_to(cr, center_x-10, center_y+15);
        cairo_line_to(cr, center_x+20, center_y);
        cairo_line_to(cr, center_x-10 , center_y-15);
        cairo_fill(cr);
        cairo_stroke(cr);
      }

    return TRUE;
  }
static gboolean draw_stop(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble scale_x=width/120.0;
    gdouble scale_y=height/40.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    if(stop_pressed) cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    else cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x-10, center_y-10);
    cairo_rectangle(cr, center_x-10, center_y-10, 30, 20);
    cairo_fill(cr);
    cairo_stroke(cr);

    return TRUE;
  }
static gboolean draw_forward(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble scale_x=width/100.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    if(forward_pressed) cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    else cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x-30, center_y-15);
    cairo_line_to(cr, center_x-30, center_y+15);
    cairo_line_to(cr, center_x+0, center_y);
    cairo_line_to(cr, center_x-30 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+5, center_y-15);
    cairo_line_to(cr, center_x+5, center_y+15);
    cairo_line_to(cr, center_x+35, center_y);
    cairo_line_to(cr, center_x+5 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+40, center_y-15);
    cairo_line_to(cr, center_x+40, center_y+15);
    cairo_stroke(cr);

    return TRUE;
  }
static gboolean draw_backward(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble scale_x=width/100.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    if(backward_pressed) cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    else cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x, center_y-15);
    cairo_line_to(cr, center_x, center_y+15);
    cairo_line_to(cr, center_x-30, center_y);
    cairo_line_to(cr, center_x, center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+35, center_y-15);
    cairo_line_to(cr, center_x+35, center_y+15);
    cairo_line_to(cr, center_x+5, center_y);
    cairo_line_to(cr, center_x+35 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x-40, center_y-15);
    cairo_line_to(cr, center_x-40, center_y+15);
    cairo_stroke(cr);

    return TRUE;
  }
static void play_clicked(GtkWidget *widget, gpointer data)
  {
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      {
        stop_pressed=FALSE;
      }
  }
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    stop_pressed=TRUE;
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(data), TRUE);

    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GdkWindow *win = gtk_button_get_event_window(GTK_BUTTON(widget));
    cairo_t *cr = gdk_cairo_create(win);

    gdouble scale_x=width/120.0;
    gdouble scale_y=height/40.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x-10, center_y-10);
    cairo_rectangle(cr, center_x-10, center_y-10, 40, 30);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_destroy(cr);

    return TRUE;
  }
static gboolean stop_release(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GdkWindow *win = gtk_button_get_event_window(GTK_BUTTON(widget));
    cairo_t *cr = gdk_cairo_create(win);

    gdouble scale_x=width/120.0;
    gdouble scale_y=height/40.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x-10, center_y-10);
    cairo_rectangle(cr, center_x-10, center_y-10, 40, 30);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_destroy(cr);

    return TRUE;
  }
static gboolean forward_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    forward_pressed=TRUE;
    speed=8.0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GdkWindow *win = gtk_button_get_event_window(GTK_BUTTON(widget));
    cairo_t *cr = gdk_cairo_create(win);

    gdouble scale_x=width/100.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x-30, center_y-15);
    cairo_line_to(cr, center_x-30, center_y+15);
    cairo_line_to(cr, center_x+0, center_y);
    cairo_line_to(cr, center_x-30 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+5, center_y-15);
    cairo_line_to(cr, center_x+5, center_y+15);
    cairo_line_to(cr, center_x+35, center_y);
    cairo_line_to(cr, center_x+5 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+40, center_y-15);
    cairo_line_to(cr, center_x+40, center_y+15);
    cairo_stroke(cr);

    cairo_destroy(cr);

    return TRUE;
  }
static gboolean forward_release(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    forward_pressed=FALSE;
    speed=4.0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GdkWindow *win = gtk_button_get_event_window(GTK_BUTTON(widget));
    cairo_t *cr = gdk_cairo_create(win);

    gdouble scale_x=width/100.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);
      
    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x-30, center_y-15);
    cairo_line_to(cr, center_x-30, center_y+15);
    cairo_line_to(cr, center_x+0, center_y);
    cairo_line_to(cr, center_x-30 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+5, center_y-15);
    cairo_line_to(cr, center_x+5, center_y+15);
    cairo_line_to(cr, center_x+35, center_y);
    cairo_line_to(cr, center_x+5 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+40, center_y-15);
    cairo_line_to(cr, center_x+40, center_y+15);
    cairo_stroke(cr);

    cairo_destroy(cr);

    return TRUE;
  }
static gboolean backward_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    backward_pressed=TRUE;
    speed=-8.0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GdkWindow *win = gtk_button_get_event_window(GTK_BUTTON(widget));
    cairo_t *cr = gdk_cairo_create(win);

    gdouble scale_x=width/100.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);

    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x, center_y-15);
    cairo_line_to(cr, center_x, center_y+15);
    cairo_line_to(cr, center_x-30, center_y);
    cairo_line_to(cr, center_x, center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+35, center_y-15);
    cairo_line_to(cr, center_x+35, center_y+15);
    cairo_line_to(cr, center_x+5, center_y);
    cairo_line_to(cr, center_x+35 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x-40, center_y-15);
    cairo_line_to(cr, center_x-40, center_y+15);
    cairo_stroke(cr);

    cairo_destroy(cr);

    return TRUE;
  }
static gboolean backward_release(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    backward_pressed=FALSE;
    speed=4.0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GdkWindow *win = gtk_button_get_event_window(GTK_BUTTON(widget));
    cairo_t *cr = gdk_cairo_create(win);

    gdouble scale_x=width/100.0;
    gdouble scale_y=height/50.0;
    gdouble center_x=(width/2.0)/scale_x; 
    gdouble center_y=(height/2.0)/scale_y;  
    cairo_scale(cr, scale_x, scale_y);

    cairo_set_source_rgba(cr, 0.25, 0.25, 0.25, 0.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_move_to(cr, center_x, center_y-15);
    cairo_line_to(cr, center_x, center_y+15);
    cairo_line_to(cr, center_x-30, center_y);
    cairo_line_to(cr, center_x, center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x+35, center_y-15);
    cairo_line_to(cr, center_x+35, center_y+15);
    cairo_line_to(cr, center_x+5, center_y);
    cairo_line_to(cr, center_x+35 , center_y-15);
    cairo_fill(cr);
    cairo_stroke(cr);

    cairo_move_to(cr, center_x-40, center_y-15);
    cairo_line_to(cr, center_x-40, center_y+15);
    cairo_stroke(cr);

    cairo_destroy(cr);

    return TRUE;
  }
static void show_widget(GtkWidget *widget, gpointer data)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      {
        gtk_widget_hide(GTK_WIDGET(data));
        gtk_button_set_label(GTK_BUTTON(widget), "Show Widget");
      }
    else
      {
        gtk_widget_show(GTK_WIDGET(data));
        gtk_button_set_label(GTK_BUTTON(widget), "Hide Widget");
      }
  }








