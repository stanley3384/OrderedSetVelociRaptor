
//For use with drawing_main.c. Look in drawing_main.c for more information.

#include<gtk/gtk.h>
#include "drawing.h"

#define SMILEY_DRAWING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), SMILEY_DRAWING_TYPE, SmileyDrawingPrivate))

typedef struct _SmileyDrawingPrivate SmileyDrawingPrivate;

struct _SmileyDrawingPrivate
{
  gdouble color_rgba[4];
};

enum
{
  PROP_0,
  RED,
  GREEN,
  BLUE,
  ALPHA
};

//Private functions.
static void smiley_drawing_class_init(SmileyDrawingClass *klass);
static void smiley_drawing_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void smiley_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void smiley_drawing_init(SmileyDrawing *da);
static gboolean smiley_drawing_draw(GtkWidget *widget, cairo_t *cr);

GType smiley_drawing_get_type(void)
{
  static GType entry_type=0;
  if(!entry_type)
    {
      static const GTypeInfo entry_info={sizeof(SmileyDrawingClass), NULL, NULL, (GClassInitFunc) smiley_drawing_class_init, NULL, NULL, sizeof(SmileyDrawing), 0, (GInstanceInitFunc)smiley_drawing_init};
      entry_type=g_type_register_static(GTK_TYPE_DRAWING_AREA, "SmileyDrawing", &entry_info, 0);
    }

  return entry_type;
}
static void smiley_drawing_class_init(SmileyDrawingClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=smiley_drawing_set_property;
  gobject_class->get_property=smiley_drawing_get_property;

  //Draw when first shown.
  widget_class->draw=smiley_drawing_draw;

  g_type_class_add_private(klass, sizeof(SmileyDrawingPrivate));

  g_object_class_install_property(gobject_class, RED, g_param_spec_double("Red", "Red Channel", "Channel 1", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, GREEN, g_param_spec_double("Green", "Green Channel", "Channel 2", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, BLUE, g_param_spec_double("Blue", "Blue Channel", "Channel 3", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, ALPHA, g_param_spec_double("Alpha", "Alpha Channel", "Channel 4", 0, 1, 0, G_PARAM_READWRITE));

}
//Needed for g_object_set().
static void smiley_drawing_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  SmileyDrawing *da=SMILEY_DRAWING(object);
  gdouble color_rgba[4]={-1.0, -1.0, -1.0, -1.0};

  switch(prop_id)
  {
    case RED:
      color_rgba[0]=g_value_get_double(value);
      smiley_drawing_set_color(da, color_rgba);
      break;
    case GREEN:
      color_rgba[1]=g_value_get_double(value);
      smiley_drawing_set_color(da, color_rgba);
      break;
    case BLUE:
      color_rgba[2]=g_value_get_double(value);
      smiley_drawing_set_color(da, color_rgba);
      break;  
    case ALPHA:
      color_rgba[3]=g_value_get_double(value);
      smiley_drawing_set_color(da, color_rgba);
      break; 
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void smiley_drawing_set_color(SmileyDrawing *da, gdouble color_rgba[4])
{
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);
  gint i=0;
  
  for(i=0;i<4;i++)
  {
    if(color_rgba[i]>=0.0&&color_rgba[i]<=1.0)
      {
        priv->color_rgba[i]=color_rgba[i];
      }
  }

  gtk_widget_queue_draw(GTK_WIDGET(da));
}
//Needed for g_object_set(). 
static void smiley_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  SmileyDrawing *da=SMILEY_DRAWING(object);
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case RED:
      g_value_set_double(value, priv->color_rgba[0]);
      break;
    case GREEN:
      g_value_set_double(value, priv->color_rgba[1]);
      break;
    case BLUE:
      g_value_set_double(value, priv->color_rgba[2]);
      break;
    case ALPHA:
      g_value_set_double(value, priv->color_rgba[3]);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
gchar* smiley_drawing_get_color(SmileyDrawing *da)
{
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);
  return g_strdup_printf("%f, %f, %f, %f", priv->color_rgba[0], priv->color_rgba[1], priv->color_rgba[2], priv->color_rgba[3]);
}
static void smiley_drawing_init(SmileyDrawing *da)
{
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);
  gint i=0;

  //Initialize color array.
  for(i=0;i<4;i++) priv->color_rgba[i]=0.0;
}
GtkWidget* smiley_drawing_new(void)
{
  return GTK_WIDGET(g_object_new(smiley_drawing_get_type(), NULL));
}
static gboolean smiley_drawing_draw(GtkWidget *widget, cairo_t *cr)
{
  gint width=gtk_widget_get_allocated_width(widget);
  gint height=gtk_widget_get_allocated_height(widget);
  gint center_x=width/2;
  gint center_y=height/2;
  SmileyDrawing *da=SMILEY_DRAWING(widget);
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);

  //Draw blue if no color is chosen at the start.
  if(priv->color_rgba[0]==0&&priv->color_rgba[1]==0&&priv->color_rgba[0]==0)
    {
      priv->color_rgba[0]=0.0;
      priv->color_rgba[0]=0.0;
      priv->color_rgba[2]=1.0;
      priv->color_rgba[3]=1.0;
    }

  //Smiley
  cairo_set_source_rgba(cr, priv->color_rgba[0], priv->color_rgba[1], priv->color_rgba[2], priv->color_rgba[3]);
  cairo_set_line_width(cr, 10);
  //Circle
  cairo_arc(cr, center_x, center_y, 100, 0, 2*G_PI);
  cairo_stroke(cr);
  //Left eye.
  cairo_arc(cr, center_x-35, center_y-38, 7, 0, 2*G_PI);
  cairo_fill(cr);
  cairo_stroke(cr);
  //Right eye.
  cairo_arc(cr, center_x+35, center_y-38, 7, 0, 2*G_PI);
  cairo_fill(cr);
  cairo_stroke(cr);
  //Smile
  cairo_arc(cr, center_x, center_y+4, 60, 0, G_PI);
  cairo_stroke(cr);

  return FALSE;
}

