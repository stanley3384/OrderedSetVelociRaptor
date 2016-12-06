
/*

    For use with penrose_main.c. Look in penrose_main2.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "drawing2.h"

#define PENROSE_TRIANGLE_DRAWING_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), PENROSE_TRIANGLE_DRAWING_TYPE, PenroseTriangleDrawingPrivate))

typedef struct _PenroseTriangleDrawingPrivate PenroseTriangleDrawingPrivate;

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
 
}
static void penrose_triangle_drawing_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
{
  //No properties to set.
  switch(prop_id)
  {    
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
      break;
  }
}
static void penrose_triangle_drawing_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
{
  //No properties to get
  switch(prop_id)
  {
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
  }
}
static void penrose_triangle_drawing_init(PenroseTriangleDrawing *da)
{
  //Set the name for CSS
  gtk_widget_set_name(GTK_WIDGET(da), "drawing2");

  //Get the CSS colors.
  GError *css_error=NULL;
  GtkCssProvider *provider=gtk_css_provider_new();
  GtkStyleContext *context=gtk_widget_get_style_context(GTK_WIDGET(da));
  gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
  gtk_css_provider_load_from_path(provider, "drawing2.css", &css_error);
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

  //Paint background with CSS values.
  GtkStyleContext *context=gtk_widget_get_style_context(GTK_WIDGET(da));
  gtk_render_background(context, cr, 0, 0, width, height);
  //Set drawing color with CSS values.
  GdkRGBA color;
  gtk_style_context_get_color(context, GTK_STATE_FLAG_NORMAL, &color);
  //g_print("%s\n", gdk_rgba_to_string(&color));
  cairo_set_source_rgba(cr, color.red, color.green, color.blue, color.alpha);

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
  G_OBJECT_CLASS(penrose_triangle_drawing_parent_class)->finalize(object); 
}

