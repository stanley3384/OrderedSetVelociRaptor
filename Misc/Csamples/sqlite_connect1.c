
/*
  Test code for getting table and field names from a sqlite database into a treeview. 

  gcc -Wall sqlite_connect1.c -o sqlite_connect1 `pkg-config --cflags --libs gtk+-3.0` -lsqlite3

  C. Eric Cashon. 
*/

#include<gtk/gtk.h>
#include<sqlite3.h>

static void connect_db(GtkWidget *button1, GArray *widgets)
  {
    g_print("Connect DB\n");
    gint sql_return1=0;
    gint sql_return2=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_stmt *stmt2=NULL;
    gchar *sql1="SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
    gchar *database=g_strdup_printf("%s", gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 0))));
    //Save table and field names in a treestore.
    GtkTreeIter iter1, iter2;
    GtkTreeStore *store=gtk_tree_store_new(1, G_TYPE_STRING);

    //Test if file is there.
    if(!g_file_test(database, G_FILE_TEST_EXISTS))
      {
        g_print("Couldn't find file %s\n", database);
      }
    else
      {
        sqlite3_open(database,&cnn);
        sqlite3_prepare_v2(cnn, sql1, -1, &stmt1, 0);
        sql_return1=sqlite3_step(stmt1);
        while(sql_return1==SQLITE_ROW)
         {
           g_print("%s\n", sqlite3_column_text(stmt1, 0));
           gtk_tree_store_append(store, &iter1, NULL); 
           gtk_tree_store_set(store, &iter1, 0, sqlite3_column_text(stmt1, 0), -1);
           //Get the table fields.
           gchar *fields=sqlite3_mprintf("PRAGMA table_info('%q');", sqlite3_column_text(stmt1, 0));
           sqlite3_prepare_v2(cnn,fields,-1,&stmt2,0);
           sql_return2=sqlite3_step(stmt2);
             while(sql_return2==SQLITE_ROW)
               {
                  g_print("  %s\n", sqlite3_column_text(stmt2, 1));
                  gtk_tree_store_append(store, &iter2, &iter1);
                  gtk_tree_store_set(store, &iter2, 0, sqlite3_column_text(stmt2, 1), -1);
                  sql_return2=sqlite3_step(stmt2);
               }
           sqlite3_finalize(stmt2);
           sqlite3_free(fields);

           sql_return1=sqlite3_step(stmt1);
         }
        sqlite3_finalize(stmt1);
        sqlite3_close(cnn);

        //Print values loaded into the treeview store to check they are there.
        g_print("Print Treeview Values\n");
        GtkTreeIter iter3, iter4;
        gboolean check1=FALSE;
        gboolean check2=FALSE;
        check1=gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter3);
        while(check1)
          {
            gchar *string1=NULL;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter3, 0, &string1, -1);
            g_print("%s\n", string1);
            check2=gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &iter4, &iter3);
            while(check2)
              {
                gchar *string2=NULL;
                gtk_tree_model_get(GTK_TREE_MODEL(store), &iter4, 0, &string2, -1);
                g_print("  %s\n", string2);
                check2=gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter4);
                if(string2!=NULL) g_free(string2);
              }
            check1=gtk_tree_model_iter_next(GTK_TREE_MODEL(store), &iter3);
            if(string1!=NULL) g_free(string1);
          }
        //Attach the store data to the treeview.
        gtk_tree_view_set_model(GTK_TREE_VIEW(g_array_index(widgets, GtkWidget*, 1)), GTK_TREE_MODEL(store));  
      }
     
    g_object_unref(G_OBJECT(store));
    if(database!=NULL) g_free(database);
  }
static void get_treeview_selected(GtkWidget *button1, GArray *widgets)
  {
    g_print("Selected Table and Field\n");
    gchar *table=NULL;
    gchar *field=NULL;
    gboolean check1=FALSE;
    gboolean check2=FALSE;
    GtkTreeIter parent, child;
    GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(g_array_index(widgets, GtkWidget*, 1)));
    GtkTreeModel *model=gtk_tree_view_get_model(GTK_TREE_VIEW(g_array_index(widgets, GtkWidget*, 1)));

    check1=gtk_tree_selection_get_selected(selection, NULL, &child);
    if(check1) gtk_tree_model_get(model, &child, 0, &field, -1);
    if(check1) check2=gtk_tree_model_iter_parent(model, &parent, &child);
    if(check1&&check2)
      {
        gtk_tree_model_get(model, &parent, 0, &table, -1);
        g_print("%s %s\n", table, field);
      }
    else
      {
        g_print("Connect and then select a table and a field.\n");
      }
    if(table!=NULL) g_free(table);
    if(field!=NULL) g_free(field);
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    printf("Quit Program\n");
    gtk_main_quit();
  }
int main(int argc, char *argv[])
  {
    GtkWidget *window, *entry1, *button1, *button2, *treeview1, *grid;

    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), "Sqlite Connect");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    entry1=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry1), "VelociRaptorData.db");
    gtk_widget_set_hexpand(entry1, TRUE);

    button1=gtk_button_new_with_label("Connect");

    button2=gtk_button_new_with_label("Print Selected");

    //A treeview for tables and fields.
    treeview1=gtk_tree_view_new();
    GtkCellRenderer *renderer1=gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer1),"foreground", "purple", NULL);
    GtkTreeViewColumn *column1=gtk_tree_view_column_new_with_attributes("Tables and Fields", renderer1, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), column1);

    //Pass entry and treeview to callback.
    GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
    g_array_append_val(widgets, entry1);
    g_array_append_val(widgets, treeview1);
    g_signal_connect(button1, "clicked", G_CALLBACK(connect_db), widgets);
    g_signal_connect(button2, "clicked", G_CALLBACK(get_treeview_selected), widgets);

    grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_attach(GTK_GRID(grid), entry1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), treeview1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 3, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
