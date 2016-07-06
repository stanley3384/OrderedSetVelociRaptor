
/*

    Another test program for the stepped progress bar. Connect a timer and change the step stop variable.

    gcc -Wall -Werror stepped_progress_bar.c stepped_progress_bars.c -o stepped_progress_bars `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>
#include "stepped_progress_bar.h"

static GRand *rand;

static gboolean time_draw(GtkWidget *widgets[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    rand=g_rand_new();

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Stepped Progress Bars");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
   
    GtkWidget *bar1=stepped_progress_bar_new();
    gtk_widget_set_hexpand(bar1, TRUE);
    gtk_widget_set_vexpand(bar1, TRUE);

    GtkWidget *bar2=stepped_progress_bar_new();
    gtk_widget_set_hexpand(bar2, TRUE);
    gtk_widget_set_vexpand(bar2, TRUE);
    stepped_progress_bar_set_foreground_rgba1(STEPPED_PROGRESS_BAR(bar2), "rgba(255, 0, 0, 255)");
    stepped_progress_bar_set_foreground_rgba2(STEPPED_PROGRESS_BAR(bar2), "rgba(200, 0, 0, 255)");
  
    GtkWidget *bar3=stepped_progress_bar_new();
    gtk_widget_set_hexpand(bar3, TRUE);
    gtk_widget_set_vexpand(bar3, TRUE);
    stepped_progress_bar_set_foreground_rgba1(STEPPED_PROGRESS_BAR(bar3), "rgba(0, 255, 0, 255)");
    stepped_progress_bar_set_foreground_rgba2(STEPPED_PROGRESS_BAR(bar3), "rgba(0, 200, 0, 255)");

    GtkWidget *bar4=stepped_progress_bar_new();
    gtk_widget_set_hexpand(bar4, TRUE);
    gtk_widget_set_vexpand(bar4, TRUE);
    stepped_progress_bar_set_foreground_rgba1(STEPPED_PROGRESS_BAR(bar4), "rgba(255, 0, 255, 255)");
    stepped_progress_bar_set_foreground_rgba2(STEPPED_PROGRESS_BAR(bar4), "rgba(200, 0, 255, 255)");

    GtkWidget *bar5=stepped_progress_bar_new();
    gtk_widget_set_hexpand(bar5, TRUE);
    gtk_widget_set_vexpand(bar5, TRUE);
    stepped_progress_bar_set_foreground_rgba1(STEPPED_PROGRESS_BAR(bar5), "rgba(255, 255, 0, 255)");
    stepped_progress_bar_set_foreground_rgba2(STEPPED_PROGRESS_BAR(bar5), "rgba(255, 200, 0, 255)");
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), bar1, 0, 0, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), bar2, 1, 0, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), bar3, 2, 0, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), bar4, 3, 0, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), bar5, 4, 0, 1, 10);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    //Set up CSS colors for the background window and the button.
    gchar *css_string=NULL;
    gint minor_version=gtk_get_minor_version();
    g_print("Minor Version %i\n", minor_version);    
    if(minor_version>=18) css_string=g_strdup("window{background: #000000;}");
    else css_string=g_strdup("GtkWindow{background: #000000;}");
    GError *css_error=NULL;
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    if(css_error!=NULL)
      {
        g_print("CSS error %s\n", css_error->message);
        g_error_free(css_error);
      }
    g_object_unref(provider);
    if(css_string!=NULL) g_free(css_string);

    GtkWidget *widgets[]={bar1, bar2, bar3, bar4, bar5};    
    g_timeout_add(1000, (GSourceFunc)time_draw, widgets);

    gtk_widget_show_all(window);                  
    gtk_main();

    g_rand_free(rand);

    return 0;
  }
static gboolean time_draw(GtkWidget *widgets[])
  {
    gint i=0;
    gint step_stop=(gint)(20*g_rand_double(rand));
    for(i=0;i<5;i++)
      {
        stepped_progress_bar_set_step_stop(STEPPED_PROGRESS_BAR(widgets[i]), step_stop);
        step_stop=(gint)(20*g_rand_double(rand));
      }
    
    return TRUE;
  }

