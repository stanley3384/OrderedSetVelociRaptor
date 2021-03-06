
/*

     Test Code. Get OpenGL working with shaders in GTK3. Uses plate.vert and plate.frag shaders. Change around as needed. Try a glu tessellator for shading.
     The http://www.lighthouse3d.com/opengl/glsl/ tutorial is very helpful.

Test the environment and FPS on atom netbook with Mesa driver. Ubuntu 12.04. OK, it works.
    vblank_mode=0 glxgears
    glxinfo | grep "OpenGL version"
    glxinfo
    xrandr

About Xlib windows and displays.
    http://www.sbin.org/doc/Xlib/chapt_03.html

Compile with
    gcc -Wall gtk3shadersGL.c -o gtk3shadersGL -lGL -lGLU -lGLEW -lX11 -lm `pkg-config --cflags --libs gtk+-3.0 gdk-x11-3.0`

C. Eric Cashon
*/

#include<X11/Xlib.h>
#include<GL/glew.h>
#include<GL/glu.h>
#include<GL/glx.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>


static GtkWidget *window=NULL;
static GtkWidget *da=NULL;
static GdkWindow *DrawingWindow=NULL;
static Window X_window;
static Display *X_display=NULL;
static GLXContext X_context;
static XVisualInfo *X_visual=NULL;
static XWindowAttributes X_attributes;
//Let glXChooseVisual pick color depth. Start with a low color depth of 8.
static GLint attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 8, GLX_DOUBLEBUFFER, None};
static float ang=0.0;
static guint timer_id;
static float scaleGL=2.5;
static float rotation[]={1.0 , 0.0, 0.0};
static bool rotate_drawing=true;
//For vertex and fragment shaders.
GLuint vShader,fShader,pShader;
GLfloat SquareVertices[]={1.0,1.0,0.0, 1.0,-1.0,0.0, -1.0,-1.0,0.0, -1.0,1.0,0.0};
//Change red to purple in shaders.
GLfloat Colors[]={1.0,0.0,0.0,0.5, 1.0,0.0,0.0,0.5, 0.0,0.0,1.0,1.0, 0.0,0.0,1.0,1.0};
//For glu tessellator.
GLdouble TessSquare[16][3]={{1.0,1.0,0.0}, {1.0,-1.0,0.0}, {-1.0,-1.0,0.0}, {-1.0,1.0,0.0}, {0.75,0.75,0.0}, {0.75,-0.75,0.0}, {-0.75,-0.75,0.0}, {-0.75,0.75,0.0}, {0.5,0.5,0.0}, {0.5,-0.5,0.0}, {-0.5,-0.5,0.0}, {-0.5,0.5,0.0}, {0.25,0.25,0.0}, {0.25,-0.25,0.0}, {-0.25,-0.25,0.0}, {-0.25,0.25,0.0}};
GLUtesselator *tess=NULL;
bool tessellate=false;

