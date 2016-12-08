
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

G_END_DECLS

#endif 
