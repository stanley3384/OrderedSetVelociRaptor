
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

static void column1_clicked(GtkTreeViewColumn *treeviewcolumn, gpointer data);
static void run_sql(GtkWidget *button, gpointer data[]);
static void save_sqlite_data();
static void get_sqlite_data(const gchar *sql_string, gpointer data[]);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tree Sort");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkListStore *store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    gpointer data1[]={NULL, store};
    save_sqlite_data();
    get_sqlite_data(NULL, data1);

    GtkTreeModel *sortmodel=gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));

    GtkWidget *tree = gtk_tree_view_new_with_model(sortmodel);
    g_object_unref(G_OBJECT(store));
    gtk_widget_set_hexpand(tree, TRUE);
    gtk_widget_set_vexpand(tree, TRUE);

    GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
    g_object_set(renderer1, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column1 = gtk_tree_view_column_new_with_attributes("program", renderer1, "text", PROGRAM, NULL);
    gtk_tree_view_column_set_sort_column_id(column1, PROGRAM);
    g_signal_connect(column1, "clicked", G_CALLBACK(column1_clicked), NULL);
    gtk_tree_view_column_set_alignment(column1, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column1);

    GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
    g_object_set(renderer2, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("image", renderer2, "text", IMAGE, NULL);
    gtk_tree_view_column_set_alignment(column2, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column2);
   
    GtkCellRenderer *renderer3 = gtk_cell_renderer_text_new();
    g_object_set(renderer3, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column3 = gtk_tree_view_column_new_with_attributes("used", renderer3, "text", USED, NULL);
    gtk_tree_view_column_set_sort_column_id(column3, USED);
    gtk_tree_view_column_set_alignment(column3, 0.5);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column3);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), tree);

    GtkWidget *entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "SELECT * FROM treeview_data;");
    gtk_widget_set_hexpand(entry, TRUE);

    GtkWidget *button=gtk_button_new_with_label("Run SQL");
    gpointer data2[]={entry, store, column1, column2, column3, tree};
    g_signal_connect(button, "clicked", G_CALLBACK(run_sql), data2);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
static void column1_clicked(GtkTreeViewColumn *treeviewcolumn, gpointer data)
  {
    g_print("Column1 Clicked\n");
  }
static void run_sql(GtkWidget *button, gpointer data[])
  {
    gchar *sql_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(data[0])));
    gtk_list_store_clear(GTK_LIST_STORE(data[1]));
    get_sqlite_data(sql_string, data);
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

    //Sort the data as needed with SQL.
    if(sql_string!=NULL)
      { 
        sqlite3_prepare_v2(cnn, sql_string, -1, &stmt1, 0);
        columns=sqlite3_column_count(stmt1);
      }
    else
      {
        sqlite3_prepare_v2(cnn, "SELECT program, image, used FROM treeview_data ORDER BY used DESC, program ASC;", -1, &stmt1, 0);
        columns=sqlite3_column_count(stmt1);
      }
    if(stmt1==NULL) valid_string=FALSE;

    //Another way to check the statement.
    if(valid_string)
      {
        ret_val=sqlite3_step(stmt1);
        //Check generalized column types.
        gint column_types[columns+1];
        g_print("Column Names and Types\n");
        for(i=0;i<columns;i++)
          {
            column_types[i]=sqlite3_column_type(stmt1, i);
            g_print("%s %i\n", sqlite3_column_name(stmt1, i), column_types[i]);
            g_ptr_array_add(column_names, g_strdup(sqlite3_column_name(stmt1, i)));
          }
        //Get the rows.
        while(ret_val==SQLITE_ROW)
          {
            for(i=0;i<columns;i++)
              {
                switch(column_types[i])
                  {
                    case 1:
                      //SQLITE_INTEGER 1
                      g_print("%i ", sqlite3_column_int(stmt1, i));
                      break;
                    case 2:
                      //SQLITE_FLOAT 2
                      g_print("%f ", sqlite3_column_double(stmt1, i));
                      break;
                    case 3:
                      //SQLITE_TEXT 3
                      g_print("%s ", sqlite3_column_text(stmt1, i));
                      break;
                    case 4:
                      //SQLITE_BLOB 4
                      g_print("Blob ");
                      break;
                    case 5:
                      //SQLITE_NULL 5
                      g_print("NULL ");
                      break;
                  }
              }
            g_print("\n");
            //Load the data into the store.
            gtk_list_store_append(data[1], &iter);
            for(i=0;i<columns;i++)
              {
                if(g_strcmp0(g_ptr_array_index(column_names, i), "program")==0)
                  {
                    gtk_list_store_set(data[1], &iter, PROGRAM, sqlite3_column_text(stmt1, i), -1);
                  }
                else if(g_strcmp0(g_ptr_array_index(column_names, i), "image")==0)
                  {
                    gtk_list_store_set(data[1], &iter, IMAGE, sqlite3_column_text(stmt1, i), -1);
                  }
                else if(g_strcmp0(g_ptr_array_index(column_names, i), "used")==0)
                  {
                    gtk_list_store_set(data[1], &iter, USED, sqlite3_column_int(stmt1, i), -1);
                  }
                else
                  {
                    g_print("List Store Error\n");
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
    
    //Reset columns locations.
    if(sql_string!=NULL)
      {
        gint n_columns=gtk_tree_view_get_n_columns(GTK_TREE_VIEW(data[5]));
        GtkTreeViewColumn *tree_column=NULL;
        for(i=0;i<n_columns;i++)
          {
            tree_column=gtk_tree_view_get_column(GTK_TREE_VIEW(data[5]), 0);
            g_object_ref(tree_column);
            gtk_tree_view_remove_column(GTK_TREE_VIEW(data[5]), tree_column);
          }
        for(i=0;i<columns;i++)
          {
            if(g_strcmp0(g_ptr_array_index(column_names, i), "program")==0)
              {
                gtk_tree_view_append_column(GTK_TREE_VIEW(data[5]), GTK_TREE_VIEW_COLUMN(data[2]));
              }
            if(g_strcmp0(g_ptr_array_index(column_names, i), "image")==0)
              {
                gtk_tree_view_append_column(GTK_TREE_VIEW(data[5]), GTK_TREE_VIEW_COLUMN(data[3]));
              }
            if(g_strcmp0(g_ptr_array_index(column_names, i), "used")==0)
              {
                gtk_tree_view_append_column(GTK_TREE_VIEW(data[5]), GTK_TREE_VIEW_COLUMN(data[4]));
              }
          }
      }

    g_ptr_array_free(column_names, TRUE);
  }
