
/*

    Re-write the report_generator.py program in C. A ways to go yet. Just some of the UI has been copied
over so far along with a few functions. Work in progress.
 
    gcc -Wall OSVreport_generator.c -o OSVreport_generator -I/usr/include/json-glib-1.0 `pkg-config --cflags --libs gtk+-3.0` -ljson-glib-1.0 -lsqlite3

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include <json-glib/json-glib.h>
#include<sqlite3.h>
#include<stdlib.h>

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
static void start_draw_report(GtkNotebook *notebook, GtkWidget *page, guint page_num, GtkWidget *ws2[]);
static void drawing_area_preview(GtkWidget *da, cairo_t *cr, GtkWidget *ws[]);

//Need to package some of these globals.
//Globals for blocking signals when inserting rows into combo boxes.
static gint row_combo_block=0;
static gint column_combo_block=0;
static gint table_combo_block=0;
static gint da_block=0;
static gboolean da_blocking=FALSE;
//For comparison and clearing pointer arrays when entry value changes.
gint g_row_value=10;
gint g_column_value=5;
gint g_table_value=5;
//Globals for storing labels.
static GPtrArray *g_row_labels=NULL;
static GPtrArray *g_column_labels=NULL;
static GPtrArray *g_table_labels=NULL;
//Drawing globals.
static gint plate_counter=1;
static gint plate_counter_sql=1;
static gint line_count=0;

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
    gtk_entry_set_text(GTK_ENTRY(entry4), "3");

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
    g_signal_connect(entry1, "focus_out_event", G_CALLBACK(clear_row_labels), button5);
    g_signal_connect(entry2, "focus_in_event", G_CALLBACK(save_current_column_value), button5);
    g_signal_connect(entry2, "focus_out_event", G_CALLBACK(clear_column_labels), button5);

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

    GtkWidget *ws[]={entry1, entry2, entry3, entry4, entry5, entry6, entry7, entry8, entry9, entry10, entry11, combo1, combo2, combo3, combo4, combo5, combo6, combo7, check1, check2, textview1, window, label2};
    g_signal_connect(menu1item1, "activate", G_CALLBACK(open_report), ws);
    g_signal_connect(menu1item2, "activate", G_CALLBACK(save_report), ws);
    g_signal_connect(button5, "clicked", G_CALLBACK(labels_dialog), ws);
    g_signal_connect(button6, "clicked", G_CALLBACK(table_labels_dialog), ws);

    //Initailize global arrays.
    g_row_labels=g_ptr_array_new_full(5, g_free);
    g_column_labels=g_ptr_array_new_full(5, g_free);
    g_table_labels=g_ptr_array_new_full(5, g_free);
 
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
    da_block=g_signal_connect(da, "draw", G_CALLBACK(draw_report), ws);

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
    GtkWidget *ws2[]={da, scroll2};
    g_signal_connect(notebook, "switch_page", G_CALLBACK(start_draw_report), ws2);

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
        sqlite3_open("VelociRaptorData.db", &cnn);
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
            for(i=0;i<array_length; i++)
              {
                g_ptr_array_remove_index_fast(g_column_labels, 0);
              }
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
           GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
           found_member=json_reader_read_member(reader, "markup");
           if(!found_member) goto bad_element;
           gtk_text_buffer_set_text(buffer, json_reader_get_string_value(reader), -1);
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
        GtkTextIter start, end;
        GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
        gtk_text_buffer_get_start_iter(buffer, &start);
        gtk_text_buffer_get_end_iter(buffer, &end);
        json_builder_set_member_name(builder, "markup");
        json_builder_add_string_value(builder, gtk_text_buffer_get_text(buffer, &start, &end, TRUE));
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
    gint return_value=validate_entries(ws);
    if(return_value!=0&&da_blocking==FALSE)
      {
        g_signal_handler_block((gpointer)da, da_block);
        da_blocking=TRUE;
      }
    if(return_value==0&&da_blocking==TRUE)
      {
        g_signal_handler_unblock((gpointer)da, da_block);
        da_blocking=FALSE;
      }
    if(return_value==0) drawing_area_preview(da, cr, ws);
    return TRUE;
  }
static void start_draw_report(GtkNotebook *notebook, GtkWidget *page, guint page_num, GtkWidget *ws2[])
  {
    if(da_blocking)
      {
        g_signal_handler_unblock((gpointer)ws2[0], da_block);
        da_blocking=FALSE;
      }
    GtkAdjustment *adj1=gtk_scrolled_window_get_hadjustment(GTK_SCROLLED_WINDOW(ws2[1]));
    GtkAdjustment *adj2=gtk_scrolled_window_get_vadjustment(GTK_SCROLLED_WINDOW(ws2[1]));
    gtk_adjustment_set_value(adj1, 0);
    gtk_adjustment_set_value(adj2, 0);
  }
static void drawing_area_preview(GtkWidget *da, cairo_t *cr, GtkWidget *ws[])
  {
    g_print("Draw Report\n");
    plate_counter=1;
    plate_counter_sql=1;
    line_count=0;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ws[20]));
    gint count_lines=gtk_text_buffer_get_line_count(buffer);
    gint tables=atoi(gtk_entry_get_text(GTK_ENTRY(ws[7])));
    gchar *font_string=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(ws[17]));
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
    g_print("Font %s, Tables %i, Count Lines %i\n", font_string, tables, count_lines);
    cairo_set_source_rgb(cr, 0, 0, 0);
    pango_layout_set_markup(pango_layout, "Test Draw\nTest Draw\nTest Draw\nTest Draw\nTest Draw", -1);
    pango_cairo_show_layout(cr, pango_layout);
    g_object_unref(pango_layout);
    if(font_string!=NULL) g_free(font_string);
  }









