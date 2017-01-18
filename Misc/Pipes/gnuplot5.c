
/*

    Use gnuplot to output to a file and then open the file in an image widget. This uses
GSubprocess to start gnuplot. When gnuplot is finished it signals that it is done and a new image
is opened in the image widget.

    gcc -Wall gnuplot5.c -o gnuplot5 `pkg-config gtk+-3.0 --cflags --libs`

    A listing of gnuplot terminals.
    http://www.bersch.net/gnuplot-doc/complete-list-of-terminals.html

    Tested with Ubuntu16.04 32bit and GTK3.18

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<string.h>

static GRand *rand=NULL;
//Where to write the image file to.
static gchar *test_dir={"/home/eric/Velo/Misc/Pipes/TEST.png"};

static void gnuplot_finished(GObject *source_object, GAsyncResult *res, GtkWidget *image);
static void plot_data(GtkWidget *image);

static void gnuplot_finished(GObject *source_object, GAsyncResult *res, GtkWidget *image)
  {   
    gtk_image_set_from_file(GTK_IMAGE(image), test_dir);
    gtk_widget_queue_draw(image);
    if(g_subprocess_get_successful(G_SUBPROCESS(source_object))) g_print("Gnuplot Success\n");
    g_object_unref(source_object);
    //Plot the image again.
    plot_data(image);
  }
static void plot_data(GtkWidget *image)
  {
    static gint i=1;
    g_print("\nPlot %i\n", i++);
    gdouble rand_num=g_rand_double(rand);
    gint width=gtk_widget_get_allocated_width(GTK_WIDGET(image));
    gint height=gtk_widget_get_allocated_height(GTK_WIDGET(image));
    
    gchar *cmd=g_strdup("/usr/bin/gnuplot");
    gchar *script=g_strdup_printf("set terminal pngcairo size %d,%d\nset output '%s'\nset xlabel \"sine wave\"\nset yrange [-1:1]\nset ylabel \"amplitude\"\nplot %f*sin(x)", width, height, test_dir, rand_num);
    g_print("%s\n", script);
    GError *error1=NULL;
    GError *error2=NULL;
    GError *error3=NULL;
    GError *error4=NULL;

    GSubprocess *process=g_subprocess_new(G_SUBPROCESS_FLAGS_STDIN_PIPE, &error1, cmd, NULL);
    GOutputStream *stream=g_subprocess_get_stdin_pipe(process);
    g_output_stream_write(stream, script, strlen(script)+1, NULL, &error2);
    g_output_stream_flush(stream, NULL, &error3);
    g_output_stream_close(stream, NULL, &error4); 
    
    if(error1!=NULL)
      {
        g_print("Error1: %s\n", error1->message);
        g_error_free(error1);
      }
    else if(error2!=NULL)
      {
        g_print("Error2: %s\n", error2->message);
        g_error_free(error2);
      }
    else if(error3!=NULL)
      {
        g_print("Error3: %s\n", error3->message);
        g_error_free(error3);
      }
    else if(error4!=NULL)
      {
        g_print("Error4: %s\n", error4->message);
        g_error_free(error4);
      }
    else
      {
        //Want to know when gnuplot finishes.
        g_subprocess_wait_async(process, NULL, (GAsyncReadyCallback)gnuplot_finished, image); 
      }

    g_free(cmd);  
    g_free(script);   
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gnuplot GTK");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(gtk_main_quit), NULL);

    rand=g_rand_new();

    GtkWidget *image=gtk_image_new();
    gtk_widget_set_size_request(image, 400, 400);
    gtk_widget_set_hexpand(image, TRUE);
    gtk_widget_set_vexpand(image, TRUE);

    GtkWidget *view=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view), image);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    //Loop the plotting. When one plot is done and opened in the image widget, start another plot.
    plot_data(image);

    gtk_main();

    g_rand_free(rand);

    return 0;
  }