static void set_shaders()
 {
   GMappedFile *mapVS=NULL;
   GMappedFile *mapFS=NULL;
   const char *vs=NULL;
   const char *fs=NULL;
   GLint isCompiled=0;

   //global vShader and fShader
   vShader = glCreateShader(GL_VERTEX_SHADER);
   fShader = glCreateShader(GL_FRAGMENT_SHADER);

   mapVS = g_mapped_file_new("plate.vert", FALSE, NULL);
   g_print("Vertex Shader chars %i\n", g_mapped_file_get_length(mapVS));
   vs=g_mapped_file_get_contents(mapVS);
   //g_print("%s\n", vs);

   mapFS = g_mapped_file_new("plate.frag", FALSE, NULL);
   g_print("Fragment Shader chars %i\n", g_mapped_file_get_length(mapFS));
   fs=g_mapped_file_get_contents(mapFS);
   //g_print("%s\n", fs);

   glShaderSource(vShader, 1, &vs, NULL);
   glShaderSource(fShader, 1, &fs, NULL);

   g_mapped_file_unref(mapVS);
   g_mapped_file_unref(mapFS);

   glCompileShader(vShader);
   glGetShaderiv(vShader, GL_COMPILE_STATUS, &isCompiled);
   if(isCompiled == GL_FALSE) g_print("Vertex shader didn't compile!\n");
   else g_print("Vertex Shader Compiled\n");

   isCompiled=0;
   glCompileShader(fShader);
   glGetShaderiv(fShader, GL_COMPILE_STATUS, &isCompiled);
   if(isCompiled == GL_FALSE) g_print("Fragment shader didn't compile!\n");
   else g_print("Fragment Shader Compiled\n");

   //global pShader
   pShader = glCreateProgram();
   glAttachShader(pShader,fShader);
   glAttachShader(pShader,vShader);

   glLinkProgram(pShader);
   GLint isLinked = 0;
   glGetProgramiv(pShader, GL_LINK_STATUS, &isLinked);
   if(isLinked == GL_FALSE) g_print("Couldn't link shaders!\n");
   else g_print("Shaders Linked\n");

   //Check info log.
   int infologLength=0;
   int charsWritten=0;
   char *infoLog=NULL;
   glGetProgramiv(pShader ,GL_INFO_LOG_LENGTH ,&infologLength);
   if(infologLength>0)
     {
       infoLog=(char *)malloc(infologLength);
       glGetProgramInfoLog(pShader, infologLength, &charsWritten, infoLog);
       if(charsWritten>0) g_print("Info Log %s\n",infoLog);
       free(infoLog);
     }

   glUseProgram(pShader);

   //Get versions
   printf("OpenGL %s\n", glGetString(GL_VERSION));
   printf("GLSL %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
   printf("GLU %s\n", gluGetString(GLU_VERSION));
   int major=0;
   int minor=0;
   glXQueryVersion(X_display, &major, &minor);
   printf("GLX version %i %i\n", major, minor);
   printf("GLEW %s\n", glewGetString(GLEW_VERSION));

  }
static void drawGL(GtkWidget *da, cairo_t *cr, gpointer data)
 {
    int i=0;
    int j=0;
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glLoadIdentity();
    gluLookAt(0.0,5.0,10.0, 0.0,0.0,0.0,0.0f,1.0f,0.0f);
   
    //Rotate around x-axis
    glRotatef(ang, rotation[0], rotation[1], rotation[2]);

    //Scale
    glScalef(scaleGL, scaleGL, scaleGL);

    //Draw a square.
    if(tessellate==false)
      {        
        glDrawArrays(GL_QUADS, 0, 4);
      } 

    //The tessellator.
    if(tessellate==true)
      {  
        gluTessBeginPolygon(tess, NULL);
        for(i=0;i<4;i++)
           {
             gluTessBeginContour(tess);
             for(j=0;j<4;j++)
                {
                  gluTessVertex(tess, TessSquare[4*i+j], TessSquare[4*i+j]);
                }
             gluTessEndContour(tess); 
           }
        gluTessEndPolygon(tess); 

        //Line loops around contours.
        for(i=0;i<4;i++)
           {
             glBegin(GL_LINE_LOOP);
             for(j=0;j<4;j++)
                {
                  glVertex3f(TessSquare[4*i+j][0], TessSquare[4*i+j][1], TessSquare[4*i+j][2]);
                }
             glEnd(); 
           }
        
      }     

    //Axis lines
    glColor4f(1.0, 0.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(1.8, 0.0, 0.0);
    glEnd();
	
    glColor4f(0.0, 1.0, 0.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 1.8, 0.0);
    glEnd();
	
    glColor4f(0.0, 0.0, 1.0, 1.0);
    glBegin(GL_LINES);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 1.8);
    glEnd();

    glXSwapBuffers(X_display, X_window);
 }
void beginCallback(GLenum which)
{
   glBegin(which);
}
void endCallback(void)
{
   glEnd();
}
void errorCallback(GLenum errorCode)
{
   const GLubyte *estring;
   estring = gluErrorString(errorCode);
   fprintf (stderr, "Tessellation Error: %s\n", estring);
   exit(0);
}
void vertexCallback(GLvoid *vertex)
{
   const GLdouble *pointer;
   pointer = (GLdouble *) vertex;
   glColor3dv(pointer+3);
   glVertex3dv(vertex);
}
static void configureGL(GtkWidget *da, gpointer data)
 {
   printf("Get Drawing Area\n");
   DrawingWindow=gtk_widget_get_window(GTK_WIDGET(da));

   if(DrawingWindow==NULL)
     {
        printf("Couldn't get GdkWindow!\n");
     }
   else if(gtk_widget_get_allocated_width(da)<2)
     {
        printf("Drawing Width %i\n", gtk_widget_get_allocated_width(da));
     }
   else
     {
       GdkScreen *g_screen=gdk_screen_get_default();
       GdkVisual *g_visual=gdk_screen_get_system_visual(g_screen);
       //Get list of available color depths.
       GList *screen_list=gdk_screen_list_visuals(g_screen);
       GList *temp=NULL;
       int depth1=0;
       int depth2=0;
       printf("Available Color Depths ");
       temp=screen_list;
       if(temp!=NULL)
         {
           while(temp!=NULL)
             {
               depth1=gdk_visual_get_depth(GDK_VISUAL(temp->data));
               if(depth1!=depth2) printf("%i ", depth1);
               depth2=depth1;
               temp=temp->next;
             }
           printf("\n");
         }
       else printf("Couldn't get color depths!\n");
       g_list_free(screen_list);
       //Check current color depth.
       printf("Current Color Depth %i\n", gdk_visual_get_depth(g_visual));
       //Get x window.
       X_window=gdk_x11_window_get_xid(GDK_WINDOW(DrawingWindow));
       X_display=gdk_x11_get_default_xdisplay();
       //Get best visual for glX. 
       X_visual=glXChooseVisual(X_display, 0, attributes);
       if(X_visual==NULL) printf("Couldn't get GL display visual!\n");
       X_context=glXCreateContext(X_display, X_visual, NULL, GL_TRUE);
       XGetWindowAttributes(X_display, X_window, &X_attributes);
       glXMakeCurrent(X_display, X_window, X_context);
       XMapWindow(X_display, X_window);
      }

   if((int)X_attributes.width>1)
     {
       printf("Configure GL\n");
       printf("GL Viewport Width %i Height %i ColorDepth %i\n", (int)X_attributes.width, (int)X_attributes.height, (int)X_attributes.depth);
       //Initialize glew.
       glewExperimental=GL_TRUE;
       printf("Initialize GLEW with GL version 2.0\n");
       GLenum err=glewInit();
       if(err != GLEW_OK) printf("Error: %s\n", glewGetErrorString(err)); 
       if(glewIsSupported("GL_VERSION_2_0")) printf("Ready for OpenGL 2.0\n");
       else printf("OpenGL 2.0 not supported\n");
       //Compile shaders
       set_shaders();
       glMatrixMode(GL_PROJECTION);
       glLoadIdentity();
       glViewport(0, 0, X_attributes.width, X_attributes.height);	
       //Set the perspective.
       gluPerspective(45,X_attributes.width/X_attributes.height,1,1000);
       glMatrixMode(GL_MODELVIEW);
       //Set Square Array.
       glEnableClientState(GL_VERTEX_ARRAY);
       glEnableClientState(GL_COLOR_ARRAY);
       glVertexPointer(3, GL_FLOAT, 0, SquareVertices); 
       glColorPointer(4, GL_FLOAT, 0, Colors);
       //Set tessellator and callback functions.
       tess = gluNewTess();
       gluTessCallback(tess, GLU_TESS_VERTEX, (GLvoid (*) ()) &vertexCallback);
       gluTessCallback(tess, GLU_TESS_BEGIN, (GLvoid (*) ()) &beginCallback);
       gluTessCallback(tess, GLU_TESS_END, (GLvoid (*) ()) &endCallback);
       gluTessCallback(tess, GLU_TESS_ERROR, (GLvoid (*) ()) &errorCallback);
       gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE); 
       //gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_ODD);
       //gluTessProperty(tess, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_NONZERO);
       glShadeModel(GL_SMOOTH);
     }
 }
