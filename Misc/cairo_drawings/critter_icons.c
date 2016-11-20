
/*
    Try to draw some critter icons with cairo. Needs work. If I can get some decent drawings then
I can build a switch widget with them.

    gcc -Wall critter_icons.c -o critter_icons `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean click_drawing1(GtkWidget *widget, GdkEvent *event, gpointer *data);
static gboolean click_drawing2(GtkWidget *widget, GdkEvent *event, gpointer *data);
static gboolean click_drawing3(GtkWidget *widget, GdkEvent *event, gpointer *data);
static gboolean click_drawing4(GtkWidget *widget, GdkEvent *event, gpointer *data);
static gboolean draw_critter(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_lizard(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_turtle(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_rabbit(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_cheetah(GtkWidget *da, cairo_t *cr, gpointer data);

//For drawing 1,2,3 or 4.
static gint drawing=1;

int main(int argc, char *argv[])
{
  gtk_init(&argc, &argv);

  GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title(GTK_WINDOW(window), "Critter Icons");
  gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
  gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);

  g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

  GtkWidget *da=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da, TRUE);
  gtk_widget_set_vexpand(da, TRUE);
  g_signal_connect(da, "draw", G_CALLBACK(draw_critter), NULL);

  GtkWidget *label=gtk_label_new("You can't handle turtle speed!");
  gpointer widgets[]={da, label};

  //The lower panel of drawing areas.
  GtkWidget *da1=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da1, TRUE);
  gtk_widget_set_vexpand(da1, TRUE);
  g_signal_connect(da1, "draw", G_CALLBACK(draw_turtle), NULL);
  gtk_widget_set_events(da1, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(da1, "button_press_event", G_CALLBACK(click_drawing1), widgets);

  GtkWidget *da2=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da2, TRUE);
  gtk_widget_set_vexpand(da2, TRUE);
  g_signal_connect(da2, "draw", G_CALLBACK(draw_lizard), NULL);
  gtk_widget_set_events(da2, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(da2, "button_press_event", G_CALLBACK(click_drawing2), widgets);
  
  GtkWidget *da3=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da3, TRUE);
  gtk_widget_set_vexpand(da3, TRUE);
  g_signal_connect(da3, "draw", G_CALLBACK(draw_rabbit), NULL);
  gtk_widget_set_events(da3, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(da3, "button_press_event", G_CALLBACK(click_drawing3), widgets);

  GtkWidget *da4=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da4, TRUE);
  gtk_widget_set_vexpand(da4, TRUE);
  g_signal_connect(da4, "draw", G_CALLBACK(draw_cheetah), NULL);
  gtk_widget_set_events(da4, GDK_BUTTON_PRESS_MASK);
  g_signal_connect(da4, "button_press_event", G_CALLBACK(click_drawing4), widgets);
  
  GtkWidget *grid=gtk_grid_new();
  gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 4, 4);
  gtk_grid_attach(GTK_GRID(grid), da1, 0, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), da2, 1, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), da3, 2, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), da4, 3, 5, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), label, 0, 6, 4, 1);
  
  gtk_container_add(GTK_CONTAINER(window), grid);
  gtk_widget_show_all(window);

  gtk_main();

  return 0;
}
static gboolean click_drawing1(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=1;
  gtk_label_set_text(GTK_LABEL(data[1]), "You can't handle the turtle speed!");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  return FALSE;
}
static gboolean click_drawing2(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=2;
  gtk_label_set_text(GTK_LABEL(data[1]), "This is a water dragon lizard, OK.");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  return FALSE;
}
static gboolean click_drawing3(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=3;
  gtk_label_set_text(GTK_LABEL(data[1]), "Getting a little faster at rabbit speed.");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  return FALSE;
}
static gboolean click_drawing4(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=4;
  gtk_label_set_text(GTK_LABEL(data[1]), "Maybe a cheetah in the works.");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  return FALSE;
}
static gboolean draw_critter(GtkWidget *da, cairo_t *cr, gpointer data)
{
  if(drawing==1) draw_turtle(da, cr, data);
  else if(drawing==2) draw_lizard(da, cr, data);
  else if(drawing==3) draw_rabbit(da, cr, data);
  else draw_cheetah(da, cr, data);
  return FALSE;
}
static gboolean draw_turtle(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0); 
  cairo_paint(cr);

  //Outside rectangle
  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0); 
  cairo_set_line_width(cr, 7.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  //Scale drawing line by 400x400 drawing.
  cairo_set_line_width(cr, 6.0*(gdouble)height/400.0);
  //Top Shell
  cairo_move_to(cr, 12.0*width/16.0, 9.0*height/16.0);
  cairo_curve_to(cr, 6.5*width/16.0, 1.0*height/16.0, 6.5*width/16.0, 1.0*height/16.0, 1.0*width/16.0, 9.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Top tail
  cairo_line_to(cr, 0.0, 9.5*height/16.0);
  cairo_stroke_preserve(cr);
  //End of tail to back foot. 
  cairo_curve_to(cr, 3.0*width/16.0, 9.5*height/16.0, 3.0*width/16.0, 9.5*height/16.0, 3.0*width/16.0, 11.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Bottom of back foot.
  cairo_line_to(cr, 4.0*width/16.0, 11.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Bottom of shell
  cairo_curve_to(cr, 2.0*width/16.0, 9.5*height/16.0, 10.5*width/16.0, 9.5*height/16.0, 10.0*width/16.0, 11.0*height/16.0);
  cairo_stroke_preserve(cr);
  //The bottom of front foot
  cairo_line_to(cr, 11.0*width/16.0, 11.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Front foot to bottom of head
  cairo_curve_to(cr, 10.0*width/16.0, 9.5*height/16.0, 10.5*width/16.0, 9.5*height/16.0, 12.0*width/16.0, 10.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Turtle head back to start.
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_curve_to(cr, 16.0*width/16.0, 10.5*height/16.0, 16.0*width/16.0, 6.0*height/16.0, 12.0*width/16.0, 9.0*height/16.0);
  cairo_stroke(cr);
  
  //The eye. Scale radius based on 400x400 drawing.
  cairo_arc(cr, 14.0*width/16.0, 8.5*height/16.0, 5.0*(gdouble)height/400.0, 0.0, 2*G_PI);
  cairo_fill(cr);
 

  return FALSE;
}
static gboolean draw_lizard(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0); 
  cairo_paint(cr);

  //Outside rectangle
  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0); 
  cairo_set_line_width(cr, 7.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  //scale line width by height. Original drawing 400x400.
  cairo_set_line_width(cr, 6.0*(gdouble)height/400.0);
  //Head fin
  cairo_move_to(cr, 15.0*width/16.0, 2.0*height/16.0);
  cairo_curve_to(cr, width/2.0, 1.0*height/16.0, width/2.0, 3.0*height/16.0, 12.0*width/16.0, 4.0*height/16.0);
  cairo_stroke_preserve(cr); 
  //Back fin
  cairo_curve_to(cr, 5.0*width/16.0, 4.0*height/16.0, 6.0*width/16.0, 5.0*height/16.0, 8.0*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Tail fin 
  cairo_curve_to(cr, width/4.0, 5.0*height/16.0, width/4.0, 7.0*height/16.0, 3.0*width/16.0, 8.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Tail top 
  cairo_curve_to(cr, width/8.0, 9.0*height/16.0, width/8.0, 9.0*height/16.0, 0.0, 10.0*height/16.0);
  cairo_stroke_preserve(cr); 
  //Tail bottom
  cairo_curve_to(cr, width/4.0, 8.0*height/16.0, width/4.0, 8.0*height/16.0, 8.0*width/16.0, 7.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back leg top
  cairo_curve_to(cr, width/8.0, 12.0*height/16.0, width/8.0, 12.0*height/16.0, 2.0*width/16.0, 12.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Toes
  cairo_line_to(cr, 3.5*width/16.0, 11.0*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 2.0*width/16.0, 12.5*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 3.5*width/16.0, 11.5*height/16.0);
  cairo_stroke_preserve(cr); 
  cairo_line_to(cr, 2.0*width/16.0, 13.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Foot to body
  cairo_curve_to(cr, width/4.0, 11.0*height/16.0, width/4.0, 11.0*height/16.0, 9.0*width/16.0, 7.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Belly
  cairo_curve_to(cr, 11.0*width/16.0, 7.0*height/16.0, 11.0*width/16.0, 7.0*height/16.0, 12.0*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Hand
  cairo_line_to(cr, 10.0*width/16.0, 9.0*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 11.0*width/16.0, 8.0*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 10.0*width/16.0, 9.5*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 12.5*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back to the start
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
  cairo_curve_to(cr, 14.0*width/16.0, 4.0*height/16.0, 14.0*width/16.0, 4.0*height/16.0, 15.0*width/16.0, 2.0*height/16.0);
  cairo_stroke(cr);

  //The eye. Scale based on 400x400 drawing.
  cairo_arc(cr, 13.0*width/16.0, 2.5*height/16.0, 5.0*(gdouble)height/400.0, 0.0, 2*G_PI);
  cairo_fill(cr);

  return FALSE;
}
static gboolean draw_rabbit(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0); 
  cairo_paint(cr);

  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0); 
  cairo_set_line_width(cr, 7.0);
  //Outside rectangle
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  //scale line width by height. Original drawing 400x400.
  cairo_set_line_width(cr, 6.0*(gdouble)height/400.0);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  //Head
  cairo_move_to(cr, 15.0*width/16.0, 8.0*height/16.0);
  cairo_curve_to(cr, 15.0*width/16.0, 7.0*height/16.0, 14.0*width/16.0, 6.0*height/16.0, 12.0*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Ear
  cairo_curve_to(cr, 10.5*width/16.0, 3.5*height/16.0, 10.0*width/16.0, 4.0*height/16.0, 9.0*width/16.0, 3.0*height/16.0);
  cairo_stroke_preserve(cr); 
  cairo_curve_to(cr, 9.0*width/16.0, 3.5*height/16.0, 9.0*width/16.0, 3.5*height/16.0, 9.5*width/16.0, 4.0*height/16.0);
  cairo_stroke_preserve(cr); 
  cairo_curve_to(cr, 9.0*width/16.0, 3.5*height/16.0, 9.0*width/16.0, 3.5*height/16.0, 8.0*width/16.0, 3.2*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_curve_to(cr, 9.0*width/16.0, 4.0*height/16.0, 9.5*width/16.0, 6.0*height/16.0, 10.5*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back 
  cairo_curve_to(cr, 6.0*width/16.0, 6.0*height/16.0, 6.0*width/16.0, 6.0*height/16.0, 3.0*width/16.0, 10.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Tail
  cairo_curve_to(cr, 2.0*width/16.0, 9.0*height/16.0, 1.5*width/16.0, 10.0*height/16.0, 3.0*width/16.0, 11.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Back leg
  cairo_curve_to(cr, 2.0*width/16.0, 12.0*height/16.0, 1.0*width/16.0, 14.0*height/16.0, 1.5*width/16.0, 14.5*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_curve_to(cr, 2.0*width/16.0, 15.0*height/16.0, 3.5*width/16.0, 12.0*height/16.0, 3.5*width/16.0, 12.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Thigh
  cairo_curve_to(cr, 4.0*width/16.0, 13.0*height/16.0, 6.0*width/16.0, 14.0*height/16.0, 6.0*width/16.0, 9.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Move to the start of drawing
  cairo_move_to(cr, 15.0*width/16.0, 8.0*height/16.0);
  cairo_curve_to(cr, 14.0*width/16.0, 10.0*height/16.0, 12.0*width/16.0, 9.0*height/16.0, 12.0*width/16.0, 9.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Line to front feet
  cairo_line_to(cr, 11.0*width/16.0, 9.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Front feet
  cairo_curve_to(cr, 13.0*width/16.0, 11.0*height/16.0, 13.0*width/16.0, 11.0*height/16.0, 10.0*width/16.0, 9.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Belly
  cairo_curve_to(cr, 7.0*width/16.0, 9.0*height/16.0, 7.0*width/16.0, 10.5*height/16.0, 6.0*width/16.0, 11.0*height/16.0);
  cairo_stroke(cr);

  //The eye. Scale based on 400x400 drawing.
  cairo_arc(cr, 13.0*width/16.0, 7.0*height/16.0, 5.0*(gdouble)height/400.0, 0.0, 2*G_PI);
  cairo_fill(cr);

  return FALSE;
}
static gboolean draw_cheetah(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0); 
  cairo_paint(cr);

  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0); 
  cairo_set_line_width(cr, 7.0);
  //Outside rectangle
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);
  return FALSE;
}

