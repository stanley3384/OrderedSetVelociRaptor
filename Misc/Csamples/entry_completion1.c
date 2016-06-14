
/*

    Test an entry completion with GTK+. The reference counting is a little bit tricky here. There 
is a good explanation of reference counting in GTK+ at the GTK Forum(Gtk memory management in a nutshell).

    http://www.gtkforums.com/viewtopic.php?f=3&t=2412

    The program gets the program file names from /usr/bin and loads them into a drop down entry
completion list. It doesn't try to run the selected program.

    gcc -Wall entry_completion1.c -o entry_completion1 `pkg-config --cflags --libs gtk+-3.0`

    Tested with Ubuntu14.04 and GTK3.10

    C. Eric Cashon

*/

#include<gtk/gtk.h>

static void entry_activated(GtkEntry *entry, gpointer data)
  {
    gchar *string=NULL;
    string=g_strdup_printf("Run: %s", gtk_entry_get_text(entry));
    gtk_label_set_text(GTK_LABEL(data), string);
    g_free(string); 
  }
static gboolean match_selected(GtkEntryCompletion *comp, GtkTreeModel *tree, GtkTreeIter *iter, gpointer data)
  {
    gchar *string=NULL;
    GValue value=G_VALUE_INIT;
    gtk_tree_model_get_value(tree, iter, 0, &value);
    string=g_strdup_printf("Run: %s", g_value_get_string(&value));
    gtk_label_set_text(GTK_LABEL(data), string);
    g_value_unset(&value);
    g_free(string);
    return FALSE; 
  }
int main (int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Run");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 50);
    //Move the window to the upper left corner.
    gtk_window_move(GTK_WINDOW(window), 0, 0);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //GtkListStore derives directly from GObject. Own the initial reference count at one. Unlike floating references of widgets that are automatically sinked.
    GtkListStore *store=gtk_list_store_new(1, G_TYPE_STRING);

    GtkTreeIter iter;
    const gchar *dir_path=NULL;
    GDir *dir=g_dir_open("/usr/bin", 0, NULL);
    dir_path=g_dir_read_name(dir);
    while(dir_path!=NULL)
      {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0, dir_path, -1);
        dir_path=g_dir_read_name(dir);
      }
    g_dir_close(dir);

    //GtkEntryCompletion derives directly from GObject. Own the initial reference count at one.
    GtkEntryCompletion *comp=gtk_entry_completion_new();
    gtk_entry_completion_set_model(comp, GTK_TREE_MODEL(store));
    //The entry completion owns the reference to store now and increases the count to 2. Drop reference count by one.
    g_object_unref(G_OBJECT(store));
    gtk_entry_completion_set_text_column(comp, 0);

    GtkWidget *entry=gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_completion(GTK_ENTRY(entry), comp);
    //The entry owns the reference to the entry completion now and increases the count to 2. Drop reference count by one.
    g_object_unref(G_OBJECT(comp));

    GtkWidget *label=gtk_label_new("Enter Program Name");
    gtk_widget_set_hexpand(label, TRUE);

    g_signal_connect(entry, "activate", G_CALLBACK(entry_activated), label);
    g_signal_connect(comp, "match-selected", G_CALLBACK(match_selected), label);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 1, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
