
/*

    For use with penrose_main.c. Look in penrose_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "penrose.h"

#define PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PENROSE_TRIANGLE_DRAWING_TYPE, PenroseTriangleDrawingPrivate))

typedef struct _PenroseTriangleDrawingPrivate PenroseTriangleDrawingPrivate;

struct _PenroseTriangleDrawingPrivate
{
  gchar *background_string;
  gchar *foreground_string;
  gdouble background[4];
  gdouble foreground[4];
  gboolean draw_css;
};

enum
{
  PROP_0,
  BACKGROUND,
  FOREGROUND,
  DRAW_CSS
};

//Private functions.
static void penrose_triangle_drawing_class_init(PenroseTriangleDrawingClass *klass);
static void penrose_triangle_drawing_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void penrose_triangle_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void penrose_triangle_drawing_init(PenroseTriangleDrawing *da);
static gboolean penrose_triangle_drawing_draw(GtkWidget *widget, cairo_t *cr);
static void penrose_triangle_drawing_finalize(GObject *gobject);

G_DEFINE_TYPE(PenroseTriangleDrawing, penrose_triangle_drawing, GTK_TYPE_DRAWING_AREA)

static void penrose_triangle_drawing_class_init(PenroseTriangleDrawingClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=penrose_triangle_drawing_set_property;
  gobject_class->get_property=penrose_triangle_drawing_get_property;

  //Draw when first shown.
  widget_class->draw=penrose_triangle_drawing_draw;
  gobject_class->finalize=penrose_triangle_drawing_finalize; 

  g_type_class_add_private(klass, sizeof(PenroseTriangleDrawingPrivate));

  g_object_class_install_property(gobject_class, BACKGROUND, g_param_spec_string("background", "background", "background", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, FOREGROUND, g_param_spec_string("foreground", "foreground", "foreground", NULL, G_PARAM_READWRITE));

g_object_class_install_property(gobject_class, DRAW_CSS, g_param_spec_boolean("draw_css", "draw_css", "draw_css", TRUE, G_PARAM_READWRITE));
 
}
//For if someone tries to set a property with g_object_set().
static void penrose_triangle_drawing_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  PenroseTriangleDrawing *da=PENROSE_TRIANGLE_DRAWING(object);

  switch(prop_id)
  {  
    case BACKGROUND:
      penrose_triangle_drawing_set_background(da, g_value_get_string(value));
      break;
    case FOREGROUND:
      penrose_triangle_drawing_set_foreground(da, g_value_get_string(value));
      break;
    case DRAW_CSS:
      penrose_triangle_drawing_set_draw_css(da, g_value_get_boolean(value));
      break;  
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void penrose_triangle_drawing_set_background(PenroseTriangleDrawing *da, const gchar *background_string)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, background_string))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->background[0]=rgba.red;
      priv->background[1]=rgba.green;
      priv->background[2]=rgba.blue;
      priv->background[3]=rgba.alpha;
      priv->draw_css=FALSE;
      if(priv->background_string!=NULL) g_free(priv->background_string);
      priv->background_string=g_strdup(background_string); 
    }
  else
    {
      g_print("background_string error\n");
    } 
}
void penrose_triangle_drawing_set_foreground(PenroseTriangleDrawing *da, const gchar *foreground_string)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, foreground_string))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->foreground[0]=rgba.red;
      priv->foreground[1]=rgba.green;
      priv->foreground[2]=rgba.blue;
      priv->foreground[3]=rgba.alpha;
      priv->draw_css=FALSE;
      if(priv->foreground_string!=NULL) g_free(priv->foreground_string);
      priv->foreground_string=g_strdup(foreground_string); 
    }
  else
    {
      g_print("foreground_string error\n");
    } 
}
void penrose_triangle_drawing_set_draw_css(PenroseTriangleDrawing *da, gboolean draw_css)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);
  priv->draw_css=draw_css; 
}
//For g_object_get().
static void penrose_triangle_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  PenroseTriangleDrawing *da=PENROSE_TRIANGLE_DRAWING(object);
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);

  switch(prop_id)
  {
    case BACKGROUND:
      g_value_set_string(value, priv->background_string);
      break;
    case FOREGROUND:
      g_value_set_string(value, priv->foreground_string);
      break;
    case DRAW_CSS:
      g_value_set_boolean(value, priv->draw_css);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
const gchar* penrose_triangle_drawing_get_background(PenroseTriangleDrawing *da)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);
  return priv->background_string;  
}
const gchar* penrose_triangle_drawing_get_foreground(PenroseTriangleDrawing *da)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);
  return priv->foreground_string;  
}
gboolean penrose_triangle_drawing_get_draw_css(PenroseTriangleDrawing *da)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);
  return priv->draw_css;  
}
static void penrose_triangle_drawing_init(PenroseTriangleDrawing *da)
{
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);

  //Set default white and black drawing colors.
  priv->background[0]=1.0;
  priv->background[1]=1.0;
  priv->background[2]=1.0;
  priv->background[3]=1.0;
  priv->foreground[0]=0.0;
  priv->foreground[1]=0.0;
  priv->foreground[2]=0.0;
  priv->foreground[3]=1.0;
  priv->draw_css=TRUE;

  //Get the CSS colors.
  GError *css_error=NULL;
  GtkCssProvider *provider=gtk_css_provider_new();
  GtkStyleContext *context=gtk_widget_get_style_context(GTK_WIDGET(da));
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_css_provider_load_from_path(provider, "penrose.css", &css_error);
  if(css_error!=NULL)
    {
      g_print("CSS loader error %s\n", css_error->message);
      g_error_free(css_error);
    }
  g_object_unref(provider);  
}
GtkWidget* penrose_triangle_drawing_new(void)
{
  return GTK_WIDGET(g_object_new(penrose_triangle_drawing_get_type(), NULL));
}
static gboolean penrose_triangle_drawing_draw(GtkWidget *widget, cairo_t *cr)
{
  gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
  
  gdouble scale_x=width/1000.0;
  gdouble scale_y=height/1000.0;
  PenroseTriangleDrawing *da=PENROSE_TRIANGLE_DRAWING(widget);
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);

  //Draw background and foreground with accessor function values or CSS.
  if(priv->draw_css)
    {
      GtkStyleContext *context=gtk_widget_get_style_context(GTK_WIDGET(da));
      gtk_render_background(context, cr, 0, 0, width, height);
      GdkRGBA color;
      gtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, &color);
      cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);
    }
  else
    {
      cairo_set_source_rgba(cr, priv->background[0], priv->background[1], priv->background[2], priv->background[3]);
      cairo_paint(cr);
      cairo_set_source_rgba(cr, priv->foreground[0], priv->foreground[1], priv->foreground[2], priv->background[3]);      
    }

  //The penrose triangle.
  cairo_move_to(cr, 155.*scale_x, 165.*scale_y);
  cairo_line_to(cr, 155.*scale_x, 838.*scale_y);
  cairo_line_to(cr, 265.*scale_x, 900.*scale_y);
  cairo_line_to(cr, 849.*scale_x, 564.*scale_y);
  cairo_line_to(cr, 849.*scale_x, 438.*scale_y);
  cairo_line_to(cr, 265.*scale_x, 100.*scale_y);
  cairo_line_to(cr, 155.*scale_x, 165.*scale_y);
  cairo_move_to(cr, 265.*scale_x, 100.*scale_y);
  cairo_line_to(cr, 265.*scale_x, 652.*scale_y);
  cairo_line_to(cr, 526.*scale_x, 502.*scale_y);
  cairo_move_to(cr, 369.*scale_x, 411.*scale_y);
  cairo_line_to(cr, 633.*scale_x, 564.*scale_y);
  cairo_move_to(cr, 369.*scale_x, 286.*scale_y);
  cairo_line_to(cr, 369.*scale_x, 592.*scale_y);
  cairo_move_to(cr, 369.*scale_x, 286.*scale_y);
  cairo_line_to(cr, 849.*scale_x, 564.*scale_y);
  cairo_move_to(cr, 633.*scale_x, 564.*scale_y);
  cairo_line_to(cr, 155.*scale_x, 838.*scale_y);
  cairo_stroke(cr);

  return FALSE;
}
static void penrose_triangle_drawing_finalize(GObject *object)
{
  g_print("Finalize\n");
  PenroseTriangleDrawing *da=PENROSE_TRIANGLE_DRAWING(object);
  PenroseTriangleDrawingPrivate *priv=PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(da);
  
  g_free(priv->background_string);
  g_free(priv->foreground_string);

  G_OBJECT_CLASS(penrose_triangle_drawing_parent_class)->finalize(object); 
}

