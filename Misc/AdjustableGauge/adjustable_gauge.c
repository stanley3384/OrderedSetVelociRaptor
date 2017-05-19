
/*
    For use with adjustable_gauge_main.c. Look in adjustable_gauge_main.c for more information.

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>
#include "adjustable_gauge.h"

#define ADJUSTABLE_GAUGE_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), ADJUSTABLE_GAUGE_TYPE, AdjustableGaugePrivate))

typedef struct _AdjustableGaugePrivate AdjustableGaugePrivate;

struct _AdjustableGaugePrivate
{
  gdouble first_cutoff;
  gdouble second_cutoff;
  gdouble needle;
  gdouble scale_bottom;
  gdouble scale_top;
  gdouble inside_radius;
  gdouble outside_radius;
  gint gauge_drawing_name;
  gboolean draw_gradient;
  //colors
  gchar *background_string;
  gchar *text_color_string;
  gchar *arc_color_string1;
  gchar *arc_color_string2;
  gchar *arc_color_string3;
  gchar *needle_color_string;
  gdouble background[4];
  gdouble text_color[4];
  gdouble arc_color1[4];
  gdouble arc_color2[4];
  gdouble arc_color3[4];
  gdouble needle_color[4];
};

enum
{
  PROP_0,
  FIRST_CUTOFF,
  SECOND_CUTOFF,
  NEEDLE,
  SCALE_BOTTOM,
  SCALE_TOP,
  INSIDE_RADIUS,
  OUTSIDE_RADIUS,
  GAUGE_DRAWING_NAME,
  DRAW_GRADIENT,
  //color properties
  BACKGROUND,
  TEXT_COLOR,
  ARC_COLOR1,
  ARC_COLOR2,
  ARC_COLOR3,
  NEEDLE_COLOR
};

//Private functions.
static void adjustable_gauge_class_init(AdjustableGaugeClass *klass);
static void adjustable_gauge_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void adjustable_gauge_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void adjustable_gauge_init(AdjustableGauge *da);
static gboolean adjustable_gauge_draw(GtkWidget *widget, cairo_t *cr);
static void adjustable_voltage_gauge_draw(GtkWidget *da, cairo_t *cr);
static void voltage_arc_solid(GtkWidget *da, cairo_t *cr);
static void adjustable_speedometer_gauge_draw(GtkWidget *da, cairo_t *cr);
static void speedometer_arc_solid(GtkWidget *da, cairo_t *cr);
//Draw arcs with a gradient.
static void draw_arc(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1);
static void draw_arc_gradient(GtkWidget *da, cairo_t *cr, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3, gdouble x4, gdouble y4, gdouble color_start1[], gdouble color_mid1[], gdouble color_stop1[], gint gradient_id);
static void adjustable_gauge_finalize(GObject *gobject);

G_DEFINE_TYPE(AdjustableGauge, adjustable_gauge, GTK_TYPE_DRAWING_AREA)

static void adjustable_gauge_class_init(AdjustableGaugeClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=adjustable_gauge_set_property;
  gobject_class->get_property=adjustable_gauge_get_property;

  //Draw when first shown.
  widget_class->draw=adjustable_gauge_draw;
  gobject_class->finalize=adjustable_gauge_finalize;

  g_type_class_add_private(klass, sizeof(AdjustableGaugePrivate));

  g_object_class_install_property(gobject_class, FIRST_CUTOFF, g_param_spec_double("first_cutoff", "first_cutoff", "first_cutoff", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SECOND_CUTOFF, g_param_spec_double("second_cutoff", "second_cutoff", "second_cutoff", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, NEEDLE, g_param_spec_double("needle", "needle", "needle", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SCALE_BOTTOM, g_param_spec_double("scale_bottom", "scale_bottom", "scale_bottom", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SCALE_TOP, g_param_spec_double("scale_top", "scale_top", "scale_top", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, INSIDE_RADIUS, g_param_spec_double("inside_radius", "inside_radius", "inside_radius", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, OUTSIDE_RADIUS, g_param_spec_double("outside_radius", "outside_radius", "outside_radius", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SCALE_TOP, g_param_spec_int("gauge_drawing_name", "gauge_drawing_name", "gauge_drawing_name", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, DRAW_GRADIENT, g_param_spec_boolean("draw_gradient", "draw_gradient", "draw_gradient", FALSE, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, BACKGROUND, g_param_spec_string("background", "background", "background", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, TEXT_COLOR, g_param_spec_string("text_color", "text_color", "text_color", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, ARC_COLOR1, g_param_spec_string("arc_color1", "arc_color1", "arc_color1", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, ARC_COLOR2, g_param_spec_string("arc_color2", "arc_color2", "arc_color2", NULL, G_PARAM_READWRITE));
 
  g_object_class_install_property(gobject_class, ARC_COLOR3, g_param_spec_string("arc_color3", "arc_color3", "arc_color3", NULL, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, NEEDLE_COLOR, g_param_spec_string("needle_color", "needle_color", "needle_color", NULL, G_PARAM_READWRITE));

}
//Needed for g_object_set().
static void adjustable_gauge_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  AdjustableGauge *da=ADJUSTABLE_GAUGE(object);

  switch(prop_id)
  {
    case FIRST_CUTOFF:
      adjustable_gauge_set_first_cutoff(da, g_value_get_double(value));
      break;
    case SECOND_CUTOFF:
      adjustable_gauge_set_second_cutoff(da, g_value_get_double(value));
      break;
    case NEEDLE:
      adjustable_gauge_set_needle(da, g_value_get_double(value));
      break;
    case SCALE_BOTTOM:
      adjustable_gauge_set_scale_bottom(da, g_value_get_double(value));
      break;
    case SCALE_TOP:
      adjustable_gauge_set_scale_top(da, g_value_get_double(value));
      break; 
    case INSIDE_RADIUS:
      adjustable_gauge_set_inside_radius(da, g_value_get_double(value));
      break; 
    case OUTSIDE_RADIUS:
      adjustable_gauge_set_outside_radius(da, g_value_get_double(value));
      break; 
    case GAUGE_DRAWING_NAME:
      adjustable_gauge_set_drawing(da, g_value_get_int(value));
      break;
    case DRAW_GRADIENT:
      adjustable_gauge_set_draw_gradient(da, g_value_get_boolean(value));
      break;
    case BACKGROUND:
      adjustable_gauge_set_background(da, g_value_get_string(value));
      break;
    case TEXT_COLOR:
      adjustable_gauge_set_text_color(da, g_value_get_string(value));
      break;
    case ARC_COLOR1:
      adjustable_gauge_set_arc_color1(da, g_value_get_string(value));
      break;
    case ARC_COLOR2:
      adjustable_gauge_set_arc_color2(da, g_value_get_string(value));
      break;
    case ARC_COLOR3:
      adjustable_gauge_set_arc_color3(da, g_value_get_string(value));
      break;
    case NEEDLE_COLOR:
      adjustable_gauge_set_needle_color(da, g_value_get_string(value));
      break;        
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void adjustable_gauge_set_first_cutoff(AdjustableGauge *da, gdouble first_cutoff)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
 
  if(first_cutoff>=priv->scale_bottom&&first_cutoff<=priv->scale_top)
    { 
      //Subtract a small amount for rounding errors. Need a better way to do this.
      priv->first_cutoff=first_cutoff-0.000001;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The first cutoff is out of range.");
    }
}
void adjustable_gauge_set_second_cutoff(AdjustableGauge *da, gdouble second_cutoff)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
 
  if(second_cutoff>=priv->scale_bottom&&second_cutoff<=priv->scale_top)
    { 
      //Subtract a small amount for rounding errors. Need a better way to do this.
      priv->second_cutoff=second_cutoff-0.000001;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The second cutoff is out of range.");
    }
}
void adjustable_gauge_set_needle(AdjustableGauge *da, gdouble needle)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(needle>=priv->scale_bottom&&needle<=priv->scale_top)
    {
      priv->needle=needle;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("The needle is out of range.");
    }
} 
void adjustable_gauge_set_scale_bottom(AdjustableGauge *da, gdouble scale_bottom)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(scale_bottom<priv->scale_top)
    {
      priv->scale_bottom=scale_bottom;
      priv->needle=scale_bottom;
      //Check if the cutoffs are in range. If they aren't reset them to the bottom of the scale.
      if(priv->first_cutoff<priv->scale_bottom||priv->first_cutoff>priv->scale_top)
        {
          priv->first_cutoff=priv->scale_bottom+0.000001;
          g_warning("The first cutoff was reset to the bottom of scale.");
        }
      if(priv->second_cutoff<priv->scale_bottom||priv->second_cutoff>priv->scale_top)
        {
          priv->second_cutoff=priv->scale_bottom+0.000001;
          g_warning("The second cutoff was reset to the bottom of scale.");
        }
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("The bottom of the scale is out of range.");
    }
} 
void adjustable_gauge_set_scale_top(AdjustableGauge *da, gdouble scale_top)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(scale_top>priv->scale_bottom)
    {
      priv->scale_top=scale_top;
      //Check if the cutoffs are in range. If they aren't reset them to the top of the scale.
      if(priv->first_cutoff<priv->scale_bottom||priv->first_cutoff>priv->scale_top)
        {
          priv->first_cutoff=priv->scale_top-0.000001;
          g_warning("The first cutoff was reset to the top of scale.");
        }
      if(priv->second_cutoff<priv->scale_bottom||priv->second_cutoff>priv->scale_top)
        {
          priv->second_cutoff=priv->scale_top-0.000001;;
          g_warning("The second cutoff was reset to the top of scale.");
        }
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("The top of the scale is out of range.");
    }
} 
void adjustable_gauge_set_inside_radius(AdjustableGauge *da, gdouble inside_radius)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(inside_radius>=1.0&&inside_radius<=3.0)
    {
      priv->inside_radius=inside_radius;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("Inside radius values 1.0<=x<=3.0.");
    }
} 
void adjustable_gauge_set_outside_radius(AdjustableGauge *da, gdouble outside_radius)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(outside_radius>=3.0&&outside_radius<=5.0)
    {
      priv->outside_radius=outside_radius;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("Outside radius values 3.0<=x<=5.0.");
    }
} 
void adjustable_gauge_set_drawing(AdjustableGauge *da, gint drawing_name)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(drawing_name==1)
    {
      priv->gauge_drawing_name=drawing_name;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      priv->gauge_drawing_name=0;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
}
void adjustable_gauge_set_draw_gradient(AdjustableGauge *da, gboolean draw_gradient)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(draw_gradient==TRUE)
    {
      priv->draw_gradient=draw_gradient;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      priv->draw_gradient=FALSE;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
}
void adjustable_gauge_set_background(AdjustableGauge *da, const gchar *background_string)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, background_string))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->background[0]=rgba.red;
      priv->background[1]=rgba.green;
      priv->background[2]=rgba.blue;
      priv->background[3]=rgba.alpha;
      if(priv->background_string!=NULL) g_free(priv->background_string);
      priv->background_string=g_strdup(background_string); 
    }
  else
    {
      g_warning("background_string error\n");
    } 
} 
void adjustable_gauge_set_text_color(AdjustableGauge *da, const gchar *text_color_string)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, text_color_string))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->text_color[0]=rgba.red;
      priv->text_color[1]=rgba.green;
      priv->text_color[2]=rgba.blue;
      priv->text_color[3]=rgba.alpha;
      if(priv->text_color_string!=NULL) g_free(priv->text_color_string);
      priv->text_color_string=g_strdup(text_color_string); 
    }
  else
    {
      g_warning("text_color_string error\n");
    } 
} 
void adjustable_gauge_set_arc_color1(AdjustableGauge *da, const gchar *arc_color_string1)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, arc_color_string1))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->arc_color1[0]=rgba.red;
      priv->arc_color1[1]=rgba.green;
      priv->arc_color1[2]=rgba.blue;
      priv->arc_color1[3]=rgba.alpha;
      if(priv->arc_color_string1!=NULL) g_free(priv->arc_color_string1);
      priv->arc_color_string1=g_strdup(arc_color_string1); 
    }
  else
    {
      g_warning("arc_color_string1 error\n");
    } 
}
void adjustable_gauge_set_arc_color2(AdjustableGauge *da, const gchar *arc_color_string2)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, arc_color_string2))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->arc_color2[0]=rgba.red;
      priv->arc_color2[1]=rgba.green;
      priv->arc_color2[2]=rgba.blue;
      priv->arc_color2[3]=rgba.alpha;
      if(priv->arc_color_string2!=NULL) g_free(priv->arc_color_string2);
      priv->arc_color_string2=g_strdup(arc_color_string2); 
    }
  else
    {
      g_warning("arc_color_string2 error\n");
    } 
}
void adjustable_gauge_set_arc_color3(AdjustableGauge *da, const gchar *arc_color_string3)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, arc_color_string3))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->arc_color3[0]=rgba.red;
      priv->arc_color3[1]=rgba.green;
      priv->arc_color3[2]=rgba.blue;
      priv->arc_color3[3]=rgba.alpha;
      if(priv->arc_color_string3!=NULL) g_free(priv->arc_color_string3);
      priv->arc_color_string3=g_strdup(arc_color_string3); 
    }
  else
    {
      g_warning("arc_color_string3 error\n");
    } 
} 
void adjustable_gauge_set_needle_color(AdjustableGauge *da, const gchar *needle_color_string)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, needle_color_string))
    {
      //g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->needle_color[0]=rgba.red;
      priv->needle_color[1]=rgba.green;
      priv->needle_color[2]=rgba.blue;
      priv->needle_color[3]=rgba.alpha;
      if(priv->needle_color_string!=NULL) g_free(priv->needle_color_string);
      priv->needle_color_string=g_strdup(needle_color_string); 
    }
  else
    {
      g_warning("needle_color_string error\n");
    } 
}     
static void adjustable_gauge_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  AdjustableGauge *da=ADJUSTABLE_GAUGE(object);
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case FIRST_CUTOFF:
      g_value_set_double(value, priv->first_cutoff);
      break;
    case SECOND_CUTOFF:
      g_value_set_double(value, priv->second_cutoff);
      break;
    case NEEDLE:
      g_value_set_double(value, priv->needle);
      break;
    case SCALE_BOTTOM:
      g_value_set_double(value, priv->scale_bottom);
      break;
    case SCALE_TOP:
      g_value_set_double(value, priv->scale_top);
      break;
    case INSIDE_RADIUS:
      g_value_set_double(value, priv->inside_radius);
      break;
    case OUTSIDE_RADIUS:
      g_value_set_double(value, priv->outside_radius);
      break;
    case GAUGE_DRAWING_NAME:
      g_value_set_int(value, priv->gauge_drawing_name);
      break;
    case DRAW_GRADIENT:
      g_value_set_boolean(value, priv->draw_gradient);
      break;
    case BACKGROUND:
      g_value_set_string(value, priv->background_string);
      break;
    case TEXT_COLOR:
      g_value_set_string(value, priv->text_color_string);
      break;
    case ARC_COLOR1:
      g_value_set_string(value, priv->arc_color_string1);
      break;
    case ARC_COLOR2:
      g_value_set_string(value, priv->arc_color_string2);
      break;
    case ARC_COLOR3:
      g_value_set_string(value, priv->arc_color_string3);
      break;
    case NEEDLE_COLOR:
      g_value_set_string(value, priv->needle_color_string);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
gdouble adjustable_gauge_get_first_cutoff(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->first_cutoff;
}
gdouble adjustable_gauge_get_second_cutoff(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->second_cutoff;
}
gdouble adjustable_gauge_get_needle(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->needle;
}
gdouble adjustable_gauge_get_scale_bottom(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->scale_bottom;
}
gdouble adjustable_gauge_get_scale_top(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->scale_top;
}
gdouble adjustable_gauge_get_inside_radius(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->inside_radius;
}
gdouble adjustable_gauge_get_outside_radius(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->outside_radius;
}
gint adjustable_gauge_get_drawing(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->gauge_drawing_name;
}
gboolean adjustable_gauge_get_draw_gradient(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->draw_gradient;
}
const gchar* adjustable_gauge_get_background(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->background_string;
}
const gchar* adjustable_gauge_get_text_color(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->text_color_string;
}
const gchar* adjustable_gauge_get_arc_color1(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->arc_color_string1;
}
const gchar* adjustable_gauge_get_arc_color2(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->arc_color_string2;
}
const gchar* adjustable_gauge_get_arc_color3(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->arc_color_string1;
}
const gchar* adjustable_gauge_get_needle_color(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->arc_color_string1;
}
static void adjustable_gauge_init(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  //Initailize default gauge properties.
  priv->first_cutoff=80.0;
  priv->second_cutoff=90.0;
  priv->needle=0.0;
  priv->scale_bottom=0.0;
  priv->scale_top=100.0;
  priv->inside_radius=2.0;
  priv->outside_radius=4.0;
  priv->gauge_drawing_name=0;
  priv->draw_gradient=FALSE;

  //Set the initial colors.
  priv->background[0]=0.0;
  priv->background[1]=0.0;
  priv->background[2]=0.0;
  priv->background[3]=1.0;
  priv->text_color[0]=1.0;
  priv->text_color[1]=1.0;
  priv->text_color[2]=1.0;
  priv->text_color[3]=1.0;
  priv->arc_color1[0]=0.0;
  priv->arc_color1[1]=1.0;
  priv->arc_color1[2]=0.0;
  priv->arc_color1[3]=1.0;
  priv->arc_color2[0]=1.0;
  priv->arc_color2[1]=1.0;
  priv->arc_color2[2]=0.0;
  priv->arc_color2[3]=1.0;
  priv->arc_color3[0]=1.0;
  priv->arc_color3[1]=0.0;
  priv->arc_color3[2]=0.0;
  priv->arc_color3[3]=1.0;
  priv->needle_color[0]=1.0;
  priv->needle_color[1]=1.0;
  priv->needle_color[2]=1.0;
  priv->needle_color[3]=1.0;
  
  priv->background_string=g_strdup("rgba(0, 0, 0, 1.0)");
  priv->text_color_string=g_strdup("rgba(255, 255, 255, 1.0)");
  priv->arc_color_string1=g_strdup("rgba(0, 255, 0, 1.0)");
  priv->arc_color_string2=g_strdup("rgba(255, 255, 0, 1.0)");
  priv->arc_color_string3=g_strdup("rgba(255, 0, 0, 1.0)");
  priv->needle_color_string=g_strdup("rgba(255, 255, 255, 1.0)");

}
GtkWidget* adjustable_gauge_new()
{
  return GTK_WIDGET(g_object_new(adjustable_gauge_get_type(), NULL));
}
static gboolean adjustable_gauge_draw(GtkWidget *da, cairo_t *cr)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  if(priv->gauge_drawing_name==VOLTAGE_GAUGE) adjustable_voltage_gauge_draw(da, cr);
  else adjustable_speedometer_gauge_draw(da, cr);
  return FALSE;
}
static void adjustable_voltage_gauge_draw(GtkWidget *da, cairo_t *cr)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
  gdouble scale_text=0;
  gdouble w1=0;

  //Scale.
  if(width<height)
    {
      scale_text=width/400.0;
      w1=(gdouble)width/10.0;
    }
  else
    {
      scale_text=height/400.0;
      w1=(gdouble)height/10.0;
    }

  /*
    What value will fail to clip the 24 trapezoids to a circular ring? Check and see.
    Get the unit circle points at pi/12 times the outside radius. Get the points on a line
    half way between those points and (outside_radius, 0.0). Use that to get the radius
    from there back to the origin. Just use a 0.1 clip adjustment.
 
  gdouble unit_x=priv->outside_radius*w1*(1.0-(1.0-(1+sqrt(3.0))/(2.0*sqrt(2.0))));
  gdouble unit_y=priv->outside_radius*w1*((sqrt(3.0)-1.0)/(2.0*sqrt(2.0)));
  gdouble trap_half_x=priv->outside_radius*w1-(0.5*(priv->outside_radius*w1-unit_x));
  gdouble trap_half_y=0.5*unit_y;
  gdouble trap_half_r=sqrt(trap_half_x*trap_half_x+trap_half_y*trap_half_y);
  gint i=0;
  for(i=0;i<10;i++)
    {
      g_print("%i clip%f <= trap%f <= circle%f\n", i, (priv->outside_radius-(gdouble)i/10.0)*w1, trap_half_r, priv->outside_radius*w1);
    }
  */

  gdouble inside=(priv->inside_radius+0.1)*w1;
  gdouble outside=(priv->outside_radius-0.1)*w1;
  gdouble middle=(priv->inside_radius+0.5*(priv->outside_radius-priv->inside_radius))*w1;
    
  cairo_set_source_rgba(cr, priv->background[0], priv->background[1], priv->background[2], priv->background[3]);
  cairo_paint(cr);

  //transforms
  cairo_translate(cr, width/2.0, 5.0*height/8.0);

  if(priv->draw_gradient)
    {
      cairo_save(cr);
      cairo_arc(cr, 0.0, 0.0, (priv->outside_radius-0.1)*w1, 0.0, 2.0*G_PI);
      cairo_clip(cr);
      draw_arc(da, cr, -G_PI/12.0, 10, G_PI+G_PI/12);
      cairo_restore(cr);
      cairo_arc(cr, 0.0, 0.0, (priv->inside_radius+0.1)*w1, 0.0, 2.0*G_PI);
      cairo_fill(cr); 
    }
  else
    {
      voltage_arc_solid(da, cr);
    }

  //Difference of top and bottom used to standardize values.
  gdouble diff=priv->scale_top-priv->scale_bottom;

  /*
    Needle line between 1 and 11. Standardize needle on this scale because it goes from sections
    1-11 of the upper 12 half of a 24 section circle.
  */
  gdouble standard_needle=(((priv->needle-priv->scale_bottom)/diff)*10.0)+1.0;
  if(priv->needle>priv->scale_top)
    {
      g_print("Gauge overload %f!\n", priv->needle);
      standard_needle=11.0;
    }
  if(priv->needle<priv->scale_bottom)
    {
      g_print("Gauge underload %f!\n", priv->needle);
      standard_needle=1.0;
    }
  cairo_set_source_rgba(cr, priv->needle_color[0], priv->needle_color[1], priv->needle_color[2], priv->needle_color[3]);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, -cos(standard_needle*G_PI/12.0)*outside, -sin(standard_needle*G_PI/12.0)*outside);
  cairo_stroke(cr);

  //Text for needle value.
   cairo_set_source_rgba(cr, priv->text_color[0], priv->text_color[1], priv->text_color[2], priv->text_color[3]);
  cairo_text_extents_t extents1;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 46*scale_text);
  gchar *string1=g_strdup_printf("%3.2f", priv->needle);
  cairo_text_extents(cr, string1, &extents1); 
  cairo_move_to(cr, -extents1.width/2, 0.5*inside+extents1.height/2);  
  cairo_show_text(cr, string1);
  g_free(string1);

  //Text for bottom end scale value.
  cairo_text_extents_t extents2;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 22*scale_text);
  gchar *string2=g_strdup_printf("%3.2f", priv->scale_bottom);
  cairo_text_extents(cr, string2, &extents2); 
  cairo_move_to(cr, -extents2.width/2-middle, extents2.height/2);  
  cairo_show_text(cr, string2);
  g_free(string2);

  //Text for top end scale value.
  cairo_text_extents_t extents3;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 22*scale_text);
  gchar *string3=g_strdup_printf("%3.2f", priv->scale_top);
  cairo_text_extents(cr, string3, &extents3); 
  cairo_move_to(cr, -extents3.width/2+middle, extents3.height/2);  
  cairo_show_text(cr, string3);
  g_free(string3);

}
static void voltage_arc_solid(GtkWidget *da, cairo_t *cr)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
  gdouble w1=0;

  //Scale.
  if(width<height) w1=(gdouble)width/10.0;
  else w1=(gdouble)height/10.0;

  gdouble inside=(priv->inside_radius+0.1)*w1;
  gdouble outside=(priv->outside_radius-0.1)*w1;

  //Green underneath 
  cairo_set_source_rgba(cr, priv->arc_color1[0], priv->arc_color1[1], priv->arc_color1[2], priv->arc_color1[3]);
  cairo_set_line_width(cr, 3.0);
  cairo_arc_negative(cr, 0, 0, inside, 23.0*G_PI/12.0, 13.0*G_PI/12.0);
  cairo_line_to(cr, -(cos(G_PI/12.0)*outside), -sin(G_PI/12.0)*outside);
  cairo_arc(cr, 0, 0, outside, 13.0*G_PI/12.0, 23.0*G_PI/12.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Difference of top and bottom used to standardize values.
  gdouble diff=priv->scale_top-priv->scale_bottom;

  //Draw yellow next. Standardized on 13 to 23 scale. The upper 12 half of a 24 section circle.
  gdouble standard_first_cutoff=(((priv->first_cutoff-priv->scale_bottom)/diff)*10.0)+13.0;
   cairo_set_source_rgba(cr, priv->arc_color2[0], priv->arc_color2[1], priv->arc_color2[2], priv->arc_color2[3]);
  cairo_arc_negative(cr, 0, 0, inside, 23.0*G_PI/12.0, standard_first_cutoff*G_PI/12.0);
  cairo_arc(cr, 0, 0, outside, standard_first_cutoff*G_PI/12.0, 23.0*G_PI/12.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Draw red top. Standardized on 13 to 23 scale. The upper 12 half of a 24 section circle.
  gdouble standard_second_cutoff=(((priv->second_cutoff-priv->scale_bottom)/diff)*10.0)+13.0;
   cairo_set_source_rgba(cr, priv->arc_color3[0], priv->arc_color3[1], priv->arc_color3[2], priv->arc_color3[3]);
  cairo_arc_negative(cr, 0, 0, inside, 23.0*G_PI/12.0, standard_second_cutoff*G_PI/12.0);
  cairo_arc(cr, 0, 0, outside, standard_second_cutoff*G_PI/12.0, 23.0*G_PI/12.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);
}
static void adjustable_speedometer_gauge_draw(GtkWidget *da, cairo_t *cr)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
  gdouble scale_text=0;
  gdouble w1=0;
    
  cairo_set_source_rgba(cr, priv->background[0], priv->background[1], priv->background[2], priv->background[3]);
  cairo_paint(cr);

  //Scale.
  if(width<height)
    {
      scale_text=width/400.0;
      w1=(gdouble)width/10.0;
    }
  else
    {
      w1=(gdouble)height/10.0;
      scale_text=height/400.0;
    }

  //Move to center.
  cairo_translate(cr, width/2.0, height/2.0);

  if(priv->draw_gradient)
    {
      cairo_save(cr);
      cairo_arc(cr, 0.0, 0.0, (priv->outside_radius-0.1)*w1, 0.0, 2.0*G_PI);
      cairo_clip(cr);
      draw_arc(da, cr, -G_PI/12.0, 20, 2.0*G_PI/3.0);
      cairo_restore(cr);
      cairo_arc(cr, 0.0, 0.0, (priv->inside_radius+0.1)*w1, 0.0, 2.0*G_PI);
      cairo_fill(cr); 
    }
  else
    {
      speedometer_arc_solid(da, cr);
    }

  //Position variables for tick marks and text.
  gdouble diff=priv->scale_top-priv->scale_bottom;
  gdouble tick_radius1=(priv->inside_radius+0.65*(priv->outside_radius-priv->inside_radius))*w1;
  gdouble tick_radius2=(priv->inside_radius+0.75*(priv->outside_radius-priv->inside_radius))*w1;
  gdouble text_radius=(priv->inside_radius+0.4*(priv->outside_radius-priv->inside_radius))*w1;
  gdouble needle_radius=(priv->inside_radius+0.25*(priv->outside_radius-priv->inside_radius))*w1;
  
  //Set large tick marks.
  gint i=0;
  cairo_set_source_rgba(cr, priv->text_color[0], priv->text_color[1], priv->text_color[2], priv->text_color[3]);
  gdouble tenth_scale=diff/10.0;
  gdouble tick_mark=(5.0*G_PI/3.0)/10.0;
  gdouble temp=0;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_text_extents_t tick_extents;
  cairo_set_font_size(cr, 20*scale_text);
  cairo_set_line_width(cr, 3.0);
  cairo_move_to(cr, 0, 0);
  for(i=0;i<11;i++)
    {
      temp=(gdouble)i*tick_mark;
      cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*tick_radius1, -sin((4.0*G_PI/3.0)-temp)*tick_radius1);
      cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*(priv->outside_radius-0.1)*w1, -sin((4.0*G_PI/3.0)-temp)*(priv->outside_radius-0.1)*w1);
      cairo_stroke(cr);
      //String values at large tick marks.
      gchar *tick_string=g_strdup_printf("%i", (gint)(priv->scale_bottom+(gdouble)i*tenth_scale));
      cairo_text_extents(cr, tick_string, &tick_extents);
      cairo_move_to(cr, (cos((4.0*G_PI/3.0)-temp)*text_radius)-tick_extents.width/2.0, (-sin((4.0*G_PI/3.0)-temp)*text_radius)+tick_extents.height/2.0);
      cairo_show_text(cr, tick_string);
      g_free(tick_string);
      //Reset position to the center.
      cairo_move_to(cr, 0, 0);
    }

  //Set small tick marks.
  cairo_set_line_width(cr, 2.0);
  gdouble half_tick=tick_mark/2.0;
  cairo_move_to(cr, 0, 0);
  for(i=0;i<10;i++)
    {
      temp=(gdouble)i*tick_mark+half_tick;
      cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*tick_radius2, -sin((4.0*G_PI/3.0)-temp)*tick_radius2);
      cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*(priv->outside_radius-0.1)*w1, -sin((4.0*G_PI/3.0)-temp)*(priv->outside_radius-0.1)*w1);
      cairo_stroke(cr);
      cairo_move_to(cr, 0, 0);
    }

  //The needle line.
  cairo_set_line_width(cr, 3.0);
  cairo_set_source_rgba(cr, priv->needle_color[0], priv->needle_color[1], priv->needle_color[2], priv->needle_color[3]);
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  gdouble standard_needle=(((priv->needle-priv->scale_bottom)/diff)*(5.0*G_PI/3.0));
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, cos((4.0*G_PI/3.0)-standard_needle)*needle_radius, -sin((4.0*G_PI/3.0)-standard_needle)*needle_radius);
  cairo_stroke(cr);
    
  //Text for needle value.
  cairo_set_source_rgba(cr, priv->text_color[0], priv->text_color[1], priv->text_color[2], priv->text_color[3]);
  cairo_text_extents_t extents1;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 30*scale_text);
  gchar *string1=g_strdup_printf("%i", (gint)priv->needle);
  cairo_text_extents(cr, string1, &extents1); 
  cairo_move_to(cr, -extents1.width/2, tick_radius1+extents1.height/2);  
  cairo_show_text(cr, string1);
  g_free(string1);
}
static void speedometer_arc_solid(GtkWidget *da, cairo_t *cr)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
  gdouble w1=0;

  //Scale.
  if(width<height) w1=(gdouble)width/10.0;
  else w1=(gdouble)height/10.0;

  //Inside outside radius of arc.
  gdouble inside=(priv->inside_radius+0.1)*w1;
  gdouble outside=(priv->outside_radius-0.1)*w1;

  //Green default value underneath 
   cairo_set_source_rgba(cr, priv->arc_color1[0], priv->arc_color1[1], priv->arc_color1[2], priv->arc_color1[3]);
  cairo_set_line_width(cr, 3.0);
  cairo_arc_negative(cr, 0, 0, inside, -5.0*G_PI/3.0, -4.0*G_PI/3.0);
  cairo_line_to(cr, (cos(-4.0*G_PI/3.0)*outside), sin(-4.0*G_PI/3.0)*outside);
  cairo_arc(cr, 0, 0, outside, -4.0*G_PI/3.0, -5.0*G_PI/3.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  gdouble diff=priv->scale_top-priv->scale_bottom;

  //Yellow default value next.
  gdouble standard_first_cutoff=(((priv->first_cutoff-priv->scale_bottom)/diff)*(5.0*G_PI/3.0));
   cairo_set_source_rgba(cr, priv->arc_color2[0], priv->arc_color2[1], priv->arc_color2[2], priv->arc_color2[3]);
  cairo_arc_negative(cr, 0, 0, inside, -5.0*G_PI/3.0, -4.0*G_PI/3.0+standard_first_cutoff);
  cairo_arc(cr, 0, 0, outside, -4.0*G_PI/3.0+standard_first_cutoff, -5.0*G_PI/3.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Red default value top.
  gdouble standard_second_cutoff=(((priv->second_cutoff-priv->scale_bottom)/diff)*(5.0*G_PI/3.0));
   cairo_set_source_rgba(cr, priv->arc_color3[0], priv->arc_color3[1], priv->arc_color3[2], priv->arc_color3[3]);
  cairo_arc_negative(cr, 0, 0, inside, -5.0*G_PI/3.0, -4.0*G_PI/3.0+standard_second_cutoff);
  cairo_arc(cr, 0, 0, outside, -4.0*G_PI/3.0+standard_second_cutoff, -5.0*G_PI/3.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);  
}
static void draw_arc(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  gint i=0;
  gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
  gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
  gdouble w1=0;

  //Scale. Keep square.
  if(width<height) w1=width/10.0;
  else w1=height/10.0;

  //Rotate from x=1, y=0 position in unit circle.
  cairo_rotate(cr, r1); 
   
  //Draw the trapezoids and paint them with a gradient.
  gdouble start=0.0;
  gdouble line_radius1=0;
  gdouble line_radius2=0;
  gdouble temp_cos1=0;
  gdouble temp_sin1=0;
  gdouble temp_cos2=0;
  gdouble temp_sin2=0;
  gdouble prev_cos1=priv->inside_radius*w1;
  gdouble prev_sin1=0.0;
  gdouble prev_cos2=priv->outside_radius*w1;
  gdouble prev_sin2=0.0;
  const gdouble ir=priv->inside_radius*w1;
  const gdouble or=priv->outside_radius*w1;
  //Standardize on a 0-100 scale.
  gdouble mid_color_pos=fabs((priv->first_cutoff-priv->scale_bottom)/(priv->scale_top-priv->scale_bottom))*100.0;

  //Colors and slopes for gradients.
  gdouble color_start1[3];
  gdouble color_mid1[3];
  gdouble color_stop1[3];
  gdouble diff0=0;
  gdouble diff1=0;
  gdouble diff2=0;
  gdouble diff3=0;
  gdouble diff4=0;
  gdouble diff5=0;

  //For when a trapezoid needs to be split when the gradient color is inside the trapezoid.
  gdouble split_trap=(mid_color_pos/100.0)*sections;
  gdouble split_trap_int, split_trap_frac;  
  split_trap_frac=modf(split_trap, &split_trap_int);
  gdouble trap_cos1=0;
  gdouble trap_sin1=0;
  gdouble trap_cos2=0;
  gdouble trap_sin2=0;

  for(i=0;i<sections;i++)
    {
      temp_cos1=cos(start-(next_section*(i+1)));
      temp_sin1=sin(start-(next_section*(i+1)));
      temp_cos2=temp_cos1;
      temp_sin2=temp_sin1;

      //The polar form of the equation for an ellipse to get the radius. Radius based on width.
      line_radius1=((ir)*(ir))/sqrt(((ir)*(ir)*temp_sin1*temp_sin1)+((ir)*(ir)*temp_cos1*temp_cos1));
      line_radius2=((or)*(or))/sqrt(((or)*(or)*temp_sin1*temp_sin1) + ((or)*(or)*temp_cos1*temp_cos1));

      temp_cos1=temp_cos1*line_radius1;
      temp_sin1=temp_sin1*line_radius1;
      temp_cos2=temp_cos2*line_radius2;
      temp_sin2=temp_sin2*line_radius2;

      /*
        Set the colors for the 4 corners of the trapezoid.
        100 means there is no mid color in the drawing. Just draw start to end.
      */
      if(mid_color_pos==100)
        {
          diff0=priv->arc_color3[0]-priv->arc_color1[0];
          diff1=priv->arc_color3[1]-priv->arc_color1[1];
          diff2=priv->arc_color3[2]-priv->arc_color1[2];
          color_start1[0]=priv->arc_color1[0]+(diff0*(gdouble)(i)/(gdouble)sections);
          color_start1[1]=priv->arc_color1[1]+(diff1*(gdouble)(i)/(gdouble)sections);
          color_start1[2]=priv->arc_color1[2]+(diff2*(gdouble)(i)/(gdouble)sections);
          color_stop1[0]=priv->arc_color1[0]+(diff0*(gdouble)(i+1)/(gdouble)sections);
          color_stop1[1]=priv->arc_color1[1]+(diff1*(gdouble)(i+1)/(gdouble)sections);
          color_stop1[2]=priv->arc_color1[2]+(diff2*(gdouble)(i+1)/(gdouble)sections);
        }
      else if(i<split_trap_int)
        { 
          diff0=priv->arc_color2[0]-priv->arc_color1[0];
          diff1=priv->arc_color2[1]-priv->arc_color1[1];
          diff2=priv->arc_color2[2]-priv->arc_color1[2];
          color_start1[0]=priv->arc_color1[0]+(diff0*(gdouble)(i)/split_trap);
          color_start1[1]=priv->arc_color1[1]+(diff1*(gdouble)(i)/split_trap);
          color_start1[2]=priv->arc_color1[2]+(diff2*(gdouble)(i)/split_trap);
          color_mid1[0]=priv->arc_color1[0]+(diff0*(gdouble)(i+1)/split_trap);
          color_mid1[1]=priv->arc_color1[1]+(diff1*(gdouble)(i+1)/split_trap);
          color_mid1[2]=priv->arc_color1[2]+(diff2*(gdouble)(i+1)/split_trap);
        }
      else if(split_trap_int==i)
        { 
          diff0=priv->arc_color2[0]-priv->arc_color1[0];
          diff1=priv->arc_color2[1]-priv->arc_color1[1];
          diff2=priv->arc_color2[2]-priv->arc_color1[2];
          diff3=priv->arc_color3[0]-priv->arc_color2[0];
          diff4=priv->arc_color3[1]-priv->arc_color2[1];
          diff5=priv->arc_color3[2]-priv->arc_color2[2];
          color_start1[0]=priv->arc_color1[0]+(diff0*(gdouble)(i)/(gdouble)split_trap);
          color_start1[1]=priv->arc_color1[1]+(diff1*(gdouble)(i)/(gdouble)split_trap);
          color_start1[2]=priv->arc_color1[2]+(diff2*(gdouble)(i)/(gdouble)split_trap);

          color_mid1[0]=priv->arc_color1[0]+(diff0*((gdouble)i+split_trap_frac)/split_trap);
          color_mid1[1]=priv->arc_color1[1]+(diff1*((gdouble)i+split_trap_frac)/split_trap);
          color_mid1[2]=priv->arc_color1[2]+(diff2*((gdouble)i+split_trap_frac)/split_trap);

          color_stop1[0]=priv->arc_color2[0]+(diff3*(1.0-split_trap_frac)/((gdouble)sections-split_trap));
          color_stop1[1]=priv->arc_color2[1]+(diff4*(1.0-split_trap_frac)/((gdouble)sections-split_trap));
          color_stop1[2]=priv->arc_color2[2]+(diff5*(1.0-split_trap_frac)/((gdouble)sections-split_trap));
        }
      else
        { 
          diff0=priv->arc_color3[0]-priv->arc_color2[0];
          diff1=priv->arc_color3[1]-priv->arc_color2[1];
          diff2=priv->arc_color3[2]-priv->arc_color2[2];
          color_mid1[0]=priv->arc_color2[0]+(diff0*((gdouble)i-split_trap)/((gdouble)sections-split_trap));
          color_mid1[1]=priv->arc_color2[1]+(diff1*((gdouble)i-split_trap)/((gdouble)sections-split_trap));
          color_mid1[2]=priv->arc_color2[2]+(diff2*((gdouble)i-split_trap)/((gdouble)sections-split_trap));
          color_stop1[0]=priv->arc_color2[0]+(diff0*((gdouble)i-split_trap+1.0)/((gdouble)sections-split_trap));
          color_stop1[1]=priv->arc_color2[1]+(diff1*((gdouble)i-split_trap+1.0)/((gdouble)sections-split_trap));
          color_stop1[2]=priv->arc_color2[2]+(diff2*((gdouble)i-split_trap+1.0)/((gdouble)sections-split_trap));
        }

       //Draw the trapezoid or ring gradient.
       if(mid_color_pos==100)
         {
           draw_arc_gradient(da, cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 0);
         } 
       else if(i<split_trap_int)
         {  
           draw_arc_gradient(da, cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 1);
         }
       else if(mid_color_pos<100&&(gint)split_trap_int==i)
         {
           trap_cos1=prev_cos1+split_trap_frac*(temp_cos1-prev_cos1);
           trap_sin1=prev_sin1+split_trap_frac*(temp_sin1-prev_sin1);
           trap_cos2=prev_cos2+split_trap_frac*(temp_cos2-prev_cos2);
           trap_sin2=prev_sin2+split_trap_frac*(temp_sin2-prev_sin2);
           //Draw before and after the split in the trapezoid.
           draw_arc_gradient(da, cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, trap_cos1, trap_sin1, trap_cos2, trap_sin2, color_start1, color_mid1, color_stop1, 1);
           draw_arc_gradient(da, cr, trap_cos1, trap_sin1, trap_cos2, trap_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 2);
         } 
       else
         {
           draw_arc_gradient(da, cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 2);
         }     
      
       //Save previous values.
       prev_cos1=temp_cos1;
       prev_sin1=temp_sin1;
       prev_cos2=temp_cos2;
       prev_sin2=temp_sin2;
     }
}
static void draw_arc_gradient(GtkWidget *da, cairo_t *cr, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3, gdouble x4, gdouble y4, gdouble color_start1[], gdouble color_mid1[], gdouble color_stop1[], gint gradient_id)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  //Draw the gradients.    
  cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
  cairo_mesh_pattern_begin_patch(pattern1);
  cairo_mesh_pattern_move_to(pattern1, x2, y2);
  cairo_mesh_pattern_line_to(pattern1, x4, y4);
  cairo_mesh_pattern_line_to(pattern1, x3, y3);
  cairo_mesh_pattern_line_to(pattern1, x1, y1);
  cairo_mesh_pattern_line_to(pattern1, x2, y2);

  //Draw the gradient across the whole arc.
  if(gradient_id==0)
    {
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_start1[0], color_start1[1], color_start1[2], priv->arc_color1[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2], priv->arc_color3[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2], priv->arc_color3[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_start1[0], color_start1[1], color_start1[2], priv->arc_color1[3]);
    }
  //Draw the gradient to the mid point.
  else if(gradient_id==1)
    {
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_start1[0], color_start1[1], color_start1[2], priv->arc_color1[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_mid1[0], color_mid1[1], color_mid1[2], priv->arc_color2[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_mid1[0], color_mid1[1], color_mid1[2], priv->arc_color2[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_start1[0], color_start1[1], color_start1[2], priv->arc_color1[3]);
    }
  //Draw the gradient from the mid point to the end.
  else
    {
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_mid1[0], color_mid1[1], color_mid1[2], priv->arc_color2[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2], priv->arc_color3[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2], priv->arc_color3[3]);
      cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_mid1[0], color_mid1[1], color_mid1[2], priv->arc_color2[3]);
    }
  cairo_mesh_pattern_end_patch(pattern1);
  cairo_set_source(cr, pattern1);
  cairo_paint(cr);
  cairo_pattern_destroy(pattern1);   
}
static void adjustable_gauge_finalize(GObject *object)
{ 
  AdjustableGauge *da=ADJUSTABLE_GAUGE(object);
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  
  g_free(priv->background_string);
  g_free(priv->text_color_string);
  g_free(priv->arc_color_string1);
  g_free(priv->arc_color_string2);
  g_free(priv->arc_color_string3);

  G_OBJECT_CLASS(adjustable_gauge_parent_class)->finalize(object);
}



















