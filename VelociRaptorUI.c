
/*

    This file has int main() along with the UI code for the VelociRaptor program.

    Some ideas for microtiter plate analyses. If plate data is linearized into a column then it
makes it easier to calculate different sets of statistical values. Give it a try.

    Copyright (c) 2015 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
cecashon@aol.com

    Compile with make and makefile.

    Tested on GTK 3.18 with Ubuntu 16.04.

*/

//for using popen()
#define _XOPEN_SOURCE //500

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <apop.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_cdf.h>
#include <sqlite3.h>
#include <math.h>
#include "HotellingsT2.h"
#include "VelociRaptorMath.h"
#include "VelociRaptorUI_Validation.h"
#include "VelociRaptorGlobal.h"
#include "VelociRaptorPrinting.h"
#include "VelociRaptorPermutations.h"
#include "VelociRaptorHtmlTable.h"

//File global variables. Some weird naming for globals.
static GtkWindow *pMainWindow=NULL;   
static const gchar *pPlateNumberText=NULL;
static const gchar *pCurrentFont=NULL;
static guint32 iTextArrayCount=0;
static guint32 iRandomDataArrayCount=0;
static gint iReferenceCountDialogWindow=0;
static gboolean bUnderline=FALSE;

//Program global variables.
const gchar *pPlateSizeText=NULL;
const gchar *pPlateStatsText=NULL;
const gchar *pPlatePosControlText=NULL;
const gchar *pPlateNegControlText=NULL;
const gchar *pWindowTitle=NULL;
gint iBreakLoop=0;

//Global integration parameters in Dunnett's. Declared in VelociRaptorGlobal.h. Used with Fortran.
int MAXPTS_C=1000;
double ABSEPS_C=0.01;

double wrap_gsl_rng_uniform(gsl_rng *r, double param);
static void destroy_event(GtkWidget*, gpointer);
static void activate_pos_control_event(GtkWidget*, GtkEntry*);
static void activate_neg_control_event(GtkWidget*, GtkEntry*);
static void font_chooser_dialog(GtkWidget*, GtkTextView*);
static void change_underline(GtkWidget*, GtkTextView*);
static void change_selection_font(GtkWidget*, GtkTextView*);
static void change_global_font(GtkWidget*, GtkTextView*);
static void change_margin(GtkWidget*, GtkTextView*);
static void distributions_dialog(GtkButton*, gpointer);
static gboolean dialog_state_change(GtkWidget*, GdkEvent*, gpointer);
static void dialog_reference_destroy(GtkWidget*, gint, gpointer);
static void basic_statistics_dialog(GtkWidget*, GtkTextView*);
static void gaussian_dialog(GtkWidget*, GtkTextView*);
static void homogeniety_of_variance_dialog(GtkWidget*, GtkTextView*);
static void one_way_anova_dialog(GtkWidget*, GtkTextView*);
static void comparison_with_control_dialog(GtkWidget*, GtkTextView*);
static void dunnetts_parameters_dialog(GtkWidget*, gpointer);
static void hotelling_dialog(GtkWidget*, GtkTextView*);
static void permutations_dialog(GtkWidget*, GtkTextView*);
static void z_factor_dialog(GtkWidget*, GtkTextView*);
static void contingency_dialog(GtkWidget*, GtkTextView*);
static void exit_comparison_loop_event(GtkWidget*,gint, gpointer);
static void exit_hotelling_dialog(GtkWidget* ,gint , gpointer);
static void exit_z_factor_dialog(GtkWidget* , gint , gpointer);
static void database_to_scatter_graph_dialog(GtkWidget* , gpointer);
static void database_to_error_graph_dialog(GtkWidget* , gpointer);
static void database_to_box_graph_dialog(GtkWidget* , gpointer);
static void format_text_dialog(GtkButton*, gpointer);
static void clear_format_event(GtkButton*, gpointer);
static void heatmap_dialog(GtkButton *button, gpointer data);
static void rise_fall_text_dialog(GtkButton*, gpointer);
static void heatmap_html_dialog(GtkButton*, gpointer);
static void html_table_dialog(GtkButton*, gpointer);
static void send_text_to_database_dialog(GtkButton*, gpointer);
static void test_data_button_clicked(GtkButton*, gpointer, int, double, int);
static void text_button_clicked(GtkButton*, GtkTextView*);
static void next_button_clicked(GtkButton*, GtkTreeView*);
static void cell_edited(GtkCellRendererText*, gchar*, gchar*, GtkTreeView*);
gboolean entry_field_changed(GtkWidget*, GdkEvent*, gpointer);
gboolean control_changed(GtkWidget*, gpointer);
static void cursor_changed(GtkTreeView*, gpointer);
static void select_data(GtkWidget*, GtkTreeView*);
static void pop_up_button_press_event(GtkWidget*, GdkEventButton*, GtkWidget*);
//Not working correctly.
//static void copy_selected_to_clipboard(GtkWidget*, GtkWidget*);
static void copy_plates_to_clipboard_dialog(GtkWidget *copy, GtkWidget *treeview);
static void copy_plates_to_clipboard_withtruncate_dialog(GtkWidget *copy, GtkWidget *treeview);
static void copy_plates_to_clipboard(GtkWidget*, GtkWidget*,gint,gint,gint);
static void select_all(GtkTreeView*, gpointer);
static void copy_treeview_to_database(GtkWidget*, GtkWidget*);
static void setup_tree_view_data(GtkTreeView*, int, double, int);
static void setup_tree_view_percent(GtkTreeView*, GtkTreeView*);
static void setup_tree_view_text(GtkTreeView*, GArray*);
static void get_text_file(GtkWidget*, GtkWidget*);
static void append_text_dialog(GtkWidget*, GtkWidget*);
static void show_file_dialog_append(GArray*);
static void append_text_files(GSList*, const gchar*);
static void sqlite_connect_dialog(GtkWidget*, GtkWidget*);
static void build_aux_table_dialog(GtkWidget*, GtkWidget*);
static void build_combo_table_dialog(GtkWidget*, GtkWidget*);
static void build_permutation_table_dialog(GtkWidget*, GtkWidget*);
static void about_dialog(GtkWidget*, GtkWidget*);
static GdkPixbuf* draw_velociraptor();
static gboolean draw_veloci_raptor_feet(GtkWidget*, cairo_t *cr, gpointer);
static void connect_sqlite_db(GtkWidget *button1, GArray *widgets);
static void get_treeview_selected(GtkWidget *button1, GArray *widgets);
static void get_single_field_values(gchar *table, gchar *field, GArray *widgets);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
    
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    //Quiet dialog messages about transient parent. Just add the global pMainWindow to the dialogs.
    pMainWindow=GTK_WINDOW(window);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), "Ordered Set VelociRaptor");
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 825, 300);

    //Set the icon for the launcher and about dialog.
    GdkPixbuf *dino=draw_velociraptor();
    gtk_window_set_default_icon(dino);
    
    GtkWidget *raptor_feet=gtk_drawing_area_new();
    gtk_widget_set_hexpand(raptor_feet, TRUE);
    gtk_widget_set_size_request(raptor_feet, 825, 35);
    g_signal_connect(G_OBJECT(raptor_feet), "draw", G_CALLBACK(draw_veloci_raptor_feet), window);

    GtkWidget *button=gtk_button_new_with_label("Get Test Data");
    gtk_widget_set_halign(button, GTK_ALIGN_CENTER);
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_widget_set_margin_left(button, 20);
    G_GNUC_END_IGNORE_DEPRECATIONS

    GtkWidget *textbutton=gtk_button_new_with_label("Erase White Board");
    gtk_widget_set_hexpand(textbutton, TRUE);

    GtkWidget *clear_format=gtk_button_new_with_label("Clear Format");
    gtk_widget_set_hexpand(clear_format, TRUE);

    GtkWidget *underline_button=gtk_toggle_button_new_with_label("Underline");
    gtk_widget_set_halign(underline_button, GTK_ALIGN_START);
    gtk_widget_set_hexpand(underline_button, FALSE);
    gtk_widget_set_tooltip_text(underline_button, "Underline Selection Font");

    GtkWidget *selection_button=gtk_button_new_with_label("Selection");
    gtk_widget_set_halign(selection_button, GTK_ALIGN_START);
    gtk_widget_set_hexpand(selection_button, FALSE);
    gtk_widget_set_tooltip_text(selection_button, "Selection Font");

    GtkWidget *global_button=gtk_button_new_with_label("Global");
    gtk_widget_set_halign(global_button, GTK_ALIGN_START);
    gtk_widget_set_hexpand(global_button, FALSE);
    gtk_widget_set_tooltip_text(global_button, "Global Font");

    GtkWidget *font_chooser=gtk_font_button_new_with_font("Monospace 9");
    gtk_widget_set_hexpand(font_chooser, TRUE);
    gtk_widget_set_tooltip_text(font_chooser, "Font Chooser");

    GtkWidget *file_menu=gtk_menu_new(); 
    GtkWidget *import_item=gtk_menu_item_new_with_label("Import Text File");
    GtkWidget *append_item=gtk_menu_item_new_with_label("Append Text Files");
    GtkWidget *sqlite_item=gtk_menu_item_new_with_label("Sqlite Connect");
    GtkWidget *print_item=gtk_menu_item_new_with_label("Print White Board");
    GtkWidget *quit_item=gtk_menu_item_new_with_label("Quit");

    GtkWidget *file_menu2=gtk_menu_new();
    GtkWidget *build_aux_item=gtk_menu_item_new_with_label("Auxiliary Table");
    GtkWidget *build_combo_item=gtk_menu_item_new_with_label("Combination Table");
    GtkWidget *build_permut_item=gtk_menu_item_new_with_label("Permutation Table");
    GtkWidget *build_board_item=gtk_menu_item_new_with_label("White Board Table");

    GtkWidget *file_menu3=gtk_menu_new();
    GtkWidget *basic_stats_item=gtk_menu_item_new_with_label("Descriptive Statistics");
    GtkWidget *gaussian_item=gtk_menu_item_new_with_label("Normality Test");
    GtkWidget *variance_item=gtk_menu_item_new_with_label("Homogeniety of Variance");
    GtkWidget *anova_item=gtk_menu_item_new_with_label("One-Way ANOVA");
    GtkWidget *dunn_sidak_item=gtk_menu_item_new_with_label("Comparison with Control");
    GtkWidget *hotelling_item=gtk_menu_item_new_with_label("Comparison with Contrasts");
    GtkWidget *permutations_item=gtk_menu_item_new_with_label("Permutation Testing");
    GtkWidget *z_factor_item=gtk_menu_item_new_with_label("Calculate Z-factor");
    GtkWidget *contingency_item=gtk_menu_item_new_with_label("Contingency Data");

    GtkWidget *file_menu4=gtk_menu_new();
    GtkWidget *scatter_item=gtk_menu_item_new_with_label("Scatter Plot");
    GtkWidget *error_item=gtk_menu_item_new_with_label("Error Plot");
    GtkWidget *box_item=gtk_menu_item_new_with_label("Box Plot");

    GtkWidget *file_menu5=gtk_menu_new();
    GtkWidget *heatmap_item=gtk_menu_item_new_with_label("Heatmap Platemap");
    GtkWidget *conditional_item=gtk_menu_item_new_with_label("Conditional Format Platemap");
    GtkWidget *rise_fall_item=gtk_menu_item_new_with_label("Rise Fall Platemap");
    GtkWidget *html_item=gtk_menu_item_new_with_label("Heatmap Platemap HTML");
    GtkWidget *html_table_item=gtk_menu_item_new_with_label("SQL Query to HTML");

    GtkWidget *file_menu6=gtk_menu_new();
    GtkWidget *about_item=gtk_menu_item_new_with_label("Ordered Set VelociRaptor");

    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), import_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), append_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), sqlite_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), print_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu), quit_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu2), build_aux_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu2), build_combo_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu2), build_permut_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu2), build_board_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), basic_stats_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), gaussian_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), variance_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), anova_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), dunn_sidak_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), hotelling_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), permutations_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), z_factor_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu3), contingency_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu4), scatter_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu4), error_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu4), box_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu5), heatmap_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu5), conditional_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu5), rise_fall_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu5), html_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu5), html_table_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(file_menu6), about_item);


    g_signal_connect(quit_item, "activate", G_CALLBACK(destroy_event), NULL);
    //print_item signal connected after textview
    g_signal_connect(import_item, "activate", G_CALLBACK(get_text_file), window);
    g_signal_connect(append_item, "activate", G_CALLBACK(append_text_dialog), window);
    g_signal_connect(sqlite_item, "activate", G_CALLBACK(sqlite_connect_dialog), window);
    g_signal_connect(about_item, "activate", G_CALLBACK(about_dialog), window);
    //anova_item signal connected after textview.
    g_signal_connect(scatter_item, "activate", G_CALLBACK(database_to_scatter_graph_dialog), NULL);
    g_signal_connect(error_item, "activate", G_CALLBACK(database_to_error_graph_dialog), NULL);
    g_signal_connect(box_item, "activate", G_CALLBACK(database_to_box_graph_dialog), NULL);
    g_signal_connect(build_aux_item, "activate", G_CALLBACK(build_aux_table_dialog), window);
    g_signal_connect(build_combo_item, "activate", G_CALLBACK(build_combo_table_dialog), window);
    g_signal_connect(build_permut_item, "activate", G_CALLBACK(build_permutation_table_dialog), window);
     
    GtkWidget *menu_bar=gtk_menu_bar_new();
    //gtk_widget_show(menu_bar);
    GtkWidget *file_item=gtk_menu_item_new_with_label("File");
    GtkWidget *file_item2=gtk_menu_item_new_with_label("Data");
    GtkWidget *file_item3=gtk_menu_item_new_with_label("Analysis");
    GtkWidget *file_item4=gtk_menu_item_new_with_label("Graph");
    GtkWidget *file_item5=gtk_menu_item_new_with_label("Format");
    GtkWidget *file_item6=gtk_menu_item_new_with_label("About");

    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item), file_menu);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item2), file_menu2);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item3), file_menu3);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item4), file_menu4);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item5), file_menu5);
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(file_item6), file_menu6);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item2);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item3);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item4);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item5);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), file_item6);
       
    GtkWidget *textview=gtk_text_view_new();
    gtk_widget_set_hexpand(textview, TRUE);
    gtk_widget_set_vexpand(textview, TRUE);
    //Set initial font.
    PangoFontDescription *pfd=pango_font_description_from_string("Monospace 9");
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS 
    gtk_widget_override_font(GTK_WIDGET(textview), pfd);
    G_GNUC_END_IGNORE_DEPRECATIONS
    g_signal_connect(underline_button, "clicked", G_CALLBACK(change_underline), textview);
    g_signal_connect(selection_button, "clicked", G_CALLBACK(change_selection_font), textview);
    g_signal_connect(global_button, "clicked", G_CALLBACK(change_global_font), textview);
    g_signal_connect(font_chooser, "font-set", G_CALLBACK(font_chooser_dialog), textview);

    g_signal_connect(basic_stats_item, "activate", G_CALLBACK(basic_statistics_dialog), textview);
    g_signal_connect(gaussian_item, "activate", G_CALLBACK(gaussian_dialog), textview);
    g_signal_connect(variance_item, "activate", G_CALLBACK(homogeniety_of_variance_dialog), textview);
    g_signal_connect(anova_item, "activate", G_CALLBACK(one_way_anova_dialog), textview);
    g_signal_connect(dunn_sidak_item, "activate", G_CALLBACK(comparison_with_control_dialog), textview);
    g_signal_connect(hotelling_item, "activate", G_CALLBACK(hotelling_dialog), textview);
    g_signal_connect(permutations_item, "activate", G_CALLBACK(permutations_dialog), textview);
    g_signal_connect(z_factor_item, "activate", G_CALLBACK(z_factor_dialog), textview);
    g_signal_connect(contingency_item, "activate", G_CALLBACK(contingency_dialog), textview);

    g_signal_connect(G_OBJECT(heatmap_item), "activate", G_CALLBACK(heatmap_dialog), textview);
    g_signal_connect(G_OBJECT(conditional_item), "activate", G_CALLBACK(format_text_dialog), textview);
    g_signal_connect(G_OBJECT(rise_fall_item), "activate", G_CALLBACK(rise_fall_text_dialog), textview);
    g_signal_connect(G_OBJECT(html_item), "activate", G_CALLBACK(heatmap_html_dialog), NULL);
    g_signal_connect(G_OBJECT(html_table_item), "activate", G_CALLBACK(html_table_dialog), NULL);
    g_signal_connect(G_OBJECT(build_board_item), "activate", G_CALLBACK(send_text_to_database_dialog), textview);

    //For printing.
    Widgets *w=g_slice_new(Widgets);
    w->window=GTK_WIDGET(window);
    w->textview=GTK_WIDGET(textview);
    g_signal_connect(print_item, "activate", G_CALLBACK(print_textview), (gpointer)w);
        
    GtkWidget *scrolled_win=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scrolled_win, TRUE);
    gtk_widget_set_vexpand(scrolled_win, TRUE);
     
    gtk_container_add(GTK_CONTAINER(scrolled_win), textview);

    GtkWidget *plate_parameters_label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(plate_parameters_label), "<span weight='bold' underline='single'>Parameters</span>");

    GtkWidget *text_label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(text_label), "<span weight='bold' underline='single'>White Board</span>");
    gtk_widget_set_halign(text_label, GTK_ALIGN_CENTER);

    GtkWidget *plate_number_label=gtk_label_new("Number of Plates");
    GtkWidget *plate_size_label=gtk_label_new("Size of Plate");
    GtkWidget *plate_stats_label=gtk_label_new("Set Size for Stats");
    GtkWidget *plate_pos_control_label=gtk_label_new("Positive Controls");
    GtkWidget *plate_neg_control_label=gtk_label_new("Negative Controls");

    GtkWidget *control_check=gtk_check_button_new_with_label("Optional Control Locations");

    GtkWidget *plate_number_entry=gtk_entry_new();
    GtkWidget *plate_size_entry=gtk_entry_new();
    GtkWidget *plate_stats_entry=gtk_entry_new();
    GtkWidget *plate_pos_control_entry=gtk_entry_new();
    GtkWidget *plate_neg_control_entry=gtk_entry_new();

    gtk_widget_set_halign(plate_number_entry, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(plate_size_entry, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(plate_stats_entry, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(plate_pos_control_entry, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(plate_neg_control_entry, GTK_ALIGN_CENTER);

    gtk_entry_set_text(GTK_ENTRY(plate_number_entry), "3");
    gtk_entry_set_text(GTK_ENTRY(plate_size_entry), "96");
    gtk_entry_set_text(GTK_ENTRY(plate_stats_entry), "4");
    gtk_entry_set_text(GTK_ENTRY(plate_pos_control_entry), "");
    gtk_entry_set_text(GTK_ENTRY(plate_neg_control_entry), "");

    gtk_widget_set_sensitive(GTK_WIDGET(plate_pos_control_entry),FALSE);
    gtk_widget_set_sensitive(GTK_WIDGET(plate_neg_control_entry),FALSE);

    g_signal_connect(G_OBJECT(control_check), "clicked", G_CALLBACK(activate_pos_control_event), plate_pos_control_entry);
    g_signal_connect(G_OBJECT(control_check), "clicked", G_CALLBACK(activate_neg_control_event), plate_neg_control_entry);

    //Set global pointers to values.
    pPlateNumberText=gtk_entry_get_text(GTK_ENTRY(plate_number_entry));
    pPlateSizeText=gtk_entry_get_text(GTK_ENTRY(plate_size_entry));
    pPlateStatsText=gtk_entry_get_text(GTK_ENTRY(plate_stats_entry));
    pPlatePosControlText=gtk_entry_get_text(GTK_ENTRY(plate_pos_control_entry));
    pPlateNegControlText=gtk_entry_get_text(GTK_ENTRY(plate_neg_control_entry));
    pCurrentFont=gtk_font_button_get_font_name(GTK_FONT_BUTTON(font_chooser));
     
    gtk_entry_set_width_chars(GTK_ENTRY(plate_number_entry), 5);
    gtk_entry_set_width_chars(GTK_ENTRY(plate_size_entry), 5);
    gtk_entry_set_width_chars(GTK_ENTRY(plate_stats_entry), 5);
    /*
      Put these entries in a fixed container so they don't expand in the grid.
      This code was added because the grid in GTK3.18 will resize the entries.
      Doesn't seem to be a problem in GTK3.10.
    */
    GtkWidget *fixed_plate_number_entry=gtk_fixed_new();
    gtk_fixed_put(GTK_FIXED(fixed_plate_number_entry), plate_number_entry, 10, 10);
    GtkWidget *fixed_plate_size_entry=gtk_fixed_new();
    gtk_fixed_put(GTK_FIXED(fixed_plate_size_entry), plate_size_entry, 10, 10);
    GtkWidget *fixed_plate_stats_entry=gtk_fixed_new();
    gtk_fixed_put(GTK_FIXED(fixed_plate_stats_entry), plate_stats_entry, 10, 10);

    gtk_entry_set_width_chars(GTK_ENTRY(plate_pos_control_entry), 10);
    gtk_entry_set_width_chars(GTK_ENTRY(plate_neg_control_entry), 10);

    GtkWidget *margin_combo=gtk_combo_box_text_new_with_entry();
    gtk_widget_set_hexpand(margin_combo, FALSE);
    gtk_widget_set_halign(margin_combo, GTK_ALIGN_START);
    gtk_widget_set_tooltip_text(margin_combo, "Left Margin");
    GtkWidget *combo_entry=gtk_bin_get_child(GTK_BIN(margin_combo));
    gtk_widget_set_hexpand(combo_entry, FALSE);
    gtk_entry_set_width_chars(GTK_ENTRY(combo_entry), 3);
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "1", "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "2", "10");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "3", "20");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "4", "30");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "5", "40");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "6", "50");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(margin_combo), "7", "60");
    gtk_combo_box_set_active(GTK_COMBO_BOX(margin_combo), 0);
    g_signal_connect(margin_combo, "changed", G_CALLBACK(change_margin), textview);

    /*
      Put this in a fixed container so they don't expand in the grid.
      This code was added because the grid in GTK3.18 will resize the entries.
      Doesn't seem to be a problem in GTK3.10.
    */
    GtkWidget *fixed_margin_combo=gtk_fixed_new();
    gtk_fixed_put(GTK_FIXED(fixed_margin_combo), margin_combo, 10, 10);

    GtkWidget *grid1=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid1), plate_parameters_label, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_number_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_size_label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_stats_label, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), control_check, 0, 6, 2, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_pos_control_label, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_neg_control_label, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), fixed_plate_number_entry, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), fixed_plate_size_entry, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), fixed_plate_stats_entry, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_pos_control_entry, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), plate_neg_control_entry, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), button, 0, 9, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid1), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid1), 10);

    GtkWidget *grid2=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid2), text_label, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid2), scrolled_win, 0, 1, 4, 7);
    gtk_grid_attach(GTK_GRID(grid2), textbutton, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), clear_format, 1, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), font_chooser, 2, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), fixed_margin_combo, 5, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), selection_button, 5, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), underline_button, 5, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), global_button, 5, 7, 1, 1);
    gtk_grid_set_column_homogeneous(GTK_GRID(grid2), FALSE);
    gtk_grid_set_row_spacing(GTK_GRID(grid2), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid2), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid2), 10);

    GtkWidget *pane=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_add1(GTK_PANED(pane), grid1);
    gtk_paned_add2(GTK_PANED(pane), grid2);
    gtk_widget_set_name(pane, "pane");

    GtkWidget *grid3=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid3), menu_bar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid3), raptor_feet, 0, 1, 8, 1);
    gtk_grid_attach(GTK_GRID(grid3), pane, 0, 2, 8, 7);
     
    g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(distributions_dialog), NULL);
    g_signal_connect(G_OBJECT(textbutton), "clicked", G_CALLBACK(text_button_clicked), (gpointer) textview);
    g_signal_connect(G_OBJECT(clear_format), "clicked", G_CALLBACK(clear_format_event), textview);

    //Attempt to break out of a focus event without runtime errors.
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), plate_number_entry);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), plate_size_entry);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), plate_stats_entry);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), plate_pos_control_entry);
    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), plate_neg_control_entry);

    g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), NULL); 
 
     //connect_after to try to avoid a run-time error when moving from the entry fields to a dialog.
    g_signal_connect_after(G_OBJECT(plate_number_entry), "focus_out_event", G_CALLBACK(entry_field_changed), textview);
    g_signal_connect_after(G_OBJECT(plate_size_entry), "focus_out_event", G_CALLBACK(entry_field_changed), textview);
    g_signal_connect_after(G_OBJECT(plate_stats_entry), "focus_out_event", G_CALLBACK(entry_field_changed), textview); 
    g_signal_connect_after(G_OBJECT(plate_pos_control_entry), "focus_out_event", G_CALLBACK(control_changed), textview);
    g_signal_connect(G_OBJECT(plate_neg_control_entry), "focus_out_event", G_CALLBACK(control_changed), textview);  
    
    gtk_container_add(GTK_CONTAINER(window), grid3);

    GError *css_error=NULL;
    gint minor_version=gtk_get_minor_version();
    gchar *css_string=NULL;

    //GTK CSS changed in 3.20. The CSS for after 3.20 may need to be modified to have it work.
    if(minor_version>20)
      {
        css_string=g_strdup("window {background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(0,255,0,0.5)), color-stop(0.5,rgba(180,180,180,0.5)), color-stop(1.0,rgba(25,0,200,0.5)));} paned {background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(152,251,152,1)), color-stop(0.5,rgba(180,180,180,1)), color-stop(1.0,rgba(123,104,238,1)));} button{background: rgba(210,210,210,1.0)}");
      }
    else
      {
        css_string=g_strdup("GtkWindow{background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(0,255,0,0.5)), color-stop(0.5,rgba(180,180,180,0.5)), color-stop(1.0,rgba(25,0,200,0.5)));} GtkPaned{background-image: -gtk-gradient (linear, left bottom, right top, color-stop(0.0,rgba(152,251,152,1)), color-stop(0.5,rgba(180,180,180,1)), color-stop(1.0,rgba(123,104,238,1)));} GtkButton{background: rgba(210,210,210,1.0)}");
      }

    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    if(css_error!=NULL) 
      {
        g_print("CSS loader error %s\n", css_error->message);
        g_error_free(css_error);
      }
    if(css_string!=NULL) g_free(css_string);
    g_object_unref(provider);
     
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
  }
