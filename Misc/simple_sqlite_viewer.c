
/*
   Test code for sorting data with a treeview and sqlite. Test with the VelociRaptorData.db
to see if it will work well with the VelociRaptor application.

   gcc -Wall simple_sqlite_viewer.c -o simple_sqlite_viewer `pkg-config --cflags --libs gtk+-3.0` -lsqlite3

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<sqlite3.h>

//The database to query.
static GString *database=NULL;
static gboolean valid_database=FALSE;

static void run_sql(GtkWidget *button, gpointer data[]);
static void get_sqlite_data(const gchar *sql_string, gpointer data[]);
static void close_program(GtkWidget *widget, gpointer data);
static void connect_db(GtkWidget *button1, GArray *widgets);
static void error_message(const gchar *string, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    database=g_string_new("VelociRaptorData.db");

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Simple SQLite Viewer");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 500, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    //Setup for the left pane.
    GtkWidget *l_entry1=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(l_entry1), "VelociRaptorData.db");
    gtk_widget_set_hexpand(l_entry1, TRUE);

    GtkWidget *l_button1=gtk_button_new_with_label("Connect");

    //A treeview for tables and fields.
    GtkWidget *l_tree=gtk_tree_view_new();
    gtk_widget_set_hexpand(l_tree, TRUE);
    gtk_widget_set_vexpand(l_tree, TRUE);
    GtkCellRenderer *l_renderer1=gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(l_renderer1),"foreground", "blue", NULL);
    GtkTreeViewColumn *l_column1=gtk_tree_view_column_new_with_attributes("Tables and Fields", l_renderer1, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(l_tree), l_column1);

    GtkWidget *l_scrolled_win=gtk_scrolled_window_new(NULL, NULL);    
    gtk_container_add(GTK_CONTAINER(l_scrolled_win), l_tree);

    //Pass entry and treeview to callback.
    GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
    g_array_append_val(widgets, l_entry1);
    g_array_append_val(widgets, l_tree);
    g_array_append_val(widgets, window);
    g_signal_connect(l_button1, "clicked", G_CALLBACK(connect_db), widgets);

    GtkWidget *l_grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(l_grid), 5);
    gtk_grid_attach(GTK_GRID(l_grid), l_entry1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(l_grid), l_button1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(l_grid), l_scrolled_win, 0, 2, 1, 1);

    //Setup for right pane.
    GtkWidget *r_tree = gtk_tree_view_new();
    gtk_widget_set_hexpand(r_tree, TRUE);
    gtk_widget_set_vexpand(r_tree, TRUE);

    GtkWidget *r_scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(r_scroll), r_tree);

    GtkWidget *r_entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(r_entry), "SELECT * FROM data;");
    gtk_widget_set_hexpand(r_entry, TRUE);

    GtkWidget *r_button=gtk_button_new_with_label("Run SQL");
    gpointer data[]={r_entry, r_tree, l_tree, window};
    g_signal_connect(r_button, "clicked", G_CALLBACK(run_sql), data);

    GtkWidget *r_grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(r_grid), r_scroll, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(r_grid), r_entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(r_grid), r_button, 0, 2, 1, 1);

    //The pane widget.
    GtkWidget *pane=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add1(GTK_PANED(pane), l_grid);
    gtk_paned_add2(GTK_PANED(pane), r_grid);

    gtk_container_add(GTK_CONTAINER(window), pane);

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
    if(valid_database)
      {
        //Only want SELECT statements for the treeview.
        if(g_utf8_strlen(sql_string, -1)>6&&(sql_string[0]=='S'||sql_string[0]=='s'))
          {
            get_sqlite_data(sql_string, data);
          }
        else
          {
            g_print("Need a valid SQL string.\n");
            error_message("Need a valid SQL string.", data[3]);
          }
      }
    else
      {
        g_print("Connect to a valid SQLite database.\n");
        error_message("Connect to a valid SQLite database.", data[3]);
      }
  }
static void get_sqlite_data(const gchar *sql_string, gpointer data[])
  {
    GtkTreeIter iter;
    gint ret_val=0;
    gboolean valid_string=TRUE;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_open(database->str, &cnn);
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
        error_message("The SQL statement isn't valid.", data[3]);
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
static void close_program(GtkWidget *widget, gpointer data)
  {
    printf("Quit Program\n");
    gtk_main_quit();
  }
static void connect_db(GtkWidget *button1, GArray *widgets)
  {
    g_print("Connect DB\n");
    gint sql_return1=0;
    gint sql_return2=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_stmt *stmt2=NULL;
    gchar *sql1="SELECT name FROM sqlite_master WHERE type='table' ORDER BY name;";
    gchar *try_database=g_strdup_printf("%s", gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 0))));
    //Save table and field names in a treestore.
    GtkTreeIter iter1, iter2;
    GtkTreeStore *store=gtk_tree_store_new(1, G_TYPE_STRING);

    //Test if file is there.
    if(!g_file_test(try_database, G_FILE_TEST_EXISTS))
      {
        g_print("Couldn't find file %s\n", try_database);
        gchar *message=g_strdup_printf("Couldn't find file %s.", try_database);
        error_message(message, g_array_index(widgets, GtkWidget*, 2));
        g_free(message);
        valid_database=FALSE;
        //Set empty store.
        gtk_tree_view_set_model(GTK_TREE_VIEW(g_array_index(widgets, GtkWidget*, 1)), GTK_TREE_MODEL(store)); 
      }
    else
      {
        g_string_assign(database, try_database);
        valid_database=TRUE;
        sqlite3_open(try_database,&cnn);
        sqlite3_prepare_v2(cnn, sql1, -1, &stmt1, 0);
        sql_return1=sqlite3_step(stmt1);
        while(sql_return1==SQLITE_ROW)
         {
           //g_print("%s\n", sqlite3_column_text(stmt1, 0));
           gtk_tree_store_append(store, &iter1, NULL); 
           gtk_tree_store_set(store, &iter1, 0, sqlite3_column_text(stmt1, 0), -1);
           //Get the table fields.
           gchar *fields=sqlite3_mprintf("PRAGMA table_info('%q');", sqlite3_column_text(stmt1, 0));
           sqlite3_prepare_v2(cnn,fields,-1,&stmt2,0);
           sql_return2=sqlite3_step(stmt2);
             while(sql_return2==SQLITE_ROW)
               {
                  //g_print("  %s\n", sqlite3_column_text(stmt2, 1));
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
        //g_print("Print Treeview Values\n");
        GtkTreeIter iter3, iter4;
        gboolean check1=FALSE;
        gboolean check2=FALSE;
        check1=gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store), &iter3);
        while(check1)
          {
            gchar *string1=NULL;
            gtk_tree_model_get(GTK_TREE_MODEL(store), &iter3, 0, &string1, -1);
            //g_print("%s\n", string1);
            check2=gtk_tree_model_iter_children(GTK_TREE_MODEL(store), &iter4, &iter3);
            while(check2)
              {
                gchar *string2=NULL;
                gtk_tree_model_get(GTK_TREE_MODEL(store), &iter4, 0, &string2, -1);
                //g_print("  %s\n", string2);
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
    if(try_database!=NULL) g_free(try_database);
  }
static void error_message(const gchar *string, gpointer data)
  {
    GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(data), GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", string);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
