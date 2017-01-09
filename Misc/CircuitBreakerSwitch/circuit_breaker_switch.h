
/*

    For use with circuit_breaker_switch.c. Look in circuit_breaker_switch_main.c for more information.

    C. Eric Cashon

*/

#ifndef __CIRCUIT_BREAKER_SWITCH_H__
#define __CIRCUIT_BREAKER_SWITCH_H__

//circuit_breaker_switch.h included after #include<gtk/gtk.h> in circuit_breaker_switch.c. Everything is in gtk.

G_BEGIN_DECLS

#define CIRCUIT_BREAKER_SWITCH_TYPE (circuit_breaker_switch_get_type())
#define CIRCUIT_BREAKER_SWITCH(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), CIRCUIT_BREAKER_SWITCH_TYPE, CircuitBreakerSwitch))
#define CIRCUIT_BREAKER_SWITCH_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), CIRCUIT_BREAKER_SWITCH_TYPE, CircuitBreakerSwitchClass))
#define IS_CIRCUIT_BREAKER_SWITCH(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), CIRCUIT_BREAKER_SWITCH_TYPE))
#define IS_CIRCUIT_BREAKER_SWITCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), CIRCUIT_BREAKER_SWITCH_TYPE)
#define CIRCUIT_BREAKER_SWITCH_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), CIRCUIT_BREAKER_SWITCH_TYPE, CircuitBreakerSwitchClass))

typedef struct _CircuitBreakerSwitch CircuitBreakerSwitch;
typedef struct _CircuitBreakerSwitchClass CircuitBreakerSwitchClass;
struct _CircuitBreakerSwitch
{
  GtkDrawingArea da;
};
struct _CircuitBreakerSwitchClass
{
  GtkDrawingAreaClass parent_class;
};

//Circuit breaker direction.
enum
{
  BREAKER_HORIZONTAL_RIGHT,
  BREAKER_VERTICAL_UP
};

//Circuit breaker state
enum
{
  BREAKER_ON,
  BREAKER_STARTING,
  BREAKER_OFF,
  BREAKER_BREAK
};

//Public functions.
GType circuit_breaker_switch_get_type(void) G_GNUC_CONST;
GtkWidget* circuit_breaker_switch_new();

void circuit_breaker_switch_set_direction(CircuitBreakerSwitch *da, gint breaker_direction);
gint circuit_breaker_switch_get_direction(CircuitBreakerSwitch *da);
void circuit_breaker_switch_set_state(CircuitBreakerSwitch *da, gint state);
gint circuit_breaker_switch_get_state(CircuitBreakerSwitch *da);
void circuit_breaker_switch_set_icon(CircuitBreakerSwitch *da, gboolean state);
gint circuit_breaker_switch_get_icon(CircuitBreakerSwitch *da);

G_END_DECLS

#endif 
