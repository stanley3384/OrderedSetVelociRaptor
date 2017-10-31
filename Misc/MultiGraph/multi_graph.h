
/*

    For use with multi_graph.c. Look in multi_graph_main.c for more information.

    C. Eric Cashon

*/

#ifndef __MULTI_GRAPH_H__
#define __MULTI_GRAPH_H__

G_BEGIN_DECLS

#define MULTI_GRAPH_TYPE (multi_graph_get_type())
#define MULTI_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), MULTI_GRAPH_TYPE, MultiGraph))
#define MULTI_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), MULTI_GRAPH_TYPE, MultiGraphClass))
#define IS_MULTI_GRAPH(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), MULTI_GRAPH_TYPE))
#define IS_MULTI_GRAPH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), MULTI_GRAPH_TYPE)
#define MULTI_GRAPH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), MULTI_GRAPH_TYPE, MultiGraphClass))

typedef struct _MultiGraph MultiGraph;
typedef struct _MultiGraphClass MultiGraphClass;
struct _MultiGraph
{
  GtkDrawingArea da;
};
struct _MultiGraphClass
{
  GtkDrawingAreaClass parent_class;
};

//Public functions.
GType multi_graph_get_type(void) G_GNUC_CONST;
GtkWidget* multi_graph_new();
//Property get and set.
void multi_graph_set_background_color(MultiGraph *da, const gchar *dim_color_string);
const gchar* multi_graph_get_background_color(MultiGraph *da);
void multi_graph_set_font_color(MultiGraph *da, const gchar *font_color_string);
const gchar* multi_graph_get_font_color(MultiGraph *da);
void multi_graph_set_grid_color(MultiGraph *da, const gchar *grid_color_string);
const gchar* multi_graph_get_grid_color(MultiGraph *da);
void multi_graph_set_tick_color(MultiGraph *da, const gchar *tick_color_string);
const gchar* multi_graph_get_tick_color(MultiGraph *da);
void multi_graph_set_rows(MultiGraph *da, gint rows);
gint multi_graph_get_graph_rows(MultiGraph *da);
void multi_graph_set_columns(MultiGraph *da, gint columns);
gint multi_graph_get_graph_columns(MultiGraph *da);
void multi_graph_set_x_font_scale(MultiGraph *da, gint x_font_scale);
gint multi_graph_get_x_font_scale(MultiGraph *da);
void multi_graph_set_y_font_scale(MultiGraph *da, gint y_font_scale);
gint multi_graph_get_y_font_scale(MultiGraph *da);
void multi_graph_set_draw_lines(MultiGraph *da, gint draw_lines);
gint multi_graph_get_draw_lines(MultiGraph *da);
void multi_graph_set_scale_dots(MultiGraph *da, gint scale_dots);
gint multi_graph_get_scale_dots(MultiGraph *da);
void multi_graph_set_compose(MultiGraph *da, gint compose);
gint multi_graph_get_compose(MultiGraph *da);
//Set point, line, curve or rectangle color to draw with for each graph. Doesn't have a set get pair. 
void multi_graph_set_line_color(MultiGraph *da, gint graph_id, const gchar *line_color_string);
//Set the number of points to draw for each graph. This adjusts the x_ticks as well.
void multi_graph_set_points(MultiGraph *da, gint graph_id, gint points);
gint multi_graph_get_points(MultiGraph *da, gint graph_id);
//Set the number of y_ticks for the graph.
void multi_graph_set_y_ticks(MultiGraph *da, gint graph_id, gint ticks);
gint multi_graph_get_y_ticks(MultiGraph *da, gint graph_id);
//Set the max value of the y-axis.
void multi_graph_set_y_max(MultiGraph *da, gint graph_id, gdouble y_max);
gdouble multi_graph_get_y_max(MultiGraph *da, gint graph_id);
//Add a point to the start of the graph.
void multi_graph_feed_point(MultiGraph *da, gint graph_id, gdouble x, gdouble y);
//Swap the positions of two graphs.
void multi_graph_swap_graphs(MultiGraph *da, gint id1, gint id2);

G_END_DECLS

#endif 