double wrap_gsl_rng_uniform(gsl_rng *r, double param)
  {
    return param*gsl_rng_uniform(r);
  }
static void destroy_event(GtkWidget *window, gpointer data)
  {
    guint  signal_id;
    gulong handler_id;

    //Block the signals. Problem the focus_out_event fires before destroy. Use connect after.
    //If a entry has focus from grab focus and destroy is fired, run time errors happen.
    if(GTK_ENTRY(data))
      {
        //g_print("Stop Signals\n");
        signal_id = g_signal_lookup("focus_out_event", GTK_TYPE_ENTRY);
        handler_id = g_signal_handler_find( (gpointer)data,G_SIGNAL_MATCH_ID, signal_id, 0, NULL, NULL, NULL );
        g_signal_handler_disconnect(data, handler_id);
      }
    else
      {
        gtk_main_quit();
        g_print("Close Application\n");
      }
  }
static void activate_pos_control_event(GtkWidget *check, GtkEntry *entry)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
      {
        gtk_widget_set_sensitive(GTK_WIDGET(entry),TRUE);
        gtk_entry_set_text(GTK_ENTRY(entry), "1,2,3,4");
        pPlatePosControlText=gtk_entry_get_text(GTK_ENTRY(entry));
      }
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
      {
        //Set the global variables to empty strings.
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        pPlatePosControlText=gtk_entry_get_text(GTK_ENTRY(entry));
        gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
      }
  }
static void activate_neg_control_event(GtkWidget *check, GtkEntry *entry)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
      {
        gtk_widget_set_sensitive(GTK_WIDGET(entry),TRUE);
        gtk_entry_set_text(GTK_ENTRY(entry), "5,6,7,8");
        pPlateNegControlText=gtk_entry_get_text(GTK_ENTRY(entry));
      }
    if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check)))
      {
        //Set the global variables to empty strings.
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        gtk_entry_set_text(GTK_ENTRY(entry), "");
        pPlateNegControlText=gtk_entry_get_text(GTK_ENTRY(entry));
        gtk_widget_set_sensitive(GTK_WIDGET(entry),FALSE);
      }
  }
static void font_chooser_dialog(GtkWidget *button, GtkTextView *textview)
  {
    pCurrentFont=gtk_font_button_get_font_name(GTK_FONT_BUTTON(button));
    printf("%s\n", pCurrentFont);    
  }
static void change_underline(GtkWidget *button, GtkTextView *textview)
  {
    if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(button)))
      {
        bUnderline=TRUE;
      }
    else
      {
        bUnderline=FALSE;
      }
  }
static void change_selection_font(GtkWidget *button, GtkTextView *textview)
  {
    //Set the font for the textview.
    gint i=0;
    gint as_return=0;
    static gint tag_counter=0;
    GString *tag_name=g_string_new(NULL);
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(textview);
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    g_string_printf(tag_name, "tag%i", tag_counter);

    //Remove prior selection tags in the current selection.
    for(i=0;i<tag_counter;i++)
       {
         gchar *string=NULL;
         as_return=asprintf(&string, "tag%i", i);
         if(as_return!=-1)
           {
             gtk_text_buffer_remove_tag_by_name(buffer, string, &start, &end);
             //printf("Remove Tag %s\n", string);
             free(string);
           }
         else
           {
             printf("Memory allocation error in asprintf.\n");
           }
       }

    if(!bUnderline)
      {
        gtk_text_buffer_create_tag(buffer, tag_name->str ,"font" , pCurrentFont , NULL);
      }
    else
      {
        gtk_text_buffer_create_tag(buffer, tag_name->str ,"font" , pCurrentFont , "underline", PANGO_UNDERLINE_SINGLE, NULL);
      }

    gtk_text_buffer_apply_tag_by_name(buffer, tag_name->str, &start, &end); 
    tag_counter++;    
 
    g_string_free(tag_name, TRUE);
  }
static void change_global_font(GtkWidget *button, GtkTextView *textview)
  {
    PangoFontDescription *pfd;

    pfd = pango_font_description_from_string(pCurrentFont);
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS 
    gtk_widget_override_font(GTK_WIDGET(textview), pfd);
    G_GNUC_END_IGNORE_DEPRECATIONS
  }
static void change_margin(GtkWidget *margin, GtkTextView *textview)
  {
    gint left_margin=0;
    gchar *combo_text=NULL;
    combo_text=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(margin));
    left_margin=atoi(combo_text);
    //printf("Left Margin %i\n", left_margin);
    gtk_text_view_set_indent(textview, left_margin);
    if(combo_text!=NULL) g_free(combo_text);
  }
static gboolean dialog_state_change(GtkWidget *dialog, GdkEvent *event, gpointer data)
  {        
    pWindowTitle=gtk_window_get_title(GTK_WINDOW(dialog)); 
    //printf("Active Window Title %s\n", pWindowTitle);      
    return FALSE;  
  }
static void dialog_reference_destroy(GtkWidget *dialog , gint response, gpointer data)
  {
    const gchar *title;
    
    if(response==GTK_RESPONSE_DELETE_EVENT)
       {
        title=gtk_window_get_title(GTK_WINDOW(dialog));
        g_print("Close %s Dialog\n", title);
        if(g_strcmp0(title, "Plate Data")==0)
          {
           iReferenceCountDialogWindow=0;
           iTextArrayCount=0;
           iRandomDataArrayCount=0;
          }
        gtk_widget_destroy(dialog);
       }       
  }
static void distributions_dialog(GtkButton *button, gpointer data)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *label4, *label5, *label6, *entry1, *entry2, *entry3, *entry4, *entry5, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Get Test Data", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Gaussian Distribution       ");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Chi-Squared Distribution");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Rayleigh Distribution        ");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Uniform Distribution        ");
     
    label1=gtk_label_new("Generate Some Numbers For Testing");
    label2=gtk_label_new("SD");
    label3=gtk_label_new("DF");
    label4=gtk_label_new("Scale");
    label5=gtk_label_new("Scale");
    label6=gtk_label_new("Seed Value");

    entry1=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 4);
    gtk_entry_set_text(GTK_ENTRY(entry1), "50");

    entry2=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 4);
    gtk_entry_set_text(GTK_ENTRY(entry2), "1");

    entry3=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 4);
    gtk_entry_set_text(GTK_ENTRY(entry3), "1");

    entry4=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry4), 4);
    gtk_entry_set_text(GTK_ENTRY(entry4), "50");

    entry5=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry5), 8);
    gtk_entry_set_text(GTK_ENTRY(entry5), "0");

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry5, 2, 5, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
     
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint radio_button=0;
        gint seed_value=0;
        gdouble param=1;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
            seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
            param=atof(gtk_entry_get_text(GTK_ENTRY(entry1))); 
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
            seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
            param=atof(gtk_entry_get_text(GTK_ENTRY(entry2))); 
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
            seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
            param=atof(gtk_entry_get_text(GTK_ENTRY(entry3))); 
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
            seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
            param=atof(gtk_entry_get_text(GTK_ENTRY(entry4))); 
          }
        else
          {
            //exit
          }
        if(seed_value>=0&&param>=1)
          {
            test_data_button_clicked(button, NULL, seed_value, param, radio_button);
          }
        else
          {
            printf("Seed Value >= 0 and the Parameter Value >= 1\n");
            simple_message_dialog("Seed Value >= 0 and the Parameter Value >= 1");
          }
      }  
    gtk_widget_destroy(dialog);
  }
static void basic_statistics_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Descriptive Statistics", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Descriptive Statistics from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Descriptive Statistics from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Descriptive Statistics from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Descriptive Statistics from Percent By Picks");
     
    label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database");

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint radio_button=0;
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }

        if(groups_database_validation(NULL)==0)
          {
            basic_statistics_sql(textview, radio_button);
          }
      }
    gtk_widget_destroy(dialog);
  }
static void gaussian_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Normality Test", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Normality Test from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Normality Test from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Normality Test from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Normality Test from Percent By Picks");
     
    label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database.");
    label2=gtk_label_new("Anderson Darling test using 1.0+0.75/n+2.25/n^2\nfor the adjusted value. Easy to change the code\nfor other adjusted values. If p_value>alpha then\ncan't reject the null hypothesis of normality.");

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        printf("Begin Anderson Darling\n");
        gint radio_button=0;         
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }

        if(groups_database_validation(NULL)==0)
          {
            anderson_darling_test(textview, radio_button);
          }
   
      }
    g_print("Anderson Darling Finished\n");
    gtk_widget_destroy(dialog);
  }
static void homogeniety_of_variance_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *entry1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Homogeniety of Variance", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, " Variance from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Variance from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Variance from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Variance from Percent By Picks");
     
    label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database.");
    label2=gtk_label_new("Levene's test using the median");
    label3=gtk_label_new("Alpha for Critical Value");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 2, 6, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint radio_button=0;
        gint check1=0;
        gdouble alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1)));       
        g_print("Begin Levene's\n");         
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }

        check1=critical_value_changed_validation(entry1);
     
        if(check1==0)
          {
            if(groups_database_validation(NULL)==0)
              {
                levenes_variance_test(textview, radio_button, alpha);
              }
          }  
        g_print("Levene's Finished\n");
      }
    gtk_widget_destroy(dialog);
  }
