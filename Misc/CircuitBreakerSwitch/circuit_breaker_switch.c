
/*

    For use with circuit_breaker_switch_main.c. Look in circuit_breaker_switch_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "circuit_breaker_switch.h"

#define CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), CIRCUIT_BREAKER_SWITCH_TYPE, CircuitBreakerSwitchPrivate))

typedef struct _CircuitBreakerSwitchPrivate CircuitBreakerSwitchPrivate;

struct _CircuitBreakerSwitchPrivate
{  
  //Variables for the circuit breaker switch.
  guint breaker_direction;
  gint breaker_state;
};

enum
{
  PROP_0,
  BREAKER_DIRECTION,
  BREAKER_STATE
};

//Private functions.
static void circuit_breaker_switch_class_init(CircuitBreakerSwitchClass *klass);
static void circuit_breaker_switch_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void circuit_breaker_switch_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void circuit_breaker_switch_init(CircuitBreakerSwitch *da);
static gboolean circuit_breaker_switch_draw(GtkWidget *widget, cairo_t *cr);
static void circuit_breaker_switch_horizontal_right_draw(GtkWidget *da, cairo_t *cr);
static void circuit_breaker_switch_vertical_up_draw(GtkWidget *da, cairo_t *cr);
static void circuit_breaker_switch_finalize(GObject *gobject);

G_DEFINE_TYPE(CircuitBreakerSwitch, circuit_breaker_switch, GTK_TYPE_DRAWING_AREA)

static void circuit_breaker_switch_class_init(CircuitBreakerSwitchClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=circuit_breaker_switch_set_property;
  gobject_class->get_property=circuit_breaker_switch_get_property;

  //Draw when first shown.
  widget_class->draw=circuit_breaker_switch_draw;
  gobject_class->finalize = circuit_breaker_switch_finalize;

  g_type_class_add_private(klass, sizeof(CircuitBreakerSwitchPrivate));

  g_object_class_install_property(gobject_class, BREAKER_DIRECTION, g_param_spec_int("breaker_direction", "breaker_direction", "breaker_direction", 0, 1, 0, G_PARAM_READWRITE));
}
//Needed for g_object_set().
static void circuit_breaker_switch_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  CircuitBreakerSwitch *da=CIRCUIT_BREAKER_SWITCH(object);

  switch(prop_id)
  {
    g_print("Prop %i\n", prop_id);
    case BREAKER_DIRECTION:
      circuit_breaker_switch_set_direction(da, g_value_get_int(value));
      break;
    case BREAKER_STATE:
      circuit_breaker_switch_set_state(da, g_value_get_int(value));
      break;  
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void circuit_breaker_switch_set_direction(CircuitBreakerSwitch *da, gint breaker_direction)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
 
  if(breaker_direction==0||breaker_direction==1)
    {
      priv->breaker_direction=breaker_direction; 
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("Progress direction can be CIRCUIT_HORIZONTAL_RIGHT or CIRCUIT_VERTICAL_UP only.");
    }
}
void circuit_breaker_switch_set_state(CircuitBreakerSwitch *da, gint breaker_state)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
 
  if(breaker_state>=0&&breaker_state<=3)
    { 
      priv->breaker_state=breaker_state;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The circuit state can be ON=0, STARTING=1, OFF=2 or CIRCUIT_BREAK=3.");
    }
}
static void circuit_breaker_switch_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  CircuitBreakerSwitch *da=CIRCUIT_BREAKER_SWITCH(object);
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case BREAKER_DIRECTION:
      g_value_set_int(value, priv->breaker_direction);
      break;
    case BREAKER_STATE:
      g_value_set_int(value, priv->breaker_state);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
int circuit_breaker_switch_get_direction(CircuitBreakerSwitch *da)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
  return priv->breaker_direction;
}
gint circuit_breaker_switch_get_state(CircuitBreakerSwitch *da)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
  return priv->breaker_state;
}
static void circuit_breaker_switch_init(CircuitBreakerSwitch *da)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);

  //Initailize default progress bar properties.
  priv->breaker_direction=BREAKER_HORIZONTAL_RIGHT;
  //Start in the OFF position.
  priv->breaker_state=2; 
}
GtkWidget* circuit_breaker_switch_new()
{
  return GTK_WIDGET(g_object_new(circuit_breaker_switch_get_type(), NULL));
}
static gboolean circuit_breaker_switch_draw(GtkWidget *da, cairo_t *cr)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);

  if(priv->breaker_direction==BREAKER_HORIZONTAL_RIGHT) circuit_breaker_switch_horizontal_right_draw(da, cr);
  else circuit_breaker_switch_vertical_up_draw(da, cr);
  return FALSE;
}
static void circuit_breaker_switch_horizontal_right_draw(GtkWidget *da, cairo_t *cr)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);

  //Position slider button.
  gint button_start=width/2.0;
  gint button_mid1=button_start+button_start/3.0;
  gint button_mid2=button_start+2.0*button_start/3.0;
  gint button_end=width;
  if(priv->breaker_state==2)
    {
      button_mid1=button_mid1-button_start;
      button_mid2=button_mid2-button_start;
      button_end=button_end-button_start;
      button_start=0;
    }
  if(priv->breaker_state==3)
    {
      button_mid1=button_mid1-button_start/2.0;
      button_mid2=button_mid2-button_start/2.0;
      button_end=button_end-button_start/2.0;
      button_start=width/4.0;
    }

  //Paint background.
  if(priv->breaker_state==0)
    {
      cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
      cairo_paint(cr);
    }
  if(priv->breaker_state==1)
    {
      cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
      cairo_paint(cr);
    }
  if(priv->breaker_state==2)
    {
      cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
      cairo_paint(cr);
    }
  if(priv->breaker_state==3)
    {
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
      cairo_paint(cr);
    }

  //Adjust the font size. 
  gint font_size=14;
  if(priv->breaker_state==1||priv->breaker_state==3)
    {
      if(height>width) font_size=(gint)(font_size*width/100.0);
      else font_size=(gint)(font_size*height/100.0);
    }
  else
    {
      if(height>width) font_size=(gint)(font_size*width/50.0);
      else font_size=(gint)(font_size*height/50.0);
    }
  
  //Draw the text.
  cairo_text_extents_t extents1;
  cairo_text_extents_t extents2;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, font_size);
  if(priv->breaker_state==0)
    {
      cairo_text_extents(cr, "ON", &extents1);
      cairo_move_to(cr, width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
      cairo_show_text(cr, "ON");
    }
  else if(priv->breaker_state==1)
    {
      cairo_text_extents(cr, "STARTING", &extents1);
      cairo_move_to(cr, width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
      cairo_show_text(cr, "STARTING");
    }
  else if(priv->breaker_state==2)
    {
      cairo_text_extents(cr, "OFF", &extents1);
      cairo_move_to(cr, 3.0*width/4.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);   
      cairo_show_text(cr, "OFF");
    }
  else
    {
      cairo_text_extents(cr, "BREAK", &extents1);
      cairo_move_to(cr, width/8.0 - extents1.width/2.0, height/2.0 + extents1.height/2.0);  
      cairo_show_text(cr, "BREAK");
      cairo_text_extents(cr, "CONTACT", &extents2);
      cairo_move_to(cr, 7.0*width/8.0 - extents2.width/2.0, height/2.0 + extents2.height/2.0);
      cairo_show_text(cr, "CONTACT");  
    }  
    
  //The left button gradient and rectangle.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(button_start, 0.0, button_mid1, 0.0);  
  cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.6, 0.6, 0.6, 1.0); 
  cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 0.0, 0.0, 1.0); 
  cairo_set_source(cr, pattern1);    
  cairo_rectangle(cr, button_start, 0, button_mid1, height);
  cairo_fill(cr);

  //The middle of the button.
  cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 1.0);
  cairo_rectangle(cr, button_mid1, 0, button_mid2-button_mid1, height);
  cairo_fill(cr);

  //The right button gradient and rectangle.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(button_mid2, 0.0, button_end, 0.0);
  cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0); 
  cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.6, 0.6, 0.6, 1.0); 
  cairo_set_source(cr, pattern2);     
  cairo_rectangle(cr, button_mid2, 0, button_end-button_mid2, height);
  cairo_fill(cr);

  //Draw black rectangle for the edge.
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_set_line_width(cr, 3.0);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke(cr);

  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
}
static void circuit_breaker_switch_vertical_up_draw(GtkWidget *da, cairo_t *cr)
{
  CircuitBreakerSwitchPrivate *priv=CIRCUIT_BREAKER_SWITCH_GET_PRIVATE(da);
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);
    
  //Position slider button.
  gint button_start=height/2.0;
  gint button_mid1=button_start+button_start/3.0;
  gint button_mid2=button_start+2.0*button_start/3.0;
  gint button_end=height;
  if(priv->breaker_state==2)
    {
      button_mid1=button_mid1-button_start;
      button_mid2=button_mid2-button_start;
      button_end=button_end-button_start;
      button_start=0;
    }
  if(priv->breaker_state==3)
    {
      button_mid1=button_mid1-button_start/2.0;
      button_mid2=button_mid2-button_start/2.0;
      button_end=button_end-button_start/2.0;
      button_start=height/4.0;
    }

  //Paint background.
  if(priv->breaker_state==0)
    {
      cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
      cairo_paint(cr);
    }
  if(priv->breaker_state==1)
    {
      cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
      cairo_paint(cr);
    }
  if(priv->breaker_state==2)
    {
      cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
      cairo_paint(cr);
    }
  if(priv->breaker_state==3)
    {
      cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
      cairo_paint(cr);
    }

  //Adjust the font size. 
  gint font_size=14;
  if(priv->breaker_state==1||priv->breaker_state==3)
    {
      if(height<width) font_size=(gint)(font_size*height/100.0);
      else font_size=(gint)(font_size*width/100.0);
    }
  else
    {
      if(height<width) font_size=(gint)(font_size*height/50.0);
      else font_size=(gint)(font_size*width/50.0);
    }
  
  //Draw the text.
  cairo_text_extents_t extents1;
  cairo_text_extents_t extents2;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, font_size);
  if(priv->breaker_state==0)
    {
      cairo_text_extents(cr, "ON", &extents1);
      cairo_move_to(cr, width/2.0 - extents1.width/2.0, height/4.0 + extents1.height/2.0);  
      cairo_show_text(cr, "ON");
    }
  else if(priv->breaker_state==1)
    {
      cairo_text_extents(cr, "STARTING", &extents1);
      cairo_move_to(cr, width/2.0 - extents1.width/2.0, height/4.0 + extents1.height/2.0);  
      cairo_show_text(cr, "STARTING");
    }
  else if(priv->breaker_state==2)
    {
      cairo_text_extents(cr, "OFF", &extents1);
      cairo_move_to(cr, width/2.0 - extents1.width/2.0, 3.0*height/4.0 + extents1.height/2.0);   
      cairo_show_text(cr, "OFF");
    }
  else
    {
      cairo_text_extents(cr, "BREAK", &extents1);
      cairo_move_to(cr, width/2.0 - extents1.width/2.0, height/8.0 + extents1.height/2.0);  
      cairo_show_text(cr, "BREAK");
      cairo_text_extents(cr, "CONTACT", &extents2);
      cairo_move_to(cr, width/2.0 - extents2.width/2.0, 7.0*height/8.0 + extents2.height/2.0);
      cairo_show_text(cr, "CONTACT");  
    }  
    
  //The left button gradient and rectangle.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, button_start, 0.0, button_mid1);  
  cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.6, 0.6, 0.6, 1.0); 
  cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 0.0, 0.0, 1.0); 
  cairo_set_source(cr, pattern1);    
  cairo_rectangle(cr, 0, button_start, width, button_mid1);
  cairo_stroke_preserve(cr);
  cairo_fill(cr);

  //The middle of the button.
  cairo_set_source_rgba(cr, 0.6, 0.6, 0.6, 1.0);
  cairo_rectangle(cr, 0, button_mid1, width, button_mid2-button_mid1);
  cairo_fill(cr);

  //The right button gradient and rectangle.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, button_mid2, 0.0, button_end);
  cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0); 
  cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.6, 0.6, 0.6, 1.0); 
  cairo_set_source(cr, pattern2);     
  cairo_rectangle(cr, 0, button_mid2, width, button_end-button_mid2);
  cairo_fill(cr);

  //Draw black rectangle for the edge.
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_set_line_width(cr, 3.0);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke(cr);

  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
}
static void circuit_breaker_switch_finalize(GObject *object)
{
  G_OBJECT_CLASS(circuit_breaker_switch_parent_class)->finalize(object);
}


