static gboolean rotate(gpointer data)
 {
   if(rotate_drawing==true)
     {
       ang++;
       gtk_widget_queue_draw_area(GTK_WIDGET(da), 0, 0, gtk_widget_get_allocated_width(da), gtk_widget_get_allocated_height(da)); 
     } 
   return TRUE;
 }
static void stop_rotation(GtkWidget *da, gpointer data)
 {
   if(rotate_drawing==true) rotate_drawing=false;
   else rotate_drawing=true;
 }
static void scale_drawing(GtkRange *range,  gpointer data)
 {  
   scaleGL=2.5-gtk_range_get_value(range)/5.0;     
 }
static void rotation_axis(GtkWidget *axis, gpointer data)
 {
   if(*(int*)data==0)
     {
       rotation[0]=1.0;
       rotation[1]=0.0;
       rotation[2]=0.0;
     }
   else if(*(int*)data==1)
     {
       rotation[0]=0.0;
       rotation[1]=1.0;
       rotation[2]=0.0;
     }
   else
     {
       rotation[0]=0.0;
       rotation[1]=0.0;
       rotation[2]=1.0;
     }
 }
static void draw_glu_tessellation(GtkWidget *menu_item, gpointer data)
 {
   if(tessellate==true)
      {
        tessellate=false;
        gtk_menu_item_set_label(GTK_MENU_ITEM(menu_item), "Glu Tessellate True");
      }
   else
      {
        tessellate=true;
        gtk_menu_item_set_label(GTK_MENU_ITEM(menu_item), "Glu Tessellate False");
      }
 }
