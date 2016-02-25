/*
     Test opencv drawing with GTK+.
    
     This apt-get for OpenCV works with GTK version 2 not version 3.

     sudo apt-get install libopencv-dev

     gcc -Wall opencv_test1.c -o opencv_test1 `pkg-config gtk+-2.0 opencv --cflags --libs` -lm

     Tested on Ubuntu14.04

     C. Eric Cashon
*/ 
 
#include<gtk/gtk.h>
#include<opencv/cv.h>
#include<highgui.h>

static gboolean draw_image(GtkWidget *widget, GdkEventExpose *event, gpointer data);
static GdkPixbuf* get_opencv_image(IplImage *opencv_image, const gchar *image_file);
static GdkPixbuf* get_pixbuf_image(const gchar *image_file);
static GdkPixbuf* get_pixbuf_image_rgb(gint width, gint height);
static GdkPixbuf* get_pixbuf_remove_rgb(GdkPixbuf *pixbuf, gint color);

//Image file in local folder. Tested with (width 391 height 248 step 1176 channels 3 order 0) image.
gchar *image_file="dino3.png";

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);
 
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
    gtk_window_set_title(GTK_WINDOW(window), "Remove Color");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size (GTK_WINDOW(window), 800, 400);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //Reference image.
    GtkWidget *image=gtk_image_new_from_file(image_file);
    GtkWidget *view1=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view1), image);
    GtkWidget *scroll1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll1), view1);

    IplImage *opencv_image=NULL;
    GdkPixbuf *pixbuf1=get_opencv_image(opencv_image, image_file);
 
    //Put the pixbuf from opencv in a drawing area with green edited out and draw circles with cairo.
    GtkWidget *drawing_area=gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 2000, 2000);
    //g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_image), pixbuf1);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(draw_image), pixbuf1);
    GtkWidget *view2=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view2), drawing_area);
    GtkWidget *scroll2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll2), view2);

    //Just use a pixbuf to edit out blue from the image.
    GdkPixbuf *pixbuf2=get_pixbuf_image(image_file);
    GtkWidget *image2=gtk_image_new_from_pixbuf(pixbuf2);
    GtkWidget *view3=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view3), image2);
    GtkWidget *scroll3 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll3), view3);

    //Make a rgb pixbuf to compare with above images.
    GdkPixbuf *pixbuf3=get_pixbuf_image_rgb(200, 200);
    GtkWidget *image3=gtk_image_new_from_pixbuf(pixbuf3);
    GtkWidget *view4=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view4), image3);
    GtkWidget *scroll4 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll4), view4);

    //Remove green from rgb.
    GdkPixbuf *pixbuf4=get_pixbuf_remove_rgb(pixbuf3, 1);
    GtkWidget *image4=gtk_image_new_from_pixbuf(pixbuf4);
    GtkWidget *view5=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view5), image4);
    GtkWidget *scroll5 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll5), view5);

    //Remove blue from rgb.
    GdkPixbuf *pixbuf5=get_pixbuf_remove_rgb(pixbuf3, 2);
    GtkWidget *image5=gtk_image_new_from_pixbuf(pixbuf5);
    GtkWidget *view6=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view6), image5);
    GtkWidget *scroll6 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll6), view6);

    GtkWidget *table=gtk_table_new(2, 3, TRUE);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll1, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll2, 1, 2, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll3, 2, 3, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll4, 0, 1, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll5, 1, 2, 1, 2);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll6, 2, 3, 1, 2);

    gtk_container_add(GTK_CONTAINER(window), table);
 
    gtk_widget_show_all(window);
    gtk_main();

    if(opencv_image!=NULL) cvReleaseImage(&opencv_image);
    g_object_unref(pixbuf1);
    g_object_unref(pixbuf2);
    g_object_unref(pixbuf3);
    g_object_unref(pixbuf4);
    g_object_unref(pixbuf5);

    return 0;
  }
/*
//GTK3 doesn't work with this.
gboolean draw_image(GtkWidget *widget, cairo_t *cr, gpointer data)
  { 
    GdkWindow *win=gtk_widget_get_window(widget);
    cairo_surface_t *surface=gdk_cairo_surface_create_from_pixbuf(GDK_PIXBUF(data), 1, win);
    gdk_cairo_set_source_window(cr, win, 0, 0);
    cairo_surface_destroy(surface); 
 
    return TRUE;
  }
*/
//Draw some circles over the image.
gboolean draw_image(GtkWidget *widget, GdkEventExpose *event, gpointer data)
  {  
    gint width=gdk_pixbuf_get_width((GdkPixbuf*)data);
    gint height=gdk_pixbuf_get_height((GdkPixbuf*)data);
    gdk_draw_pixbuf(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)], (GdkPixbuf*)(data), 0, 0, 0, 0, width, height, GDK_RGB_DITHER_NONE, 0, 0);

    //Draw over image with Cairo.
    cairo_t *cr=gdk_cairo_create(widget->window);
    cairo_set_line_width(cr, 6);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_arc(cr, 125, 100, 50, 0, 2*G_PI);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_arc(cr, 175, 100, 50, 0, 2*G_PI);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_arc(cr, 75, 100, 50, 0, 2*G_PI);
    cairo_stroke(cr);
    cairo_destroy(cr);
 
    return TRUE;
  } 
