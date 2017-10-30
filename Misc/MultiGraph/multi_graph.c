
/*

    For use with multi_graph_main.c. Look in multi_graph_main.c for more information.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "multi_graph.h"
#include<stdlib.h>

#define MULTI_GRAPH_GET_PRIVATE(obj) (G_TYPE_INSTANCE_GET_PRIVATE((obj), MULTI_GRAPH_TYPE, MultiGraphPrivate))

typedef struct _MultiGraphPrivate MultiGraphPrivate;

struct _MultiGraphPrivate
  {    
    gdouble background_color[4];
    gchar *background_color_string;
    gdouble font_color[4];
    gchar *font_color_string;
    gdouble grid_color[4];
    gchar *grid_color_string;
    gdouble tick_color[4];
    gchar *tick_color_string;
    //The array of line colors.
    gdouble (*lc)[4];
    gint graph_rows;
    gint graph_columns;
    gdouble test_increment_x;
    GArray *x_ticks;
    GArray *y_ticks;
    GArray *y_max;
    GArray *data_points;
    gint x_font_scale;
    gint y_font_scale;
    gint draw_lines;
    gint scale_dots;
    gint compose;
  };

enum
  {
    PROP_0,
    MULTI_GRAPH_BACKGROUND_COLOR,
    MULTI_GRAPH_FONT_COLOR,
    MULTI_GRAPH_GRID_COLOR,
    MULTI_GRAPH_TICK_COLOR,
    MULTI_GRAPH_ROWS,
    MULTI_GRAPH_COLUMNS,
    MULTI_GRAPH_X_FONT_SCALE,
    MULTI_GRAPH_Y_FONT_SCALE,
    MULTI_GRAPH_DRAW_LINES,
    MULTI_GRAPH_SCALE_DOTS,
    MULTI_GRAPH_COMPOSE
  };

struct point{
  gdouble x;
  gdouble y;
};
struct controls{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
};

//Colors for the 16 graphs.
static gdouble lc[16][4]=
{
  {1.0, 1.0, 0.0, 1.0},
  {0.0, 1.0, 0.0, 1.0},
  {0.0, 1.0,  1.0, 1.0},
  {1.0,  0.0,  1.0, 1.0},
  {1.0, 0.0, 0.0, 1.0},
  {1.0, 1.0, 0.5, 1.0},
  {0.5, 1.0,  0.5, 1.0},
  {0.5,  1.0,  1.0, 1.0},
  {1.0, 0.5, 1.0, 1.0},
  {1.0, 0.5, 0.5, 1.0},
  {1.0, 0.75, 0.25, 1.0},
  {0.75, 1.0,  0.25, 1.0},
  {0.25,  0.75,  1.0, 1.0},
  {1.0, 0.25, 0.75, 1.0},
  {1.0, 0.25, 0.25, 1.0},
  {0.25,  0.5,  0.5, 1.0}
};

//Private functions.
static void multi_graph_class_init(MultiGraphClass *klass);
static void multi_graph_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec);
static void multi_graph_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec);
static void multi_graph_init(MultiGraph *da);
static gboolean multi_graph_draw(GtkWidget *widget, cairo_t *cr);
//Bezier control points from coordinates for smoothing.
static GArray* control_points_from_coords2(const GArray *dataPoints);

static void multi_graph_finalize(GObject *gobject);

G_DEFINE_TYPE(MultiGraph, multi_graph, GTK_TYPE_DRAWING_AREA)

static void multi_graph_class_init(MultiGraphClass *klass)
  { 
    GObjectClass *gobject_class;
    GtkWidgetClass *widget_class;

    gobject_class=(GObjectClass*)klass;
    widget_class=(GtkWidgetClass*)klass;

    //Set the property funtions.
    gobject_class->set_property=multi_graph_set_property;
    gobject_class->get_property=multi_graph_get_property;

    //Draw when first shown.
    widget_class->draw=multi_graph_draw;
    gobject_class->finalize = multi_graph_finalize;

    g_type_class_add_private(klass, sizeof(MultiGraphPrivate));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_BACKGROUND_COLOR, g_param_spec_string("background_color", "background_color", "background_color", NULL, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_FONT_COLOR, g_param_spec_string("font_color", "font_color", "font_color", NULL, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_GRID_COLOR, g_param_spec_string("grid_color", "grid_color", "grid_color", NULL, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_TICK_COLOR, g_param_spec_string("tick_color", "tick_color", "tick_color", NULL, G_PARAM_READWRITE));
    
    g_object_class_install_property(gobject_class, MULTI_GRAPH_ROWS, g_param_spec_int("multi_graph_rows", "multi_graph_rows", "multi_graph_rows", 1, 16, 1, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_COLUMNS, g_param_spec_int("multi_graph_columns", "multi_graph_columns", "multi_graph_columns", 1, 16, 1, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_X_FONT_SCALE, g_param_spec_int("multi_graph_x_font_scale", "multi_graph_x_font_scale", "multi_graph_x_font_scale", -20, 20, 0, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_Y_FONT_SCALE, g_param_spec_int("multi_graph_y_font_scale", "multi_graph_y_font_scale", "multi_graph_y_font_scale", -20, 20, 0, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_DRAW_LINES, g_param_spec_int("multi_graph_draw_lines", "multi_graph_draw_lines", "multi_graph_draw_lines", 0, 4, 0, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_SCALE_DOTS, g_param_spec_int("multi_graph_scale_dots", "multi_graph_scale_dots", "multi_graph_scale_dots", -20, 20, 0, G_PARAM_READWRITE));

    g_object_class_install_property(gobject_class, MULTI_GRAPH_COMPOSE, g_param_spec_int("multi_graph_compose", "multi_graph_compose", "multi_graph_compose", 0, 15, 0, G_PARAM_READWRITE));

  }
//Needed for g_object_set().
static void multi_graph_set_property(GObject *object, guint prop_id, const GValue *value, GParamSpec *pspec)
  {
    MultiGraph *da=MULTI_GRAPH(object);

    switch(prop_id)
    {
      g_print("Prop %i\n", prop_id);
      case MULTI_GRAPH_BACKGROUND_COLOR:
        multi_graph_set_background_color(da, g_value_get_string(value));
        break;
      case MULTI_GRAPH_FONT_COLOR:
        multi_graph_set_font_color(da, g_value_get_string(value));
        break;
      case MULTI_GRAPH_GRID_COLOR:
        multi_graph_set_grid_color(da, g_value_get_string(value));
        break;
      case MULTI_GRAPH_TICK_COLOR:
        multi_graph_set_tick_color(da, g_value_get_string(value));
        break;
      case MULTI_GRAPH_ROWS:
        multi_graph_set_rows(da, g_value_get_int(value));
        break;
      case MULTI_GRAPH_COLUMNS:
        multi_graph_set_columns(da, g_value_get_int(value));
        break;
      case MULTI_GRAPH_X_FONT_SCALE:
        multi_graph_set_x_font_scale(da, g_value_get_int(value));
        break;
      case MULTI_GRAPH_Y_FONT_SCALE:
        multi_graph_set_y_font_scale(da, g_value_get_int(value));
        break;
      case MULTI_GRAPH_DRAW_LINES:
        multi_graph_set_draw_lines(da, g_value_get_int(value));
        break;
      case MULTI_GRAPH_SCALE_DOTS:
        multi_graph_set_scale_dots(da, g_value_get_int(value));
        break;
      case MULTI_GRAPH_COMPOSE:
        multi_graph_set_compose(da, g_value_get_int(value));
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
        break;
    }
  }
void multi_graph_set_background_color(MultiGraph *da, const gchar *background_color_string)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    GdkRGBA rgba;
    if(gdk_rgba_parse(&rgba, background_color_string))
      {
        priv->background_color[0]=rgba.red;
        priv->background_color[1]=rgba.green;
        priv->background_color[2]=rgba.blue;
        priv->background_color[3]=rgba.alpha;
        if(priv->background_color_string!=NULL) g_free(priv->background_color_string);
        priv->background_color_string=g_strdup(background_color_string); 
      }
    else
      {
        g_warning("background_color_string error\n");
      } 
  }
void multi_graph_set_font_color(MultiGraph *da, const gchar *font_color_string)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    GdkRGBA rgba;
    if(gdk_rgba_parse(&rgba, font_color_string))
      {
        priv->font_color[0]=rgba.red;
        priv->font_color[1]=rgba.green;
        priv->font_color[2]=rgba.blue;
        priv->font_color[3]=rgba.alpha;
        if(priv->font_color_string!=NULL) g_free(priv->font_color_string);
        priv->font_color_string=g_strdup(font_color_string); 
      }
    else
      {
        g_warning("font_color_string error\n");
      } 
  }
void multi_graph_set_grid_color(MultiGraph *da, const gchar *grid_color_string)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    GdkRGBA rgba;
    if(gdk_rgba_parse(&rgba, grid_color_string))
      {
        priv->grid_color[0]=rgba.red;
        priv->grid_color[1]=rgba.green;
        priv->grid_color[2]=rgba.blue;
        priv->grid_color[3]=rgba.alpha;
        if(priv->grid_color_string!=NULL) g_free(priv->grid_color_string);
        priv->grid_color_string=g_strdup(grid_color_string); 
      }
    else
      {
        g_warning("grid_color_string error\n");
      } 
  }
void multi_graph_set_tick_color(MultiGraph *da, const gchar *tick_color_string)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    GdkRGBA rgba;
    if(gdk_rgba_parse(&rgba, tick_color_string))
      {
        priv->tick_color[0]=rgba.red;
        priv->tick_color[1]=rgba.green;
        priv->tick_color[2]=rgba.blue;
        priv->tick_color[3]=rgba.alpha;
        if(priv->tick_color_string!=NULL) g_free(priv->tick_color_string);
        priv->tick_color_string=g_strdup(tick_color_string); 
      }
    else
      {
        g_warning("tick_color_string error\n");
      } 
  }
//Uses a static array. Doesn't set a color string or have a set get pair.
void multi_graph_set_line_color(MultiGraph *da, gint graph_id, const gchar *line_color_string)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    if(graph_id>=0&&graph_id<=15)
      {
        GdkRGBA rgba;
        if(gdk_rgba_parse(&rgba, line_color_string))
          {
            priv->lc[graph_id][0]=rgba.red;
            priv->lc[graph_id][1]=rgba.green;
            priv->lc[graph_id][2]=rgba.blue;
            priv->lc[graph_id][3]=rgba.alpha;
          }
        else
          {
            g_warning("line_color_string error\n");
          } 
      }
    else
      {
        g_warning("Graph id: 0<=x<=15\n");
      }
  }
void multi_graph_set_rows(MultiGraph *da, gint rows)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(rows>=1&&rows<=16)
      { 
        priv->graph_rows=rows;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph rows; 1<=x<=16.");
      }
  } 
void multi_graph_set_columns(MultiGraph *da, gint columns)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(columns>=1&&columns<=16)
      { 
        priv->graph_columns=columns;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph columns; 1<=x<=16.");
      }
  }
void multi_graph_set_x_font_scale(MultiGraph *da, gint x_font_scale)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(x_font_scale>=-20&&x_font_scale<=20)
      { 
        priv->x_font_scale=x_font_scale;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph x_font_scale; -20<=x<=20.");
      }
  }  
void multi_graph_set_y_font_scale(MultiGraph *da, gint y_font_scale)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(y_font_scale>=-20&&y_font_scale<=20)
      { 
        priv->y_font_scale=y_font_scale;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph y_font_scale; -20<=x<=20.");
      }
  }  
void multi_graph_set_draw_lines(MultiGraph *da, gint draw_lines)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(draw_lines>=0&&draw_lines<=3)
      { 
        priv->draw_lines=draw_lines;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph draw_lines; 0<=x<=3.");
      }
  } 
void multi_graph_set_scale_dots(MultiGraph *da, gint scale_dots)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(scale_dots>=-20&&scale_dots<=20)
      { 
        priv->scale_dots=scale_dots;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph scale_dots; -20<=x<=20.");
      }
  } 
void multi_graph_set_compose(MultiGraph *da, gint compose)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
 
    if(compose>=0&&compose<=15)
      { 
        priv->compose=compose;
        gtk_widget_queue_draw(GTK_WIDGET(da));
      }
    else
      {
        g_warning("Multi Graph compose; 0<=x<=15.");
      }
  } 
static void multi_graph_get_property(GObject *object, guint prop_id, GValue *value, GParamSpec *pspec)
  {
    MultiGraph *da=MULTI_GRAPH(object);
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
  
    switch(prop_id)
    {
      case MULTI_GRAPH_BACKGROUND_COLOR:
        g_value_set_string(value, priv->background_color_string);
        break;
      case MULTI_GRAPH_FONT_COLOR:
        g_value_set_string(value, priv->font_color_string);
        break;
      case MULTI_GRAPH_GRID_COLOR:
        g_value_set_string(value, priv->grid_color_string);
        break;
      case MULTI_GRAPH_TICK_COLOR:
        g_value_set_string(value, priv->tick_color_string);
        break;
      case MULTI_GRAPH_ROWS:
        g_value_set_int(value, priv->graph_rows);
        break;
      case MULTI_GRAPH_COLUMNS:
        g_value_set_int(value, priv->graph_columns);
        break;
      case MULTI_GRAPH_X_FONT_SCALE:
        g_value_set_int(value, priv->x_font_scale);
        break;
      case MULTI_GRAPH_Y_FONT_SCALE:
        g_value_set_int(value, priv->y_font_scale);
        break;
      case MULTI_GRAPH_DRAW_LINES:
        g_value_set_int(value, priv->draw_lines);
        break;
      case MULTI_GRAPH_SCALE_DOTS:
        g_value_set_int(value, priv->scale_dots);
        break;
      case MULTI_GRAPH_COMPOSE:
        g_value_set_int(value, priv->compose);
        break;
      default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
  }
const gchar* multi_graph_get_background_color(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->background_color_string;
  }
const gchar* multi_graph_get_font_color(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->font_color_string;
  }
const gchar* multi_graph_get_grid_color(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->grid_color_string;
  }
const gchar* multi_graph_get_tick_color(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->tick_color_string;
  }
gint multi_graph_get_graph_rows(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->graph_rows;
  }
gint multi_graph_get_graph_columns(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->graph_columns;
  }
gint multi_graph_get_x_font_scale(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->x_font_scale;
  }
gint multi_graph_get_y_font_scale(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->y_font_scale;
  }
gint multi_graph_get_draw_lines(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->draw_lines;
  }
gint multi_graph_get_scale_dots(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->scale_dots;
  }
gint multi_graph_get_compose(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    return priv->compose;
  }
static void multi_graph_init(MultiGraph *da)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    gint i=0;
    gint j=0;
    gint temp=0;
    gdouble d_temp=100;

    //Set some initial colors.
    priv->background_color[0]=0.0;
    priv->background_color[1]=0.0;
    priv->background_color[2]=0.0;
    priv->background_color[3]=1.0;
    priv->background_color_string=g_strdup("rgba(0, 0, 0, 1.0)");
    priv->font_color[0]=1.0;
    priv->font_color[1]=1.0;
    priv->font_color[2]=1.0;
    priv->font_color[3]=1.0;
    priv->font_color_string=g_strdup("rgba(255, 255, 255, 1.0)");
    priv->grid_color[0]=0.0;
    priv->grid_color[1]=1.0;
    priv->grid_color[2]=1.0;
    priv->grid_color[3]=1.0;
    priv->grid_color_string=g_strdup("rgba(0, 255, 255, 1.0)");
    priv->tick_color[0]=0.0;
    priv->tick_color[1]=0.0;
    priv->tick_color[2]=1.0;
    priv->tick_color[3]=1.0;
    priv->tick_color_string=g_strdup("rgba(0, 0, 255, 1.0)");

    priv->lc=lc;

    priv->graph_rows=1;
    priv->graph_columns=1;
    priv->test_increment_x=1;
    priv->x_font_scale=0;
    priv->y_font_scale=0;
    priv->draw_lines=0;
    priv->scale_dots=0;
    priv->compose=0;

    priv->x_ticks=g_array_sized_new(FALSE, FALSE, sizeof(gint), 16);
    priv->y_ticks=g_array_sized_new(FALSE, FALSE, sizeof(gint), 16);
    priv->y_max=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), 16);
    //Start 16 graphs with x_ticks=10, y_ticks=5 and y_max=100.
    temp=10;
    for(i=0;i<16;i++) g_array_append_val(priv->x_ticks, temp);
    temp=5;
    for(i=0;i<16;i++) g_array_append_val(priv->y_ticks, temp);
    temp=100;
    for(i=0;i<16;i++) g_array_append_val(priv->y_max, d_temp);

    //Initialize data arrays with height 50.
    struct point pt;
    priv->data_points=g_array_sized_new(FALSE, FALSE, sizeof(GArray*), 16);
    GArray *temp_array=NULL;
    for(i=0;i<16;i++)
      {
        temp_array=g_array_sized_new(FALSE, FALSE, sizeof(struct point), 10);
        //Initialize baseline to 1 for all the graphs.
        pt.y=0.01;
        for(j=0;j<10;j++)
          {
            pt.x=(gdouble)j;
            g_array_append_val(temp_array, pt);
          }
        g_array_append_val(priv->data_points, temp_array);
      }      
  }
GtkWidget* multi_graph_new()
  {
    return GTK_WIDGET(g_object_new(multi_graph_get_type(), NULL));
  }
static void multi_graph_finalize(GObject *object)
  {
    MultiGraph *da=MULTI_GRAPH(object);
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    gint i=0;
    g_free(priv->background_color_string);
    g_array_free(priv->x_ticks, TRUE);
    g_array_free(priv->y_ticks, TRUE);
    g_array_free(priv->y_max, TRUE);
    for(i=0;i<priv->data_points->len;i++)
      {
        g_array_free(g_array_index(priv->data_points, GArray*, i), TRUE);
      }
    g_array_free(priv->data_points, TRUE);

    G_OBJECT_CLASS(multi_graph_parent_class)->finalize(object);
  }
void multi_graph_set_points(MultiGraph *da, gint graph_id, gint points)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    gint i=0;
    gint len=0;
    GArray *temp=NULL;
    gint *i_pt=NULL;
    gint diff=0;
    struct point pt;
    pt.y=0.01;

    //Get the length of the dataset.
    temp=g_array_index(priv->data_points, GArray*, graph_id);
    len=temp->len;
    diff=abs(points-len);
    
    if(graph_id>=0&&graph_id<=15)
      {
        //Set a minimum of 5 points.
        if(points>5)
          {
            //Trim points. If pts==points, don't change anything.
            if(len>points)
              {
                for(i=0;i<diff;i++)
                  {
                    g_array_remove_index_fast(temp, points);
                  }
                //Reset the x-tick marks.     
                i_pt=&g_array_index(priv->x_ticks, gint, graph_id);
                *i_pt=points;
              }
            //Add points
            if(len<points)
              {
                for(i=0;i<diff;i++)
                  {
                    pt.x=len;
                    g_array_append_val(temp, pt);
                    len++;
                  }
                //Reset the x-tick marks.
                i_pt=&g_array_index(priv->x_ticks, gint, graph_id);
                *i_pt=points;
             }
          }
        else
          {
            g_warning("Need at least 5 points to graph.\n");
          }
      }
    else
      {
        g_warning("Graph id: 0<=x<=15\n");
      }
  }
gint multi_graph_get_points(MultiGraph *da, gint graph_id)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);

    if(graph_id>=0&&graph_id<=15)
      {
        GArray *temp=g_array_index(priv->data_points, GArray*, graph_id);
        return temp->len;
      }
    else
      {
        g_warning("Graph id: 0<=x<=15\n");
        return 0;
      }
  }
void multi_graph_feed_point(MultiGraph *da, gint graph_id, gdouble x, gdouble y)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    gint i=0;
    struct point pt;
    struct point *x_point=NULL;
    pt.x=x;
    pt.y=y;
    GArray *temp=g_array_index(priv->data_points, GArray*, graph_id);
    gint len=temp->len-1;
    g_array_remove_index_fast(temp, len);
    g_array_prepend_val(temp, pt);
    /*
       Why is the x point re-indexed this way? Why not just use the loop index for evenly
spaced x. The problem is in getting the bezier points for smoothing. There needs to be valid
x coordinates in the array to get valid bezier points for the curves. Just using a loop
index, for evenly spaced points, lines and rectangles, would work fine but not for smooth curves.
    */
    for(i=1;i<temp->len;i++)
      {
        x_point=&g_array_index(temp, struct point, i);
        x_point->x=(gdouble)i;
      }
  }
