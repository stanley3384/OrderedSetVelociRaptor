
/*

    Test code for comparing lazy loading and threads with a long running function and
a progress bar. 

    gcc -Wall progress2.c -o progress2 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>

//Set some loops for work to be done. Using a netbook with a Atom N270 cpu. Might need to
//change these for a faster cpu but be careful of guint overflow.
#define SET1 1000000000
#define SET2 100000000
//Number of times the lazy load functions get called. 10 times 100 million in this case.
#define CALLS 10

//ID's for timer and lazy load function.
static guint idle_id=0;
static guint timeout_id=0;
//The thread_active variable is used for both lazy_load_items and thread_load_items functions.
//It tells the timer used for the progress bar when to stop. 
static gint thread_active=0;
//Test string and int shared by threads.
static GString *c_time1=NULL;
static gint load_progress=0;
static GMutex mutex1;
//Worker thread for long running function.
static GThread *thread=NULL;

static gint thread_load_items(gpointer data);
static gboolean lazy_load_items(gpointer data);
static gboolean check_progress(gpointer data[]);
static void show_progress(GtkWidget *widget, gpointer data[]);
static void close_program(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    g_mutex_init(&mutex1);
    c_time1=g_string_new(NULL);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Show Progress");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    GtkWidget *progress=gtk_progress_bar_new();
    gtk_widget_set_hexpand(progress, TRUE);
    gtk_widget_set_vexpand(progress, TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);

    GtkWidget *radio1=gtk_radio_button_new_with_label(NULL, "Lazy Load");
    GtkWidget *radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Thread");

    GtkWidget *label1= gtk_label_new("Check and Timer String");
    gtk_widget_set_hexpand(label1, TRUE);

    GtkWidget *button=gtk_button_new_with_label("Show Progress");
    gtk_widget_set_hexpand(button, TRUE);

    gpointer widgets[]={progress, radio1, radio2, label1, button};
    g_signal_connect(button, "clicked", G_CALLBACK(show_progress), widgets);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), progress, 0, 4, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);
    gtk_widget_hide(progress);
    gtk_main();
    return 0;   
  }
static int thread_load_items(gpointer data)
  {
    guint32 i=0;
    gdouble percent=0;    
    
    for(i=0;i<SET1;i++)
      {
        if(i%SET2==0)
          {
            g_print("progress %f\n", (gdouble)i/SET1);
            percent=(gdouble)(i+1)/SET1;
            g_atomic_int_set(&load_progress, (gint)(percent*100));
            g_mutex_lock(&mutex1);
            g_string_printf(c_time1, "Thread Time Update %lld", (long long)(g_get_monotonic_time()/1000000));
            g_mutex_unlock(&mutex1); 
          }
      }
    
    g_print("Items Loaded\n");
    g_atomic_int_set(&thread_active, 0); 
    
    return 0;    
  }
static gboolean lazy_load_items(gpointer data)
  {
    guint32 i=0;
    guint32 top=0;
    gdouble percent=0;    
    static guint32 counter=0;

    top=SET2*counter+SET2;

    //Don't need a mutex here because lazy loading is on a single thread. Put in for testing.
    g_mutex_lock(&mutex1);
    g_string_printf(c_time1, "Lazy Time Update %lld", (long long)(g_get_monotonic_time()/1000000));
    g_mutex_unlock(&mutex1); 

    for(i=SET2*counter;i<top;i++)
      {       
        //Try to match work done in threaded code.
        if(i%SET2==0)
          {
          }
      }

    g_print("progress %f\n", (gdouble)i/SET1);
    percent=(gdouble)(i+1)/SET1;
    g_atomic_int_set(&load_progress, (gint)(percent*100));
    counter++;  
    if(counter==CALLS)
      {
        g_atomic_int_set(&thread_active, 0);
        counter=0;
        g_print("Items Loaded\n");  
        return FALSE;
      }
    else return TRUE;    
  }
static gboolean check_progress(gpointer data[])
  {
    g_print("Check Progress\n");
    static guint32 counter=1;
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data[0]), (gdouble)(g_atomic_int_get(&load_progress)/100.0));
    gchar *string1=g_strdup_printf("progress %i%%", g_atomic_int_get(&load_progress));
    gtk_progress_bar_set_text(GTK_PROGRESS_BAR(data[0]), string1);
    g_free(string1);

    g_mutex_lock(&mutex1);
    gchar *string2=g_strdup_printf("Check %i, %s", counter, c_time1->str);
    g_mutex_unlock(&mutex1);
    gtk_label_set_text(GTK_LABEL(data[3]), string2);
    g_free(string2);
  
    if(g_atomic_int_get(&thread_active)==1)
      {
        counter++;
        return TRUE;
      }
    else
      {
        counter=1;
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(data[0]), 0);
        gchar *string3=g_strdup_printf("progress %i%%", 0);
        gtk_progress_bar_set_text(GTK_PROGRESS_BAR(data[0]), string3);
        g_free(string3);
        gtk_widget_hide(GTK_WIDGET(data[0]));
        g_atomic_int_set(&load_progress, 0);
        if(thread!=NULL)
          { 
            //Make sure the thread returned. Also freed.
            g_print("Thread Return %i\n", (int)g_thread_join(thread));
            thread=NULL;
          }
        timeout_id=0;
        idle_id=0;
        gtk_widget_set_sensitive(GTK_WIDGET(data[4]), TRUE);
        return FALSE;
      }
  }
static void show_progress(GtkWidget *widget, gpointer data[])
  {  
    gtk_widget_set_sensitive(widget, FALSE);
    gtk_widget_show(GTK_WIDGET(data[0])); 
    g_atomic_int_set(&thread_active, 1);

    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(data[1])))
      {
        idle_id=g_idle_add_full(G_PRIORITY_DEFAULT_IDLE, lazy_load_items, data[0], NULL);
      }
    else
      {
        thread=g_thread_new("TestThread", (GThreadFunc)thread_load_items, data[0]);
      }
   
    timeout_id=g_timeout_add(500, (GSourceFunc)check_progress, data);     
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    //Remove sources if running to prevent errors on exit.
    if(idle_id) g_source_remove(idle_id);
    if(timeout_id) g_source_remove(timeout_id);
    g_mutex_clear(&mutex1);
    gtk_main_quit();
  }