static GdkPixbuf* get_opencv_image(IplImage *opencv_image, const gchar *image_file)
  {
    //Get an OpenCV image.
    opencv_image=cvLoadImage(image_file, CV_LOAD_IMAGE_COLOR);
 
    //Some data about the image.
    gint width=opencv_image->width;
    gint height=opencv_image->height;
    gint step=opencv_image->widthStep;
    gint channels=opencv_image->nChannels;
    gint order=opencv_image->dataOrder;
    guchar *data=(guchar*)opencv_image->imageData;
    g_print("width %i height %i step %i channels %i order %i\n", width, height, step, channels, order);
    
    //Test changing some pixels in the open_cv image.
    gint i=0;
    gint j=0;
    gint width1=width*channels;
    for(i=0;i<height;i++)
      {
        for(j=0;j<width1;j+=channels)
          {
            //Zero out the green channel.
            data[i*width1+j+1]=0;  
          }
      }
    
    cvCvtColor(opencv_image, opencv_image, CV_BGR2RGB); 

    //Change opencv image to a pixbuf.
    GdkPixbuf *pixbuf=gdk_pixbuf_new_from_data((guchar*)opencv_image->imageData, GDK_COLORSPACE_RGB, FALSE, opencv_image->depth, width, height, step, NULL, NULL);

    return pixbuf;
  }
static GdkPixbuf* get_pixbuf_image(const gchar *image_file)
  {
    gint i=0;
    gint j=0;
    GdkPixbuf *pixbuf=gdk_pixbuf_new_from_file(image_file, NULL);
    gint width=gdk_pixbuf_get_width(pixbuf);
    gint height=gdk_pixbuf_get_height(pixbuf);
    gint step=gdk_pixbuf_get_rowstride(pixbuf);
    gint channels=gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels, *p;
    pixels=gdk_pixbuf_get_pixels(pixbuf);

    width=width*channels;
    for(i=0;i<width;i++)
      {
        for(j=0;j<height;j++)
          {
            p=pixels+j*step+i*channels;
            //Remove blue.
            p[2]=0;
          }
      }

    return pixbuf;
  }
static GdkPixbuf* get_pixbuf_image_rgb(gint width, gint height)
  {
    gint i=0;
    gint j=0;
    GdkPixbuf *pixbuf=gdk_pixbuf_new(GDK_COLORSPACE_RGB, FALSE, 8, width, height);
    gint step=gdk_pixbuf_get_rowstride(pixbuf);
    gint channels=gdk_pixbuf_get_n_channels(pixbuf);
    guchar *pixels, *p;
    pixels=gdk_pixbuf_get_pixels(pixbuf);
    gint sections=height/3;
    gint sections2=sections*2;
    gint extra=height%3;
    gint sections3=sections*3+extra;
    
    for(i=0;i<width;i++)
      {
        for(j=0;j<sections;j++)
          {
            p=pixels+j*step+i*channels;
            p[0]=255;
            p[1]=0;
            p[2]=0;
          }
      }
    for(i=0;i<width;i++)
      {
        for(j=sections;j<sections2;j++)
          {
            p=pixels+j*step+i*channels;
            p[0]=0;
            p[1]=255;
            p[2]=0;
          }
      }
    for(i=0;i<width;i++)
      {
        for(j=sections2;j<sections3;j++)
          {
            p=pixels+j*step+i*channels;
            p[0]=0;
            p[1]=0;
            p[2]=255;
          }
      }
    
    return pixbuf;
  }
static GdkPixbuf* get_pixbuf_remove_rgb(GdkPixbuf *pixbuf, gint color)
  {
    gint i=0;
    gint j=0;
    GdkPixbuf *pixbuf2=gdk_pixbuf_copy(pixbuf);
    gint width=gdk_pixbuf_get_width(pixbuf2);
    gint height=gdk_pixbuf_get_height(pixbuf2);
    gint step=gdk_pixbuf_get_rowstride(pixbuf2);
    gint channels=gdk_pixbuf_get_n_channels(pixbuf2);
    guchar *pixels, *p;
    pixels=gdk_pixbuf_get_pixels(pixbuf2);
    
    if(color==0)
      {
        for(i=0;i<width;i++)
          {
            for(j=0;j<height;j++)
              {
                p=pixels+j*step+i*channels;
                p[0]=0;
              }
          }
      }
    else if(color==1)
      {
        for(i=0;i<width;i++)
          {
            for(j=0;j<height;j++)
              {
                p=pixels+j*step+i*channels;
                p[1]=0;
              }
          }
      }
    else
      {
        for(i=0;i<width;i++)
          {
            for(j=0;j<height;j++)
              {
                p=pixels+j*step+i*channels;
                p[2]=0;
              }
          }
      }

    return pixbuf2;
   
  }






