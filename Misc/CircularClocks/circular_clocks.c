
/*

    For use with circular_clocks_main.c. Look in circular_clocks_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<math.h>
#include "circular_clocks.h"

#define CIRCULAR_CLOCKS_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), CIRCULAR_CLOCKS_TYPE, CircularClocksPrivate))

typedef struct _CircularClocksPrivate CircularClocksPrivate;

//For use with getting control points from the coordinate points.
struct point{
  gdouble x;
  gdouble y;
}points;
struct controls{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
}controls;

//Some gear variables.
struct gear_vars{
  gdouble inside_radius;
  gdouble outside_radius;
  gdouble spindle_radius;
  gint teeth;
  gdouble bevel;
  gboolean fill;
  gdouble fill_color[4];
  gdouble rotation;
  gboolean draw_bezier;
}gear_vars;

struct _CircularClocksPrivate
{  
  gint clock_name;
  guint current_second;
  guint timer_id;
  struct gear_vars *g1;
};

enum
{
  PROP_0,
  CLOCK_NAME,
};

//Private functions.
static void circular_clocks_class_init(CircularClocksClass *klass);
static void circular_clocks_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void circular_clocks_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void circular_clocks_init(CircularClocks *da);
static void circular_clocks_finalize(GObject *gobject);
//For the timer.
static gboolean animate_clock(gpointer da);
//Draw a clock.
static gboolean circular_clocks_draw(GtkWidget *widget, cairo_t *cr);
//Different types of clocks.
static void draw_gradient_clock(GtkWidget *da, cairo_t *cr);
static void draw_wave_clock(GtkWidget *da, cairo_t *cr);
static void draw_braid_clock(GtkWidget *da, cairo_t *cr);
static void draw_coords1(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second);
static void draw_coords2(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second);
static void draw_coords3(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second);
static void draw_gear_clock(GtkWidget *da, cairo_t *cr, struct gear_vars *g1);
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1, gint second, struct gear_vars *g1);
static void gear(cairo_t *cr, gdouble w1, struct gear_vars *g1);
static void draw_gouraud_mesh_clock(GtkWidget *da, cairo_t *cr);
//Control points from coordinates.
static GArray* control_points_from_coords2(const GArray *dataPoints);

G_DEFINE_TYPE(CircularClocks, circular_clocks, GTK_TYPE_DRAWING_AREA)

static void circular_clocks_class_init(CircularClocksClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=circular_clocks_set_property;
  gobject_class->get_property=circular_clocks_get_property;

  //Draw when first shown.
  widget_class->draw=circular_clocks_draw;
  gobject_class->finalize = circular_clocks_finalize;

  g_type_class_add_private(klass, sizeof(CircularClocksPrivate));

  g_object_class_install_property(gobject_class, CLOCK_NAME, g_param_spec_int("clock_name", "clock_name", "clock_name", 0, 1, 0, G_PARAM_READWRITE));
}
//Needed for g_object_set().
static void circular_clocks_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  CircularClocks *da=CIRCULAR_CLOCKS(object);

  switch(prop_id)
  {
    case CLOCK_NAME:
      circular_clocks_set_clock(da, g_value_get_int(value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void circular_clocks_set_clock(CircularClocks *da, gint clock_name)
{
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(da);
 
  if(clock_name>=0&&clock_name<5)
    {
      priv->clock_name=clock_name; 
      if(clock_name==GRADIENT_CLOCK)
        {
          g_source_remove(priv->timer_id);
          priv->timer_id=g_timeout_add_seconds(3, (GSourceFunc)animate_clock, GTK_WIDGET(da));
        }
      else
        {
          g_source_remove(priv->timer_id);
          priv->timer_id=g_timeout_add(250, (GSourceFunc)animate_clock, GTK_WIDGET(da));
        }
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("Pick a valid clock. GRADIENT_CLOCK, WAVE_CLOCK, BRAID_CLOCK, GEAR_CLOCK or GOURAUD_MESH_CLOCK.");
    }
}
static void circular_clocks_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  CircularClocks *da=CIRCULAR_CLOCKS(object);
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case CLOCK_NAME:
      g_value_set_int(value, priv->clock_name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
gint circular_clocks_get_clock(CircularClocks *da)
{
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(da);
  return priv->clock_name;
}
static void circular_clocks_init(CircularClocks *da)
{
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(da);

  //Initialize default clock.
  priv->clock_name=GRADIENT_CLOCK;
  //Initialize the current second.
  priv->current_second=0;
  //Initialize the timer for the gradient clock.
  priv->timer_id=g_timeout_add_seconds(3, (GSourceFunc)animate_clock, da);

  //Initialize variables for gear clock.
  priv->g1=g_new(struct gear_vars, 1);
  priv->g1->inside_radius=3.0;
  priv->g1->outside_radius=4.0;
  priv->g1->spindle_radius=2.0;
  priv->g1->teeth=12;
  priv->g1->bevel=G_PI/36.0;
  priv->g1->fill=TRUE;
  priv->g1->fill_color[0]=0.75;
  priv->g1->fill_color[1]=0.75;
  priv->g1->fill_color[2]=0.75;
  priv->g1->fill_color[3]=1.0;
  priv->g1->rotation=-G_PI/2.0; 
  priv->g1->draw_bezier=TRUE;   
}
static void circular_clocks_finalize(GObject *object)
{
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(object);
  g_free(priv->g1);
  if(priv->timer_id!=0) g_source_remove(priv->timer_id);
  G_OBJECT_CLASS(circular_clocks_parent_class)->finalize(object);
}
GtkWidget* circular_clocks_new()
{
  return GTK_WIDGET(g_object_new(circular_clocks_get_type(), NULL));
}
static gboolean animate_clock(gpointer da)
{  
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(da);

  //Check if we are on a new second. If so, redraw.
  GTimeZone *time_zone=g_time_zone_new_local();
  GDateTime *date_time=g_date_time_new_now(time_zone);
  gint second=g_date_time_get_second(date_time);
  g_time_zone_unref(time_zone);
  g_date_time_unref(date_time);

  if(second!=priv->current_second)
    {
      gtk_widget_queue_draw(GTK_WIDGET(da));
      priv->current_second=second; 
    }
   
  return TRUE;
}
static gboolean circular_clocks_draw(GtkWidget *da, cairo_t *cr)
{
  CircularClocksPrivate *priv=CIRCULAR_CLOCKS_GET_PRIVATE(da);

  if(priv->clock_name==GRADIENT_CLOCK) draw_gradient_clock(da, cr);
  else if(priv->clock_name==WAVE_CLOCK) draw_wave_clock(da, cr);
  else if(priv->clock_name==BRAID_CLOCK) draw_braid_clock(da, cr);
  else if(priv->clock_name==GEAR_CLOCK) draw_gear_clock(da, cr, priv->g1);
  else draw_gouraud_mesh_clock(da, cr);
  return FALSE;
}
static void draw_gradient_clock(GtkWidget *da, cairo_t *cr)
 {
   /*
     From http://stackoverflow.com/questions/1734745/how-to-create-circle-with-b%C3%A9zier-curves
     So for 4 points it is (4/3)*tan(pi/8) = 4*(sqrt(2)-1)/3 = 0.552284749831
   */
   gdouble points=0.552284749831;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Draw background.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Blue outside ring.
   cairo_save(cr);
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 8.3*w1/2.0, 8.3*h1/2.0);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_fill(cr);
   cairo_restore(cr);

   //Gray inside.
   cairo_save(cr);
   cairo_set_source_rgb(cr, 0.8, 0.8, 1.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 8.0*w1/2.0, 8.0*h1/2.0);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_fill(cr);
   cairo_restore(cr);

   //Color circle pyramid.
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   cairo_set_line_width(cr, 4.0);
   //purple
   cairo_arc(cr, 5.0*w1, 2.95*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //yellow
   cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
   cairo_arc(cr, 4.85*w1, 3.25*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //cyan
   cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
   cairo_arc(cr, 5.15*w1, 3.25*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //red
   cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
   cairo_arc(cr, 4.70*w1, 3.55*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //green
   cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
   cairo_arc(cr, 5.0*w1, 3.55*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   //blue
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_arc(cr, 5.3*w1, 3.55*h1, 0.10*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   
   //Quadrant 1 arc.
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, (5.0*w1)+(points*4.0*width/10), 1.0*h1,  9.0*w1,  (5.0*h1)-(points*4.0*height/10), 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 8.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 8.0*w1, (5.0*h1)-(points*3.0*height/10),  (5.0*w1)+(points*3.0*w1), 2.0*h1, 5.0*w1, 2.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch(pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);

   //Quadrant 2 arc.
   cairo_pattern_t *pattern2=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern2);
   cairo_mesh_pattern_move_to(pattern2, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 1.0*w1, (5.0*h1)-(points*4.0*height/10), (5.0*w1)-(points*4.0*width/10), 1.0*h1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 5.0*w1, 2.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, (5.0*w1)-(points*3.0*width/10), 2.0*h1,  2.0*w1, (5.0*h1)-(points*3.0*h1), 2.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 2, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 3, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern2);
   cairo_set_source(cr, pattern2);
   cairo_paint(cr);

   //Quadrant 3 arc.
   cairo_pattern_t *pattern3=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern3);
   cairo_mesh_pattern_move_to(pattern3, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, (5.0*w1)-(points*4.0*width/10), 9.0*h1, 1.0*w1, (5.0*h1)+(points*4.0*height/10), 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 2.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 2.0*w1, (5.0*h1)+(points*3.0*height/10),  (5.0*w1)-(points*3.0*width/10), 8.0*h1, 5.0*w1, 8.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 0, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 1, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 2, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch (pattern3);
   cairo_set_source(cr, pattern3);
   cairo_paint(cr);

   //Quadrant 4 arc.
   cairo_pattern_t *pattern4=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern4);
   cairo_mesh_pattern_move_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 9.0*w1, (5.0*h1)+(points*4.0*height/10), (5.0*w1)+(points*4.0*width/10),  9.0*h1, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 5.0*w1, 8.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, (5.0*w1)+(points*3.0*width/10), 8.0*h1,  8.0*w1, (5.0*h1)+(points*3.0*height/10), 8.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 0, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 1, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 2, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 3, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_end_patch (pattern4);
   cairo_set_source(cr, pattern4);
   cairo_paint(cr);
   
   cairo_pattern_destroy(pattern1);
   cairo_pattern_destroy(pattern2);
   cairo_pattern_destroy(pattern3);
   cairo_pattern_destroy(pattern4);

   //Set the clock text.
   gint i=0;
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CGC"};
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   //Start at 12 or radius with just the y component.
   gdouble hour_radius=3.5*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 20);
   cairo_move_to(cr, 0.0, 0.0);
   cairo_translate(cr, width/2.0, height/2.0);
   //Color 12 with blue for contrast with yellow. Other numbers are white.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
       cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
     }

   //Put the clock name on the clock.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_text_extents(cr, hours[12], &tick_extents);
   cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
   cairo_show_text(cr, hours[12]);

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   hour=hour+minute/60.0;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   if(hour>12) hour=hour-12;

   //Hour hand.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=((1.75*w1)*(1.75*h1))/sqrt(((1.75*w1)*(1.75*w1)*temp_sin*temp_sin) + ((1.75*h1)*(1.75*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=((2.75*w1)*(2.75*h1))/sqrt(((2.75*w1)*(2.75*w1)*temp_sin*temp_sin) + ((2.75*h1)*(2.75*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);
 }
static void draw_wave_clock(GtkWidget *da, cairo_t *cr)
 {
   gint i=0;
   gint j=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   gint second=g_date_time_get_second(date_time);
   hour=hour+minute/60.0;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   if(hour>12) hour=hour-12;

   //Paint background.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Points for a circular wave clock. Made of 120 total segments.
   gint rotations=12;
   gint pieces=10;
   gint total=rotations*pieces;
   gint index=0;
   struct point p1;
   GArray *coords1=g_array_sized_new(FALSE, FALSE, sizeof(struct point), total);
   GArray *coords2=g_array_sized_new(FALSE, FALSE, sizeof(struct point), total);
   
   gint move1=0;
   gint move2=0;
   gdouble sind1=-0.25;
   for(i=0;i<rotations;i++)
     {
       for(j=0;j<pieces;j++)
         {   
           index=i*pieces+j;      
           p1.x=(4.25-sind1)*w1*cos((index)*G_PI/60.0-G_PI/2.0);
           p1.y=(4.25-sind1)*h1*sin((index)*G_PI/60.0-G_PI/2.0);
           g_array_append_val(coords1, p1);
           p1.x=(2.5-sind1)*w1*cos((index)*G_PI/60.0-G_PI/2.0);
           p1.y=(2.5-sind1)*h1*sin((index)*G_PI/60.0-G_PI/2.0);
           g_array_append_val(coords2, p1);
           //Add a wiggle to the wave if it is animated.
           if(second>0)
             {
               move1=2*second-3;
               move2=2*second+1;
               if(move1<index&&index<move2) sind1=(0.05)*sin(G_PI*2.0*((gdouble)j+1)/pieces-G_PI/2.0);
               else sind1=(0.25)*sin(G_PI*2.0*((gdouble)j+1)/pieces-G_PI/2.0);
             }
           else sind1=(0.25)*sin(G_PI*2.0*((gdouble)j+1)/pieces-G_PI/2.0);
         }
     } 
   p1.x=0.0;
   p1.y=-4.5*h1;
   g_array_append_val(coords1, p1);
   p1.x=0.0;
   p1.y=-2.75*h1;
   g_array_append_val(coords2, p1);   
  
   //Get the control points.
   GArray *control1=control_points_from_coords2(coords1);
   GArray *control2=control_points_from_coords2(coords2);

   //Draw from the center.
   cairo_translate(cr, width/2.0, height/2.0);
    
   //Draw the curves and gradients.
   gint translate=2*second;
   gdouble color_start1[4];
   gdouble color_stop1[4];
   struct point outside1;
   struct point outside2;
   struct point inside1;
   struct point inside2;
   struct controls c1;
   struct controls c2;   
   for(i=0;i<rotations;i++)
     {
       for(j=0;j<pieces;j++)
         {
           //Get points for the segment.
           index=i*pieces+j;
           outside1=g_array_index(coords1, struct point, index);
           outside2=g_array_index(coords1, struct point, index+1);
           c1=g_array_index(control1, struct controls, index);
           inside1=g_array_index(coords2, struct point, index);
           inside2=g_array_index(coords2, struct point, index+1);
           c2=g_array_index(control2, struct controls, index);

           //Draw gradient for the segment.
           if(index<translate)
             {
               color_start1[0]=0.0;
               color_start1[1]=(gdouble)index/translate;
               color_start1[2]=1.0-((gdouble)index)/translate;
               color_start1[3]=0.7;
               color_stop1[0]=0.0;
               color_stop1[1]=((gdouble)index+1.0)/translate;
               color_stop1[2]=1.0-((gdouble)index+1.0)/translate;
               color_stop1[3]=0.7;
             }
           else
             {
               color_start1[0]=0.0;
               color_start1[1]=1.0-((gdouble)index)/(gdouble)total;
               color_start1[2]=((gdouble)index)/(gdouble)total;
               color_start1[3]=0.7;
               color_stop1[0]=0.0;
               color_stop1[1]=1.0-((gdouble)index+1.0)/(gdouble)total;
               color_stop1[2]=((gdouble)index+1.0)/(gdouble)total;
               color_stop1[3]=0.7;
             }
           cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
           cairo_mesh_pattern_begin_patch(pattern1);
           cairo_mesh_pattern_move_to(pattern1, outside1.x, outside1.y);
           cairo_mesh_pattern_curve_to(pattern1, c1.x1, c1.y1, c1.x2, c1.y2, outside2.x, outside2.y);
           cairo_mesh_pattern_line_to(pattern1, inside2.x, inside2.y);
           cairo_mesh_pattern_curve_to(pattern1, c2.x2, c2.y2, c2.x1, c2.y1, inside1.x, inside1.y);
           cairo_mesh_pattern_line_to(pattern1, outside1.x, outside1.y);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_start1[0], color_start1[1], color_start1[2], color_start1[3]);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2], color_stop1[3]);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2], color_stop1[3]);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_start1[0], color_start1[1], color_start1[2], color_start1[3]);
           cairo_mesh_pattern_end_patch(pattern1);
           cairo_set_source(cr, pattern1);
           cairo_paint(cr);
           cairo_pattern_destroy(pattern1);

           //Draw inside line so it can be closed and filled.
           cairo_curve_to(cr, c2.x1, c2.y1, c2.x2, c2.y2, inside2.x, inside2.y);
           cairo_stroke_preserve(cr);                
         }      
     }

   //Fill inside of clock.
   cairo_set_source_rgba(cr, 0.0, 0.3, 1.0, 0.7);
   cairo_fill(cr);
   
   g_array_free(coords1, TRUE);
   g_array_free(coords2, TRUE);
   g_array_free(control1, TRUE);
   g_array_free(control2, TRUE);

   //Set the clock text.
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CWC"};
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   //Start at 12 or radius with just the y component.
   gdouble hour_radius=3.5*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 20);
   cairo_move_to(cr, 0.0, 0.0);
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
     }

   //Put the clock name on the clock.
   cairo_text_extents(cr, hours[12], &tick_extents);
   cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
   cairo_show_text(cr, hours[12]);

   //Hour hand.
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=((1.5*w1)*(1.5*h1))/sqrt(((1.5*w1)*(1.5*w1)*temp_sin*temp_sin) + ((1.5*h1)*(1.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=((2.5*w1)*(2.5*h1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin*temp_sin) + ((2.5*h1)*(2.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);
 }
static void draw_braid_clock(GtkWidget *da, cairo_t *cr)
 {
   gint i=0;
   gint j=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   gint second=g_date_time_get_second(date_time);
   hour=hour+minute/60.0;
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   if(hour>12) hour=hour-12;

   //Paint background.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Points for a circular braid clock.
   gint rotations=6;
   gint inc=10;
   struct point p1;
   GArray *coords1=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords2=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords3=g_array_new(FALSE, FALSE, sizeof(struct point));
   gdouble sind1=-0.75;
   gdouble sind2=0.0;
   for(i=0;i<rotations;i++)
     {
       for(j=0;j<inc;j++)
         {         
           p1.x=(3.0-sind1)*w1*cos((i*inc+j)*G_PI/30.0-G_PI/2.0);
           p1.y=(3.0-sind1)*h1*sin((i*inc+j)*G_PI/30.0-G_PI/2.0);
           g_array_append_val(coords1, p1);
           p1.x=(3.0+sind1)*w1*cos((i*inc+j)*G_PI/30.0-G_PI/2.0);
           p1.y=(3.0+sind1)*h1*sin((i*inc+j)*G_PI/30.0-G_PI/2.0);
           g_array_append_val(coords2, p1);
           p1.x=(3.0+sind2)*w1*cos((i*inc+j)*G_PI/30.0-G_PI/2.0);
           p1.y=(3.0+sind2)*h1*sin((i*inc+j)*G_PI/30.0-G_PI/2.0);
           g_array_append_val(coords3, p1);  
           sind1=(0.75)*sin(G_PI*2.0*((gdouble)j+1)/inc-G_PI/2.0);
           sind2=(0.75)*sin(G_PI*2.0*((gdouble)j+1)/inc);
         }
     } 
   p1.x=0.0;
   p1.y=-3.75*h1;
   g_array_append_val(coords1, p1);
   p1.x=0.0;
   p1.y=-2.25*h1;
   g_array_append_val(coords2, p1); 
   p1.x=0.0;
   p1.y=-3.0*h1;
   g_array_append_val(coords3, p1);     
  
   //Get the control points.
   GArray *control1=control_points_from_coords2(coords1);
   GArray *control2=control_points_from_coords2(coords2);
   GArray *control3=control_points_from_coords2(coords3);

   //Draw from the center.
   cairo_translate(cr, width/2.0, height/2.0);
    
   //Draw the curves.
   gint draw_order=-1;
   cairo_set_line_width(cr, 20.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   gint half1=2*rotations;
   gint half2=inc/2;    
   for(i=0;i<half1;i++)
     {
       draw_order++;
       if(draw_order>2) draw_order=0;
       //Braid order 1 to 3, 2 to 1, 3 to 2.
       switch(draw_order)
         {
           case 0:
             draw_coords1(cr, coords1, control1, i, half2, second);
             draw_coords2(cr, coords2, control2, i, half2, second);
             draw_coords3(cr, coords3, control3, i, half2, second);
             break;
           case 1:
             draw_coords3(cr, coords3, control3, i, half2, second);
             draw_coords1(cr, coords1, control1, i, half2, second);
             draw_coords2(cr, coords2, control2, i, half2, second);
             break;
           case 2:
             draw_coords2(cr, coords2, control2, i, half2, second);
             draw_coords3(cr, coords3, control3, i, half2, second);
             draw_coords1(cr, coords1, control1, i, half2, second);
             break;
         }          
     }
   
   g_array_free(coords1, TRUE);
   g_array_free(coords2, TRUE);
   g_array_free(coords3, TRUE);
   g_array_free(control1, TRUE);
   g_array_free(control2, TRUE);
   g_array_free(control3, TRUE);

   //Set the clock text.
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CBC"};
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   //Start at 12 or radius with just the y component.
   gdouble hour_radius=3.5*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 20);
   cairo_move_to(cr, 0.0, 0.0);
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //The polar form of the equation for an ellipse to get the radius.
       hour_radius=((3.5*w1)*(3.5*h1))/sqrt(((3.5*w1)*(3.5*w1)*temp_sin*temp_sin) + ((3.5*h1)*(3.5*h1)*temp_cos*temp_cos));
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
     }

   //Put the clock name on the clock.
   cairo_text_extents(cr, hours[12], &tick_extents);
   cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
   cairo_show_text(cr, hours[12]);

   //Hour hand.
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=((1.5*w1)*(1.5*h1))/sqrt(((1.5*w1)*(1.5*w1)*temp_sin*temp_sin) + ((1.5*h1)*(1.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=((2.5*w1)*(2.5*h1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin*temp_sin) + ((2.5*h1)*(2.5*h1)*temp_cos*temp_cos));
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);
 }
static void draw_coords1(cairo_t *cr, GArray *coords1, GArray *control1, gint i, gint half2, gint second)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(second==index) cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
        d1=g_array_index(coords1, struct point, index);
        d2=g_array_index(coords1, struct point, index+1);
        c1=g_array_index(control1, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
static void draw_coords2(cairo_t *cr, GArray *coords2, GArray *control2, gint i, gint half2, gint second)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(second==index) cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
        d1=g_array_index(coords2, struct point, index);
        d2=g_array_index(coords2, struct point, index+1);
        c1=g_array_index(control2, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }
static void draw_coords3(cairo_t *cr, GArray *coords3, GArray *control3, gint i, gint half2, gint second)
  {
    gint j=0;
    gint index=0;
    struct point d1;
    struct point d2;
    struct controls c1; 

    for(j=0;j<half2;j++)
      {
        index=i*half2+j;
        if(second==index) cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        else cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        d1=g_array_index(coords3, struct point, index);
        d2=g_array_index(coords3, struct point, index+1);
        c1=g_array_index(control3, struct controls, index);
        cairo_move_to(cr, d1.x, d1.y);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, d2.x, d2.y);
        cairo_stroke(cr);
      }
  }

static void draw_gear_clock(GtkWidget *da, cairo_t *cr, struct gear_vars *g1)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Get the current time.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gdouble hour=(gdouble)g_date_time_get_hour(date_time);
   gdouble minute=(gdouble)g_date_time_get_minute(date_time);
   gint second=g_date_time_get_second(date_time);
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);
   hour=hour+minute/60.0;
   if(hour>12) hour=hour-12;

   //Paint background.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Keep the gear drawing circular.
   if(w1>h1) w1=h1;
     
   //Draw the gear or gears.
   gears1(cr, width, height, w1, second, g1);

   //Circular coordinates with marks at each second or pi/30.
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_arc(cr, 0.0, 0.0, 4.0*w1, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   gdouble start=-G_PI/2.0;
   gdouble next_second=-G_PI/30.0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   for(i=0;i<60;i++)
     {
       temp_cos1=cos(start-(next_second*i));
       temp_sin1=sin(start-(next_second*i));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       temp_cos1=temp_cos1*3.6*w1;
       temp_sin1=temp_sin1*3.6*w1;
       temp_cos2=temp_cos2*4.0*w1;
       temp_sin2=temp_sin2*4.0*w1;
       cairo_move_to(cr, temp_cos1, temp_sin1);
       cairo_line_to(cr, temp_cos2, temp_sin2);
       cairo_stroke(cr);
     }

   //Set the clock text.
   gchar *hours[]={"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "CGrC"};
   gdouble hour_start=-G_PI/2.0;
   gdouble next_hour=-G_PI/6.0;
   //Start at 12 or radius with just the y component.
   gdouble hour_radius=3.5*h1;
   gdouble temp_cos=0;
   gdouble temp_sin=0;
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 20);
   cairo_move_to(cr, 0.0, 0.0);
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   for(i=0;i<12;i++)
     {
       temp_cos=cos(hour_start-(next_hour*i));
       temp_sin=sin(hour_start-(next_hour*i));
       //Keep the gear clock square.
       hour_radius=3.5*w1;
       cairo_text_extents(cr, hours[i], &tick_extents);
       temp_cos=temp_cos*hour_radius-tick_extents.width/2.0;
       temp_sin=temp_sin*hour_radius+tick_extents.height/2.0;
       cairo_move_to(cr, temp_cos, temp_sin);
       cairo_show_text(cr, hours[i]);
     }

   //Put the clock name on the clock.
   cairo_text_extents(cr, hours[12], &tick_extents);
   cairo_move_to(cr, -(tick_extents.width/2.0), (1.5*h1)+(tick_extents.height/2.0));
   cairo_show_text(cr, hours[12]);

   //Hour hand.
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_hour*hour));
   temp_sin=sin(hour_start-(next_hour*hour));
   hour_radius=1.5*w1;
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);

   //Minute hand.
   gdouble next_minute=-G_PI/(6.0*5.0);
   cairo_move_to(cr, 0.0, 0.0);
   temp_cos=cos(hour_start-(next_minute*minute));
   temp_sin=sin(hour_start-(next_minute*minute));
   hour_radius=2.5*w1;
   temp_cos=temp_cos*hour_radius;
   temp_sin=temp_sin*hour_radius;
   cairo_line_to(cr, temp_cos, temp_sin);
   cairo_stroke(cr);
 }
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1, gint second, struct gear_vars *g1)
  {
    cairo_save(cr);
    cairo_set_source_rgb(cr, g1->fill_color[0], g1->fill_color[1], g1->fill_color[2]);
    cairo_set_line_width(cr, 1.0);
    cairo_translate(cr, width/2.0, height/2.0);
    cairo_rotate(cr, g1->rotation+second*G_PI/30);

    //Draw the gear. 
    gear(cr, w1, g1);

    //Draw the inside circle of the gear.
    cairo_arc(cr, 0.0, 0.0, g1->spindle_radius*w1, 0, 2*G_PI);
    if(g1->fill)
      {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_fill(cr);
      }
    else cairo_stroke(cr);
   
    //The timing mark at 0 radians on the unit circle.
    cairo_set_line_width(cr, 8.0);
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_move_to(cr, (g1->inside_radius*w1)+0.5*(g1->outside_radius*w1-g1->inside_radius*w1), 0.0);
    cairo_line_to(cr, g1->outside_radius*w1, 0.0);
    cairo_stroke(cr);  
    cairo_restore(cr);
  }
