
/*

From; An example of using GtkGLExt in C
      Written by Davyd Madeley <davyd@madeley.id.au> and made available under a
      BSD license.
  
Test code derived from the above reference. GTK2 example code. Put a couple of axis lines to rotate around and a pyramid in.

C. Eric Cashon

gcc gtk_GL.c -o gtk_GL `pkg-config --cflags --libs gtk+-2.0 gtkglext-1.0 gtkglext-x11-1.0`


*/

#include <gtk/gtk.h>
#include <gtk/gtkgl.h>
#include <GL/gl.h>

float boxv[][3] = {
	{ -0.5, -0.5, -0.5 },
	{  0.5, -0.5, -0.5 },
	{  0.5,  0.5, -0.5 },
	{ -0.5,  0.5, -0.5 },
	{ -0.5, -0.5,  0.5 },
	{  0.5, -0.5,  0.5 },
	{  0.5,  0.5,  0.5 },
	{ -0.5,  0.5,  0.5 }
};

float pyramidv[][3]={
        {0.5, 0.5, 0.5},
        {0.5, -0.5, 0.5},
        {-0.5, 0.5, 0.5},
        {-0.5, -0.5, 0.5},
        {0.0, 0.0, 1.5}
};

static float ALPHA=1.0;
static float ang=0.0;

static gboolean expose(GtkWidget *da, GdkEventExpose *event, gpointer user_data)
  {
    GdkGLContext *glcontext = gtk_widget_get_gl_context(da);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(da);

    if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
      {
	g_assert_not_reached ();
      }

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
	
    //Rotate around x-axis
    glRotatef(ang, 1.0, 0.0, 0.0);
  
    glShadeModel(GL_FLAT);

    //Pyramid
    glBegin(GL_TRIANGLES);
    glColor4f(0.0, 0.0, 1.0, ALPHA);
    glVertex3fv(pyramidv[0]);
    glVertex3fv(pyramidv[1]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(pyramidv[1]);
    glVertex3fv(pyramidv[2]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(pyramidv[2]);
    glVertex3fv(pyramidv[3]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(pyramidv[0]);
    glVertex3fv(pyramidv[3]);
    glVertex3fv(pyramidv[4]);
    glEnd();

    //Cube
    glBegin (GL_QUADS);
    glColor4f(0.0, 0.0, 1.0, ALPHA);
    glVertex3fv(boxv[0]);
    glVertex3fv(boxv[1]);
    glVertex3fv(boxv[2]);
    glVertex3fv(boxv[3]);

    glVertex3fv(boxv[0]);
    glVertex3fv(boxv[4]);
    glVertex3fv(boxv[5]);
    glVertex3fv(boxv[1]);
	
    glVertex3fv(boxv[2]);
    glVertex3fv(boxv[6]);
    glVertex3fv(boxv[7]);
    glVertex3fv(boxv[3]);
	
    glVertex3fv(boxv[0]);
    glVertex3fv(boxv[3]);
    glVertex3fv(boxv[7]);
    glVertex3fv(boxv[4]);
	
    glVertex3fv(boxv[1]);
    glVertex3fv(boxv[5]);
    glVertex3fv(boxv[6]);
    glVertex3fv(boxv[2]);
    glEnd ();

    //Axis lines
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.8, 0.0, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.8, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.8);
    glEnd();

    //Cube and pyramid lines
    glBegin(GL_LINES);
    glColor3f (1.0, 1.0, 1.0);

    glVertex3fv(pyramidv[0]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(pyramidv[1]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(pyramidv[2]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(pyramidv[3]);
    glVertex3fv(pyramidv[4]);

    glVertex3fv(boxv[0]);
    glVertex3fv(boxv[1]);
	
    glVertex3fv(boxv[1]);
    glVertex3fv(boxv[2]);
	
    glVertex3fv(boxv[2]);
    glVertex3fv(boxv[3]);
	
    glVertex3fv(boxv[3]);
    glVertex3fv(boxv[0]);
	
    glVertex3fv(boxv[4]);
    glVertex3fv(boxv[5]);
	
    glVertex3fv(boxv[5]);
    glVertex3fv(boxv[6]);
	
    glVertex3fv(boxv[6]);
    glVertex3fv(boxv[7]);
	
    glVertex3fv(boxv[7]);
    glVertex3fv(boxv[4]);
	
    glVertex3fv(boxv[0]);
    glVertex3fv(boxv[4]);
	
    glVertex3fv(boxv[1]);
    glVertex3fv(boxv[5]);
	
    glVertex3fv(boxv[2]);
    glVertex3fv(boxv[6]);
	
    glVertex3fv(boxv[3]);
    glVertex3fv(boxv[7]);
    glEnd();

    glPopMatrix ();

    if(gdk_gl_drawable_is_double_buffered(gldrawable))
      {
	gdk_gl_drawable_swap_buffers(gldrawable);
      }
    else
      {
	glFlush();
      }

    gdk_gl_drawable_gl_end(gldrawable);

    return TRUE;
}

static gboolean configure (GtkWidget *da, GdkEventConfigure *event, gpointer user_data)
  {
    GdkGLContext *glcontext = gtk_widget_get_gl_context(da);
    GdkGLDrawable *gldrawable = gtk_widget_get_gl_drawable(da);

    if(!gdk_gl_drawable_gl_begin(gldrawable, glcontext))
      {
	g_assert_not_reached();
      }

    glLoadIdentity();
    glViewport(0, 0, da->allocation.width, da->allocation.height);
    glOrtho(-10,10,-10,10,-20050,10000);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glScalef(5.0, 5.0, 5.0);
	
    gdk_gl_drawable_gl_end(gldrawable);

    return TRUE;
 }
static gboolean rotate(gpointer user_data)
 {
   GtkWidget *da = GTK_WIDGET(user_data);

   ang++;

   gdk_window_invalidate_rect(da->window, &da->allocation, FALSE);
   gdk_window_process_updates(da->window, FALSE);

   return TRUE;
 }
int main(int argc, char **argv)
 {
   GtkWidget *window=NULL;
   GtkWidget *da=NULL;
   GdkGLConfig *glconfig;

   gtk_init(&argc, &argv);
   gtk_gl_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW (window), 500, 500);
   da=gtk_drawing_area_new();

   gtk_container_add(GTK_CONTAINER (window), da);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
   gtk_widget_set_events(da, GDK_EXPOSURE_MASK);

   gtk_widget_show(window);

   glconfig=gdk_gl_config_new_by_mode(GDK_GL_MODE_RGB | GDK_GL_MODE_DEPTH | GDK_GL_MODE_DOUBLE);

   if(!glconfig)
     {
	g_assert_not_reached();
     }

   if(!gtk_widget_set_gl_capability(da, glconfig, NULL, TRUE, GDK_GL_RGBA_TYPE))
     {
	g_assert_not_reached ();
     }

   g_signal_connect(da, "configure-event", G_CALLBACK (configure), NULL);
   g_signal_connect(da, "expose-event", G_CALLBACK (expose), NULL);

   gtk_widget_show_all (window);

   g_timeout_add (1000/60, rotate, da);

   gtk_main ();
}