void multi_graph_swap_graphs(MultiGraph *da, gint id1, gint id2)
  {
    if(id1>=0&&id1<=15&&id2>=0&&id2<=15)
      {
        MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
        GArray **temp1=NULL;
        GArray **temp2=NULL;
        GArray *temp=NULL;
        gdouble *d_pt1=NULL;
        gdouble *d_pt2=NULL;
        gdouble d_temp1=0;
        gdouble d_temp2=0;
        gint *i_pt1=NULL;
        gint *i_pt2=NULL;
        gint i_temp1=0;
        gint i_temp2=0;
        temp1=&g_array_index(priv->data_points, GArray*, id1);
        temp2=&g_array_index(priv->data_points, GArray*, id2);
        temp=*temp1;
        *temp1=*temp2;
        *temp2=temp;
        //Swap x tick values.
        i_pt1=&g_array_index(priv->x_ticks, gint, id1);
        i_temp1=(*i_pt1);
        i_pt2=&g_array_index(priv->x_ticks, gint, id2);
        i_temp2=(*i_pt2);
        *i_pt1=i_temp2;
        *i_pt2=i_temp1;
        //Swap y tick values.
        i_pt1=&g_array_index(priv->y_ticks, gint, id1);
        i_temp1=(*i_pt1);
        i_pt2=&g_array_index(priv->y_ticks, gint, id2);
        i_temp2=(*i_pt2);
        *i_pt1=i_temp2;
        *i_pt2=i_temp1;  
        //Swap y label values.
        d_pt1=&g_array_index(priv->y_max, gdouble, id1);
        d_temp1=(*d_pt1);
        d_pt2=&g_array_index(priv->y_max, gdouble, id2);
        d_temp2=(*d_pt2);
        *d_pt1=d_temp2;
        *d_pt2=d_temp1;
        //Swap line colors.
        d_temp1=lc[id1][0];
        lc[id1][0]=lc[id2][0];
        lc[id2][0]=d_temp1;
        d_temp1=lc[id1][1];
        lc[id1][1]=lc[id2][1];
        lc[id2][1]=d_temp1;
        d_temp1=lc[id1][2];
        lc[id1][2]=lc[id2][2];
        lc[id2][2]=d_temp1;
        d_temp1=lc[id1][3];
        lc[id1][3]=lc[id2][3];
        lc[id2][3]=d_temp1;
      }
    else
      {
        g_warning("Multi Graph ID's; 0<=x<=15.");
      }
  }