static void gear(cairo_t *cr, gdouble w1, struct gear_vars *g1)
  {
    gint i=0;
    gdouble step=-G_PI*2.0/(g1->teeth);

    //Outside points.
    gdouble outside_start1=-(g1->bevel);
    gdouble outside_cos1=0;
    gdouble outside_sin1=0;
    gdouble outside_start2=(g1->bevel);
    gdouble outside_cos2=0;
    gdouble outside_sin2=0;
    //Inside points.
    gdouble inside_start1=G_PI/(g1->teeth)-(g1->bevel);
    gdouble inside_cos1=0;
    gdouble inside_sin1=0;
    gdouble inside_start2=G_PI/(g1->teeth)+(g1->bevel);
    gdouble inside_cos2=0;
    gdouble inside_sin2=0;
    //Draw the gear teeth.
    outside_cos1=cos(outside_start1-(step*i))*(g1->outside_radius)*w1;
    outside_sin1=sin(outside_start1-(step*i))*(g1->outside_radius)*w1;                
    cairo_move_to(cr, outside_cos1, outside_sin1);
    if(!g1->draw_bezier)
      {
        for(i=0;i<(g1->teeth);i++)
          {
            outside_cos2=cos(outside_start2-(step*i))*(g1->outside_radius)*w1;
            outside_sin2=sin(outside_start2-(step*i))*(g1->outside_radius)*w1;                
            cairo_line_to(cr, outside_cos2, outside_sin2);
            cairo_stroke_preserve(cr);   
         
            inside_cos1=cos(inside_start1-(step*i))*(g1->inside_radius)*w1;
            inside_sin1=sin(inside_start1-(step*i))*(g1->inside_radius)*w1;
            cairo_line_to(cr, inside_cos1, inside_sin1);
            cairo_stroke_preserve(cr); 

            inside_cos2=cos(inside_start2-(step*i))*(g1->inside_radius)*w1;
            inside_sin2=sin(inside_start2-(step*i))*(g1->inside_radius)*w1;
            cairo_line_to(cr, inside_cos2, inside_sin2);
            cairo_stroke_preserve(cr); 

            outside_cos1=cos(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;
            outside_sin1=sin(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;   
            cairo_line_to(cr, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);                           
         }
      }
    else
      {
        for(i=0;i<(g1->teeth);i++)
          {              
            outside_cos2=cos(outside_start2-(step*i))*(g1->outside_radius)*w1;
            outside_sin2=sin(outside_start2-(step*i))*(g1->outside_radius)*w1;                
            cairo_line_to(cr, outside_cos2, outside_sin2);
            cairo_stroke_preserve(cr);   
         
            inside_cos1=cos(inside_start1-(step*i))*(g1->inside_radius)*w1;
            inside_sin1=sin(inside_start1-(step*i))*(g1->inside_radius)*w1;

            inside_cos2=cos(inside_start2-(step*i))*(g1->inside_radius)*w1;
            inside_sin2=sin(inside_start2-(step*i))*(g1->inside_radius)*w1;

            outside_cos1=cos(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;
            outside_sin1=sin(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;   
            cairo_curve_to(cr, inside_cos1, inside_sin1, inside_cos2, inside_sin2, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);                   
         }
     }
    cairo_close_path(cr);
    if(g1->fill) cairo_fill(cr); 
    else cairo_stroke(cr);
  }
static void draw_gouraud_mesh_clock(GtkWidget *da, cairo_t *cr)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   
   //The seconds value.
   GTimeZone *time_zone=g_time_zone_new_local();
   GDateTime *date_time=g_date_time_new_now(time_zone);
   gint hours=g_date_time_get_hour(date_time);
   gint minutes=g_date_time_get_minute(date_time);
   gint seconds=g_date_time_get_second(date_time);
   gboolean pm=FALSE;
   if(hours>12)
     {
       hours=hours-12;
       pm=TRUE;
     }
   g_time_zone_unref(time_zone);
   g_date_time_unref(date_time);

   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Set background.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Black inside.
   cairo_save(cr);
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 4.0*w1, 4.0*h1);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_fill(cr);
   cairo_restore(cr);

   //The sweeping line and tick marks.
   cairo_save(cr);
   gdouble start=-G_PI/2.0;
   gdouble next_second=-G_PI/30.0;
   gdouble line_radius1=0;
   gdouble line_radius2=0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   gdouble prev_cos=0;
   gdouble prev_sin=0;
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_line_width(cr, 2.0);
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0); 
   for(i=0;i<60;i++)
     {
       temp_cos1=cos(start-(next_second*i));
       temp_sin1=sin(start-(next_second*i));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       //The polar form of the equation for an ellipse to get the radius.
       line_radius1=((3.6*w1)*(3.6*h1))/sqrt(((3.6*w1)*(3.6*w1)*temp_sin1*temp_sin1) + ((3.6*h1)*(3.6*h1)*temp_cos1*temp_cos1));
       line_radius2=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin1*temp_sin1) + ((4.0*h1)*(4.0*h1)*temp_cos1*temp_cos1));
       temp_cos1=temp_cos1*line_radius1;
       temp_sin1=temp_sin1*line_radius1;
       temp_cos2=temp_cos2*line_radius2;
       temp_sin2=temp_sin2*line_radius2;
       cairo_move_to(cr, temp_cos1, temp_sin1);
       cairo_line_to(cr, temp_cos2, temp_sin2);
       cairo_stroke(cr);
       if(i==seconds)
         {
           //Extend outside the inside circle to fill the clip region. Cover with a outside ring.
           line_radius1=((4.1*w1)*(4.1*h1))/sqrt(((4.1*w1)*(4.1*w1)*temp_sin1*temp_sin1) + ((4.1*h1)*(4.1*h1)*temp_cos1*temp_cos1));
           prev_cos=prev_cos*line_radius1;
           prev_sin=prev_sin*line_radius1;
           temp_cos1=temp_cos1*line_radius1;
           temp_sin1=temp_sin1*line_radius1;
           cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
           cairo_mesh_pattern_begin_patch(pattern1);
           cairo_mesh_pattern_move_to(pattern1, prev_cos, prev_sin);
           cairo_mesh_pattern_line_to(pattern1, temp_cos1, temp_sin1);
           cairo_mesh_pattern_line_to(pattern1, 0.0, 0.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, 1.0, 0.0, 1.0, 0.7);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, 0.0, 0.0, 1.0, 1.0);
           cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, 1.0, 0.0, 1.0, 0.7);
           cairo_mesh_pattern_end_patch(pattern1);
           cairo_set_source(cr, pattern1);
           cairo_paint(cr);   
           cairo_pattern_destroy(pattern1);
           cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
         }
       prev_cos=temp_cos1;
       prev_sin=temp_sin1;
    }
   cairo_restore(cr);

   //Black outside ring.
   cairo_save(cr);
   cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
   cairo_set_line_width(cr, .08);
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_scale(cr, 4.0*w1, 4.0*h1);
   cairo_arc(cr, 0.0, 0.0, 1.0, 0.0, 2.0*M_PI);
   cairo_stroke(cr);
   cairo_restore(cr);

   //Draw the seconds
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
   cairo_text_extents_t tick_extents;
   cairo_set_font_size(cr, 25*width/400);
   gchar *string=NULL;
   if(seconds<10&&minutes<10)
     {
       if(pm) string=g_strdup_printf("%i:0%i:0%i PM", hours, minutes, seconds);
       else string=g_strdup_printf("%i:0%i:0%i AM", hours, minutes, seconds);
     }
   else if(seconds<10)
     {
       if(pm) string=g_strdup_printf("%i:%i:0%i PM", hours, minutes, seconds);
       else string=g_strdup_printf("%i:%i:0%i AM", hours, minutes, seconds);
     }
   else if(minutes<10)
     {
       if(pm) string=g_strdup_printf("%i:0%i:%i PM", hours, minutes, seconds);
       else string=g_strdup_printf("%i:0%i:%i AM", hours, minutes, seconds);
     }
   else
     {
       if(pm) string=g_strdup_printf("%i:%i:%i PM", hours, minutes, seconds);
       else string=g_strdup_printf("%i:%i:%i AM", hours, minutes, seconds);
     }
   cairo_text_extents(cr, string, &tick_extents);
   cairo_move_to(cr, 0.0-tick_extents.width/2.0, 1.5*h1+tick_extents.height/2.0);
   cairo_show_text(cr, string);
   g_free(string);
 }
