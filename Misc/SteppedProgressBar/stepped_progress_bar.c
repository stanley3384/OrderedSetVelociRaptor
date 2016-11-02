
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
  //Colors for gradients as strings.
  gchar *background_string_rgba1;
  gchar *background_string_rgba2;
  gchar *foreground_string_rgba1;
  gchar *foreground_string_rgba2;
  //Arrays for gradient colors.
  gdouble background_rgba1[4];
  gdouble background_rgba2[4];
  gdouble foreground_rgba1[4];
  gdouble foreground_rgba2[4];
  //Variables for the stepped progress bar.
  guint progress_direction;
  gint steps;
  gint total_steps;
  gint step_stop;
  gdouble gradient_end;
};

enum
{
  PROP_0,
  BACKGROUND_RGBA1,
  BACKGROUND_RGBA2,
  FOREGROUND_RGBA1,
  FOREGROUND_RGBA2,
  PROGRESS_DIRECTION,
  STEPS,
  STEP_STOP
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

G_DEFINE_TYPE(SteppedProgressBar, stepped_progress_bar, GTK_TYPE_DRAWING_AREA)

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

  stepped_progress_bar_signals[CHANGED_SIGNAL]=g_signal_new("step-changed", G_TYPE_FROM_CLASS(klass), G_SIGNAL_RUN_FIRST|G_SIGNAL_ACTION, G_STRUCT_OFFSET(SteppedProgressBarClass, color_changed), NULL, NULL, g_cclosure_marshal_VOID__VOID, G_TYPE_NONE, 0);

