
/*

    For use with penrose_main.c. Look in penrose_main.c for more information.

    C. Eric Cashon

*/

#ifndef __PENROSE_TRIANGLE_DRAWING_H__
#define __PENROSE_TRIANGLE_DRAWING_H__

G_BEGIN_DECLS

struct _PenroseTriangleDrawing
{
  GtkDrawingArea da;
};

//Use the newer declarations.
#define PENROSE_TRIANGLE_DRAWING_TYPE penrose_triangle_drawing_get_type()
G_DECLARE_FINAL_TYPE(PenroseTriangleDrawing, penrose_triangle_drawing, PENROSE_TRIANGLE, DRAWING, GtkDrawingArea)

GtkWidget* penrose_triangle_drawing_new(void);

void penrose_triangle_drawing_set_background(PenroseTriangleDrawing *da, const gchar *background_string);
void penrose_triangle_drawing_set_foreground(PenroseTriangleDrawing *da, const gchar *foreground_string);
void penrose_triangle_drawing_set_draw_css(PenroseTriangleDrawing *da, gboolean draw_css);
const gchar* penrose_triangle_drawing_get_background(PenroseTriangleDrawing *da);
const gchar* penrose_triangle_drawing_get_foreground(PenroseTriangleDrawing *da);
gboolean penrose_triangle_drawing_get_draw_css(PenroseTriangleDrawing *da);

G_END_DECLS

#endif 
