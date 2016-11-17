
/*

    For use with headlight_toggle.c. Look in headlight_toggle_main.c for more information.

    C. Eric Cashon

*/

#ifndef __HEADLIGHT_TOGGLE_H__
#define __HEADLIGHT_TOGGLE_H__

G_BEGIN_DECLS

#define HEADLIGHT_TOGGLE_TYPE (headlight_toggle_get_type())
#define HEADLIGHT_TOGGLE(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), HEADLIGHT_TOGGLE_TYPE, HeadlightToggle))
#define HEADLIGHT_TOGGLE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), HEADLIGHT_TOGGLE_TYPE, HeadlightToggleClass))
#define IS_HEADLIGHT_TOGGLE(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), HEADLIGHT_TOGGLE_TYPE))
#define IS_HEADLIGHT_TOGGLE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), HEADLIGHT_TOGGLE_TYPE)
#define HEADLIGHT_TOGGLE_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), HEADLIGHT_TOGGLE_TYPE, HeadlightToggleClass))

typedef struct _HeadlightToggle HeadlightToggle;
typedef struct _HeadlightToggleClass HeadlightToggleClass;
struct _HeadlightToggle
{
  GtkDrawingArea da;
};
struct _HeadlightToggleClass
{
  GtkDrawingAreaClass parent_class;
};

//Toggle state
enum
{
  HEADLIGHT_TOGGLE_OFF,
  HEADLIGHT_TOGGLE_ON
};

//Toggle drawing icon.
enum
{
  HEADLIGHT_ICON,
  EMERGENCY_LIGHT_ICON
};

//Public functions.
GType headlight_toggle_get_type(void) G_GNUC_CONST;
GtkWidget* headlight_toggle_new();

void headlight_toggle_set_state(HeadlightToggle *da, gint state);
gint headlight_toggle_get_state(HeadlightToggle *da);
void headlight_toggle_set_icon(HeadlightToggle *da, gint state);
gint headlight_toggle_get_icon(HeadlightToggle *da);

G_END_DECLS

#endif 
