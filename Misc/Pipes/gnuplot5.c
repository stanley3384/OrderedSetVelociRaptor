
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

static gboolean read_err(GObject *pollable_stream, gpointer user_data);
static void gnuplot_finished(GObject *source_object, GAsyncResult *res, GtkWidget *image);
static void plot_data(GtkWidget *image);

static gboolean read_err(GObject *pollable_stream, gpointer user_data)
  {
    g_print("Read Error\n");
    char *buffer=g_malloc(500);
    memset(buffer, '\0', 500);
    gssize bytes_read=0;

    bytes_read=g_pollable_input_stream_read_nonblocking((GPollableInputStream*)pollable_stream, buffer, 499, NULL, NULL);
    if(bytes_read>0) g_print("Error From Gnuplot: %s", buffer);
    g_input_stream_close((GInputStream*)pollable_stream, NULL, NULL);
    g_free(buffer);

    return FALSE;
  }
static void gnuplot_finished(GObject *source_object, GAsyncResult *res, GtkWidget *image)
  {   
    gtk_image_set_from_file(GTK_IMAGE(image), test_dir);
    gtk_widget_queue_draw(image);
    if(g_subprocess_get_successful(G_SUBPROCESS(source_object))) g_print("Gnuplot Success\n");
    g_object_unref(source_object);
    g_print("Gnuplot Finished\n");
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

    //The Gnuplot process.
    GSubprocess *sub_process=g_subprocess_new(G_SUBPROCESS_FLAGS_STDIN_PIPE|G_SUBPROCESS_FLAGS_STDERR_PIPE, NULL, cmd, NULL);
    //Set up the stderr pipe and callback.
    GInputStream *err_stream=g_subprocess_get_stderr_pipe(sub_process);
    GSource *source=g_pollable_input_stream_create_source((GPollableInputStream*)err_stream, NULL);
    g_source_attach(source, NULL);
    g_source_set_callback(source, (GSourceFunc)read_err, NULL, NULL);
    //Set up finished callback.
    g_subprocess_wait_async(sub_process, NULL, (GAsyncReadyCallback)gnuplot_finished, image); 
    //Set up the stdin pipe and send script to gnuplot.
    GOutputStream *stream=g_subprocess_get_stdin_pipe(sub_process);
    g_output_stream_write(stream, script, strlen(script)+1, NULL, NULL);
    g_output_stream_flush(stream, NULL, NULL);
    g_output_stream_close(stream, NULL, NULL);    
    
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
