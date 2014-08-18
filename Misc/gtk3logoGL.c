
/*

     Test Code. Draw the GTK+ logo with OpenGL.  

Test the environment and FPS on atom netbook with Mesa driver. Ubuntu 12.04. OK, it works.
    vblank_mode=0 glxgears
    glxinfo

About Xlib windows and displays.
    http://www.sbin.org/doc/Xlib/chapt_03.html

Compile with
    gcc -Wall gtk3logoGL.c -o gtk3logoGL -lGL -lGLU -lX11 -lm `pkg-config --cflags --libs gtk+-3.0 gdk-x11-3.0`

C. Eric Cashon
*/

#include<X11/Xlib.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>

static GtkWidget *window=NULL;
static GtkWidget *da=NULL;
static GdkWindow *DrawingWindow=NULL;
static Window X_window;
static Display *X_display=NULL;
static GLXContext X_context;
static XVisualInfo *X_visual=NULL;
static XWindowAttributes X_attributes;
static GLint attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
static float angle=0.0;
static guint timer_id=0;
static float scaleGL=2.0;
static gboolean rotate_drawing=TRUE;
//Bezier points for the letter G.
GLfloat bezier_G1[4][3] = {{1.0, -0.2, 1.0}, {1.0, -1.1, -1.0}, {1.0, 1.1, -1.0}, {1.0, 0.2, 1.0}};
GLfloat bezier_G2[4][3] = {{-1.0, -0.2, 1.0}, {-1.0, -1.1, -1.0}, {-1.0, 1.1, -1.0}, {-1.0, 0.2, 1.0}};


static gboolean drawGL(GtkWidget *da, cairo_t *cr, gpointer data)
 {
    int i=0;
    GLfloat line_width=7.0;
    GLfloat point1=1.0;
    GLfloat point2=-1.0;
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glPushMatrix();
	
    glRotatef(angle, 1, 0, 0);
    glRotatef(angle, 0, -0.70, -0.5);
    glScalef(scaleGL, scaleGL, scaleGL);
    glShadeModel(GL_FLAT);

    line_width=7.0-(1.8/(scaleGL+0.3));

    //red1
    glLineWidth(line_width);
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(point2, point1, point1);
    glVertex3f(point1, point1, point1);
    glVertex3f(point1, point1, point2);
    glVertex3f(point2, point1, point2);
    glEnd();
    //Draw T on red1.
    glColor3f(0.0, 0.0, 0.0);  
    glBegin(GL_LINES);
    glVertex3f(0.4, point1, -0.4);
    glVertex3f(0.4, point1, 0.4);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(-0.5, point1, 0.0);
    glVertex3f(0.4, point1, 0.0);
    glEnd();
    

    //red2
    glColor3f(1.0, 0.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(point2, point2, point2);
    glVertex3f(point1, point2, point2);
    glVertex3f(point1, point2, point1);
    glVertex3f(point2, point2, point1);
    glEnd();
    //Draw T on red2.
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(-0.4, point2, 0.4);
    glVertex3f(-0.4, point2, -0.4);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(0.5, point2, 0.0);
    glVertex3f(-0.4, point2, 0.0);
    glEnd();

    //green1
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(point2, point2, point1);
    glVertex3f(point1, point2, point1);
    glVertex3f(point1, point1, point1);
    glVertex3f(point2, point1, point1);
    glEnd();
    //Draw K on green1.
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(-0.4, -0.3, point1);
    glVertex3f(0.4, -0.3, point1);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(0.0, -0.3, point1);
    glVertex3f(0.4, 0.3, point1);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(0.0, -0.3, point1);
    glVertex3f(-0.4, 0.3, point1);
    glEnd();

    //green2
    glColor3f(0.0, 1.0, 0.0);
    glBegin(GL_POLYGON);
    glVertex3f(point2, point1, point2);
    glVertex3f(point1, point1, point2);
    glVertex3f(point1, point2, point2);
    glVertex3f(point2, point2, point2);
    glEnd();
    //Draw K on green2.
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINES);
    glVertex3f(-0.4, 0.3, point2);
    glVertex3f(0.4, 0.3, point2);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.3, point2);
    glVertex3f(-0.4, -0.3, point2);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.3, point2);
    glVertex3f(0.4, -0.3, point2);
    glEnd();

    //blue1
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(point1, point2, point1);
    glVertex3f(point1, point2, point2);
    glVertex3f(point1, point1, point2);
    glVertex3f(point1, point1, point1);
    glEnd();
    //Draw G on blue1.
    glMap1f(GL_MAP1_VERTEX_3, -0.22, 1.2, 3, 4, &bezier_G1[0][0]);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for(i=0;i<=20;i++) glEvalCoord1f((GLfloat) i/20.0);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(point1, 0.2, 0.2);
    glVertex3f(point1, 0.4, 0.2);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(point1, 0.2, 0.2);
    glVertex3f(point1, 0.2, 0.1);
    glEnd();

    //blue2
    glColor3f(0.0, 0.0, 1.0);
    glBegin(GL_POLYGON);
    glVertex3f(point2, point2, point2);
    glVertex3f(point2, point2, point1);
    glVertex3f(point2, point1, point1);
    glVertex3f(point2, point1, point2);
    glEnd();
    //Draw G on blue2.
    glMap1f(GL_MAP1_VERTEX_3, -0.22, 1.2, 3, 4, &bezier_G2[0][0]);
    glColor3f(0.0, 0.0, 0.0);
    glBegin(GL_LINE_STRIP);
    for(i=0;i<=20;i++) glEvalCoord1f((GLfloat) i/20.0);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(point2, -0.2, 0.2);
    glVertex3f(point2, -0.4, 0.2);
    glEnd();
    glBegin(GL_LINES);
    glVertex3f(point2, -0.2, 0.2);
    glVertex3f(point2, -0.2, 0.1);
    glEnd(); 

    //Black lines on box edges.
    glBegin (GL_LINES);
    glColor3f(0.0, 0.0, 0.0);
    glVertex3f(point2, point2, point2);
    glVertex3f(point1, point2, point2);	
    glVertex3f(point1, point2, point2);
    glVertex3f(point1, point1, point2);	
    glVertex3f(point1, point1, point2);
    glVertex3f(point2, point1, point2);	
    glVertex3f(point2, point1, point2);
    glVertex3f(point2, point2, point2);	
    glVertex3f(point2, point2, point1);
    glVertex3f(point1, point2, point1);	
    glVertex3f(point1, point2, point1);
    glVertex3f(point1, point1, point1);	
    glVertex3f(point1, point1, point1);
    glVertex3f(point2, point1, point1);	
    glVertex3f(point2, point1, point1);
    glVertex3f(point2, point2, point1);	
    glVertex3f(point2, point2, point2);
    glVertex3f(point2, point2, point1);	
    glVertex3f(point1, point2, point2);
    glVertex3f(point1, point2, point1);	
    glVertex3f(point1, point1, point2);
    glVertex3f(point1, point1, point1);	
    glVertex3f(point2, point1, point2);
    glVertex3f(point2, point1, point1);
    glEnd ();

 
    glPopMatrix ();
    glXSwapBuffers(X_display, X_window);
    return TRUE;

 }