static void one_way_anova_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    /*
      Calculations based on
         De Muth, J.E.(2006). "Basic Statistics and Pharmaceutical Statistical Applications", Second Edition, Chapman & Hall/CRC, p. 208-209.
    */
    GtkWidget *dialog, *grid, *label1, *label2, *entry1, *radio1, *radio2, *radio3, *radio4, *check_button1, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("One-Way ANOVA", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, " One-Way ANOVA from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "One-Way ANOVA from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "One-Way ANOVA from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), " One-Way ANOVA from Percent By Picks");
     
    label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database");
    label2=gtk_label_new("Alpha for Critical Value");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");
     
    check_button1=gtk_check_button_new_with_label("Standard ANOVA Format");
    gtk_widget_set_halign(check_button1, GTK_ALIGN_CENTER);

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button1, 1, 6, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint check_box=0;
        gint radio_button=0;
        gint check1=0;
        gdouble alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1)));       

        printf("Begin One-Way ANOVA\n");
         
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }
         
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button1)))
          {
            check_box=1;
          }

        check1=critical_value_changed_validation(entry1);
     
        if(check1==0)
          {
            if(groups_database_validation(NULL)==0)
              {
                one_way_anova_sql(textview, radio_button, check_box, alpha);
              }
          }  
        g_print("ANOVA Finished\n");
      }
    gtk_widget_destroy(dialog);
  }
static void comparison_with_control_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *entry1, *entry2, *radio1, *radio2, *radio3, *radio4, *radio_bonferroni, *radio_sidak, *radio_dunnetts, *radio_hotellingsT2, *progress, *content_area, *action_area, *dunnett_button;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Comparison with Control", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Picks");

    dunnett_button=gtk_button_new_with_mnemonic("  !  ");
    g_signal_connect(G_OBJECT(dunnett_button), "clicked", G_CALLBACK(dunnetts_parameters_dialog), NULL);

    radio_bonferroni=gtk_radio_button_new_with_label(NULL, "Bonferroni Critical Values");
    radio_sidak=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_bonferroni), "Dunn-Sidak Critical Values");
    radio_dunnetts=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_sidak), "Dunnett's Critical Values");
    radio_hotellingsT2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_sidak), "Hotelling's T2 Critical Values\n (Balanced Sets Only)");
     
    label1=gtk_label_new(" Build Auxiliary Table First. Data Pulled From the Database.\n Control is the Groups or Picks Value From the Auxiliary Table.");
    label2=gtk_label_new("Alpha for Critical Value(two-tail)");
    label3=gtk_label_new("Control GROUP BY Value             ");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 5);
    gtk_entry_set_text(GTK_ENTRY(entry2), "1");

    progress=gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 2, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio_bonferroni, 1, 7, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio_sidak, 1, 8, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio_dunnetts, 1, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), dunnett_button, 3, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio_hotellingsT2, 1, 10, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), progress, 1, 11, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);

    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_comparison_loop_event), NULL);

    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Begin Multiple Comparisons\n");
        gint radio_button=1;
        gint radio_crit_value=1;
        gdouble alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1))); 
        gint control_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));         
        gint check1=0;
        gint check2=0;
        gint check3=0; 
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }
        else
          {
            //exit
          }
        //Choose type of critical values.
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_bonferroni)))
          {
            radio_crit_value=1;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sidak)))
          {
            radio_crit_value=2;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_dunnetts)))
          {
            radio_crit_value=3;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_hotellingsT2)))
          {
            radio_crit_value=4;
          }
        else
          {
            //exit
          }

        //check entry fields
        check1=critical_value_changed_validation(entry1);
        if(radio_button==1||radio_button==2)
          {
            check2=groups_database_validation(entry2);
          }
        if(radio_button==3||radio_button==4)
          {
            check3=picks_database_validation(entry2);
          }           

        if(check1==0&&check2==0&&check3==0)
          {
            if(radio_crit_value==4)
              {
                g_print("Call Hotellings T2\n");
                hotellings_T2(radio_button,alpha,0,control_value-1,textview,progress,NULL,0);
              }
            else
              {
                comparison_with_control_sql(radio_button, control_value, alpha, radio_crit_value, textview, progress, &iBreakLoop); 
              }
          }
        g_print("Multiple Comparisons Finished\n");
      }
    gtk_widget_destroy(dialog);
  }
static void dunnetts_parameters_dialog(GtkWidget *dialog, gpointer data)
  {
    GtkWidget *dialog2, *grid, *label1, *label2, *label3, *entry1, *entry2, *content_area, *action_area;
    int result;

    dialog2=gtk_dialog_new_with_buttons("Dunnett's Integration Parameters", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog2), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog2), 20);

    label1=gtk_label_new("Parameters to set for numerical integration. Increase\nMax Points if returned Error is too large for the Value\nreturned from integration. The Value(Error) pair is\noutput to the terminal. If Inform = 0, normal completion\nof integration. A lot of points to evaluate can take some\ntime.");
    label2=gtk_label_new("Max Points");
    label3=gtk_label_new("Error Tolerance for Alpha");
    
    entry1=gtk_entry_new();
    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 7);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 7);
    gtk_entry_set_text(GTK_ENTRY(entry1), "1000");
    gtk_entry_set_text(GTK_ENTRY(entry2), "0.01");
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 2, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog2));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog2));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog2);
    result=gtk_dialog_run(GTK_DIALOG(dialog2));

    if(result==GTK_RESPONSE_OK)
      {
       gint temp1=0;
       gdouble temp2=0;
       temp1=atoi(gtk_entry_get_text(GTK_ENTRY(entry1))); 
       temp2=atof(gtk_entry_get_text(GTK_ENTRY(entry2)));
       //Set Global variables.
       if(temp1>=1000&&temp1<=100000)
         {
           MAXPTS_C=atoi(gtk_entry_get_text(GTK_ENTRY(entry1))); 
           g_print("MAXPTS=%i\n", MAXPTS_C);
         }
       else
         {
           g_print("Max Points Values 1000<=x<=100000\n");
           simple_message_dialog("Max Points Values 1000<=x<=100000");
         }

       if(temp2>=0.000001&&temp2<=0.1)
         {
           ABSEPS_C=atof(gtk_entry_get_text(GTK_ENTRY(entry2)));
           g_print("ABSEPS=%f\n", ABSEPS_C);
         }
       else
         {
           g_print("Error Tolerance for Alpha 0.000001<=x<=0.1\n");
           simple_message_dialog("Error Tolerance for Alpha 0.000001<=x<=0.1");
         }         
      }
    gtk_widget_destroy(dialog2);
  }
static void hotelling_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *entry1, *radio1, *radio2, *radio3, *radio4, *textview1, *scroll1, *progress, *content_area, *action_area;
    GtkTextBuffer *buffer1;
    GtkTextIter start1;
    GtkTextIter end1;
    gint radio_button=1;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Comparison with Contrasts", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 500);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Picks");
    
    label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database.\nControl is the Groups or Picks Value From the Auxiliary Table.\nFor balanced sets only. The contrast matrix can only have\n1's, 0's or -1's for now.");
    label2=gtk_label_new("Alpha for Hotelling's T2 Critical Value");
    label3=gtk_label_new("Contrast Matrix");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");

    textview1=gtk_text_view_new();
    gtk_widget_set_vexpand(textview1, TRUE);
    buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1));
    gtk_text_buffer_insert_at_cursor(buffer1, "-1 -1 1 1\n1 -1 1 -1\n1 -1 -1 1", -1);
    scroll1=gtk_scrolled_window_new(NULL, NULL); 
    gtk_container_add(GTK_CONTAINER(scroll1), textview1);

    progress=gtk_progress_bar_new();
    gtk_widget_set_hexpand(progress, TRUE);
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scroll1, 1, 7, 1, 3);
    gtk_grid_attach(GTK_GRID(grid), progress, 1, 10, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_hotelling_dialog), NULL);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Begin Hotelling's T2 Comparisons\n");
        gsl_matrix *SuppliedContrasts=NULL;
        gint rows=0;
        gint columns=0;
        gint check1=0;
        gint check2=0;
        gint check3=0;
        gint numbers=0;
        gdouble alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1))); 
        //get string from textview
        gtk_text_buffer_get_bounds(buffer1, &start1, &end1);
        gchar *string1=gtk_text_buffer_get_text(buffer1, &start1, &end1, TRUE);

        //get value of radiobutton.
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }

        //check the contrast matrix for errors
        check1=contrast_matrix_validation(string1, &rows, &columns);
        g_print("Check = %i rows = %i columns = %i\n", check1, rows, columns);
  
        //check critical value 0<x<0.3
        check2=critical_value_changed_validation(entry1);

        //build contrast matrix
        if(check1==0&&check2==0)
          {
            numbers=rows*columns;
            SuppliedContrasts=gsl_matrix_alloc(rows, columns);
            check3=build_contrast_matrix_from_array(string1, SuppliedContrasts, rows, columns, numbers);
          }

        //do T2 calculations
        if(check1==0&&check2==0&&check3==0)
          {
            if(groups_database_validation(NULL)==0)
              {
                hotellings_T2(radio_button, alpha, 0, -1, textview, progress, SuppliedContrasts, columns);
              }
          }

        if(SuppliedContrasts!=NULL)
          {
           gsl_matrix_free(SuppliedContrasts);
          }
        g_free(string1);
      }
    g_print("Hotelling's T2 Finished\n");
    gtk_widget_destroy(dialog);
  }
static void exit_comparison_loop_event(GtkWidget *dialog , gint response, gpointer data)
  {
    if(response==GTK_RESPONSE_CANCEL||response==GTK_RESPONSE_CLOSE)
      {
        g_print("Exit Loop\n");
        //gtk_widget_destroy(dialog);
        iBreakLoop=1;//Break out of multiple comparison loop.
      }
  }
static void exit_hotelling_dialog(GtkWidget *dialog , gint response, gpointer data)
  {
    if(response==GTK_RESPONSE_CANCEL||response==GTK_RESPONSE_CLOSE)
      {
        g_print("Hotelling's T2 Dialog Cancelled.\n");
        //gtk_widget_destroy(dialog);
      }
  }
static void permutations_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *label4, *label5, *label6, *label7, *label8, *entry1, *entry2, *entry3, *radio1, *radio2, *radio3, *radio4, *random_radio1, *random_radio2, *random_radio3, *tail_combo, *test_combo, *p_function, *progress, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Permutation Testing", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Picks");

    random_radio1=gtk_radio_button_new_with_label(NULL, "Mersenne Twister 19937");
    random_radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(random_radio1), "Tausworthe 2");
    random_radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(random_radio1), "RANLUX 389 ");

    label1=gtk_label_new(" Build Auxiliary Table First. Data Pulled From the Database.\n Control is the Groups or Picks Value From the Auxiliary Table.");
    label2=gtk_label_new("Control GROUP BY Value");
    label3=gtk_label_new("Number of Permutations");
    label4=gtk_label_new("Random Number Generators");
    label5=gtk_label_new("Seed Value");
    label6=gtk_label_new("Probability Side");
    label7=gtk_label_new("Test Statistic");
    label8=gtk_label_new("Probability Function");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "1");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
    gtk_entry_set_text(GTK_ENTRY(entry2), "10000");

    entry3=gtk_entry_new();
    gtk_widget_set_halign(entry3, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 8);
    gtk_entry_set_text(GTK_ENTRY(entry3), "0");

    tail_combo=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tail_combo), "0", "abs");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tail_combo), "1", "upper");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(tail_combo), "2", "lower");
    gtk_combo_box_set_active(GTK_COMBO_BOX(tail_combo), 0);

    test_combo=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(test_combo), "0", "Mean Difference");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(test_combo), "1", "Welch's t-test");
    gtk_combo_box_set_active(GTK_COMBO_BOX(test_combo), 0);

    p_function=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(p_function), "0", "Unadjusted P");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(p_function), "1", "minP");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(p_function), "2", "maxT");
    gtk_combo_box_set_active(GTK_COMBO_BOX(p_function), 0);

    progress=gtk_progress_bar_new();
    gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 1, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 1, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 1, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 3, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), tail_combo, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 2, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), test_combo, 3, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label8, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), p_function, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 1, 8, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), random_radio1, 1, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), random_radio2, 1, 10, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), random_radio3, 1, 11, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 1, 12, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 2, 12, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), progress, 1, 13, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
     
    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_comparison_loop_event), NULL);

    gtk_widget_show_all(dialog);

    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        printf("Begin Permutation Calculation\n");
        gint check1=0;
        gint check2=0;
        gint radio_button=1;
        gint random_button=1;
        gint tail=gtk_combo_box_get_active(GTK_COMBO_BOX(tail_combo))+1;
        gint test=gtk_combo_box_get_active(GTK_COMBO_BOX(test_combo))+1;
        gint function=gtk_combo_box_get_active(GTK_COMBO_BOX(p_function))+1;
        gint control=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gint permutations=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        gint seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry3)));
         
        //Get value of radiobutton.
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }

        //Get random generator number.
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_radio1)))
          {
            random_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_radio2)))
          {
            random_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_radio3)))
          {
            random_button=3;
          }
  
        //Check that values are in the database.
        if(radio_button==1||radio_button==2)
          {
            check1=groups_database_validation(entry1);
          }
        if(radio_button==3||radio_button==4)
          {
            check2=picks_database_validation(entry1);
          }  

        //Set some bounds for the number of permutations
        if(permutations<10||permutations>500000)
          {
            g_print("Permutations Bounds 10<=x<=500000\n");
            simple_message_dialog("Permutations Bounds 10<=x<=500000");
          }
        else if(seed_value<0||seed_value>100000000)
          {
            g_print("Seed Value Bounds 0<=x<=10000000\n");
            simple_message_dialog("Seed Value Bounds 0<=x<=10000000");
          }
        else if(check1!=0||check2!=0)
          {
            //Exit. Message dialog in database validation functions.
            g_print("Exit Permutations\n");
          }
        else
          {
            if(function==1)
              {
                unadjusted_p_sql(permutations, radio_button, control, tail, test, function ,textview, GTK_PROGRESS_BAR(progress), &iBreakLoop, seed_value, random_button);
              }
            if(function==2||function==3)
              {
               unadjusted_p_sql(permutations, radio_button, control, tail, test, function , textview, GTK_PROGRESS_BAR(progress), &iBreakLoop, seed_value, random_button);
               if(iBreakLoop==0)
                 {
                   minP_sql(permutations, radio_button, control, tail, test , function, textview, GTK_PROGRESS_BAR(progress), &iBreakLoop, seed_value, random_button);
                 }
               else
                 {
                   iBreakLoop=0;
                 }
              }          
          }         
      }
    g_print("Permutations Finished\n");
    gtk_widget_destroy(dialog);    
  }
static void z_factor_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *entry1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Calculate Z-factor", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Data By Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Picks");

    label1=gtk_label_new(" Build Auxiliary Table First. Data Pulled From the Database.\n Control is the Groups or Picks Value From the Auxiliary Table.");
    label2=gtk_label_new("Control GROUP BY Value");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "1");
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 5, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_z_factor_dialog), NULL);

    gtk_widget_show_all(dialog);

    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Begin Z-factor Calculation\n");
        gint radio_button=1;
        gint control=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gint check1=0; 
         
        //get value of radiobutton.
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }
        else
          {
            //exit
          }

        //check entry is in database
        if(radio_button==1||radio_button==2)
          {
            check1=groups_database_validation(entry1);
          }
        if(radio_button==3||radio_button==4)
          {
            check1=picks_database_validation(entry1);
          }

        if(check1==0)
          {
            z_factor(radio_button, control, textview);
          }
      }
    g_print("Z-factor Finished\n");
    gtk_widget_destroy(dialog);
  }
static void exit_z_factor_dialog(GtkWidget *dialog , gint response, gpointer data)
  {
    if(response==GTK_RESPONSE_CANCEL||response==GTK_RESPONSE_CLOSE)
      {
        printf("Z-factor Dialog Cancelled.\n");
        //gtk_widget_destroy(dialog);
      }
  }
static void contingency_dialog(GtkWidget *menu, GtkTextView *textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *label4, *label5, *entry1, *entry2, *entry3, *check_button1, *check_button2, *check_button3, *check_button4, *check_button5, *check_button6, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Contingency Data", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("Data is pulled from the database as a single column.\nCalculations done in RC format. Warning! This is summed\ndata. Only the data needs to be loaded into the database.\nNo percent column or auxiliary table is needed for this\ncalculation.");
    label2=gtk_label_new("Rows");
    label3=gtk_label_new("Columns");
    label4=gtk_label_new("Chi-squared Critical Value");
    label5=gtk_label_new("Measures of Association");

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
    gtk_entry_set_text(GTK_ENTRY(entry1), "3");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 5);
    gtk_entry_set_text(GTK_ENTRY(entry2), "3");

    entry3=gtk_entry_new();
    gtk_widget_set_halign(entry3, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 5);
    gtk_entry_set_text(GTK_ENTRY(entry3), "0.05");

    check_button1=gtk_check_button_new_with_label("Pearson C");
    check_button2=gtk_check_button_new_with_label("Pearson C*");
    check_button3=gtk_check_button_new_with_label("Tshuprow's T");
    check_button4=gtk_check_button_new_with_label("Cramer's V");
    check_button5=gtk_check_button_new_with_label("Somer's d");
    check_button6=gtk_check_button_new_with_label("Gamma");
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button1, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button2, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button3, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button4, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button5, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button6, 1, 7, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    //g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_z_factor_dialog), NULL);

    gtk_widget_show_all(dialog);

    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Begin Contingency Calculation\n");
        gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        gdouble alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry3)));
        gint plates=atoi(pPlateNumberText);
        gint check_box1=0;
        gint check_box2=0;
        gint check_box3=0;
        gint check_box4=0;
        gint check_box5=0;
        gint check_box6=0;
        gint check1=0;
        gint check2=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button1)))
          {
            check_box1=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button2)))
          {
            check_box2=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button3)))
          {
            check_box3=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button4)))
          {
            check_box4=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button5)))
          {
            check_box5=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button6)))
          {
            check_box6=1;
          }

        g_print("Check Boxes %i %i %i %i %i %i\n", check_box1, check_box2, check_box3, check_box4, check_box5, check_box6);     
        g_print("Row=%i Columns=%i Plates=%i Alpha=%f\n", rows, columns, plates, alpha);

        //check entry boxes
        if(rows<1||columns<1)
          {
            check1=1;
            simple_message_dialog("The Rows or Columns is Less Than 1.");
          }
        check2=critical_value_changed_validation(entry3);

        if(check1==0&&check2==0)
          {
            build_temp_table_for_contingency(plates, rows, columns);
            calculate_contingency_values(alpha, textview, check_box1, check_box2, check_box3, check_box4, check_box5, check_box6);
          }       
      }
    g_print("Contingency Finished\n");
    gtk_widget_destroy(dialog);
  }