  g_object_class_install_property(gobject_class, BACKGROUND_RGBA1, g_param_spec_string("background_rgba1", "background_rgba1", "background_rgba1", "background_rgba1", G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, BACKGROUND_RGBA2, g_param_spec_string("background_rgba2", "background_rgba2", "background_rgba2", "background_rgba2", G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, FOREGROUND_RGBA1, g_param_spec_string("foreground_rgba1", "foreground_rgba1", "foreground_rgba1", "foreground_rgba1", G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, FOREGROUND_RGBA2, g_param_spec_string("foreground_rgba2", "foreground_rgba2", "foreground_rgba2", "foreground_rgba2", G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, PROGRESS_DIRECTION, g_param_spec_int("progress_direction", "progress_direction", "progress_direction", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, STEPS, g_param_spec_int("steps", "steps", "steps", 0, 1, 0, G_PARAM_READWRITE));

  g_object_class_install_property(gobject_class, STEP_STOP, g_param_spec_int("step_stop", "step_stop", "step_stop", 0, 1, 0, G_PARAM_READWRITE));

}
//Needed for g_object_set().
static void stepped_progress_bar_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  SteppedProgressBar *da=STEPPED_PROGRESS_BAR(object);

  switch(prop_id)
  {
    case BACKGROUND_RGBA1:
      stepped_progress_bar_set_background_rgba1(da, g_value_get_string(value));
      break;
    case BACKGROUND_RGBA2:
      stepped_progress_bar_set_background_rgba2(da, g_value_get_string(value));
      break;
    case FOREGROUND_RGBA1:
      stepped_progress_bar_set_foreground_rgba1(da, g_value_get_string(value));
      break;
    case FOREGROUND_RGBA2:
      stepped_progress_bar_set_foreground_rgba2(da, g_value_get_string(value));
      break;
    case PROGRESS_DIRECTION:
      stepped_progress_bar_set_progress_direction(da, g_value_get_int(value));
      break;
    case STEPS:
      stepped_progress_bar_set_steps(da, g_value_get_int(value));
      break;
    case STEP_STOP:
      stepped_progress_bar_set_step_stop(da, g_value_get_int(value));
      break;  
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
void stepped_progress_bar_set_progress_direction(SteppedProgressBar *da, gint progress_direction)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
 
  if(progress_direction==0||progress_direction==1)
    {
      priv->progress_direction=progress_direction; 
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("Progress direction can be HORIZONTAL_RIGHT or VERTICAL_UP only.");
    }
}
void stepped_progress_bar_set_steps(SteppedProgressBar *da, gint steps)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
 
  //Steps range is 5 to 100.
  if(steps>4&&steps<101)
    { 
      priv->steps=steps;
      priv->total_steps=20*priv->steps;
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
  else
    {
      g_warning("The steps range is 4 < x < 101.");
    }
}
void stepped_progress_bar_set_step_stop(SteppedProgressBar *da, gint step_stop)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  gint total_steps=priv->steps;

  if(step_stop>=0&&step_stop<=total_steps)
    {
      priv->step_stop=step_stop;
      g_signal_emit_by_name((gpointer)da, "step-changed");
      gtk_widget_queue_draw(GTK_WIDGET(da));
    }
   else
    {
      g_warning("The step_stop range is 0 <= x<= steps.");
    }
} 
static void stepped_progress_bar_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  SteppedProgressBar *da=STEPPED_PROGRESS_BAR(object);
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  
  switch(prop_id)
  {
    case BACKGROUND_RGBA1:
      g_value_set_string(value, priv->background_string_rgba1);
      break;
    case BACKGROUND_RGBA2:
      g_value_set_string(value, priv->background_string_rgba2);
      break;
    case FOREGROUND_RGBA1:
      g_value_set_string(value, priv->foreground_string_rgba1);
      break;
    case FOREGROUND_RGBA2:
      g_value_set_string(value, priv->foreground_string_rgba2);
      break;
    case PROGRESS_DIRECTION:
      g_value_set_int(value, priv->progress_direction);
      break;
    case STEPS:
      g_value_set_int(value, priv->steps);
      break;
    case STEP_STOP:
      g_value_set_int(value, priv->step_stop);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
int stepped_progress_bar_get_progress_direction(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->progress_direction;
}
int stepped_progress_bar_get_steps(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->steps;
}
int stepped_progress_bar_get_step_stop(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->step_stop;
}
static void stepped_progress_bar_init(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  //Initialize default color strings.
  priv->background_string_rgba1=g_strdup("rgba(0, 255, 255, 255)");
  priv->background_string_rgba2=g_strdup("rgba(0, 0, 255, 255)");
  priv->foreground_string_rgba1=g_strdup("rgba(255, 255, 0, 255)");
  priv->foreground_string_rgba2=g_strdup("rgba(255, 0, 0, 255)");

  //Initailize default progress bar properties.
  priv->progress_direction=VERTICAL_UP;
  priv->steps=20;
  priv->total_steps=20*priv->steps;
  priv->step_stop=10;
  priv->gradient_end=19.0;

  //Initailize default gradient colors.
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

  if(priv->progress_direction==HORIZONTAL_RIGHT) stepped_progress_bar_horizontal_right_draw(da, cr);
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
      cairo_pattern_add_color_stop_rgba(pattern1, (gdouble)(i/(gdouble)priv->total_steps), priv->background_rgba1[0], priv->background_rgba1[1], priv->background_rgba1[2], priv->background_rgba1[3]); 
      cairo_pattern_add_color_stop_rgba(pattern1, (gdouble)(i+priv->gradient_end)/(gdouble)priv->total_steps, priv->background_rgba2[0], priv->background_rgba2[1], priv->background_rgba2[2], priv->background_rgba2[3]); 
    }
  cairo_set_source(cr, pattern1);
     
  cairo_rectangle(cr, 0, 0, width, height);
  cairo_fill(cr);

  //The foreground gradient.
  cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
  for(i=0;i<=priv->total_steps;i+=20)
    { 
      cairo_pattern_add_color_stop_rgba(pattern2, (gdouble)(i/(gdouble)priv->total_steps), priv->foreground_rgba1[0], priv->foreground_rgba1[1], priv->foreground_rgba1[2], priv->foreground_rgba1[3]); 
      cairo_pattern_add_color_stop_rgba(pattern2, (gdouble)(i+priv->gradient_end)/(gdouble)priv->total_steps, priv->foreground_rgba2[0], priv->foreground_rgba2[1], priv->foreground_rgba1[2], priv->foreground_rgba1[3]); 
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
     
  cairo_rectangle(cr, 0, 0, height, height);
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
  SteppedProgressBar *da=STEPPED_PROGRESS_BAR(object);
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  
  g_free(priv->background_string_rgba1);
  g_free(priv->background_string_rgba2);
  g_free(priv->foreground_string_rgba1);
  g_free(priv->foreground_string_rgba2);

  G_OBJECT_CLASS(stepped_progress_bar_parent_class)->finalize(object);
}
void stepped_progress_bar_set_background_rgba1(SteppedProgressBar *da, const gchar *background_string_rgba1)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, background_string_rgba1))
    {
      g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->background_rgba1[0]=rgba.red;
      priv->background_rgba1[1]=rgba.green;
      priv->background_rgba1[2]=rgba.blue;
      priv->background_rgba1[3]=rgba.alpha;
      if(priv->background_string_rgba1!=NULL) g_free(priv->background_string_rgba1);
      priv->background_string_rgba1=g_strdup(background_string_rgba1); 
    }
  else
    {
      g_print("background_string_rgba1 error\n");
    } 
}
const gchar* stepped_progress_bar_get_background_rgba1(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->background_string_rgba1;  
}

void stepped_progress_bar_set_background_rgba2(SteppedProgressBar *da, const gchar *background_string_rgba2)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, background_string_rgba2))
    {
      g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->background_rgba2[0]=rgba.red;
      priv->background_rgba2[1]=rgba.green;
      priv->background_rgba2[2]=rgba.blue;
      priv->background_rgba2[3]=rgba.alpha;
      if(priv->background_string_rgba2!=NULL) g_free(priv->background_string_rgba2);
      priv->background_string_rgba2=g_strdup(background_string_rgba2); 
    }
  else
    {
      g_print("background_string_rgba2 error\n");
    } 
}
const gchar* stepped_progress_bar_get_background_rgba2(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->background_string_rgba2;
} 

