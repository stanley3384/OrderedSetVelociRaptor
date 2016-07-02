
/*

    For use with stepped_progress_bar_main.c. Look in stepped_progress_bar_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "stepped_progress_bar.h"

#define STEPPED_PROGRESS_BAR_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), STEPPED_PROGRESS_BAR_TYPE, SteppedProgressBarPrivate))

typedef struct _SteppedProgressBarPrivate SteppedProgressBarPrivate;

struct _SteppedProgressBarPrivate
{
  //Just some test variables that need to be removed.
  gchar *progress_name;
  gdouble color_rgba[4];
  //Variables for the stepped progress bar.
  guint progress_direction;
  gint steps;
  gint total_steps;
  gint step_stop;
  gdouble gradient_end;
  gdouble background_rgba1[4];
  gdouble background_rgba2[4];
  gdouble foreground_rgba1[4];
  gdouble foreground_rgba2[4];
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

static guint stepped_progress_bar_signals[LAST_SIGNAL]={0};

//Private functions.
static void stepped_progress_bar_class_init(SteppedProgressBarClass *klass);
static void stepped_progress_bar_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void stepped_progress_bar_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void stepped_progress_bar_init(SteppedProgressBar *da);
static gboolean stepped_progress_bar_draw(GtkWidget *widget, cairo_t *cr);
static void stepped_progress_bar_horizontal_right_draw(GtkWidget *da, cairo_t *cr);
static void stepped_progress_bar_vertical_up_draw(GtkWidget *da, cairo_t *cr);
static void stepped_progress_bar_finalize(GObject *gobject);

GType stepped_progress_bar_get_type(void)
{
  static GType entry_type=0;
  if(!entry_type)
    {
      static const GTypeInfo entry_info={sizeof(SteppedProgressBarClass), NULL, NULL, (GClassInitFunc) stepped_progress_bar_class_init, NULL, NULL, sizeof(SteppedProgressBar), 0, (GInstanceInitFunc)stepped_progress_bar_init};
      entry_type=g_type_register_static(GTK_TYPE_DRAWING_AREA, "SteppedProgressBar", &entry_info, 0);
    }

  return entry_type;
}
static void stepped_progress_bar_class_init(SteppedProgressBarClass *klass)
{ 
  GObjectClass *gobject_class;
  GtkWidgetClass *widget_class;

  gobject_class=(GObjectClass*)klass;
  widget_class=(GtkWidgetClass*)klass;

  //Set the property funtions.
  gobject_class->set_property=stepped_progress_bar_set_property;
  gobject_class->get_property=stepped_progress_bar_get_property;

  //Draw when first shown.
  widget_class->draw=stepped_progress_bar_draw;
  gobject_class->finalize = stepped_progress_bar_finalize;

  g_type_class_add_private(klass, sizeof(SteppedProgressBarPrivate));

  stepped_progress_bar_signals[CHANGED_SIGNAL]=g_signal_new("color-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST|G_SIGNAL_ACTION, G_STRUCT_OFFSET(SteppedProgressBarClass, color_changed), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  g_object_class_install_property(gobject_class, NAME, g_param_spec_string("ProgressName", "ProgressName", "Some Progress Name", "Progress", G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, RED, g_param_spec_double("Red", "Red Channel", "Channel 1", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, GREEN, g_param_spec_double("Green", "Green Channel", "Channel 2", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, BLUE, g_param_spec_double("Blue", "Blue Channel", "Channel 3", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, ALPHA, g_param_spec_double("Alpha", "Alpha Channel", "Channel 4", 0, 1, 0, G_PARAM_READWRITE));

}
//Needed for g_object_set().
static void stepped_progress_bar_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  SteppedProgressBar *da=STEPPED_PROGRESS_BAR(object);
  gdouble color_rgba[4]={-1.0, -1.0, -1.0, -1.0};

  switch(prop_id)
  {
    case NAME:
      stepped_progress_bar_set_name(da, g_value_get_string(value));
      break;
    case RED:
      color_rgba[0]=g_value_get_double(value);
      stepped_progress_bar_set_color(da, color_rgba);
      break;
    case GREEN:
      color_rgba[1]=g_value_get_double(value);
      stepped_progress_bar_set_color(da, color_rgba);
      break;
    case BLUE:
      color_rgba[2]=g_value_get_double(value);
      stepped_progress_bar_set_color(da, color_rgba);
      break;  
    case ALPHA:
      color_rgba[3]=g_value_get_double(value);
      stepped_progress_bar_set_color(da, color_rgba);
      break; 
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;

  }
}
void stepped_progress_bar_set_color(SteppedProgressBar *da, gdouble color_rgba[4])
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
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
static void stepped_progress_bar_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  SteppedProgressBar *da=STEPPED_PROGRESS_BAR(object);
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case NAME:
      g_value_set_string(value, priv->progress_name);
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
gchar* stepped_progress_bar_get_color(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return g_strdup_printf("%f, %f, %f, %f", priv->color_rgba[0], priv->color_rgba[1], priv->color_rgba[2], priv->color_rgba[3]);
}
static void stepped_progress_bar_init(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  gint i=0;

  //Initialize progress name.
  priv->progress_name=g_strdup("Progress");

  //Initialize color array.
  for(i=0;i<4;i++) priv->color_rgba[i]=0.0;

  //Set some test variables.
  priv->progress_direction=1;
  priv->steps=20;
  priv->total_steps=20*priv->steps;
  priv->step_stop=10;
  priv->gradient_end=19.0;
  //Setup default gradient colors.
  priv->background_rgba1[0]=0.0;
  priv->background_rgba1[1]=1.0;
  priv->background_rgba1[2]=1.0;
  priv->background_rgba1[3]=1.0;
  priv->background_rgba2[0]=0.0;
  priv->background_rgba2[1]=0.0;
  priv->background_rgba2[2]=1.0;
  priv->background_rgba2[3]=1.0;
  priv->foreground_rgba1[0]=1.0;
  priv->foreground_rgba1[1]=1.0;
  priv->foreground_rgba1[2]=0.0;
  priv->foreground_rgba1[3]=1.0;
  priv->foreground_rgba2[0]=1.0;
  priv->foreground_rgba2[1]=0.0;
  priv->foreground_rgba2[2]=0.0;
  priv->foreground_rgba2[3]=1.0;
}
GtkWidget* stepped_progress_bar_new()
{
  return GTK_WIDGET(g_object_new(stepped_progress_bar_get_type(), NULL));
}
static gboolean stepped_progress_bar_draw(GtkWidget *da, cairo_t *cr)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  if(priv->progress_direction==0) stepped_progress_bar_horizontal_right_draw(da, cr);
  else stepped_progress_bar_vertical_up_draw(da, cr);
  return FALSE;
}
static void stepped_progress_bar_horizontal_right_draw(GtkWidget *da, cairo_t *cr)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  gint width=gtk_widget_get_allocated_width(da);
  gint height=gtk_widget_get_allocated_height(da);
  gint i=0;
    
  //The background gradient.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
  for(i=0;i<=priv->total_steps;i+=20)
    { 
      cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i/(gdouble)priv->total_steps), 0.0, 1.0, 1.0); 
      cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i+priv->gradient_end)/(gdouble)priv->total_steps, 0.0, 0.0, 1.0); 
    }
  cairo_set_source(cr, pattern1);
     
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);

  //The foreground gradient.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
  for(i=0;i<=priv->total_steps;i+=20)
    { 
      cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i/(gdouble)priv->total_steps), 1.0, 1.0, 0.0); 
      cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i+priv->gradient_end)/(gdouble)priv->total_steps, 1.0, 0.0, 0.0); 
    }
  cairo_set_source(cr, pattern2);
     
  cairo_rectangle(cr, 0, 0, ((gdouble)priv->step_stop/(gdouble)priv->steps)*(gdouble)width, height);
  cairo_fill(cr);

  cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
  cairo_set_line_width(cr, 6);
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_stroke(cr);

  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
}
static void stepped_progress_bar_vertical_up_draw(GtkWidget *da, cairo_t *cr)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  gint width=gtk_widget_get_allocated_width(da)*10;
  gint height=gtk_widget_get_allocated_height(da);
  gint i=0;
    
  //Transforms for drawing.
  cairo_save(cr);
  if(width>height) cairo_scale(cr, (gdouble)width/(gdouble)height, 1.0);
  cairo_rotate(cr, G_PI/2.0);
  cairo_translate(cr, 0.0, -height);

  //The background pattern.
  cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0, height, 0.0);
  for(i=0;i<=priv->total_steps;i+=20)
    { 
      cairo_pattern_add_color_stop_rgba(pattern1, (gdouble)(i/(gdouble)priv->total_steps), priv->background_rgba1[0], priv->background_rgba1[1], priv->background_rgba1[2], priv->background_rgba1[3]); 
      cairo_pattern_add_color_stop_rgba(pattern1, (gdouble)(i+priv->gradient_end)/(gdouble)priv->total_steps, priv->background_rgba2[0], priv->background_rgba2[1], priv->background_rgba2[2], priv->background_rgba2[3]); 
    }
  cairo_set_source(cr, pattern1);
     
  cairo_rectangle(cr, 0, 0, height, width);
  cairo_fill(cr);

  //The foreground pattern.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, height, 0.0);
  for(i=0;i<=priv->total_steps;i+=20)
    { 
      cairo_pattern_add_color_stop_rgba(pattern2, (gdouble)(i/(gdouble)priv->total_steps), priv->foreground_rgba1[0], priv->foreground_rgba1[1], priv->foreground_rgba1[2], priv->foreground_rgba1[3]); 
      cairo_pattern_add_color_stop_rgba(pattern2, (gdouble)(i+priv->gradient_end)/(gdouble)priv->total_steps, priv->foreground_rgba2[0], priv->foreground_rgba2[1], priv->foreground_rgba2[2], priv->foreground_rgba2[3]); 
    }
  cairo_set_source(cr, pattern2);
     
  cairo_rectangle(cr, (1.0-(priv->step_stop/(gdouble)priv->steps))*height, 0, (priv->step_stop/(gdouble)priv->steps)*height, height);
  cairo_fill(cr);

  cairo_restore(cr);
  cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
  cairo_set_line_width(cr, 6);
  cairo_rectangle(cr, 0, 0, width/10, height);
  cairo_stroke(cr);

  cairo_pattern_destroy(pattern1);
  cairo_pattern_destroy(pattern2);
}
static void stepped_progress_bar_finalize(GObject *object)
{
  g_print("Finalize\n");
  SteppedProgressBar *da=STEPPED_PROGRESS_BAR(object);
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  
  g_free(priv->progress_name);
}
void stepped_progress_bar_set_name(SteppedProgressBar *da, const gchar *progress_name)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  if(priv->progress_name!=NULL) g_free(priv->progress_name);
  priv->progress_name=g_strdup(progress_name);  
}
const gchar* stepped_progress_bar_get_name(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->progress_name;  
}