static void database_to_scatter_graph_dialog(GtkWidget *menu , gpointer data)
  {
    GtkWidget *dialog, *grid, *entry1, *entry2, *label1, *label2, *radio1, *radio2, *combo1, *content_area, *action_area;
    gint result;
    
    g_print("Send Data from Database to Graph\n");

    dialog=gtk_dialog_new_with_buttons("Scatter Plot", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Data Scatter Plot");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Percent Scatter Plot");
     
    label1=gtk_label_new("Records Lower Bound");
    label2=gtk_label_new("Records Upper Bound");     

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
    gtk_entry_set_text(GTK_ENTRY(entry1), "1");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
    gtk_entry_set_text(GTK_ENTRY(entry2), "100");

    combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "wxt terminal");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "x11 terminal");
    gtk_widget_set_hexpand(combo1, TRUE);  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 4, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint lower_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gint upper_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        gint combo=gtk_combo_box_get_active(GTK_COMBO_BOX(combo1));
        gint radio_button=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }

        if(lower_bound>0&&upper_bound>0&&lower_bound<=upper_bound)
          {
            if(groups_database_validation(NULL)==0)
              {
                database_to_scatter_graph_sql(radio_button, combo, lower_bound, upper_bound);
              }
          }
        else
          {
            g_print("Check Upper and Lower Bounds\n");
            simple_message_dialog("Check Upper and Lower Bounds.");
          }
      }
    gtk_widget_destroy(dialog);
  }
static void database_to_error_graph_dialog(GtkWidget *menu , gpointer data)
  {
    GtkWidget *dialog, *grid, *entry1, *entry2, *label1, *label2, *label3, *label4, *radio1, *radio2, *radio3, *radio4, *radio5, *radio6, *radio7, *combo1, *content_area, *action_area;
    gint result;
    
    g_print("Send Data from Database to Graph\n");

    dialog=gtk_dialog_new_with_buttons("Error Plot", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Error Plot by Data and Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Error Plot by Percent and Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Error Plot by Data and Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Error Plot by Percent and Picks");

    radio5=gtk_radio_button_new_with_label(NULL, "Error Plot 1 * STDEV");
    radio6=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio5), "Error Plot 2 * STDEV");
    radio7=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio5), "Error Plot 3 * STDEV");

    label1=gtk_label_new("Build Auxiliary Table for Box Plots");
    label2=gtk_label_new("Standard Deviation");
    label3=gtk_label_new("Records Lower Bound");
    label4=gtk_label_new("Records Upper Bound");     

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
    gtk_entry_set_text(GTK_ENTRY(entry1), "1");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
    gtk_entry_set_text(GTK_ENTRY(entry2), "10");

    combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "wxt terminal");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "x11 terminal");
    gtk_widget_set_hexpand(combo1, TRUE);  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio5, 0, 6, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio6, 0, 7, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio7, 0, 8, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 11, 2, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint lower_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gint upper_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        gint combo=gtk_combo_box_get_active(GTK_COMBO_BOX(combo1));
        gint radio_button1=0;
        gint radio_button2=0;
  
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button1=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button1=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button1=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button1=4;
          }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio5)))
          {
            radio_button2=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio6)))
          {
            radio_button2=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio7)))
          {
            radio_button2=3;
          }
        
        if(lower_bound>0&&upper_bound>0&&lower_bound<=upper_bound)
          {
            if(groups_database_validation(NULL)==0)
              {
                database_to_error_graph_sql(radio_button1, radio_button2, combo, lower_bound, upper_bound);
              }
          }
        else
          {
            g_print("Check Upper and Lower Bounds\n");
            simple_message_dialog("Check Upper and Lower Bounds.");
          }
        
      }
    gtk_widget_destroy(dialog);
  }
static void database_to_box_graph_dialog(GtkWidget *menu , gpointer data)
  {
    GtkWidget *dialog, *grid, *entry1, *entry2, *label0, *label1, *label2, *radio1, *radio2, *radio3, *radio4, *combo1, *content_area, *action_area;
    gint result;
    
    g_print("Send Data from Database to Graph\n");

    dialog=gtk_dialog_new_with_buttons("Box Plot", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Box Plot by Data and Groups");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Box Plot by Percent and Groups");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Box Plot by Data and Picks");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Box Plot by Percent and Picks");
     
    label0=gtk_label_new("Build Auxiliary Table for Box Plots");
    label1=gtk_label_new("Records Lower Bound");
    label2=gtk_label_new("Records Upper Bound");     

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
    gtk_entry_set_text(GTK_ENTRY(entry1), "1");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
    gtk_entry_set_text(GTK_ENTRY(entry2), "10");

    combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "wxt terminal");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "x11 terminal");
    gtk_widget_set_hexpand(combo1, TRUE);  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label0, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 7, 2, 1);   
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint lower_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gint upper_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        gint combo=gtk_combo_box_get_active(GTK_COMBO_BOX(combo1));
        gint radio_button=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            radio_button=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            radio_button=4;
          }
        
        if(lower_bound>0&&upper_bound>0&&lower_bound<=upper_bound)
          {
            if(groups_database_validation(NULL)==0)
              {
                database_to_box_graph_sql(radio_button, combo, lower_bound, upper_bound);
              }
          }
        else
          {
            g_print("Check Upper and Lower Bounds\n");
            simple_message_dialog("Check Upper and Lower Bounds.");
          }
              
       }
    gtk_widget_destroy(dialog);
  }
static void about_dialog(GtkWidget *menu, GtkWidget *window)
  {
    const gchar *authors[]={"C. Eric Cashon", "Including the fine art.", "Check the references file\n for more author details.", NULL};

    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog), pMainWindow);
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Ordered Set VelociRaptor");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A mean number crunching machine");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2015 C. Eric Cashon");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static GdkPixbuf* draw_velociraptor()
  {
    //Some amateur drawing and cropping of the program dino. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1024, 576);
    cairo_t *cr=cairo_create(surface);
    cairo_pattern_t *pattern=NULL;
    int i=0;
    int move=330;
    int height=220;
    int width=250;
    double ScaleWidth=350;
    double ScaleHeight=350;
    int points[21][2] = { 
      { 40, 85 }, 
      { 105, 75 }, 
      { 140, 10 }, 
      { 165, 75 }, 
      { 490, 100 },
      { 790, 225 },
      { 860, 310 }, 
      //{ 900, 380 }, curve nose
      { 860, 420 },
      { 820, 380 },
      { 780, 420 },
      { 740, 380 },
      { 700, 420 },
      { 660, 380 },
      { 650, 385 },
      { 810, 520 }, 
      { 440, 540 },
      { 340, 840 },
      { 240, 840 },
      { 140, 200 },
      { 90, 125 },
      { 40, 85 } 
  };
    g_print("Draw Dino\n");
    
    //Scaled from a 1024x576 screen. Original graphic.
    ScaleWidth=width/1024.0;
    ScaleHeight=height/576.0;

    //Clear the surface.
    cairo_save(cr);
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_restore(cr);
    
    cairo_save(cr);
    //Draw raptor points and fill in green.
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    //Draw point to point.
    for(i=0; i<20; i++)
      {
        cairo_line_to(cr, points[i][0]+move, points[i][1]);
      }
    //Draw curve at nose.
    cairo_move_to(cr, 860+move, 310);
    cairo_curve_to(cr, 900+move, 380, 900+move, 380, 860+move, 420);
    cairo_close_path(cr);
    cairo_set_source_rgb(cr, 0, 1, 0);
    cairo_fill(cr);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Set up rotated black ellipses.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgba(cr, 0, 0, 0, 1);
    cairo_set_line_width(cr, 7.0);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    for( i=0; i<36; i+=2)
      {
        cairo_save(cr);
        cairo_rotate(cr, i*G_PI/36);
        cairo_scale(cr, 0.3, 1);
        cairo_arc(cr, 0, 0, 60, 0, 2 * G_PI);
        cairo_stroke(cr);
        cairo_restore(cr);
      }
    cairo_restore(cr);

    //Set up rotated purple ellipses.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgba(cr, 1, 0, 1.0, 1);
    cairo_set_line_width(cr, 3.0);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    for(i=1; i<36; i+=2)
      {
        cairo_save(cr);
        cairo_rotate(cr, i*G_PI/36);
        cairo_scale(cr, 0.3, 1);
        cairo_arc(cr, 0, 0, 60, 0, 2 * G_PI);
        cairo_stroke(cr);
        cairo_restore(cr);
      }
    cairo_restore(cr);

    //Pattern for the center eye ellipse.
    pattern = cairo_pattern_create_linear(-120.0, 30.0, 120.0, 30.0);
    cairo_pattern_add_color_stop_rgb(pattern, 0.1, 0, 0, 0);
    cairo_pattern_add_color_stop_rgb(pattern, 0.5, 0, 0.5, 1);
    cairo_pattern_add_color_stop_rgb(pattern, 0.9, 0, 0, 0);

    //Draw center elipse of eye.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    cairo_rotate(cr, 18 * G_PI/36);
    cairo_scale(cr, 0.3, 1);
    cairo_arc(cr, 0, 0, 60, 0, 2 * G_PI);
    cairo_close_path(cr);
    cairo_set_source(cr, pattern);
    cairo_fill(cr);
    cairo_restore(cr);

    //Draw center circle for the eye.
    cairo_save(cr);
    cairo_scale(cr, ScaleWidth, ScaleHeight);
    cairo_set_source_rgb(cr, 0, 0, 0);
    cairo_set_line_width(cr, 3);
    cairo_translate(cr, width/(3.0*ScaleWidth), height/(3.0*ScaleHeight));
    cairo_translate(cr, move, 0);
    cairo_rotate(cr, 18*G_PI/36);
    cairo_scale(cr, 0.3, 1);
    cairo_arc(cr, 0, 0, 15, 0, 2 * G_PI);
    cairo_close_path(cr);
    cairo_fill(cr);
    cairo_restore(cr);

    GdkPixbuf *dino=gdk_pixbuf_get_from_surface(surface, 0, 0, 350, 350);
    GdkPixbuf *crop_dino=gdk_pixbuf_new_subpixbuf(dino, 70, 0, 250, 250);

    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    cairo_pattern_destroy(pattern);
    return crop_dino;
  }
static gboolean draw_veloci_raptor_feet(GtkWidget *widget, cairo_t *cr, gpointer data)
  {   
    gint i=0;
    gint j=0;
    gint scale_width_count=0;
    gint foot_count=0;
    gint window_width=gtk_widget_get_allocated_width(GTK_WIDGET(data));
  
    scale_width_count=window_width-1024;

    if(scale_width_count<=0)
      {
        foot_count=22;
      }
    if(scale_width_count>0)
      {
        foot_count=22+scale_width_count%45;
      } 

    gint points[9][2] = { 
    { -70, -200 }, 
    { -50, -170 }, 
    { -40, -200 }, 
    { -30, -170 }, 
    { -10, -200 },
    { -20, -140 }, 
    { -20, -90 },
    { -60, -160 }, 
    { -70, -200 } 
    };

    gint points2[9][2] = { 
    { 70, -40 }, 
    { 50, -10 }, 
    { 40, -40 }, 
    { 30, -10 }, 
    { 10, -40 },
    { 20, 20 }, 
    { 20, 70 },
    { 50, 20 }, 
    { 70, -40 } 
    };
  
    cairo_set_line_width(cr, 4);   
    cairo_translate(cr, 0, 15);
    cairo_rotate(cr, G_PI/2);
    cairo_scale(cr, 0.15, 0.15);
    cairo_set_source_rgb(cr, 0, 1, 0);
 
    for(i=0;i<foot_count;i++)
      {      
        for(j=0;j<8;j++)
          {
            cairo_line_to(cr, points[j][0], points[j][1]);
          }

        cairo_close_path(cr);
        cairo_stroke_preserve(cr);
        cairo_fill(cr);

        for(j=0;j<8;j++)
          {
            cairo_line_to(cr, points2[j][0], points2[j][1]);
          }

        cairo_close_path(cr);
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
        cairo_translate(cr, 0, -340); 
      }

    return TRUE;
  }
static void get_text_file(GtkWidget *menu, GtkWidget *window)
  {
    //Import a simple text file into the application.
    GtkButton *button=NULL;
    GFile *text_file;
    GFileInputStream *file_stream=NULL;
    gssize length;
    GFileInfo *file_info;
    gint file_size = -1;
    GtkWidget *dialog;
    gchar *text_buffer=NULL;
    GString *temp_buffer=g_string_new(NULL);
    GArray *data_array=NULL;
    gchar *p_char=NULL;
    guint32 counter=0;
    gint text_present=0;
    gint line_number=0;
    double temp=0;
    gboolean new_number=FALSE;

    if(iReferenceCountDialogWindow==0)
      {
        g_print("Import Text File\n");

        dialog=gtk_file_chooser_dialog_new("Open Text File",GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, "Open", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);

        gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
        gint result=gtk_dialog_run(GTK_DIALOG(dialog));

        if(result==GTK_RESPONSE_ACCEPT)
          {
            text_file=gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
            file_stream=g_file_read(text_file, NULL, NULL);
            file_info=g_file_input_stream_query_info (G_FILE_INPUT_STREAM(file_stream),G_FILE_ATTRIBUTE_STANDARD_SIZE,NULL, NULL);     
            file_size=g_file_info_get_size(file_info);
            g_print("Text Length = %d\n", file_size);
            g_object_unref(file_info);
            text_buffer=(char *) malloc(sizeof(gchar) * file_size);
            memset(text_buffer, 0, file_size);
            length=g_input_stream_read(G_INPUT_STREAM(file_stream), text_buffer, file_size, NULL, NULL);
            g_print("Length of Buffer = %i\n", length);
            data_array=g_array_new(FALSE, FALSE, sizeof(gdouble));
            p_char=text_buffer;
            gtk_widget_destroy(dialog);

            //Ignore last newline character.
            while(counter<(length-1))
              {
                if(g_ascii_isdigit(*p_char)||*p_char =='.'||*p_char=='-')
                  {
                    g_string_append_printf(temp_buffer, "%c", *p_char);
                    new_number=TRUE;
                  }
                else if(*p_char=='\n'||*p_char==' '|| *p_char==',' || *p_char=='|')
                  {
                    if(new_number==TRUE)
                      {
                        temp=g_ascii_strtod(temp_buffer->str, NULL);
                        g_array_append_val(data_array, temp);
                        g_string_truncate(temp_buffer, 0);
                        new_number=FALSE;
                       }
                   }
                else
                   {
                     line_number=data_array[0].len + 1;
                     g_print("Not a Number at Number %i. Ignore value %c.\n", line_number, *p_char);
                     //Code change. Just ignore non-numbers when reading in text data.
                     //text_present=1;
                     //break;
                   }
                p_char++;
                counter++;
              }
            g_free(text_buffer);
            g_print("Counter=%i and Length=%i\n", counter, length);

            if(text_present==0)
              {
                test_data_button_clicked(button, data_array,0,0,0);
                g_print("Imported Text File\n");
              }
            else
              {
                g_print("Unsuccessful Import\n");
                simple_message_dialog("Import unsuccessful! Check text file format\n and try again with the terminal window open\n to see what lines caused the errors.");
              }
            g_array_free(data_array, TRUE);
          }  
        else
          {
            gtk_widget_destroy(dialog);
          }
      } 
    else
      {
        simple_message_dialog("Only one initial dataset can be opened at a time!");
      } 

    g_string_free(temp_buffer, TRUE);
       
  }
static void append_text_dialog(GtkWidget *menu, GtkWidget *window)
  {
    GtkWidget *dialog, *label, *entry, *grid, *content_area;
    gint result=0;

    dialog=gtk_dialog_new_with_buttons("Append Text Files", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Cancel", GTK_RESPONSE_CANCEL, "Select Text Files", GTK_RESPONSE_OK, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 280, 100);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    label=gtk_label_new("Combined Text File Name");
    gtk_widget_set_hexpand(label, TRUE);

    entry=gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry), "combined.txt");

    GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
    g_array_append_val(widgets, window);
    g_array_append_val(widgets, entry);

    grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    gtk_widget_show_all(dialog);
    
    result=gtk_dialog_run(GTK_DIALOG(dialog));
    if(result==GTK_RESPONSE_OK)
      {
        gtk_widget_hide(dialog);
        show_file_dialog_append(widgets);
      }

    g_array_free(widgets, TRUE);
    gtk_widget_destroy(dialog);

  }
static void show_file_dialog_append(GArray *widgets)
  {
    GtkWidget *dialog;
    
    dialog=gtk_file_chooser_dialog_new("Open Text File",GTK_WINDOW(g_array_index(widgets, GtkWidget*, 0)), GTK_FILE_CHOOSER_ACTION_OPEN, "Open", GTK_RESPONSE_ACCEPT, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), TRUE);
    
    gint result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_ACCEPT)
      {
        GSList *text_file_paths=gtk_file_chooser_get_filenames(GTK_FILE_CHOOSER(dialog));
        const gchar *new_file_name=gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 1)));
        append_text_files(text_file_paths, new_file_name);
        g_slist_free_full(text_file_paths, g_free);
      }

    gtk_widget_destroy(dialog);

  }