void stepped_progress_bar_set_foreground_rgba1(SteppedProgressBar *da, const gchar *foreground_string_rgba1)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, foreground_string_rgba1))
    {
      g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->foreground_rgba1[0]=rgba.red;
      priv->foreground_rgba1[1]=rgba.green;
      priv->foreground_rgba1[2]=rgba.blue;
      priv->foreground_rgba1[3]=rgba.alpha;
      if(priv->foreground_string_rgba1!=NULL) g_free(priv->foreground_string_rgba1);
      priv->foreground_string_rgba1=g_strdup(foreground_string_rgba1); 
    }
  else
    {
      g_print("foreground_string_rgba1 error\n");
    }   
}
const gchar* stepped_progress_bar_get_foreground_rgba1(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->foreground_string_rgba1;
} 
 
void stepped_progress_bar_set_foreground_rgba2(SteppedProgressBar *da, const gchar *foreground_string_rgba2)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);

  GdkRGBA rgba;
  if(gdk_rgba_parse(&rgba, foreground_string_rgba2))
    {
      g_print("red %f, green %f, blue %f, alpha %f\n", rgba.red, rgba.green, rgba.blue, rgba.alpha);
      priv->foreground_rgba2[0]=rgba.red;
      priv->foreground_rgba2[1]=rgba.green;
      priv->foreground_rgba2[2]=rgba.blue;
      priv->foreground_rgba2[3]=rgba.alpha;
      if(priv->foreground_string_rgba2!=NULL) g_free(priv->foreground_string_rgba2);
      priv->foreground_string_rgba2=g_strdup(foreground_string_rgba2); 
    }
  else
    {
      g_print("foreground_string_rgba2 error\n");
    }   
}
const gchar* stepped_progress_bar_get_foreground_rgba2(SteppedProgressBar *da)
{
  SteppedProgressBarPrivate *priv=STEPPED_PROGRESS_BAR_GET_PRIVATE(da);
  return priv->foreground_string_rgba2;
}   


















