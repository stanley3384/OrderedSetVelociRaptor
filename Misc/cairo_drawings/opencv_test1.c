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

//Image file in local folder. Tested with (width 391 height 248 step 1176 channels 3 order 0) image.
gchar *image_file="dino3.png";

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
gboolean draw_image(GtkWidget *widget, GdkEventExpose *event, gpointer *data)
  {  
    gdk_draw_pixbuf(widget->window, widget->style->fg_gc[GTK_WIDGET_STATE(widget)], (GdkPixbuf*)(data[1]), 0, 0, 0, 0, ((IplImage*)data[0])->width, ((IplImage*)data[0])->height, GDK_RGB_DITHER_NONE, 0, 0);

    //Draw over image with Cairo.
    cairo_t *cr=gdk_cairo_create(widget->window);
    cairo_set_line_width(cr, 6);
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_arc(cr, 100, 100, 50, 0, 2*G_PI);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_arc(cr, 150, 100, 50, 0, 2*G_PI);
    cairo_stroke(cr);
    cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    cairo_arc(cr, 50, 100, 50, 0, 2*G_PI);
    cairo_stroke(cr);
    cairo_destroy(cr);
 
    return TRUE;
  } 
int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);
 
    GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL); 
    gtk_window_set_title(GTK_WINDOW(window), "Remove Color");
    gtk_window_set_default_size (GTK_WINDOW(window), 400, 400);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //Get an OpenCV image.
    IplImage *opencv_image=cvLoadImage(image_file, CV_LOAD_IMAGE_COLOR);
 
    //Some data about the image.
    gint width=opencv_image->width;
    gint height=opencv_image->height;
    gint step=opencv_image->widthStep;
    gint channels=opencv_image->nChannels;
    gint order=opencv_image->dataOrder;
    guchar *data=(guchar*)opencv_image->imageData;
    g_print("width %i height %i step %i channels %i order %i\n", width, height, step, channels, order);
    
    //Test changing some pixels.
    gint i=0;
    gint j=0;
    width=width*channels;
    for(i=0;i<height;i++)
      {
        for(j=0;j<width;j+=channels)
          {
            //Zero out the green channel.
            data[i*width+j+1]=data[i*width+j+1]&0;
            //Try some other combos. 
            //data[i*width+j+0]=data[i*width+j+0]|0b11111111;
            //data[i*width+j+0]=data[i*width+j+0]&0x00;
            //data[i*width+j+0]=data[i*width+j+0]>>1;       
          }
      }
    
    cvCvtColor(opencv_image, opencv_image, CV_BGR2RGB); 

    GdkPixbuf *pixbuf=gdk_pixbuf_new_from_data((guchar*)opencv_image->imageData, GDK_COLORSPACE_RGB, FALSE, opencv_image->depth, width, height, step, NULL, NULL);

    gpointer pix_image[]={opencv_image, pixbuf};
 
    GtkWidget *drawing_area=gtk_drawing_area_new();
    gtk_widget_set_size_request(drawing_area, 2000, 2000);
    //g_signal_connect(G_OBJECT(drawing_area), "draw", G_CALLBACK(draw_image), pixbuf);
    g_signal_connect(G_OBJECT(drawing_area), "expose_event", G_CALLBACK(draw_image), pix_image);

    GtkWidget *view1=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view1), drawing_area);
    GtkWidget *scroll1 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll1), view1);
   
    GtkWidget *image=gtk_image_new_from_file(image_file);

    GtkWidget *view2=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view2), image);
    GtkWidget *scroll2 = gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll2), view2);

    GtkWidget *table=gtk_table_new(2, 1, TRUE);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll2, 0, 1, 0, 1);
    gtk_table_attach_defaults(GTK_TABLE(table), scroll1, 0, 1, 1, 2);

    gtk_container_add(GTK_CONTAINER(window), table);
 
    gtk_widget_show_all(window);
    gtk_main();

    cvReleaseImage(&opencv_image);
    g_object_unref(pixbuf);
    return 0;
}