static void append_text_files(GSList *text_file_paths, const gchar *new_file_name)
  {
    gint selected_files=g_slist_length(text_file_paths);
    gint file_size=0;
    gint i=0;
    size_t bytes=0;
    FILE *combined=NULL;
    gboolean null_pointer=FALSE;
    
    g_print("Selected Files %i\n", selected_files);
    for(i=0;i<selected_files;i++)
      {
        g_print("%i %s\n", i, (char*)g_slist_nth_data(text_file_paths, i));
      }
     
    combined=fopen(new_file_name, "a");

    for(i=0;i<selected_files;i++)
      {
        char *pTextBuffer=NULL;
        FILE *fp=NULL;
        fp=fopen((char*)g_slist_nth_data(text_file_paths, i), "r");
        if(fp!=NULL&&combined!=NULL)
          {
            fseek(fp , 0 , SEEK_END);
            file_size=ftell(fp);
            rewind(fp);
            printf("Filesize %i\n", file_size);
            pTextBuffer=(char *)malloc(sizeof(char) * file_size);
            if(pTextBuffer!=NULL)
              {
                //Just read and write to a file. No string functions.
                bytes=fread(pTextBuffer, file_size, 1, fp);
                if(bytes==0) g_print("Zero Bytes Read\n");
                bytes=fwrite(pTextBuffer, file_size, 1, combined);
                if(bytes==0) g_print("Zero Bytes Write\n");
              }
            else
              {
                g_print("Malloc Error\n");
                null_pointer=TRUE;
              }
            fclose(fp);
            if(pTextBuffer!=NULL) free(pTextBuffer);
          }
        else
          {
            g_print("File Opening Error.\n");
            null_pointer=TRUE;
          }
       }

    if(combined!=NULL) fclose(combined);    
    if(null_pointer==FALSE) g_print("%s Created\n", new_file_name);
    
  }
static void sqlite_connect_dialog(GtkWidget *menu, GtkWidget *window)
  {
    g_print("Sqlite Connect Dialog\n");
    GtkWidget *dialog, *label1, *entry1, *button1, *treeview1, *grid, *content_area, *scrolled_win;
    gint result=0;

    dialog=gtk_dialog_new_with_buttons("Sqlite Connect", GTK_WINDOW(window), GTK_DIALOG_MODAL, "Get Data", GTK_RESPONSE_OK, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 350);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);

    label1=gtk_label_new("Gets a single column of data.\nEnter the path of the database.\nConnect and get data.");

    entry1=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry1), "VelociRaptorData.db");
    gtk_widget_set_hexpand(entry1, TRUE);

    button1=gtk_button_new_with_label("Connect");

    //A treeview for tables and fields.
    treeview1=gtk_tree_view_new();
    gtk_widget_set_hexpand(treeview1, TRUE);
    gtk_widget_set_vexpand(treeview1, TRUE);
    GtkCellRenderer *renderer1=gtk_cell_renderer_text_new();
    g_object_set(G_OBJECT(renderer1),"foreground", "purple", NULL);
    GtkTreeViewColumn *column1=gtk_tree_view_column_new_with_attributes("Tables and Fields", renderer1, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview1), column1);

    scrolled_win=gtk_scrolled_window_new(NULL, NULL);    
    gtk_container_add(GTK_CONTAINER(scrolled_win), treeview1);

    //Pass entry and treeview to callback.
    GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
    g_array_append_val(widgets, entry1);
    g_array_append_val(widgets, treeview1);
    g_signal_connect(button1, "clicked", G_CALLBACK(connect_sqlite_db), widgets);
    //g_signal_connect(button2, "clicked", G_CALLBACK(get_treeview_selected), widgets);

    grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scrolled_win, 0, 3, 1, 1);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_container_add(GTK_CONTAINER(content_area), grid);

    gtk_widget_show_all(dialog);
    gtk_widget_grab_focus(button1);
    result=gtk_dialog_run(GTK_DIALOG(dialog));
    if(result==GTK_RESPONSE_OK)
      {
        //g_print("Get Data\n");
        get_treeview_selected(NULL, widgets);
      }

     gtk_widget_destroy(dialog);
  }
static void text_button_clicked(GtkButton *button, GtkTextView *textview)
  {
    //Clear the contents of the TextView.
    g_print("Text Cleared\n");
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter end;
   
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (textview));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_delete(buffer, &start, &end);

  }
static void clear_format_event(GtkButton *button, gpointer data)
  {
    g_print("Clear Format\n");
    GtkTextIter start1, end1;
    GtkTextBuffer *buffer;

    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW (data));
    gtk_text_buffer_get_bounds(buffer, &start1, &end1);
    gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);
  }
static void heatmap_dialog(GtkButton *button, gpointer data)
  {
    GtkWidget *dialog, *grid, *label1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    gint result;
   
    g_print("Heatmap Text\n");

    dialog=gtk_dialog_new_with_buttons("Heatmap Platemap", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("Heatmap Gradient");
    gtk_widget_set_hexpand(label1, TRUE);

    radio1=gtk_radio_button_new_with_label(NULL, "Heatmap(rgb)");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Heatmap(iris)");
    radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Heatmap(iris2)");
    radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Heatmap(sun)");
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_widget_set_margin_left(radio1, 30);
    gtk_widget_set_margin_left(radio2, 30);
    gtk_widget_set_margin_left(radio3, 30);
    gtk_widget_set_margin_left(radio4, 30);
    G_GNUC_END_IGNORE_DEPRECATIONS
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio3, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), radio4, 0, 4, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gdouble high=0;
        gdouble low=0;
        gdouble difference=0;
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            format_text_platemap_heatmap_high_low(GTK_TEXT_VIEW(data), &high, &low);
            difference=abs(high-low)/10.0;
            if(difference!=0)
              {
                g_print("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
                format_text_platemap_heatmap(GTK_TEXT_VIEW(data), high, low);
              }
            else
              {
                simple_message_dialog("Can't find a high and low number.");
              }
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            format_text_platemap_heatmap_high_low(GTK_TEXT_VIEW(data), &high, &low);
            difference=abs(high-low)/10.0;
            if(difference!=0)
              {
                g_print("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
                format_text_platemap_heatmap_iris(GTK_TEXT_VIEW(data), high, low);
              }
            else
              {
                simple_message_dialog("Can't find a high and low number.");
              }
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            format_text_platemap_heatmap_high_low(GTK_TEXT_VIEW(data), &high, &low);
            difference=abs(high-low)/10.0;
            if(difference!=0)
              {
                g_print("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
                format_text_platemap_heatmap_iris2(GTK_TEXT_VIEW(data), high, low);
              }
            else
              {
                simple_message_dialog("Can't find a high and low number.");
              }
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            format_text_platemap_heatmap_high_low(GTK_TEXT_VIEW(data), &high, &low);
            difference=abs(high-low)/10.0;
            if(difference!=0)
              {
                g_print("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
                format_text_platemap_heatmap_sun(GTK_TEXT_VIEW(data), high, low);
              }
            else
              {
                simple_message_dialog("Can't find a high and low number.");
              }
          }
      }
    gtk_widget_destroy(dialog);
  }
static void rise_fall_text_dialog(GtkButton *button, gpointer data)
  {
    GtkWidget *dialog, *grid, *entry1, *label1, *label2, *radio1, *radio2, *content_area, *action_area;
    gint result;
    guint32 buffer_count;
    //char cArrayNumber[25];//if a change of space format it can segfault.
    GString *array_number=g_string_new("");
    guint32 space=0;
    guint32 prev_space=0;
    guint32 two_prev_space=0;
    guint32 i=0;
    guint32 j=0;
    gfloat previous=G_MAXFLOAT;//cludge, start with max value of float for comparison rise.
    gfloat previous2=G_MINFLOAT;//first value to compare with fall run.
    gint a_switch=0;
    guint32 begin_iter=0;
    guint32 end_iter=0;
    gint set_size=2;
    gint size=5;
    char temp;
    GtkTextBuffer *buffer;
    GtkTextIter start1, end1, start_iter;
    GtkTextTagTable *tag_table;
    GtkTextTag *tag_rise_fall;

    g_print("RiseFall Text\n");

    dialog=gtk_dialog_new_with_buttons("RiseFall Platemap", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Rising Sets");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Declining Sets");
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_widget_set_margin_left(radio1, 20);
    gtk_widget_set_margin_left(radio2, 20);
    G_GNUC_END_IGNORE_DEPRECATIONS

    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
    gtk_entry_set_text(GTK_ENTRY(entry1), "5");

    label1=gtk_label_new("Set Runs");
    label2=gtk_label_new("Run Set Size   >=");
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    gtk_widget_set_margin_left(label1, 20);
    gtk_widget_set_margin_left(label2, 20);
    G_GNUC_END_IGNORE_DEPRECATIONS
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 3, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 20);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        //Get active widget value
        size=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
         
        buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW (data));
        tag_table=gtk_text_buffer_get_tag_table(buffer);
        tag_rise_fall=gtk_text_tag_table_lookup(tag_table, "purple_foreground");

        if(tag_rise_fall==NULL)
          {
            gtk_text_buffer_create_tag (buffer, "purple_foreground", "foreground", "Purple", NULL);  
          }
        else
          {
            //remove tags.
            gtk_text_buffer_get_bounds(buffer, &start1, &end1);
            gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);
          }

        buffer_count=gtk_text_buffer_get_char_count(buffer); 
        gtk_text_buffer_get_start_iter(buffer, &start_iter);

        //start of large block for radio1.
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            //Look for rise fall trends
            for(i=0; i<buffer_count; i++)
              {
                temp=gtk_text_iter_get_char(&start_iter);
                //ignore newlines
                if(temp!='\n')
                  {
                    if(g_ascii_isdigit(temp)|| temp=='.')
                      {
                        //cArrayNumber[j]=gtk_text_iter_get_char(&start_iter);
                        g_string_insert_c(array_number, j, gtk_text_iter_get_char(&start_iter)); 
                        j++;
                      }
                    if(temp==' ')
                      {
                        space=i;
                        //Two choices(>=,<=)
                        if(previous<atof(array_number->str)) //switch to less than falls run
                          {
                            if(a_switch==0)
                              {
                                //g_print("%f %f ",previous, atof(array_number->str));
                                begin_iter=two_prev_space+1;
                                a_switch=1;
                                set_size=2;
                              }
                            else
                              {
                                //g_print("%f ", atof(array_number->str));
                                //count sets of a specified size
                                set_size++;
                                if(i==buffer_count-3&&set_size>=size) //ends with " \n\n\n"
                                  {
                                    //end off of last space
                                    end_iter=space;
                                    //printf("\nBegin %i End %i\n", iBeginIter, iEndIter);
                                    gtk_text_buffer_get_iter_at_offset (buffer, &start1, begin_iter);
                                    gtk_text_buffer_get_iter_at_offset (buffer, &end1, end_iter);
                                    gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                  }
                              }
                           }
                         else
                           {
                             if(a_switch==1)
                               {
                                 //printf("\n");
                                 end_iter=prev_space;
                                 if(set_size>=size)
                                   {
                                     //printf("Begin %i End %i\n", iBeginIter, iEndIter);
                                     gtk_text_buffer_get_iter_at_offset (buffer, &start1, begin_iter);
                                     gtk_text_buffer_get_iter_at_offset (buffer, &end1, end_iter);
                                     gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                   }
                                 else
                                   {
                                     //printf("Iters Not Applied!\n");
                                   }
                                }
                              a_switch=0;
                            }
                          two_prev_space=prev_space;
                          prev_space=space;
                          previous=atof(array_number->str);
                          //memset(array_number, 0, 25);
                          g_string_truncate(array_number,0);
                          j=0;
                        }
                     }
                   gtk_text_iter_forward_chars(&start_iter, 1);
                 } //end of for loop 1.
             }//end of radio1.

          //start of large block for radio2.
          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              //Look for rise fall trends
              for(i=0; i<buffer_count; i++)
                {
                  temp=gtk_text_iter_get_char(&start_iter);
                  //ignore newlines
                  if(temp!='\n')
                    {
                      if(g_ascii_isdigit(temp)|| temp=='.')
                        {
                          //cArrayNumber[j]=gtk_text_iter_get_char(&start_iter);
                          g_string_insert_c(array_number, j, gtk_text_iter_get_char(&start_iter));
                          j++;
                         }
                       if(temp==' ')
                         {
                           space=i;
                           //Two choices(>=,<=)
                           if(previous2>atof(array_number->str)) //falls run
                             {
                               if(a_switch==0)
                                 {
                                   //g_print("%f %f ",previous2, atof(array_number->str));
                                   begin_iter=two_prev_space+1;
                                   a_switch=1;
                                   set_size=2;
                                 }
                               else
                                 {
                                   //g_print("%f ", atof(array_number->str));
                                   //count sets of a specified size
                                   set_size++;
                                   if(i==buffer_count-3) //ends with " \n\n\n"
                                     {
                                       //end off of last space
                                       end_iter=space;
                                       //g_print("\nBegin %i End %i\n", begin_iter, end_iter);
                                       gtk_text_buffer_get_iter_at_offset (buffer, &start1, begin_iter);
                                       gtk_text_buffer_get_iter_at_offset (buffer, &end1, end_iter);
                                       gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                      }
                                 }
                             }
                           else
                             {
                               if(a_switch==1)
                                 {
                                   g_print("\n");
                                   end_iter=prev_space;
                                   if(set_size>=size)
                                     {
                                       //g_print("Begin %i End %i\n", begin_iter, end_iter);
                                       gtk_text_buffer_get_iter_at_offset (buffer, &start1, begin_iter);
                                       gtk_text_buffer_get_iter_at_offset (buffer, &end1, end_iter);
                                       gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                     }
                                   else
                                     {
                                       //printf("Iters Not Applied!\n");
                                     }
                                 }
                               a_switch=0;
                             }
                           two_prev_space=prev_space;
                           prev_space=space;
                           previous2=atof(array_number->str);
                           // memset(cArrayNumber, 0, 25);
                           g_string_truncate(array_number,0);
                           j=0;
                         }
                     }
                   gtk_text_iter_forward_chars(&start_iter, 1);
                 } //end of for loop 2.
             }//end of radio2.         
        } //end of GTK_RESPONSE_OK

     g_string_free (array_number, TRUE);
     gtk_widget_destroy(dialog);
       
  }
static void heatmap_html_dialog(GtkButton *button, gpointer p)
  {
     GtkWidget *dialog, *grid, *entry1, *entry2, *label1, *label2, *label3, *label4, *label5, *label6, *radio1, *radio2, *combo1, *combo2, *combo3, *content_area, *action_area;
    gint result;
    
    g_print("Send Plate Data from Database to HTML\n");

    dialog=gtk_dialog_new_with_buttons("Heatmap Platemap HTML", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    radio1=gtk_radio_button_new_with_label(NULL, "Data to HTML");
    radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Percent to HTML");
     
    label1=gtk_label_new("Output data to platemap format in\n HTML. File name heatmap.html"); 
    label2=gtk_label_new("Rows");
    label3=gtk_label_new("Columns"); 
    label4=gtk_label_new("Precision");
    label5=gtk_label_new("Font Size"); 
    label6=gtk_label_new("Heatmap");  
         
    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
    gtk_entry_set_text(GTK_ENTRY(entry1), "8");

    entry2=gtk_entry_new();
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 3);
    gtk_entry_set_text(GTK_ENTRY(entry2), "12");

    combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "3", "3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "4", "4");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 2);

    combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "0", "6");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "1", "7");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "2", "8");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "3", "9");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "4", "10");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "5", "11");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "6", "12");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "7", "13");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "8", "14");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 4);

    combo3=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "0", "RGB");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "1", "iris1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "2", "iris2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "3", "sun");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio1, 0, 1, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), radio2, 0, 2, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 2, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 2, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 0, 6, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 0, 7, 2, 1);  

    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint rows=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));//const char
        gint columns=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));//const char
        gchar *prec1=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo1));
        gint precision=atoi(prec1);
        gchar *font1=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo2));
        gint font_size=atoi(font1);
        gint gradient=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo3)));
        gint radio_button=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            radio_button=1;
            heatmap_to_html_sql(radio_button, rows, columns+1, precision, font_size, gradient);
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            radio_button=2;
            heatmap_to_html_sql(radio_button, rows, columns+1, precision, font_size, gradient);
          }
  
        if(prec1!=NULL) g_free(prec1);
        if(font1!=NULL) g_free(font1);

       }
    gtk_widget_destroy(dialog);
  
  }
