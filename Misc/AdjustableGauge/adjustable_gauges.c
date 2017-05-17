
/*

    Another test program for the adjustable gauge widget. Add the critter_icons.c code for
the critter drawings and to test some speeds. Use a GdkFrameClock for smooth animation. Drawing
a gradient takes longer than a solid color so the the gradient might not work with the frame
clock.

    gcc -Wall -Werror adjustable_gauge.c adjustable_gauges.c -o gauges `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>
#include "adjustable_gauge.h"

//The frame clock id's.
static guint tick_id1=0;
static guint tick_id2=0;
static guint tick_id3=0;
static guint tick_id4=0;
//Save the needle position between calls.
static gdouble needle1=0.0;
static gdouble needle2=0.0;
static gdouble needle3=20.0;
static gdouble needle4=0.0;
static gdouble needle_speed=0.01;
//The top of each scale.
static const gdouble top1=100.0;
static const gdouble top2=200.0;
static const gdouble top3=200.0;
static const gdouble top4=500.0;

//For critter icon drawing 1,2,3 or 4.
static gint drawing=1;
//Change color of background, line drawing and selection rectangle.
static gdouble bgc[4]={0.0, 0.0, 0.0, 1.0};
static gdouble fgc[4]={0.0, 1.0, 1.0, 1.0};
static gdouble selection[4]={1.0, 1.0, 1.0, 1.0};


static gboolean tick_draw(GtkWidget *widget, GdkFrameClock *frame_clock, GtkWidget *gauges[]);
static gboolean click_drawing1(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[]);
static gboolean click_drawing2(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[]);
static gboolean click_drawing3(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[]);
static gboolean click_drawing4(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[]);
static gboolean draw_lizard(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_turtle(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_rabbit(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_cheetah(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_window(GtkWidget *da, cairo_t *cr, gpointer data);
static void quit_program(GtkWidget *widget, GtkWidget *gauges[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 600, 600);
    gtk_window_set_title(GTK_WINDOW(window), "Gauges");
    gtk_widget_set_app_paintable(window, TRUE);
    g_signal_connect(window, "draw", G_CALLBACK(draw_window), NULL);
       
    GtkWidget *gauge1=adjustable_gauge_new();
    adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(gauge1), SPEEDOMETER_GAUGE);
    adjustable_gauge_set_background(ADJUSTABLE_GAUGE(gauge1), "rgba(255, 255, 255, 1.0)");
    adjustable_gauge_set_text_color(ADJUSTABLE_GAUGE(gauge1), "rgba(0, 0, 0, 1.0)");
    adjustable_gauge_set_needle_color(ADJUSTABLE_GAUGE(gauge1), "rgba(0, 0, 0, 1.0)");
    gtk_widget_set_name(gauge1, "g1");
    gtk_widget_set_hexpand(gauge1, TRUE);
    gtk_widget_set_vexpand(gauge1, TRUE); 

    GtkWidget *gauge2=adjustable_gauge_new();
    adjustable_gauge_set_drawing(ADJUSTABLE_GAUGE(gauge2), SPEEDOMETER_GAUGE);
    adjustable_gauge_set_background(ADJUSTABLE_GAUGE(gauge2), "rgba(255, 255, 255, 1.0)");
    adjustable_gauge_set_text_color(ADJUSTABLE_GAUGE(gauge2), "rgba(0, 0, 0, 1.0)");
    adjustable_gauge_set_needle_color(ADJUSTABLE_GAUGE(gauge2), "rgba(0, 0, 0, 1.0)");
    gtk_widget_set_hexpand(gauge2, TRUE);
    gtk_widget_set_vexpand(gauge2, TRUE);
    //Set scale values before cutoff values.
    adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(gauge2), 0.0);
    adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(gauge2), top2);
    adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(gauge2), 150.0);
    adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(gauge2), 180.0); 
  
    GtkWidget *gauge3=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge3, TRUE);
    gtk_widget_set_vexpand(gauge3, TRUE);
    adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(gauge3), 20.0);
    adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(gauge3), top3);
    adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(gauge3), 120.0);
    adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(gauge3), 170.0); 
   
    GtkWidget *gauge4=adjustable_gauge_new();
    gtk_widget_set_hexpand(gauge4, TRUE);
    gtk_widget_set_vexpand(gauge4, TRUE);
    adjustable_gauge_set_scale_bottom(ADJUSTABLE_GAUGE(gauge4), 0.0);
    adjustable_gauge_set_scale_top(ADJUSTABLE_GAUGE(gauge4), top4);
    adjustable_gauge_set_first_cutoff(ADJUSTABLE_GAUGE(gauge4), 450.0);
    adjustable_gauge_set_second_cutoff(ADJUSTABLE_GAUGE(gauge4), 480.0);

    //The lower panel of drawing area critters.
    GtkWidget *label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label), "<span foreground='white'>You can't handle turtle speed!</span>");
    
    GtkWidget *turtle=gtk_drawing_area_new();
    gtk_widget_set_hexpand(turtle, TRUE);
    gtk_widget_set_vexpand(turtle, TRUE);
    g_signal_connect(turtle, "draw", G_CALLBACK(draw_turtle), NULL);
    gtk_widget_set_events(turtle, GDK_BUTTON_PRESS_MASK);

    GtkWidget *lizard=gtk_drawing_area_new();
    gtk_widget_set_hexpand(lizard, TRUE);
    gtk_widget_set_vexpand(lizard, TRUE);
    g_signal_connect(lizard, "draw", G_CALLBACK(draw_lizard), NULL);
    gtk_widget_set_events(lizard, GDK_BUTTON_PRESS_MASK);
  
    GtkWidget *rabbit=gtk_drawing_area_new();
    gtk_widget_set_hexpand(rabbit, TRUE);
    gtk_widget_set_vexpand(rabbit, TRUE);
    g_signal_connect(rabbit, "draw", G_CALLBACK(draw_rabbit), NULL);
    gtk_widget_set_events(rabbit, GDK_BUTTON_PRESS_MASK);

    GtkWidget *cheetah=gtk_drawing_area_new();
    gtk_widget_set_hexpand(cheetah, TRUE);
    gtk_widget_set_vexpand(cheetah, TRUE);
    g_signal_connect(cheetah, "draw", G_CALLBACK(draw_cheetah), NULL);
    gtk_widget_set_events(cheetah, GDK_BUTTON_PRESS_MASK);

    GtkWidget *critters[]={label, turtle, lizard, rabbit, cheetah};
    g_signal_connect(turtle, "button_press_event", G_CALLBACK(click_drawing1), critters);
    g_signal_connect(lizard, "button_press_event", G_CALLBACK(click_drawing2), critters);
    g_signal_connect(rabbit, "button_press_event", G_CALLBACK(click_drawing3), critters);
    g_signal_connect(cheetah, "button_press_event", G_CALLBACK(click_drawing4), critters); 
    
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), gauge1, 0, 0, 2, 2);
    gtk_grid_attach(GTK_GRID(grid), gauge2, 2, 0, 2, 2);
    gtk_grid_attach(GTK_GRID(grid), gauge3, 0, 2, 2, 2);
    gtk_grid_attach(GTK_GRID(grid), gauge4, 2, 2, 2, 2);

    gtk_grid_attach(GTK_GRID(grid), turtle, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), lizard, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), rabbit, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), cheetah, 3, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 5, 4, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);
  
    GtkWidget *gauges[]={gauge1, gauge2, gauge3, gauge4};
    tick_id1=gtk_widget_add_tick_callback(gauge1, (GtkTickCallback)tick_draw, gauges, NULL); 
    tick_id2=gtk_widget_add_tick_callback(gauge2, (GtkTickCallback)tick_draw, gauges, NULL); 
    tick_id3=gtk_widget_add_tick_callback(gauge3, (GtkTickCallback)tick_draw, gauges, NULL); 
    tick_id4=gtk_widget_add_tick_callback(gauge4, (GtkTickCallback)tick_draw, gauges, NULL);    
    
    g_signal_connect(window, "destroy", G_CALLBACK(quit_program), gauges);

    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean tick_draw(GtkWidget *widget, GdkFrameClock *frame_clock, GtkWidget *gauges[])
  {
    gint i=0;

    needle1=needle1+needle_speed;
    needle2=needle2+needle_speed;
    needle3=needle3+needle_speed;
    needle4=needle4+needle_speed;
    for(i=0;i<4;i++)
      {
        switch(i)
          {
            case 0:
              if(needle1<top1)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle1);
                }
              else
                {
                  g_print("Reset Gauge1\n");
                  needle1=0.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle1);
                }
              break;
            case 1:
              if(needle2<top2)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle2);
                }
              else
                {
                  g_print("Reset Gauge2\n");
                  needle2=0.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle2);
                }
              break;
            case 2:
              if(needle3<top3)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle3);
                }
              else
                {
                  g_print("Reset Gauge3\n");
                  needle3=20.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle3);
                }
              break;
            case 3:
              if(needle4<top4)
                {
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle4);
                }
              else
                {
                  g_print("Reset Gauge4\n");
                  needle4=0.0;
                  adjustable_gauge_set_needle(ADJUSTABLE_GAUGE(gauges[i]), needle4);
                }
          }        
      }
    
    return G_SOURCE_CONTINUE;
  }
//The critter drawings and clicks.
static gboolean click_drawing1(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[])
{
  drawing=1;
  needle_speed=0.01;

  //Draw the top active critter.
  gtk_label_set_markup(GTK_LABEL(critters[0]), "<span foreground='white'>You can't handle turtle speed!</span>");

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(critters[1]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[2]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[3]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[4]));

  return FALSE;
}
static gboolean click_drawing2(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[])
{
  drawing=2;
  needle_speed=0.04;

  gtk_label_set_markup(GTK_LABEL(critters[0]), "<span foreground='white'>This is a water dragon lizard, not an alien, OK.</span>");

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(critters[1]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[2]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[3]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[4]));

  return FALSE;
}
static gboolean click_drawing3(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[])
{
  drawing=3;
  needle_speed=0.08;

  gtk_label_set_markup(GTK_LABEL(critters[0]), "<span foreground='white'>This is no turtle!</span>");

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(critters[1]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[2]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[3]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[4]));

  return FALSE;
}
static gboolean click_drawing4(GtkWidget *widget, GdkEvent *event, GtkWidget *critters[])
{
  drawing=4;
  needle_speed=0.12;

  gtk_label_set_markup(GTK_LABEL(critters[0]), "<span foreground='white'>Cheetah speed. Hang on!</span>");

  //Redraw panel critters.
  gtk_widget_queue_draw(GTK_WIDGET(critters[1]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[2]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[3]));
  gtk_widget_queue_draw(GTK_WIDGET(critters[4]));

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

  //Move the turtle and scale a little.
  cairo_translate(cr, 1.5*width/16.0, 2.0*height/16.0);
  cairo_scale(cr, 0.90, 1.0);
 
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

  //Move the lizard and scale a little.
  cairo_translate(cr, 1.5*width/16.0, 1.0*height/16.0);
  cairo_scale(cr, 0.90, 1.0); 
 
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
static gboolean draw_window(GtkWidget *da, cairo_t *cr, gpointer data)
{
  cairo_set_source_rgba(cr, bgc[0], bgc[1], bgc[2], bgc[3]);
  cairo_paint(cr); 
  return FALSE;
}
static void quit_program(GtkWidget *widget, GtkWidget *gauges[])
  {
    if(tick_id1!=0) gtk_widget_remove_tick_callback(gauges[0], tick_id1);
    if(tick_id2!=0) gtk_widget_remove_tick_callback(gauges[1], tick_id2);
    if(tick_id3!=0) gtk_widget_remove_tick_callback(gauges[2], tick_id3);
    if(tick_id4!=0) gtk_widget_remove_tick_callback(gauges[3], tick_id4);
    gtk_main_quit();
  }


