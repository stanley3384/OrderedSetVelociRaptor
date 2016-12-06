
/*

    For use with penrose_main.c. Look in penrose_main.c for more information.

    C. Eric Cashon

*/

#ifndef __PENROSE_TRIANGLE_DRAWING_H__
#define __PENROSE_TRIANGLE_DRAWING_H__

//drawing.h included after #include<gtk/gtk.h> in drawing.c. Everything is in gtk.

G_BEGIN_DECLS

#define PENROSE_TRIANGLE_DRAWING_TYPE (penrose_triangle_drawing_get_type())
#define PENROSE_TRIANGLE_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_CAST((obj), PENROSE_TRIANGLE_DRAWING_TYPE, PenroseTriangleDrawing))
#define PENROSE_TRIANGLE_DRAWING_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST((klass), PENROSE_TRIANGLE_DRAWING_TYPE, PenroseTriangleDrawingClass))
#define IS_PENROSE_TRIANGLE_DRAWING(obj) (G_TYPE_CHECK_INSTANCE_TYPE((obj), PENROSE_TRIANGLE_DRAWING_TYPE))
#define IS_PENROSE_TRIANGLE_DRAWING_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), PENROSE_TRIANGLE_DRAWING_TYPE)
#define PENROSE_TRIANGLE_DRAWING_GET_CLASS(obj) (G_TYPE_INSTANCE_GET_CLASS((obj), PENROSE_TRIANGLE_DRAWING_TYPE, PenroseTriangleDrawingClass))

typedef struct _PenroseTriangleDrawing PenroseTriangleDrawing;
typedef struct _PenroseTriangleDrawingClass PenroseTriangleDrawingClass;
struct _PenroseTriangleDrawing
{
  GtkDrawingArea da;
};
struct _PenroseTriangleDrawingClass
{
  GtkDrawingAreaClass parent_class;
  void (*color_changed) (PenroseTriangleDrawing *da);
};

//Public functions.
GType penrose_triangle_drawing_get_type(void) G_GNUC_CONST;
GtkWidget* penrose_triangle_drawing_new(void);

G_END_DECLS

#endif 