static void html_table_dialog(GtkButton *button, gpointer p)
  {
    GtkWidget *dialog, *grid, *textview, *scrolled_win, *label1, *label2, *label3, *label4, *label5, *label6, *label7, *label8, *label9, *combo1, *combo2, *combo3, *combo4, *combo5, *combo6, *combo7, *content_area, *action_area;
    GtkTextBuffer *buffer1;
    gint result;
    
    g_print("Send Tablular Data from Database to HTML\n");

    dialog=gtk_dialog_new_with_buttons("Tabular Data to HTML", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 450);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    textview=gtk_text_view_new();
    gtk_widget_set_vexpand(textview, TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    scrolled_win=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scrolled_win, TRUE); 
    gtk_container_add(GTK_CONTAINER(scrolled_win), textview);
    buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_insert_at_cursor(buffer1, "SELECT * FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID AND T1.KeyID<11;", -1);

    label1=gtk_label_new("Output data in tabular format in\nHTML. File name table.html");
    label2=gtk_label_new("SQL Statement");      
    label3=gtk_label_new("Precision");
    label4=gtk_label_new("Font Size");
    label5=gtk_label_new("Font Color");
    label6=gtk_label_new("Field Background Color");
    label7=gtk_label_new("Data Background Color"); 
    label8=gtk_label_new("Alternate Color"); 
    label9=gtk_label_new("Data Background Color2"); 
     
    combo1=gtk_combo_box_text_new();     
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "0");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "1");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "2");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "3", "3");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "4", "4");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 2);

    combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "0", "6");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "1", "7");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "2", "8");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "3", "9");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "4", "10");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "5", "11");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "6", "12");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "7", "13");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo2), "8", "14");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 4);

    //The 17 html standard colors are: aqua, black, blue, fuchsia, gray, green, lime, maroon, navy, olive, orange, purple, red, silver, teal, white, and yellow.

    combo3=gtk_combo_box_text_new();     
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "0", "black");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "1", "white");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "2", "blue");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "3", "orange");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo3), "4", "olive");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);

    combo4=gtk_combo_box_text_new();     
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "0", "silver");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "1", "blue");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "2", "black");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "3", "purple");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "4", "red");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "5", "maroon");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo4), "6", "teal");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo4), 0);

    combo5=gtk_combo_box_text_new();     
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "0", "white");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "1", "green");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "2", "navy");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "3", "lime");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "4", "yellow");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "5", "aqua");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo5), "6", "fuchsia");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo5), 0);

    combo6=gtk_combo_box_text_new();     
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo6), "0", "Constant");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo6), "1", "By Row");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo6), "2", "By Column");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo6), 0);

    combo7=gtk_combo_box_text_new();     
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "0", "white");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "1", "green");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "2", "navy");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "3", "lime");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "4", "yellow");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "5", "aqua");
    gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo7), "6", "fuchsia");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo7), 1);
     
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 4, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), scrolled_win, 0, 2, 4, 2);
    gtk_grid_attach(GTK_GRID(grid), combo1, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo4, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo5, 3, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo6, 3, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo7, 3, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label8, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label9, 2, 6, 1, 1);

    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gint check=0;
        gchar database_name[]="VelociRaptorData.db";
        gchar html_file_name[]="table.html";
        GtkTextIter start1;
        GtkTextIter end1;
   
        gchar *prec1=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo1));
        gint precision=atoi(prec1);
        gchar *font1=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo2));
        gint font_size=atoi(font1);
        gchar *font_color=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo3));
        gchar *field_bg_color=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo4));
        gchar *bg_color1=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo5));
        gint alternate=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo6)));
        gchar *bg_color2=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo7));
        gtk_text_buffer_get_bounds(buffer1, &start1, &end1);
        gchar *sql=gtk_text_buffer_get_text(buffer1, &start1, &end1, TRUE);
      
        check=check_sql_for_select(sql);
    
        if(check==0)
          {
            g_print("%s\n", sql);
            parse_sql_field_names(html_file_name, database_name, sql, precision, font_size, bg_color1, bg_color2, alternate, field_bg_color, font_color);
      }
        else
          {
            g_print("Unable to parse SQL statement.\n");
            simple_message_dialog("Unable to parse SQL statement.");
          }

        if(prec1!=NULL) g_free(prec1);
        if(font1!=NULL) g_free(font1);
        if(font_color!=NULL) g_free(font_color);
        if(field_bg_color!=NULL) g_free(field_bg_color);
        if(bg_color1!=NULL) g_free(bg_color1);
        if(bg_color2!=NULL) g_free(bg_color2);
        if(sql!=NULL) g_free(sql);
       
       }
    gtk_widget_destroy(dialog);
  
  }
static void send_text_to_database_dialog(GtkButton* button, gpointer textview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *entry1, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Send Text To Database", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("Send the text data in the textview\n to the database.");
    label2=gtk_label_new("Table Name");
    
    entry1=gtk_entry_new();
    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 12);
    gtk_entry_set_text(GTK_ENTRY(entry1), "AnalysisData");
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 1, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
        guint length=gtk_text_buffer_get_char_count(buffer);
        const gchar *table_name=gtk_entry_get_text(GTK_ENTRY(entry1));
        if(length>5)
          {
            send_text_to_database(table_name, textview);
          }
        else
          {
            printf("There Isn't Any Tabular Data on the Whiteboard.\n");
            simple_message_dialog("There isn't any tabular data\non the white board.");
          }
      }
    gtk_widget_destroy(dialog);
     
  }
static void build_aux_table_dialog(GtkWidget *menu, GtkWidget *window)
  {
    GtkWidget *dialog, *grid, *label0, *label1, *label2, *label3, *label4, *label5, *label6, *label7, *pick_entry1, *pick_entry2, *pick_entry3, *pick_entry4, *value_entry1, *value_entry2, *value_entry3, *value_entry4, *check_button1, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Build Auxiliary Table", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);
     
    label0=gtk_label_new("Groups Field Built Automatically From Set Size For Stats. Press OK For Default Table.\nPicks Field Values Need To Be In Sequential Order Starting From 1.");
    label1=gtk_label_new("Picks Location 1");
    label2=gtk_label_new("Picks Location 2");
    label3=gtk_label_new("Picks Location 3");
    label4=gtk_label_new("Picks Location 4");
    label5=gtk_label_new("Picks Value");
    label6=gtk_label_new("Location");
    label7=gtk_label_new("*Optional Picks");
    
    pick_entry1=gtk_entry_new();
    pick_entry2=gtk_entry_new();
    pick_entry3=gtk_entry_new();
    pick_entry4=gtk_entry_new();
    value_entry1=gtk_entry_new();
    value_entry2=gtk_entry_new();
    value_entry3=gtk_entry_new();
    value_entry4=gtk_entry_new();

    gtk_widget_set_halign(pick_entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(pick_entry2, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(pick_entry3, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(pick_entry4, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry2, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry3, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry4, GTK_ALIGN_CENTER);

    gtk_entry_set_width_chars(GTK_ENTRY(pick_entry1), 25);
    gtk_entry_set_width_chars(GTK_ENTRY(pick_entry2), 25);
    gtk_entry_set_width_chars(GTK_ENTRY(pick_entry3), 25);
    gtk_entry_set_width_chars(GTK_ENTRY(pick_entry4), 25);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 4);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 4);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry3), 4);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry4), 4);

    gtk_entry_set_text(GTK_ENTRY(pick_entry1), "1,2,3,4");
    gtk_entry_set_text(GTK_ENTRY(pick_entry2), "5,6,7,8");
    gtk_entry_set_text(GTK_ENTRY(pick_entry3), "9,10,11,12");
    gtk_entry_set_text(GTK_ENTRY(pick_entry4), "13,14,15,16");
    gtk_entry_set_text(GTK_ENTRY(value_entry1), "1");
    gtk_entry_set_text(GTK_ENTRY(value_entry2), "2");
    gtk_entry_set_text(GTK_ENTRY(value_entry3), "3");
    gtk_entry_set_text(GTK_ENTRY(value_entry4), "4");

    g_signal_connect(G_OBJECT(pick_entry1), "focus_out_event", G_CALLBACK(control_changed), NULL);
    g_signal_connect(G_OBJECT(pick_entry2), "focus_out_event", G_CALLBACK(control_changed), NULL); 
    g_signal_connect(G_OBJECT(pick_entry3), "focus_out_event", G_CALLBACK(control_changed), NULL); 
    g_signal_connect(G_OBJECT(pick_entry4), "focus_out_event", G_CALLBACK(control_changed), NULL); 

    check_button1=gtk_check_button_new_with_label("Append Picks Values to Existing Table");  
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label0, 0, 0, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pick_entry1, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry1, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pick_entry2, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry2, 2, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pick_entry3, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry3, 2, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), pick_entry4, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry4, 2, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check_button1, 1, 6, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Build Auxiliary Table\n");
        //get from global variables
        gint plate_number=atoi(pPlateNumberText);
        gint plate_size=atoi(pPlateSizeText);
        gint group_size=atoi(pPlateStatsText); 
           
        GString *buffer=g_string_new("");
        GArray *pick_groups_array1=g_array_new(FALSE, FALSE, sizeof(gint));
        GArray *pick_groups_array2=g_array_new(FALSE, FALSE, sizeof(gint));
        GArray *pick_groups_array3=g_array_new(FALSE, FALSE, sizeof(gint));
        GArray *pick_groups_array4=g_array_new(FALSE, FALSE, sizeof(gint));
        gint pick_groups_array_length1=0;
        gint pick_groups_array_length2=0;
        gint pick_groups_array_length3=0;
        gint pick_groups_array_length4=0;
        const gchar *pick_groups1=gtk_entry_get_text(GTK_ENTRY(pick_entry1));
        const gchar *pick_groups2=gtk_entry_get_text(GTK_ENTRY(pick_entry2));
        const gchar *pick_groups3=gtk_entry_get_text(GTK_ENTRY(pick_entry3));
        const gchar *pick_groups4=gtk_entry_get_text(GTK_ENTRY(pick_entry4));
        gint value1=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry1)));
        gint value2=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
        gint value3=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry3)));
        gint value4=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry4)));
        gint pick_groups1_length=strlen(pick_groups1);
        gint pick_groups2_length=strlen(pick_groups2);
        gint pick_groups3_length=strlen(pick_groups3);
        gint pick_groups4_length=strlen(pick_groups4);
        gint temp;

        gint array_length=plate_size*plate_number;
        gint array_plate[array_length];
        gint array_wells[array_length];
        gint array_picks[array_length];
        gint array_groups[array_length]; 
        gint counter=0;
        gint label=1;
        gint i=0;
        gint j=0;
        gint k=0;
           
        //Load pick entry values into int arrays.
        if(pick_groups1_length>0)
          {
            for(i=0;i<pick_groups1_length;i++)
              {
                if(pick_groups1[i]!=','|| i==pick_groups1_length-1)
                  {
                    g_string_append_printf(buffer,"%c", pick_groups1[i]);
                  }
                else
                  {
                    temp=atoi(buffer->str);
                    temp=temp-1;
                    g_array_append_val(pick_groups_array1,temp);
                    g_string_truncate(buffer,0);
                  }
               }
             temp=atoi(buffer->str);
             temp=temp-1;
             g_array_append_val(pick_groups_array1,temp);
             pick_groups_array_length1=pick_groups_array1[0].len;
             g_string_truncate(buffer,0);
           }
       
         if(pick_groups2_length>0)
           {
             for(i=0;i<pick_groups2_length;i++)
               {
                 if(pick_groups2[i]!=','|| i==pick_groups2_length-1)
                   {
                     g_string_append_printf(buffer,"%c", pick_groups2[i]);
                   }
                 else
                   {
                     temp=atoi(buffer->str);
                     temp=temp-1;
                     g_array_append_val(pick_groups_array2, temp);
                     g_string_truncate(buffer, 0);
                   }
                }
              temp=atoi(buffer->str);
              temp=temp-1;
              g_array_append_val(pick_groups_array2,temp);
              pick_groups_array_length2=pick_groups_array2[0].len;
              g_string_truncate(buffer,0);
            }

          if(pick_groups3_length>0)
            {
              for(i=0;i<pick_groups3_length;i++)
                {
                  if(pick_groups3[i]!=','|| i==pick_groups3_length-1)
                    {
                      g_string_append_printf(buffer,"%c", pick_groups3[i]);
                    }
                  else
                    {
                      temp=atoi(buffer->str);
                      temp=temp-1;
                      g_array_append_val(pick_groups_array3,temp);
                      g_string_truncate(buffer, 0);
                    }
                 }
               temp=atoi(buffer->str);
               temp=temp-1;
               g_array_append_val(pick_groups_array3,temp);
               pick_groups_array_length3=pick_groups_array3[0].len;
               g_string_truncate(buffer,0);
             }

             if(pick_groups4_length>0)
               {
                for(i=0;i<pick_groups4_length;i++)
                  {
                    if(pick_groups4[i]!=','|| i==pick_groups4_length-1)
                      {
                        g_string_append_printf(buffer,"%c", pick_groups4[i]);
                      }
                    else
                      {
                        temp=atoi(buffer->str);
                        temp=temp-1;
                        g_array_append_val(pick_groups_array4,temp);
                        g_string_truncate(buffer,0);
                      }
                 }
               temp=atoi(buffer->str);
               temp=temp-1;
               g_array_append_val(pick_groups_array4, temp);
               pick_groups_array_length4=pick_groups_array4[0].len;
               g_string_truncate(buffer,0);
             }

             //Label Plate Numbers.
             for(i=0;i<plate_number;i++)
               {
                 for(j=0;j<plate_size;j++)
                   {
                     array_plate[counter]=i+1;
                     counter++;
                   }
               }
             counter=0;

             //Label wells in plate.
             for(i=0;i<plate_number;i++)
               {
                 for(j=0;j<plate_size;j++)
                   {
                     array_wells[counter]=j+1;
                     counter++;
                   }
               }
             counter=0;
    
             //Zero pick array.
             for(i=0;i<(plate_number*plate_size);i++)
               {
                 array_picks[i]=0;
               }

             //Label selected groups in plate based on user picks.
             if(pick_groups_array_length1>0)
               {
                 for(i=0;i<plate_number;i++)
                   {
                     k=0;
                     for(j=0;j<plate_size;j++)
                       {
                         if(k<pick_groups_array_length1 && j==g_array_index(pick_groups_array1, gint, k))
                           {
                             array_picks[counter]=value1;
                             k++;
                             counter++;
                           }
                         else
                           {
                             counter++;
                           }
                       }
                   }
                 counter=0; 
               }
               
             if(pick_groups_array_length2>0)
               {
                 for(i=0;i<plate_number;i++)
                   {
                     k=0;
                     for(j=0;j<plate_size;j++)
                       {
                         if(k<pick_groups_array_length2 && j==g_array_index(pick_groups_array2, gint, k))
                           {
                             array_picks[counter]=value2;
                             k++;
                             counter++;
                           }
                         else
                           {
                             //array_picks[counter]=0;
                             counter++;
                           }
                       }
                    }
                 counter=0;
                }

              if(pick_groups_array_length3>0)
                {
                 for(i=0;i<plate_number;i++)
                   {
                     k=0;
                     for(j=0;j<plate_size;j++)
                       {
                         if(k<pick_groups_array_length3 && j==g_array_index(pick_groups_array3, gint, k))
                           {
                             array_picks[counter]=value3;
                             k++;
                             counter++;
                           }
                         else
                           {
                             //array_picks[counter]=0;
                             counter++;
                           }
                       }
                    }
                  counter=0;
                 }

             if(pick_groups_array_length4>0)
               {
                 for(i=0;i<plate_number;i++)
                   {
                     k=0;
                     for(j=0;j<plate_size;j++)
                        {
                          if(k<pick_groups_array_length4 && j==g_array_index(pick_groups_array4, gint, k))
                            {
                              array_picks[counter]=value4;
                              k++;
                              counter++;
                            }
                          else
                            {
                              //array_picks[counter]=0;
                              counter++;
                            }
                        }
                   }
                 counter=0;
                }

             //Label Groups like Picks
             for(i=0;i<plate_number;i++)
               {
                 for(j=0;j<plate_size/group_size;j++)  
                   {
                     for(k=0;k<group_size;k++)
                       {
                         array_groups[counter]=label;
                         counter++;
                       }
                     label++;
                   }
                 label=1; 
               }

             counter=0;


           //Take care of loading the database.
           sqlite3 *handle;
           //int retval;
           char delete_table[]="DELETE FROM Aux";
           char create_table[] = "CREATE TABLE IF NOT EXISTS Aux (KeyID int PRIMARY KEY, Plate int NOT NULL, Wells int NOT NULL, Picks int NOT NULL, Groups int NOT NULL)";

           sqlite3_open("VelociRaptorData.db",&handle);

           if(!gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button1)))
             {
               sqlite3_exec(handle, delete_table,0,0,0);
               sqlite3_exec(handle,create_table,0,0,0);
             }

           if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(check_button1)))
             {
               sqlite3_stmt *stmt1;
               sqlite3_prepare(handle, "UPDATE Aux SET Picks=? WHERE KeyID =?;", -1, &stmt1, 0);
               sqlite3_exec(handle, "BEGIN TRANSACTION", NULL, NULL, NULL);
               for(i=0;i<array_length;i++)
                 {
                   if(array_picks[i]>0)
                     {
                       g_print("UPDATE Aux SET Picks=%i WHERE KeyID = %i;\n", array_picks[i], i+1);
                       sqlite3_bind_int(stmt1, 1, array_picks[i]);
                       sqlite3_bind_int(stmt1, 2, i+1); 
                       sqlite3_step(stmt1);
                       sqlite3_reset(stmt1); 
                     }
                  }
                 sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
                 sqlite3_finalize(stmt1);
                 sqlite3_close(handle);
                 g_print("Auxiliary Table Updated\n");
               }
            else
               {
                 sqlite3_stmt *stmt2;
                 sqlite3_prepare(handle, "INSERT INTO Aux VALUES (?,?,?,?,?);", -1, &stmt2, 0);
                 sqlite3_exec(handle, "BEGIN TRANSACTION", NULL, NULL, NULL);
                 for(i=0;i<array_length;i++)
                    {
                      g_print("INSERT INTO Aux VALUES(%i,%i,%i,%i,%i)\n", i+1, array_plate[i], array_wells[i], array_picks[i], array_groups[i]);
                      sqlite3_bind_int(stmt2, 1, i+1);
                      sqlite3_bind_int(stmt2, 2, array_plate[i]); 
                      sqlite3_bind_int(stmt2, 3, array_wells[i]); 
                      sqlite3_bind_int(stmt2, 4, array_picks[i]); 
                      sqlite3_bind_int(stmt2, 5, array_groups[i]); 
                      sqlite3_step(stmt2);
                      sqlite3_reset(stmt2); 
                    }
                 sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
                 sqlite3_finalize(stmt2);
                 sqlite3_close(handle);
                 g_print("Auxiliary Table Built\n");
              }
          g_string_free(buffer, TRUE);
          g_array_free(pick_groups_array1, TRUE);
          g_array_free(pick_groups_array2, TRUE);
          g_array_free(pick_groups_array3, TRUE);
          g_array_free(pick_groups_array4, TRUE);
        }
     
     gtk_widget_destroy(dialog);
  }
