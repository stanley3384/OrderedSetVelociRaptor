
/*

    Re-write the report_generator.py program in C. A good comparison with Python, Cython and C.
The C program is faster for doing the scrolling and redraws. Also, the binary is smaller than what
Cython produces.  

    For the program image the code will look for dino2.png. It is in the Python folder. Have the
png in the same folder as the report generator for showing the image in the about dialog and the toolbar.

    GTK 3.10 on Ubuntu 14.04.
 
    gcc -Wall -O2 OSVreport_generator.c -o OSVreport_generator -I/usr/include/json-glib-1.0 `pkg-config --cflags --libs gtk+-3.0` -ljson-glib-1.0 -lsqlite3 -lm

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include <json-glib/json-glib.h>
#include<sqlite3.h>
#include<stdlib.h>
#include<string.h>
#include<math.h>

//Textview funtions.
static void change_textview_font(GtkWidget *widget, gpointer data);
static void set_bold_tag(GtkWidget *widget, gpointer data);
static void set_underline_tag(GtkWidget *widget, gpointer data);
static void set_font_tags(GtkWidget *widget, gpointer data);
static void clear_tags(GtkWidget *widget, gpointer data);
//General program functions.
static void about_dialog(GtkWidget *widget, gpointer data);
static void message_dialog(gchar *string);
static void change_sql_entry(GtkWidget *widget, gpointer data);
static gint validate_entries(GtkWidget *ws[]);
static gint check_sql_string(const gchar *sql_string, GtkWidget *ws[]);
static gboolean save_current_row_value(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean clear_row_labels(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean save_current_column_value(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean clear_column_labels(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean save_current_table_value(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean clear_table_labels(GtkWidget *widget, GdkEvent *event, gpointer data);
static void get_pango_markup(GtkWidget *ws[], GString *string);
static void get_tags(GtkTextTag *tag, void *data[]);
static void load_pango_list(GString *string, void *data[]);
static int get_next_list_value(void *data[], gint last);
static void parse_saved_markup_string(gchar *markup_string, GtkWidget *ws[]);
//For saving and getting reports.
static void open_json_file(gchar *file_name, GtkWidget *ws[]);
static void save_json_file(gchar *file_name, GtkWidget *ws[]);
static void open_report(GtkWidget *widget, GtkWidget *ws[]);
static void save_report(GtkWidget *widget, GtkWidget *ws[]);
//Dialog and funtions for row and column labels.
static void labels_dialog(GtkWidget *widget, GtkWidget *ws[]);
static void row_combo_changed(GtkWidget *widget, gpointer data);
static void column_combo_changed(GtkWidget *widget, gpointer data);
static void load_labels(GtkWidget *widget, GtkWidget *cs[]);
static void change_standard_labels(GtkWidget *widget, GtkWidget *cs[]);
static void get_letters(GPtrArray *micro_labels, gint rows);
//Dialog and functions for table labels.
static void table_labels_dialog(GtkWidget *widget, GtkWidget *ws[]);
static void activate_table_labels_button(GtkWidget *widget, gpointer data);
static void table_combo_changed(GtkWidget *widget, gpointer data);
static void load_table_labels(GtkWidget *widget, gpointer data);
//Drawing functions.
static gboolean draw_report(GtkWidget *da, cairo_t *cr, GtkWidget *ws[]);
static void start_draw_report(GtkNotebook *notebook, GtkWidget *page, guint page_num, GtkWidget *ws[]);
static void drawing_area_preview(GtkWidget *da, cairo_t *cr, GtkWidget *ws[]);
static void get_table_string(GString *string, GtkWidget *ws[], gint page_number, gint table, gint count_lines);
static void draw_tables(PangoLayout *pango_layout, cairo_t *cr, GtkWidget *ws[], gint page_number, gint table, gint count_lines);
static void heatmap_value_rgb(gdouble data_value, gdouble min, double max, gdouble *red, gdouble *green, gdouble *blue);
static void heatmap_value_bry(gdouble data_value, gdouble min, double max, gdouble *red, gdouble *green, gdouble *blue);
static void heatmap_value_iris(gdouble data_value, gdouble min, double max, gdouble *red, gdouble *green, gdouble *blue);
static void get_test_data1(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left, gint round_float);
static void get_test_data2(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left);
static void get_test_data3(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left);
static void get_db_data_for_crosstab(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left, gint round_float, gchar *sql_string);
static void get_db_data_for_table(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left, gint round_float, gchar *sql_string);
static void get_labels_for_drawing(gint tables, gint rows, gint columns, gint column_width, gint shift_column_left);
static gdouble round1(gdouble x, guint digits);
//Print functions.
static void print_dialog(GtkWidget *widget, GtkWidget *ws[]);
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, GtkWidget *ws[]);
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, GtkWidget *ws[]);
static void end_print(GtkPrintOperation *operation, GtkPrintContext *context, gpointer data);

//Need to package some of these globals.
//Globals for blocking signals when inserting rows into combo boxes.
static gint row_combo_block=0;
static gint column_combo_block=0;
static gint table_combo_block=0;
static gint da_block=0;
static gint row_clear_block=0;
static gint column_clear_block=0;
static gboolean da_blocking=FALSE;
//For comparison and clearing pointer arrays when entry value changes.
gint g_row_value=10;
gint g_column_value=5;
gint g_table_value=5;
//Globals for storing labels.
static GPtrArray *g_row_labels=NULL;
static GPtrArray *g_column_labels=NULL;
static GPtrArray *g_table_labels=NULL;
//Drawing array for values.
static GPtrArray *g_data_values=NULL;
static GArray *g_min_max=NULL;
//Drawing globals.
static gint plate_counter=1;
static gint plate_counter_sql=1;
//Printing globals
static gint lines_per_page=0;
static gint total_lines=0;
static gint table_count=0;
static gint table_print=0;
gboolean drawing_data_valid=FALSE;
static PangoLayout *pango_layout_print=NULL;
//The sqlite database.
gchar *database_name="VelociRaptorData.db";

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "OSV Report Generator");
    gtk_container_set_border_width(GTK_CONTAINER(window), 15);
    gtk_window_set_default_size(GTK_WINDOW(window), 750, 550);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Open Report");
    GtkWidget *menu1item2=gtk_menu_item_new_with_label("Save Report");
    GtkWidget *menu1item3=gtk_menu_item_new_with_label("Print Report");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item2);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item3);
    GtkWidget *title1=gtk_menu_item_new_with_label("File");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    
    GtkWidget *menu2=gtk_menu_new();
    GtkWidget *menu2item1=gtk_menu_item_new_with_label("Report Generator");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu2), menu2item1);
    GtkWidget *title2=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title2), menu2);
    g_signal_connect(menu2item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title2);

    GtkWidget *textview1=gtk_text_view_new();
    gtk_widget_set_hexpand(textview1, TRUE);
    gtk_widget_set_vexpand(textview1, TRUE);
    GtkTextBuffer *buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1));
    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(buffer1), "       This is the title for the report.\n This is a short description.", -1);
    gtk_text_buffer_create_tag(buffer1, "font='8'", "font", "8", NULL);
    gtk_text_buffer_create_tag(buffer1, "font='10'", "font", "10", NULL);
    gtk_text_buffer_create_tag(buffer1, "font='12'", "font", "12", NULL);
    gtk_text_buffer_create_tag(buffer1, "font='14'", "font", "14", NULL);
    gtk_text_buffer_create_tag(buffer1, "font='16'", "font", "16", NULL);
    gtk_text_buffer_create_tag(buffer1, "weight='900'", "weight", "900", NULL); 
    gtk_text_buffer_create_tag(buffer1, "underline='single'", "underline", PANGO_UNDERLINE_SINGLE, NULL);
    PangoFontDescription *font=pango_font_description_from_string("Monospace 12"); 
    gtk_widget_override_font(textview1, font);
    pango_font_description_free(font);
    
    GtkWidget *scroll1=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scroll1, TRUE);
    gtk_widget_set_vexpand(scroll1, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll1), textview1);

    GtkWidget *label0=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label0), "<b>Draw Grids, Crosstabs and Tables</b>");
    gtk_widget_set_hexpand(label0, TRUE);
    gtk_widget_set_halign(label0, GTK_ALIGN_CENTER);

    GtkWidget *label1=gtk_label_new("Rows");
    GtkWidget *entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
    gtk_entry_set_text(GTK_ENTRY(entry1), "10");

    GtkWidget *label2=gtk_label_new("Columns");
    GtkWidget *entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 3);
    gtk_entry_set_text(GTK_ENTRY(entry2), "5");
    
    GtkWidget *label3=gtk_label_new("Shift Right");
    GtkWidget *entry3=gtk_entry_new();
    gtk_widget_set_halign(entry3, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 3);
    gtk_entry_set_text(GTK_ENTRY(entry3), "10");

    GtkWidget *label4=gtk_label_new("Shift Down");
    GtkWidget *entry4=gtk_entry_new();
    gtk_widget_set_halign(entry4, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry4), 3);
    gtk_entry_set_text(GTK_ENTRY(entry4), "2");

    GtkWidget *label5=gtk_label_new("Column Width");
    GtkWidget *entry5=gtk_entry_new();
    gtk_widget_set_halign(entry5, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry5), 3);
    gtk_entry_set_text(GTK_ENTRY(entry5), "10");

    GtkWidget *label6=gtk_label_new("Pad Number");
    GtkWidget *entry6=gtk_entry_new();
    gtk_widget_set_halign(entry6, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry6), 3);
    gtk_entry_set_text(GTK_ENTRY(entry6), "1");

    GtkWidget *label7=gtk_label_new("Pad Column");
    GtkWidget *entry7=gtk_entry_new();
    gtk_widget_set_halign(entry7, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry7), 3);
    gtk_entry_set_text(GTK_ENTRY(entry7), "1");

    GtkWidget *label8=gtk_label_new("Tables");
    GtkWidget *entry8=gtk_entry_new();
    gtk_widget_set_halign(entry8, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry8), 3);
    gtk_entry_set_text(GTK_ENTRY(entry8), "5");

    GtkWidget *label9=gtk_label_new("Table Label");
    GtkWidget *entry9=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry9), "Plate ");

    GtkWidget *entry10=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry10), "SELECT Percent FROM Data");
    gtk_widget_set_sensitive(entry10, FALSE);

    GtkWidget *label11=gtk_label_new("Round Floats");
    GtkWidget *entry11=gtk_entry_new();
    gtk_widget_set_halign(entry11, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry11), 3);
    gtk_entry_set_text(GTK_ENTRY(entry11), "3");

    GtkWidget *label12=gtk_label_new("Data Source");

    GtkWidget *check1=gtk_check_button_new_with_label("Add Table Label");

    GtkWidget *check2=gtk_check_button_new_with_label("Grid Numbers");
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check2), TRUE);

    GtkWidget *button2=gtk_button_new_with_label("Bold");
    gtk_widget_set_hexpand(button2, FALSE);
    g_signal_connect(button2, "clicked", G_CALLBACK(set_bold_tag), textview1);

    GtkWidget *button3=gtk_button_new_with_label("Underline");
    gtk_widget_set_hexpand(button3, FALSE);
    g_signal_connect(button3, "clicked", G_CALLBACK(set_underline_tag), textview1);

    GtkWidget *button4=gtk_button_new_with_label("Clear");
    gtk_widget_set_hexpand(button4, FALSE);
    g_signal_connect(button4, "clicked", G_CALLBACK(clear_tags), textview1);

    GtkWidget *button5=gtk_button_new_with_label("Set Labels");
    g_signal_connect(entry1, "focus_in_event", G_CALLBACK(save_current_row_value), button5);
    row_clear_block=g_signal_connect(entry1, "focus_out_event", G_CALLBACK(clear_row_labels), button5);
    g_signal_connect(entry2, "focus_in_event", G_CALLBACK(save_current_column_value), button5);
    column_clear_block=g_signal_connect(entry2, "focus_out_event", G_CALLBACK(clear_column_labels), button5);

    GtkWidget *button6=gtk_button_new_with_label("Set Table Labels");
    gtk_widget_set_sensitive(button6, FALSE);
    g_signal_connect(check1, "clicked", G_CALLBACK(activate_table_labels_button), button6);
    g_signal_connect(entry8, "focus_in_event", G_CALLBACK(save_current_table_value), button6);
    g_signal_connect(entry8, "focus_out_event", G_CALLBACK(clear_table_labels), button6);
    
    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "White");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Blue");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "RGB");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "BRY");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Iris");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo1), "1");

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "No Labels");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Tabular");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "CrossTab");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo2), "1");

    GtkWidget *combo3=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "Monospace 8");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "Monospace 10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "Monospace 12");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 3, "4", "Monospace 14");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 4, "5", "Monospace 16");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo3), "3");
    g_signal_connect(combo3, "changed", G_CALLBACK(change_textview_font), textview1);

    GtkWidget *combo4=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 0, "1", "Random");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 1, "2", "RCsequence");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 2, "3", "CRsequence");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 3, "4", "CrosstabFromDB");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 4, "5", "TableFromDB");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo4), "1");
    g_signal_connect(combo4, "changed", G_CALLBACK(change_sql_entry), entry10);

    GtkWidget *combo5=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 0, "1", "No Frame");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 1, "2", "Black Frame");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 2, "3", "Red Frame");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 3, "4", "Green Frame");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 4, "5", "Blue Frame");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo5), "2");

    GtkWidget *combo6=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 0, "1", "Nudge 1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 1, "2", "Nudge 2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 2, "3", "Nudge 3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 3, "4", "Nudge 4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 4, "5", "Nudge 5");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo6), "2");

    GtkWidget *combo7=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo7), 0, "1", "Font 8");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo7), 1, "2", "Font 10");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo7), 2, "3", "Font 12");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo7), 3, "4", "Font 14");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo7), 4, "5", "Font 16");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo7), "3");
    g_signal_connect(combo7, "changed", G_CALLBACK(set_font_tags), textview1);

    //Initailize global arrays.
    g_row_labels=g_ptr_array_new_full(5, g_free);
    g_column_labels=g_ptr_array_new_full(5, g_free);
    g_table_labels=g_ptr_array_new_full(5, g_free);
    g_data_values=g_ptr_array_new_full(5000, g_free);
    g_min_max=g_array_new(FALSE, TRUE, sizeof(gdouble));
 
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 5);
    gtk_grid_attach(GTK_GRID(grid), scroll1, 0, 0, 5, 4);
    gtk_grid_attach(GTK_GRID(grid), button2, 5, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button3, 5, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo7, 5, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button4, 5, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label0, 0, 4, 6, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label8, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry8, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 3, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 2, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 3, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 2, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry5, 3, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label9, 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry9, 1, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label12, 0, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label11, 4, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry11, 5, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 4, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry6, 5, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 4, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry7, 5, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 2, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button5, 3, 8, 1, 1);       
    gtk_grid_attach(GTK_GRID(grid), combo5, 4, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo6, 5, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check1, 3, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button6, 4, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check2, 5, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo4, 1, 10, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), entry10, 0, 11, 6, 1);
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 1, 12, 1, 1);
    
    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_size_request(da, 10000, 10000);

    GtkWidget *scroll2=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scroll2, TRUE);
    gtk_widget_set_vexpand(scroll2, TRUE);

    GtkWidget *layout=gtk_layout_new(NULL, NULL);
    gtk_layout_set_size(GTK_LAYOUT(layout), 10000, 10000);
    gtk_widget_set_hexpand(layout, TRUE);
    gtk_widget_set_vexpand(layout, TRUE);
    gtk_layout_put(GTK_LAYOUT(layout), da, 0, 0);
    gtk_container_add(GTK_CONTAINER(scroll2), layout);

    GtkWidget *notebook=gtk_notebook_new();
    GtkWidget *nb_label1=gtk_label_new("Setup");
    GtkWidget *nb_label2=gtk_label_new("Drawing");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid, nb_label1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll2, nb_label2);

    GtkWidget *ws[]={entry1, entry2, entry3, entry4, entry5, entry6, entry7, entry8, entry9, entry10, entry11, combo1, combo2, combo3, combo4, combo5, combo6, combo7, check1, check2, textview1, window, label2, da, scroll2};
    g_signal_connect(menu1item1, "activate", G_CALLBACK(open_report), ws);
    g_signal_connect(menu1item2, "activate", G_CALLBACK(save_report), ws);
    g_signal_connect(menu1item3, "activate", G_CALLBACK(print_dialog), ws);
    g_signal_connect(button5, "clicked", G_CALLBACK(labels_dialog), ws);
    g_signal_connect(button6, "clicked", G_CALLBACK(table_labels_dialog), ws);
    da_block=g_signal_connect(da, "draw", G_CALLBACK(draw_report), ws);
    g_signal_connect(notebook, "switch_page", G_CALLBACK(start_draw_report), ws);

    gtk_container_add(GTK_CONTAINER(window), notebook);
    
    GError *css_error=NULL;
    gchar css_string[]="GtkWindow, GtkNotebook{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(0,255,0,0.5)), color-stop(0.5,rgba(180,180,180,0.5)), color-stop(1.0,rgba(255,0,255,0.5)));}GtkButton{background: rgba(220,220,220,0.5);}";
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
static void change_textview_font(GtkWidget *widget, gpointer data)
  {
    gchar *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    PangoFontDescription *font=pango_font_description_from_string(text); 
    gtk_widget_override_font(GTK_WIDGET(data), font);
    pango_font_description_free(font);
    if(text!=NULL)g_free(text);
  }
static void set_bold_tag(GtkWidget *widget, gpointer data)
  {
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    if(gtk_text_buffer_get_has_selection(buffer))
      {
        gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
        gtk_text_buffer_apply_tag_by_name(buffer, "weight='900'", &start, &end);
      }
  }
static void set_underline_tag(GtkWidget *widget, gpointer data)
  {
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    if(gtk_text_buffer_get_has_selection(buffer))
      {
        gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
        gtk_text_buffer_apply_tag_by_name(buffer, "underline='single'", &start, &end);
      }
  }
static void set_font_tags(GtkWidget *widget, gpointer data)
  {
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    gint combo7_id=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));
    if(gtk_text_buffer_get_has_selection(buffer))
      {
        gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
        gtk_text_buffer_remove_tag_by_name(buffer, "font='8'", &start, &end);
        gtk_text_buffer_remove_tag_by_name(buffer, "font='10'", &start, &end);
        gtk_text_buffer_remove_tag_by_name(buffer, "font='12'", &start, &end);
        gtk_text_buffer_remove_tag_by_name(buffer, "font='14'", &start, &end);
        gtk_text_buffer_remove_tag_by_name(buffer, "font='16'", &start, &end);
        if(combo7_id==1)gtk_text_buffer_apply_tag_by_name(buffer, "font='8'", &start, &end);
        else if(combo7_id==2)gtk_text_buffer_apply_tag_by_name(buffer, "font='10'", &start, &end);
        else if(combo7_id==3)gtk_text_buffer_apply_tag_by_name(buffer, "font='12'", &start, &end);
        else if(combo7_id==4)gtk_text_buffer_apply_tag_by_name(buffer, "font='14'", &start, &end);
        else gtk_text_buffer_apply_tag_by_name(buffer, "font='16'", &start, &end);
      }
  }
static void clear_tags(GtkWidget *widget, gpointer data)
  {
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(data));
    gtk_text_buffer_get_start_iter(buffer, &start);
    gtk_text_buffer_get_end_iter(buffer, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "OSV Report Generator");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A report generator for the Ordered Set VelociRaptor program.");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2015 C. Eric Cashon");
    GError *error=NULL;
    GdkPixbuf *pixbuf=gdk_pixbuf_new_from_file("dino2.png", &error);
     if(!pixbuf)
       {
         g_print("%s\n", error->message);
         g_error_free(error);
       }
     else
       {
         gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), pixbuf);
       }
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static void message_dialog(gchar *string)
  {
    GtkWidget *dialog=gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s",  string);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);    
  }
static void change_sql_entry(GtkWidget *widget, gpointer data)
  {
    gint active_id=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));
    if(active_id==1||active_id==2||active_id==3) gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
    else gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
  }
static gint validate_entries(GtkWidget *ws[])
  {
    gint e1=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
    gint e2=atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])));
    gint e3=atoi(gtk_entry_get_text(GTK_ENTRY(ws[2])));
    gint e4=atoi(gtk_entry_get_text(GTK_ENTRY(ws[3])));
    gint e5=atoi(gtk_entry_get_text(GTK_ENTRY(ws[4])));
    gint e6=atoi(gtk_entry_get_text(GTK_ENTRY(ws[5])));
    gint e7=atoi(gtk_entry_get_text(GTK_ENTRY(ws[6])));
    gint e8=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
    gint e11=atoi(gtk_entry_get_text(GTK_ENTRY(ws[10])));
    gchar *message=NULL;

    if(0>=e1||e1>100)
      {
        message=g_strdup_printf("Rows %s, Range 0<Rows<=100", gtk_entry_get_text(GTK_ENTRY(ws[0]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(0>=e2||e2>50)
      {
        message=g_strdup_printf("Columns %s, Range 0<Columns<=50", gtk_entry_get_text(GTK_ENTRY(ws[1]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(0>e3||e3>30)
      {
        message=g_strdup_printf("Shift Right %s, Range 0<=Shift Right<=30", gtk_entry_get_text(GTK_ENTRY(ws[0]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(1>e4||e4>10)
      {
        message=g_strdup_printf("Shift Down %s, Range 1<=Shift Down<=10", gtk_entry_get_text(GTK_ENTRY(ws[3]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(2>e5||e5>20)
      {
        message=g_strdup_printf("Column Width %s, Range 2<=Column Width<=20", gtk_entry_get_text(GTK_ENTRY(ws[4]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(0>e6||e6>5)
      {
        message=g_strdup_printf("Pad Number %s, Range 0<=Pad Number<=5", gtk_entry_get_text(GTK_ENTRY(ws[5]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(0>e7||e7>5)
      {
        message=g_strdup_printf("Pad Column %s, Range 0<=Pad Column<=5", gtk_entry_get_text(GTK_ENTRY(ws[6]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(1>e8||e8>20)
      {
        message=g_strdup_printf("Tables %s, Range 1<=Tables<=20", gtk_entry_get_text(GTK_ENTRY(ws[7]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(1>e11||e11>7)
      {
        message=g_strdup_printf("Round Floats %s, Range 0<Round Floats<=7", gtk_entry_get_text(GTK_ENTRY(ws[10]))); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(e1*e2*e8>5000)
      {
        message=g_strdup_printf("Max Rectangles %i, Range rows*columns*tables<=5000", e1*e2*e8); 
        message_dialog(message);
        g_free(message);
        return 1;
      }
    else if(gtk_widget_get_sensitive(ws[9]))
      {
        gint database_rows=check_sql_string(gtk_entry_get_text(GTK_ENTRY(ws[9])), ws);
        g_print("Database rows %i\n", database_rows);
        gint numbers=e1*e2*e8;
        if(database_rows<numbers&&database_rows>0)
          {
            message=g_strdup_printf("There are not enough rows in the database table\nto print the requested rows, columns and tables.\nDatabase rows %i Requested %i", database_rows, numbers);
            message_dialog(message);
            g_free(message);
            return 1; 
          }
         else if(database_rows==0)
          {
            message=g_strdup_printf("The SQL statement isn't valid.");
            message_dialog(message);
            g_free(message);
            return 1; 
          }
         else return 0;
      }
    else return 0;
  }
static gint check_sql_string(const gchar *sql_string, GtkWidget *ws[])
  {
    gboolean valid_string=TRUE;
    gint ret_value=0;
    gchar *sql1="SELECT count(*) ";
    gchar *select_rows=NULL;
    //Build a row count SELECT statement.
    GMatchInfo *match_info1;
    GRegex *regex1=g_regex_new("(?i)(FROM).*", 0, 0, NULL);
    g_regex_match(regex1, sql_string, 0, &match_info1);
    if(g_match_info_matches(match_info1))
      {
        gchar *string1=g_match_info_fetch(match_info1, 0);
        g_print("Found: %s\n", string1);
        select_rows=g_strdup_printf("%s%s%s", sql1, string1, ";");
        g_print("%s\n", select_rows);  
        g_free(string1);
      }
    //Count the number of columns in the SELECT statement and change UI columns.
    //At least one column even if there isn't any problem code. Need at least a 1
    //if the column entry gets changed.
    GMatchInfo *match_info2;
    GRegex *regex2=g_regex_new("(?i)(?<=SELECT)(.*?)(?=FROM)", 0, 0, NULL);
    g_regex_match(regex2, sql_string, 0, &match_info2);
    if(g_match_info_matches(match_info2))
      {
        gchar *string2=g_match_info_fetch(match_info2, 0);
        g_print("Found: %s\n", string2);
        gchar **string_array=g_strsplit(string2, ",", -1);
        gint i=0;
        while(string_array[i]!=NULL) i++;
        g_print("Columns in SELECT %i\n", i);
        g_strfreev(string_array);
        g_free(string2);
        if(atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[14])))==5)
          {
            if(atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])))!=i)
              {
                gchar *text=g_strdup_printf("%i", i);
                gtk_entry_set_text(GTK_ENTRY(ws[1]), text);
                g_free(text);
                gtk_label_set_markup(GTK_LABEL(ws[22]), "<span foreground='blue'>Columns Changed</span>");
              }
          }
      }
    if(g_match_info_matches(match_info1))
      {
        sqlite3 *cnn=NULL;
        sqlite3_stmt *stmt1=NULL;
        sqlite3_stmt *stmt2=NULL;
        sqlite3_open(database_name, &cnn);
        sqlite3_prepare_v2(cnn, sql_string, -1, &stmt1, 0);
        if(stmt1==NULL) valid_string=FALSE;
        if(valid_string)
          {
            sqlite3_prepare_v2(cnn, select_rows, -1, &stmt2, 0);
            if(stmt2!=NULL)
              {
                sqlite3_step(stmt2);
                ret_value=sqlite3_column_int(stmt2, 0);
              }
          }
        if(stmt1!=NULL) sqlite3_finalize(stmt1);
        if(stmt2!=NULL) sqlite3_finalize(stmt2);
        sqlite3_close(cnn);
      }

    g_match_info_free(match_info1);
    g_regex_unref(regex1);
    g_match_info_free(match_info2);
    g_regex_unref(regex2);
    if(select_rows!=NULL) g_free(select_rows);
    return ret_value;
  }
static gboolean save_current_row_value(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint r_value=atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(r_value>0&&r_value<=100) g_row_value=r_value;
    else g_print("Rows %s, Range 0<Rows<=100\n", gtk_entry_get_text(GTK_ENTRY(widget)));         
    return FALSE;  
  }
static gboolean clear_row_labels(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint i=0;
    gint r_value=atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(r_value>0&&r_value<=100)
      {
        if(g_row_labels->len>0&&r_value!=g_row_value)
          {
            g_print("Clear Row Labels\n");
            gint array_length=g_row_labels->len;
            for(i=0;i<array_length; i++)
              {
                g_ptr_array_remove_index_fast(g_row_labels, 0);
              }
            GtkWidget *child=gtk_bin_get_child(GTK_BIN(data));
            gtk_label_set_markup(GTK_LABEL(child), "<span foreground='blue'>Labels Changed</span>");
          } 
      }
    else g_print("Rows %s, Range 0<Rows<=100\n", gtk_entry_get_text(GTK_ENTRY(widget)));  
    return FALSE;
  }
static gboolean save_current_column_value(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint r_value=atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(r_value>0&&r_value<=50) g_column_value=r_value;
    else g_print("Columns %s, Range 0<Columns<=50\n", gtk_entry_get_text(GTK_ENTRY(widget)));         
    return FALSE;  
  }
static gboolean clear_column_labels(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint i=0;
    gint r_value=atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(r_value>0&&r_value<=50)
      {
        if(g_column_labels->len>0&&r_value!=g_column_value)
          {
            g_print("Clear Column Labels\n");
            gint array_length=g_column_labels->len;
            for(i=0;i<array_length; i++) g_ptr_array_remove_index_fast(g_column_labels, 0);
            GtkWidget *child=gtk_bin_get_child(GTK_BIN(data));
            gtk_label_set_markup(GTK_LABEL(child), "<span foreground='blue'>Labels Changed</span>");
          }
      }
    else g_print("Columns %s, Range 0<Columns<=50\n", gtk_entry_get_text(GTK_ENTRY(widget)));  
    return FALSE;
  }
static gboolean save_current_table_value(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint r_value=atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(r_value>0&&r_value<=20) g_table_value=r_value;
    else g_print("Tables %s, Range 0<Tables<=20\n", gtk_entry_get_text(GTK_ENTRY(widget)));         
    return FALSE;  
  }
static gboolean clear_table_labels(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint i=0;
    gint r_value=atoi(gtk_entry_get_text(GTK_ENTRY(widget)));
    if(r_value>0&&r_value<=20)
      {
        if(g_table_labels->len>0&&r_value!=g_table_value)
          {
            g_print("Clear Table Labels\n");
            gint array_length=g_table_labels->len;
            for(i=0;i<array_length; i++)
              {
                g_ptr_array_remove_index_fast(g_table_labels, 0);
              }
            GtkWidget *child=gtk_bin_get_child(GTK_BIN(data));
            gtk_label_set_markup(GTK_LABEL(child), "<span foreground='blue'>Table Labels Changed</span>");
          }
      }
    else g_print("Tables %s, Range 0<Tables<=100\n", gtk_entry_get_text(GTK_ENTRY(widget)));  
    return FALSE;
  }
static void get_pango_markup(GtkWidget *ws[], GString *string)
  {
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
    GtkTextTagTable *tag_list=gtk_text_buffer_get_tag_table(buffer);
    GPtrArray *pango_tag_list=g_ptr_array_new_full(10, g_free);
    void *data[]={buffer, pango_tag_list};
    gtk_text_tag_table_foreach(tag_list, (GtkTextTagTableForeach)get_tags , data);
    load_pango_list(string, data);
    g_ptr_array_free(pango_tag_list, TRUE);  
  }
static void get_tags(GtkTextTag *tag, void *data[])
  {
    gboolean loop=TRUE;
    gboolean switch1=FALSE;
    gint offset1=0;
    gint offset2=0;
    GtkTextIter start;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(data[0]), &start);    
    gboolean not_end=gtk_text_iter_forward_to_tag_toggle(&start, tag);

    while(loop)
      {
        if(not_end)
          {
            offset1=gtk_text_iter_get_offset(&start);
            if(offset1==offset2) break;
            if(switch1)
              {
                gchar *tag_name=NULL;
                g_object_get(tag, "name", &tag_name, NULL);
                //g_print("Tag Found at %i %i %s\n", offset2, offset1, tag_name);
                g_ptr_array_add(data[1], g_strdup_printf("%i", offset2));
                g_ptr_array_add(data[1], g_strdup_printf("%i", offset1));
                g_ptr_array_add(data[1], g_strdup(tag_name));
                if(tag_name!=NULL) g_free(tag_name);
                switch1=FALSE;
              }
            else switch1=TRUE;
            offset2=offset1;
            not_end=gtk_text_iter_forward_to_tag_toggle(&start, tag);
          }
        else loop=FALSE;
      }

  }
static void load_pango_list(GString *string, void *data[])
  {
    gint i=0;
    gint j=0;
    gint check_in_list=G_MININT;
    GtkTextIter start, end;
    gtk_text_buffer_get_start_iter(data[0], &start);
    gtk_text_buffer_get_end_iter(data[0], &end);
    gchar *text=gtk_text_buffer_get_text(data[0], &start, &end, TRUE);
    gint length1=strlen(text);
    guint length2=((GPtrArray*)data[1])->len/3;
    //g_print("Length2 %i\n", length2);
    gboolean open_tags[]={FALSE, FALSE, FALSE, FALSE, FALSE, FALSE, FALSE};
    gboolean span_open=FALSE;
    for(i=0;i<length1;i++)
      {
        if(i>check_in_list&&check_in_list!=-1)
          {
            check_in_list=get_next_list_value(data, check_in_list); 
            //g_print("Check %i %i\n", i, check_in_list);
          }
        if(i==check_in_list)
          {
            for(j=0;j<length2;j++)
              {
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "underline='single'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[0]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "weight='900'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[1]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='8'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[2]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='10'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[3]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='12'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[4]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='14'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[5]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='16'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j))==i)
                  {
                    open_tags[6]=TRUE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "underline='single'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[0]=FALSE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "weight='900'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[1]=FALSE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='8'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[2]=FALSE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='10'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[3]=FALSE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='12'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[4]=FALSE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='14'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[5]=FALSE;
                  }
                if(g_strcmp0((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+2), "font='16'")==0&&atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], 3*j+1))==i)
                  {
                    open_tags[6]=FALSE;
                  }
              }
            if(span_open)
              {
                g_string_append(string, "</span>");
                span_open=FALSE;
              }
            if(open_tags[0]||open_tags[1]||open_tags[2]||open_tags[3]||open_tags[4]||open_tags[5]||open_tags[6])
              {
                g_string_append(string, "<span");
                for(j=0;j<7;j++)
                  {
                    if(open_tags[j]&&j==0) g_string_append(string, " underline='single'");
                    if(open_tags[j]&&j==1) g_string_append(string, " weight='900'");
                    if(open_tags[j]&&j==2) g_string_append(string, " font='8'");
                    if(open_tags[j]&&j==3) g_string_append(string, " font='10'");
                    if(open_tags[j]&&j==4) g_string_append(string, " font='12'");
                    if(open_tags[j]&&j==5) g_string_append(string, " font='14'");
                    if(open_tags[j]&&j==6) g_string_append(string, " font='16'");
                  }
                g_string_append(string, ">"); 
                span_open=TRUE;   
              }
          }
        g_string_append_c(string, text[i]); 
      }
    //g_print("%s\n", string->str);
    if(text!=NULL) g_free(text);
  }
static int get_next_list_value(void *data[], gint last)
  {
    //Every third element is a tag name in the list. 
    gint i=0;
    gint value=-1;
    gint lower=-1;
    gint upper=G_MAXINT;
    gint length=((GPtrArray*)data[1])->len;
    for(i=0;i<length;i++)
     {
       lower=atoi((gchar*)g_ptr_array_index((GPtrArray*)data[1], i));
       if((i+1)%3!=0)
         {
         if(lower>last&&lower<upper)
           {
             value=lower;
             upper=lower; 
           }
         if(lower>last&&upper<lower)
           {
             value=upper;
           } 
         }       
    }
    return value;
  }
static void parse_saved_markup_string(gchar *markup_string, GtkWidget *ws[])
  {
    gint i=0;
    gint j=0;
    GtkTextIter start_iter, span_iter;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
    gtk_text_buffer_set_text(buffer, markup_string, -1);
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    gtk_text_buffer_get_start_iter(buffer, &span_iter);
    gint index=0;
    gboolean count=TRUE;
    gint move_ahead_six=0;
    GArray *tag_locations=g_array_new(FALSE, FALSE, sizeof(gint));
    GPtrArray *tag_names=g_ptr_array_new_full(10, g_free);
    GString *new_string=g_string_new("");
    GString *span_string=g_string_new("");

    //Parse markup string. Just check for span tags.
    while(!gtk_text_iter_is_end(&start_iter))
      {
        if('<'==gtk_text_iter_get_char(&start_iter))
          {
            gtk_text_iter_assign(&span_iter, &start_iter);
            gtk_text_iter_forward_chars(&span_iter, 5);
            gchar *test_string=gtk_text_buffer_get_text(buffer, &start_iter, &span_iter, FALSE);
            if(g_strcmp0(test_string, "<span")==0)
              {
                count=FALSE;
                //g_print("start %i ", index);
                g_array_append_val(tag_locations, index);
                move_ahead_six=0;
              }
            if(g_strcmp0(test_string, "</spa")==0)
              {
                count=FALSE;
                //g_print("end %i\n", index);
                g_array_append_val(tag_locations, index);
                move_ahead_six=0;
              }
            if(test_string!=NULL) g_free(test_string);
          }
        if(count==FALSE)
          {
            move_ahead_six+=1;
            if(move_ahead_six>6)
              {
                if('>'!=gtk_text_iter_get_char(&start_iter))
                  {
                    g_string_append_c(span_string, gtk_text_iter_get_char(&start_iter));
                  }
              }
          }
        if(count==TRUE)
          {
            index+=1;
            g_string_append_c(new_string, gtk_text_iter_get_char(&start_iter));
          }
        if('>'==gtk_text_iter_get_char(&start_iter)&&count==FALSE)
          {
            count=TRUE;
            if(span_string->len!= 0)
              {
                //g_print("Tag %s ", span_string->str);
                g_ptr_array_add(tag_names, g_strdup(span_string->str)); 
              }
            g_string_erase(span_string, 0, -1);
          }
        gtk_text_iter_forward_char(&start_iter);
      }

    gtk_text_buffer_set_text(buffer, new_string->str, -1);
    //g_print("%s\n", new_string->str);

    GtkTextIter offset1, offset2;
    gtk_text_buffer_get_start_iter(buffer, &offset1);
    gtk_text_buffer_get_start_iter(buffer, &offset2);
    gint length1=tag_locations->len/2;
    gint start=0;
    gint end=0;
    for(i=0;i<length1;i++)
      {
        start=g_array_index(tag_locations, gint, 2*i);
        end=g_array_index(tag_locations, gint, 2*i+1);
        gtk_text_iter_set_offset(&offset1, start);
        gtk_text_iter_set_offset(&offset2, end);
        gchar **string_array=g_strsplit((gchar*)g_ptr_array_index(tag_names, i), " ", -1);
        j=0;
        while(string_array[j]!=NULL)
          {
            //g_print("%i %i %s\n", start, end, string_array[j]);
            gtk_text_buffer_apply_tag_by_name(buffer, string_array[j], &offset1, &offset2);
            j++;
          }
        g_strfreev(string_array);
      }
    
    g_array_free(tag_locations, TRUE);
    g_ptr_array_free(tag_names, TRUE);
    g_string_free(new_string, TRUE);
    g_string_free(span_string, TRUE);
  }
static void open_json_file(gchar *file_name, GtkWidget *ws[])
  {
    gint i=0;
    if(g_file_test(file_name, G_FILE_TEST_EXISTS)) 
     {
       //Get the data from the file.
       GError *error=NULL;
       JsonParser *parser = json_parser_new();
       json_parser_load_from_file(parser, file_name, &error);
       if(error)
         {
           g_print("Couldn't open report file. %s\n", error->message);
           g_error_free(error);
         }
       else
         {
           gboolean found_member=TRUE;
           JsonReader *reader = json_reader_new(json_parser_get_root(parser));
           found_member=json_reader_read_member(reader, "e1");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[0]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e2");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[1]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e3");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[2]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e4");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[3]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e5");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[4]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e6");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[5]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e7");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[6]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e8");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[7]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e9");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[8]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e10");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[9]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "e11");
           if(!found_member) goto bad_element;
           gtk_entry_set_text(GTK_ENTRY(ws[10]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c1");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[11]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c2");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[12]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c3");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[13]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c4");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[14]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c5");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[15]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c6");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[16]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "c7");
           if(!found_member) goto bad_element;
           gtk_combo_box_set_active_id(GTK_COMBO_BOX(ws[17]), json_reader_get_string_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "ch1");
           if(!found_member) goto bad_element;
           gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ws[18]), (gboolean)json_reader_get_int_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "ch2");
           if(!found_member) goto bad_element;
           gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ws[19]), (gboolean)json_reader_get_int_value(reader));
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "markup");
           if(!found_member) goto bad_element;
           gchar *markup_string=g_strdup(json_reader_get_string_value(reader));
           //Parse and set the string to the text buffer.  
           parse_saved_markup_string(markup_string, ws);
           if(markup_string!=NULL) g_free(markup_string);
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "g_row_labels");
           if(!found_member) goto bad_element;
           gint elements1=json_reader_count_elements(reader);
           gint array_length1=g_row_labels->len;
           for(i=0;i<array_length1; i++)
             {
               g_ptr_array_remove_index_fast(g_row_labels, 0);
             }
           for(i=0;i<elements1;i++)
             {
               json_reader_read_element(reader, i);
               g_ptr_array_add(g_row_labels, g_strdup(json_reader_get_string_value(reader))); 
               json_reader_end_element(reader);
             }
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "g_column_labels");
           if(!found_member) goto bad_element;
           gint elements2=json_reader_count_elements(reader);
           gint array_length2=g_column_labels->len;
           for(i=0;i<array_length2; i++)
             {
               g_ptr_array_remove_index_fast(g_column_labels, 0);
             }
           for(i=0;i<elements2;i++)
             {
               json_reader_read_element(reader, i);
               g_ptr_array_add(g_column_labels, g_strdup(json_reader_get_string_value(reader))); 
               json_reader_end_element(reader);
             }
           json_reader_end_member(reader);
           found_member=json_reader_read_member(reader, "g_table_labels");
           if(!found_member) goto bad_element;
           gint elements3=json_reader_count_elements(reader);
           gint array_length3=g_table_labels->len;
           for(i=0;i<array_length3; i++)
             {
               g_ptr_array_remove_index_fast(g_table_labels, 0);
             }
           for(i=0;i<elements3;i++)
             {
               json_reader_read_element(reader, i);
               g_ptr_array_add(g_table_labels, g_strdup(json_reader_get_string_value(reader))); 
               json_reader_end_element(reader);
             }
           json_reader_end_member(reader);
           g_print("Report Opened\n");
           bad_element:
           if(!found_member) g_print("Bad Element in Report\n");
           g_object_unref(reader);
        } 
      g_object_unref(parser);
    }
  } 
static void save_json_file(gchar *file_name, GtkWidget *ws[])
  {
    gint ret_val=validate_entries(ws);
    gint i=0;
    if(ret_val==0)
      {
        JsonBuilder *builder=json_builder_new();
        json_builder_begin_object (builder);
        json_builder_set_member_name(builder, "e1");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[0])));
        json_builder_set_member_name(builder, "e2");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[1])));
        json_builder_set_member_name(builder, "e3");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[2])));
        json_builder_set_member_name(builder, "e4");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[3])));
        json_builder_set_member_name(builder, "e5");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[4])));
        json_builder_set_member_name(builder, "e6");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[5])));
        json_builder_set_member_name(builder, "e7");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[6])));
        json_builder_set_member_name(builder, "e8");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[7])));
        json_builder_set_member_name(builder, "e9");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[8])));
        json_builder_set_member_name(builder, "e10");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[9])));
        json_builder_set_member_name(builder, "e11");
        json_builder_add_string_value(builder, gtk_entry_get_text(GTK_ENTRY(ws[10])));
        json_builder_set_member_name(builder, "c1");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[11])));
        json_builder_set_member_name(builder, "c2");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[12])));
        json_builder_set_member_name(builder, "c3");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[13])));
        json_builder_set_member_name(builder, "c4");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[14])));
        json_builder_set_member_name(builder, "c5");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[15])));
        json_builder_set_member_name(builder, "c6");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[16])));
        json_builder_set_member_name(builder, "c7");
        json_builder_add_string_value(builder, gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[17])));
        json_builder_set_member_name(builder, "ch1");
        json_builder_add_int_value(builder, (int)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws[18])));
        json_builder_set_member_name(builder, "ch2");
        json_builder_add_int_value(builder, (int)gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws[19])));
        GString *markup=g_string_new("");
        get_pango_markup(ws, markup);
        json_builder_set_member_name(builder, "markup");
        json_builder_add_string_value(builder, markup->str);
        g_string_free(markup, TRUE);
        json_builder_set_member_name(builder, "g_row_labels");
        json_builder_begin_array(builder);
        for(i=0;i<g_row_labels->len;i++)
          {
            json_builder_add_string_value(builder, (char*)g_ptr_array_index(g_row_labels, i));
          }
        json_builder_end_array(builder); 
        json_builder_set_member_name(builder, "g_column_labels");
        json_builder_begin_array(builder);
        for(i=0;i<g_column_labels->len;i++)
          {
            json_builder_add_string_value(builder, (char*)g_ptr_array_index(g_column_labels, i));
          }
        json_builder_end_array(builder);  
        json_builder_set_member_name(builder, "g_table_labels");
        json_builder_begin_array(builder);
        for(i=0;i<g_table_labels->len;i++)
          {
            json_builder_add_string_value(builder, (char*)g_ptr_array_index(g_table_labels, i));
          }
        json_builder_end_array(builder);        
        json_builder_end_object(builder);

        //Save the data to the file.
        GError *error=NULL;
        JsonGenerator *generator = json_generator_new();
        JsonNode *root = json_builder_get_root(builder);
        json_generator_set_root(generator, root);
        gboolean file_saved=json_generator_to_file(generator, file_name, &error);
        if(file_saved) g_print("File Saved\n");
        else g_print("JSON File: %s\n", error->message);

        json_node_free(root);
        g_object_unref(builder);
        g_object_unref(generator);
      }
    else g_print("Couldn't save report file.\n");
  }
static void open_report(GtkWidget *widget, GtkWidget *ws[])
  {
    GtkWidget *dialog=gtk_file_chooser_dialog_new("Open Report", GTK_WINDOW(ws[21]), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, "Open", GTK_RESPONSE_ACCEPT, NULL);
    
    gint result=gtk_dialog_run(GTK_DIALOG(dialog));
    if(result==GTK_RESPONSE_ACCEPT)
      {
        gchar *file_name=NULL;
        file_name=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if(file_name) open_json_file(file_name, ws);
        if(file_name!=NULL) g_free(file_name);
      }
    gtk_widget_destroy(dialog); 
  }
static void save_report(GtkWidget *widget, GtkWidget *ws[])
  {
    GtkWidget *dialog=gtk_file_chooser_dialog_new("Save Report", GTK_WINDOW(ws[21]), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    
    gint result=gtk_dialog_run(GTK_DIALOG(dialog));
    if(result==GTK_RESPONSE_ACCEPT)
      {
        gchar *filename=NULL;
        filename=gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
        if(filename) save_json_file(filename, ws);
        if(filename!=NULL) g_free(filename);
      }
    gtk_widget_destroy(dialog); 
  }
static void labels_dialog(GtkWidget *widget, GtkWidget *ws[])
  {
    gtk_button_set_label(GTK_BUTTON(widget), "Set Labels");
    gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
    gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])));
    gint i=0;
    if(rows>0&&rows<100&&columns>0&&columns<50)
      {
        GtkWidget *dialog=gtk_dialog_new_with_buttons("Set Labels", GTK_WINDOW(ws[21]), GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);
        GtkWidget *row_label=gtk_label_new("Set Row Labels");
        GtkWidget *row_combo=gtk_combo_box_text_new_with_entry();
        for(i=0;i<rows;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf("row %i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(row_combo), id, label);
            g_free(id);
            g_free(label);
          }
        gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(row_combo), 0);
        gtk_combo_box_set_active(GTK_COMBO_BOX(row_combo), 0);
        gint active_row1=0;
        row_combo_block=g_signal_connect(GTK_COMBO_BOX(row_combo), "changed", G_CALLBACK(row_combo_changed), &active_row1);

        GtkWidget *column_label=gtk_label_new("Set Column Labels");
        GtkWidget *column_combo=gtk_combo_box_text_new_with_entry();
        for(i=0;i<columns;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf("column %i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(column_combo), id, label);
            g_free(id);
            g_free(label);
          }
        gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(column_combo), 0);
        gtk_combo_box_set_active(GTK_COMBO_BOX(column_combo), 0);
        gint active_row2=0;
        column_combo_block=g_signal_connect(GTK_COMBO_BOX(column_combo), "changed", G_CALLBACK(column_combo_changed), &active_row2);

        GtkWidget *cs[]={row_combo, column_combo};

        GtkWidget *standard_label=gtk_label_new("Standard Formats");
        GtkWidget *standard_combo=gtk_combo_box_text_new();
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(standard_combo), "1", "RowColumns");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(standard_combo), "2", "Numbers");
        gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(standard_combo), "3", "Microtiter");
        gtk_combo_box_set_active_id(GTK_COMBO_BOX(standard_combo), "1");
        g_signal_connect(GTK_COMBO_BOX(standard_combo), "changed", G_CALLBACK(change_standard_labels), cs);

        GtkWidget *focus_button=gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
        gtk_widget_grab_focus(focus_button);
        g_signal_connect(focus_button, "clicked", G_CALLBACK(load_labels), cs);

        GtkWidget *grid=gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
        gtk_grid_attach(GTK_GRID(grid), row_label, 0, 0, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), row_combo, 0, 1, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), column_label, 0, 2, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), column_combo, 0, 3, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), standard_label, 0, 4, 1, 1);
        gtk_grid_attach(GTK_GRID(grid), standard_combo, 0, 5, 1, 1);
 
        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), grid);
        gtk_widget_show_all(dialog);

        gint result=gtk_dialog_run(GTK_DIALOG(dialog));
        if(result==GTK_RESPONSE_OK)
          {
            g_print("Row Label Values\n");
            gint length1=g_row_labels->len;
            for(i=0;i<length1;i++) g_print("%s ", (char*)g_ptr_array_index(g_row_labels, i));
            g_print("\n");
            g_print("Column Label Values\n"); 
            gint length2=g_column_labels->len;
            for(i=0;i<length2;i++) g_print("%s ", (char*)g_ptr_array_index(g_column_labels, i));
            g_print("\n");                   
          }
        gtk_widget_destroy(dialog);            
      }
    else
      {
        gchar *message=g_strdup_printf("0<Rows<100 and 0<Columns<50"); 
        message_dialog(message);
        g_free(message);
      }
  }
static void row_combo_changed(GtkWidget *widget, gpointer data)
  {
    gint active_row=*(gint*)data;
    gchar *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    gint text_id=gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if(text_id!=-1)
      {
        active_row=text_id;
        *(gint*)data=active_row;
      }
    if(text!=NULL)
      {
        g_signal_handler_block((gpointer)widget, row_combo_block);
        gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(widget), active_row, text);
        gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(widget), active_row+1);
        g_signal_handler_unblock((gpointer)widget, row_combo_block);
      }
    if(text!=NULL) g_free(text);
  }
static void column_combo_changed(GtkWidget *widget, gpointer data)
  {
    gint active_row=*(gint*)data;
    gchar *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    gint text_id=gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if(text_id!=-1)
      {
        active_row=text_id;
        *(gint*)data=active_row;
      }
    if(text!=NULL)
      {
        g_signal_handler_block((gpointer)widget, column_combo_block);
        gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(widget), active_row, text);
        gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(widget), active_row+1);
        g_signal_handler_unblock((gpointer)widget, column_combo_block);
      }
    if(text!=NULL) g_free(text);
  }
static void load_labels(GtkWidget *widget, GtkWidget *cs[])
  {
    //Row Labels.
    GtkTreeIter tree_iter1;
    GtkTreeModel *model1=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[0]));
    gboolean iter_found1=gtk_tree_model_get_iter_first(model1, &tree_iter1);
    gint i=0;
    gint array_length1=g_row_labels->len;
    for(i=0;i<array_length1; i++)
      {
        g_ptr_array_remove_index_fast(g_row_labels, 0);
      }
    while(iter_found1)
      {
        gchar *str_data1=NULL;
        gtk_tree_model_get(model1, &tree_iter1, 0, &str_data1, -1);
        g_ptr_array_add(g_row_labels, g_strdup(str_data1)); 
        iter_found1=gtk_tree_model_iter_next(model1, &tree_iter1);
        if(str_data1!=NULL) g_free(str_data1);
      }
    //Column Labels.
    GtkTreeIter tree_iter2;
    GtkTreeModel *model2=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[1]));
    gboolean iter_found2=gtk_tree_model_get_iter_first(model2, &tree_iter2);
    gint array_length2=g_column_labels->len;
    for(i=0;i<array_length2; i++)
      {
        g_ptr_array_remove_index_fast(g_column_labels, 0);
      }
    while(iter_found2)
      {
        gchar *str_data2=NULL;
        gtk_tree_model_get(model2, &tree_iter2, 0, &str_data2, -1);
        g_ptr_array_add(g_column_labels, g_strdup(str_data2)); 
        iter_found2=gtk_tree_model_iter_next(model2, &tree_iter2);
        if(str_data2!=NULL) g_free(str_data2);
      }
  }
static void change_standard_labels(GtkWidget *widget, GtkWidget *cs[])
  {
    gint combo_id=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(widget)));
    gint i=0;
    if(combo_id==1)
      {
        //Row labels.
        GtkTreeIter tree_iter1;
        GtkTreeModel *model1=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[0]));
        gtk_tree_model_get_iter_first(model1, &tree_iter1);
        gint rows1=1;
        while(gtk_tree_model_iter_next(model1, &tree_iter1)) rows1++;
        g_signal_handler_block((gpointer)cs[0], row_combo_block);
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(cs[0]));
        for(i=0;i<rows1;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf("row %i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cs[0]), id, label);
            g_free(id);
            g_free(label);
          }
        g_signal_handler_unblock((gpointer)cs[0], row_combo_block);
        gtk_combo_box_set_active(GTK_COMBO_BOX(cs[0]), 0);
        //Column labels.
        GtkTreeIter tree_iter2;
        GtkTreeModel *model2=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[1]));
        gtk_tree_model_get_iter_first(model2, &tree_iter2);
        gint rows2=1;
        while(gtk_tree_model_iter_next(model2, &tree_iter2)) rows2++;
        g_signal_handler_block((gpointer)cs[1], column_combo_block);
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(cs[1]));
        for(i=0;i<rows2;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf("column %i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cs[1]), id, label);
            g_free(id);
            g_free(label);
          }
        g_signal_handler_unblock((gpointer)cs[1], column_combo_block);
        gtk_combo_box_set_active(GTK_COMBO_BOX(cs[1]), 0);
      }
    if(combo_id==2)
      {
        //Row labels.
        GtkTreeIter tree_iter1;
        GtkTreeModel *model1=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[0]));
        gtk_tree_model_get_iter_first(model1, &tree_iter1);
        gint rows1=1;
        while(gtk_tree_model_iter_next(model1, &tree_iter1)) rows1++;
        g_signal_handler_block((gpointer)cs[0], row_combo_block);
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(cs[0]));
        for(i=0;i<rows1;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf(" %i ", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cs[0]), id, label);
            g_free(id);
            g_free(label);
          }
        g_signal_handler_unblock((gpointer)cs[0], row_combo_block);
        gtk_combo_box_set_active(GTK_COMBO_BOX(cs[0]), 0);
        //Column labels.
        GtkTreeIter tree_iter2;
        GtkTreeModel *model2=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[1]));
        gtk_tree_model_get_iter_first(model2, &tree_iter2);
        gint rows2=1;
        while(gtk_tree_model_iter_next(model2, &tree_iter2)) rows2++;
        g_signal_handler_block((gpointer)cs[1], column_combo_block);
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(cs[1]));
        for(i=0;i<rows2;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf("%i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cs[1]), id, label);
            g_free(id);
            g_free(label);
          }
        g_signal_handler_unblock((gpointer)cs[1], column_combo_block);
        gtk_combo_box_set_active(GTK_COMBO_BOX(cs[1]), 0);
      }
    if(combo_id==3)
      {
        //Row labels.
        GtkTreeIter tree_iter1;
        GtkTreeModel *model1=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[0]));
        gtk_tree_model_get_iter_first(model1, &tree_iter1);
        gint rows1=1;
        while(gtk_tree_model_iter_next(model1, &tree_iter1)) rows1++;
        GPtrArray *micro_labels=g_ptr_array_new_full(10, g_free);
        get_letters(micro_labels, rows1);
        g_signal_handler_block((gpointer)cs[0], row_combo_block);
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(cs[0]));
        for(i=0;i<rows1;i++)
          {
            gchar *id=g_strdup_printf("%i", i);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cs[0]), id, (char*)g_ptr_array_index(micro_labels, i));
            g_free(id);
          }
        g_ptr_array_free(micro_labels, TRUE);
        g_signal_handler_unblock((gpointer)cs[0], row_combo_block);
        gtk_combo_box_set_active(GTK_COMBO_BOX(cs[0]), 0);
        //Column labels.
        GtkTreeIter tree_iter2;
        GtkTreeModel *model2=gtk_combo_box_get_model(GTK_COMBO_BOX(cs[1]));
        gtk_tree_model_get_iter_first(model2, &tree_iter2);
        gint rows2=1;
        while(gtk_tree_model_iter_next(model2, &tree_iter2)) rows2++;
        g_signal_handler_block((gpointer)cs[1], column_combo_block);
        gtk_combo_box_text_remove_all(GTK_COMBO_BOX_TEXT(cs[1]));
        for(i=0;i<rows2;i++)
          {
            gchar *id=g_strdup_printf("%i", i);
            gchar *label=g_strdup_printf("%i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(cs[1]), id, label);
            g_free(id);
            g_free(label);
          }
        g_signal_handler_unblock((gpointer)cs[1], column_combo_block);

        gtk_combo_box_set_active(GTK_COMBO_BOX(cs[1]), 0);
      }
  }
static void get_letters(GPtrArray *micro_labels, gint rows)
  {
     char letters[] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
    int i=0;
    int j=0;
    int k=0;
    int counter=0;
    
    if(rows<=26)
      {
        for(i=0;i<rows;i++)
          {
            gchar *str_data=g_strdup_printf(" %c ", letters[i]); 
            g_ptr_array_add(micro_labels, g_strdup(str_data)); 
            if(str_data!=NULL) g_free(str_data);
          }
      }
    if(rows>26&&rows<=376)
      {
        for(i=0;i<26;i++)
          {
            gchar *str_data=g_strdup_printf(" %c ", letters[i]); 
            g_ptr_array_add(micro_labels, g_strdup(str_data)); 
            if(str_data!=NULL) g_free(str_data);
            counter++;
          }
        for(i=0;i<26;i++)
          {
            for(j=0;j<26-i;j++)
              {
                gchar *str_data=g_strdup_printf(" %c%c ", letters[i], letters[j+i]); 
                g_ptr_array_add(micro_labels, g_strdup(str_data)); 
                if(str_data!=NULL) g_free(str_data);
                if(counter==rows) break;
                counter++;
              }
            if(counter==rows) break;
          }
      }
    if(rows>376)
      {
        for(i=0;i<26;i++)
          {
            printf(" %i.%c", counter, letters[i]);
            gchar *str_data=g_strdup_printf(" %c ", letters[i]); 
            g_ptr_array_add(micro_labels, g_strdup(str_data)); 
            if(str_data!=NULL) g_free(str_data);
            counter++;
          }
        for(i=0;i<26;i++)
          {
            for(j=0;j<26-i;j++)
              {
                gchar *str_data=g_strdup_printf(" %c%c ", letters[i], letters[j+i]); 
                g_ptr_array_add(micro_labels, g_strdup(str_data)); 
                if(str_data!=NULL) g_free(str_data);
                counter++;
              }
          }
        for(i=0;i<26;i++)
          {
            for(j=0;j<26-i;j++)
              {
                for(k=0;k<26-i-j;k++)
                  {
                    gchar *str_data=g_strdup_printf(" %c%c%c ", letters[i], letters[j+i], letters[j+i+k]); 
                    g_ptr_array_add(micro_labels, g_strdup(str_data)); 
                    if(str_data!=NULL) g_free(str_data);
                    if(counter==rows) break;
                    counter++;
                  }
                if(counter==rows) break;
              }
            if(counter==rows) break;
          }
      } 
  }
static void table_labels_dialog(GtkWidget *widget, GtkWidget *ws[])
  {
    gtk_button_set_label(GTK_BUTTON(widget), "Set Table Labels");
    gint tables=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
    gint i=0;
    if(tables>0&&tables<21)
      {
        GtkWidget *dialog=gtk_dialog_new_with_buttons("Set Table Labels", GTK_WINDOW(ws[21]), GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, "OK", GTK_RESPONSE_OK, NULL);
        gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
        GtkWidget *table_label=gtk_label_new("Get Labels from SQL");
        GtkWidget *sql_entry=gtk_entry_new();
        gtk_entry_set_text(GTK_ENTRY(sql_entry), "SELECT percent FROM data");
        GtkWidget *button1=gtk_button_new_with_label("Get SQL Labels");
        GtkWidget *table_combo=gtk_combo_box_text_new_with_entry();
        for(i=0;i<tables;i++)
          {
            gchar *label=g_strdup_printf("%i", i+1);  
            gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(table_combo), label, label);
            g_free(label);
          }
        gtk_combo_box_set_entry_text_column(GTK_COMBO_BOX(table_combo), 0);
        gtk_combo_box_set_active(GTK_COMBO_BOX(table_combo), 0);
        gint active_row=0;
        table_combo_block=g_signal_connect(GTK_COMBO_BOX(table_combo), "changed", G_CALLBACK(table_combo_changed), &active_row);

        GtkWidget *focus_button=gtk_dialog_get_widget_for_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK); 
        gtk_widget_grab_focus(focus_button);
        g_signal_connect(focus_button, "clicked", G_CALLBACK(load_table_labels), table_combo);

        GtkWidget *grid=gtk_grid_new();
        gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
        gtk_grid_attach(GTK_GRID(grid), table_label, 1, 0, 2, 1);
        gtk_grid_attach(GTK_GRID(grid), sql_entry, 0, 1, 4, 1);
        gtk_grid_attach(GTK_GRID(grid), button1, 1, 2, 2, 1);
        gtk_grid_attach(GTK_GRID(grid), table_combo, 0, 3, 4, 1);

        GtkWidget *content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
        gtk_container_add(GTK_CONTAINER(content_area), grid);
        gtk_widget_show_all(dialog);

        gint result=gtk_dialog_run(GTK_DIALOG(dialog));
        if(result==GTK_RESPONSE_OK)
          {
            g_print("Table Label Values\n");
            gint length=g_table_labels->len;
            for(i=0;i<length;i++) g_print("%s ", (char*)g_ptr_array_index(g_table_labels, i));
            g_print("\n");                 
          }
        gtk_widget_destroy(dialog);           
      }
    else
      {
        gchar *message=g_strdup_printf("Tables %s, Range 1<=Tables<=20", gtk_entry_get_text(GTK_ENTRY(ws[7]))); 
        message_dialog(message);
        g_free(message);
      }
  }
static void activate_table_labels_button(GtkWidget *widget, gpointer data)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widget)))
      {
        gtk_widget_set_sensitive(GTK_WIDGET(data), TRUE);
      }
    else
      {
        gtk_widget_set_sensitive(GTK_WIDGET(data), FALSE);
      }
  }
static void table_combo_changed(GtkWidget *widget, gpointer data)
  {
    gint active_row=*(gint*)data;
    gchar *text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(widget));
    gint text_id=gtk_combo_box_get_active(GTK_COMBO_BOX(widget));
    if(text_id!=-1)
      {
        active_row=text_id;
        *(gint*)data=active_row;
      }
    if(text!=NULL)
      {
        g_signal_handler_block((gpointer)widget, table_combo_block);
        gtk_combo_box_text_insert_text(GTK_COMBO_BOX_TEXT(widget), active_row, text);
        gtk_combo_box_text_remove(GTK_COMBO_BOX_TEXT(widget), active_row+1);
        g_signal_handler_unblock((gpointer)widget, table_combo_block);
      }
    if(text!=NULL) g_free(text);
  }
static void load_table_labels(GtkWidget *widget, gpointer data)
  {
    GtkTreeIter tree_iter;
    GtkTreeModel *model=gtk_combo_box_get_model(GTK_COMBO_BOX(data));
    gboolean iter_found=gtk_tree_model_get_iter_first(model, &tree_iter);
    gint i=0;
    gint array_length=g_table_labels->len;
    for(i=0;i<array_length; i++)
      {
        g_ptr_array_remove_index_fast(g_table_labels, 0);
      }
    while(iter_found)
      {
        gchar *str_data=NULL;
        gtk_tree_model_get(model, &tree_iter, 0, &str_data, -1);
        g_ptr_array_add(g_table_labels, g_strdup(str_data)); 
        iter_found=gtk_tree_model_iter_next(model, &tree_iter);
        if(str_data!=NULL) g_free(str_data);
      }
  }
static gboolean draw_report(GtkWidget *da, cairo_t *cr, GtkWidget *ws[])
  {
    if(da_blocking==FALSE)
      {
        g_signal_handler_block((gpointer)da, da_block);
        da_blocking=TRUE;
      }
    if(da_blocking==TRUE)
      {
        g_signal_handler_unblock((gpointer)da, da_block);
        da_blocking=FALSE;
      }
  
    if(drawing_data_valid) drawing_area_preview(da, cr, ws);
   
    return TRUE;
  }
static void start_draw_report(GtkNotebook *notebook, GtkWidget *page, guint page_num, GtkWidget *ws[])
  {
    //g_print("Start Draw %i\n", page_num);
    gint i=0;
    gint combo4_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[14])));
    gint return_value=0;
    if(page_num==1) return_value=validate_entries(ws);
    if(return_value==0) drawing_data_valid=TRUE;
    else drawing_data_valid=FALSE;
    //Don't unblock on first call.
    static guint block_lag=0;

    if(page_num==1)
      {
        g_signal_handler_block((gpointer)ws[0], row_clear_block);
        g_signal_handler_block((gpointer)ws[1], column_clear_block);
        block_lag++;
      }
    if(page_num==0&&block_lag>0)
      {
        g_signal_handler_unblock((gpointer)ws[0], row_clear_block);
        g_signal_handler_unblock((gpointer)ws[1], column_clear_block);
      }
    if(da_blocking)
      {
        g_print("Unblock\n");
        g_signal_handler_unblock((gpointer)ws[23], da_block);
        da_blocking=FALSE;
      }
    if(page_num==1&&return_value==0)
      {
        GtkAdjustment *adj1=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(ws[24]));
        GtkAdjustment *adj2=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(ws[24]));
        gtk_adjustment_set_value(adj1, 0);
        gtk_adjustment_set_value(adj2, 0);
        gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
        gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])));
        gint column_width=atoi(gtk_entry_get_text(GTK_ENTRY(ws[4])));
        gint shift_number_left=atoi(gtk_entry_get_text(GTK_ENTRY(ws[5])));
        gint shift_column_left=atoi(gtk_entry_get_text(GTK_ENTRY(ws[6])));
        gint tables=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
        gchar *sql_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(ws[9])));
        gint round_float=atoi(gtk_entry_get_text(GTK_ENTRY(ws[10])));
        //Clear data arrays.
        gint array_length1=g_data_values->len;
        for(i=0;i<array_length1; i++) g_ptr_array_remove_index_fast(g_data_values, 0);
        gint array_length2=g_min_max->len;
        for(i=0;i<array_length2;i++) g_array_remove_index_fast(g_min_max, 0);       
        //Get the data.
        if(combo4_index==1)
          {
            get_test_data1(rows, columns, tables, column_width, shift_number_left, round_float);
          }
        if(combo4_index==2)
          {
            get_test_data2(rows, columns, tables, column_width, shift_number_left);
          }
        if(combo4_index==3)
          {
            get_test_data3(rows, columns, tables, column_width, shift_number_left);
          }
        if(combo4_index==4)
          {
            get_db_data_for_crosstab(rows, columns, tables, column_width, shift_number_left, round_float, sql_string);
          }
        else
          {
            get_db_data_for_table(rows, columns, tables, column_width, shift_number_left, round_float, sql_string);
          }
        //Get the labels. Shift them or truncate them to fit in the rectangles.
        get_labels_for_drawing(tables, rows, columns, column_width, shift_column_left);
        if(sql_string!=NULL) g_free(sql_string);
      }
  }
static void drawing_area_preview(GtkWidget *da, cairo_t *cr, GtkWidget *ws[])
  {
    gint i=0;
    plate_counter=1;
    plate_counter_sql=1;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
    gint count_lines=gtk_text_buffer_get_line_count(buffer);
    gint tables=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
    gchar *font_string=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ws[13]));
    font_string=g_strdup_printf("%s", font_string);
    PangoFontDescription *font_desc=pango_font_description_from_string(font_string); 
    PangoContext *pango_context=gtk_widget_get_pango_context(da);
    PangoLayout *pango_layout=pango_layout_new(pango_context);
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);
    pango_layout_set_width(pango_layout, PANGO_SCALE*width);
    pango_layout_set_height(pango_layout, PANGO_SCALE*height);
    pango_layout_set_font_description(pango_layout, font_desc);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_set_source_rgb(cr, 0, 0, 0);

    // Get the markup string.
    GString *markup=g_string_new("");
    get_pango_markup(ws, markup);

    GString *string=g_string_new("");
    GString *table_string=g_string_new(markup->str);
    for(i=0;i<tables;i++)
      {
        get_table_string(string, ws, 0, i, count_lines);
        draw_tables(pango_layout, cr, ws, 0, i, count_lines);
        g_string_append_printf(table_string, "%s", string->str);
        g_string_truncate(string, 0);        
      }

    cairo_set_source_rgb(cr, 0, 0, 0);
    pango_layout_set_markup(pango_layout, table_string->str, -1);
    pango_cairo_show_layout(cr, pango_layout);

    g_object_unref(pango_layout);
    g_string_free(string, TRUE);
    g_string_free(markup, TRUE);
    g_string_free(table_string, TRUE);
    if(font_string!=NULL) g_free(font_string);
  }
static void get_table_string(GString *string, GtkWidget *ws[], gint page_number, gint table, gint count_lines)
  {
    gint i=0;
    gint j=0;
    gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
    gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])));
    gint shift_margin=atoi(gtk_entry_get_text(GTK_ENTRY(ws[2])));
    gint shift_below_text=atoi(gtk_entry_get_text(GTK_ENTRY(ws[3])));
    gchar *table_name=g_strdup(gtk_entry_get_text(GTK_ENTRY(ws[8])));
    gboolean check1_active=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws[18]));
    gint combo2_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[12])));
    gboolean show_numbers = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws[19]));
    gint shift_index=(table)*(rows*columns);

    //Space between tables.
    if(shift_below_text>1)
      {
        gchar *shift_below_str=g_strnfill(shift_below_text-1, '\n');
        g_string_append_printf(string, "%s\n", shift_below_str);
        g_free(shift_below_str);
      }
  
    //Padding for moving the drawing to the right.
    gchar *shift_margin_str=g_strnfill(shift_margin, ' ');

    //Add table label if needed.
    if(check1_active) g_string_append_printf(string, "%s%s", shift_margin_str, table_name);
    if(check1_active&&g_table_labels->len!=0) g_string_append_printf(string, "%s\n", (char*)g_ptr_array_index(g_table_labels, table-1));
    if(check1_active&&g_table_labels->len==0) g_string_append_printf(string, "%i\n", table+1);
    

    //Shift column labels by row label width.
    if(combo2_index==2) g_string_append_printf(string, "%s", shift_margin_str);
    if(combo2_index==3)
      {
        gint row_label_length=strlen((char*)g_ptr_array_index(g_row_labels, 0));
        gchar *shift_column_labels=g_strnfill(row_label_length, ' ');
        if(combo2_index==3) g_string_append_printf(string, "%s%s", shift_margin_str, shift_column_labels);
        g_free(shift_column_labels);
      }

    //Get string for column labels.
    if(combo2_index==2||combo2_index==3)
      {
        for(i=0;i<columns;i++)
          {
            g_string_append_printf(string, "%s", (char*)g_ptr_array_index(g_column_labels, i));
          }
        g_string_append_printf(string, "%s", "\n");
      }

    //Get string for row labels and table.
    for(i=0;i<rows;i++)
      {
        if(combo2_index==3) g_string_append_printf(string, "%s%s", shift_margin_str, (char*)g_ptr_array_index(g_row_labels, i));
        else g_string_append_printf(string, "%s", shift_margin_str);
        if(!show_numbers) g_string_append_printf(string, "%s", "\n");
        if(show_numbers)
          {
            for(j=0;j<columns;j++)
              {
                g_string_append_printf(string, "%s", (char*)g_ptr_array_index(g_data_values, (i*columns+j)+shift_index)); 
              }
            g_string_append_printf(string, "%s", "\n");
          }
      }

    if(shift_margin_str!=NULL) g_free(shift_margin_str);
    if(table_name!=NULL) g_free(table_name);
  }
static void draw_tables(PangoLayout *pango_layout, cairo_t *cr, GtkWidget *ws[], gint page_number, gint table, gint count_lines)
  {
    gint i=0;
    gint j=0;
    gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
    gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])));
    gint shift_margin=atoi(gtk_entry_get_text(GTK_ENTRY(ws[2])));
    gint shift_below_text=atoi(gtk_entry_get_text(GTK_ENTRY(ws[3])));
    gint column_width=atoi(gtk_entry_get_text(GTK_ENTRY(ws[4])));
    gint combo1_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[11])));
    gint combo2_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[12])));
    gboolean check1_active=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(ws[18]));
    //shift_index getting heatmap color values.
    gint shift_index=(table)*(rows*columns);
    gint shift_below_text2=0;
    gint markup_difference=0;
    gint table_title_shift=1;
    
    //If printing set the drawing to table_print.
    if(page_number>0) table=table_print;
    //Shift tables for multiple tables.
    if(check1_active) table_title_shift=2;
    if(combo2_index==1) table_title_shift=0;
    if(combo2_index==1&&check1_active) table_title_shift=1;
    if(page_number==0) shift_below_text2=(count_lines+table_title_shift)+shift_below_text+(table*(rows+shift_below_text+table_title_shift));
    if(page_number>0&&combo2_index==1)
      {
        shift_below_text2=shift_below_text+(table*(rows+shift_below_text+table_title_shift));
      }
    if(page_number>0&&combo2_index!=1)
      {
        shift_below_text2=shift_below_text+(table*(rows+shift_below_text+table_title_shift));
      }
    //g_print("shift below %i table title %i\n", shift_below_text2, table_title_shift);
    //Global variable for reseting drawings on print page.
    table_print++;

    //Get size difference in text due to font tags on page 1.
    if(page_number==0)
      {
        GtkTextIter start, end;
        GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        gchar *text_buffer=gtk_text_buffer_get_text(buffer, &start, &end, TRUE);
        pango_layout_set_markup(pango_layout, text_buffer, -1);
        PangoRectangle rectangle_ink1;
        PangoRectangle rectangle_log1;
        pango_layout_get_extents(pango_layout, &rectangle_ink1, &rectangle_log1);
        // Get the markup string.
        GString *markup=g_string_new("");
        get_pango_markup(ws, markup);
        pango_layout_set_markup(pango_layout, markup->str, -1);
        g_string_free(markup, TRUE);
        PangoRectangle rectangle_ink2;
        PangoRectangle rectangle_log2;
        pango_layout_get_extents(pango_layout, &rectangle_ink2, &rectangle_log2);
        if(rectangle_log2.height-rectangle_log1.height>0)
          {
            markup_difference=rectangle_log2.height-rectangle_log1.height;
          }
        else markup_difference=0; 
      }
    else markup_difference=0;

    //Get rectangle for one monospace char for sizing.
    pango_layout_set_markup(pango_layout, "5", -1);
    PangoRectangle rectangle_ink;
    PangoRectangle rectangle_log;
    pango_layout_get_extents(pango_layout, &rectangle_ink, &rectangle_log);
    //g_print("Width %i Height %i", rectangle_ink.width, rectangle_ink.height);

    //Draw vertical label rectangle for crosstabs.
    gint top=0;
    if(page_number==0) top=shift_below_text2-2;
    else if(combo2_index==1&&page_number>0) top=shift_below_text2-1;
    else top=shift_below_text2-(1-table_title_shift);
    if(table_title_shift>0&&page_number>0&&combo2_index==1) top=top+table_title_shift;  
    gint bottom = top + rows;
    //g_print("top %i bottom %i shift2 %i\n", top, bottom, shift_below_text2);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    gint max_vertical_label=strlen((char*)g_ptr_array_index(g_row_labels, 0));
    if(combo2_index==3)
      {
        cairo_rectangle(cr, ((shift_margin) * rectangle_log.width)/PANGO_SCALE, (rectangle_log.height * (top) + markup_difference)/PANGO_SCALE, (rectangle_log.width/PANGO_SCALE)*(max_vertical_label+.10), (rectangle_log.height*(rows+1))/PANGO_SCALE);
        cairo_fill(cr);
        cairo_stroke(cr);
        //Draw lines for rectangle.
        //Vertical left.
        //cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, ((shift_margin)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height* (top) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((shift_margin)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height*(bottom+1) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr);
        //Vertical right is drawn with grid.
        //Short top horizontal.
        cairo_move_to(cr, ((shift_margin)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height* (top) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((shift_margin+max_vertical_label)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height*(top) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr);  
        //Short bottom horizontal
        cairo_move_to(cr, ((shift_margin)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height* (bottom+1) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((shift_margin+max_vertical_label)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height*(bottom+1) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr);
      } 

    //Draw horizontal label rectangle for both crosstab and tabular data.
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    if(combo2_index==1||combo2_index==2) max_vertical_label=0;
    if(combo2_index==2||combo2_index==3)
      {
        cairo_rectangle(cr, ((shift_margin+max_vertical_label) * rectangle_log.width)/PANGO_SCALE, (rectangle_log.height * (top) + markup_difference)/PANGO_SCALE, ((rectangle_log.width*columns*column_width)/PANGO_SCALE), (rectangle_log.height)/PANGO_SCALE);
        cairo_fill(cr);
        cairo_stroke(cr);
        //Draw lines for rectangle.
        cairo_set_line_width(cr, 2);
        //Top horizontal
        //cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
        cairo_move_to(cr, ((shift_margin+max_vertical_label)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height* (top) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((shift_margin+max_vertical_label+(column_width*columns)) *rectangle_log.width)/PANGO_SCALE, (rectangle_log.height*(top) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr); 
        //Short top vertical left.
        cairo_move_to(cr, ((shift_margin+max_vertical_label)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height* (top) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((shift_margin+max_vertical_label)*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height*(top+1) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr); 
        //Short top vertical right.
        cairo_move_to(cr, ((shift_margin+max_vertical_label + (column_width*columns))*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height * (top) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((shift_margin+max_vertical_label+(column_width*columns))*rectangle_log.width)/PANGO_SCALE, (rectangle_log.height*(top+1) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr); 
      }

    //Background color for each cell.
    gdouble red=0;
    gdouble green=0;
    gdouble blue=0;
    gdouble min=g_array_index(g_min_max, gdouble, 2*table);
    gdouble max=g_array_index(g_min_max, gdouble, (2*table)+1);
    //g_print("%i min %f max %f\n", table, min, max);
    gdouble data_value=0;
    gint counter=0;

    if(page_number==0) top=shift_below_text2-1;
    else if(combo2_index==1&&page_number>0) top=shift_below_text2;
    else top=shift_below_text2+table_title_shift;
    if(table_title_shift>0&&page_number>0&&combo2_index==1) top=top+table_title_shift;   
    bottom=top+rows;

    shift_margin=shift_margin+max_vertical_label;
    if(combo1_index!=1)
      {
        for(i=0;i<rows;i++)
          {
            for(j=0;j<columns+1;j++)
              {
                if(combo1_index==2)
                  {
                    if(i%2) cairo_set_source_rgb(cr, 0.5, 0.7, 1.0);
                    else cairo_set_source_rgb(cr, 0.7, 1.0, 1.0);
                  }
                else
                  {
                    //Correct for rectangles that don't fill. Should be a better way to do this.
                    //Extend width by 2. This leaves a fragment on the bottom right that needs 
                    //to be corrected after the grid is drawn.
                    if(j!=columns)
                      {
                        if(combo1_index==3)
                          { 
                            data_value=g_ascii_strtod((char*)g_ptr_array_index(g_data_values, counter+shift_index), NULL);
                            //g_print("%i %s %f\n", counter+shift_index, (char*)g_ptr_array_index(g_data_values, counter+shift_index), data_value); 
                            counter++;
                            heatmap_value_rgb(data_value, min, max, &red, &green, &blue);
                          }
                        else if(combo1_index==4)
                          {
                            data_value=g_ascii_strtod((char*)g_ptr_array_index(g_data_values, counter+shift_index), NULL);
                            
                            counter++;
                            heatmap_value_bry(data_value, min, max, &red, &green, &blue);
                          }
                        else
                          {
                            data_value=g_ascii_strtod((char*)g_ptr_array_index(g_data_values, counter+shift_index), NULL);
                            counter++;
                            heatmap_value_iris(data_value, min, max, &red, &green, &blue); 
                          }
                        //g_print("r %f g %f b %f\n", red, green, blue);
                        cairo_set_source_rgb(cr, red, green, blue);
                      }
                  } 
                if(j!=columns)
                  {
                    cairo_rectangle(cr, ((shift_margin +(column_width*j)) * rectangle_log.width)/PANGO_SCALE, (rectangle_log.height * (top + i) + markup_difference)/PANGO_SCALE, (rectangle_log.width/PANGO_SCALE)*(column_width+2), (rectangle_log.height)/PANGO_SCALE);
                    cairo_fill(cr);
                    cairo_stroke(cr);
                  }
                else
                  {
                    //White out after end of table.
                    if(i!=0)
                      {
                        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0); 
                        cairo_rectangle(cr, ((shift_margin +(column_width*j)) * rectangle_log.width)/PANGO_SCALE, (rectangle_log.height * (top + i - 1) + markup_difference)/PANGO_SCALE, (rectangle_log.width/PANGO_SCALE)*column_width, (rectangle_log.height*2)/PANGO_SCALE);
                        cairo_fill(cr);
                        cairo_stroke(cr);
                      }
                  }
              }
          }    
      }

    //Table grid for test numbers.
    gint total_chars=column_width*columns;
    gdouble left_margin=(shift_margin*rectangle_log.width)/PANGO_SCALE;
    //First draw over fragment left by overlapping color backgrounds.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_line_width(cr, 2);
    cairo_move_to(cr, ((rectangle_log.width * total_chars)/PANGO_SCALE) + left_margin, (rectangle_log.height  *(top + rows) + markup_difference)/PANGO_SCALE);
    cairo_line_to(cr, ((rectangle_log.width * (total_chars+3))/PANGO_SCALE) + left_margin, (rectangle_log.height  *(top + rows) + markup_difference)/PANGO_SCALE);
    cairo_stroke(cr); 
    cairo_move_to(cr, ((rectangle_log.width * total_chars)/PANGO_SCALE) + left_margin, (rectangle_log.height  *(top) + markup_difference)/PANGO_SCALE);
    cairo_line_to(cr, ((rectangle_log.width * (total_chars+3))/PANGO_SCALE) + left_margin, (rectangle_log.height  *(top) + markup_difference)/PANGO_SCALE);
    cairo_stroke(cr); 
    //Draw grid.
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_SQUARE);
    cairo_set_line_width(cr, 2);
    cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
    for(i=0;i<rows+1;i++)
      { 
        cairo_move_to(cr, left_margin, (rectangle_log.height * (top + i) + markup_difference)/PANGO_SCALE);
        cairo_line_to(cr, ((rectangle_log.width * total_chars)/PANGO_SCALE) + left_margin, (rectangle_log.height  *(top + i) + markup_difference)/PANGO_SCALE);
        cairo_stroke(cr); 
        for(j=0;j<columns+1;j++)
          { 
            cairo_move_to(cr, ((rectangle_log.width * j * column_width)/PANGO_SCALE) + left_margin, (rectangle_log.height*top + markup_difference)/PANGO_SCALE);
            cairo_line_to(cr, ((rectangle_log.width * j * column_width)/PANGO_SCALE) + left_margin, (rectangle_log.height*bottom + markup_difference)/PANGO_SCALE);
            cairo_stroke(cr);
          }
      } 
  }
static void heatmap_value_rgb(gdouble data_value, gdouble min, double max, gdouble *red, gdouble *green, gdouble *blue)
  {
    gdouble percent=((data_value-min)/(max-min));
    //g_print("Percent %f\n", percent);
    if(percent>0.75)
      {
        *red=1.0; 
        *green=0.0+(4*(1-percent));
        *blue = 0.0;
      }
    else if(percent<=0.75&&percent>0.50)
      {
        *red=1.0-(4*(0.75-percent));
        *green=1.0;
        *blue=0.0;
      }
    else if(percent<=0.50&&percent>0.25)
      {
        *red=0.0; 
        *green=1.0;
        *blue=0.0+(4*(0.5-percent));
      }
    else
      {
        *red=0.0;
        *green=1.0-(4*(0.25-percent));
        *blue=1.0;
      }  
  }
static void heatmap_value_bry(gdouble data_value, gdouble min, double max, gdouble *red, gdouble *green, gdouble *blue)
  {
    gdouble percent=((data_value-min)/(max-min));
    if(percent>0.50)
      {
        *red=1.0; 
        *green=1.0-(2*(1-percent));
        *blue=0.0;
      }
    else
      {
        *red=1.0-(2*(0.50-percent));
        *green=0.0;
        *blue=0.0+(2*(0.50-percent));
      }  
  }
static void heatmap_value_iris(gdouble data_value, gdouble min, double max, gdouble *red, gdouble *green, gdouble *blue)
  {
    gdouble percent=((data_value-min)/(max-min));
    *red=1.0; 
    *green=1.0-(1-percent);
    *blue=0.0+(1-percent); 
  }
static void get_test_data1(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left, gint round_float)
  {
    g_print("Get Test Data1\n");
    gint i=0;
    gint j=0;
    gint k=0;
    gint l=0;
    gint fill_temp=0;
    gdouble random_number=0;
    gint counter=-1;
    gboolean found_decimal=FALSE;
    gint buffer_len=column_width-shift_number_left;
    gchar buffer1[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buffer2[buffer_len+1];
    gdouble min=G_MAXDOUBLE;
    gdouble max=-G_MAXDOUBLE;
    GRand *rand1=g_rand_new();
    gint length=0;
    gchar *fill_end=NULL;
    if(shift_number_left>0) fill_end=g_strnfill(shift_number_left, ' ');
    for(i=0;i<tables;i++)
      {
        for(j=0;j<rows;j++)
          {
            for(k=0;k<columns;k++)
              {
                random_number=round1(100*g_rand_double(rand1), round_float);
                g_ascii_formatd(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%f", random_number);
                length=strlen(buffer1);
                counter=-1;
                found_decimal=FALSE;
                for(l=0;l<buffer_len+1;l++) 
                  {
                    if(buffer1[l]=='.') found_decimal=TRUE;
                    if(counter==round_float)
                      {
                        buffer2[l]='\0';
                        break;
                      }
                    buffer2[l]=buffer1[l];
                    if(found_decimal) counter++;
                  }
                length=strlen(buffer2);
                fill_temp=column_width-length-shift_number_left;
                gchar *fill_start=NULL;
                if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
                if(fill_temp>0&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s%s", fill_start, buffer2, fill_end));
                    //g_print("1|%s|%s%s| %i fill %i\n", fill_start, buffer, fill_end, strlen(buffer), fill_temp);
                  }
                else if(fill_temp<1&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", buffer2, fill_end));
                    //g_print("2|%s%s| %i fill %i\n", buffer, fill_end, strlen(buffer), fill_temp);
                  }
                else if(fill_temp>0&&shift_number_left==0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", fill_start, buffer2));
                    //g_print("3|%s%s| %i fill %i\n", fill_start, buffer, strlen(buffer), fill_temp);
                  }
                else
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s", buffer2));
                    //g_print("4|%s| %i fill %i\n", buffer, strlen(buffer), fill_temp);
                  }
                if(random_number<min) min=random_number;
                if(random_number>max) max=random_number;
                if(fill_start!=NULL) g_free(fill_start);
              }
          }
        g_array_append_val(g_min_max, min);
        g_array_append_val(g_min_max, max);
        //g_print("min %f max %f\n", min, max);
        min=G_MAXDOUBLE;
        max=-G_MAXDOUBLE;
      }
    
    if(fill_end!=NULL) g_free(fill_end);
    g_rand_free(rand1);
  }
static void get_test_data2(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left)
  {
    g_print("Get Test Data2\n");
    gint i=0;
    gint j=0;
    gint k=0;
    gint fill_temp=0;
    gdouble min=G_MAXDOUBLE;
    gdouble max=-G_MAXDOUBLE;
    gint counter=1;
    gint length=0;
    gchar *fill_end=NULL;
    if(shift_number_left>0) fill_end=g_strnfill(shift_number_left, ' ');
    for(i=0;i<tables;i++)
      {
        for(j=0;j<rows;j++)
          {
            for(k=0;k<columns;k++)
              {
                gchar *string=g_strdup_printf("%i", counter);
                counter++;
                length=strlen(string);
                fill_temp=column_width-length-shift_number_left;
                gchar *fill_start=NULL;
                if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
                if(fill_temp>0&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s%s", fill_start, string, fill_end));
                  }
                else if(fill_temp<1&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", string, fill_end));
                  }
                else if(fill_temp>0&&shift_number_left==0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", fill_start, string));
                  }
                else
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s", string));
                  }
                if(counter<min) min=counter;
                if(counter>max) max=counter;
                if(fill_start!=NULL) g_free(fill_start);
                if(string!=NULL) g_free(string);
              }
          }
        g_array_append_val(g_min_max, min);
        g_array_append_val(g_min_max, max);
        //g_print("min %f max %f\n", min, max);
        min=G_MAXDOUBLE;
        max=-G_MAXDOUBLE;
      }
    
    if(fill_end!=NULL) g_free(fill_end);
  }
static void get_test_data3(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left)
  {
    g_print("Get Test Data3\n");
    gint i=0;
    gint j=0;
    gint k=0;
    gint fill_temp=0;
    gdouble min=G_MAXDOUBLE;
    gdouble max=-G_MAXDOUBLE;
    gint sequence_number=0;
    gint length=0;
    gchar *fill_end=NULL;
    if(shift_number_left>0) fill_end=g_strnfill(shift_number_left, ' ');
    for(i=0;i<tables;i++)
      {
        for(j=0;j<rows;j++)
          {
            for(k=0;k<columns;k++)
              {
                sequence_number=(rows*k+1)+j+(rows*columns*i);
                gchar *string=g_strdup_printf("%i", sequence_number);
                length=strlen(string);
                fill_temp=column_width-length-shift_number_left;
                gchar *fill_start=NULL;
                if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
                if(fill_temp>0&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s%s", fill_start, string, fill_end));
                  }
                else if(fill_temp<1&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", string, fill_end));
                  }
                else if(fill_temp>0&&shift_number_left==0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", fill_start, string));
                  }
                else
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s", string));
                  }
                if(sequence_number<min) min=sequence_number;
                if(sequence_number>max) max=sequence_number;
                if(fill_start!=NULL) g_free(fill_start);
                if(string!=NULL) g_free(string);
              }
          }
        g_array_append_val(g_min_max, min);
        g_array_append_val(g_min_max, max);
        //g_print("min %f max %f\n", min, max);
        min=G_MAXDOUBLE;
        max=-G_MAXDOUBLE;
      }
    
    if(fill_end!=NULL) g_free(fill_end);
  }
static void get_db_data_for_crosstab(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left, gint round_float, gchar *sql_string)
  {
    g_print("Get Test Data1\n");
    gint i=0;
    gint j=0;
    gint k=0;
    gint l=0;
    gint fill_temp=0;
    gdouble double_number=0;
    gint int_number=0;
    gint counter=-1;
    gboolean found_decimal=FALSE;
    gint buffer_len=column_width-shift_number_left;
    gchar buffer1[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buffer2[buffer_len+1];
    gdouble min=G_MAXDOUBLE;
    gdouble max=-G_MAXDOUBLE;
    gchar *select_string=g_strdup_printf("%s LIMIT %i;", sql_string, tables*rows*columns);
    gint length=0;
    gchar *fill_end=NULL;
    if(shift_number_left>0) fill_end=g_strnfill(shift_number_left, ' ');
  
    gint step=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_open(database_name, &cnn);
    sqlite3_prepare_v2(cnn, select_string, -1, &stmt1, 0);
    g_print("Column Type %s\n", sqlite3_column_decltype(stmt1, 0));
    gint column_type=0;
    
    for(i=0;i<tables;i++)
      {
        for(j=0;j<rows;j++)
          {
            for(k=0;k<columns;k++)
              {
                step=sqlite3_step(stmt1);
                column_type=sqlite3_column_type(stmt1, 0);
                if(step==SQLITE_ROW)
                  {
                    if(column_type==SQLITE_INTEGER)
                      {
                        int_number=sqlite3_column_int(stmt1, 0);
                      }
                    else if(column_type==SQLITE_FLOAT)
                      {
                        double_number=round1(sqlite3_column_double(stmt1, 0), round_float);
                      }
                    else double_number=0;
                  }
                else double_number=0;
                if(column_type==SQLITE_INTEGER)
                  {
                    g_snprintf(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%i", int_number);
                  }
                else if(column_type==SQLITE_FLOAT)
                  {
                    g_ascii_formatd(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%f", double_number);
                  }
                else g_ascii_formatd(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%f", double_number);
                length=strlen(buffer1);
                counter=-1;
                found_decimal=FALSE;
                for(l=0;l<buffer_len+1;l++) 
                  {
                    if(buffer1[l]=='.') found_decimal=TRUE;
                    if(counter==round_float)
                      {
                        buffer2[l]='\0';
                        break;
                      }
                    buffer2[l]=buffer1[l];
                    if(found_decimal) counter++;
                  }
                length=strlen(buffer2);
                fill_temp=column_width-length-shift_number_left;
                gchar *fill_start=NULL;
                if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
                if(fill_temp>0&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s%s", fill_start, buffer2, fill_end));
                  }
                else if(fill_temp<1&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", buffer2, fill_end));
                  }
                else if(fill_temp>0&&shift_number_left==0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", fill_start, buffer2));
                  }
                else
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s", buffer2));
                  }
                if(column_type==SQLITE_INTEGER)
                  {
                    if(int_number<min) min=int_number;
                    if(int_number>max) max=int_number;
                  }
                if(column_type==SQLITE_FLOAT)
                  {
                    if(double_number<min) min=double_number;
                    if(double_number>max) max=double_number;
                  }
                if(fill_start!=NULL) g_free(fill_start);
              }
          }
        g_array_append_val(g_min_max, min);
        g_array_append_val(g_min_max, max);
        //g_print("min %f max %f\n", min, max);
        min=G_MAXDOUBLE;
        max=-G_MAXDOUBLE;
      }

    if(stmt1!=NULL) sqlite3_finalize(stmt1);
    sqlite3_close(cnn);
    if(select_string!=NULL) g_free(select_string);
    if(fill_end!=NULL) g_free(fill_end);
  }
static void get_db_data_for_table(gint rows, gint columns, gint tables, gint column_width, gint shift_number_left, gint round_float, gchar *sql_string)
  {
    g_print("Get Test Data1\n");
    gint i=0;
    gint j=0;
    gint k=0;
    gint l=0;
    gint fill_temp=0;
    gdouble double_number=0;
    gint int_number=0;
    gint counter=-1;
    gboolean found_decimal=FALSE;
    gint buffer_len=column_width-shift_number_left;
    gchar buffer1[G_ASCII_DTOSTR_BUF_SIZE];
    gchar buffer2[buffer_len+1];
    gdouble min=G_MAXDOUBLE;
    gdouble max=-G_MAXDOUBLE;
    gchar *select_string=g_strdup_printf("%s LIMIT %i;", sql_string, tables*rows*columns);
    gint length=0;
    gchar *fill_end=NULL;
    if(shift_number_left>0) fill_end=g_strnfill(shift_number_left, ' ');
  
    gint step=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_open(database_name, &cnn);
    sqlite3_prepare_v2(cnn, select_string, -1, &stmt1, 0);
    gint sql_columns=sqlite3_column_count(stmt1);
    gint column_type=0;
    gint array_length=g_column_labels->len;
    for(i=0;i<array_length; i++)
      {
        g_ptr_array_remove_index_fast(g_column_labels, 0);
      }
    for(i=0;i<sql_columns;i++)
      {
        g_ptr_array_add(g_column_labels, g_strdup(sqlite3_column_name(stmt1, i)));
      }
    
    for(i=0;i<tables;i++)
      {
        for(j=0;j<rows;j++)
          {
            step=sqlite3_step(stmt1);
            for(k=0;k<sql_columns;k++)
              {
                gchar *sql_text=NULL;
                if(step==SQLITE_ROW)
                  {
                    column_type=sqlite3_column_type(stmt1, k);
                    if(column_type==SQLITE_INTEGER)
                      {
                        int_number=sqlite3_column_int(stmt1, k);
                      }
                    else if(column_type==SQLITE_FLOAT)
                      {
                        double_number=round1(sqlite3_column_double(stmt1, k), round_float);
                      }
                    else if(column_type==SQLITE_TEXT)
                      {
                        sql_text=g_strdup((char*)sqlite3_column_text(stmt1, k));
                      }
                    else if(column_type==SQLITE_NULL) double_number=0;
                    else double_number=0;
                  }
                else double_number=0;
                if(column_type==SQLITE_INTEGER)
                  {
                    g_snprintf(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%i", int_number);
                  }
                else if(column_type==SQLITE_FLOAT)
                  {
                    g_ascii_formatd(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%f", double_number);
                  }
                else if(column_type==SQLITE_TEXT)
                  {
                    g_snprintf(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%s", sql_text);
                  }
                else g_ascii_formatd(buffer1, G_ASCII_DTOSTR_BUF_SIZE, "%f", double_number);
                length=strlen(buffer1);
                counter=-1;
                found_decimal=FALSE;
                for(l=0;l<buffer_len+1;l++) 
                  {
                    if(buffer1[l]=='.') found_decimal=TRUE;
                    if(counter==round_float)
                      {
                        buffer2[l]='\0';
                        break;
                      }
                    buffer2[l]=buffer1[l];
                    if(found_decimal) counter++;
                  }
                length=strlen(buffer2);
                fill_temp=column_width-length-shift_number_left;
                gchar *fill_start=NULL;
                if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
                if(fill_temp>0&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s%s", fill_start, buffer2, fill_end));
                  }
                else if(fill_temp<1&&shift_number_left>0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", buffer2, fill_end));
                  }
                else if(fill_temp>0&&shift_number_left==0)
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s%s", fill_start, buffer2));
                  }
                else
                  {
                    g_ptr_array_add(g_data_values, g_strdup_printf("%s", buffer2));
                  }
                if(double_number<min) min=double_number;
                if(double_number>max) max=double_number;
                if(fill_start!=NULL) g_free(fill_start);
                if(sql_text!=NULL) g_free(sql_text);
              }
          }
        g_array_append_val(g_min_max, min);
        g_array_append_val(g_min_max, max);
        //g_print("min %f max %f\n", min, max);
        min=G_MAXDOUBLE;
        max=-G_MAXDOUBLE;
      }

    if(stmt1!=NULL) sqlite3_finalize(stmt1);
    sqlite3_close(cnn);
    if(select_string!=NULL) g_free(select_string);
    if(fill_end!=NULL) g_free(fill_end);
  }
static void get_labels_for_drawing(gint tables, gint rows, gint columns, gint column_width, gint shift_column_left)
  {
    g_print("Edit Labels\n");
    gint i=0;
    gint array_length=0;
    gint fill_temp=0;
    gchar buffer[column_width+1];
    gchar *fill_end=g_strnfill(shift_column_left, ' ');
    gchar length=0;
    //If column width changes, remake default column labels.
    static gint pre_column_width=-1;
    static gint pre_shift_column=-1;
    if(pre_column_width==-1) pre_column_width=column_width;
    if(pre_shift_column==-1) pre_shift_column=shift_column_left;
    //Set row labels for drawing. Check validate_entries() for row limit.
    if(g_row_labels->len==0||g_row_labels->len!=rows||pre_column_width!=column_width)
      {
        array_length=g_row_labels->len;
        for(i=0;i<array_length; i++) g_ptr_array_remove_index_fast(g_row_labels, 0);
        if(rows<10)
          {
            for(i=0;i<rows;i++) g_ptr_array_add(g_row_labels, g_strdup_printf(" %i ", i+1));
          }
        if(rows>=10&&rows<100)
          {
            for(i=0;i<rows;i++)
              {
                if(i<9) g_ptr_array_add(g_row_labels, g_strdup_printf("  %i ", i+1));
                if(i>=9) g_ptr_array_add(g_row_labels, g_strdup_printf(" %i ", i+1));
              }
          } 
        if(rows>=100&&rows<1000)
          {
            for(i=0;i<rows;i++)
              {
                if(i<9) g_ptr_array_add(g_row_labels, g_strdup_printf("   %i ", i+1));
                if(i>=9&&i<99) g_ptr_array_add(g_row_labels, g_strdup_printf("  %i ", i+1));
                if(i>=99&&i<999) g_ptr_array_add(g_row_labels, g_strdup_printf(" %i ", i+1));
              }
          }             
      }
    //Need to square off labels from dialog also.
 
    //Set column labels for drawing. Need to correct padding.
    if(g_column_labels->len==0||g_column_labels->len!=columns||pre_column_width!=column_width||pre_shift_column!=shift_column_left)
      {
        array_length=g_column_labels->len;
        for(i=0;i<array_length; i++) g_ptr_array_remove_index_fast(g_column_labels, 0);
        for(i=0;i<columns;i++)
          {
            g_snprintf(buffer, column_width+1, "%s%i%s", "column", i+1, fill_end);
            length=strlen(buffer);
            fill_temp=column_width-length;
            gchar *fill_start=NULL;
            if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
            //g_print("1fill %i %i %i\n", fill_temp, shift_column_left, length);
            if(fill_temp>0) g_ptr_array_add(g_column_labels, g_strdup_printf("%s%s", fill_start, buffer));
            else g_ptr_array_add(g_column_labels, g_strdup_printf("%s", buffer));
            if(fill_start!=NULL) g_free(fill_start);
          }
      } 
    else
     {
        //Need to copy because couldn't maintain the order in the g_column_labels array.
        GPtrArray *temp_labels=g_ptr_array_new_full(columns, g_free);
        for(i=0;i<columns;i++)
          {
            g_snprintf(buffer, column_width+1, "%s%s", (char*)g_ptr_array_index(g_column_labels, i), fill_end);
            length=strlen(buffer);
            fill_temp=column_width-length;
            gchar *fill_start=NULL;
            if(fill_temp>0) fill_start=g_strnfill(fill_temp, ' ');
            //g_print("2fill %i %i %i\n", fill_temp, shift_column_left, length);
            if(fill_temp>0) g_ptr_array_add(temp_labels, g_strdup_printf("%s%s", fill_start, buffer));
            else g_ptr_array_add(temp_labels, g_strdup_printf("%s", buffer));
            if(fill_start!=NULL) g_free(fill_start);
          }
        array_length=g_column_labels->len;
        for(i=0;i<array_length; i++) g_ptr_array_remove_index_fast(g_column_labels, 0);
        for(i=0;i<columns;i++) g_ptr_array_add(g_column_labels, g_strdup_printf("%s", (char*)g_ptr_array_index(temp_labels, i)));
        g_ptr_array_free(temp_labels, TRUE);
     }

    //If table labels array doesn't match the number of tables remove items from the array.
    if(g_table_labels->len!=0&&g_table_labels->len!=tables)
      {
        array_length=g_table_labels->len;
        for(i=0;i<array_length; i++) g_ptr_array_remove_index_fast(g_table_labels, 0);
      }

    gint length1=g_row_labels->len;
    for(i=0;i<length1;i++) g_print("%s ", (char*)g_ptr_array_index(g_row_labels, i));
    g_print("\n");
    gint length2=g_column_labels->len;
    for(i=0;i<length2;i++) g_print("%s ", (char*)g_ptr_array_index(g_column_labels, i));
    g_print("\n");

    pre_column_width=column_width;
    if(fill_end!=NULL) g_free(fill_end);    
  }
static double round1(gdouble x, guint digits)
  {
    gdouble fac = pow(10, digits);
    return round(x*fac)/fac;
  }
static void print_dialog(GtkWidget *widget, GtkWidget *ws[])
  {
    g_print("Print Dialog\n");
    GtkPrintOperation *operation=gtk_print_operation_new();
    gtk_print_operation_set_default_page_setup(operation, NULL);
    g_signal_connect(operation, "begin_print", G_CALLBACK(begin_print), ws);
    g_signal_connect(operation, "draw_page", G_CALLBACK(draw_page), ws);
    g_signal_connect(operation, "end_print", G_CALLBACK(end_print), NULL);
    GtkPrintOperationResult res=gtk_print_operation_run(operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(ws[21]), NULL);
    if(res==GTK_PRINT_OPERATION_RESULT_APPLY)
      {
        g_print("Print\n");
      }
    g_object_unref(operation);
  }
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, GtkWidget *ws[])
  {
    g_print("Begin Print\n");
    gint i=0;
    gdouble page_width=gtk_print_context_get_width(context);
    gdouble page_height=gtk_print_context_get_height(context);
    PangoContext *pango_context=gtk_widget_get_pango_context(ws[20]);
    PangoFontDescription *description=pango_context_get_font_description(pango_context);
    pango_layout_print=gtk_print_context_create_pango_layout(context);
    pango_layout_set_font_description(pango_layout_print, description);
    pango_layout_set_width(pango_layout_print, page_width*PANGO_SCALE);
    pango_layout_set_height(pango_layout_print, page_height*PANGO_SCALE);
    gint text_height=pango_layout_get_height(pango_layout_print);

    //Set start table count global to 0.
    table_count=0;
    
    //Figure out number of pages.
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
    gint count_lines=gtk_text_buffer_get_line_count(buffer);
    
    gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
    gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(ws[1])));
    gint shift_below_text=atoi(gtk_entry_get_text(GTK_ENTRY(ws[3])));
    gint column_width=atoi(gtk_entry_get_text(GTK_ENTRY(ws[4])));
    gint shift_number_left=atoi(gtk_entry_get_text(GTK_ENTRY(ws[5])));
    gint shift_column_left=atoi(gtk_entry_get_text(GTK_ENTRY(ws[6])));
    gint tables=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
    gint round_float=atoi(gtk_entry_get_text(GTK_ENTRY(ws[10])));
    gint combo4_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[14])));
    gint combo6_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[16])));
    gint label_lines=tables;

    pango_layout_set_markup(pango_layout_print, "5", -1);
    PangoRectangle rectangle_ink;
    PangoRectangle rectangle_log;
    pango_layout_get_extents(pango_layout_print, &rectangle_ink, &rectangle_log);

    lines_per_page=(int)(text_height/rectangle_log.height);
    g_print("Lines per page %i\n", lines_per_page);
    total_lines=count_lines+((shift_below_text-1)*tables)+(tables*rows)+label_lines+combo6_index;
    g_print("Total Lines %i Lines per Page %i\n", total_lines, lines_per_page);
    gint pages=(int)(ceil((double)total_lines/(double)lines_per_page));
    g_print("Pages %i\n", pages);

    gtk_print_operation_set_n_pages(operation, pages);
    //Turn off wrapping.
    pango_layout_set_width(pango_layout_print, -1);

    //Clear data arrays for test data.
    gint array_length1=g_data_values->len;
    for(i=0;i<array_length1; i++) g_ptr_array_remove_index_fast(g_data_values, 0);
    gint array_length2=g_min_max->len;
    for(i=0;i<array_length2;i++) g_array_remove_index_fast(g_min_max, 0);       
    //Get the data.
    if(combo4_index==1)
      {
        get_test_data1(rows, columns, tables, column_width, shift_number_left, round_float);
      }
    if(combo4_index==2)
      {
        get_test_data2(rows, columns, tables, column_width, shift_number_left);
      }
    if(combo4_index==3)
      {
        get_test_data3(rows, columns, tables, column_width, shift_number_left);
      }
    else
      {
        get_test_data1(rows, columns, tables, column_width, shift_number_left, round_float);
      }
    //Get the labels. Shift them or truncate them to fit in the rectangles.
    get_labels_for_drawing(tables, rows, columns, column_width, shift_column_left);
    
  }
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, GtkWidget *ws[])
  {
    g_print("Draw Page %i\n", page_nr);
    gint i=0;
    gint index=0;
    gint tables_left_to_print=0;
    gint tables_on_page=0;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
    gint count_lines=gtk_text_buffer_get_line_count(buffer);
    gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(ws[0])));
    gint tables=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
    gint shift_below_text=atoi(gtk_entry_get_text(GTK_ENTRY(ws[3])));
    gint combo5_index = atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(ws[15])));

    //Account for first page with title and text.
    gint tables_first_page = (int)floor((double)(lines_per_page)/(double)(rows+shift_below_text+count_lines));
    if(tables<tables_first_page) tables_first_page=tables;
    gint tables_next_page=(int)(lines_per_page/(rows+shift_below_text-1));
    if(page_nr==0)
      {
        tables_left_to_print=tables;
      }
    else
      {
        index=page_nr-1;        
        tables_left_to_print=(int)(tables-(index*tables_next_page)-tables_first_page);
      }
    g_print("Tables Left to Print %i\n", tables_left_to_print);
    gint pages=(int)ceil((double)total_lines/(double)lines_per_page);
    if(page_nr==0) tables_on_page=tables_first_page;
    else if(page_nr==pages-1) tables_on_page=tables_left_to_print;
    else tables_on_page=tables_next_page;
    g_print("Tables on Page %i\n", tables_on_page);
    cairo_t *cr=gtk_print_context_get_cairo_context(context);

    // Get the markup string.
    GString *markup=g_string_new("");
    get_pango_markup(ws, markup);

    GString *string=g_string_new("");
    GString *table_string=g_string_new("");
    if(page_nr==0) g_string_append(table_string, markup->str);
    //Reset global variable for drawing on pages.
    table_print=0;
    for(i=0;i<tables_on_page;i++)
      {
        get_table_string(string, ws, page_nr, table_count, count_lines);
        draw_tables(pango_layout_print, cr, ws, page_nr, table_count, count_lines);
        g_string_append_printf(table_string, "%s", string->str);
        g_string_truncate(string, 0);
        table_count++;        
      }

    if(combo5_index!=1)
      {
        if(combo5_index==2) cairo_set_source_rgb(cr, 0, 0, 0);
        else if(combo5_index==3) cairo_set_source_rgb(cr, 1.0, 0, 0);
        else if(combo5_index==4) cairo_set_source_rgb(cr, 0, 1.0, 0);
        else if(combo5_index==5) cairo_set_source_rgb(cr, 0, 0, 1.0);
        else cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_rectangle(cr, 0, 0, gtk_print_context_get_width(context), gtk_print_context_get_height(context));
        cairo_stroke(cr);
      }

    cairo_set_source_rgb(cr, 0, 0, 0);
    pango_layout_set_markup(pango_layout_print, table_string->str, -1);
    pango_cairo_show_layout(cr, pango_layout_print);

    g_string_free(string, TRUE);
    g_string_free(markup, TRUE);
    g_string_free(table_string, TRUE);
    
  }
static void end_print(GtkPrintOperation *operation, GtkPrintContext *context, gpointer data)
  {
    g_print("End Print\n");
    g_object_unref(pango_layout_print);
  }









