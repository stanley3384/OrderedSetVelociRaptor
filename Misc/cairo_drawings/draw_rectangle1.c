
/*

    Layer two drawing areas with an overlay. Draw rectangles, circles and lines on the top layer
and add them to the bottom layer. This way you can add shapes and line art to a cairo surface
and be able to clear them.

    gcc -Wall draw_rectangle1.c -o draw_rectangle1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/

#include<gtk/gtk.h>

//Bottom drawing area for the green rectangle.
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer *data);
//Top drawing area for the red rectangle.
static gboolean draw_foreground(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data);
//Add the rectangle from the top layer to the bottom layer.
static void add_rectangle(GtkWidget *widget, gpointer *data);
static void clear_rectangles(GtkWidget *widget, gpointer *data);
//Change combo box row id's.
static void combo1_changed(GtkComboBox *combo1, gpointer data);
static void combo2_changed(GtkComboBox *combo2, gpointer data);


//For blocking motion signal. Block when not drawing top rectangle
static gint motion_id=0;
//Coordinates for the drawing rectangle. Fifth number is color of rectangle.
static gdouble rect[]={0.0, 0.0, 0.0, 0.0, 0.0};
static gdouble shape_combo1=0.0;
static gdouble color_combo2=0.0;

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    GtkWidget *window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Rectangle");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    //Save some rectangles and circles for drawing in da1.
    GArray *rectangles=g_array_new(FALSE, FALSE, sizeof(gdouble));
    GArray *circles=g_array_new(FALSE, FALSE, sizeof(gdouble));
    GArray *lines=g_array_new(FALSE, FALSE, sizeof(gdouble));
    gpointer shapes[]={rectangles, circles, lines};

    //Bottom drawing area.
    GtkWidget *da1 = gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_background), shapes);

    GtkWidget *overlay=gtk_overlay_new();
    gtk_widget_set_hexpand(overlay, TRUE);
    gtk_widget_set_vexpand(overlay, TRUE);
    gtk_container_add(GTK_CONTAINER(overlay), da1);

    //Top drawing area
    GtkWidget *da2 = gtk_drawing_area_new();
    gtk_widget_set_hexpand(da2, TRUE);
    gtk_widget_set_vexpand(da2, TRUE);
    //Add some extra events to the top drawing area.
    gtk_widget_add_events(da2, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK);
    g_signal_connect(da2, "draw", G_CALLBACK(draw_foreground), NULL);
    g_signal_connect(da2, "button-press-event", G_CALLBACK(start_press), NULL);
    g_signal_connect(da2, "button-release-event", G_CALLBACK(stop_press), NULL);
    motion_id=g_signal_connect(da2, "motion-notify-event", G_CALLBACK(cursor_motion), NULL);
    g_signal_handler_block(da2, motion_id);

    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), da2);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Rectangle");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Circle");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Line");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), NULL);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "red");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "green");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "blue");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
    g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), NULL);

    GtkWidget *button1=gtk_button_new_with_label("Add Shape");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(add_rectangle), shapes);

    gpointer clear[]={rectangles, circles, lines, da1};
    GtkWidget *button2=gtk_button_new_with_label("Clear Shapes");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(button2, "clicked", G_CALLBACK(clear_rectangles), clear);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), overlay, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 2, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 2, 5, 2, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    g_array_free(rectangles, TRUE);
    g_array_free(circles, TRUE);
    g_array_free(lines, TRUE);

    return 0;
  }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer *data)
  {
    //Get drawing area size.
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);

    //Paint the background white.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //Put a green rectangle in the background.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 20, 20, width-40, height-40);
    cairo_stroke(cr);

    //Draw the stored shapes.
    gint i=0;
    gdouble x1=0;
    gdouble y1=0;
    gdouble x2=0;
    gdouble y2=0;
    gdouble color=0;
    
    gint len1=((GArray*)data[0])->len/5;
    gint len2=((GArray*)data[1])->len/5;
    gint len3=((GArray*)data[2])->len/5;

    //Draw saved rectangles.
    for(i=0;i<len1;i++)
      {
        x1=g_array_index((GArray*)data[0], gdouble, 5*i);
        y1=g_array_index((GArray*)data[0], gdouble, 5*i+1);
        x2=g_array_index((GArray*)data[0], gdouble, 5*i+2);
        y2=g_array_index((GArray*)data[0], gdouble, 5*i+3);
        color=g_array_index((GArray*)data[0], gdouble, 5*i+4);

        if(color==0) cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        else if(color==1) cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);

        cairo_rectangle(cr, x1, y1, x2-x1, y2-y1);
        cairo_stroke(cr);
      }

    //Draw saved circles. 
    for(i=0;i<len2;i++)
      {
        //g_print("Len %i\n", len2);
        x1=g_array_index((GArray*)data[1], gdouble, 5*i);
        y1=g_array_index((GArray*)data[1], gdouble, 5*i+1);
        x2=g_array_index((GArray*)data[1], gdouble, 5*i+2);
        y2=g_array_index((GArray*)data[1], gdouble, 5*i+3);
        color=g_array_index((GArray*)data[1], gdouble, 5*i+4);
     
        if(color==0) cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        else if(color==1) cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);

        gdouble axis_x=(x2-x1)/2.0;
        gdouble axis_y=(y2-y1)/2.0;
        gdouble radius=0.5*(x2-x1);
        if(axis_x>0.0&&axis_y>0)
          {
            cairo_save(cr);
            cairo_translate(cr, x1+axis_x, y1+axis_y);
            cairo_scale(cr, axis_x/axis_y, axis_y/axis_x);
            cairo_arc(cr, 0.0, 0.0, radius, 0, 2*G_PI);
            cairo_restore(cr);
            cairo_stroke(cr);
          }
      }

    //Draw saved lines.
    for(i=0;i<len3;i++)
      {
        x1=g_array_index((GArray*)data[2], gdouble, 5*i);
        y1=g_array_index((GArray*)data[2], gdouble, 5*i+1);
        x2=g_array_index((GArray*)data[2], gdouble, 5*i+2);
        y2=g_array_index((GArray*)data[2], gdouble, 5*i+3);
        color=g_array_index((GArray*)data[2], gdouble, 5*i+4);
     
        if(color==0) cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
        else if(color==1) cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
        else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);

        cairo_move_to(cr, x1, y1);
        cairo_line_to(cr, x2, y2);
        cairo_stroke(cr);
      }

    return TRUE;
  }
static gboolean draw_foreground(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    //Paint the foreground transparent.
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);

    //Draw shapes.
    if(color_combo2==0) cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    else if(color_combo2==1) cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);

    cairo_set_line_width(cr, 6);
    if(shape_combo1==0)
      {
        cairo_rectangle(cr, rect[0], rect[1], rect[2]-rect[0], rect[3]-rect[1]);
        cairo_stroke(cr);
      }
    else if(shape_combo1==1)
      {
        gdouble axis_x=(rect[2]-rect[0])/2.0;
        gdouble axis_y=(rect[3]-rect[1])/2.0;
        gdouble radius=0.5*(rect[2]-rect[0]);
        if(axis_x>0.0&&axis_y>0)
          {
            cairo_save(cr);
            cairo_translate(cr, rect[0]+axis_x, rect[1]+axis_y);
            cairo_scale(cr, axis_x/axis_y, axis_y/axis_x);
            cairo_arc(cr, 0.0, 0.0, radius, 0, 2*G_PI);
            cairo_restore(cr);
            cairo_stroke(cr);
          }
      }
    else
      {
        cairo_move_to(cr, rect[0], rect[1]);
        cairo_line_to(cr, rect[2], rect[3]);
        cairo_stroke(cr);
      }

    return TRUE;
  }
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_unblock(widget, motion_id);
    GdkWindow *win=gtk_widget_get_window(widget);
    cairo_t *cr=gdk_cairo_create(win);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_paint(cr);
    cairo_destroy(cr);

    rect[0]=event->button.x;
    rect[1]=event->button.y;

    return TRUE;
  }
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_block(widget, motion_id);
    return TRUE;
  }
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    rect[2]=event->button.x;
    rect[3]=event->button.y;
    gtk_widget_queue_draw(widget);
    return TRUE;
  }
static void add_rectangle(GtkWidget *widget, gpointer *data)
  {
    if(shape_combo1==0)
      {
        g_array_append_val((GArray*)data[0], rect[0]);
        g_array_append_val((GArray*)data[0], rect[1]);
        g_array_append_val((GArray*)data[0], rect[2]);
        g_array_append_val((GArray*)data[0], rect[3]);
        g_array_append_val((GArray*)data[0], color_combo2);
      }
    else if(shape_combo1==1)
      {
        g_array_append_val((GArray*)data[1], rect[0]);
        g_array_append_val((GArray*)data[1], rect[1]);
        g_array_append_val((GArray*)data[1], rect[2]);
        g_array_append_val((GArray*)data[1], rect[3]);
        g_array_append_val((GArray*)data[1], color_combo2);
      }
    else
      {
        g_array_append_val((GArray*)data[2], rect[0]);
        g_array_append_val((GArray*)data[2], rect[1]);
        g_array_append_val((GArray*)data[2], rect[2]);
        g_array_append_val((GArray*)data[2], rect[3]);
        g_array_append_val((GArray*)data[2], color_combo2);
      }
    //g_print("%f %f %f %f %f\n", rect[0], rect[1], rect[2], rect[3], color_combo2);
  }
static void clear_rectangles(GtkWidget *widget, gpointer *data)
  {
    rect[0]=0.0;
    rect[1]=0.0;
    rect[2]=0.0;
    rect[3]=0.0;
    rect[4]=0.0;

    gint len1=((GArray*)data[0])->len; 
    gint len2=((GArray*)data[1])->len;
    gint len3=((GArray*)data[2])->len;  

    if(len1>0) g_array_remove_range((GArray*)data[0], 0, len1);
    if(len2>0) g_array_remove_range((GArray*)data[1], 0, len2);
    if(len3>0) g_array_remove_range((GArray*)data[2], 0, len3);

    gtk_widget_queue_draw(GTK_WIDGET(data[3]));
  }
static void combo1_changed(GtkComboBox *combo1, gpointer data)
  {
    shape_combo1=(gdouble)gtk_combo_box_get_active(combo1);
  }
static void combo2_changed(GtkComboBox *combo2, gpointer data)
  {
    color_combo2=(gdouble)gtk_combo_box_get_active(combo2);
  }