static void build_combo_table_dialog(GtkWidget *menu, GtkWidget *window)
  {
     //Build a combinations SQL script, save it to a file and send it off to the database.
     GtkWidget *dialog, *grid, *label1, *label2, *label3, *value_entry1, *value_entry2, *value_entry3, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Build Combinations Table", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("SQL Table Name");
    label2=gtk_label_new("Combination Set Size");
    label3=gtk_label_new("Combinations Sub Set");
    
    value_entry1=gtk_entry_new();
    value_entry2=gtk_entry_new();
    value_entry3=gtk_entry_new();
     
    gtk_widget_set_halign(value_entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry2, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry3, GTK_ALIGN_CENTER);

    gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 12);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 2);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry3), 2);

    gtk_entry_set_text(GTK_ENTRY(value_entry1), "combinations");
    gtk_entry_set_text(GTK_ENTRY(value_entry2), "7");
    gtk_entry_set_text(GTK_ENTRY(value_entry3), "5");
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry3, 1, 2, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Build Combinations Table\n");
        gint combo_set=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
        gint combo_subset=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry3)));
        const gchar *table_name=gtk_entry_get_text(GTK_ENTRY(value_entry1));

        //cap at 15
        if(combo_set>=2&&combo_set<=15)
          {
            if(combo_subset>=2&&combo_subset<=combo_set)
              {
                build_combo_table_sql(combo_set, combo_subset, table_name);
              }
            else
              {
                g_print("The combination subset range is 2<=x<=ComboSet\n");
                simple_message_dialog("The combination subset range is 2<=x<=ComboSet");
              }
          }
        else
          {
            g_print("The combination set range is 2<=x<=15\n");
            simple_message_dialog("The combination set range is 2<=x<=15");
          }       
     }
    g_print("Combinations Finished\n");
    gtk_widget_destroy(dialog);
  }
static void build_permutation_table_dialog(GtkWidget *menu, GtkWidget *window)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *value_entry1, *value_entry2, *content_area, *action_area;
    gint result;

    dialog=gtk_dialog_new_with_buttons("Build Permutations Table", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("SQL Table Name");
    label2=gtk_label_new("Permutation Set Size");
    
    value_entry1=gtk_entry_new();
    value_entry2=gtk_entry_new();
    gtk_widget_set_halign(value_entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 12);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 2);
    gtk_entry_set_text(GTK_ENTRY(value_entry1), "permutations");
    gtk_entry_set_text(GTK_ENTRY(value_entry2), "5");
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry2, 1, 1, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        g_print("Build Permutations Table\n");
        gint combo_set=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
        const gchar *table_name=gtk_entry_get_text(GTK_ENTRY(value_entry1));
        //Cap at 9
        if(combo_set>=2&&combo_set<=9)
          {
            build_permutation_table_sql(combo_set, table_name);
          }
         else
          {
            g_print("The permutation set range of is 2<=x<=9.\n");
            simple_message_dialog("The permutation set range is 2<=x<=9.\n");
          }         
       }
     g_print("Permutations Finished\n");
     gtk_widget_destroy(dialog);
  }
static void format_text_dialog(GtkButton *button, gpointer data)
  {
    GtkWidget *dialog, *grid, *entry1, *entry2, *entry3, *entry4, *label1, *label2, *label3, *label4, *content_area, *action_area;
    gint result;
    
    g_print("Format Text\n");

    dialog=gtk_dialog_new_with_buttons("Format Platemap", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("Format Blue   >");
    label2=gtk_label_new("Format Red   <");
    label3=gtk_label_new("Format Green   >");
    label4=gtk_label_new("Format Green   <");

    entry1=gtk_entry_new();
    entry2=gtk_entry_new();
    entry3=gtk_entry_new();
    entry4=gtk_entry_new();

    gtk_widget_set_halign(entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(entry2, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(entry3, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(entry4, GTK_ALIGN_CENTER);

    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 8);
    gtk_entry_set_width_chars(GTK_ENTRY(entry4), 8);

    gtk_entry_set_text(GTK_ENTRY(entry1), "800.0");
    gtk_entry_set_text(GTK_ENTRY(entry2), "200.0");
    gtk_entry_set_text(GTK_ENTRY(entry3), "400.0");
    gtk_entry_set_text(GTK_ENTRY(entry4), "600.0");

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 1, 3, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        gdouble d_entry1=atof(gtk_entry_get_text(GTK_ENTRY(entry1)));
        gdouble d_entry2=atof(gtk_entry_get_text(GTK_ENTRY(entry2)));
        gdouble d_entry3=atof(gtk_entry_get_text(GTK_ENTRY(entry3)));
        gdouble d_entry4=atof(gtk_entry_get_text(GTK_ENTRY(entry4)));
        format_text_platemap(d_entry1, d_entry2, d_entry3, d_entry4, GTK_TEXT_VIEW(data));         
      }
    gtk_widget_destroy(dialog);      
  }
gboolean entry_field_changed(GtkWidget *entry, GdkEvent *event, gpointer data)
  {
    entry_field_validation(pPlateSizeText, pPlateStatsText, entry);
    return FALSE; 
  }
gboolean control_changed(GtkWidget *entry, gpointer data)
  {
    control_changed_validation(pPlateSizeText, entry);
    return FALSE;
  }

static void pop_up_button_press_event(GtkWidget *treeview, GdkEventButton *event, GtkWidget *menu)
  {
    g_print("Pop-up Menu Connected\n");
    if((event->button==3)&&(event->type==GDK_BUTTON_RELEASE))
      {
        gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
      } 
  }
/*
   Not working correctly. Comment out.
static void copy_selected_to_clipboard(GtkWidget *copy, GtkWidget *treeview)
  {
    //Copy the selected TreeView data to the clipboard.
    GtkClipboard *clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
    GtkTreeModel *model2=gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
    GtkTreePath *path;
    GtkTreeIter iter;
    GList *list;
    GList *row;
    GString *buffer=g_string_new(NULL);
    gchar *string=NULL;
    const gchar *title;
    double dnumber=0;
    guint32 count;

    count=gtk_tree_selection_count_selected_rows(GTK_TREE_SELECTION(selection));
    g_print("%i Rows Selected\n", count);
    
    list=gtk_tree_selection_get_selected_rows(GTK_TREE_SELECTION(selection), &model2);
    //listcount=g_list_length(list);
    row=list;
    title=(gtk_menu_item_get_label(GTK_MENU_ITEM(copy)));
    g_print("Menu is %s\n", title);
       
          while (row)
              {
                path = (GtkTreePath*)(row->data);
                string=gtk_tree_path_to_string(path);
                gtk_tree_model_get_iter(model2, &iter, path);
                gtk_tree_model_get(model2, &iter, 1, &dnumber, -1);
                g_string_append_printf(buffer, "%f \n", dnumber);
                row = row->next;
              }
           
    gtk_clipboard_set_text(clipboard, buffer->str, strlen(buffer->str));
    //g_list_foreach (list, (GFunc) gtk_tree_path_free, NULL);
    g_list_free (list);
    if(string!=NULL)
       {
        g_free(string);
       }
    g_string_free(buffer,TRUE);
  }
*/
static void copy_plates_to_clipboard_dialog(GtkWidget *copy, GtkWidget *treeview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *value_entry1, *value_entry2, *content_area, *action_area;
    int result;
    int rows=0;
    int columns=0;

    dialog=gtk_dialog_new_with_buttons("Plate Map", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("         Rows");
    label2=gtk_label_new("         Columns");
    
    value_entry1=gtk_entry_new();
    value_entry2=gtk_entry_new();
    gtk_widget_set_halign(value_entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry2, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 4);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 4);
    gtk_entry_set_text(GTK_ENTRY(value_entry1), "8");
    gtk_entry_set_text(GTK_ENTRY(value_entry2), "12");
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry2, 1, 1, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 40);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        rows=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry1)));
        columns=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
        if(rows<1||columns<1)
          {
            simple_message_dialog("Rows and columns need to be greater than zero.");
          }
        else
          {
            copy_plates_to_clipboard(copy,treeview,rows,columns,0);
          }
      }
    gtk_widget_destroy(dialog);
  }
static void copy_plates_to_clipboard_withtruncate_dialog(GtkWidget *copy, GtkWidget *treeview)
  {
    GtkWidget *dialog, *grid, *label1, *label2, *label3, *value_entry1, *value_entry2, *value_entry3, *content_area, *action_area;
    int result;
    int rows=0;
    int columns=0;
    int digits=0;

    dialog=gtk_dialog_new_with_buttons("Plate Map", pMainWindow, GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, "Cancel", GTK_RESPONSE_CANCEL, NULL);
    gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
    gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

    label1=gtk_label_new("      Rows");
    label2=gtk_label_new("      Columns");
    label3=gtk_label_new("      Truncate Digits");
    
    value_entry1=gtk_entry_new();
    value_entry2=gtk_entry_new();
    value_entry3=gtk_entry_new();
    gtk_widget_set_halign(value_entry1, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry2, GTK_ALIGN_CENTER);
    gtk_widget_set_halign(value_entry3, GTK_ALIGN_CENTER);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 3);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 3);
    gtk_entry_set_width_chars(GTK_ENTRY(value_entry3), 3);
    gtk_entry_set_text(GTK_ENTRY(value_entry1), "8");
    gtk_entry_set_text(GTK_ENTRY(value_entry2), "12");
    gtk_entry_set_text(GTK_ENTRY(value_entry3), "7");
 
    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), value_entry3, 1, 2, 1, 1);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 30);

    content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS
    action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
    G_GNUC_END_IGNORE_DEPRECATIONS
    gtk_container_add(GTK_CONTAINER(content_area), grid); 
    gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

    gtk_widget_show_all(dialog);
    result=gtk_dialog_run(GTK_DIALOG(dialog));

    if(result==GTK_RESPONSE_OK)
      {
        rows=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry1)));
        columns=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
        digits=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry3)));         
        if(digits<1||digits>10)
          {
            simple_message_dialog("Truncate digits 0<x<=10.");
          }
        else if(rows<1||columns<1)
          {
            simple_message_dialog("Rows and columns need to be greater than zero.");
          }
        else
          {
            copy_plates_to_clipboard(copy,treeview,rows,columns,digits);
          }
      }
    gtk_widget_destroy(dialog);
  }
static void copy_plates_to_clipboard(GtkWidget *copy, GtkWidget *treeview, gint rows, gint columns, gint digits)
  {
    //Copy one dimensional data to two dimensions or plate well format.
    gint plate_size=atoi(pPlateSizeText);
    gint number_of_plates=atoi(pPlateNumberText);
    const gchar *title;
    GtkTreeIter iter;
    GtkTreeModel *model;
    guint32 model_rows=0;
    gdouble temp=0;

    g_print("Rows %i Columns %i PlateSize %i\n", rows, columns, plate_size);
    if(rows*columns!=plate_size)
      {
        simple_message_dialog("The Rows in Plate times the Columns in Plate need\n to equal the Size of Plate!");
      }
    else
      {
        model=gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");
        do
          {
            model_rows++;
          }
        while(gtk_tree_model_iter_next(model,&iter));
        gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");
        double data_array[model_rows];
        model_rows=0;
        do
          {
            gtk_tree_model_get(model, &iter, 1, &temp, -1);
            data_array[model_rows]=temp;
            model_rows++;
          }
        while(gtk_tree_model_iter_next(model,&iter));

        title=(gtk_menu_item_get_label(GTK_MENU_ITEM(copy)));
        g_print("Menu is %s\n", title);

        if(model_rows==(number_of_plates*rows*columns))
          {
            if(g_strcmp0(title, "Copy Plates to Clipboard(int)")==0)
              {
                PlateMapInt(data_array, number_of_plates, plate_size, rows, columns);  
              }
            else if(g_strcmp0(title, "Copy Plates to Clipboard(float)")==0)
              {
                PlateMapDouble(data_array, number_of_plates, plate_size, rows, columns);
              }
            else
              {
                PlateMapDoubleTruncate(data_array, number_of_plates, plate_size, rows, columns, digits);  
              }
            }
         else
            {
              simple_message_dialog("The record count should equal the Number of Plates\n times Rows times Columns!");
            }
         g_print("Plate Map is on Clipboard\n");
     }
  }
static void select_data(GtkWidget *selection, GtkTreeView *treeview)
  {
    g_print("Data Selected\n");
  }
    
static void cursor_changed(GtkTreeView *treeview, gpointer data)
  {
    //g_print("Treeview Cursor Changed\n");
  }
static void select_all(GtkTreeView *treeview, gpointer data)
  {
    g_print("Select All\n"); 
  }
static void copy_treeview_to_database(GtkWidget *copy, GtkWidget *treeview)
  {
    copy_treeview_to_database_sql(copy,treeview,pWindowTitle);
  }
static void cell_edited(GtkCellRendererText *renderer, gchar *path, gchar *new_text, GtkTreeView *treeview)
  {
    //Edit the text in a TreeView. Problem with formatting and editing of significant digits and 
    //double data in the model.
    GtkTreeIter iter;
    GtkTreeModel *model;
    int test;

    g_print("Row Number %s\n", path);
    g_print("New Number %s\n", new_text);
    if(g_ascii_strcasecmp(new_text, "")!=0)
      {
        model=gtk_tree_view_get_model(treeview);
        test=gtk_tree_model_get_n_columns(model);
        g_print("Columns %i\n", test);
        if(gtk_tree_model_get_iter_from_string(model, &iter, path))
          {
           if(gtk_list_store_iter_is_valid(GTK_LIST_STORE(model), &iter))
             {
              g_print("Iterator is Valid\n");
              gtk_list_store_set(GTK_LIST_STORE(model), &iter, 1, g_ascii_strtod(new_text, NULL), -1);
             }
          }
      }
   }
static void test_data_button_clicked (GtkButton *button, gpointer data, int seed_value, double param, int radio_button)
{
   GtkWidget *dialog, *content_area, *treeview, *action_area, *label1, *next_button, *scrolled_win, *menu, *copyplates1, *copyplates2, *copyplates3, *copyappend;
   GtkTreeSelection *selection;
   GtkAccelGroup *group = NULL;
   gint array_count;
   gint array_not_void=0;
   gdouble temp;
   GArray *data_array;
   gint check=0;

    //Simple validation checks.
    if(data==NULL)
      {
        gint plate_size=atoi(pPlateSizeText);
        gint number_of_plates=atoi(pPlateNumberText);
        gint set_size_for_statistics=atoi(pPlateStatsText);
        printf("Plates %i, PlateSize %i, Stats %i\n",number_of_plates,plate_size,set_size_for_statistics);
        if(plate_size<1||number_of_plates<1||set_size_for_statistics<1)
          {
            check=1;
          }
        if(set_size_for_statistics>0)
          {
            if(plate_size%set_size_for_statistics!=0)
              {
                check=1;
              }
          }
        if(plate_size<2*set_size_for_statistics)
          {
            check=1;
          }
      }

    //Only one initial data set opened at a time. Trouble with which data set referenced for
    //calculations.
    if(iReferenceCountDialogWindow==1)
     {
       simple_message_dialog("Only one initial dataset can be opened at a time!");
     }
    else if(check==1)
     {
       simple_message_dialog("The number of plates, plate size or stats field value is invalid.");
     }
    else
     {
        if(data!=NULL)
          {
           data_array=(GArray*)data;
           array_count=data_array[0].len;
           temp=g_array_index(data_array, double, 0);
           g_print("Check Array Length %i First Value %f\n", array_count, temp);
           array_not_void=1;
          }

       dialog=gtk_dialog_new();
       gtk_window_set_transient_for(GTK_WINDOW(dialog), pMainWindow);
       gtk_window_set_title(GTK_WINDOW(dialog), "Plate Data");

       content_area=gtk_dialog_get_content_area (GTK_DIALOG(dialog));
       G_GNUC_BEGIN_IGNORE_DEPRECATIONS
       action_area=gtk_dialog_get_action_area (GTK_DIALOG(dialog));
       G_GNUC_END_IGNORE_DEPRECATIONS 

       label1=gtk_label_new("");
       gtk_container_add (GTK_CONTAINER (action_area), label1);
       next_button=gtk_button_new_with_label("  Calculate Percent>  ");
       gtk_container_add (GTK_CONTAINER (action_area), next_button);
   
       treeview=gtk_tree_view_new();
       g_signal_connect(G_OBJECT(next_button), "clicked", G_CALLBACK(next_button_clicked), (gpointer) treeview);

       if(array_not_void==0)
         {
           setup_tree_view_data(GTK_TREE_VIEW(treeview), seed_value, param, radio_button);
         }
       else
         {
           setup_tree_view_text(GTK_TREE_VIEW(treeview), data_array);
         }
  
        scrolled_win=gtk_scrolled_window_new(NULL, NULL);
        gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win), GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
  
        selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
        gtk_tree_selection_set_mode(selection, GTK_SELECTION_MULTIPLE);
        g_signal_connect(G_OBJECT(selection), "changed", G_CALLBACK(select_data), (gpointer) treeview);
        g_signal_connect(G_OBJECT(treeview), "cursor_changed", G_CALLBACK(cursor_changed), (gpointer) treeview);
        g_signal_connect(G_OBJECT(treeview), "select_all", G_CALLBACK(select_all), (gpointer) treeview);

        group = gtk_accel_group_new();
        gtk_window_add_accel_group(GTK_WINDOW(dialog), group);
    
        menu=gtk_menu_new();
        //copy=gtk_menu_item_new_with_label("Copy Selected to Clipboard");
        copyplates1=gtk_menu_item_new_with_label("Copy Plates to Clipboard(int)");
        copyplates2=gtk_menu_item_new_with_label("Copy Plates to Clipboard(float)");
        copyplates3=gtk_menu_item_new_with_label("Copy Plates to Clipboard(truncate)");
        copyappend=gtk_menu_item_new_with_label("Copy Table to SQLite database");
        //g_signal_connect(G_OBJECT(copy), "activate", G_CALLBACK(copy_selected_to_clipboard), treeview);
        g_signal_connect(G_OBJECT(copyappend), "activate", G_CALLBACK(copy_treeview_to_database), treeview);
        //gtk_widget_add_accelerator(copy, "activate", group, GDK_KEY_C, GDK_CONTROL_MASK, GTK_ACCEL_MASK);
        g_signal_connect(G_OBJECT(copyplates1), "activate", G_CALLBACK(copy_plates_to_clipboard_dialog), treeview);
        g_signal_connect(G_OBJECT(copyplates2), "activate", G_CALLBACK(copy_plates_to_clipboard_dialog), treeview);
        g_signal_connect(G_OBJECT(copyplates3), "activate", G_CALLBACK(copy_plates_to_clipboard_withtruncate_dialog), treeview);
        gtk_menu_attach_to_widget(GTK_MENU(menu), treeview, NULL);
        //gtk_menu_shell_append(GTK_MENU_SHELL(menu), copy);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), copyplates1);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), copyplates2);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), copyplates3);
        gtk_menu_shell_append(GTK_MENU_SHELL(menu), copyappend);
        gtk_widget_show_all(menu);
        g_signal_connect(G_OBJECT(treeview), "button_release_event", G_CALLBACK(pop_up_button_press_event),menu);

        gtk_widget_set_size_request(scrolled_win, 100, 400);
        gtk_container_add(GTK_CONTAINER(scrolled_win), treeview);
        gtk_container_add(GTK_CONTAINER(content_area), scrolled_win);

        //reference count the dialog and then destroy.
        g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(dialog_reference_destroy), NULL);
        g_signal_connect(GTK_WINDOW(dialog), "window_state_event", G_CALLBACK(dialog_state_change),NULL);

        gtk_widget_set_events(treeview, GDK_BUTTON_PRESS_MASK);
        gtk_widget_realize(treeview);
        gtk_widget_show_all (dialog);
        pWindowTitle=gtk_window_get_title(GTK_WINDOW(dialog));
        iReferenceCountDialogWindow=1;
     }
}

