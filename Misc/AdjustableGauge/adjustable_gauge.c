
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
  gint gauge_drawing_name;
};

enum
{
  PROP_0,
  FIRST_CUTOFF,
  SECOND_CUTOFF,
  NEEDLE,
  SCALE_BOTTOM,
  SCALE_TOP,
  GAUGE_DRAWING_NAME
};

//Private functions.
static void adjustable_gauge_class_init(AdjustableGaugeClass *klass);
static void adjustable_gauge_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void adjustable_gauge_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void adjustable_gauge_init(AdjustableGauge *da);
static gboolean adjustable_gauge_draw(GtkWidget *widget, cairo_t *cr);
static void adjustable_voltage_gauge_draw(GtkWidget *da, cairo_t *cr);
static void adjustable_speedometer_gauge_draw(GtkWidget *da, cairo_t *cr);

GType adjustable_gauge_get_type(void)
{
  static GType entry_type=0;
  if(!entry_type)
    {
      static const GTypeInfo entry_info={sizeof(AdjustableGaugeClass), NULL, NULL, (GClassInitFunc) adjustable_gauge_class_init, NULL, NULL, sizeof(AdjustableGauge), 0, (GInstanceInitFunc)adjustable_gauge_init};
      entry_type=g_type_register_static(GTK_TYPE_DRAWING_AREA, "AdjustableGauge", &entry_info, 0);
    }

  return entry_type;
}
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

  g_type_class_add_private(klass, sizeof(AdjustableGaugePrivate));

  g_object_class_install_property(gobject_class, FIRST_CUTOFF, g_param_spec_double("first_cutoff", "first_cutoff", "first_cutoff", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SECOND_CUTOFF, g_param_spec_double("second_cutoff", "second_cutoff", "second_cutoff", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, NEEDLE, g_param_spec_double("needle", "needle", "needle", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SCALE_BOTTOM, g_param_spec_double("scale_bottom", "scale_bottom", "scale_bottom", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SCALE_TOP, g_param_spec_double("scale_top", "scale_top", "scale_top", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, SCALE_TOP, g_param_spec_int("gauge_drawing_name", "gauge_drawing_name", "gauge_drawing_name", 0, 1, 0, G_PARAM_READWRITE));

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
    case GAUGE_DRAWING_NAME:
      adjustable_gauge_set_drawing(da, g_value_get_int(value));
      break; 
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void adjustable_gauge_set_first_cutoff(AdjustableGauge *da, gdouble first_cutoff)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
 
  //Steps first_cutoff >= 0.
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
 
  //Steps second_cutoff >= 0.
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

  if(scale_bottom>=0.0)
    {
      priv->scale_bottom=scale_bottom;
      priv->needle=scale_bottom;
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
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("The top of the scale is out of range.");
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
    case GAUGE_DRAWING_NAME:
      g_value_set_int(value, priv->gauge_drawing_name);
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
gint adjustable_gauge_get_drawing(AdjustableGauge *da)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);
  return priv->gauge_drawing_name;
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
  priv->gauge_drawing_name=0;
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

  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);
  gint center_x=width/2;
  gint center_y=height/2;
  //Original drawing 400x400.
  gdouble scale_y=(gdouble)height/400.0;
    
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  cairo_paint(cr);

  //transforms
  cairo_translate(cr, center_x, center_y+(50*scale_y));
  cairo_scale(cr, 1.30*scale_y, 1.30*scale_y);

  //Green underneath 
  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
  cairo_set_line_width(cr, 3.0);
  cairo_arc_negative(cr, 0, 0, 100, 23.0*G_PI/12.0, 13.0*G_PI/12.0);
  cairo_line_to(cr, -(cos(G_PI/12.0)*150), -sin(G_PI/12.0)*150);
  cairo_arc(cr, 0, 0, 150, 13.0*G_PI/12.0, 23.0*G_PI/12.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Difference of top and bottom used to standardize values.
  gdouble diff=priv->scale_top-priv->scale_bottom;

  //Draw yellow next. Standardized on 13 to 23 scale.
  gdouble standard_first_cutoff=(((priv->first_cutoff-priv->scale_bottom)/diff)*10.0)+13.0;
  cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
  cairo_arc_negative(cr, 0, 0, 100, 23.0*G_PI/12.0, standard_first_cutoff*G_PI/12.0);
  cairo_arc(cr, 0, 0, 150, standard_first_cutoff*G_PI/12.0, 23.0*G_PI/12.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Draw red top. Standardized on 13 to 23 scale.
  gdouble standard_second_cutoff=(((priv->second_cutoff-priv->scale_bottom)/diff)*10.0)+13.0;
  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
  cairo_arc_negative(cr, 0, 0, 100, 23.0*G_PI/12.0, standard_second_cutoff*G_PI/12.0);
  cairo_arc(cr, 0, 0, 150, standard_second_cutoff*G_PI/12.0, 23.0*G_PI/12.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Needle line between 1 and 11. Standardize needle on this scale.
  gdouble standard_needle=(((priv->needle-priv->scale_bottom)/diff)*10.0)+1.0;
  //g_print("N %f St %f T %f B %f diff %f\n", needle, standard_needle, scale_top, scale_bottom, diff);
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
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, -(cos(standard_needle*G_PI/12.0)*150), -sin(standard_needle*G_PI/12.0)*150);
  cairo_stroke(cr);

  //Text for needle value.
  cairo_text_extents_t extents1;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 46);
  gchar *string1=g_strdup_printf("%3.2f", priv->needle);
  cairo_text_extents(cr, string1, &extents1); 
  cairo_move_to(cr, -extents1.width/2, 40.0+extents1.height/2);  
  cairo_show_text(cr, string1);
  g_free(string1);

  //Text for bottom end scale value.
  cairo_text_extents_t extents2;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 22);
  gchar *string2=g_strdup_printf("%3.2f", priv->scale_bottom);
  cairo_text_extents(cr, string2, &extents2); 
  cairo_move_to(cr, -extents2.width/2-125, extents2.height/2-10);  
  cairo_show_text(cr, string2);
  g_free(string2);

  //Text for top end scale value.
  cairo_text_extents_t extents3;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 22);
  gchar *string3=g_strdup_printf("%3.2f", priv->scale_top);
  cairo_text_extents(cr, string3, &extents3); 
  cairo_move_to(cr, -extents3.width/2+125, extents3.height/2-10);  
  cairo_show_text(cr, string3);
  g_free(string3);

}
static void adjustable_speedometer_gauge_draw(GtkWidget *da, cairo_t *cr)
{
  AdjustableGaugePrivate *priv=ADJUSTABLE_GAUGE_GET_PRIVATE(da);

  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);
  gint center_x=width/2;
  gint center_y=height/2;
  gdouble scale_y=(gdouble)height/400.0;
    
  cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
  cairo_paint(cr);

  //transforms
  cairo_translate(cr, center_x, center_y+(scale_y));
  cairo_scale(cr, 1.10*scale_y, 1.10*scale_y);

  //Green underneath 
  cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
  cairo_set_line_width(cr, 3.0);
  cairo_arc_negative(cr, 0, 0, 80, -5.0*G_PI/3.0, -4.0*G_PI/3.0);
  cairo_line_to(cr, (cos(-4.0*G_PI/3.0)*150), sin(-4.0*G_PI/3.0)*150);
  cairo_arc(cr, 0, 0, 150, -4.0*G_PI/3.0, -5.0*G_PI/3.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  gdouble diff=priv->scale_top-priv->scale_bottom;

  //Yellow next.
  gdouble standard_first_cutoff=(((priv->first_cutoff-priv->scale_bottom)/diff)*(5.0*G_PI/3.0));
  cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
  cairo_arc_negative(cr, 0, 0, 80, -5.0*G_PI/3.0, -4.0*G_PI/3.0+standard_first_cutoff);
  cairo_arc(cr, 0, 0, 150, -4.0*G_PI/3.0+standard_first_cutoff, -5.0*G_PI/3.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Red top.
  gdouble standard_second_cutoff=(((priv->second_cutoff-priv->scale_bottom)/diff)*(5.0*G_PI/3.0));
  cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
  cairo_arc_negative(cr, 0, 0, 80, -5.0*G_PI/3.0, -4.0*G_PI/3.0+standard_second_cutoff);
  cairo_arc(cr, 0, 0, 150, -4.0*G_PI/3.0+standard_second_cutoff, -5.0*G_PI/3.0);
  cairo_close_path(cr);
  cairo_fill(cr);
  cairo_stroke(cr);

  //Set large tick marks.
  gint i=0;
  cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
  gdouble tenth_scale=diff/10.0;
  gdouble tick_mark=(5.0*G_PI/3.0)/10.0;
  gdouble temp=0;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
  cairo_text_extents_t tick_extents;
  cairo_set_font_size(cr, 20);
  cairo_move_to(cr, 0, 0);
  for(i=0;i<11;i++)
    {
      temp=(gdouble)i*tick_mark;
      cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*140, -sin((4.0*G_PI/3.0)-temp)*140);
      cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*150, -sin((4.0*G_PI/3.0)-temp)*150);
      cairo_stroke(cr);
      //String values at bit tick marks.
      gchar *tick_string=g_strdup_printf("%i", (int)((gdouble)i*tenth_scale));
      cairo_text_extents(cr, tick_string, &tick_extents);
      cairo_move_to(cr, (cos((4.0*G_PI/3.0)-temp)*115)-tick_extents.width/2.0, (-sin((4.0*G_PI/3.0)-temp)*115)+tick_extents.height/2.0);
      cairo_show_text(cr, tick_string);
      g_free(tick_string);
      //Reset position to the center.
      cairo_move_to(cr, 0, 0);
    }

  //Set small tick marks.
  cairo_set_line_width(cr, 3.0);
  gdouble half_tick=tick_mark/2.0;
  cairo_move_to(cr, 0, 0);
  for(i=0;i<10;i++)
    {
      temp=(gdouble)i*tick_mark+half_tick;
      cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*145, -sin((4.0*G_PI/3.0)-temp)*145);
      cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*150, -sin((4.0*G_PI/3.0)-temp)*150);
      cairo_stroke(cr);
      cairo_move_to(cr, 0, 0);
    }

  //The needle line.
  cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
  gdouble standard_needle=(((priv->needle-priv->scale_bottom)/diff)*(5.0*G_PI/3.0));
  cairo_move_to(cr, 0, 0);
  cairo_line_to(cr, cos((4.0*G_PI/3.0)-standard_needle)*90, -sin((4.0*G_PI/3.0)-standard_needle)*90);
  cairo_stroke(cr);
    
  //Text for needle value.
  cairo_text_extents_t extents1;
  cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
  cairo_set_font_size(cr, 30);
  gchar *string1=g_strdup_printf("%i", (gint)priv->needle);
  cairo_text_extents(cr, string1, &extents1); 
  cairo_move_to(cr, -extents1.width/2, 140.0+extents1.height/2);  
  cairo_show_text(cr, string1);
  g_free(string1);

}



















