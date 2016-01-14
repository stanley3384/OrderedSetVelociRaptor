
/*
    This is another way to save settings. A good comparison with the gsettings examples.
The settings for the program are stored in a sqlite database so that they can be retrieved
at startup and from multiple instances of the program. The database gets automatically
created and if it gets deleted the program will recreate it.  

    Tested on Ubuntu14.04 and GTK3.10. 

    Compile program: gcc -Wall settings_sql1.c -o settings_sql1 `pkg-config --cflags --libs gtk+-3.0` -lsqlite3 

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<sqlite3.h>

gchar *database="program_settings.db";

static gboolean draw_window_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void redraw_window(GtkWidget *widget, gpointer data);
static void initialize_default_settings();
static void save_settings(GtkWidget *widget, gpointer *data);
static void get_settings_sql(GString *number, GString *color);
static void get_saved_settings(GtkWidget *widget, gpointer *data);

int main(int argc, char **argv)
  {
    gtk_init(&argc, &argv);

    gchar *current_dir=g_get_current_dir();
    gchar *database_path=g_strdup_printf("%s/%s", current_dir, database);
    if(!g_file_test(database_path, G_FILE_TEST_EXISTS)) initialize_default_settings();
    g_free(current_dir);
    g_free(database_path);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);    
   
    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "3");
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_widget_set_vexpand(combo1, TRUE);
    
    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "red");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "green");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "blue");
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_widget_set_vexpand(combo2, TRUE);
    g_signal_connect(combo2, "changed", G_CALLBACK(redraw_window), window);

    gtk_widget_set_app_paintable(window, TRUE);
    g_signal_connect(window, "draw", G_CALLBACK(draw_window_background), combo2);

    gpointer combos[]={combo1, combo2};
    get_saved_settings(NULL, combos);

    GtkWidget *button1=gtk_button_new_with_label("Save Settings");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(save_settings), combos);

    GtkWidget *button2=gtk_button_new_with_label("Get Settings");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(button2, "clicked", G_CALLBACK(get_saved_settings), combos);

    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 3, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;   
  }
static gboolean draw_window_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==0) cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    else if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==1) cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);
    return FALSE;
  }
static void redraw_window(GtkWidget *widget, gpointer data)
  {
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void initialize_default_settings()
  {
    g_print("Initialize Default Settings\n");
    sqlite3 *cnn=NULL;
 
    gchar *sql1="CREATE TABLE IF NOT EXISTS ProgramSettings(name TEXT, value TEXT);";
    gchar *sql2="INSERT INTO ProgramSettings VALUES('number', '1');";
    gchar *sql3="INSERT INTO ProgramSettings VALUES('color', 'red');";

    sqlite3_open(database, &cnn);
    sqlite3_exec(cnn, sql1, 0, 0, 0);
    sqlite3_exec(cnn, sql2, 0, 0, 0);
    sqlite3_exec(cnn, sql3, 0, 0, 0);    
    sqlite3_close(cnn);   
  }
static void save_settings(GtkWidget *widget, gpointer *data)
  {
    g_print("Save Settings\n");
    gchar *combo1_text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(data[0]));
    gchar *combo2_text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(data[1]));
    gchar *sql1=g_strdup_printf("UPDATE ProgramSettings SET value='%s' WHERE name='number';", combo1_text);
    gchar *sql2=g_strdup_printf("UPDATE ProgramSettings SET value='%s' WHERE name='color';", combo2_text);

    sqlite3 *cnn=NULL;
    sqlite3_open(database, &cnn);
    sqlite3_exec(cnn, sql1, 0, 0, 0);
    sqlite3_exec(cnn, sql2, 0, 0, 0);    
    sqlite3_close(cnn);   

    g_free(combo1_text);
    g_free(combo2_text);
    g_free(sql1);
    g_free(sql2);
  }
static void get_settings_sql(GString *number, GString *color)
  {
    g_print("Get Settings SQL\n");
    gint sql_return=0;
    gchar *sql1="SELECT value FROM ProgramSettings WHERE name=='number';";
    gchar *sql2="SELECT value FROM ProgramSettings WHERE name=='color';";
    sqlite3_stmt *stmt1=NULL;
    sqlite3_stmt *stmt2=NULL;
    sqlite3 *cnn=NULL;

    sqlite3_open(database, &cnn);
    sqlite3_prepare_v2(cnn, sql1, -1, &stmt1, 0);
    sqlite3_prepare_v2(cnn, sql2, -1, &stmt2, 0);
    sql_return=sqlite3_step(stmt1);
    while(sql_return==SQLITE_ROW)
      {
        g_print("number: %s\n", sqlite3_column_text(stmt1, 0));
        g_string_append_printf(number, "%s", sqlite3_column_text(stmt1, 0));
        sql_return=sqlite3_step(stmt1);
      }
    sql_return=sqlite3_step(stmt2);
    while(sql_return==SQLITE_ROW)
      {
        g_print("color: %s\n", sqlite3_column_text(stmt2, 0));
        g_string_append_printf(color, "%s", sqlite3_column_text(stmt2, 0));
        sql_return=sqlite3_step(stmt2);
      }
    sqlite3_finalize(stmt1); 
    sqlite3_finalize(stmt2);   
    sqlite3_close(cnn); 
  }
static void get_saved_settings(GtkWidget *widget, gpointer *data)
  {
    g_print("Get Saved Settings\n");
    GString *number=g_string_new(NULL);
    GString *color=g_string_new(NULL);

    get_settings_sql(number, color);
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(data[0]), number->str);
    if(g_strcmp0("green", color->str)==0) gtk_combo_box_set_active(GTK_COMBO_BOX(data[1]), 1);
    else if(g_strcmp0("blue", color->str)==0) gtk_combo_box_set_active(GTK_COMBO_BOX(data[1]), 2);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(data[1]), 0);
    
    g_string_free(number, TRUE);
    g_string_free(color, TRUE);
  }