static void close_program()
 {
   glXDestroyContext(X_display, X_context);
   XFree(X_visual);
   //Remove tessellator.
   gluDeleteTess(tess);
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   printf("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   GtkWidget *rotate_menu, *rotate_x, *rotate_y, *rotate_z, *menu_bar, *rotate_item, *draw_menu, *draw_item, *draw_tess;
   int x1=0;
   int y1=1;
   int z1=2;
   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "GTK3 OpenGL with Shaders");
   gtk_window_set_default_size(GTK_WINDOW(window), 500, 550);

   rotate_menu=gtk_menu_new();
   rotate_x=gtk_menu_item_new_with_label("Rotate x-axis");
   rotate_y=gtk_menu_item_new_with_label("Rotate y-axis");
   rotate_z=gtk_menu_item_new_with_label("Rotate z-axis");
   gtk_menu_shell_append(GTK_MENU_SHELL(rotate_menu), rotate_x);
   gtk_menu_shell_append(GTK_MENU_SHELL(rotate_menu), rotate_y);
   gtk_menu_shell_append(GTK_MENU_SHELL(rotate_menu), rotate_z);
   g_signal_connect(rotate_x, "activate", G_CALLBACK(rotation_axis), &x1);
   g_signal_connect(rotate_y, "activate", G_CALLBACK(rotation_axis), &y1);
   g_signal_connect(rotate_z, "activate", G_CALLBACK(rotation_axis), &z1);
   draw_menu=gtk_menu_new();
   draw_tess=gtk_menu_item_new_with_label("Glu Tessellate True");
   gtk_menu_shell_append(GTK_MENU_SHELL(draw_menu), draw_tess);
   g_signal_connect(draw_tess, "activate", G_CALLBACK(draw_glu_tessellation), NULL);
   menu_bar=gtk_menu_bar_new();
   gtk_widget_show(menu_bar);
   rotate_item=gtk_menu_item_new_with_label("Rotate");
   draw_item=gtk_menu_item_new_with_label("Draw");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(rotate_item), rotate_menu);
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(draw_item), draw_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), rotate_item);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), draw_item);

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

   gtk_grid_attach(GTK_GRID(grid1), menu_bar, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), scale1, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), da, 0, 3, 1, 1);
  
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   gtk_widget_show(window);

   g_signal_connect(da, "configure-event", G_CALLBACK(configureGL), NULL);
   g_signal_connect(da, "draw", G_CALLBACK(drawGL), NULL);

   gtk_widget_show_all(window);

   timer_id=g_timeout_add(25, rotate, da);

   gtk_main();
   return 0;
}

