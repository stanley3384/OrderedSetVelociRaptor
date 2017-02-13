
/*
    Use the keyboard delete key to delete from an entry completion pop-up list. This is overstepping
the GTK+ api so results can be undefined or changed at any time by the GTK developers. Not recommended
programming techniques. But... might need the functionality and the internals of the entry completion
probably don't change much. Either way, this is just test code to see if it can be done.

    gcc -Wall entry_completion2.c -o entry_completion2 `pkg-config --cflags --libs gtk+-3.0`

    Tested with Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

//The compiler needs the struct from gtkentryprivate.h.
struct _GtkEntryCompletionPrivate
{
  GtkWidget *entry;

  GtkWidget *tree_view;
  GtkTreeViewColumn *column;
  GtkTreeModelFilter *filter_model;
  GtkListStore *actions;
  GtkCellArea *cell_area;

  GtkEntryCompletionMatchFunc match_func;
  gpointer match_data;
  GDestroyNotify match_notify;

  gint minimum_key_length;
  gint text_column;

  gchar *case_normalized_key;

  /* only used by GtkEntry when attached: */
  GtkWidget *popup_window;
  GtkWidget *vbox;
  GtkWidget *scrolled_window;
  GtkWidget *action_view;

  gulong completion_timeout;
  gulong changed_id;
  gulong insert_text_id;

  gint current_selected;

  guint first_sel_changed : 1;
  guint ignore_enter      : 1;
  guint has_completion    : 1;
  guint inline_completion : 1;
  guint popup_completion  : 1;
  guint popup_set_width   : 1;
  guint popup_single_match : 1;
  guint inline_selection   : 1;
  guint has_grab           : 1;

  gchar *completion_prefix;

  GSource *check_completion_idle;

  GdkDevice *device;
};

static gboolean match_selected(GtkEntryCompletion *completion, GtkTreeModel *tree, GtkTreeIter *iter, gpointer data)
  {
    gchar *string=NULL;
    GtkTreeModel *model=gtk_entry_completion_get_model(GTK_ENTRY_COMPLETION(completion));
    gtk_tree_model_get(model, iter, 0, &string, -1);
    gtk_list_store_remove(GTK_LIST_STORE(model), iter);
    g_print("Remove %s\n", string);
    g_free(string);
    return TRUE; 
  }
static gboolean delete_pressed(GtkWidget *widget, GdkEvent *event, gpointer completion)
  {
    if(((GdkEventKey*)event)->keyval==GDK_KEY_Delete)
      {
        g_print("Delete Key\n");
        GtkTreeIter iter;
        GtkTreeIter child_iter;
        gboolean entry_set=TRUE;

        //Need to use some private data pointers.
        GtkEntryCompletionPrivate *priv=G_TYPE_INSTANCE_GET_PRIVATE((completion), GTK_TYPE_ENTRY_COMPLETION, GtkEntryCompletionPrivate);
        GtkTreeModel *model=gtk_entry_completion_get_model(GTK_ENTRY_COMPLETION(completion));
        GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(priv->tree_view));
        if(gtk_tree_selection_get_selected(selection, &model, &iter))
          {
            gtk_tree_model_filter_convert_iter_to_child_iter(priv->filter_model, &child_iter, &iter);
            GtkTreeModel *child_model=gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model));
            g_signal_handler_block(widget, priv->changed_id);
            g_signal_emit_by_name(completion, "match-selected", child_model, &child_iter, &entry_set);
            g_signal_handler_unblock(widget, priv->changed_id);
          }
      }
    return FALSE;
  }
int main (int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Run");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 50);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //GtkListStore derives directly from GObject. Own the initial reference count at one. Unlike floating references of widgets that are automatically sinked.
    GtkListStore *store=gtk_list_store_new(1, G_TYPE_STRING);

    //Add some test data into a list store.
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
    GtkEntryCompletion *completion=gtk_entry_completion_new();
    gtk_entry_completion_set_model(completion, GTK_TREE_MODEL(store));
    //The entry completion owns the reference to store now and increases the count to 2. Drop reference count by one.
    g_object_unref(G_OBJECT(store));
    gtk_entry_completion_set_text_column(completion, 0);
   
    GtkWidget *entry=gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_completion(GTK_ENTRY(entry), completion);
    //The entry owns the reference to the entry completion now and increases the count to 2. Drop reference count by one.
    g_object_unref(G_OBJECT(completion));

    g_signal_connect(entry, "key-press-event", G_CALLBACK(delete_pressed), completion);
    g_signal_connect(completion, "match-selected", G_CALLBACK(match_selected), entry);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 0, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
