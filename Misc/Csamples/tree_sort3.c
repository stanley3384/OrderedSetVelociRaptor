
/*
   Test code for sorting data with a treeview and sqlite.
   Extended from the discussion on GTK+ Forum called "how to sort multiple columns in GTK_TREE_SORTABLE"

   gcc -Wall tree_sort3.c -o tree_sort3 `pkg-config --cflags --libs gtk+-3.0` -lsqlite3

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<sqlite3.h>

enum
{
   PROGRAM,
   IMAGE,
   USED,
   COLUMNS
};

//SQLite will automatically create the database with this name.
const gchar *database="treeview_test.db";

static void run_sql(GtkWidget *button, gpointer data[]);
static void save_sqlite_data();
static void get_initial_sqlite_data(const gchar *sql_string, GtkListStore *store);
static void get_sqlite_data(const gchar *sql_string, gpointer data[]);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tree Sort");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 350, 200);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkListStore *store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    //Initial sort order for treeview.
    const gchar *sql_string={"SELECT program, image, used FROM treeview_data ORDER BY used DESC, program ASC;"};
    save_sqlite_data();
    get_initial_sqlite_data(sql_string, store);

    GtkWidget *tree = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(G_OBJECT(store));
    gtk_widget_set_hexpand(tree, TRUE);
    gtk_widget_set_vexpand(tree, TRUE);

    GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
    g_object_set(renderer1, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column1 = gtk_tree_view_column_new_with_attributes("program", renderer1, "text", PROGRAM, NULL);
    gtk_tree_view_column_set_alignment(column1, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column1);

    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("image", renderer1, "text", IMAGE, NULL);
    gtk_tree_view_column_set_alignment(column2, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column2);
   
    GtkTreeViewColumn *column3 = gtk_tree_view_column_new_with_attributes("used", renderer1, "text", USED, NULL);
    gtk_tree_view_column_set_alignment(column3, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column3);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tree);

    GtkWidget *entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "SELECT * FROM treeview_data;");
    gtk_widget_set_hexpand(entry, TRUE);

    GtkWidget *button=gtk_button_new_with_label("Run SQL");
    gpointer data[]={entry, tree};
    g_signal_connect(button, "clicked", G_CALLBACK(run_sql), data);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
static void run_sql(GtkWidget *button, gpointer data[])
  {
    gchar *sql_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(data[0])));
    if(g_utf8_strlen(sql_string, -1)) get_sqlite_data(sql_string, data);
    else g_print("Need a valid SQL string.\n");
  }
static void save_sqlite_data()
  {
    g_print("SQLITE Save\n");
    gint ret_val=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    gchar program1[]="GCC";
    gchar program2[]="Emacs";
    gchar program3[]="Inkscape";
    gchar program4[]="Latex";
    gchar program5[]="Firefox";
    gchar image1[]="image1";
    gchar image2[]="image2";
    gchar image3[]="image3";
    gchar image4[]="image4";
    gchar image5[]="image5";
    gchar sql1[]="INSERT INTO treeview_data(program, image, used) VALUES(?,?,?)";
    
    sqlite3_open(database, &cnn);

    sqlite3_exec(cnn, "DROP TABLE IF EXISTS treeview_data;", 0, 0, 0);
    sqlite3_exec(cnn, "CREATE TABLE IF NOT EXISTS treeview_data (program text, image text, used int);", 0, 0, 0);
    sqlite3_exec(cnn, "BEGIN TRANSACTION;", 0, 0, 0);
    ret_val=sqlite3_prepare_v2(cnn, sql1, -1, &stmt1, 0);

    //Check if statement is good.
    if(ret_val==SQLITE_OK)
      {  
        sqlite3_bind_text(stmt1, 1, program1, g_utf8_strlen(program1, -1), 0);
        sqlite3_bind_text(stmt1, 2, image1, g_utf8_strlen(image1, -1), 0);
        sqlite3_bind_int(stmt1, 3, 7);
        sqlite3_step(stmt1);
        sqlite3_reset(stmt1);
        sqlite3_bind_text(stmt1, 1, program2, g_utf8_strlen(program2, -1), 0);
        sqlite3_bind_text(stmt1, 2, image2, g_utf8_strlen(image2, -1), 0);
        sqlite3_bind_int(stmt1, 3, 7);
        sqlite3_step(stmt1);
        sqlite3_reset(stmt1);
        sqlite3_bind_text(stmt1, 1, program3, g_utf8_strlen(program3, -1), 0);
        sqlite3_bind_text(stmt1, 2, image3, g_utf8_strlen(image3, -1), 0);
        sqlite3_bind_int(stmt1, 3, 5);
        sqlite3_step(stmt1);
        sqlite3_reset(stmt1);
        sqlite3_bind_text(stmt1, 1, program4, g_utf8_strlen(program4, -1), 0);
        sqlite3_bind_text(stmt1, 2, image4, g_utf8_strlen(image4, -1), 0);
        sqlite3_bind_int(stmt1, 3, 18);
        sqlite3_step(stmt1);
        sqlite3_reset(stmt1);
        sqlite3_bind_text(stmt1, 1, program5, g_utf8_strlen(program5, -1), 0);
        sqlite3_bind_text(stmt1, 2, image5, g_utf8_strlen(image5, -1), 0);
        sqlite3_bind_int(stmt1, 3, 18);
        sqlite3_step(stmt1);
        sqlite3_reset(stmt1);
      }
      
    sqlite3_exec(cnn, "END TRANSACTION;", 0, 0, 0);
    if(stmt1!=NULL) sqlite3_finalize(stmt1);
    sqlite3_close(cnn);
  }
static void get_initial_sqlite_data(const gchar *sql_string, GtkListStore *store)
  {
    g_print("SQLITE Read Iniitial\n");
    GtkTreeIter iter;
    gint ret_val=0;
    gboolean valid_string=TRUE;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_open(database, &cnn);

    sqlite3_prepare_v2(cnn, sql_string, -1, &stmt1, 0);
    if(stmt1==NULL) valid_string=FALSE;
    if(valid_string)
      {
        ret_val=sqlite3_step(stmt1);
        //Get the rows.
        while(ret_val==SQLITE_ROW)
          {
            //Print rows to screen.
            g_print("%s %s %i\n", sqlite3_column_text(stmt1, 0), sqlite3_column_text(stmt1, 1), sqlite3_column_int(stmt1, 2));                        
            //Load the data for the default store on startup.
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, PROGRAM, sqlite3_column_text(stmt1, 0), IMAGE, sqlite3_column_text(stmt1, 1), USED, sqlite3_column_int(stmt1, 2),  -1);            
            ret_val=sqlite3_step(stmt1);
          }
      }

    if(stmt1!=NULL) sqlite3_finalize(stmt1);
    sqlite3_close(cnn);  
  }
static void get_sqlite_data(const gchar *sql_string, gpointer data[])
  {
    g_print("SQLITE Read\n");
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
        g_print("Column Names and Types\n");
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
            g_print("%s %i\n", sqlite3_column_name(stmt1, i), column_types[i]);
            g_ptr_array_add(column_names, g_strdup(sqlite3_column_name(stmt1, i)));
          }
        //Reset the list store based on column types.
        if(sql_string!=NULL)
          {
            g_print("List Columns %i\n", columns);
            gtk_list_store_set_column_types(GTK_LIST_STORE(new_store), columns, type_array);
          }
        //Get the rows.
        while(ret_val==SQLITE_ROW)
          {
            //Print rows to screen.
            for(i=0;i<columns;i++)
              {
                switch(column_types[i])
                  {
                    case SQLITE_INTEGER:
                      //SQLITE_INTEGER 1, define G_TYPE_INT G_TYPE_MAKE_FUNDAMENTAL(6)
                      g_print("%i ", sqlite3_column_int(stmt1, i));
                      break;
                    case SQLITE_FLOAT:
                      //SQLITE_FLOAT 2 #define G_TYPE_DOUBLE G_TYPE_MAKE_FUNDAMENTAL(15)
                      g_print("%f ", sqlite3_column_double(stmt1, i));
                      break;
                    case SQLITE_TEXT:
                      //SQLITE_TEXT 3 #define G_TYPE_STRING G_TYPE_MAKE_FUNDAMENTAL(16)
                      g_print("%s ", sqlite3_column_text(stmt1, i));
                      break;
                    case SQLITE_BLOB:
                      //SQLITE_BLOB 4
                      g_print("Blob ");
                      break;
                    case SQLITE_NULL:
                      //SQLITE_NULL 5
                      g_print("NULL ");
                      break;
                  }
              }
            g_print("\n");
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
            gtk_tree_view_append_column(GTK_TREE_VIEW(data[1]), t_column);
          }
      }
    
    g_ptr_array_free(column_names, TRUE);
  }

