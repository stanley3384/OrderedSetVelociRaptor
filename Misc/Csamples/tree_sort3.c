
/*
   Test code for sorting data with a treeview and sqlite.

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
static void get_sqlite_data(GtkListStore *store, const gchar *sql_string);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tree Sort");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkListStore *store = gtk_list_store_new(COLUMNS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_UINT);
    save_sqlite_data();
    get_sqlite_data(store, NULL);

    GtkTreeModel *sortmodel=gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));

    GtkWidget *tree = gtk_tree_view_new_with_model(sortmodel);
    g_object_unref(G_OBJECT(store));
    gtk_widget_set_hexpand(tree, TRUE);
    gtk_widget_set_vexpand(tree, TRUE);

    GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
    g_object_set(renderer1, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column1 = gtk_tree_view_column_new_with_attributes("Program", renderer1, "text", PROGRAM, NULL);
    gtk_tree_view_column_set_sort_column_id(column1, PROGRAM);
    g_signal_connect(column1, "clicked", G_CALLBACK(column1_clicked), NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column1);

    GtkCellRenderer *renderer2 = gtk_cell_renderer_text_new();
    g_object_set(renderer2, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("Image", renderer2, "text", IMAGE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column2);
   
    GtkCellRenderer *renderer3 = gtk_cell_renderer_text_new();
    g_object_set(renderer3, "xalign", 0.5, "editable", FALSE, NULL);
    GtkTreeViewColumn *column3 = gtk_tree_view_column_new_with_attributes("Used", renderer3, "text", USED, NULL);
    gtk_tree_view_column_set_sort_column_id(column3, USED);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column3);

    GtkWidget *entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "SELECT * FROM treeview_data;");
    gtk_widget_set_hexpand(entry, TRUE);

    GtkWidget *button=gtk_button_new_with_label("Run SQL");
    gpointer data[]={entry, store};
    g_signal_connect(button, "clicked", G_CALLBACK(run_sql), data);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), tree, 0, 0, 1, 1);
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
    get_sqlite_data(data[1], sql_string);
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
static void get_sqlite_data(GtkListStore *store, const gchar *sql_string)
  {
    g_print("SQLITE Read\n");
    GtkTreeIter iter;
    gint ret_val=0;
    gboolean valid_string=TRUE;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_open(database, &cnn);
    //Sort the data as needed.
    if(sql_string!=NULL)
      { 
        sqlite3_prepare_v2(cnn, sql_string, -1, &stmt1, 0);
      }
    else
      {
        sqlite3_prepare_v2(cnn, "SELECT program, image, used FROM treeview_data ORDER BY used DESC, program ASC;", -1, &stmt1, 0);
      }
    if(stmt1==NULL) valid_string=FALSE;

    //Another way to check the statement.
    if(valid_string)
      {
        ret_val=sqlite3_step(stmt1);
        while(ret_val==SQLITE_ROW)
          {
            g_print("%s, %s, %i, \n", sqlite3_column_text(stmt1, 0), sqlite3_column_text(stmt1, 1), sqlite3_column_int(stmt1, 2));
            gtk_list_store_append(store, &iter);
            gtk_list_store_set(store, &iter, PROGRAM, sqlite3_column_text(stmt1, 0), IMAGE, sqlite3_column_text(stmt1, 1), USED, sqlite3_column_int(stmt1, 2), -1);
            ret_val=sqlite3_step(stmt1);
          }
      }
    if(stmt1!=NULL) sqlite3_finalize(stmt1);
    sqlite3_close(cnn);
  }

