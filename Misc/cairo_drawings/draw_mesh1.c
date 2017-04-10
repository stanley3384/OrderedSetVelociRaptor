
/*
    Draw a moving mesh.

    gcc -Wall draw_mesh1.c -o draw_mesh1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean draw_mesh(GtkWidget *widget, cairo_t *cr, gpointer data);
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data);
static void combo_changed(GtkComboBox *combo, gpointer data);

//For blocking motion signal. Block when not drawing top rectangle
static gint motion_id=0;
//Coordinates for the drawing rectangle. Fifth number is color of rectangle.
static gdouble rect[]={0.0, 0.0, 0.0, 0.0, 0.0};
//Starting control points for drawing a mesh.
static gdouble mesh[]={1.0, 3.0, 3.0, 4.0, 3.0, 3.0, 4.0, 1.0};
static gdouble mesh_combo=0.0;

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    GtkWidget *window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Mesh");
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    //Add some extra events to the top drawing area.
    gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK);
    g_signal_connect(da, "draw", G_CALLBACK(draw_mesh), NULL);
    g_signal_connect(da, "button-press-event", G_CALLBACK(start_press), NULL);
    g_signal_connect(da, "button-release-event", G_CALLBACK(stop_press), NULL);
    motion_id=g_signal_connect(da, "motion-notify-event", G_CALLBACK(cursor_motion), NULL);
    g_signal_handler_block(da, motion_id);

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "Drag Point1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "Drag Point2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "Drag Point3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 3, "4", "Drag Point4");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), NULL);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    rect[2]=0.1*width;
    rect[3]=0.3*height;
    gtk_widget_queue_draw(da);

    gtk_main();

    return 0;
  }
static gboolean draw_mesh(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    cairo_set_line_width(cr, 6);
    
    //Layout axis for drawing mesh.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, 2.0*w1, 2.0*h1, 6.0*w1, 6.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 2.0*w1, 5.0*h1);
    cairo_line_to(cr, 8.0*w1, 5.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*w1, 2.0*h1);
    cairo_line_to(cr, 5.0*w1, 8.0*h1);
    cairo_stroke(cr);

    gint i=0;
    gint j=0;
    //Save the mesh points.
    //g_print("x %f y %f x %f y %f\n", (rect[2]/width)*10.0, (rect[3]/height)*10.0, rect[2], rect[3]);
    if(mesh_combo==0)
      {
        mesh[0]=(rect[2]/width)*10.0;
        mesh[1]=(rect[3]/height)*10.0;
      }
    else if(mesh_combo==1)
      {
        mesh[2]=(rect[2]/width)*10.0;
        mesh[3]=(rect[3]/height)*10.0;
      }
    else if(mesh_combo==2)
      {
        mesh[4]=(rect[2]/width)*10.0;
        mesh[5]=(rect[3]/height)*10.0;
      }
    else
      {
        mesh[6]=(rect[2]/width)*10.0;
        mesh[7]=(rect[3]/height)*10.0;
      }
    cairo_set_line_width(cr, 4);
 
    //Patch1
    cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
    cairo_mesh_pattern_begin_patch(pattern1);
    cairo_mesh_pattern_move_to(pattern1, 2.0*w1, 2.0*h1);
    cairo_mesh_pattern_curve_to(pattern1, mesh[4]*w1, mesh[5]*h1, mesh[6]*w1, mesh[7]*h1, 5.0*w1, 2.0*h1);
    cairo_mesh_pattern_curve_to(pattern1, mesh[0]*w1+3.0*w1, mesh[1]*h1, mesh[2]*w1+3.0*w1, mesh[3]*h1, 5.0*w1, 5.0*h1);
    cairo_mesh_pattern_curve_to(pattern1, mesh[6]*w1, mesh[7]*h1+3.0*h1, mesh[4]*w1, mesh[5]*h1+3.0*h1, 2.0*w1, 5.0*h1);
    cairo_mesh_pattern_curve_to(pattern1, mesh[2]*w1, mesh[3]*h1, mesh[0]*w1, mesh[1]*h1, 2.0*w1, 2.0*h1);   
    cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 0.0, 0.0);
    cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 1.0, 0.0, 1.0);
    cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 1.0, 0.0);
    cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 0.0, 0.0, 1.0);
    cairo_mesh_pattern_end_patch(pattern1);
    cairo_set_source(cr, pattern1);
    cairo_paint(cr);
    cairo_pattern_destroy(pattern1);

    //6 vertical Bezier curves over layout axis.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_save(cr);
    for(i=0;i<2;i++)
      {
        for(j=0;j<3;j++)
          {
            cairo_move_to(cr, 2.0*w1, 2.0*h1);
            //cairo_curve_to(cr, 1.0*w1, 3.0*h1, 3.0*w1, 4.0*h1, 2.0*w1, 5.0*h1);
            cairo_curve_to(cr, mesh[0]*w1, mesh[1]*h1, mesh[2]*w1, mesh[3]*h1, 2.0*w1, 5.0*h1);
            cairo_stroke(cr);
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_translate(cr, 3.0*w1, 0.0);
          }
        cairo_translate(cr, -9.0*w1, 3.0*h1);
      }
    cairo_restore(cr);

    //6 horizontal Bezier curves over layout axis.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_save(cr);
    for(i=0;i<2;i++)
      {
        for(j=0;j<3;j++)
          {
            cairo_move_to(cr, 2.0*w1, 2.0*h1);
            //cairo_curve_to(cr, 3.0*w1, 3.0*h1, 4.0*w1, 1.0*h1, 5.0*w1, 2.0*h1);
            cairo_curve_to(cr, mesh[4]*w1, mesh[5]*h1, mesh[6]*w1, mesh[7]*h1, 5.0*w1, 2.0*h1);
            cairo_stroke(cr);
            cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
            cairo_translate(cr, 0.0, 3.0*h1);
          }
       cairo_translate(cr, 3.0*w1, -9.0*h1);
      }
    cairo_restore(cr);

    cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 12);    
    cairo_set_source_rgb(cr, 0.0 , 0.0 , 0.0);
    cairo_move_to(cr, mesh[0]*width/10.0, mesh[1]*height/10.0);
    cairo_show_text(cr, "1");
    cairo_stroke(cr);
    cairo_move_to(cr, mesh[2]*width/10.0, mesh[3]*height/10.0);
    cairo_show_text(cr, "2");
    cairo_stroke(cr);
    cairo_move_to(cr, mesh[4]*width/10.0, mesh[5]*height/10.0);
    cairo_show_text(cr, "3");
    cairo_stroke(cr);
    cairo_move_to(cr, mesh[6]*width/10.0, mesh[7]*height/10.0);
    cairo_show_text(cr, "4");
    cairo_stroke(cr);

    return TRUE;
  }
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_unblock(widget, motion_id);

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
static void combo_changed(GtkComboBox *combo, gpointer data)
  {
    mesh_combo=(gdouble)gtk_combo_box_get_active(combo);
  }











