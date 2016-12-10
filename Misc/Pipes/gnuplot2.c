
/*

    Use gnuplot to output directly to a drawing area widget. This works with the x11 terminal. The 
drawing doesn't look as good as the pngcairo terminal but it is all done in memory.   

    gcc -Wall gnuplot2.c -o gnuplot2 `pkg-config gtk+-3.0 --cflags --libs`

    A listing of gnuplot terminals.
    http://www.bersch.net/gnuplot-doc/complete-list-of-terminals.html

    Tested with Ubuntu14.04 32bit and GTK3.10
    This no longer works on Ubuntu16.04 and GTK3.18

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<stdio.h>

static GRand *rand=NULL;

static gboolean plot_data(gpointer da)
  {
    static gint i=1;
    g_print("Plot %i\n", i++);
    gdouble rand_num=g_rand_double(rand);

    GdkWindow *win=gtk_widget_get_window(GTK_WIDGET(da));
    Window win_id=gdk_x11_window_get_xid(win);
    g_print("WindowID %02x\n", (unsigned int)win_id);
    gchar *hex=g_strdup_printf("%02x", (unsigned int)win_id); 

    FILE *Gnuplot=NULL;
    Gnuplot=popen("gnuplot", "w");     
    if(Gnuplot==NULL) printf("Couldn't Open Gnuplot\n");
    else
       {
         fprintf(Gnuplot, "set terminal x11 window '%s'\n", hex);
         fprintf(Gnuplot,"set xlabel \"sine wave\"\n");
         fprintf(Gnuplot,"set yrange [-1:1]\n");
         fprintf(Gnuplot,"set ylabel \"amplitude\"\n");
         fprintf(Gnuplot,"plot %f*sin(x)\n", rand_num);
         fflush(Gnuplot);
         pclose(Gnuplot);
       }

    g_free(hex);
   
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

    GtkWidget *da = gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 400, 400);
    gtk_widget_set_double_buffered(da, FALSE);
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);

    GtkWidget *view=gtk_viewport_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(view), da);
    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), view);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    //Add a timer to update the drawing.
    g_timeout_add(500, (GSourceFunc)plot_data, da);

    gtk_widget_show_all(window);

    gtk_main();

    g_rand_free(rand);

    return 0;
  }
