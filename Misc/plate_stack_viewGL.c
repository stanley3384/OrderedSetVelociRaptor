
/*

     Test Code. A 3d stacked heatmapped microtiter platemap.

Test the environment and FPS on atom netbook with Mesa driver. Ubuntu 12.04. OK, it works.
    vblank_mode=0 glxgears
    glxinfo

About Xlib windows and displays.
    http://www.sbin.org/doc/Xlib/chapt_03.html

Compile with
    gcc -Wall plate_stack_viewGL.c -o plate_stack_viewGL -lGL -lGLU -lX11 -lm -lm -lgsl -lgslcblas `pkg-config --cflags --libs gtk+-3.0 gdk-x11-3.0`

C. Eric Cashon
*/

#include<X11/Xlib.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_randist.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>

static GtkWidget *window=NULL;
static GtkWidget *da=NULL;
static GdkWindow *DrawingWindow=NULL;
static Window X_window;
static Display *X_display;
static GLXContext X_context;
static XVisualInfo *X_visual;
static XWindowAttributes X_attributes;
static GLint attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
static float ang=0.0;
static guint timer_id;
static float scaleGL=2.5;
static float rotation[]={1.0 , 0.0, 0.0};
static bool rotate_drawing=true;
static gsl_matrix *test_data_points= NULL;

//Set some values to test.
static int rows=8;
static int columns=12;
static int plates=7;

void get_data_points(int rows1, int columns1, int plates1)
 {
   int i=0;
   int j=0;
   int k=0;
   const gsl_rng_type *T;
   gsl_rng *r;
   gsl_rng_env_setup();
   T = gsl_rng_mt19937;
   r = gsl_rng_alloc(T);

   //Global variable. Allocate once. Free on exit.
   if(test_data_points==NULL) test_data_points=gsl_matrix_alloc(plates*rows, columns);
   //printf("rows %i columns %i\n", test_data_points->size1, test_data_points->size2);

   for(i=0;i<plates;i++)
      {
        for(j=0;j<rows;j++)
           {
              for(k=0;k<columns;k++)
                 {
                   //printf("plate %i row %i column %i plate_counter %i counter %i matrix_row %i matrix_column %i\n", i, j, k, (columns*j)+k, (rows*columns*i)+(columns*j)+k, i*rows+j, k);
                   gsl_matrix_set(test_data_points, i*rows+j, k, gsl_rng_uniform(r));
                   //printf("%f ", gsl_matrix_get(test_data_points, i*rows+j, k));
                 }
             // printf("\n");
           }
      }
 }
static void heatmap_rgb(double temp1, double high, double low, float rgb[])
 {
    //reset rgb. >50% green to red. <50% green to blue.
    if(temp1>(high-low)/2.0)
      {
        rgb[0]=0.0;
        rgb[1]=1.0;
        rgb[2]=0.0;
        rgb[0]=rgb[0]+(temp1*(high-low));
        rgb[1]=rgb[1]-(temp1*(high-low));
      }
    else
      {
       rgb[0]=0.0;
       rgb[1]=1.0;
       rgb[2]=0.0;
       rgb[1]=rgb[1]-((1.0-temp1)*(high-low));
       rgb[2]=rgb[2]+((1.0-temp1)*(high-low));
      }
    
    //printf("Value %f red %f green %f blue %f scale %f\n", temp1, rgb[0], rgb[1], rgb[2], temp1*(high-low));
 }
static void drawGL(GtkWidget *da, gpointer data)
 {
    int i=0;
    int j=0;
    int k=0;
    double temp1=0.0;
    float rgb[]={0.0, 0.0, 0.0};
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
	
    //Rotate around x-axis
    glRotatef(ang, rotation[0], rotation[1], rotation[2]);

    //Scale
    glScalef(scaleGL, scaleGL, scaleGL);
  
    glShadeModel(GL_FLAT);
    
    //Axis lines
    glLineWidth(4.0);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(rows, 0.0, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, columns, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, plates);
    glEnd();

    glPointSize(8.0);
    glBegin(GL_POINTS);
    
    for(i=0; i<plates; i++)
       {
         for(j=0; j<rows; j++)
            {
              for(k=0;k<columns;k++)
                 {
                   temp1=gsl_matrix_get(test_data_points, i*rows+j, k);
                   heatmap_rgb(temp1, 1.0, 0.0, rgb);
                   glColor4f(rgb[0], rgb[1], rgb[2], 1.0);
                   glVertex3f(j/3.0,k,i);
                 }
            }
       }
    glEnd();

    glPopMatrix ();
    glXSwapBuffers(X_display, X_window);

 }
static void configureGL(GtkWidget *da, gpointer data)
 {
   printf("Congigure GL\n");
   DrawingWindow=gtk_widget_get_window(GTK_WIDGET(da));

   if(DrawingWindow==NULL)
     {
       printf("Couldn't get GdkWindow\n");
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
   glScalef(2.5, 2.5, 2.5);

 }
static gboolean rotate(gpointer data)
 {
   if(rotate_drawing==true)
     {
       ang++;
       gtk_widget_queue_draw_area(GTK_WIDGET(da), 0, 0, 500, 550); 
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
static void close_program()
 {
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   //free matrix on exit.
   gsl_matrix_free(test_data_points);
   printf("Quit Program\n");
   gtk_main_quit();
 }
int main(int argc, char **argv)
 {
   GtkWidget *rotate_menu, *rotate_x, *rotate_y, *rotate_z, *menu_bar, *rotate_item;
   int x1=0;
   int y1=1;
   int z1=2;
   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "GTK3 OpenGL");
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
   menu_bar=gtk_menu_bar_new();
   gtk_widget_show(menu_bar);
   rotate_item=gtk_menu_item_new_with_label("Rotate");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(rotate_item), rotate_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), rotate_item);

   GtkWidget *label1=gtk_label_new("OpenGL Drawing Area with Scale Slider");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *scale1=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,1,20,1);
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

   //Test some random points in plate format.
   get_data_points(rows, columns, plates);

   timer_id=g_timeout_add(1000/10, rotate, da);

   gtk_main();
   return 0;
}

