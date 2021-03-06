
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
//Change color of background, line drawing and selection rectangle.
static gdouble bgc[4]={0.0, 0.0, 1.0, 1.0};
static gdouble fgc[4]={0.0, 1.0, 1.0, 1.0};
static gdouble selection[4]={1.0, 0.0, 1.0, 1.0};

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
  //Used to identify the top drawing area. Don't want to draw an active rectangle around it.
  gtk_widget_set_name(da, "da");
  //Draw one of the critters below.
  g_signal_connect(da, "draw", G_CALLBACK(draw_critter), NULL);

  GtkWidget *label=gtk_label_new("You can't handle turtle speed!");

  //The lower panel of drawing area critters.
  GtkWidget *da1=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da1, TRUE);
  gtk_widget_set_vexpand(da1, TRUE);
  g_signal_connect(da1, "draw", G_CALLBACK(draw_turtle), NULL);
  gtk_widget_set_events(da1, GDK_BUTTON_PRESS_MASK);

  GtkWidget *da2=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da2, TRUE);
  gtk_widget_set_vexpand(da2, TRUE);
  g_signal_connect(da2, "draw", G_CALLBACK(draw_lizard), NULL);
  gtk_widget_set_events(da2, GDK_BUTTON_PRESS_MASK);
  
  GtkWidget *da3=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da3, TRUE);
  gtk_widget_set_vexpand(da3, TRUE);
  g_signal_connect(da3, "draw", G_CALLBACK(draw_rabbit), NULL);
  gtk_widget_set_events(da3, GDK_BUTTON_PRESS_MASK);

  GtkWidget *da4=gtk_drawing_area_new();
  gtk_widget_set_hexpand(da4, TRUE);
  gtk_widget_set_vexpand(da4, TRUE);
  g_signal_connect(da4, "draw", G_CALLBACK(draw_cheetah), NULL);
  gtk_widget_set_events(da4, GDK_BUTTON_PRESS_MASK);

  gpointer widgets[]={da, label, da1, da2, da3, da4};
  g_signal_connect(da1, "button_press_event", G_CALLBACK(click_drawing1), widgets);
  g_signal_connect(da2, "button_press_event", G_CALLBACK(click_drawing2), widgets);
  g_signal_connect(da3, "button_press_event", G_CALLBACK(click_drawing3), widgets);
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

  //Draw the top active critter.
  gtk_label_set_text(GTK_LABEL(data[1]), "You can't handle the turtle speed!");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(data[2]));
  gtk_widget_queue_draw(GTK_WIDGET(data[3]));
  gtk_widget_queue_draw(GTK_WIDGET(data[4]));
  gtk_widget_queue_draw(GTK_WIDGET(data[5]));

  return FALSE;
}
static gboolean click_drawing2(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=2;

  //Draw the top active critter.
  gtk_label_set_text(GTK_LABEL(data[1]), "This is a water dragon lizard, not an alien, OK.");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(data[2]));
  gtk_widget_queue_draw(GTK_WIDGET(data[3]));
  gtk_widget_queue_draw(GTK_WIDGET(data[4]));
  gtk_widget_queue_draw(GTK_WIDGET(data[5]));

  return FALSE;
}
static gboolean click_drawing3(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=3;

  //Draw the top active critter.
  gtk_label_set_text(GTK_LABEL(data[1]), "This is no turtle!");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(data[2]));
  gtk_widget_queue_draw(GTK_WIDGET(data[3]));
  gtk_widget_queue_draw(GTK_WIDGET(data[4]));
  gtk_widget_queue_draw(GTK_WIDGET(data[5]));

  return FALSE;
}
static gboolean click_drawing4(GtkWidget *widget, GdkEvent *event, gpointer *data)
{
  drawing=4;

  //Draw the top active critter.
  gtk_label_set_text(GTK_LABEL(data[1]), "Cheetah speed. Hang on!");
  gtk_widget_queue_draw(GTK_WIDGET(data[0]));

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(data[2]));
  gtk_widget_queue_draw(GTK_WIDGET(data[3]));
  gtk_widget_queue_draw(GTK_WIDGET(data[4]));
  gtk_widget_queue_draw(GTK_WIDGET(data[5]));

  return FALSE;
}
static gboolean draw_critter(GtkWidget *da, cairo_t *cr, gpointer data)
{
  //Draw critter in top window.
  if(drawing==1) draw_turtle(da, cr, NULL);
  else if(drawing==2) draw_lizard(da, cr, NULL);
  else if(drawing==3) draw_rabbit(da, cr, NULL);
  else draw_cheetah(da, cr, NULL);

  return FALSE;
}
static gboolean draw_turtle(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, bgc[0], bgc[1], bgc[2], bgc[3]); 
  cairo_paint(cr);

  if(drawing==1&&!g_strcmp0(gtk_widget_get_name(da), "da")==0)
    {
      cairo_set_source_rgba(cr, selection[0], selection[1], selection[2], selection[3]);
    }
  else cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]); 
  cairo_set_line_width(cr, 7.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);
  cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]); 

  //Move the turtle a little.
  cairo_translate(cr, 0.7*width/16.0, 2.0*height/16.0);
 
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
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, bgc[0], bgc[1], bgc[2], bgc[3]); 
  cairo_paint(cr);

  if(drawing==2&&!g_strcmp0(gtk_widget_get_name(da), "da")==0)
    {
      cairo_set_source_rgba(cr, selection[0], selection[1], selection[2], selection[3]);
    }
  else cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]);
  cairo_set_line_width(cr, 7.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);
  cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]);

  //Move the lizard a little.
  cairo_translate(cr, 0.6*width/16.0, 1.0*height/16.0); 
 
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
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, bgc[0], bgc[1], bgc[2], bgc[3]); 
  cairo_paint(cr);

  if(drawing==3&&!g_strcmp0(gtk_widget_get_name(da), "da")==0)
    {
      cairo_set_source_rgba(cr, selection[0], selection[1], selection[2], selection[3]);
    }
  else cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]); 
  cairo_set_line_width(cr, 7.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);
  cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]); 
 
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
  cairo_curve_to(cr, 9.0*width/16.0, 10.0*height/16.0, 9.0*width/16.0, 10.0*height/16.0, 6.0*width/16.0, 11.0*height/16.0);
  cairo_stroke(cr);

  //The eye. Scale based on 400x400 drawing.
  cairo_arc(cr, 13.0*width/16.0, 7.0*height/16.0, 5.0*(gdouble)height/400.0, 0.0, 2*G_PI);
  cairo_fill(cr);

  return FALSE;
}
static gboolean draw_cheetah(GtkWidget *da, cairo_t *cr, gpointer data)
{
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

  cairo_set_source_rgba(cr, bgc[0], bgc[1], bgc[2], bgc[3]); 
  cairo_paint(cr);

  if(drawing==4&&!g_strcmp0(gtk_widget_get_name(da), "da")==0)
    {
      cairo_set_source_rgba(cr, selection[0], selection[1], selection[2], selection[3]);
    }
  else cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]); 
  cairo_set_line_width(cr, 7.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);
  cairo_set_source_rgba(cr, fgc[0], fgc[1], fgc[2], fgc[3]);

  //Move the cheetah a little.
  cairo_translate(cr, 0.5*width/16.0, 0.0); 
 
  //scale line width by height. Original drawing 400x400.
  cairo_set_line_width(cr, 6.0*(gdouble)height/400.0);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  //Head 
  cairo_move_to(cr, 14.5*width/16.0, 7.5*height/16.0);
  cairo_curve_to(cr, 14.75*width/16.0, 7.5*height/16.0, 14.5*width/16.0, 7.0*height/16.0, 14.25*width/16.0, 6.5*height/16.0);
  cairo_stroke_preserve(cr); 
  cairo_curve_to(cr, 14.0*width/16.0, 6.25*height/16.0, 13.5*width/16.0, 6.0*height/16.0, 12.5*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr); 
  //Ear
  cairo_curve_to(cr, 12.25*width/16.0, 5.5*height/16.0, 12.25*width/16.0, 5.5*height/16.0, 12.0*width/16.0, 6.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back hump
  cairo_curve_to(cr, 11.5*width/16.0, 7.5*height/16.0, 10.5*width/16.0, 6.0*height/16.0, 10.0*width/16.0, 7.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back
  cairo_curve_to(cr, 9.0*width/16.0, 8.5*height/16.0, 7.5*width/16.0, 7.5*height/16.0, 6.0*width/16.0, 8.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Tail
  cairo_curve_to(cr, -1.0*width/16.0, 8.0*height/16.0, -1.0*width/16.0, 9.0*height/16.0, 5.0*width/16.0, 9.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back leg
  cairo_curve_to(cr, 3.25*width/16.0, 9.5*height/16.0, 4.0*width/16.0, 11.0*height/16.0, 3.0*width/16.0, 12.0*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_curve_to(cr, 2*width/16.0, 13.0*height/16.0, 2.0*width/16.0, 13.0*height/16.0, 1.5*width/16.0, 13.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Back foot
  cairo_curve_to(cr, 1.50*width/16.0, 14.0*height/16.0, 1.50*width/16.0, 14.0*height/16.0, 2.0*width/16.0, 14.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back leg to stomach
   cairo_curve_to(cr, 4.75*width/16.0, 10.0*height/16.0, 5.0*width/16.0, 13.0*height/16.0, 7.0*width/16.0, 10.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Belly
  cairo_curve_to(cr, 9.0*width/16.0, 10.5*height/16.0, 9.0*width/16.0, 10.5*height/16.0, 11.0*width/16.0, 9.5*height/16.0);
  cairo_stroke_preserve(cr);
  //Front leg
  cairo_curve_to(cr, 14.0*width/16.0, 11.0*height/16.0, 14.0*width/16.0, 10.0*height/16.0, 11.5*width/16.0, 9.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Bottom neck
  cairo_curve_to(cr, 12.0*width/16.0, 8.5*height/16.0, 12.0*width/16.0, 8.5*height/16.0, 12.5*width/16.0, 8.25*height/16.0);
  cairo_stroke_preserve(cr);
  cairo_curve_to(cr, 13.0*width/16.0, 8.0*height/16.0, 13.0*width/16.0, 8.0*height/16.0, 14.15*width/16.0, 8.0*height/16.0);
  cairo_stroke_preserve(cr);
  //Back to the start
  cairo_curve_to(cr, 14.5*width/16.0, 7.5*height/16.0, 14.5*width/16.0, 7.5*height/16.0, 14.5*width/16.0, 7.5*height/16.0);
  cairo_stroke(cr);
  
  //The eye. Scale based on 400x400 drawing.
  cairo_arc(cr, 13.75*width/16.0, 6.75*height/16.0, 5.0*(gdouble)height/400.0, 0.0, 2*G_PI);
  cairo_fill(cr);
  
  return FALSE;
}

