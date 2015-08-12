
/*
   Test code for sorting data with a treeview and sqlite. Test with the VelociRaptorData.db
to see if it will work well with the VelociRaptor application.

   gcc -Wall tree_sort4.c -o tree_sort4 `pkg-config --cflags --libs gtk+-3.0` -lsqlite3

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<sqlite3.h>

//SQLite will automatically create the database with this name.
const gchar *database="VelociRaptorData.db";

static void run_sql(GtkWidget *button, gpointer data[]);
static void get_sqlite_data(const gchar *sql_string, gpointer data[]);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "SQL Tree Sort");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *tree = gtk_tree_view_new();
    gtk_widget_set_hexpand(tree, TRUE);
    gtk_widget_set_vexpand(tree, TRUE);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tree);

    GtkWidget *entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "SELECT * FROM data;");
    gtk_widget_set_hexpand(entry, TRUE);

    GtkWidget *button=gtk_button_new_with_label("Run SQL");
    gpointer data[]={entry, tree};
    g_signal_connect(button, "clicked", G_CALLBACK(run_sql), data);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    GError *css_error=NULL;
    gchar css_string[]="GtkButton{background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(0,255,0,0.5)), color-stop(0.5,rgba(180,180,180,0.5)), color-stop(1.0,rgba(25,0,200,0.5)));} GtkTreeView{background:rgba(160,160,160,0.3);} GtkLabel{background:rgba(0,0,0,0.0);}";
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    if(css_error!=NULL) g_print("CSS loader error %s\n", css_error->message);
    g_object_unref(provider);

   
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
static void run_sql(GtkWidget *button, gpointer data[])
  {
    gchar *sql_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(data[0])));
    //Only want SELECT statements for the treeview.
    if(g_utf8_strlen(sql_string, -1)>6&&(sql_string[0]=='S'||sql_string[0]=='s'))
      {
        get_sqlite_data(sql_string, data);
      }
    else g_print("Need a valid SQL string.\n");
  }
static void get_sqlite_data(const gchar *sql_string, gpointer data[])
  {
    GtkTreeIter iter;
    gint ret_val=0;
    gboolean valid_string=TRUE;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_open(database, &cnn);
    gint columns=0;
    gint i=0;
    GPtrArray *column_names=g_ptr_array_new_with_free_func(g_free);
    GtkListStore *new_store=NULL;

    //Prepare the incoming SQL statement.
    sqlite3_prepare_v2(cnn, sql_string, -1, &stmt1, 0);
    columns=sqlite3_column_count(stmt1);
    new_store=gtk_list_store_new(1, G_TYPE_STRING);
    
    if(stmt1==NULL) valid_string=FALSE;

    //Check the statement.
    if(valid_string)
      {
        ret_val=sqlite3_step(stmt1);
        //Check generalized column types. SQLite dynamically typed so this may be trouble for mixed data.
        gint column_types[columns];
        GType type_array[columns];
        for(i=0;i<columns;i++)
          {
            column_types[i]=sqlite3_column_type(stmt1, i);
            switch(column_types[i])
              {
                case 1:
                  type_array[i]=G_TYPE_INT;
                  break;
                case 2:
                  type_array[i]=G_TYPE_DOUBLE;
                  break;
                case 3:
                  type_array[i]=G_TYPE_STRING;
                  break;
                default:
                  g_print("Column Type Error\n");
               }                  
            //g_print("%s %i\n", sqlite3_column_name(stmt1, i), column_types[i]);
            g_ptr_array_add(column_names, g_strdup(sqlite3_column_name(stmt1, i)));
          }
        //Reset the list store based on column types.
        if(sql_string!=NULL)
          {
            //g_print("List Columns %i\n", columns);
            gtk_list_store_set_column_types(GTK_LIST_STORE(new_store), columns, type_array);
          }
        //Get the rows.
        while(ret_val==SQLITE_ROW)
          {            
            //Load the list store with data.
            gtk_list_store_append(new_store, &iter);
            for(i=0;i<columns;i++)
              {
                switch(column_types[i])
                  {
                    case SQLITE_INTEGER:
                      gtk_list_store_set(new_store, &iter, i, sqlite3_column_int(stmt1, i), -1);
                      break;
                    case SQLITE_FLOAT:
                      gtk_list_store_set(new_store, &iter, i, sqlite3_column_double(stmt1, i), -1);
                      break;
                    case SQLITE_TEXT:
                      gtk_list_store_set(new_store, &iter, i, sqlite3_column_text(stmt1, i), -1);
                      break;
                    default:
                      g_print("Column Type Error\n");
                  }
              }
            
            ret_val=sqlite3_step(stmt1);
          }
      }
    else            
      {
        g_print("The SQL statement isn't valid.\n");
      }
    if(stmt1!=NULL) sqlite3_finalize(stmt1);
    sqlite3_close(cnn);

    //Setup new treeview.
    if(valid_string)
      {
        //Set new model.
        gtk_tree_view_set_model(GTK_TREE_VIEW(data[1]), GTK_TREE_MODEL(new_store));
        //Drop old columns. 
        gint n_columns=gtk_tree_view_get_n_columns(GTK_TREE_VIEW(data[1]));
        GtkTreeViewColumn *t_column=NULL;
        for(i=0;i<n_columns;i++)
          {
            t_column=gtk_tree_view_get_column(GTK_TREE_VIEW(data[1]), 0);
            gtk_tree_view_remove_column(GTK_TREE_VIEW(data[1]), t_column);
          }
        //Setup new columns.
        GtkCellRenderer *renderer=gtk_cell_renderer_text_new();
        g_object_set(renderer, "xalign", 0.5, "editable", FALSE, NULL);
        for(i=0;i<columns;i++)
          {
            t_column=gtk_tree_view_column_new_with_attributes(g_ptr_array_index(column_names, i), GTK_CELL_RENDERER(renderer) , "text", i, NULL);
            gtk_tree_view_column_set_alignment(t_column, 0.5);
            gtk_tree_view_column_set_resizable(t_column, TRUE);
            gtk_tree_view_append_column(GTK_TREE_VIEW(data[1]), t_column);
          }
      }
    
    g_ptr_array_free(column_names, TRUE);
  }

