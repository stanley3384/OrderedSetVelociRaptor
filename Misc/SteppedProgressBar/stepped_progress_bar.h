
/*

    For use with stepped_progress_bar.c. Look in stepped_progress_bar_main.c for more information.

    C. Eric Cashon

*/

#ifndef __STEPPED_PROGRESS_BAR_H__
#define __STEPPED_PROGRESS_BAR_H__

//stepped_progress_bar.h included after #include<gtk/gtk.h> in stepped_progress_bar.c. Everything is in gtk.

G_BEGIN_DECLS

#define STEPPED_PROGRESS_BAR_TYPE (stepped_progress_bar_get_type())
#define STEPPED_PROGRESS_BAR(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), STEPPED_PROGRESS_BAR_TYPE, SteppedProgressBar))
#define STEPPED_PROGRESS_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), STEPPED_PROGRESS_BAR_TYPE, SteppedProgressBarClass))
#define IS_STEPPED_PROGRESS_BAR(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), STEPPED_PROGRESS_BAR_TYPE))
#define IS_STEPPED_PROGRESS_BAR_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), STEPPED_PROGRESS_BAR_TYPE)
#define STEPPED_PROGRESS_BAR_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), STEPPED_PROGRESS_BAR_TYPE, SteppedProgressBarClass))

typedef struct _SteppedProgressBar SteppedProgressBar;
typedef struct _SteppedProgressBarClass SteppedProgressBarClass;
struct _SteppedProgressBar
{
  GtkDrawingArea da;
};
struct _SteppedProgressBarClass
{
  GtkDrawingAreaClass parent_class;
  void (*color_changed) (SteppedProgressBar *da);
};

//Progress bar direction.
enum
{
  HORIZONTAL_RIGHT,
  VERTICAL_UP
};

//Public functions.
GType stepped_progress_bar_get_type(void) G_GNUC_CONST;
GtkWidget* stepped_progress_bar_new();

void stepped_progress_bar_set_progress_direction(SteppedProgressBar *da, gint progress_direction);
int stepped_progress_bar_get_progress_direction(SteppedProgressBar *da);
void stepped_progress_bar_set_steps(SteppedProgressBar *da, gint steps);
int stepped_progress_bar_get_steps(SteppedProgressBar *da);
void stepped_progress_bar_set_step_stop(SteppedProgressBar *da, gint total_steps);
int stepped_progress_bar_get_step_stop(SteppedProgressBar *da);
//Set and get the color gradients.
void stepped_progress_bar_set_background_rgba1(SteppedProgressBar *da, const gchar *background_string_rgba1);
const gchar* stepped_progress_bar_get_background_rgba1(SteppedProgressBar *da);
void stepped_progress_bar_set_background_rgba2(SteppedProgressBar *da, const gchar *background_string_rgba2);
const gchar* stepped_progress_bar_get_background_rgba2(SteppedProgressBar *da);
void stepped_progress_bar_set_foreground_rgba1(SteppedProgressBar *da, const gchar *foreground_string_rgba1);
const gchar* stepped_progress_bar_get_foreground_rgba1(SteppedProgressBar *da);
void stepped_progress_bar_set_foreground_rgba2(SteppedProgressBar *da, const gchar *foreground_string_rgba2);
const gchar* stepped_progress_bar_get_foreground_rgba2(SteppedProgressBar *da);

G_END_DECLS

#endif 