static void configureGL(GtkWidget *da, gpointer data)
 {
   g_print("Congigure GL\n");
   DrawingWindow=gtk_widget_get_window(GTK_WIDGET(da));

   if(DrawingWindow==NULL)
     {
       g_print("Couldn't get GdkWindow\n");
     }
   else
     {
       X_window=gdk_x11_window_get_xid(GDK_WINDOW(DrawingWindow));
       X_display=gdk_x11_get_default_xdisplay();
       X_visual=glXChooseVisual(X_display, 0, attributes);
       X_context=glXCreateContext(X_display, X_visual, NULL, GL_TRUE);
     }

   XGetWindowAttributes(X_display, X_window, &X_attributes);
   glXMakeCurrent(X_display, X_window, X_context);
   XMapWindow(X_display, X_window);
   printf("Viewport %i %i\n", (int)X_attributes.width, (int)X_attributes.height);
   glViewport(0, 0, X_attributes.width, X_attributes.height);
   glOrtho(-10,10,-10,10,-100,100);
   glScalef(2.0, 2.0, 2.0);
   glClearColor(0.0, 0.0, 0.0, 1.0);
   glShadeModel(GL_FLAT);
   glEnable(GL_MAP1_VERTEX_3);
   glEnable(GL_LINE_SMOOTH);
   glEnable(GL_DEPTH_TEST);
 }
static gboolean rotate(gpointer data)
 {
   if(rotate_drawing==TRUE)
     {
       angle++;
       gtk_widget_queue_draw_area(GTK_WIDGET(da), 0, 0, gtk_widget_get_allocated_width(data), gtk_widget_get_allocated_height(data)); 
     } 
   return TRUE;
 }
static void stop_rotation(GtkWidget *da, gpointer data)
 {
   if(rotate_drawing==TRUE) rotate_drawing=FALSE;
   else rotate_drawing=TRUE;
 }
static void scale_drawing(GtkRange *range,  gpointer data)
 {  
   scaleGL=2.0-gtk_range_get_value(range)/5.0;
   g_print("Scale %f\n", scaleGL);     
 }
static void close_program()
 {
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   g_print("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "GTK+ Logo");
   gtk_window_set_default_size(GTK_WINDOW(window), 500, 550);

   GtkWidget *label1=gtk_label_new("OpenGL Drawing Area with Scale Slider");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *scale1=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,1,10,1);
   gtk_widget_set_hexpand(scale1, TRUE);
   gtk_range_set_increments(GTK_RANGE(scale1),1,1);
   g_signal_connect(GTK_RANGE(scale1), "value_changed", G_CALLBACK(scale_drawing), NULL);

   da=gtk_drawing_area_new();
   gtk_widget_set_double_buffered(da, FALSE);
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK);
   g_signal_connect(da, "button-press-event", G_CALLBACK(stop_rotation), NULL);

   GtkWidget *grid1=gtk_grid_new();
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), scale1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), da, 0, 2, 1, 1);
  
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   gtk_widget_show(window);

   g_signal_connect(da, "configure-event", G_CALLBACK(configureGL), NULL);
   g_signal_connect(da, "draw", G_CALLBACK(drawGL), NULL);

   gtk_widget_show_all(window);

   timer_id=g_timeout_add(30, rotate, da);

   gtk_main();
   return 0;
}

