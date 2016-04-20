
/*
    Automatically scale a treeview font size and height of cell. To scale the height of the cell,
add an extra column of height values and bind that to the column. The other properties can be set
as usual.
    This is the C version of renderer1.py. A little code comparison.

    Tested with Ubuntu14.04, GTK3.10

    gcc -Wall renderer1.c -o renderer1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static void size_changed(GtkWidget *widget, GdkRectangle *allocation, gpointer *data)
  {
    gint height=gtk_widget_get_allocated_height(widget);

    if(height>200)
      {
        gint scale=height/20;
        g_print("Height %i, Scale %i, Cell Height %i\n", height, scale, scale+20);
        
        GtkTreeIter iter;
        gboolean valid_iter=FALSE;
        valid_iter=gtk_tree_model_get_iter_first(GTK_TREE_MODEL(data[0]), &iter);
        while(valid_iter)
          {
            gtk_list_store_set(data[0], &iter, 1, scale+20, -1);
            valid_iter=gtk_tree_model_iter_next(GTK_TREE_MODEL(data[0]), &iter);
          }
        
        gchar *string=g_strdup_printf("Arial %i", scale);
        g_object_set(data[1], "font", string, "background", "#ffff00", NULL);  
        g_free(string);
      }
    else
      {
        g_object_set(data[1], "font", "Arial 10", "background", "#ff00ff", NULL);
      }

  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Scale Text");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkTreeIter iter;
    GtkListStore *store=gtk_list_store_new(2, G_TYPE_STRING, G_TYPE_INT);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Apples",  1, 30, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Oranges", 1, 30, -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, 0, "Pears", 1, 30, -1);

    GtkWidget *tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    gtk_tree_view_set_grid_lines(GTK_TREE_VIEW(tree), GTK_TREE_VIEW_GRID_LINES_BOTH);
    gtk_widget_set_hexpand(tree, TRUE);
    gtk_widget_set_vexpand(tree, TRUE);

    GtkCellRenderer *renderer=gtk_cell_renderer_text_new();
    g_object_set(renderer, "xalign", 0.5, "editable", FALSE, "background", "#ff00ff", "font", "Arial 10", NULL);
   
    GtkTreeViewColumn *column=gtk_tree_view_column_new_with_attributes("Fruit", renderer, "text", 0, "height", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column);    

    gpointer pointers[]={store, renderer};
    g_signal_connect(window, "size_allocate", G_CALLBACK(size_changed), pointers); 

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tree);  

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