static gboolean multi_graph_draw(GtkWidget *da, cairo_t *cr)
  {
    MultiGraphPrivate *priv=MULTI_GRAPH_GET_PRIVATE(da);
    gint i=0;
    gint j=0;
    gint k=0;
    gint h=0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    gint graph_rows=priv->graph_rows;
    gint graph_columns=priv->graph_columns;
    gint draw_lines=priv->draw_lines;
    gint compose=priv->compose;
    gint x_font_scale=priv->x_font_scale;
    gint y_font_scale=priv->y_font_scale;
    gint scale_dots=priv->scale_dots;
    gdouble test_increment_x=priv->test_increment_x;
    //Some drawing variables.
    gdouble x=0.0;
    gdouble y=0.0;
    gint temp_tick=0;
    gdouble graph_width=width/graph_columns;
    gdouble graph_height=height/graph_rows;
    gdouble ratio_x=graph_width/700.0;
    gdouble ratio_y=graph_height/500.0;
    //Initialize to first tick marks in the arrays.
    gdouble x_tick=graph_width/g_array_index(priv->x_ticks, gint, 0);
    gdouble y_tick=graph_height/g_array_index(priv->y_ticks, gint, 0);
    GArray *rnd_data=NULL;

    cairo_set_source_rgba(cr, priv->background_color[0], priv->background_color[1], priv->background_color[2], priv->background_color[3]);
    cairo_paint(cr);

    //Draw grid lines before points, lines and curve and after for histogram rectangles.
    if(draw_lines!=3)
      {
        //Vertical lines.
        cairo_set_source_rgba(cr, priv->tick_color[0], priv->tick_color[1], priv->tick_color[2], priv->tick_color[3]);
        cairo_set_line_width(cr, 1);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
                for(k=0;k<g_array_index(priv->x_ticks, gint, temp_tick);k++)
                  {
                    x=j*graph_width+k*x_tick;
                    y=i*graph_height+graph_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y-graph_height);
                    cairo_stroke(cr);
                  } 
              }
          }
        //Horizontal lines.
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                temp_tick=i*graph_columns+j; 
                y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
                for(k=0;k<g_array_index(priv->y_ticks, gint, temp_tick);k++)
                  {
                    x=j*graph_width;
                    y=i*graph_height+k*y_tick;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x+graph_width, y);
                    cairo_stroke(cr);
                  }
              }
          }
      }
         
    //Draw points, lines, curves and rectangles.
    struct point pt;
    //Draw points. 
    if(draw_lines==0)
      {
        cairo_set_line_width(cr, 8*ratio_x+scale_dots);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the points in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                y=graph_height*i+graph_height;
                cairo_move_to(cr, x, y);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
                y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[id][3]);
                    rnd_data=g_array_index(priv->data_points, GArray*, id);           
                    for(k=0;k<rnd_data->len;k++)
                      {
                        pt=g_array_index(rnd_data, struct point, k);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);
                        cairo_line_to(cr, x, y);
                        cairo_stroke(cr);
                      }
                  }
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(priv->data_points, GArray*, h);
                        x_tick=graph_width/g_array_index(priv->x_ticks, gint, h);           
                        for(k=0;k<rnd_data->len;k++)
                          {
                            pt=g_array_index(rnd_data, struct point, k);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            cairo_move_to(cr, x, y);
                            cairo_line_to(cr, x, y);
                            cairo_stroke(cr);
                          }
                      }
                  } 
                cairo_restore(cr);
              }
          }    
      } 
    //Draw lines between points.  
    else if(draw_lines==1)
      {
        cairo_set_line_width(cr, 4*ratio_x+scale_dots);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the lines in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
                y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                    rnd_data=g_array_index(priv->data_points, GArray*, id);
                    pt=g_array_index(rnd_data, struct point, 0);
                    x=j*graph_width+pt.x*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*pt.y);
                    cairo_move_to(cr, x, y);           
                    for(k=1;k<rnd_data->len;k++)
                      {
                        pt=g_array_index(rnd_data, struct point, k);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_line_to(cr, x, y);
                        cairo_stroke(cr);
                        cairo_move_to(cr, x, y);
                      }
                  } 
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(priv->data_points, GArray*, h);
                        x_tick=graph_width/g_array_index(priv->x_ticks, gint, h);
                        pt=g_array_index(rnd_data, struct point, 0);
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);           
                        for(k=1;k<rnd_data->len;k++)
                          {
                            pt=g_array_index(rnd_data, struct point, k);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            cairo_line_to(cr, x, y);
                            cairo_stroke(cr);
                            cairo_move_to(cr, x, y);
                          } 
                      }
                  }
                cairo_restore(cr);
              }
          }    
      }
    //Draw curves between points.
    else if(draw_lines==2)
      {
        struct controls c1;
        gdouble ct1=0;
        gdouble ct2=0;
        gdouble ct3=0;
        gdouble ct4=0;
        cairo_set_line_width(cr, 4*ratio_x+scale_dots);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the curve in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
                y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                    rnd_data=g_array_index(priv->data_points, GArray*, id);
                    GArray *bezier_pts=control_points_from_coords2(rnd_data);
                    pt=g_array_index(rnd_data, struct point, 0);
                    x=j*graph_width+pt.x*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*pt.y);
                    cairo_move_to(cr, x, y);       
                    for(k=1;k<rnd_data->len;k++)
                      {
                        pt=g_array_index(rnd_data, struct point, k);
                        c1=g_array_index(bezier_pts, struct controls, k-1);
                        //k=pt.x for testing. For smooth curves pt.x needed for the smoothing function.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        ct1=j*graph_width+c1.x1*x_tick+x_tick;
                        ct2=i*graph_height+graph_height-(graph_height*c1.y1);
                        ct3=j*graph_width+c1.x2*x_tick+x_tick;
                        ct4=i*graph_height+graph_height-(graph_height*c1.y2);
                        cairo_curve_to(cr, ct1, ct2, ct3, ct4, x, y);
                        cairo_stroke(cr);
                        cairo_move_to(cr, x, y);
                      } 
                    g_array_free(bezier_pts, TRUE);
                  }
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(priv->data_points, GArray*, h);
                        x_tick=graph_width/g_array_index(priv->x_ticks, gint, h);
                        GArray *bezier_pts=control_points_from_coords2(rnd_data);
                        pt=g_array_index(rnd_data, struct point, 0);
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);       
                        for(k=1;k<rnd_data->len;k++)
                          {
                            pt=g_array_index(rnd_data, struct point, k);
                            c1=g_array_index(bezier_pts, struct controls, k-1);
                            //k=pt.x for testing. For smooth curves pt.x needed for the smoothing function.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            ct1=j*graph_width+c1.x1*x_tick+x_tick;
                            ct2=i*graph_height+graph_height-(graph_height*c1.y1);
                            ct3=j*graph_width+c1.x2*x_tick+x_tick;
                            ct4=i*graph_height+graph_height-(graph_height*c1.y2);
                            cairo_curve_to(cr, ct1, ct2, ct3, ct4, x, y);
                            cairo_stroke(cr);
                            cairo_move_to(cr, x, y);
                          } 
                        g_array_free(bezier_pts, TRUE);
                      }
                  }
                cairo_restore(cr);
              }
          }    
      }
    //Draw histogram rectangles.
    else
      {
        cairo_set_line_width(cr, 4*ratio_x+scale_dots);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the lines in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
                y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                    rnd_data=g_array_index(priv->data_points, GArray*, id);
                    pt=g_array_index(rnd_data, struct point, 0);
                    x=j*graph_width+pt.x*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*pt.y);
                    cairo_move_to(cr, x, y);           
                    for(k=1;k<rnd_data->len;k++)
                      {
                        cairo_rectangle(cr, x, y, x_tick, graph_height);
                        cairo_fill(cr);
                        cairo_set_source_rgba(cr, priv->tick_color[0], priv->tick_color[1], priv->tick_color[2], priv->tick_color[3]);
                        cairo_rectangle(cr, x, y, x_tick, graph_height);
                        cairo_stroke(cr);
                        cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                        cairo_fill(cr);
                        pt=g_array_index(rnd_data, struct point, k);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);
                      }
                  } 
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(priv->data_points, GArray*, h);
                        x_tick=graph_width/g_array_index(priv->x_ticks, gint, h);
                        pt=g_array_index(rnd_data, struct point, 0);
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);           
                        for(k=1;k<rnd_data->len;k++)
                          {
                            cairo_rectangle(cr, x, y, x_tick, graph_height);
                            cairo_fill(cr);
                            cairo_set_source_rgba(cr, priv->tick_color[0], priv->tick_color[1], priv->tick_color[2], priv->tick_color[3]);
                            cairo_rectangle(cr, x, y, x_tick, graph_height);
                            cairo_stroke(cr);
                            cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                            cairo_fill(cr);
                            pt=g_array_index(rnd_data, struct point, k);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            cairo_move_to(cr, x, y);
                          } 
                      }
                  }
                cairo_restore(cr);
              }
          }    
      }

    //Draw histogram lines after drawing the data.
    if(draw_lines==3)
      {
        //Vertical lines.
        cairo_set_source_rgba(cr, priv->tick_color[0], priv->tick_color[1], priv->tick_color[2], priv->tick_color[3]);
        cairo_set_line_width(cr, 1);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
                for(k=0;k<g_array_index(priv->x_ticks, gint, temp_tick);k++)
                  {
                    x=j*graph_width+k*x_tick;
                    y=i*graph_height+graph_height;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y-graph_height);
                    cairo_stroke(cr);
                  } 
              }
          }
        //Horizontal lines.
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                temp_tick=i*graph_columns+j; 
                y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
                for(k=0;k<g_array_index(priv->y_ticks, gint, temp_tick);k++)
                  {
                    x=j*graph_width;
                    y=i*graph_height+k*y_tick;
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x+graph_width, y);
                    cairo_stroke(cr);
                  }
              }
          }
      }
   
    //The x-axis numbers.
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 18*ratio_x+x_font_scale);
    cairo_set_source_rgba(cr, priv->font_color[0], priv->font_color[1], priv->font_color[2], priv->font_color[3]);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            cairo_save(cr);
            x=graph_width*j;
            y=graph_height*i;
            cairo_rectangle(cr, x, y, graph_width, graph_height);
            cairo_clip(cr);
            y=i*graph_height+graph_height;
            temp_tick=i*graph_columns+j; 
            x_tick=graph_width/g_array_index(priv->x_ticks, gint, temp_tick);
            for(k=0;k<g_array_index(priv->x_ticks, gint, temp_tick);k++)
              {
                x=j*graph_width+k*x_tick;
                gchar *string=g_strdup_printf("%i", (gint)(test_increment_x*k));
                cairo_move_to(cr, x+8.0*ratio_x, y-10.0*ratio_x);
                cairo_show_text(cr, string);
                g_free(string);
              }
            cairo_restore(cr);
          }
      }

    //The y-axis numbers.
    gint len=0;
    gdouble y_value=0;
    cairo_set_font_size(cr, 20*ratio_y+y_font_scale);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            cairo_save(cr);
            x=graph_width*j;
            y=graph_height*i;
            cairo_rectangle(cr, x, y, graph_width, graph_height);
            cairo_clip(cr);
            x=j*graph_width;
            temp_tick=i*graph_columns+j; 
            y_tick=graph_height/g_array_index(priv->y_ticks, gint, temp_tick);
            y_value=g_array_index(priv->y_max, gdouble, temp_tick)/g_array_index(priv->y_ticks, gint, temp_tick);
            len=g_array_index(priv->y_ticks, gint, temp_tick)+1;
            for(k=0;k<len;k++)
              {
                y=i*graph_height+graph_height-k*y_tick;
                gchar *string=g_strdup_printf("%.2f", (y_value*k));
                cairo_move_to(cr, x+8.0*ratio_y, y+25.0*ratio_y);
                cairo_show_text(cr, string);
                g_free(string);
              }
            cairo_restore(cr);
          }
      }

    //Draw graph blocks.
    cairo_set_source_rgba(cr, priv->grid_color[0], priv->grid_color[1], priv->grid_color[2], priv->grid_color[3]);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_stroke(cr);
    for(i=0;i<graph_columns;i++)
      {
        x=i*graph_width;
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, height);
        cairo_stroke(cr);
      }
    for(i=0;i<graph_rows;i++)
      {
        y=i*graph_height;
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
        cairo_stroke(cr);
      }
  
    return FALSE;
  }
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=0;
    if(dataPoints!=NULL) count=dataPoints->len-1;
    gdouble *fCP=NULL;
    gdouble *sCP=NULL;

    if(count>0)
      {
        fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
      }
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count<1||dataPoints==NULL)
      {
        //Return NULL.
        controlPoints=NULL;
        g_warning("Can't get control points from coordinates. NULL returned.\n");
      }
    else if(count==1)
      {
        struct point P0=g_array_index(dataPoints, struct point, 0);
        struct point P3=g_array_index(dataPoints, struct point, 1);

        //Calculate First Control Point
        //3P1 = 2P0 + P3
        struct point P1;
        P1.x=(2.0*P0.x+P3.x)/3.0;
        P1.y=(2.0*P0.y+P3.y)/3.0;

        *(fCP)=P1.x;
        *(fCP+1)=P1.y;

        //Calculate second Control Point
        //P2 = 2P1 - P0
        struct point P2;
        P2.x=(2.0*P1.x-P0.x);
        P2.y=(2.0*P1.y-P0.x);

        *(sCP)=P2.x;
        *(sCP+1)=P2.y;      
      }
    else
      {
        gdouble *rhs=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        gdouble *a=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *b=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *c=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble rhsValueX=0;
        gdouble rhsValueY=0;
        struct point P0;
        struct point P3;        
        gdouble m=0;
        gdouble b1=0;
        gdouble r2x=0;
        gdouble r2y=0;
        gdouble P1_x=0;
        gdouble P1_y=0;
   
        for(i=0;i<count;i++)
          {
            P0=g_array_index(dataPoints, struct point, i);
            P3=g_array_index(dataPoints, struct point, i+1);

            if(i==0)
              {
                *(a)=0.0;
                *(b)=2.0;
                *(c)=1.0;

                //rhs for first segment
                rhsValueX=P0.x+2.0*P3.x;
                rhsValueY=P0.y+2.0*P3.y;
              }
            else if(i==count-1)
              {
                *(a+i)=2.0;
                *(b+i)=7.0;
                *(c+i)=0.0;

                //rhs for last segment
                rhsValueX=8.0*P0.x+P3.x;
                rhsValueY=8.0*P0.y+P3.y;
              }
            else
              {
                *(a+i)=1.0;
                *(b+i)=4.0;
                *(c+i)=1.0;

                rhsValueX=4.0*P0.x+2.0*P3.x;
                rhsValueY=4.0*P0.y+2.0*P3.y;
              }
            *(rhs+i*2)=rhsValueX;
            *(rhs+i*2+1)=rhsValueY;
          }

        //Solve Ax=B. Use Tridiagonal matrix algorithm a.k.a Thomas Algorithm
        for(i=1;i<count;i++)
          {
            m=(*(a+i))/(*(b+i-1));

            b1=(*(b+i))-m*(*(c+i-1));
            *(b+i)=b1;

            r2x=(*(rhs+i*2))-m*(*(rhs+(i-1)*2));
            r2y=(*(rhs+i*2+1))-m*(*(rhs+(i-1)*2+1));

            *(rhs+i*2)=r2x;
            *(rhs+i*2+1)=r2y;
          }

        //Get First Control Points
        
        //Last control Point
        gdouble lastControlPointX=(*(rhs+2*count-2))/(*(b+count-1));
        gdouble lastControlPointY=(*(rhs+2*count-1))/(*(b+count-1));

        *(fCP+2*count-2)=lastControlPointX;
        *(fCP+2*count-1)=lastControlPointY;

        gdouble controlPointX=0;
        gdouble controlPointY=0;

        for(i=count-2;i>=0;--i)
          {
            controlPointX=(*(rhs+i*2)-(*(c+i))*(*(fCP+(i+1)*2)))/(*(b+i));
            controlPointY=(*(rhs+i*2+1)-(*(c+i))*(*(fCP+(i+1)*2+1)))/(*(b+i));

             *(fCP+i*2)=controlPointX;
             *(fCP+i*2+1)=controlPointY; 
          }

        //Compute second Control Points from first.
        for(i=0;i<count;i++)
          {
            if(i==count-1)
              {
                P3=g_array_index(dataPoints, struct point, i+1);
                P1_x=(*(fCP+i*2));
                P1_y=(*(fCP+i*2+1));

                controlPointX=(P3.x+P1_x)/2.0;
                controlPointY=(P3.y+P1_y)/2.0;

                *(sCP+count*2-2)=controlPointX;
                *(sCP+count*2-1)=controlPointY;
              }
            else
              {
                P3=g_array_index(dataPoints, struct point, i+1);                
                P1_x=(*(fCP+(i+1)*2));
                P1_y=(*(fCP+(i+1)*2+1));

                controlPointX=2.0*P3.x-P1_x;
                controlPointY=2.0*P3.y-P1_y;

                *(sCP+i*2)=controlPointX;
                *(sCP+i*2+1)=controlPointY;
              }

          }

        controlPoints=g_array_new(FALSE, FALSE, sizeof(struct controls));
        struct controls cp;
        for(i=0;i<count;i++)
          {
            cp.x1=(*(fCP+i*2));
            cp.y1=(*(fCP+i*2+1));
            cp.x2=(*(sCP+i*2));
            cp.y2=(*(sCP+i*2+1));
            g_array_append_val(controlPoints, cp);
          }

        g_free(rhs);
        g_free(a);
        g_free(b);
        g_free(c);
     }

    if(fCP!=NULL) g_free(fCP);
    if(sCP!=NULL) g_free(sCP);

    return controlPoints;
  }


















