
/*

    For use with circular_clocks.c. Look in circular_clocks_main.c for more information.

    C. Eric Cashon

*/

#ifndef __CIRCULAR_CLOCKS_H__
#define __CIRCULAR_CLOCKS_H__

G_BEGIN_DECLS

#define CIRCULAR_CLOCKS_TYPE (circular_clocks_get_type())
#define CIRCULAR_CLOCKS(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CIRCULAR_CLOCKS_TYPE, CircularClocks))
#define CIRCULAR_CLOCKS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), CIRCULAR_CLOCKS_TYPE, CircularClocksClass))
#define IS_CIRCULAR_CLOCKS(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CIRCULAR_CLOCKS_TYPE))
#define IS_CIRCULAR_CLOCKS_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CIRCULAR_CLOCKS_TYPE)
#define CIRCULAR_CLOCKS_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), CIRCULAR_CLOCKS_TYPE, CircularClocksClass))

typedef struct _CircularClocks CircularClocks;
typedef struct _CircularClocksClass CircularClocksClass;
struct _CircularClocks
{
  GtkDrawingArea da;
};
struct _CircularClocksClass
{
  GtkDrawingAreaClass parent_class;
};

//The different types of circular clocks.
enum
{
  GRADIENT_CLOCK,
  WAVE_CLOCK,
  BRAID_CLOCK,
  GEAR_CLOCK,
  GOURAUD_MESH_CLOCK
};

//Public functions.
GType circular_clocks_get_type(void) G_GNUC_CONST;
GtkWidget* circular_clocks_new();

void circular_clocks_set_clock(CircularClocks *da, gint clock);
gint circular_clocks_get_clock(CircularClocks *da);

G_END_DECLS

#endif 
