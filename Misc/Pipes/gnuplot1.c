
/*

    Use gnuplot to output to a png file and then open that in an image widget. The pngcairo 
terminal doesn't have a window parameter so save the drawing to a file and then open it.

    gcc -Wall gnuplot1.c -o gnuplot1 `pkg-config gtk+-3.0 --cflags --libs`

    A listing of gnuplot terminals.
    http://www.bersch.net/gnuplot-doc/complete-list-of-terminals.html

    Tested with Ubuntu14.04 32bit and GTK3.10
    Also works with Ubuntu16.04 and GTK3.18

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<stdio.h>

//Test directory the program is in. Need a png that gnuplot can write to.
static gchar *test_dir={"/home/eric/Velo/Misc/Pipes/TEST.png"};
static GRand *rand=NULL;

static gboolean plot_data(gpointer image)
  {
    static gint i=1;
    g_print("Plot %i\n", i++);
    gdouble rand_num=g_rand_double(rand);

    gint wid=gtk_widget_get_allocated_width(GTK_WIDGET(image));
    gint hit=gtk_widget_get_allocated_height(GTK_WIDGET(image));

    FILE *Gnuplot=NULL;
    Gnuplot=popen("gnuplot", "w");     
    if(Gnuplot==NULL) printf("Couldn't Open Gnuplot\n");
    else
       {
         fprintf(Gnuplot,"set terminal pngcairo size %d,%d\n",wid,hit);
         fprintf(Gnuplot,"set output '%s'\n", test_dir);
         fprintf(Gnuplot,"set xlabel \"sine wave\"\n");
         fprintf(Gnuplot,"set yrange [-1:1]\n");
         fprintf(Gnuplot,"set ylabel \"amplitude\"\n");
         fprintf(Gnuplot,"plot %f*sin(x)\n", rand_num);
         fflush(Gnuplot);
         pclose(Gnuplot);
       }

    gtk_image_set_from_file(GTK_IMAGE(image), test_dir);
    gtk_widget_queue_draw(GTK_WIDGET(image));
    return TRUE;
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

    GtkWidget *image = gtk_image_new();
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

    //Add a timer to update the drawing.
    g_timeout_add(500, (GSourceFunc)plot_data, image);

    gtk_widget_show_all(window);

    gtk_main();

    g_rand_free(rand);

    return 0;
  }