/*
    This is some exellent work done by Ramsundar Shandilya. Note the following for the original work
    and the rational behind it.
    
    https://medium.com/@ramshandilya/draw-smooth-curves-through-a-set-of-points-in-ios-34f6d73c8f9

    https://github.com/Ramshandilya/Bezier

    The MIT License (MIT)

    Copyright (c) 2015 Ramsundar Shandilya

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
  
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    This is a translation of the original Swift code to C. It makes the function easy to use with GTK+,
    cairo and glib.
*/
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=0;
    if(dataPoints!=NULL) count=dataPoints->len-1;
    gdouble *fCP=NULL;
    gdouble *sCP=NULL;

    if(count>0)
      {
        fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
      }
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count<1||dataPoints==NULL)
      {
        //Return NULL.
        controlPoints=NULL;
        g_print("Can't get control points from coordinates. NULL returned.\n");
      }
    else if(count==1)
      {
        struct point P0=g_array_index(dataPoints, struct point, 0);
        struct point P3=g_array_index(dataPoints, struct point, 1);

        //Calculate First Control Point
        //3P1 = 2P0 + P3
        struct point P1;
        P1.x=(2.0*P0.x+P3.x)/3.0;
        P1.y=(2.0*P0.y+P3.y)/3.0;

        *(fCP)=P1.x;
        *(fCP+1)=P1.y;

        //Calculate second Control Point
        //P2 = 2P1 - P0
        struct point P2;
        P2.x=(2.0*P1.x-P0.x);
        P2.y=(2.0*P1.y-P0.x);

        *(sCP)=P2.x;
        *(sCP+1)=P2.y;      
      }
    else
      {
        gdouble *rhs=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        gdouble *a=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *b=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *c=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble rhsValueX=0;
        gdouble rhsValueY=0;
        struct point P0;
        struct point P3;        
        gdouble m=0;
        gdouble b1=0;
        gdouble r2x=0;
        gdouble r2y=0;
        gdouble P1_x=0;
        gdouble P1_y=0;
   
        for(i=0;i<count;i++)
          {
            P0=g_array_index(dataPoints, struct point, i);
            P3=g_array_index(dataPoints, struct point, i+1);

            if(i==0)
              {
                *(a)=0.0;
                *(b)=2.0;
                *(c)=1.0;

                //rhs for first segment
                rhsValueX=P0.x+2.0*P3.x;
                rhsValueY=P0.y+2.0*P3.y;
              }
            else if(i==count-1)
              {
                *(a+i)=2.0;
                *(b+i)=7.0;
                *(c+i)=0.0;

                //rhs for last segment
                rhsValueX=8.0*P0.x+P3.x;
                rhsValueY=8.0*P0.y+P3.y;
              }
            else
              {
                *(a+i)=1.0;
                *(b+i)=4.0;
                *(c+i)=1.0;

                rhsValueX=4.0*P0.x+2.0*P3.x;
                rhsValueY=4.0*P0.y+2.0*P3.y;
              }
            *(rhs+i*2)=rhsValueX;
            *(rhs+i*2+1)=rhsValueY;
          }

        //Solve Ax=B. Use Tridiagonal matrix algorithm a.k.a Thomas Algorithm
        for(i=1;i<count;i++)
          {
            m=(*(a+i))/(*(b+i-1));

            b1=(*(b+i))-m*(*(c+i-1));
            *(b+i)=b1;

            r2x=(*(rhs+i*2))-m*(*(rhs+(i-1)*2));
            r2y=(*(rhs+i*2+1))-m*(*(rhs+(i-1)*2+1));

            *(rhs+i*2)=r2x;
            *(rhs+i*2+1)=r2y;
          }

        //Get First Control Points
        
        //Last control Point
        gdouble lastControlPointX=(*(rhs+2*count-2))/(*(b+count-1));
        gdouble lastControlPointY=(*(rhs+2*count-1))/(*(b+count-1));

        *(fCP+2*count-2)=lastControlPointX;
        *(fCP+2*count-1)=lastControlPointY;

        gdouble controlPointX=0;
        gdouble controlPointY=0;

        for(i=count-2;i>=0;--i)
          {
            controlPointX=(*(rhs+i*2)-(*(c+i))*(*(fCP+(i+1)*2)))/(*(b+i));
            controlPointY=(*(rhs+i*2+1)-(*(c+i))*(*(fCP+(i+1)*2+1)))/(*(b+i));

             *(fCP+i*2)=controlPointX;
             *(fCP+i*2+1)=controlPointY; 
          }

        //Compute second Control Points from first.
        for(i=0;i<count;i++)
          {
            if(i==count-1)
              {
                P3=g_array_index(dataPoints, struct point, i+1);
                P1_x=(*(fCP+i*2));
                P1_y=(*(fCP+i*2+1));

                controlPointX=(P3.x+P1_x)/2.0;
                controlPointY=(P3.y+P1_y)/2.0;

                *(sCP+count*2-2)=controlPointX;
                *(sCP+count*2-1)=controlPointY;
              }
            else
              {
                P3=g_array_index(dataPoints, struct point, i+1);                
                P1_x=(*(fCP+(i+1)*2));
                P1_y=(*(fCP+(i+1)*2+1));

                controlPointX=2.0*P3.x-P1_x;
                controlPointY=2.0*P3.y-P1_y;

                *(sCP+i*2)=controlPointX;
                *(sCP+i*2+1)=controlPointY;
              }

          }

        controlPoints=g_array_new(FALSE, FALSE, sizeof(struct controls));
        struct controls cp;
        for(i=0;i<count;i++)
          {
            cp.x1=(*(fCP+i*2));
            cp.y1=(*(fCP+i*2+1));
            cp.x2=(*(sCP+i*2));
            cp.y2=(*(sCP+i*2+1));
            g_array_append_val(controlPoints, cp);
          }

        g_free(rhs);
        g_free(a);
        g_free(b);
        g_free(c);
     }

    if(fCP!=NULL) g_free(fCP);
    if(sCP!=NULL) g_free(sCP);

    return controlPoints;
  }



