static void next_button_clicked(GtkButton *NextButton, GtkTreeView *treeview)
{
   //Create the TreeView for the data produced by the functions listed in the combobox.
   GtkWidget *dialog2, *content_area2, *treeview2, *action_area2, *label1, *next_button2, *scrolled_win2, *copyplates2, *copyplatesd2, *copyplatest2, *copyappend2, *menu2;
   GtkTreeSelection *selection2;
   GtkAccelGroup *group2 = NULL;

   g_print("Next Button Clicked\n");
   
   //Look for some basic errors. Need to check for random number array count also.
   if(iRandomDataArrayCount!=0 && iRandomDataArrayCount!=atoi(pPlateNumberText)*atoi(pPlateSizeText))
     {
       g_print("Records %i NumberOfPlates %i  PlateSize %i\n", iRandomDataArrayCount,atoi(pPlateNumberText),atoi(pPlateSizeText));
       simple_message_dialog("The record count should equal the\n Plate Size times Size of Plate!");
     }
   else if(iTextArrayCount!=0 && iTextArrayCount!=atoi(pPlateNumberText)*atoi(pPlateSizeText))
     {
       g_print("Records %i NumberOfPlates %i  PlateSize %i\n", iTextArrayCount,atoi(pPlateNumberText),atoi(pPlateSizeText));
       simple_message_dialog("The record count should equal the\n Plate Size times Size of Plate!");
     }
   else
     {
       dialog2=gtk_dialog_new();
       gtk_window_set_transient_for(GTK_WINDOW(dialog2), pMainWindow);
       gtk_window_set_title(GTK_WINDOW(dialog2), "Percent");

       content_area2=gtk_dialog_get_content_area(GTK_DIALOG(dialog2));
       G_GNUC_BEGIN_IGNORE_DEPRECATIONS
       action_area2=gtk_dialog_get_action_area(GTK_DIALOG(dialog2));
       G_GNUC_END_IGNORE_DEPRECATIONS
      
       label1=gtk_label_new("");
       gtk_container_add (GTK_CONTAINER (action_area2), label1);
       next_button2=gtk_button_new_with_label("Build Auxiliary Table>");
       gtk_container_add (GTK_CONTAINER (action_area2), next_button2);
       g_signal_connect(G_OBJECT(next_button2), "clicked", G_CALLBACK(build_aux_table_dialog), NULL);
            
       treeview2=gtk_tree_view_new();

        //setup tree view percent
       setup_tree_view_percent(GTK_TREE_VIEW(treeview2),GTK_TREE_VIEW(treeview));

       scrolled_win2=gtk_scrolled_window_new(NULL, NULL);
       gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win2), GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
  
       selection2=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview2));
       gtk_tree_selection_set_mode(selection2, GTK_SELECTION_MULTIPLE);

       group2=gtk_accel_group_new();
       gtk_window_add_accel_group(GTK_WINDOW(dialog2), group2);
    
       menu2=gtk_menu_new();
       //copy2=gtk_menu_item_new_with_label("Copy Selected to Clipboard");
       copyplates2=gtk_menu_item_new_with_label("Copy Plates to Clipboard(int)");
       copyplatesd2=gtk_menu_item_new_with_label("Copy Plates to Clipboard(float)");
       copyplatest2=gtk_menu_item_new_with_label("Copy Plates to Clipboard(truncate)");
       copyappend2=gtk_menu_item_new_with_label("Copy Table to SQLite database");
       //g_signal_connect(G_OBJECT(copy2), "activate", G_CALLBACK(copy_selected_to_clipboard), treeview2);
       g_signal_connect(G_OBJECT(copyappend2), "activate", G_CALLBACK(copy_treeview_to_database), treeview2);
       //gtk_widget_add_accelerator(copy2, "activate", group2, GDK_KEY_C, GDK_CONTROL_MASK, GTK_ACCEL_MASK);
       g_signal_connect(G_OBJECT(copyplates2), "activate", G_CALLBACK(copy_plates_to_clipboard_dialog), treeview2);
       g_signal_connect(G_OBJECT(copyplatesd2), "activate", G_CALLBACK(copy_plates_to_clipboard_dialog), treeview2);
       g_signal_connect(G_OBJECT(copyplatest2), "activate", G_CALLBACK(copy_plates_to_clipboard_withtruncate_dialog), treeview2);
       gtk_menu_attach_to_widget(GTK_MENU(menu2), treeview2, NULL);
       //gtk_menu_shell_append(GTK_MENU_SHELL(menu2), copy2);
       gtk_menu_shell_append(GTK_MENU_SHELL(menu2), copyplates2);
       gtk_menu_shell_append(GTK_MENU_SHELL(menu2), copyplatesd2);
       gtk_menu_shell_append(GTK_MENU_SHELL(menu2), copyplatest2);
       gtk_menu_shell_append(GTK_MENU_SHELL(menu2), copyappend2);
       gtk_widget_show_all(menu2);
       g_signal_connect(G_OBJECT(treeview2), "button_release_event", G_CALLBACK(pop_up_button_press_event),menu2);

       gtk_widget_set_size_request(scrolled_win2, 100, 400);
       gtk_container_add(GTK_CONTAINER(scrolled_win2), treeview2);
       gtk_container_add(GTK_CONTAINER(content_area2), scrolled_win2);

       g_signal_connect(G_OBJECT(dialog2), "response", G_CALLBACK(dialog_reference_destroy), NULL);
       g_signal_connect(GTK_WINDOW(dialog2), "window_state_event", G_CALLBACK(dialog_state_change),NULL);
       
       gtk_widget_show_all(dialog2);

       gtk_window_move(GTK_WINDOW(dialog2), gdk_screen_width()/2 +15, gdk_screen_height()/2-15);
       pWindowTitle=gtk_window_get_title(GTK_WINDOW(dialog2));
     }
  }
void setup_tree_view_text(GtkTreeView *treeview2, GArray *data_array)
  {
    //Create a TreeView for the imported text file.
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;
    //GtkTreeModel *model;
    gint array_count;
    guint32 i=0;
    gchar *records=NULL;

    g_print("Set Up Treeview from Text File or SQLite...\n");

    store=gtk_list_store_new(2, G_TYPE_INT, G_TYPE_DOUBLE);
    array_count=data_array[0].len;
    //Save count array count to global variable.
    iTextArrayCount=data_array[0].len;
    records=g_strdup_printf("Data (%i records)", array_count); 

    for(i=0; i<array_count; i++)
      {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0 , i+1, 1, g_array_index(data_array, gdouble, i), -1);
      }

    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview2), GTK_TREE_MODEL(store));
    g_object_unref(store);

    renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes("Counter", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);

    //Maybe render text with gtk_tree_view_column_set_cell_data_func for correct number of 
    //digits. A problem rendering double and editing double in text.
    renderer=gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, "editable-set", TRUE, NULL);
    column=gtk_tree_view_column_new_with_attributes(records, renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);
    g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_edited), (gpointer)treeview2);
       
    g_free(records);
    g_print("TreeView Built\n");
    /*
      After text treeview built check the arguments to make sure they apply. Check in next button.
    */
  }
void setup_tree_view_data(GtkTreeView *treeview, int seed_value, double param, int radio_button)
  {
    //Set up the tree model and renderers for the TreeView and call data functions.
    guint32 i=0;
    GtkListStore *store;
    GtkTreeIter iter;
    GtkCellRenderer *renderer;
    GtkTreeViewColumn *column;  
    guint32 model_rows=0;
    gchar *records=NULL;
    gdouble *data_array=NULL;
      
    //Try to handle input errors before function call and atoi.
    gint plate_size=atoi(pPlateSizeText);
    gint number_of_plates=atoi(pPlateNumberText);
    gint set_size_for_statistics=atoi(pPlateStatsText);

    g_print("Allocate Arrays on Free Store\n");
    data_array=(double*)malloc((plate_size*number_of_plates) * sizeof(gdouble));
    if(data_array==NULL)
      {
        g_print("Array Allocation Failed!\n");
        simple_message_dialog("Couldn't allocate memory for the treeview! Reduce\n the number of records in the dataset.");
      }
    iRandomDataArrayCount=plate_size*number_of_plates;
           
    g_print("Arrays Allocated\n");
    g_print("Setting Up Treeview...\n");      
        
    if(radio_button==1)
      {
        gdouble gsl_ran_gaussian();
        GenerateRandomValues(data_array, number_of_plates, plate_size, set_size_for_statistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, gsl_ran_gaussian);
      }
    if(radio_button==2)
      {
        gdouble gsl_ran_chisq();
        GenerateRandomValues(data_array, number_of_plates, plate_size, set_size_for_statistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, gsl_ran_chisq);
      }
    if(radio_button==3)
      {
        gdouble gsl_ran_rayleigh();
        GenerateRandomValues(data_array, number_of_plates, plate_size, set_size_for_statistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, gsl_ran_rayleigh);
      }
    if(radio_button==4)
      {
        gdouble wrap_gsl_rng_uniform();
        GenerateRandomValues(data_array, number_of_plates, plate_size, set_size_for_statistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, wrap_gsl_rng_uniform);
      }

    model_rows=plate_size*number_of_plates;
 
    g_print("Model Rows Count %i\n", model_rows);
    records=g_strdup_printf("Data (%i records)", model_rows); 

    store=gtk_list_store_new(2, G_TYPE_INT, G_TYPE_DOUBLE);

    for(i=0; i<(plate_size*number_of_plates); i++)
      {
        gtk_list_store_append(store, &iter);
        gtk_list_store_set(store, &iter, 0 , i+1, 1, data_array[i], -1);
      }
              
    gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
    g_object_unref(store);

    renderer=gtk_cell_renderer_text_new();
    column=gtk_tree_view_column_new_with_attributes("Counter", renderer, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

    renderer=gtk_cell_renderer_text_new();
    g_object_set(renderer, "editable", TRUE, "editable-set", TRUE, NULL);
    column=gtk_tree_view_column_new_with_attributes(records, renderer, "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
    g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_edited), (gpointer) treeview);
          
    g_free(records);
    if(data_array!=NULL)
      {
        free(data_array);
      }
        
    g_print("Free Arrays\n");
    g_print("Treeview Built\n");
        
  }
void setup_tree_view_percent(GtkTreeView *treeview2, GtkTreeView *treeview)
  {
    //Set up the tree model and renderers for the TreeView and call data functions.
     guint32 i=0;
     GtkListStore *store;
     GtkTreeIter iter;
     GtkCellRenderer *renderer;
     GtkTreeViewColumn *column;
     GtkTreeModel *model;
     gdouble temp=0;   
     guint32 model_rows=0;
     gchar *records=NULL;
     gdouble *data_array=NULL;
     gdouble *percent_array=NULL;
      
     //Try to handle input errors before function call and atoi.
     gint plate_size=atoi(pPlateSizeText);
     gint number_of_plates=atoi(pPlateNumberText);
     int set_size_for_statistics=atoi(pPlateStatsText);
        
     g_print("Allocate Arrays\n");
     data_array = (gdouble*)malloc((plate_size*number_of_plates) * sizeof(gdouble));
     percent_array = (gdouble*)malloc((plate_size*number_of_plates) * sizeof(gdouble));

     if(data_array==NULL||percent_array==NULL)
       {
         g_print("Array Allocation Failed!\n");
         simple_message_dialog("Couldn't allocate memory for the treeview! Reduce\n the number of records in the dataset.");
       }
     g_print("Arrays Allocated\n");

     g_print("Setting Up Treeview...\n");      
     model=gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
     gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");
  
     do
       {
         gtk_tree_model_get(model, &iter, 1, &temp, -1);
         data_array[model_rows]=temp;
         model_rows++;
       }
     while(gtk_tree_model_iter_next(model,&iter));

     CalculatePercentControl(data_array, percent_array, plate_size, number_of_plates, set_size_for_statistics, pPlatePosControlText, pPlateNegControlText);
           
     g_print("Model Rows Count %i\n", model_rows);
     records=g_strdup_printf("Data (%i records)", model_rows); 

     store=gtk_list_store_new(2, G_TYPE_INT, G_TYPE_DOUBLE);

     for(i=0; i<(plate_size*number_of_plates); i++)
       {
         gtk_list_store_append(store, &iter);
         gtk_list_store_set(store, &iter, 0 , i+1, 1, percent_array[i], -1);
       }
              
     gtk_tree_view_set_model(GTK_TREE_VIEW(treeview2), GTK_TREE_MODEL(store));
     g_object_unref(store);

     renderer=gtk_cell_renderer_text_new();
     column=gtk_tree_view_column_new_with_attributes("Counter", renderer, "text", 0, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);

     renderer=gtk_cell_renderer_text_new();
     g_object_set(renderer, "editable", TRUE, "editable-set", TRUE, NULL);
     column=gtk_tree_view_column_new_with_attributes(records, renderer, "text", 1, NULL);
     gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);
     g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_edited), (gpointer) treeview2);
          
     g_free(records);
     if(data_array!=NULL) free(data_array);
     if(percent_array!=NULL) free(percent_array);
        
     g_print("Free Arrays\n");
     g_print("Treeview Built\n");
        
  }
static void connect_sqlite_db(GtkWidget *button1, GArray *widgets)
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
       
        //Attach the store data to the treeview.
        gtk_tree_view_set_model(GTK_TREE_VIEW(g_array_index(widgets, GtkWidget*, 1)), GTK_TREE_MODEL(store));  
      }
     
    g_object_unref(G_OBJECT(store));
    if(database!=NULL) g_free(database);
  }
static void get_treeview_selected(GtkWidget *button1, GArray *widgets)
  {
    g_print("Get Treeview Tables and Fields\n");
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
        g_print("SELECT %s FROM %s;\n", field, table);
        get_single_field_values(table, field, widgets);
      }
    else
      {
        g_print("Connect and then select a table and a field.\n");
      }
    if(table!=NULL) g_free(table);
    if(field!=NULL) g_free(field);
  }
static void get_single_field_values(gchar *table, gchar *field, GArray *widgets)
  {
    g_print("Build Data Array\n");
    gint i=0;
    gint zero_counter=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    gint sql_return=0;
    gdouble temp=0;
    GArray *data_array=g_array_new(FALSE, FALSE, sizeof(double));
    gchar *database=g_strdup_printf("%s", gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 0))));
    char *sql1=sqlite3_mprintf("SELECT %q FROM %q;", field, table);
    //g_print("%s\n", sql1);

    sqlite3_open(database,&cnn);   
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    
    //Watch for zero's in implicit conversion.
    while(sql_return==SQLITE_ROW)
      {
        temp=sqlite3_column_double(stmt1, 0);
        if(temp<0.0000001&&temp>-0.0000001) zero_counter++;
        g_array_append_val(data_array, temp);
        sql_return=sqlite3_step(stmt1);
        i++;
      }
    
    sqlite3_finalize(stmt1);   
    sqlite3_close(cnn);
 
    g_print("SQLite Check Array Length %i First %f Last %f ZeroWarning %i\n", i, g_array_index(data_array, double, 0), g_array_index(data_array, double, i-1), zero_counter);

    if(i>0)
      {
        test_data_button_clicked(NULL, data_array,0,0,0);
      }
    else
      {
        g_print("Couldn't fill array with values.\n");
        simple_message_dialog("Couldn't fill array with values.");
      }

    g_array_free(data_array, TRUE);
    sqlite3_free(sql1);
    if(database!=NULL) g_free(database);
  }








