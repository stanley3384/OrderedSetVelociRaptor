
/*

    For use with drawing_main.c. Look in drawing_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "drawing.h"

#define SMILEY_DRAWING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), SMILEY_DRAWING_TYPE, SmileyDrawingPrivate))

typedef struct _SmileyDrawingPrivate SmileyDrawingPrivate;

struct _SmileyDrawingPrivate
{
  gchar *smiley_name;
  gdouble color_rgba[4];
};

enum
{
  PROP_0,
  NAME,
  RED,
  GREEN,
  BLUE,
  ALPHA
};

enum
{
  CHANGED_SIGNAL,
  LAST_SIGNAL
};

static guint smiley_drawing_signals[LAST_SIGNAL]={0};

//Private functions.
static void smiley_drawing_class_init(SmileyDrawingClass *klass);
static void smiley_drawing_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void smiley_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void smiley_drawing_init(SmileyDrawing *da);
static gboolean smiley_drawing_draw(GtkWidget *widget, cairo_t *cr);
static void smiley_drawing_finalize(GObject *gobject);

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
  gobject_class->finalize = smiley_drawing_finalize;

  g_type_class_add_private(klass, sizeof(SmileyDrawingPrivate));

  smiley_drawing_signals[CHANGED_SIGNAL]=g_signal_new("color-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST|G_SIGNAL_ACTION, G_STRUCT_OFFSET(SmileyDrawingClass, color_changed), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  g_object_class_install_property(gobject_class, NAME, g_param_spec_string("SmileyName", "SmileyName", "Some Name", "Alfred", G_PARAM_READWRITE));

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
    case NAME:
      smiley_drawing_set_name(da, g_value_get_string(value));
      break;
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

  g_signal_emit_by_name((gpointer)da, "color-changed");
  gtk_widget_queue_draw(GTK_WIDGET(da));
}
//Needed for g_object_set(). 
static void smiley_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  SmileyDrawing *da=SMILEY_DRAWING(object);
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case NAME:
      g_value_set_string(value, priv->smiley_name);
      break;
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

  //Initialize smiley name.
  priv->smiley_name=g_strdup("Alfred");

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
  //Just scale on the y-axis
  gdouble scale_y=(double)height/342.0;
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
  cairo_translate(cr, center_x, center_y);
  //Keep the drawing square with the y-scale.
  cairo_scale(cr, scale_y, scale_y);
  cairo_set_source_rgba(cr, priv->color_rgba[0], priv->color_rgba[1], priv->color_rgba[2], priv->color_rgba[3]);
  cairo_set_line_width(cr, 10);
  //Circle
  cairo_arc(cr, 0, 0, 100, 0, 2*G_PI);
  cairo_stroke(cr);
  //Left eye.
  cairo_arc(cr, -35, -38, 7, 0, 2*G_PI);
  cairo_fill(cr);
  cairo_stroke(cr);
  //Right eye.
  cairo_arc(cr, 35, -38, 7, 0, 2*G_PI);
  cairo_fill(cr);
  cairo_stroke(cr);
  //Smile
  cairo_arc(cr, 0, 4, 60, 0, G_PI);
  cairo_stroke(cr);

  return FALSE;
}
static void smiley_drawing_finalize(GObject *object)
{
  g_print("Finalize\n");
  SmileyDrawing *da=SMILEY_DRAWING(object);
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);
  
  g_free(priv->smiley_name);
}
void smiley_drawing_set_name(SmileyDrawing *da, const gchar *smiley_name)
{
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);

  if(priv->smiley_name!=NULL) g_free(priv->smiley_name);
  priv->smiley_name=g_strdup(smiley_name);  
}
const gchar* smiley_drawing_get_name(SmileyDrawing *da)
{
  SmileyDrawingPrivate *priv=SMILEY_DRAWING_GET_PRIVATE(da);

  return priv->smiley_name;  
}

