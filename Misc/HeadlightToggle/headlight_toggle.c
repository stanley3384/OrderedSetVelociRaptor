
/*

    For use with headlight_toggle_main.c. Look in headlight_toggle_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "headlight_toggle.h"

#define HEADLIGHT_TOGGLE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), HEADLIGHT_TOGGLE_TYPE, HeadlightTogglePrivate))

typedef struct _HeadlightTogglePrivate HeadlightTogglePrivate;

struct _HeadlightTogglePrivate
{  
  gint headlight_toggle_state;
  gint headlight_toggle_icon;
};

enum
{
  PROP_0,
  HEADLIGHT_TOGGLE_STATE,
  HEADLIGHT_TOGGLE_ICON
};

//Private functions.
static void headlight_toggle_class_init(HeadlightToggleClass *klass);
static void headlight_toggle_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void headlight_toggle_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void headlight_toggle_init(HeadlightToggle *da);
static gboolean headlight_toggle_draw(GtkWidget *widget, cairo_t *cr);
static void headlight_toggle_horizontal_right_draw(GtkWidget *da, cairo_t *cr);
static void headlight_icon_drawing(cairo_t *cr, gint width, gint height, gint headlight_toggle_state);
static void emergency_light_icon_drawing(cairo_t *cr, gint width, gint height, gint headlight_toggle_state);
static void headlight_toggle_finalize(GObject *gobject);

G_DEFINE_TYPE(HeadlightToggle, headlight_toggle, GTK_TYPE_DRAWING_AREA)

static void headlight_toggle_class_init(HeadlightToggleClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=headlight_toggle_set_property;
  gobject_class->get_property=headlight_toggle_get_property;

  //Draw when first shown.
  widget_class->draw=headlight_toggle_draw;
  gobject_class->finalize = headlight_toggle_finalize;

  g_type_class_add_private(klass, sizeof(HeadlightTogglePrivate));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_STATE, g_param_spec_int("headlight_toggle_state", "headlight_toggle_state", "headlight_toggle_state", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_ICON, g_param_spec_int("headlight_toggle_icon", "headlight_toggle_icon", "headlight_toggle_icon", 0, 1, 0, G_PARAM_READWRITE));
}
//Needed for g_object_set().
static void headlight_toggle_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  HeadlightToggle *da=HEADLIGHT_TOGGLE(object);

  switch(prop_id)
  {
    g_print("Prop %i\n", prop_id);
    case HEADLIGHT_TOGGLE_STATE:
      headlight_toggle_set_state(da, g_value_get_int(value));
      break;
    case HEADLIGHT_TOGGLE_ICON:
      headlight_toggle_set_icon(da, g_value_get_int(value));
      break;    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void headlight_toggle_set_state(HeadlightToggle *da, gint headlight_toggle_state)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
 
  if(headlight_toggle_state==0||headlight_toggle_state==1)
    { 
      priv->headlight_toggle_state=headlight_toggle_state;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The headlight toggle state can be HEADLIGHT_TOGGLE_OFF=0, HEADLIGHT_TOGGLE_ON=1");
    }
}
void headlight_toggle_set_icon(HeadlightToggle *da, gint headlight_toggle_icon)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
 
  if(headlight_toggle_icon==0||headlight_toggle_icon==1)
    { 
      priv->headlight_toggle_icon=headlight_toggle_icon;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The headlight icon can be HEADLIGHT_ICON=0, EMERGENCY_ICON=1");
    }
}
static void headlight_toggle_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  HeadlightToggle *da=HEADLIGHT_TOGGLE(object);
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case HEADLIGHT_TOGGLE_STATE:
      g_value_set_int(value, priv->headlight_toggle_state);
      break;
    case HEADLIGHT_TOGGLE_ICON:
      g_value_set_int(value, priv->headlight_toggle_icon);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
gint headlight_toggle_get_state(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->headlight_toggle_state;
}
gint headlight_toggle_get_icon(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->headlight_toggle_icon;
}
static void headlight_toggle_init(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  //Start in the OFF position.
  priv->headlight_toggle_state=HEADLIGHT_TOGGLE_OFF; 
  //Default icon drawing to headlight.
  priv->headlight_toggle_icon=HEADLIGHT_ICON; 
}
GtkWidget* headlight_toggle_new()
{
  return GTK_WIDGET(g_object_new(headlight_toggle_get_type(), NULL));
}
static gboolean headlight_toggle_draw(GtkWidget *da, cairo_t *cr)
{
  
  headlight_toggle_horizontal_right_draw(da, cr);
  
  return FALSE;
}
static void headlight_toggle_horizontal_right_draw(GtkWidget *da, cairo_t *cr)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  //Position toggle gradients.
  gint button_start=0;
  gint button_left=width/8;
  gint button_middle=width/2;
  gint button_right=7*width/8;

  //Paint background.    
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_paint(cr);
     
  //The far left button gradient and rectangle.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(button_start, 0.0, button_left, 0.0);
  if(priv->headlight_toggle_state==1)
    {  
      cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.6, 0.6, 0.6, 1.0); 
      cairo_set_source(cr, pattern1);    
      cairo_rectangle(cr, button_start, 0.0, button_left, height);
      cairo_fill(cr);
    }

  //The left gradient and rectangle.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(button_left, 0.0, button_middle, 0.0);
  if(priv->headlight_toggle_state==1)
    {
      cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.4, 0.4, 0.4, 1.0); 
      cairo_set_source(cr, pattern2);     
      cairo_rectangle(cr, button_left, 0, button_middle-button_left, height);
      cairo_fill(cr);
    }

  //The button is off. Draw left part a solid color.
  if(priv->headlight_toggle_state==0)
    {
      cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
      cairo_rectangle(cr, button_start, 0, button_middle, height);
      cairo_fill(cr);
    }

  //The lit part of the switch.
  if(priv->headlight_toggle_state==0) cairo_set_source_rgba(cr, 0.8, 0.0, 0.0, 1.0);
  else cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
  cairo_rectangle(cr, button_middle, 0, width-button_middle, height);
  cairo_fill(cr);

  //The right gradient.
  cairo_pattern_t *pattern3=cairo_pattern_create_linear(button_right, 0.0, width, 0.0);
  if(priv->headlight_toggle_state==0)
    {
      cairo_pattern_add_color_stop_rgba(pattern3, 0.0, 1.0, 0.0, 0.0, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern3, 1.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_set_source(cr, pattern3);    
      cairo_rectangle(cr, button_right, 0, width-button_right, height);
      cairo_fill(cr);
    }

  //The outside rectangle.
  cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
  cairo_set_line_width(cr, 8.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  //The headlight drawing.
  if(priv->headlight_toggle_icon==0)
    {
      headlight_icon_drawing(cr, width, height, priv->headlight_toggle_state);
    }
  else
    {
      emergency_light_icon_drawing(cr, width, height, priv->headlight_toggle_state);
    }
  
  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
  cairo_pattern_destroy(pattern3);
}
static void headlight_icon_drawing(cairo_t *cr, gint width, gint height, gint headlight_toggle_state)
{
  if(headlight_toggle_state==0)
    {
      cairo_set_source_rgba(cr, 0.8, 0.8, 0.0, 1.0);
      cairo_scale(cr, 0.9, 1.0);
      cairo_translate(cr, 0.05*width, 0.0); 
    }
  else
    {
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    }
  cairo_set_line_width(cr, 3.0);
  cairo_move_to(cr, 6.0*width/8.0, height/4.0);
  cairo_curve_to(cr, width/2.0, 3.0*height/8.0, width/2.0, 5.0*height/8.0, 6.0*width/8.0, 3.0*height/4.0);
  cairo_stroke(cr);
  //The headlight ellipse.
  cairo_save(cr);
  cairo_set_line_width(cr, 6.0);
  cairo_scale(cr, 0.2, 1.0);
  cairo_translate(cr, 4.0*6.0*width/8.0, 0.0);
  cairo_arc(cr, 6.0*width/8.0, height/2.0, height/4.0, 0.0, 2*G_PI);
  cairo_restore(cr);
  cairo_stroke(cr);
  //The 5 headlight rays.
  cairo_move_to(cr, 6.0*width/8.0, 5.0*height/16.0);
  cairo_line_to(cr, 7.0*width/8.0, 3.0*height/16.0);
  cairo_stroke(cr); 
  cairo_move_to(cr, 6.0*width/8.0, 6.5*height/16.0);
  cairo_line_to(cr, 7.0*width/8.0, 5.25*height/16.0);
  cairo_stroke(cr); 
  cairo_move_to(cr, 6.0*width/8.0, 8.0*height/16.0);
  cairo_line_to(cr, 7.0*width/8.0, 8.0*height/16.0);
  cairo_stroke(cr); 
  cairo_move_to(cr, 6.0*width/8.0, 9.25*height/16.0);
  cairo_line_to(cr, 7.0*width/8.0, 11.0*height/16.0);
  cairo_stroke(cr); 
  cairo_move_to(cr, 6.0*width/8.0, 11.0*height/16.0);
  cairo_line_to(cr, 7.0*width/8.0, 13.0*height/16.0);
  cairo_stroke(cr); 
}
static void emergency_light_icon_drawing(cairo_t *cr, gint width, gint height, gint headlight_toggle_state)
{
  if(headlight_toggle_state==0)
    {
      cairo_set_source_rgba(cr, 0.8, 0.8, 0.0, 1.0);
      cairo_scale(cr, 0.85, 1.0);
      cairo_translate(cr, 0.05*width, 0.0); 
    }
  else
    {
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    }
  cairo_set_line_width(cr, 4.0);
  cairo_move_to(cr, 6.0*width/8.0, height/4.0);
  cairo_line_to(cr, 5.0*width/8.0, 3.0*height/4.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 7.0*width/8.0, 3.0*height/4.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.0*width/8.0, height/4.0);
  cairo_stroke_preserve(cr);

  cairo_move_to(cr, 6.0*width/8.0, 1.7*height/4.0);
  cairo_line_to(cr, 5.5*width/8.0, 2.7*height/4.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.5*width/8.0, 2.7*height/4.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.0*width/8.0, 1.7*height/4.0);
  cairo_stroke_preserve(cr);
}
static void headlight_toggle_finalize(GObject *object)
{
  G_OBJECT_CLASS(headlight_toggle_parent_class)->finalize(object);
}


















