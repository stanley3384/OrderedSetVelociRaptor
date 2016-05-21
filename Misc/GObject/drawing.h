
//For use with drawing_main.c. Look in drawing_main.c for more information.

#ifndef __SMILEY_DRAWING_H__
#define __SMILEY_DRAWING_H__

//drawing.h included after #include<gtk/gtk.h> in drawing.c. Everything is in gtk.

G_BEGIN_DECLS

#define SMILEY_DRAWING_TYPE (smiley_drawing_get_type())
#define SMILEY_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), SMILEY_DRAWING_TYPE, SmileyDrawing))
#define SMILEY_DRAWING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), SMILEY_DRAWING_TYPE, SmileyDrawingClass))
#define IS_SMILEY_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), SMILEY_DRAWING_TYPE))
#define IS_SMILEY_DRAWING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), SMILEY_DRAWING_TYPE)
#define SMILEY_DRAWING_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), SMILEY_DRAWING_TYPE, SmileyDrawingClass))

typedef struct _SmileyDrawing SmileyDrawing;
typedef struct _SmileyDrawingClass SmileyDrawingClass;
struct _SmileyDrawing
{
  GtkDrawingArea da;
};
struct _SmileyDrawingClass
{
  GtkDrawingAreaClass parent_class;
  void (*color_changed) (SmileyDrawing *da);
};

//Public functions.
GType smiley_drawing_get_type(void) G_GNUC_CONST;
GtkWidget* smiley_drawing_new(void);
void smiley_drawing_set_color(SmileyDrawing *da, gdouble color_rgba[4]);
gchar* smiley_drawing_get_color(SmileyDrawing *da);
void smiley_drawing_set_name(SmileyDrawing *da, const gchar *smiley_name);
const gchar* smiley_drawing_get_name(SmileyDrawing *da);

G_END_DECLS

#endif 
