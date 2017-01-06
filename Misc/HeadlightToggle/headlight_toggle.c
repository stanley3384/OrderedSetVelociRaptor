
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
  gint headlight_toggle_direction;
  gint headlight_toggle_icon;
  //Some drawing colors.
  gchar *dim_color_string;
  gchar *lit_color_string;
  gchar *icon_dim_color_string;
  gchar *icon_lit_color_string;
  gdouble dim_color[4];
  gdouble lit_color[4];
  gdouble icon_dim_color[4];
  gdouble icon_lit_color[4];
};

enum
{
  PROP_0,
  HEADLIGHT_TOGGLE_STATE,
  HEADLIGHT_TOGGLE_DIRECTION,
  HEADLIGHT_TOGGLE_ICON,
  HEADLIGHT_TOGGLE_DIM_COLOR,
  HEADLIGHT_TOGGLE_LIT_COLOR,
  HEADLIGHT_TOGGLE_ICON_DIM_COLOR,
  HEADLIGHT_TOGGLE_ICON_LIT_COLOR
};

//Private functions.
static void headlight_toggle_class_init(HeadlightToggleClass *klass);
static void headlight_toggle_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void headlight_toggle_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void headlight_toggle_init(HeadlightToggle *da);
static gboolean headlight_toggle_draw(GtkWidget *widget, cairo_t *cr);
static void headlight_toggle_horizontal_right_draw(GtkWidget *da, cairo_t *cr);
static void headlight_toggle_vertical_up_draw(GtkWidget *da, cairo_t *cr);
static void headlight_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction);
static void emergency_light_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction);
static void fan_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction);
static void heater_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction);
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

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_DIRECTION, g_param_spec_int("headlight_toggle_direction", "headlight_toggle_direction", "headlight_toggle_direction", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_ICON, g_param_spec_int("headlight_toggle_icon", "headlight_toggle_icon", "headlight_toggle_icon", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_DIM_COLOR, g_param_spec_string("dim_color", "dim_color", "dim_color", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_LIT_COLOR, g_param_spec_string("lit_color", "lit_color", "lit_color", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_ICON_DIM_COLOR, g_param_spec_string("icon_dim_color", "icon_dim_color", "icon_dim_color", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, HEADLIGHT_TOGGLE_ICON_LIT_COLOR, g_param_spec_string("icon_lit_color", "icon_lit_color", "icon_lit_color", NULL, G_PARAM_READWRITE));

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
    case HEADLIGHT_TOGGLE_DIRECTION:
      headlight_toggle_set_direction(da, g_value_get_int(value));
      break;  
    case HEADLIGHT_TOGGLE_ICON:
      headlight_toggle_set_icon(da, g_value_get_int(value));
      break;
    case HEADLIGHT_TOGGLE_DIM_COLOR:
      headlight_toggle_set_dim_color(da, g_value_get_string(value));
      break;
    case HEADLIGHT_TOGGLE_LIT_COLOR:
      headlight_toggle_set_lit_color(da, g_value_get_string(value));
      break;
    case HEADLIGHT_TOGGLE_ICON_DIM_COLOR:
      headlight_toggle_set_icon_dim_color(da, g_value_get_string(value));
      break; 
    case HEADLIGHT_TOGGLE_ICON_LIT_COLOR:
      headlight_toggle_set_icon_lit_color(da, g_value_get_string(value));
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
      g_warning("The headlight toggle state can be HEADLIGHT_TOGGLE_OFF=0 or HEADLIGHT_TOGGLE_ON=1");
    }
}
void headlight_toggle_set_direction(HeadlightToggle *da, gint headlight_toggle_direction)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
 
  if(headlight_toggle_direction==0||headlight_toggle_direction==1)
    { 
      priv->headlight_toggle_direction=headlight_toggle_direction;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The headlight toggle direction can be HORIZONTAL_RIGHT=0 or VERTICAL_UP=1");
    }
}
void headlight_toggle_set_icon(HeadlightToggle *da, gint headlight_toggle_icon)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
 
  if(headlight_toggle_icon>=0&&headlight_toggle_icon<=3)
    { 
      priv->headlight_toggle_icon=headlight_toggle_icon;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The headlight icon can be HEADLIGHT_ICON=0, EMERGENCY_ICON=1, FAN_ICON=2 or HEATER_ICON=3.");
    }
}
void headlight_toggle_set_dim_color(HeadlightToggle *da, const gchar *dim_color_string)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, dim_color_string))
    {
      priv->dim_color[0]=rgba.red;
      priv->dim_color[1]=rgba.green;
      priv->dim_color[2]=rgba.blue;
      priv->dim_color[3]=rgba.alpha;
      if(priv->dim_color_string!=NULL) g_free(priv->dim_color_string);
      priv->dim_color_string=g_strdup(dim_color_string); 
    }
  else
    {
      g_warning("dim_color_string error\n");
    } 
}
void headlight_toggle_set_lit_color(HeadlightToggle *da, const gchar *lit_color_string)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, lit_color_string))
    {
      priv->lit_color[0]=rgba.red;
      priv->lit_color[1]=rgba.green;
      priv->lit_color[2]=rgba.blue;
      priv->lit_color[3]=rgba.alpha;
      if(priv->lit_color_string!=NULL) g_free(priv->lit_color_string);
      priv->lit_color_string=g_strdup(lit_color_string); 
    }
  else
    {
      g_warning("lit_color_string error\n");
    } 
} 
void headlight_toggle_set_icon_dim_color(HeadlightToggle *da, const gchar *icon_dim_color_string)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, icon_dim_color_string))
    {
      priv->icon_dim_color[0]=rgba.red;
      priv->icon_dim_color[1]=rgba.green;
      priv->icon_dim_color[2]=rgba.blue;
      priv->icon_dim_color[3]=rgba.alpha;
      if(priv->icon_dim_color_string!=NULL) g_free(priv->icon_dim_color_string);
      priv->icon_dim_color_string=g_strdup(icon_dim_color_string); 
    }
  else
    {
      g_warning("icon_dim_color_string error\n");
    } 
}
void headlight_toggle_set_icon_lit_color(HeadlightToggle *da, const gchar *icon_lit_color_string)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, icon_lit_color_string))
    {
      priv->icon_lit_color[0]=rgba.red;
      priv->icon_lit_color[1]=rgba.green;
      priv->icon_lit_color[2]=rgba.blue;
      priv->icon_lit_color[3]=rgba.alpha;
      if(priv->icon_lit_color_string!=NULL) g_free(priv->icon_lit_color_string);
      priv->icon_lit_color_string=g_strdup(icon_lit_color_string); 
    }
  else
    {
      g_warning("icon_lit_color_string error\n");
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
    case HEADLIGHT_TOGGLE_DIRECTION:
      g_value_set_int(value, priv->headlight_toggle_direction);
      break;
    case HEADLIGHT_TOGGLE_ICON:
      g_value_set_int(value, priv->headlight_toggle_icon);
      break;
    case HEADLIGHT_TOGGLE_DIM_COLOR:
      g_value_set_string(value, priv->dim_color_string);
      break;
    case HEADLIGHT_TOGGLE_LIT_COLOR:
      g_value_set_string(value, priv->lit_color_string);
      break;
    case HEADLIGHT_TOGGLE_ICON_DIM_COLOR:
      g_value_set_string(value, priv->icon_dim_color_string);
      break;
    case HEADLIGHT_TOGGLE_ICON_LIT_COLOR:
      g_value_set_string(value, priv->icon_lit_color_string);
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
gint headlight_toggle_get_direction(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->headlight_toggle_direction;
}
gint headlight_toggle_get_icon(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->headlight_toggle_icon;
}
const gchar* headlight_toggle_get_dim_color(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->dim_color_string;
}
const gchar* headlight_toggle_get_lit_color(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->lit_color_string;
}
const gchar* headlight_toggle_get_icon_dim_color(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->icon_dim_color_string;
}
const gchar* headlight_toggle_get_icon_lit_color(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  return priv->icon_lit_color_string;
}
static void headlight_toggle_init(HeadlightToggle *da)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  //Start in the OFF position.
  priv->headlight_toggle_state=HEADLIGHT_TOGGLE_OFF; 
  //Default drawing direction.
  priv->headlight_toggle_state=HORIZONTAL_RIGHT; 
  //Default icon drawing to headlight.
  priv->headlight_toggle_icon=HEADLIGHT_ICON; 

  //Set some initial colors.
  priv->dim_color[0]=0.8;
  priv->dim_color[1]=0.0;
  priv->dim_color[2]=0.0;
  priv->dim_color[3]=1.0;
  priv->lit_color[0]=1.0;
  priv->lit_color[1]=0.0;
  priv->lit_color[2]=0.0;
  priv->lit_color[3]=1.0;
  priv->icon_dim_color[0]=0.8;
  priv->icon_dim_color[1]=0.8;
  priv->icon_dim_color[2]=0.0;
  priv->icon_dim_color[3]=1.0;
  priv->icon_lit_color[0]=1.0;
  priv->icon_lit_color[1]=1.0;
  priv->icon_lit_color[2]=0.0;
  priv->icon_lit_color[3]=1.0;
}
GtkWidget* headlight_toggle_new()
{
  return GTK_WIDGET(g_object_new(headlight_toggle_get_type(), NULL));
}
static gboolean headlight_toggle_draw(GtkWidget *da, cairo_t *cr)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  if(priv->headlight_toggle_direction==HORIZONTAL_RIGHT) 
    {
      headlight_toggle_horizontal_right_draw(da, cr);
    }
  else
    {
      headlight_toggle_vertical_up_draw(da, cr);
    }
  
  return FALSE;
}
static void headlight_toggle_horizontal_right_draw(GtkWidget *da, cairo_t *cr)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  gdouble width=gtk_widget_get_allocated_width(da);
  gdouble height=gtk_widget_get_allocated_height(da);

  //Position toggle gradients.
  gdouble button_start=0.0;
  gdouble button_left=width/8.0;
  gdouble button_middle=width/2.0;
  gdouble button_right=7.0*width/8.0;

  //Paint background.    
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_paint(cr);
     
  //The far left button gradient and rectangle.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(button_start, 0.0, button_left, 0.0);
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_ON)
    {  
      cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.6, 0.6, 0.6, 1.0); 
      cairo_set_source(cr, pattern1);    
      cairo_rectangle(cr, button_start, 0.0, button_left, height);
      cairo_fill(cr);
    }

  //The left gradient and rectangle.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(button_left, 0.0, button_middle, 0.0);
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_ON)
    {
      cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.4, 0.4, 0.4, 1.0); 
      cairo_set_source(cr, pattern2);     
      cairo_rectangle(cr, button_left, 0, button_middle-button_left, height);
      cairo_fill(cr);
    }

  //The button is off. Draw left part a solid color.
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
      cairo_rectangle(cr, button_start, 0, button_middle, height);
      cairo_fill(cr);
    }

  //The lit part of the switch.
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF) cairo_set_source_rgba(cr, priv->dim_color[0], priv->dim_color[1], priv->dim_color[2], priv->dim_color[3]);
  else cairo_set_source_rgba(cr, priv->lit_color[0], priv->lit_color[1], priv->lit_color[2], priv->lit_color[3]);
  cairo_rectangle(cr, button_middle, 0, width-button_middle, height);
  cairo_fill(cr);

  //The right gradient.
  cairo_pattern_t *pattern3=cairo_pattern_create_linear(button_right, 0.0, width, 0.0);
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_pattern_add_color_stop_rgba(pattern3, 0.0, priv->lit_color[0], priv->lit_color[1], priv->lit_color[2], priv->lit_color[3]); 
      cairo_pattern_add_color_stop_rgba(pattern3, 1.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_set_source(cr, pattern3);    
      cairo_rectangle(cr, button_right, 0, width-button_right, height);
      cairo_fill(cr);
    }

  //Draw end line "gaps" indented 8 from outside rectangle.
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      cairo_set_line_width(cr, 16.0);
      cairo_move_to(cr, 0.0, 0.0);
      cairo_line_to(cr, 0.0, height);
      cairo_stroke(cr);
    }

  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_ON)
    {
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      cairo_set_line_width(cr, 16.0);
      cairo_move_to(cr, width, 0.0);
      cairo_line_to(cr, width, height);
      cairo_stroke(cr);
    }

  //The outside rectangle.
  cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
  cairo_set_line_width(cr, 8.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  //The headlight drawing.
  if(priv->headlight_toggle_icon==HEADLIGHT_ICON)
    {
      headlight_icon_drawing(da, cr, width, height, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  else if(priv->headlight_toggle_icon==EMERGENCY_LIGHT_ICON)
    {
      emergency_light_icon_drawing(da, cr, width, height, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  else if(priv->headlight_toggle_icon==FAN_ICON)
    {
      fan_icon_drawing(da, cr, width, height, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  else
    {
      heater_icon_drawing(da, cr, width, height, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  
  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
  cairo_pattern_destroy(pattern3);
}
static void headlight_toggle_vertical_up_draw(GtkWidget *da, cairo_t *cr)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
  gdouble width=gtk_widget_get_allocated_width(da);
  gdouble height=gtk_widget_get_allocated_height(da);

  //Position toggle gradients from top down.
  gdouble button_top=height/8.0;
  gdouble button_middle=height/2.0;
  gdouble button_bottom=7.0*height/8.0;

  //Paint background.    
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_paint(cr);
     
  //The bottom part of the button gradient and rectangle.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, button_bottom, 0.0, height);
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_ON)
    { 
      cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 0.6, 0.6, 0.6, 1.0);  
      cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_set_source(cr, pattern1);    
      cairo_rectangle(cr, 0.0, button_bottom, width, height);
      cairo_fill(cr);
    }
 
  //The bottom to middle gradient and rectangle.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, button_middle, 0.0, button_bottom);
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_ON)
    {
      cairo_pattern_add_color_stop_rgba(pattern2, 0.0, 0.4, 0.4, 0.4, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern2, 1.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_set_source(cr, pattern2); 
      cairo_rectangle(cr, 0.0, button_middle, width, button_bottom-button_middle);
      cairo_fill(cr);
    }
  
  //The button is off. Draw bottom part a solid color.
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, 0.3, 0.3, 0.3, 1.0);
      cairo_rectangle(cr, 0.0, button_middle, width, height);
      cairo_fill(cr);
    }

  //The lit top part of the switch.
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF) cairo_set_source_rgba(cr, priv->dim_color[0], priv->dim_color[1], priv->dim_color[2], priv->dim_color[3]);
  else cairo_set_source_rgba(cr, priv->lit_color[0], priv->lit_color[1], priv->lit_color[2], priv->lit_color[3]);
  cairo_rectangle(cr, 0.0, 0.0, width, button_middle);
  cairo_fill(cr);
  
  //The top gradient.
  cairo_pattern_t *pattern3=cairo_pattern_create_linear(0.0, 0.0, 0.0, button_top);
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_pattern_add_color_stop_rgba(pattern3, 0.0, 0.0, 0.0, 0.0, 1.0); 
      cairo_pattern_add_color_stop_rgba(pattern3, 1.0, priv->lit_color[0], priv->lit_color[1], priv->lit_color[2], priv->lit_color[3]); 
      cairo_set_source(cr, pattern3);    
      cairo_rectangle(cr, 0.0, 0.0, width, button_top);
      cairo_fill(cr);
    }
  
  //Draw end line "gaps" indented 8 from outside rectangle.
  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      cairo_set_line_width(cr, 16.0);
      cairo_move_to(cr, 0.0, height);
      cairo_line_to(cr, width, height);
      cairo_stroke(cr);
    }

  if(priv->headlight_toggle_state==HEADLIGHT_TOGGLE_ON)
    {
      cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
      cairo_set_line_width(cr, 16.0);
      cairo_move_to(cr, 0.0, 0.0);
      cairo_line_to(cr, width, 0.0);
      cairo_stroke(cr);
    }

  //The outside rectangle.
  cairo_set_source_rgba(cr, 0.8, 0.8, 0.8, 1.0);
  cairo_set_line_width(cr, 8.0);
  cairo_rectangle(cr, 0.0, 0.0, width, height);
  cairo_stroke(cr);

  /*
    Draw off screen and then translate the drawing back. Scale if needed. It isn't perfect
    but the drawing functions don't need to be re-written for the vertical drawing. Also there
    are some problems with adding small translations to individual drawings. 
  */
  gdouble w2=2.0*width;
  gdouble h2=width;
  gdouble ratio=w2/height;
  if(0.5*height<h2)
    {
      cairo_scale(cr, 0.5*height/h2, 0.5*height/h2);
      cairo_translate(cr, -w2/(2.0*ratio), (0.5*height/h2)*height/16.0);
    }
  else cairo_translate(cr, (-w2/2.0)+((1.0/32.0)*w2), (0.5*height/h2)*height/16.0);

  if(priv->headlight_toggle_icon==HEADLIGHT_ICON)
    {
      headlight_icon_drawing(da, cr, w2, h2, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  else if(priv->headlight_toggle_icon==EMERGENCY_LIGHT_ICON)
    {
      emergency_light_icon_drawing(da, cr, w2, h2, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  else if(priv->headlight_toggle_icon==FAN_ICON)
    {
      fan_icon_drawing(da, cr, w2, h2, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  else
    {
      heater_icon_drawing(da, cr, w2, h2, priv->headlight_toggle_state, priv->headlight_toggle_direction);
    }
  
  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
  cairo_pattern_destroy(pattern3);
  
}
static void headlight_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);
 
  if(headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, priv->icon_dim_color[0], priv->icon_dim_color[1], priv->icon_dim_color[2], priv->icon_dim_color[3]);
      if(headlight_toggle_direction==HORIZONTAL_RIGHT)
        {
          //Scale based on end gradient shift of drawing. Translate 0.5 of 1/8.
          cairo_scale(cr, 7.0/8.0, 1.0);
          cairo_translate(cr, (1.0/16.0)*width, 0.0);
        }
      else
        {
          cairo_scale(cr, 1.0, 7.0/8.0);
          cairo_translate(cr, 0.0, (1.0/16.0)*height);
        } 
    }
  else
    {
      cairo_set_source_rgba(cr, priv->icon_lit_color[0], priv->icon_lit_color[1], priv->icon_lit_color[2], priv->icon_lit_color[3]);
      if(headlight_toggle_direction==VERTICAL_UP)
        {
          cairo_translate(cr, 0.0, -(1.0/16.0)*height);
        }
    }

  cairo_set_line_width(cr, 4.0);
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
static void emergency_light_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  if(headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, priv->icon_dim_color[0], priv->icon_dim_color[1], priv->icon_dim_color[2], priv->icon_dim_color[3]);
      if(headlight_toggle_direction==HORIZONTAL_RIGHT)
        {
          //Scale based on end gradient shift of drawing. Translate 0.5 of 1/8.
          cairo_scale(cr, 7.0/8.0, 1.0);
          cairo_translate(cr, (1.0/16.0)*width, 0.0);
        }
      else
        {
          cairo_scale(cr, 1.0, 7.0/8.0);
          cairo_translate(cr, -(1.0/32.0)*width, (1.0/16.0)*height);
        } 
    }
  else
    {
      cairo_set_source_rgba(cr, priv->icon_lit_color[0], priv->icon_lit_color[1], priv->icon_lit_color[2], priv->icon_lit_color[3]);
      if(headlight_toggle_direction==VERTICAL_UP)
        {
          cairo_translate(cr, -(1.0/32.0)*width, -(1.0/16.0)*height);
        }
    }

  cairo_set_line_width(cr, 4.0);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
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
static void fan_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  if(headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, priv->icon_dim_color[0], priv->icon_dim_color[1], priv->icon_dim_color[2], priv->icon_dim_color[3]);
      if(headlight_toggle_direction==HORIZONTAL_RIGHT)
        {
          //Scale based on end gradient shift of drawing. Translate 0.5 of 1/8.
          cairo_scale(cr, 7.0/8.0, 1.0);
          cairo_translate(cr, (1.0/16.0)*width, 0.0);
        }
      else
        {
          cairo_scale(cr, 1.0, 7.0/8.0);
          cairo_translate(cr, -(1.0/32.0)*width, (1.0/12.0)*height);
        } 
    }
  else
    {
      cairo_set_source_rgba(cr, priv->icon_lit_color[0], priv->icon_lit_color[1], priv->icon_lit_color[2], priv->icon_lit_color[3]);
      if(headlight_toggle_direction==VERTICAL_UP)
        {
          cairo_translate(cr, -(1.0/32.0)*width, -(1.0/16.0)*height);
        }
    }

  cairo_set_line_width(cr, 4.0);
  cairo_scale(cr, 0.8, 0.8);
  cairo_translate(cr, 0.18*width, 0.1*height);

  //Fan blades.
  cairo_move_to(cr, 6.25*width/8.0, 3.50*height/8.0);
  cairo_curve_to(cr, 8.5*width/8.0, 0.0, 4.0*width/8.0, 0.0, 5.75*width/8.0, 3.50*height/8.0);
  cairo_stroke_preserve(cr);

  cairo_curve_to(cr, 4.0*width/8.0, -1.0*height/8.0, 4.0*width/8.0, 8.0*height/8.0, 5.75*width/8.0, 4.50*height/8.0);
  cairo_stroke_preserve(cr);

  cairo_curve_to(cr, 3.5*width/8.0, 8.0*height/8.0, 8.0*width/8.0, 8.0*height/8.0, 6.25*width/8.0, 4.50*height/8.0);
  cairo_stroke_preserve(cr);

  cairo_curve_to(cr, 8.0*width/8.0, 9.0*height/8.0, 8.0*width/8.0, 0.0, 6.25*width/8.0, 3.50*height/8.0);
  cairo_stroke(cr);

  cairo_arc(cr, 6.0*width/8.0, height/2.0, height/16.0, 0.0, 2*G_PI);
  cairo_stroke(cr);

}
static void heater_icon_drawing(GtkWidget *da, cairo_t *cr, gdouble width, gdouble height, gint headlight_toggle_state, gint headlight_toggle_direction)
{
  HeadlightTogglePrivate *priv=HEADLIGHT_TOGGLE_GET_PRIVATE(da);

  if(headlight_toggle_state==HEADLIGHT_TOGGLE_OFF)
    {
      cairo_set_source_rgba(cr, priv->icon_dim_color[0], priv->icon_dim_color[1], priv->icon_dim_color[2], priv->icon_dim_color[3]);
      if(headlight_toggle_direction==HORIZONTAL_RIGHT)
        {
          //Scale based on end gradient shift of drawing. Translate 0.5 of 1/8.
          cairo_scale(cr, 7.0/8.0, 1.0);
          cairo_translate(cr, (1.0/16.0)*width, 0.0);
        }
      else
        {
          cairo_scale(cr, 1.0, 7.0/8.0);
          cairo_translate(cr, 0.0, (1.0/16.0)*height);
        } 
    }
  else
    {
      cairo_set_source_rgba(cr, priv->icon_lit_color[0], priv->icon_lit_color[1], priv->icon_lit_color[2], priv->icon_lit_color[3]);
      if(headlight_toggle_direction==VERTICAL_UP)
        {
          cairo_translate(cr, 0.0, -(1.0/16.0)*height);
        }
    }

  //First line
  cairo_set_line_width(cr, 4.0);
  cairo_move_to(cr, 4.25*width/8.0, 2.25*height/8.0);
  cairo_curve_to(cr, 5.75*width/8.0, 4.25*height/8.0, 5.75*width/8.0, 1.25*height/8.0, 6.75*width/8.0, 2.25*height/8.0);
  cairo_stroke(cr);
  //Arrow
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 6.75*width/8.0, 2.25*height/8.0);
  cairo_line_to(cr, 6.75*width/8.0, 1.75*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 7.25*width/8.0, 2.25*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.75*width/8.0, 2.75*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.75*width/8.0, 2.25*height/8.0);
  cairo_fill(cr);

  //Second line
  cairo_set_line_width(cr, 4.0);
  cairo_move_to(cr, 4.25*width/8.0, 4.0*height/8.0);
  cairo_curve_to(cr, 5.75*width/8.0, 6.0*height/8.0, 5.75*width/8.0, 3.0*height/8.0, 6.75*width/8.0, 4.0*height/8.0);
  cairo_stroke(cr);
  //Arrow
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 6.75*width/8.0, 4.0*height/8.0);
  cairo_line_to(cr, 6.75*width/8.0, 3.5*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 7.25*width/8.0, 4.0*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.75*width/8.0, 4.5*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.75*width/8.0, 4.0*height/8.0);
  cairo_fill(cr);

  //Third line
  cairo_set_line_width(cr, 4.0);
  cairo_move_to(cr, 4.25*width/8.0, 5.75*height/8.0);
  cairo_curve_to(cr, 5.75*width/8.0, 7.75*height/8.0, 5.75*width/8.0, 4.75*height/8.0, 6.75*width/8.0, 5.75*height/8.0);
  cairo_stroke(cr);
  //Arrow
  cairo_set_line_width(cr, 1.0);
  cairo_move_to(cr, 6.75*width/8.0, 5.75*height/8.0);
  cairo_line_to(cr, 6.75*width/8.0, 5.25*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 7.25*width/8.0, 5.75*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.75*width/8.0, 6.25*height/8.0);
  cairo_stroke_preserve(cr);
  cairo_line_to(cr, 6.75*width/8.0, 5.75*height/8.0);
  cairo_fill(cr);
}
static void headlight_toggle_finalize(GObject *object)
{
  G_OBJECT_CLASS(headlight_toggle_parent_class)->finalize(object);
}


















