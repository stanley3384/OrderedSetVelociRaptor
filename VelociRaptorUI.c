
/*
Some ideas for analysis.

This file has int main() along with the UI code.

Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
cecashon@aol.com

Compile with make and makefile.

Compiled on Ubuntu version 12.04 LTS using a netbook as the test computer. Gedit was the text editor used.

*/

//for using popen()
#define _XOPEN_SOURCE //500

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <cairo.h>
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

//Global variables.   
const gchar *pPlateNumberText=NULL;
const gchar *pPlateSizeText=NULL;
const gchar *pPlateStatsText=NULL;
const gchar *pPlatePosControlText=NULL;
const gchar *pPlateNegControlText=NULL;
const gchar *pWindowTitle=NULL;
const gchar *pCurrentFont=NULL;
guint32 iTextArrayCount=0;
guint32 iRandomDataArrayCount=0;
int iReferenceCountDialogWindow=0;
int iBreakLoop=0;
gboolean underline=FALSE;

//Integration parameters in Dunnett's. Declared in VelociRaptorGlobal.h.
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
static void activate_treeview_data_event(GtkWidget*, GtkStateFlags, gpointer);
static void activate_treeview_percent_event(GtkWidget*, GtkStateFlags, gpointer);
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
static void copy_plates_to_clipboard(GtkWidget*, GtkWidget*,int,int,int);
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
static gboolean draw_veloci_raptor(GtkWidget*, cairo_t *cr, gpointer);
static gboolean draw_veloci_raptor_feet(GtkWidget*, cairo_t *cr, gpointer);
static void connect_sqlite_db(GtkWidget *button1, GArray *widgets);
static void get_treeview_selected(GtkWidget *button1, GArray *widgets);
static void get_single_field_values(gchar *table, gchar *field, GArray *widgets);

int main(int argc, char *argv[])
    {
     GtkWidget *window, *button, *scrolled_win, *textview, *MarginCombo, *TextLabel, *PlateParametersLabel, *PlateNumberLabel, *PlateSizeLabel, *PlateStatsLabel, *ControlCheck, *PlatePosControlLabel, *PlateNegControlLabel, *PlateNumberEntry, *PlateSizeEntry, *PlateStatsEntry, *PlatePosControlEntry, *PlateNegControlEntry, *MainTable, *textbutton, *FileMenu, *FileMenu2, *FileMenu3, *FileMenu4, *FileMenu5, *FileMenu6, *PrintItem, *SqliteItem, *ImportItem, *AppendItem, *QuitItem, *BasicStatsItem, *GaussianItem, *VarianceItem, *AnovaItem, *DunnSidakItem, *HotellingItem, *PermutationsItem, *ZFactorItem, *ContingencyItem, *HeatmapItem, *ConditionalItem, *RiseFallItem, *HtmlItem, *HtmlTableItem, *AboutItem, *BuildAuxItem, *BuildComboItem, *BuildPermutItem, *BuildBoardItem, *ScatterItem, *ErrorItem, *BoxItem, *MenuBar, *FileItem, *FileItem2, *FileItem3, *FileItem4, *FileItem5, *FileItem6, *ClearFormat, *RaptorFeet, *UnderlineButton, *SelectionButton, *GlobalButton, *FontChooser; 
     GError *error = NULL;
      
     //For printing
     Widgets *w;
     PangoFontDescription *pfd;

     gtk_init(&argc, &argv);
    
     window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
     gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
     gtk_window_set_title(GTK_WINDOW(window), "Ordered Set VelociRaptor");
     gtk_container_set_border_width(GTK_CONTAINER(window), 8);
     gtk_window_set_default_size(GTK_WINDOW(window), 1024, 300);
     //Set the icon for the launcher.
     GdkPixbuf *pixbuf=gdk_pixbuf_new_from_file("dino.svg", &error);
     if(!pixbuf)
       {
         g_print("%s\n", error->message);
         g_error_free(error);
       }
     else
       {
         gtk_window_set_icon(GTK_WINDOW(window), pixbuf);
       }

     RaptorFeet=gtk_drawing_area_new();
     gtk_widget_set_size_request(RaptorFeet, 1024,35);
     g_signal_connect(G_OBJECT(RaptorFeet), "draw", G_CALLBACK(draw_veloci_raptor_feet), NULL);

     button=gtk_button_new_with_mnemonic("Get Test Data");
     textbutton=gtk_button_new_with_mnemonic("Erase White Board");
     ClearFormat=gtk_button_new_with_mnemonic("Clear Format");
     UnderlineButton=gtk_toggle_button_new_with_label("Underline");
     gtk_widget_set_tooltip_text(UnderlineButton, "Underline Selection Font");
     SelectionButton=gtk_button_new_with_mnemonic("Selection");
     gtk_widget_set_tooltip_text(SelectionButton, "Selection Font");
     GlobalButton=gtk_button_new_with_mnemonic("   Global   ");
     gtk_widget_set_tooltip_text(GlobalButton, "Global Font");
     FontChooser=gtk_font_button_new_with_font("Monospace 9");
     gtk_widget_set_tooltip_text(FontChooser, "Font Chooser");

     FileMenu=gtk_menu_new(); 
     ImportItem=gtk_menu_item_new_with_label("Import Text");
     AppendItem=gtk_menu_item_new_with_label("Append Text Files");
     SqliteItem=gtk_menu_item_new_with_label("Sqlite Connect");
     PrintItem=gtk_menu_item_new_with_label("Print White Board");
     QuitItem=gtk_menu_item_new_with_label("Quit");

     FileMenu2=gtk_menu_new();
     BuildAuxItem=gtk_menu_item_new_with_label("Auxiliary Table");
     BuildComboItem=gtk_menu_item_new_with_label("Combination Table");
     BuildPermutItem=gtk_menu_item_new_with_label("Permutation Table");
     BuildBoardItem=gtk_menu_item_new_with_label("White Board Table");

     FileMenu3=gtk_menu_new();
     BasicStatsItem=gtk_menu_item_new_with_label("Descriptive Statistics");
     GaussianItem=gtk_menu_item_new_with_label("Normality Test");
     VarianceItem=gtk_menu_item_new_with_label("Homogeniety of Variance");
     AnovaItem=gtk_menu_item_new_with_label("One-Way ANOVA");
     DunnSidakItem=gtk_menu_item_new_with_label("Comparison with Control");
     HotellingItem=gtk_menu_item_new_with_label("Comparison with Contrasts");
     PermutationsItem=gtk_menu_item_new_with_label("Permutation Testing");
     ZFactorItem=gtk_menu_item_new_with_label("Calculate Z-factor");
     ContingencyItem=gtk_menu_item_new_with_label("Contingency Data");

     FileMenu4=gtk_menu_new();
     ScatterItem=gtk_menu_item_new_with_label("Scatter Plot");
     ErrorItem=gtk_menu_item_new_with_label("Error Plot");
     BoxItem=gtk_menu_item_new_with_label("Box Plot");

     FileMenu5=gtk_menu_new();
     HeatmapItem=gtk_menu_item_new_with_label("Heatmap Platemap");
     ConditionalItem=gtk_menu_item_new_with_label("Conditional Format Platemap");
     RiseFallItem=gtk_menu_item_new_with_label("Rise Fall Platemap");
     HtmlItem=gtk_menu_item_new_with_label("Heatmap Platemap HTML");
     HtmlTableItem=gtk_menu_item_new_with_label("SQL Query to HTML");

     FileMenu6=gtk_menu_new();
     AboutItem=gtk_menu_item_new_with_label("About");

     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu), ImportItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu), AppendItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu), SqliteItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu), PrintItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu), QuitItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu2), BuildAuxItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu2), BuildComboItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu2), BuildPermutItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu2), BuildBoardItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), BasicStatsItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), GaussianItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), VarianceItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), AnovaItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), DunnSidakItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), HotellingItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), PermutationsItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), ZFactorItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu3), ContingencyItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu4), ScatterItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu4), ErrorItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu4), BoxItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu5), HeatmapItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu5), ConditionalItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu5), RiseFallItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu5), HtmlItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu5), HtmlTableItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(FileMenu6), AboutItem);


     g_signal_connect(QuitItem, "activate", G_CALLBACK(destroy_event), NULL);
     //PrintItem signal connected after textview
     g_signal_connect(ImportItem, "activate", G_CALLBACK(get_text_file), window);
     g_signal_connect(AppendItem, "activate", G_CALLBACK(append_text_dialog), window);
     g_signal_connect(SqliteItem, "activate", G_CALLBACK(sqlite_connect_dialog), window);
     g_signal_connect(AboutItem, "activate", G_CALLBACK(about_dialog), window);
     //AnovaItem signal connected after textview.
     g_signal_connect(ScatterItem, "activate", G_CALLBACK(database_to_scatter_graph_dialog), NULL);
     g_signal_connect(ErrorItem, "activate", G_CALLBACK(database_to_error_graph_dialog), NULL);
     g_signal_connect(BoxItem, "activate", G_CALLBACK(database_to_box_graph_dialog), NULL);
     g_signal_connect(BuildAuxItem, "activate", G_CALLBACK(build_aux_table_dialog), window);
     g_signal_connect(BuildComboItem, "activate", G_CALLBACK(build_combo_table_dialog), window);
     g_signal_connect(BuildPermutItem, "activate", G_CALLBACK(build_permutation_table_dialog), window);
     
     MenuBar=gtk_menu_bar_new();
     gtk_widget_show(MenuBar);
     FileItem=gtk_menu_item_new_with_label("File");
     FileItem2=gtk_menu_item_new_with_label("Data");
     FileItem3=gtk_menu_item_new_with_label("Analysis");
     FileItem4=gtk_menu_item_new_with_label("Graph");
     FileItem5=gtk_menu_item_new_with_label("Format");
     FileItem6=gtk_menu_item_new_with_label("Help");

     gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileItem), FileMenu);
     gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileItem2), FileMenu2);
     gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileItem3), FileMenu3);
     gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileItem4), FileMenu4);
     gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileItem5), FileMenu5);
     gtk_menu_item_set_submenu(GTK_MENU_ITEM(FileItem6), FileMenu6);

     gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), FileItem);
     gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), FileItem2);
     gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), FileItem3);
     gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), FileItem4);
     gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), FileItem5);
     gtk_menu_shell_append(GTK_MENU_SHELL(MenuBar), FileItem6);
       
     textview=gtk_text_view_new();
     //Set initial font.
     pfd=pango_font_description_from_string("Monospace 9"); 
     gtk_widget_override_font(GTK_WIDGET(textview), pfd);
     g_signal_connect(UnderlineButton, "clicked", G_CALLBACK(change_underline), textview);
     g_signal_connect(SelectionButton, "clicked", G_CALLBACK(change_selection_font), textview);
     g_signal_connect(GlobalButton, "clicked", G_CALLBACK(change_global_font), textview);
     g_signal_connect(FontChooser, "font-set", G_CALLBACK(font_chooser_dialog), textview);

     g_signal_connect(BasicStatsItem, "activate", G_CALLBACK(basic_statistics_dialog), textview);
     g_signal_connect(GaussianItem, "activate", G_CALLBACK(gaussian_dialog), textview);
     g_signal_connect(VarianceItem, "activate", G_CALLBACK(homogeniety_of_variance_dialog), textview);
     g_signal_connect(AnovaItem, "activate", G_CALLBACK(one_way_anova_dialog), textview);
     g_signal_connect(DunnSidakItem, "activate", G_CALLBACK(comparison_with_control_dialog), textview);
     g_signal_connect(HotellingItem, "activate", G_CALLBACK(hotelling_dialog), textview);
     g_signal_connect(PermutationsItem, "activate", G_CALLBACK(permutations_dialog), textview);
     g_signal_connect(ZFactorItem, "activate", G_CALLBACK(z_factor_dialog), textview);
     g_signal_connect(ContingencyItem, "activate", G_CALLBACK(contingency_dialog), textview);

     g_signal_connect(G_OBJECT(HeatmapItem), "activate", G_CALLBACK(heatmap_dialog), textview);
     g_signal_connect(G_OBJECT(ConditionalItem), "activate", G_CALLBACK(format_text_dialog), textview);
     g_signal_connect(G_OBJECT(RiseFallItem), "activate", G_CALLBACK(rise_fall_text_dialog), textview);
     g_signal_connect(G_OBJECT(HtmlItem), "activate", G_CALLBACK(heatmap_html_dialog), NULL);
     g_signal_connect(G_OBJECT(HtmlTableItem), "activate", G_CALLBACK(html_table_dialog), NULL);
     g_signal_connect(G_OBJECT(BuildBoardItem), "activate", G_CALLBACK(send_text_to_database_dialog), textview);

     //For printing.
     w=g_slice_new(Widgets);
     w->window=GTK_WIDGET(window);
     w->textview=GTK_WIDGET(textview);
     g_signal_connect(PrintItem, "activate", G_CALLBACK(print_textview), (gpointer) w);
        
     scrolled_win=gtk_scrolled_window_new(NULL, NULL);
     
     gtk_container_add(GTK_CONTAINER(scrolled_win), textview);

     PlateParametersLabel=gtk_label_new("Parameters");
     TextLabel=gtk_label_new("White Board");
     PlateNumberLabel=gtk_label_new("Number of Plates");
     PlateSizeLabel=gtk_label_new("Size of Plate");
     PlateStatsLabel=gtk_label_new("Set Size for Stats");
     PlatePosControlLabel=gtk_label_new("Positive Controls");
     PlateNegControlLabel=gtk_label_new("Negative Controls");

     ControlCheck=gtk_check_button_new_with_label("Optional Control Locations      ");

     PlateNumberEntry=gtk_entry_new();
     PlateSizeEntry=gtk_entry_new();
     PlateStatsEntry=gtk_entry_new();
     PlatePosControlEntry=gtk_entry_new();
     PlateNegControlEntry=gtk_entry_new();

     gtk_entry_set_text(GTK_ENTRY(PlateNumberEntry), "3");
     gtk_entry_set_text(GTK_ENTRY(PlateSizeEntry), "96");
     gtk_entry_set_text(GTK_ENTRY(PlateStatsEntry), "4");
     gtk_entry_set_text(GTK_ENTRY(PlatePosControlEntry), "");
     gtk_entry_set_text(GTK_ENTRY(PlateNegControlEntry), "");

     gtk_widget_set_sensitive(GTK_WIDGET(PlatePosControlEntry),FALSE);
     gtk_widget_set_sensitive(GTK_WIDGET(PlateNegControlEntry),FALSE);

     g_signal_connect(G_OBJECT(ControlCheck), "clicked", G_CALLBACK(activate_pos_control_event), PlatePosControlEntry);
     g_signal_connect(G_OBJECT(ControlCheck), "clicked", G_CALLBACK(activate_neg_control_event), PlateNegControlEntry);

     //Set global pointers to values.
     pPlateNumberText=gtk_entry_get_text(GTK_ENTRY(PlateNumberEntry));
     pPlateSizeText=gtk_entry_get_text(GTK_ENTRY(PlateSizeEntry));
     pPlateStatsText=gtk_entry_get_text(GTK_ENTRY(PlateStatsEntry));
     pPlatePosControlText=gtk_entry_get_text(GTK_ENTRY(PlatePosControlEntry));
     pPlateNegControlText=gtk_entry_get_text(GTK_ENTRY(PlateNegControlEntry));
     pCurrentFont=gtk_font_button_get_font_name(GTK_FONT_BUTTON(FontChooser));
     
     gtk_entry_set_width_chars(GTK_ENTRY(PlateNumberEntry), 5);
     gtk_entry_set_width_chars(GTK_ENTRY(PlateSizeEntry), 5);
     gtk_entry_set_width_chars(GTK_ENTRY(PlateStatsEntry), 5);
     gtk_entry_set_width_chars(GTK_ENTRY(PlatePosControlEntry), 10);
     gtk_entry_set_width_chars(GTK_ENTRY(PlateNegControlEntry), 10);

     MarginCombo=gtk_combo_box_text_new_with_entry();
     gtk_widget_set_tooltip_text(MarginCombo, "Left Margin");
     GtkWidget *combo_entry=gtk_bin_get_child(GTK_BIN(MarginCombo));
     gtk_entry_set_width_chars(GTK_ENTRY(combo_entry), 3);
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "0");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "10");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "20");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "30");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "40");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "50");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(MarginCombo), "0", "60");
     gtk_combo_box_set_active(GTK_COMBO_BOX(MarginCombo), 0);
     g_signal_connect(MarginCombo, "changed", G_CALLBACK(change_margin), textview);

     MainTable=gtk_table_new(10,8,TRUE);
     gtk_table_attach(GTK_TABLE(MainTable), RaptorFeet, 0,8,0,1,GTK_FILL,GTK_FILL,0,0);

     gtk_table_attach(GTK_TABLE(MainTable), TextLabel, 3,6,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateParametersLabel, 0,2,1,2,GTK_FILL,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateNumberLabel, 0,1,2,3,GTK_FILL,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateSizeLabel, 0,1,3,4,GTK_FILL,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateStatsLabel, 0,1,4,5,GTK_FILL,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), ControlCheck, 0,2,5,6,GTK_FILL,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlatePosControlLabel, 0,1,6,7,GTK_FILL,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateNegControlLabel, 0,1,7,8,GTK_FILL,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(MainTable), PlateNumberEntry, 1,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateSizeEntry, 1,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateStatsEntry, 1,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlatePosControlEntry, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), PlateNegControlEntry, 1,2,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
   
     gtk_table_attach(GTK_TABLE(MainTable), scrolled_win, 2,7,2,9,GTK_FILL,GTK_FILL,0,0);

     gtk_table_attach(GTK_TABLE(MainTable), MenuBar, 6,8,0,1,GTK_FILL,GTK_FILL,0,0);

     gtk_table_attach(GTK_TABLE(MainTable), button, 0,2,8,9,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(MainTable), textbutton, 2,3,9,10,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), ClearFormat, 3,4,9,10,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), FontChooser, 4,8,9,10,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(MainTable), MarginCombo, 7,8,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), SelectionButton, 7,8,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(MainTable), UnderlineButton, 7,8,7,8,GTK_SHRINK,GTK_SHRINK,0,0);  
     gtk_table_attach(GTK_TABLE(MainTable), GlobalButton, 7,8,8,9,GTK_SHRINK,GTK_SHRINK,0,0);    

     gtk_table_set_row_spacings(GTK_TABLE(MainTable), 1);
     gtk_table_set_col_spacing(GTK_TABLE(MainTable), 1, 2);

     g_signal_connect(G_OBJECT(button), "clicked", G_CALLBACK(distributions_dialog), NULL);
     g_signal_connect(G_OBJECT(textbutton), "clicked", G_CALLBACK(text_button_clicked), (gpointer) textview);
     g_signal_connect(G_OBJECT(ClearFormat), "clicked", G_CALLBACK(clear_format_event), textview);

     //Attempt to break out of a focus event without runtime errors.
     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), PlateNumberEntry);
     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), PlateSizeEntry);
     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), PlateStatsEntry);
     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), PlatePosControlEntry);
     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), PlateNegControlEntry);

     g_signal_connect(G_OBJECT(window), "destroy", G_CALLBACK(destroy_event), NULL); 
 
     //connect_after to try to avoid a run-time error when moving from the entry fields to a dialog.
     g_signal_connect_after(G_OBJECT(PlateNumberEntry), "focus_out_event", G_CALLBACK(entry_field_changed), textview);
     g_signal_connect_after(G_OBJECT(PlateSizeEntry), "focus_out_event", G_CALLBACK(entry_field_changed), textview);
     g_signal_connect_after(G_OBJECT(PlateStatsEntry), "focus_out_event", G_CALLBACK(entry_field_changed), textview); 
     g_signal_connect_after(G_OBJECT(PlatePosControlEntry), "focus_out_event", G_CALLBACK(control_changed), textview);
     g_signal_connect(G_OBJECT(PlateNegControlEntry), "focus_out_event", G_CALLBACK(control_changed), textview);  
    
     gtk_container_add(GTK_CONTAINER(window), MainTable);
     
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
        underline=TRUE;
      }
    else
      {
        underline=FALSE;
      }
  }
static void change_selection_font(GtkWidget *button, GtkTextView *textview)
  {
    //Set the font for the textview.
    int i=0;
    int as_return=0;
    static int tag_counter=0;
    GString *tag_name=g_string_new(NULL);
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(textview);
    gtk_text_buffer_get_selection_bounds(buffer, &start, &end);
    g_string_printf(tag_name, "tag%i", tag_counter);

    //Remove prior selection tags in the current selection.
    for(i=0;i<tag_counter;i++)
       {
         char *string=NULL;
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

    if(!underline)
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
    gtk_widget_override_font(GTK_WIDGET(textview), pfd);
  }
static void change_margin(GtkWidget *margin, GtkTextView *textview)
  {
    int left_margin=0;
    left_margin=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(margin)));
    //printf("Left Margin %i\n", left_margin);
    gtk_text_view_set_indent(textview, left_margin);
  }
static void activate_treeview_data_event(GtkWidget *dialog, GtkStateFlags response, gpointer data)
  {
      /*typedef enum {
  GTK_STATE_FLAG_NORMAL       = 0,
  GTK_STATE_FLAG_ACTIVE       = 1 << 0,
  GTK_STATE_FLAG_PRELIGHT     = 1 << 1,
  GTK_STATE_FLAG_SELECTED     = 1 << 2,
  GTK_STATE_FLAG_INSENSITIVE  = 1 << 3,
  GTK_STATE_FLAG_INCONSISTENT = 1 << 4,
  GTK_STATE_FLAG_FOCUSED      = 1 << 5,
  GTK_STATE_FLAG_BACKDROP     = 1 << 6
} GtkStateFlags;*/

    //int choice=1;
    //printf("Response %i\n", response);
    if(response==64)
      {
        pWindowTitle=gtk_window_get_title(GTK_WINDOW(dialog));
        //printf("Active Dialog Window Title %s\n", pWindowTitle);
      }
       
  }
static void activate_treeview_percent_event(GtkWidget *dialog, GtkStateFlags response, gpointer data)
  {
       /*typedef enum {
  GTK_STATE_FLAG_NORMAL       = 0,
  GTK_STATE_FLAG_ACTIVE       = 1 << 0,
  GTK_STATE_FLAG_PRELIGHT     = 1 << 1,
  GTK_STATE_FLAG_SELECTED     = 1 << 2,
  GTK_STATE_FLAG_INSENSITIVE  = 1 << 3,
  GTK_STATE_FLAG_INCONSISTENT = 1 << 4,
  GTK_STATE_FLAG_FOCUSED      = 1 << 5,
  GTK_STATE_FLAG_BACKDROP     = 1 << 6
} GtkStateFlags;*/
    //int choice=1;
   
    //printf("Response %i\n", response);
    if(response==64)
      {
        pWindowTitle=gtk_window_get_title(GTK_WINDOW(dialog));
        printf("Active Dialog Window Title %s\n", pWindowTitle);
      }
  }
static void dialog_reference_destroy(GtkWidget *dialog , gint response, gpointer data)
  {
    const gchar *title;
    //GTK_RESPONSE_DELETE_EVENT = -4,

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
    GtkWidget *dialog, *table, *label1, *label2, *label3, *label4, *label5, *label6, *entry1, *entry2, *entry3, *entry4, *entry5, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Get Test Data", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Gaussian Distribution       ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Chi-Squared Distribution");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Rayleigh Distribution        ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Uniform Distribution        ");
     
     label1=gtk_label_new("Generate Some Numbers For Testing.");
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

     table=gtk_table_new(8,3,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,3,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,1,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,1,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,1,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,1,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 1,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 1,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label4, 1,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label5, 1,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(table), entry1, 2,3,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry2, 2,3,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry3, 2,3,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry4, 2,3,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
 
     gtk_table_attach(GTK_TABLE(table), label6, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry5, 2,3,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
 
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         int iRadioButton=0;
         int seed_value=0;
         double param=1;

          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
              seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
              param=atof(gtk_entry_get_text(GTK_ENTRY(entry1))); 
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
              seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
              param=atof(gtk_entry_get_text(GTK_ENTRY(entry2))); 
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
              seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
              param=atof(gtk_entry_get_text(GTK_ENTRY(entry3))); 
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
              seed_value=atoi(gtk_entry_get_text(GTK_ENTRY(entry5))); 
              param=atof(gtk_entry_get_text(GTK_ENTRY(entry4))); 
            }
         else
            {
              //exit
            }
          if(seed_value>=0&&param>=1)
            {
              test_data_button_clicked(button, NULL, seed_value, param, iRadioButton);
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
     GtkWidget *dialog, *table, *label1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Basic Statistics", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Basic Statistics from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Basic Statistics from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Basic Statistics from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Basic Statistics from Percent By Picks    ");
     
     label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database");

     table=gtk_table_new(7,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
 
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         int iRadioButton=0;

          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }

          if(groups_database_validation(NULL)==0)
            {
              basic_statistics_sql(textview, iRadioButton);
            }
       }
     gtk_widget_destroy(dialog);
   }
static void gaussian_dialog(GtkWidget *menu, GtkTextView *textview)
   {
     GtkWidget *dialog, *table, *label1, *label2, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Normality Test", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, " Normality Test from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Normality Test from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Normality Test from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Normality Test from Percent By Picks    ");
     
     label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database.");
     label2=gtk_label_new("Anderson Darling test using 1.0+0.75/n+2.25/n^2\nfor the adjusted value. Easy to change the code\nfor other adjusted values. If p_value>alpha then\ncan't reject the null hypothesis of normality.");

     table=gtk_table_new(8,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
 
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         printf("Begin Anderson Darling\n");
         int iRadioButton=0;
         
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }

         if(groups_database_validation(NULL)==0)
           {
             anderson_darling_test(textview, iRadioButton);
           }
   
       }
     printf("Anderson Darling Finished\n");
     gtk_widget_destroy(dialog);
   }
static void homogeniety_of_variance_dialog(GtkWidget *menu, GtkTextView *textview)
   {
     GtkWidget *dialog, *table, *label1, *label2, *label3, *entry1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Homogeniety of Variance", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, " Variance from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Variance from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Variance from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Variance from Percent By Picks    ");
     
     label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database.");
     label2=gtk_label_new("Levene's test using the median");
     label3=gtk_label_new("Alpha for Critical Value");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");

     table=gtk_table_new(9,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry1, 1,2,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
 
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         int iRadioButton=0;
         int check1=0;
         double alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1)));       

         printf("Begin Levene's\n");
         
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }

         check1=critical_value_changed_validation(entry1);
     
         if(check1==0)
           {
             if(groups_database_validation(NULL)==0)
               {
                 levenes_variance_test(textview, iRadioButton, alpha);
               }
           }
   
       printf("Levene's Finished\n");
       }
     gtk_widget_destroy(dialog);
   }
static void one_way_anova_dialog(GtkWidget *menu, GtkTextView *textview)
   {
    /*
      Calculations based on
         De Muth, J.E.(2006). "Basic Statistics and Pharmaceutical Statistical Applications", Second Edition, Chapman & Hall/CRC, p. 208-209.
    */
    GtkWidget *dialog, *table, *label1, *label2, *entry1, *radio1, *radio2, *radio3, *radio4, *check_button1, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("One-Way ANOVA", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, " One-Way ANOVA from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "One-Way ANOVA from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "  One-Way ANOVA from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), " One-Way ANOVA from Percent By Picks    ");
     
     label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database");
     label2=gtk_label_new("                  Alpha for Critical Value");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");
     
     check_button1=gtk_check_button_new_with_label("Standard ANOVA Format");

     table=gtk_table_new(9,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry1, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), check_button1, 1,2,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
 
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         int check_box=0;
         int iRadioButton=0;
         int check1=0;
         double alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1)));       

         printf("Begin One-Way ANOVA\n");
         
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
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
                 one_way_anova_sql(textview, iRadioButton, check_box, alpha);
               }
           }
   
       printf("ANOVA Finished\n");
      }
     gtk_widget_destroy(dialog);
    }
static void comparison_with_control_dialog(GtkWidget *menu, GtkTextView *textview)
    {
       GtkWidget *dialog, *table, *label1, *label2, *label3, *entry1, *entry2, *radio1, *radio2, *radio3, *radio4, *radio_bonferroni, *radio_sidak, *radio_dunnetts, *radio_hotellingsT2, *progress, *content_area, *action_area, *dunnett_button;
    int result;

     dialog=gtk_dialog_new_with_buttons("Comparison with Control", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups                          ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups                    ");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "  Comparison from Data By Picks                              ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), " Comparison from Percent By Picks                        ");

     dunnett_button=gtk_button_new_with_mnemonic("  !  ");
     g_signal_connect(G_OBJECT(dunnett_button), "clicked", G_CALLBACK(dunnetts_parameters_dialog), NULL);

     radio_bonferroni=gtk_radio_button_new_with_label(NULL, "Bonferroni Critical Values   ");
     radio_sidak=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_bonferroni), "Dunn-Sidak Critical Values   ");
     radio_dunnetts=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_sidak), "Dunnett's Critical Values        ");
     radio_hotellingsT2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio_sidak), "Hotelling's T2 Critical Values\n (Balanced Sets Only)");
     
     label1=gtk_label_new(" Build Auxiliary Table First. Data Pulled From the Database.\n Control is the Groups or Picks Value From the Auxiliary Table.");
     label2=gtk_label_new("Alpha for Critical Value(two-tail)");
     label3=gtk_label_new("Control GROUP BY Value             ");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 5);
     gtk_entry_set_text(GTK_ENTRY(entry2), "1");

     progress=gtk_progress_bar_new();
     gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
     
     table=gtk_table_new(14,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,6,7,GTK_FILL,GTK_FILL,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,7,8,GTK_FILL,GTK_FILL,0,0); 
     gtk_table_attach(GTK_TABLE(table), entry1, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry2, 1,2,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio_bonferroni, 0,1,8,9,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio_sidak, 0,1,9,10,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio_dunnetts, 0,1,10,11,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), dunnett_button, 1,2,10,11,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio_hotellingsT2, 0,1,11,12,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), progress, 0,2,12,13,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);

     g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_comparison_loop_event), NULL);

     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         printf("Begin Multiple Comparisons\n");
         int iRadioButton=1;
         int iRadioCritVal=1;
         double alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1))); 
         int iControlValue=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
         
         int check1=0;
         int check2=0;
         int check3=0; 

         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }
         else
            {
              //exit
            }

         //Choose type of critical values.
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_bonferroni)))
            {
              iRadioCritVal=1;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_sidak)))
            {
              iRadioCritVal=2;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_dunnetts)))
            {
              iRadioCritVal=3;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio_hotellingsT2)))
            {
              iRadioCritVal=4;
            }
         else
            {
              //exit
            }

         //check entry fields
         check1=critical_value_changed_validation(entry1);
         if(iRadioButton==1||iRadioButton==2)
           {
             check2=groups_database_validation(entry2);
           }
         if(iRadioButton==3||iRadioButton==4)
           {
             check3=picks_database_validation(entry2);
           }           

         if(check1==0&&check2==0&&check3==0)
           {
             if(iRadioCritVal==4)
               {
                 printf("Call Hotellings T2\n");
                 hotellings_T2(iRadioButton,alpha,0,iControlValue-1,textview,progress,NULL,0);
               }
             else
               {
                 comparison_with_control_sql(iRadioButton, iControlValue, alpha, iRadioCritVal, textview, progress, &iBreakLoop); 
               }
           }

        printf("Multiple Comparisons Finished\n");
      }
     gtk_widget_destroy(dialog);
    }
static void dunnetts_parameters_dialog(GtkWidget *dialog, gpointer data)
    {
      GtkWidget *dialog2, *table, *label1, *label2, *label3, *value_entry1, *value_entry2, *content_area, *action_area;
    int result;

    dialog2=gtk_dialog_new_with_buttons("Dunnett's Integration Parameters", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog2), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog2), 20);

     label1=gtk_label_new("Parameters to set for numerical integration. Increase\nMax Points if returned Error is too large for the Value\nreturned from integration. The Value(Error) pair is\noutput to the terminal. If Inform = 0, normal completion\nof integration. A lot of points to evaluate can take some\ntime.");
     label2=gtk_label_new("Max Points");
     label3=gtk_label_new("Error Tolerance for Alpha");
    
     value_entry1=gtk_entry_new();
     value_entry2=gtk_entry_new();

     gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 7);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 7);

     gtk_entry_set_text(GTK_ENTRY(value_entry1), "1000");
     gtk_entry_set_text(GTK_ENTRY(value_entry2), "0.01");
 
     table=gtk_table_new(4,2,FALSE);
    
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,2,3,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_attach(GTK_TABLE(table), value_entry1, 1,2,1,2,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry2, 1,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog2));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog2));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog2);
     result=gtk_dialog_run(GTK_DIALOG(dialog2));

     if(result==GTK_RESPONSE_OK)
        {
         int temp1=0;
         double temp2=0;

         temp1=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry1))); 
         temp2=atof(gtk_entry_get_text(GTK_ENTRY(value_entry2)));

         //Set Global variables.
         if(temp1>=1000&&temp1<=100000)
           {
             MAXPTS_C=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry1))); 
             printf("MAXPTS=%i\n", MAXPTS_C);
           }
         else
           {
             printf("Max Points Values 1000<=x<=100000\n");
             simple_message_dialog("Max Points Values 1000<=x<=100000");
           }

         if(temp2>=0.000001&&temp2<=0.1)
           {
             ABSEPS_C=atof(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
             printf("ABSEPS=%f\n", ABSEPS_C);
           }
         else
           {
             printf("Error Tolerance for Alpha 0.000001<=x<=0.1\n");
             simple_message_dialog("Error Tolerance for Alpha 0.000001<=x<=0.1");
           }
         
        }
    gtk_widget_destroy(dialog2);
  }
static void hotelling_dialog(GtkWidget *menu, GtkTextView *textview)
    {
      GtkWidget *dialog, *table, *label1, *label2, *label3, *entry1, *radio1, *radio2, *radio3, *radio4, *textview1, *scroll1, *space1, *space2, *space3, *space4, *progress, *content_area, *action_area;
    GtkTextBuffer *buffer1;
    GtkTextIter start1;
    GtkTextIter end1;
    int iRadioButton=1;
    int result;

     dialog=gtk_dialog_new_with_buttons("Comparison with Contrasts", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "  Comparison from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), " Comparison from Percent By Picks    ");
    
     label1=gtk_label_new("Build Auxiliary Table First. Data Pulled From the Database.\nControl is the Groups or Picks Value From the Auxiliary Table.\nFor balanced sets only. The contrast matrix can only have\n1's, 0's or -1's for now.");
     label2=gtk_label_new("        Alpha for Hotelling's T2 Critical Value");
     label3=gtk_label_new("Contrast Matrix  ");

     space1=gtk_label_new("");
     space2=gtk_label_new("");
     space3=gtk_label_new("");
     space4=gtk_label_new("");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "0.05");

     textview1=gtk_text_view_new();
     buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1));
     gtk_text_buffer_insert_at_cursor(buffer1, "-1 -1 1 1\n1 -1 1 -1\n1 -1 -1 1", -1);
     scroll1=gtk_scrolled_window_new(NULL, NULL); 
     gtk_container_add(GTK_CONTAINER(scroll1), textview1);

     progress=gtk_progress_bar_new();
     gtk_progress_bar_set_show_text(GTK_PROGRESS_BAR(progress), TRUE);
     
     table=gtk_table_new(14,4,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,4,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,4,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,4,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,4,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,4,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0); 
     gtk_table_attach(GTK_TABLE(table), entry1, 3,4,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,4,8,9,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), space1, 0,1,9,10,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), space2, 0,1,10,11,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), space3, 0,1,11,12,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), space4, 0,1,12,13,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), scroll1, 1,2,9,13,GTK_FILL,GTK_FILL,0,0);
     gtk_table_attach(GTK_TABLE(table), progress, 0,4,13,14,GTK_EXPAND,GTK_EXPAND,20,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 7);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_hotelling_dialog), NULL);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         printf("Begin Hotelling's T2 Comparisons\n");
         gsl_matrix *SuppliedContrasts=NULL;
         int rows=0;
         int columns=0;
         int check1=0;
         int check2=0;
         int check3=0;
         int numbers=0;

         double alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry1))); 
         //get string from textview
         gtk_text_buffer_get_bounds(buffer1, &start1, &end1);
         gchar *string1=gtk_text_buffer_get_text(buffer1, &start1, &end1, TRUE);

         //get value of radiobutton.
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }

         //check the contrast matrix for errors
         check1=contrast_matrix_validation(string1, &rows, &columns);
         printf("Check = %i rows = %i columns = %i\n", check1, rows, columns);
  
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
                 hotellings_T2(iRadioButton, alpha, 0, -1, textview, progress, SuppliedContrasts, columns);
               }
           }

         if(SuppliedContrasts!=NULL)
           {
            gsl_matrix_free(SuppliedContrasts);
           }
         g_free(string1);
       }
     printf("Hotelling's T2 Finished\n");
     gtk_widget_destroy(dialog);
    }
static void exit_comparison_loop_event(GtkWidget *dialog , gint response, gpointer data)
     {
       if(response==GTK_RESPONSE_CANCEL||response==GTK_RESPONSE_CLOSE)
         {
           printf("Exit Loop\n");
           //gtk_widget_destroy(dialog);
           iBreakLoop=1;//Break out of multiple comparison loop.
         }

     }
static void exit_hotelling_dialog(GtkWidget *dialog , gint response, gpointer data)
     {
       if(response==GTK_RESPONSE_CANCEL||response==GTK_RESPONSE_CLOSE)
         {
           printf("Hotelling's T2 Dialog Cancelled.\n");
           //gtk_widget_destroy(dialog);
         }

     }
static void permutations_dialog(GtkWidget *menu, GtkTextView *textview)
     {
       GtkWidget *dialog, *table, *label1, *label2, *label3, *label4, *label5, *label6, *label7, *label8, *entry1, *entry2, *entry3, *radio1, *radio2, *radio3, *radio4, *random_radio1, *random_radio2, *random_radio3, *tail_combo, *test_combo, *p_function, *progress, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Permutation Testing", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "  Comparison from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), " Comparison from Percent By Picks    ");

     random_radio1=gtk_radio_button_new_with_label(NULL, "Mersenne Twister 19937");
     random_radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(random_radio1), "Tausworthe 2                        ");
     random_radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(random_radio1), "RANLUX 389                          ");

     label1=gtk_label_new(" Build Auxiliary Table First. Data Pulled From the Database.\n Control is the Groups or Picks Value From the Auxiliary Table.");
     label2=gtk_label_new("Control GROUP BY Value");
     label3=gtk_label_new("Number of Permutations");
     label4=gtk_label_new("Random Number Generators");
     label5=gtk_label_new("                      Seed Value");
     label6=gtk_label_new("Probability Side");
     label7=gtk_label_new("Test Statistic");
     label8=gtk_label_new("Probability Function");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "1");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
     gtk_entry_set_text(GTK_ENTRY(entry2), "10000");

     entry3=gtk_entry_new();
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
     
     table=gtk_table_new(15,4,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,4,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,4,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,4,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,4,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,4,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,6,7,GTK_SHRINK,GTK_SHRINK,0,0); 
     gtk_table_attach(GTK_TABLE(table), entry1, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 2,3,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry2, 3,4,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label6, 0,1,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), tail_combo, 1,2,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label7, 2,3,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), test_combo, 3,4,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label8, 0,1,8,9,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), p_function, 1,2,8,9,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(table), label4, 0,4,9,10,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), random_radio1, 0,4,10,11,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), random_radio2, 0,4,11,12,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), random_radio3, 0,4,12,13,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label5, 1,2,13,14,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry3, 2,3,13,14,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(table), progress, 0,4,14,15,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_comparison_loop_event), NULL);

     gtk_widget_show_all(dialog);

     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         printf("Begin Permutation Calculation\n");
         int check1=0;
         int check2=0;
         int iRadioButton=1;
         int iRandomButton=1;
         int iTail=gtk_combo_box_get_active(GTK_COMBO_BOX(tail_combo))+1;
         int iTest=gtk_combo_box_get_active(GTK_COMBO_BOX(test_combo))+1;
         int iFunction=gtk_combo_box_get_active(GTK_COMBO_BOX(p_function))+1;
         int iControl=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
         int iPermutations= atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
         int iSeedValue=atoi(gtk_entry_get_text(GTK_ENTRY(entry3)));
         
         //Get value of radiobutton.
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }

         //Get random generator number.
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_radio1)))
            {
              iRandomButton=1;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_radio2)))
            {
              iRandomButton=2;
            }
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(random_radio3)))
            {
              iRandomButton=3;
            }
  
         //Check that values are in the database.
         if(iRadioButton==1||iRadioButton==2)
           {
             check1=groups_database_validation(entry1);
           }
         if(iRadioButton==3||iRadioButton==4)
           {
             check2=picks_database_validation(entry1);
           }  

         //Set some bounds for the number of permutations
         if(iPermutations<10||iPermutations>500000)
           {
             printf("Permutations Bounds 10<=x<=500000\n");
             simple_message_dialog("Permutations Bounds 10<=x<=500000");
           }
         else if(iSeedValue<0||iSeedValue>100000000)
           {
             printf("Seed Value Bounds 0<=x<=10000000\n");
             simple_message_dialog("Seed Value Bounds 0<=x<=10000000");
           }
         else if(check1!=0||check2!=0)
           {
             //Exit. Message dialog in database validation functions.
             printf("Exit Permutations\n");
           }
         else
           {
             if(iFunction==1)
               {
                 unadjusted_p_sql(iPermutations, iRadioButton, iControl, iTail, iTest, iFunction ,textview, GTK_PROGRESS_BAR(progress), &iBreakLoop, iSeedValue, iRandomButton);
               }
             if(iFunction==2||iFunction==3)
               {
                unadjusted_p_sql(iPermutations, iRadioButton, iControl, iTail, iTest, iFunction ,textview, GTK_PROGRESS_BAR(progress), &iBreakLoop, iSeedValue, iRandomButton);
                 if(iBreakLoop==0)
                   {
                     minP_sql(iPermutations, iRadioButton, iControl, iTail, iTest , iFunction, textview, GTK_PROGRESS_BAR(progress), &iBreakLoop, iSeedValue, iRandomButton);
                   }
                 else
                   {
                     iBreakLoop=0;
                   }
                }          
           }

         
       }
      printf("Permutations Finished\n");
      gtk_widget_destroy(dialog);
     
     }
static void z_factor_dialog(GtkWidget *menu, GtkTextView *textview)
     {
       GtkWidget *dialog, *table, *label1, *label2, *entry1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Calculate Z-factor", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Comparison from Data by Groups      ");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Comparison from Percent By Groups");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "  Comparison from Data By Picks          ");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), " Comparison from Percent By Picks    ");

     label1=gtk_label_new(" Build Auxiliary Table First. Data Pulled From the Database.\n Control is the Groups or Picks Value From the Auxiliary Table.");
     label2=gtk_label_new("Control GROUP BY Value");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "1");
     
     table=gtk_table_new(9,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio1, 0,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio2, 0,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio3, 0,2,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), radio4, 0,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,6,7,GTK_SHRINK,GTK_SHRINK,0,0); 
     gtk_table_attach(GTK_TABLE(table), entry1, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_z_factor_dialog), NULL);

     gtk_widget_show_all(dialog);

     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         printf("Begin Z-factor Calculation\n");
         int iRadioButton=1;
         int iControl=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
         int check1=0; 
         
         //get value of radiobutton.
         if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              iRadioButton=1;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
            {
              iRadioButton=2;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
            {
              iRadioButton=3;
            }
         else if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
            {
              iRadioButton=4;
            }
         else
            {
              //exit
            }

         //check entry is in database
         if(iRadioButton==1||iRadioButton==2)
           {
             check1=groups_database_validation(entry1);
           }
         if(iRadioButton==3||iRadioButton==4)
           {
             check1=picks_database_validation(entry1);
           }

         if(check1==0)
           {
             z_factor(iRadioButton, iControl, textview);
           }
       }
      printf("Z-factor Finished\n");
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
       GtkWidget *dialog, *table, *label1, *label2, *label3, *label4, *label5, *entry1, *entry2, *entry3, *check_button1, *check_button2, *check_button3, *check_button4, *check_button5, *check_button6, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Contingency Data", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("Data is pulled from the database as a single column.\nCalculations done in RC format. Warning! This is summed\ndata. Only the data needs to be loaded into the database.\nNo percent column or auxiliary table is needed for this\ncalculation.");
     label2=gtk_label_new("Rows");
     label3=gtk_label_new("Columns");
     label4=gtk_label_new("Chi-squared Critical Value");
     label5=gtk_label_new("Measures of Association");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 5);
     gtk_entry_set_text(GTK_ENTRY(entry1), "3");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 5);
     gtk_entry_set_text(GTK_ENTRY(entry2), "3");

     entry3=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry3), 5);
     gtk_entry_set_text(GTK_ENTRY(entry3), "0.05");

     check_button1=gtk_check_button_new_with_label("Pearson C      ");
     check_button2=gtk_check_button_new_with_label("Pearson C*    ");
     check_button3=gtk_check_button_new_with_label("Tshuprow's T");
     check_button4=gtk_check_button_new_with_label("Cramer's V");
     check_button5=gtk_check_button_new_with_label("Somer's d   ");
     check_button6=gtk_check_button_new_with_label("Gamma        ");
     
     table=gtk_table_new(9,2,FALSE);
     gtk_table_attach(GTK_TABLE(table), label1, 0,2,0,1,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,1,2,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label4, 0,1,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry1, 1,2,1,2,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry2, 1,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0); 
     gtk_table_attach(GTK_TABLE(table), entry3, 1,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label5, 0,1,4,5,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(table), check_button1, 0,1,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), check_button2, 0,1,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), check_button3, 0,1,7,8,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), check_button4, 1,2,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), check_button5, 1,2,6,7,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), check_button6, 1,2,7,8,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     //g_signal_connect(G_OBJECT(dialog), "response", G_CALLBACK(exit_z_factor_dialog), NULL);

     gtk_widget_show_all(dialog);

     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
         printf("Begin Contingency Calculation\n");
         int rows=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
         int columns=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
         double alpha=atof(gtk_entry_get_text(GTK_ENTRY(entry3)));
         int plates=atoi(pPlateNumberText);
         int check_box1=0;
         int check_box2=0;
         int check_box3=0;
         int check_box4=0;
         int check_box5=0;
         int check_box6=0;
         int check1=0;
         int check2=0;

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

         printf("Check Boxes %i %i %i %i %i %i\n", check_box1, check_box2, check_box3, check_box4, check_box5, check_box6);     
         printf("Row=%i Columns=%i Plates=%i Alpha=%f\n", rows, columns, plates, alpha);

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
      printf("Contingency Finished\n");
      gtk_widget_destroy(dialog);
     }
static void database_to_scatter_graph_dialog(GtkWidget *menu , gpointer data)
  {
    GtkWidget *dialog, *table, *entry1, *entry2, *label1, *label2, *radio1, *radio2, *content_area, *action_area;
    int result;
    
    g_print("Send Data from Database to Graph\n");

     dialog=gtk_dialog_new_with_buttons("Scatter Plot", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Data Scatter Plot");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Percent Scatter Plot");
     
     label1=gtk_label_new("Records Lower Bound");
     label2=gtk_label_new("Records Upper Bound");     

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
     gtk_entry_set_text(GTK_ENTRY(entry1), "1");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
     gtk_entry_set_text(GTK_ENTRY(entry2), "100");
     
     table=gtk_table_new(5,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 2, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 2, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), entry1, 0, 1, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), entry2, 0, 1, 3, 4);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 1, 2, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), label2, 1, 2, 3, 4);     

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int lower_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        int upper_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        int iRadioButton=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            iRadioButton=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            iRadioButton=2;
          }

        if(lower_bound>0&&upper_bound>0&&lower_bound<=upper_bound)
          {
            if(groups_database_validation(NULL)==0)
              {
                database_to_scatter_graph_sql(iRadioButton, lower_bound, upper_bound);
              }
          }
        else
          {
            printf("Check Upper and Lower Bounds\n");
            simple_message_dialog("Check Upper and Lower Bounds.");
          }

       }
     gtk_widget_destroy(dialog);
  }
static void database_to_error_graph_dialog(GtkWidget *menu , gpointer data)
  {
    GtkWidget *dialog, *table, *entry1, *entry2, *label1, *label2, *label3, *label4, *radio1, *radio2, *radio3, *radio4, *radio5, *radio6, *radio7, *content_area, *action_area;
    int result;
    
    g_print("Send Data from Database to Graph\n");

     dialog=gtk_dialog_new_with_buttons("Error Plot", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
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
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
     gtk_entry_set_text(GTK_ENTRY(entry1), "1");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
     gtk_entry_set_text(GTK_ENTRY(entry2), "10");
     
     table=gtk_table_new(12,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 2, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 2, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 2, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), radio3, 0, 2, 3, 4);
     gtk_table_attach_defaults(GTK_TABLE(table), radio4, 0, 2, 4, 5);
     gtk_table_attach_defaults(GTK_TABLE(table), label2, 0, 2, 5, 6);
     gtk_table_attach_defaults(GTK_TABLE(table), radio5, 0, 2, 6, 7);
     gtk_table_attach_defaults(GTK_TABLE(table), radio6, 0, 2, 7, 8);
     gtk_table_attach_defaults(GTK_TABLE(table), radio7, 0, 2, 8, 9);
     gtk_table_attach_defaults(GTK_TABLE(table), entry1, 0, 1, 9, 10);
     gtk_table_attach_defaults(GTK_TABLE(table), entry2, 0, 1, 10, 11);
     gtk_table_attach_defaults(GTK_TABLE(table), label3, 1, 2, 9, 10);
     gtk_table_attach_defaults(GTK_TABLE(table), label4, 1, 2, 10, 11);     

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int lower_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        int upper_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        int iRadioButton1=0;
        int iRadioButton2=0;
  
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            iRadioButton1=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            iRadioButton1=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            iRadioButton1=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            iRadioButton1=4;
          }

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio5)))
          {
            iRadioButton2=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio6)))
          {
            iRadioButton2=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio7)))
          {
            iRadioButton2=3;
          }
        
        if(lower_bound>0&&upper_bound>0&&lower_bound<=upper_bound)
          {
            if(groups_database_validation(NULL)==0)
              {
                database_to_error_graph_sql(iRadioButton1, iRadioButton2, lower_bound, upper_bound);
              }
          }
        else
          {
            printf("Check Upper and Lower Bounds\n");
            simple_message_dialog("Check Upper and Lower Bounds.");
          }
        
       }
     gtk_widget_destroy(dialog);
  }
static void database_to_box_graph_dialog(GtkWidget *menu , gpointer data)
  {
     GtkWidget *dialog, *table, *entry1, *entry2, *label0, *label1, *label2, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    int result;
    
    g_print("Send Data from Database to Graph\n");

     dialog=gtk_dialog_new_with_buttons("Box Plot", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
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
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
     gtk_entry_set_text(GTK_ENTRY(entry1), "1");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
     gtk_entry_set_text(GTK_ENTRY(entry2), "10");
     
     table=gtk_table_new(9,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label0, 0, 2, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 2, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 2, 3, 4);
     gtk_table_attach_defaults(GTK_TABLE(table), radio3, 0, 2, 4, 5);
     gtk_table_attach_defaults(GTK_TABLE(table), radio4, 0, 2, 5, 6);
     gtk_table_attach_defaults(GTK_TABLE(table), entry1, 0, 1, 6, 7);
     gtk_table_attach_defaults(GTK_TABLE(table), entry2, 0, 1, 7, 8);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 1, 2, 6, 7);
     gtk_table_attach_defaults(GTK_TABLE(table), label2, 1, 2, 7, 8);     

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int lower_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        int upper_bound=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        int iRadioButton=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            iRadioButton=1;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            iRadioButton=2;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio3)))
          {
            iRadioButton=3;
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio4)))
          {
            iRadioButton=4;
          }
        
        if(lower_bound>0&&upper_bound>0&&lower_bound<=upper_bound)
          {
            if(groups_database_validation(NULL)==0)
              {
                database_to_box_graph_sql(iRadioButton, lower_bound, upper_bound);
              }
          }
        else
          {
            printf("Check Upper and Lower Bounds\n");
            simple_message_dialog("Check Upper and Lower Bounds.");
          }
              
       }
    gtk_widget_destroy(dialog);
  }
static void about_dialog(GtkWidget *menu, GtkWidget *window)
  {
    GtkWidget *dialog, *content_area, *RaptorDrawing;
    const gchar *authors[]={"C. Eric Cashon", "Including the fine art.", "Check the references file\n for more author details.",NULL};

    dialog=gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Ordered Set VelociRaptor");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A mean number crunching machine");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2013 C. Eric Cashon");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);
    gtk_widget_set_size_request(dialog, 400,370);

    content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    gtk_widget_set_size_request(content_area, 400,300);
    //gtk_container_set_border_width(GTK_CONTAINER(content_area), 100);
    RaptorDrawing=gtk_drawing_area_new();
    gtk_widget_set_size_request(RaptorDrawing, 300,250);
    gtk_container_add(GTK_CONTAINER(content_area), RaptorDrawing);   
    g_signal_connect(G_OBJECT(RaptorDrawing), "draw", G_CALLBACK(draw_veloci_raptor), NULL); 

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static gboolean draw_veloci_raptor(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    cairo_pattern_t *pattern=NULL;
    int i=0;
    int move=330;
    int height=220;
    int width=250;
    double ScaleWidth=0;
    double ScaleHeight=0;
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
    cairo_set_source_rgba(cr, 0.0, 0.1, 0.8, 1.0);
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

    cairo_pattern_destroy(pattern);
    return TRUE;
  }
static gboolean draw_veloci_raptor_feet(GtkWidget *widget, cairo_t *cr, gpointer data)
 {   
  int i=0;
  int j=0;
  int width=0;
  int ScaleWidthCount=0;
  int FootCount=0;
  
  ScaleWidthCount=width-1024;

  if(ScaleWidthCount<=0)
    {
      FootCount=22;
    }
  if(ScaleWidthCount>0)
    {
      FootCount=22+ScaleWidthCount%50;
    } 

  int points[9][2] = { 
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

  int points2[9][2] = { 
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
  cairo_translate(cr, 0, 25);
  cairo_rotate(cr, G_PI/2);
  cairo_scale(cr, 0.15, 0.15);
  cairo_set_source_rgb(cr, 0, 1, 0);
 
  for(i=0;i<FootCount;i++)
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
    GFile *TextFile;
    GFileInputStream *FileStream=NULL;
    gssize length;
    GFileInfo *FileInfo;
    gint iFileSize = -1;
    GtkWidget *dialog;
    gchar *pTextBuffer=NULL;
    GString *TempBuffer=g_string_new(NULL);
    GArray *DataArray;
    gchar *pChar=NULL;
    guint32 counter=0;
    gint iTextPresent=0;
    gint iLineNumber=0;
    double dTemp=0;
    gboolean new_number=FALSE;

     if(iReferenceCountDialogWindow==0)
       {
         g_print("Import Text File\n");

         dialog=gtk_file_chooser_dialog_new("Open Text File",GTK_WINDOW(window), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);

         gtk_file_chooser_set_select_multiple(GTK_FILE_CHOOSER(dialog), FALSE);
         gint result=gtk_dialog_run(GTK_DIALOG(dialog));

         if(result==GTK_RESPONSE_ACCEPT)
            {
             TextFile=gtk_file_chooser_get_file(GTK_FILE_CHOOSER(dialog));
             FileStream=g_file_read(TextFile, NULL, NULL);
             FileInfo=g_file_input_stream_query_info (G_FILE_INPUT_STREAM(FileStream),G_FILE_ATTRIBUTE_STANDARD_SIZE,NULL, NULL);     
             iFileSize=g_file_info_get_size (FileInfo);
             g_print("Text Length = %d\n", iFileSize);
             g_object_unref (FileInfo);
             pTextBuffer=(char *) malloc(sizeof(char) * iFileSize);
             memset(pTextBuffer, 0, iFileSize);
             length=g_input_stream_read (G_INPUT_STREAM(FileStream), pTextBuffer, iFileSize, NULL, NULL);
             g_print("Length of Buffer = %i\n", length);
             DataArray=g_array_new(FALSE, FALSE, sizeof (double));
             pChar=pTextBuffer;
             gtk_widget_destroy(dialog);

             //Ignore last newline character.
                  while(counter<(length-1))
                     {
                         if(g_ascii_isdigit(*pChar)||*pChar =='.'||*pChar=='-')
                           {
                             g_string_append_printf(TempBuffer, "%c", *pChar);
                             new_number=TRUE;
                           }
                         else if(*pChar=='\n'||*pChar==' '|| *pChar==',' || *pChar=='|')
                           {
                             if(new_number==TRUE)
                               {
                                 dTemp=g_ascii_strtod(TempBuffer->str, NULL);
                                 g_array_append_val(DataArray, dTemp);
                                 g_string_truncate(TempBuffer, 0);
                                 new_number=FALSE;
                               }
                            }
                         else
                            {
                              iLineNumber=DataArray[0].len + 1;
                              g_print("Not a Number at Number%i\n", iLineNumber);
                              iTextPresent=1;
                              break;
                            }
                       pChar++;
                       counter++;
                     }
              g_free(pTextBuffer);
              g_print("Counter=%i and Length=%i\n", counter, length);

              if(iTextPresent==0)
                {
                  test_data_button_clicked(button, DataArray,0,0,0);
                  g_print("Imported Text File\n");
                }
              else
                {
                  g_print("Unsuccessful Import\n");
                  simple_message_dialog("Import unsuccessful! Check text file format\n and try again with the terminal window open\n to see what lines caused the errors.");
                }
              g_array_free(DataArray, TRUE);

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

    g_string_free(TempBuffer, TRUE);
       
  }
static void append_text_dialog(GtkWidget *menu, GtkWidget *window)
  {
    GtkWidget *dialog, *label, *entry, *grid, *content_area;
    gint result=0;

    dialog=gtk_dialog_new_with_buttons("Append Text Files", GTK_WINDOW(window), GTK_DIALOG_MODAL, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, "Select Text Files", GTK_RESPONSE_OK, NULL);
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
        show_file_dialog_append(widgets);
      }

    gtk_widget_destroy(dialog);

  }
static void show_file_dialog_append(GArray *widgets)
  {
    GtkWidget *dialog;
    
    dialog=gtk_file_chooser_dialog_new("Open Text File",GTK_WINDOW(g_array_index(widgets, GtkWidget*, 0)), GTK_FILE_CHOOSER_ACTION_OPEN, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT, NULL);
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
    int file_size=0;
    int i=0;
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

    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (data));
    gtk_text_buffer_get_bounds(buffer, &start1, &end1);
    gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);
  }
static void heatmap_dialog(GtkButton *button, gpointer data)
  {
    GtkWidget *dialog, *table, *label1, *radio1, *radio2, *radio3, *radio4, *content_area, *action_area;
    gint result;
   
     g_print("Heatmap Text\n");

     dialog=gtk_dialog_new_with_buttons("Heatmap Platemap", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("Heatmap Gradient");

     radio1=gtk_radio_button_new_with_label(NULL, "Heatmap(rgb)");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Heatmap(iris)");
     radio3=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Heatmap(iris2)");
     radio4=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Heatmap(sun)");
     
     table=gtk_table_new(5,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 1, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 1, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), radio3, 0, 1, 3, 4);
     gtk_table_attach_defaults(GTK_TABLE(table), radio4, 0, 1, 4, 5);
     
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
         double high=0;
         double low=0;
         double difference=0;
          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
            {
              format_text_platemap_heatmap_high_low(GTK_TEXT_VIEW(data), &high, &low);
              difference=abs(high-low)/10.0;
              if(difference!=0)
                {
                  printf("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
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
                  printf("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
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
                  printf("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
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
                  printf("Low %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f %.2f High\n", low, low+difference, low+2*difference, low+3*difference, low+4*difference, low+5*difference, low+6*difference, low+7*difference, low+8*difference, low+9*difference, high); 
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
    GtkWidget *dialog, *table, *entry1, *label1, *label2, *radio1, *radio2, *content_area, *action_area;
    gint result;
    guint32 iBufferCount;
    //char cArrayNumber[25];//if a change of space format it can segfault.
    GString *cArrayNumber=g_string_new("");
    guint32 iSpace=0;
    guint32 iPrevSpace=0;
    guint32 iTwoPrevSpace=0;
    guint32 i=0;
    guint32 j=0;
    gfloat previous=G_MAXFLOAT;//cludge, start with max value of float for comparison rise.
    gfloat previous2=G_MINFLOAT;//first value to compare with fall run.
    int iswitch=0;
    guint32 iBeginIter=0;
    guint32 iEndIter=0;
    int iSetSize=2;
    int iSize=5;
    char temp;
    GtkTextBuffer *buffer;
    GtkTextIter start1, end1, start_iter;
    GtkTextTagTable *TagTable;
    GtkTextTag *TagRiseFall;

     g_print("RiseFall Text\n");

     dialog=gtk_dialog_new_with_buttons("RiseFall Platemap", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     radio1=gtk_radio_button_new_with_label(NULL, "Rising Sets");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Declining Sets");

     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
     gtk_entry_set_text(GTK_ENTRY(entry1), "5");

     label1=gtk_label_new("Run Set Size >=");
     label2=gtk_label_new("Set Runs");
     
     table=gtk_table_new(4,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label2, 0, 2, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 1, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 1, 2, 3);
     gtk_table_attach(GTK_TABLE(table), label1, 0, 1, 3, 4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry1, 1, 2, 3, 4,GTK_SHRINK,GTK_SHRINK,0,0);
     
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
         //Get active widget value
         iSize=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
         
         buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW (data));
         TagTable=gtk_text_buffer_get_tag_table(buffer);
         TagRiseFall=gtk_text_tag_table_lookup(TagTable, "purple_foreground");

           if(TagRiseFall==NULL)
               {
                 gtk_text_buffer_create_tag (buffer, "purple_foreground",
                        "foreground", "Purple", NULL);  
               }
           else
               {
                 //remove tags.
                 gtk_text_buffer_get_bounds(buffer, &start1, &end1);
                 gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);
               }

           iBufferCount=gtk_text_buffer_get_char_count(buffer); 
           gtk_text_buffer_get_start_iter(buffer, &start_iter);

          //start of large block for radio1.
          if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
             {
              //Look for rise fall trends
               for(i=0; i<iBufferCount; i++)
                  {
                   temp=gtk_text_iter_get_char(&start_iter);
                   //ignore newlines
                   if(temp!='\n')
                     {
                       if(g_ascii_isdigit(temp)|| temp=='.')
                         {
                          //cArrayNumber[j]=gtk_text_iter_get_char(&start_iter);
                          g_string_insert_c(cArrayNumber, j, gtk_text_iter_get_char(&start_iter)); 
                          j++;
                         }
                       if(temp==' ')
                         {
                           iSpace=i;
                           //Two choices(>=,<=)
                           if(previous<atof(cArrayNumber->str)) //switch to less than falls run
                             {
                                   if(iswitch==0)
                                     {
                                       //printf("%f %f ",previous, atof(cArrayNumber->str));
                                       iBeginIter=iTwoPrevSpace+1;
                                       iswitch=1;
                                       iSetSize=2;
                                     }
                                   else
                                     {
                                       //printf("%f ", atof(cArrayNumber->str));
                                       //count sets of a specified size
                                       iSetSize++;
                                        if(i==iBufferCount-3&&iSetSize>=iSize) //ends with " \n\n\n"
                                            {
                                             //end off of last space
                                             iEndIter=iSpace;
                                             //printf("\nBegin %i End %i\n", iBeginIter, iEndIter);
                                             gtk_text_buffer_get_iter_at_offset (buffer, &start1, iBeginIter);
                                             gtk_text_buffer_get_iter_at_offset (buffer, &end1, iEndIter);
                                             gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                            }
                                     }
                             }
                           else
                             {
                                   if(iswitch==1)
                                     {
                                       //printf("\n");
                                       iEndIter=iPrevSpace;
                                       if(iSetSize>=iSize)
                                          {
                                            //printf("Begin %i End %i\n", iBeginIter, iEndIter);
                                            gtk_text_buffer_get_iter_at_offset (buffer, &start1, iBeginIter);
                                            gtk_text_buffer_get_iter_at_offset (buffer, &end1, iEndIter);
                                            gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                          }
                                       else
                                          {
                                            //printf("Iters Not Applied!\n");
                                          }
                                     }
                                   iswitch=0;
                             }
                           iTwoPrevSpace=iPrevSpace;
                           iPrevSpace=iSpace;
                           previous=atof(cArrayNumber->str);
                           //memset(cArrayNumber, 0, 25);
                           g_string_truncate(cArrayNumber,0);
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
               for(i=0; i<iBufferCount; i++)
                  {
                   temp=gtk_text_iter_get_char(&start_iter);
                   //ignore newlines
                   if(temp!='\n')
                     {
                       if(g_ascii_isdigit(temp)|| temp=='.')
                         {
                          //cArrayNumber[j]=gtk_text_iter_get_char(&start_iter);
                          g_string_insert_c(cArrayNumber, j, gtk_text_iter_get_char(&start_iter));
                          j++;
                         }
                       if(temp==' ')
                         {
                           iSpace=i;
                           //Two choices(>=,<=)
                           if(previous2>atof(cArrayNumber->str)) //falls run
                             {
                                   if(iswitch==0)
                                     {
                                       //printf("%f %f ",previous2, atof(cArrayNumber->str));
                                       iBeginIter=iTwoPrevSpace+1;
                                       iswitch=1;
                                       iSetSize=2;
                                     }
                                   else
                                     {
                                       //printf("%f ", atof(cArrayNumber->str));
                                       //count sets of a specified size
                                       iSetSize++;
                                        if(i==iBufferCount-3) //ends with " \n\n\n"
                                            {
                                             //end off of last space
                                             iEndIter=iSpace;
                                             //printf("\nBegin %i End %i\n", iBeginIter, iEndIter);
                                             gtk_text_buffer_get_iter_at_offset (buffer, &start1, iBeginIter);
                                             gtk_text_buffer_get_iter_at_offset (buffer, &end1, iEndIter);
                                             gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                            }
                                     }
                             }
                           else
                             {
                                   if(iswitch==1)
                                     {
                                       printf("\n");
                                       iEndIter=iPrevSpace;
                                       if(iSetSize>=iSize)
                                          {
                                            //printf("Begin %i End %i\n", iBeginIter, iEndIter);
                                            gtk_text_buffer_get_iter_at_offset (buffer, &start1, iBeginIter);
                                            gtk_text_buffer_get_iter_at_offset (buffer, &end1, iEndIter);
                                            gtk_text_buffer_apply_tag_by_name (buffer, "purple_foreground", &start1, &end1);
                                          }
                                       else
                                          {
                                            //printf("Iters Not Applied!\n");
                                          }
                                     }
                                   iswitch=0;
                             }
                           iTwoPrevSpace=iPrevSpace;
                           iPrevSpace=iSpace;
                           previous2=atof(cArrayNumber->str);
                          // memset(cArrayNumber, 0, 25);
                           g_string_truncate(cArrayNumber,0);
                           j=0;
                         }
                     }
                   gtk_text_iter_forward_chars(&start_iter, 1);
                 } //end of for loop 2.
             }//end of radio2.
          
         
        } //end of GTK_RESPONSE_OK

     g_string_free (cArrayNumber, TRUE);
     gtk_widget_destroy(dialog);
       
  }
static void heatmap_html_dialog(GtkButton *button, gpointer p)
  {
     GtkWidget *dialog, *table, *entry1, *entry2, *label1, *label2, *label3, *label4, *label5, *label6, *radio1, *radio2, *combo1, *combo2, *combo3, *content_area, *action_area;
    int result;
    
    g_print("Send Plate Data from Database to HTML\n");

     dialog=gtk_dialog_new_with_buttons("Heatmap Platemap HTML", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
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
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
     gtk_entry_set_text(GTK_ENTRY(entry1), "8");

     entry2=gtk_entry_new();
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
     
     table=gtk_table_new(8,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 2, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), radio1, 0, 2, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), radio2, 0, 2, 2, 3);
     gtk_table_attach(GTK_TABLE(table), entry1, 1, 2, 3, 4, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), entry2, 1, 2, 4, 5, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo1, 1, 2, 5, 6, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo2, 1, 2, 6, 7, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 3, 4, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0, 1, 4, 5, GTK_SHRINK,GTK_SHRINK,0,0);     
     gtk_table_attach(GTK_TABLE(table), label4, 0, 1, 5, 6, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label5, 0, 1, 6, 7, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label6, 0, 1, 7, 8, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo3, 1, 2, 7, 8, GTK_SHRINK,GTK_SHRINK,0,0);     

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int rows=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        int columns=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        int precision=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo1)));
        int font_size=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo2)));
        int gradient=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(combo3)));
        int iRadioButton=0;

        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
          {
            iRadioButton=1;
            heatmap_to_html_sql(iRadioButton, rows, columns+1, precision, font_size, gradient);
          }
        if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
          {
            iRadioButton=2;
            heatmap_to_html_sql(iRadioButton, rows, columns+1, precision, font_size, gradient);
          }

       }
     gtk_widget_destroy(dialog);
  
  }
static void html_table_dialog(GtkButton *button, gpointer p)
  {
    GtkWidget *dialog, *table, *textview, *scrolled_win, *label1, *label2, *label3, *label4, *label5, *label6, *label7, *expand1, *expand2, *expand3, *combo1, *combo2, *combo3, *combo4, *combo5, *content_area, *action_area;
    GtkTextBuffer *buffer1;
    int result;
    
    g_print("Send Tablular Data from Database to HTML\n");

     dialog=gtk_dialog_new_with_buttons("Tabular Data to HTML", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     textview=gtk_text_view_new();
     gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
     scrolled_win=gtk_scrolled_window_new(NULL, NULL); 
     gtk_container_add(GTK_CONTAINER(scrolled_win), textview);
     buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
     gtk_text_buffer_insert_at_cursor(buffer1, "SELECT * FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID AND T1.KeyID<11;", -1);

     label1=gtk_label_new("Output data in tabular format in\nHTML. File name table.html");
     label2=gtk_label_new("SQL Statement                ");      
     label3=gtk_label_new("Precision");
     label4=gtk_label_new("Font Size");
     label5=gtk_label_new("Font Color");
     label6=gtk_label_new("Field Background Color");
     label7=gtk_label_new("Data Background Color"); 
 
     expand1=gtk_label_new(" "); 
     expand2=gtk_label_new(" "); 
     expand3=gtk_label_new(" "); 
     
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
     
     table=gtk_table_new(10,6,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 5, 0, 1);
     gtk_table_attach(GTK_TABLE(table), label2, 0, 1, 1, 2, GTK_FILL,GTK_FILL,0,0);
     gtk_table_attach(GTK_TABLE(table), expand1, 0, 1, 2, 3, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), expand2, 0, 1, 3, 4, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), expand3, 0, 1, 4, 5, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), scrolled_win, 0, 6, 2, 5, GTK_FILL,GTK_FILL,0,0);    
     gtk_table_attach(GTK_TABLE(table), combo1, 1, 2, 5, 6, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo2, 1, 2, 6, 7, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo3, 1, 2, 7, 8, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo4, 1, 2, 8, 9, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), combo5, 1, 2, 9, 10, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0, 1, 5, 6, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label4, 0, 1, 6, 7, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label5, 0, 1, 7, 8, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label6, 0, 1, 8, 9, GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label7, 0, 1, 9, 10, GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int check=0;
        char database_name[]="VelociRaptorData.db";
        char html_file_name[]="table.html";
        GtkTextIter start1;
        GtkTextIter end1;

        int precision=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo1)));
        int font_size=atoi(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo2)));
        gchar *font_color=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo3));
        gchar *field_bg_color=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo4));
        gchar *bg_color=gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo5));
        gtk_text_buffer_get_bounds(buffer1, &start1, &end1);
        gchar *sql=gtk_text_buffer_get_text(buffer1, &start1, &end1, TRUE);
      
        check=check_sql_for_select(sql);
    
        if(check==0)
          {
            printf("%s\n", sql);
            parse_sql_field_names(html_file_name, database_name, sql, precision, font_size, bg_color, field_bg_color, font_color);
      }
        else
          {
            printf("Unable to parse SQL statement.\n");
            simple_message_dialog("Unable to parse SQL statement.");
          }
       
       }
     gtk_widget_destroy(dialog);
  
  }
static void send_text_to_database_dialog(GtkButton* button, gpointer textview)
  {
    GtkWidget *dialog, *table, *label1, *label2, *value_entry1, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Send Text To Database", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("Send the text data in the textview\n to the database.");
     label2=gtk_label_new("Table Name");
    
     value_entry1=gtk_entry_new();

     gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 12);

     gtk_entry_set_text(GTK_ENTRY(value_entry1), "AnalysisData");
 
     table=gtk_table_new(4,5,FALSE);
    
     gtk_table_attach(GTK_TABLE(table), label1, 0,4,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,2,3,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_attach(GTK_TABLE(table), value_entry1, 1,5,2,3,GTK_EXPAND,GTK_EXPAND,0,0);
     
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
          GtkTextBuffer *Sbuffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
          guint length=gtk_text_buffer_get_char_count(Sbuffer);
          const gchar *pTableName=gtk_entry_get_text(GTK_ENTRY(value_entry1));
          if(length>5)
            {
              send_text_to_database(pTableName, textview);
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
    GtkWidget *dialog, *table, *label1, *label2, *label3, *label4, *label5, *label6, *label7, *label8, *pick_entry1, *pick_entry2, *pick_entry3, *pick_entry4, *value_entry1, *value_entry2, *value_entry3, *value_entry4, *check_button1, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Build Auxiliary Table", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("Picks Location 1");
     label2=gtk_label_new("Picks Location 2");
     label3=gtk_label_new("Picks Location 3");
     label4=gtk_label_new("Picks Location 4");
     label5=gtk_label_new("Picks Value");
     label6=gtk_label_new("Location");
     label7=gtk_label_new("*Optional Picks");
     label8=gtk_label_new("Groups Field Built Automatically From Set Size For Stats. Press OK For Default Table.\nPicks Field Values Need To Be In Sequential Order Starting From 1.");

     pick_entry1=gtk_entry_new();
     pick_entry2=gtk_entry_new();
     pick_entry3=gtk_entry_new();
     pick_entry4=gtk_entry_new();
     value_entry1=gtk_entry_new();
     value_entry2=gtk_entry_new();
     value_entry3=gtk_entry_new();
     value_entry4=gtk_entry_new();

     check_button1=gtk_check_button_new_with_label("Append Picks Values to Existing Table");

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
 
     table=gtk_table_new(9,3,FALSE);

     gtk_table_attach(GTK_TABLE(table), label8, 0,3,0,1,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label7, 0,1,1,2,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), label6, 1,2,2,3,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label5, 2,3,2,3,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label1, 0,1,3,4,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,4,5,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,5,6,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label4, 0,1,6,7,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), pick_entry1, 1,2,3,4,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), pick_entry2, 1,2,4,5,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), pick_entry3, 1,2,5,6,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), pick_entry4, 1,2,6,7,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry1, 2,3,3,4,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry2, 2,3,4,5,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry3, 2,3,5,6,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry4, 2,3,6,7,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_attach(GTK_TABLE(table), check_button1, 0,2,7,8,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
           g_print("Build Auxiliary Table\n");
           //get from global variables
           int iPlateNumber=atoi(pPlateNumberText);
           int iPlateSize=atoi(pPlateSizeText);
           int iGroupSize=atoi(pPlateStatsText); 
           
           GString *buffer=g_string_new("");
           GArray *iPickGroupsArray1=g_array_new(FALSE, FALSE, sizeof (int));
           GArray *iPickGroupsArray2=g_array_new(FALSE, FALSE, sizeof (int));
           GArray *iPickGroupsArray3=g_array_new(FALSE, FALSE, sizeof (int));
           GArray *iPickGroupsArray4=g_array_new(FALSE, FALSE, sizeof (int));
           int iPickGroupsArrayLength1=0;
           int iPickGroupsArrayLength2=0;
           int iPickGroupsArrayLength3=0;
           int iPickGroupsArrayLength4=0;
           const gchar *pPickGroups1=gtk_entry_get_text(GTK_ENTRY(pick_entry1));
           const gchar *pPickGroups2=gtk_entry_get_text(GTK_ENTRY(pick_entry2));
           const gchar *pPickGroups3=gtk_entry_get_text(GTK_ENTRY(pick_entry3));
           const gchar *pPickGroups4=gtk_entry_get_text(GTK_ENTRY(pick_entry4));
           int iValue1=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry1)));
           int iValue2=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
           int iValue3=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry3)));
           int iValue4=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry4)));
           int iPickGroups1Length=strlen(pPickGroups1);
           int iPickGroups2Length=strlen(pPickGroups2);
           int iPickGroups3Length=strlen(pPickGroups3);
           int iPickGroups4Length=strlen(pPickGroups4);
           int itemp;

           int iArrayLength=iPlateSize*iPlateNumber;
           //int iGroups=(iPlateNumber*iPlateSize/iGroupSize);
           int iArrayPlate[iArrayLength];
           int iArrayWells[iArrayLength];
           int iArrayPicks[iArrayLength];
           int iArrayGroups[iArrayLength]; 
           int iCounter=0;
           int iLabel=1;
           int i=0;
           int j=0;
           int k=0;
           
       //Load pick entry values into int arrays.
           if(iPickGroups1Length>0)
              {
                for(i=0;i<iPickGroups1Length;i++)
                  {
                    if(pPickGroups1[i]!=','|| i==iPickGroups1Length-1)
                       {
                        g_string_append_printf(buffer,"%c", pPickGroups1[i]);
                       }
                    else
                       {
                        itemp=atoi(buffer->str);
                        itemp=itemp-1;
                        g_array_append_val(iPickGroupsArray1,itemp);
                        g_string_truncate(buffer,0);
                       }
                 }
                  itemp=atoi(buffer->str);
                  itemp=itemp-1;
                  g_array_append_val(iPickGroupsArray1,itemp);
                  iPickGroupsArrayLength1=iPickGroupsArray1[0].len;
                  g_string_truncate(buffer,0);
             }
       
          if(iPickGroups2Length>0)
              {
                for(i=0;i<iPickGroups2Length;i++)
                  {
                    if(pPickGroups2[i]!=','|| i==iPickGroups2Length-1)
                       {
                        g_string_append_printf(buffer,"%c", pPickGroups2[i]);
                       }
                    else
                       {
                        itemp=atoi(buffer->str);
                        itemp=itemp-1;
                        g_array_append_val(iPickGroupsArray2,itemp);
                        g_string_truncate(buffer,0);
                       }
                 }
                  itemp=atoi(buffer->str);
                  itemp=itemp-1;
                  g_array_append_val(iPickGroupsArray2,itemp);
                  iPickGroupsArrayLength2=iPickGroupsArray2[0].len;
                  g_string_truncate(buffer,0);
             }

             if(iPickGroups3Length>0)
              {
                for(i=0;i<iPickGroups3Length;i++)
                  {
                    if(pPickGroups3[i]!=','|| i==iPickGroups3Length-1)
                       {
                        g_string_append_printf(buffer,"%c", pPickGroups3[i]);
                       }
                    else
                       {
                        itemp=atoi(buffer->str);
                        itemp=itemp-1;
                        g_array_append_val(iPickGroupsArray3,itemp);
                        g_string_truncate(buffer,0);
                       }
                 }
                  itemp=atoi(buffer->str);
                  itemp=itemp-1;
                  g_array_append_val(iPickGroupsArray3,itemp);
                  iPickGroupsArrayLength3=iPickGroupsArray3[0].len;
                  g_string_truncate(buffer,0);
             }

             if(iPickGroups4Length>0)
              {
                for(i=0;i<iPickGroups4Length;i++)
                  {
                    if(pPickGroups4[i]!=','|| i==iPickGroups4Length-1)
                       {
                        g_string_append_printf(buffer,"%c", pPickGroups4[i]);
                       }
                    else
                       {
                        itemp=atoi(buffer->str);
                        itemp=itemp-1;
                        g_array_append_val(iPickGroupsArray4,itemp);
                        g_string_truncate(buffer,0);
                       }
                 }
                  itemp=atoi(buffer->str);
                  itemp=itemp-1;
                  g_array_append_val(iPickGroupsArray4,itemp);
                  iPickGroupsArrayLength4=iPickGroupsArray4[0].len;
                  g_string_truncate(buffer,0);
             }

             //Label Plate Numbers.
             for(i=0;i<iPlateNumber;i++)
                {
                  for(j=0;j<iPlateSize;j++)
                     {
                       iArrayPlate[iCounter]=i+1;
                       iCounter++;
                     }
                }
             iCounter=0;

             //Label wells in plate.
             for(i=0;i<iPlateNumber;i++)
                {
                  for(j=0;j<iPlateSize;j++)
                     {
                       iArrayWells[iCounter]=j+1;
                       iCounter++;
                     }
                }
             iCounter=0;
    
             //Zero pick array.
             for(i=0;i<(iPlateNumber*iPlateSize);i++)
                {
                  iArrayPicks[i]=0;
                }

             //Label selected groups in plate based on user picks.
             if(iPickGroupsArrayLength1>0)
               {
                 for(i=0;i<iPlateNumber;i++)
                    {
                     k=0;
                      for(j=0;j<iPlateSize;j++)
                         {
                            if(k<iPickGroupsArrayLength1 && j==g_array_index(iPickGroupsArray1, gint, k))
                              {
                               iArrayPicks[iCounter]=iValue1;
                               k++;
                               iCounter++;
                              }
                            else
                              {
                               iCounter++;
                              }
                        }
                   }
                 iCounter=0; 
               }
               
             if(iPickGroupsArrayLength2>0)
               {
                 for(i=0;i<iPlateNumber;i++)
                    {
                     k=0;
                      for(j=0;j<iPlateSize;j++)
                         {
                            if(k<iPickGroupsArrayLength2 && j==g_array_index(iPickGroupsArray2, gint, k))
                              {
                               iArrayPicks[iCounter]=iValue2;
                               k++;
                               iCounter++;
                              }
                            else
                              {
                               //iArrayPicks[iCounter]=0;
                               iCounter++;
                              }
                         }
                    }
                 iCounter=0;
                }

              if(iPickGroupsArrayLength3>0)
                {
                 for(i=0;i<iPlateNumber;i++)
                    {
                     k=0;
                      for(j=0;j<iPlateSize;j++)
                         {
                            if(k<iPickGroupsArrayLength3 && j==g_array_index(iPickGroupsArray3, gint, k))
                              {
                               iArrayPicks[iCounter]=iValue3;
                               k++;
                               iCounter++;
                              }
                            else
                              {
                               //iArrayPicks[iCounter]=0;
                               iCounter++;
                              }
                         }
                    }
                  iCounter=0;
                 }

             if(iPickGroupsArrayLength4>0)
                {
                 for(i=0;i<iPlateNumber;i++)
                    {
                     k=0;
                      for(j=0;j<iPlateSize;j++)
                         {
                            if(k<iPickGroupsArrayLength4 && j==g_array_index(iPickGroupsArray4, gint, k))
                              {
                               iArrayPicks[iCounter]=iValue4;
                               k++;
                               iCounter++;
                              }
                            else
                              {
                           //iArrayPicks[iCounter]=0;
                               iCounter++;
                              }
                        }
                   }
                 iCounter=0;
                }

             //Label groups based on set size for stats.
             //for(i=0;i<iGroups;i++)
                //{
                 // for(j=0;j<iGroupSize;j++)
                     //{
                       //iArrayGroups[iCounter]=i+1;
                       //iCounter++;
                     //}
                //}

             //Label Groups like Picks
             for(i=0;i<iPlateNumber;i++)
                {
                  for(j=0;j<iPlateSize/iGroupSize;j++)  
                     {
                       for(k=0;k<iGroupSize;k++)
                          {
                           iArrayGroups[iCounter]=iLabel;
                           iCounter++;
                          }
                       iLabel++;
                     }
                  iLabel=1; 
                }

             iCounter=0;


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
                 for(i=0;i<iArrayLength;i++)
                    {
                      if(iArrayPicks[i]>0)
                        {
                         g_print("UPDATE Aux SET Picks=%i WHERE KeyID = %i;\n", iArrayPicks[i], i+1);
                         sqlite3_bind_int(stmt1, 1, iArrayPicks[i]);
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
                 for(i=0;i<iArrayLength;i++)
                    {
                      printf("INSERT INTO Aux VALUES(%i,%i,%i,%i,%i)\n", i+1, iArrayPlate[i], iArrayWells[i], iArrayPicks[i], iArrayGroups[i]);
                      sqlite3_bind_int(stmt2, 1, i+1);
                      sqlite3_bind_int(stmt2, 2, iArrayPlate[i]); 
                      sqlite3_bind_int(stmt2, 3, iArrayWells[i]); 
                      sqlite3_bind_int(stmt2, 4, iArrayPicks[i]); 
                      sqlite3_bind_int(stmt2, 5, iArrayGroups[i]); 
                      sqlite3_step(stmt2);
                      sqlite3_reset(stmt2); 
                    }
                 sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
                 sqlite3_finalize(stmt2);
                 sqlite3_close(handle);
                 g_print("Auxiliary Table Built\n");
              }
          g_string_free(buffer, TRUE);
          g_array_free(iPickGroupsArray1, TRUE);
          g_array_free(iPickGroupsArray2, TRUE);
          g_array_free(iPickGroupsArray3, TRUE);
          g_array_free(iPickGroupsArray4, TRUE);
        }
     
     gtk_widget_destroy(dialog);
  }
static void build_combo_table_dialog(GtkWidget *menu, GtkWidget *window)
  {
     //Build a combinations SQL script, save it to a file and send it off to the database.
     GtkWidget *dialog, *table, *label1, *label2, *label3, *value_entry1, *value_entry2, *value_entry3, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Build Combinations Table", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("SQL Table Name");
     label2=gtk_label_new("Combination Set Size");
     label3=gtk_label_new("Combinations Sub Set");
    
     value_entry1=gtk_entry_new();
     value_entry2=gtk_entry_new();
     value_entry3=gtk_entry_new();

     gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 12);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 2);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry3), 2);

     gtk_entry_set_text(GTK_ENTRY(value_entry1), "combinations");
     gtk_entry_set_text(GTK_ENTRY(value_entry2), "7");
     gtk_entry_set_text(GTK_ENTRY(value_entry3), "5");
 
     table=gtk_table_new(5,3,FALSE);
    
     gtk_table_attach(GTK_TABLE(table), label1, 0,1,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,2,3,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,3,4,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_attach(GTK_TABLE(table), value_entry1, 1,3,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry2, 1,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry3, 1,2,3,4,GTK_SHRINK,GTK_SHRINK,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
         g_print("Build Combinations Table\n");
         int ComboSet=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
         int ComboSubSet=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry3)));
         const gchar *TableName=gtk_entry_get_text(GTK_ENTRY(value_entry1));

         //cap at 15
         if(ComboSet>=2&&ComboSet<=15)
           {
             if(ComboSubSet>=2&&ComboSubSet<=ComboSet)
               {
                 build_combo_table_sql(ComboSet, ComboSubSet, TableName);
               }
             else
               {
                 printf("The combination subset range is 2<=x<=ComboSet\n");
                 simple_message_dialog("The combination subset range is 2<=x<=ComboSet");
               }
           }
         else
           {
             printf("The combination set range is 2<=x<=15\n");
             simple_message_dialog("The combination set range is 2<=x<=15");
           }       
     }
    printf("Combinations Finished\n");
    gtk_widget_destroy(dialog);
  }
static void build_permutation_table_dialog(GtkWidget *menu, GtkWidget *window)
  {
    GtkWidget *dialog, *table, *label1, *label2, *value_entry1, *value_entry2, *content_area, *action_area;
    int result;

    dialog=gtk_dialog_new_with_buttons("Build Permutations Table", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("SQL Table Name");
     label2=gtk_label_new("Permutation Set Size");
    
     value_entry1=gtk_entry_new();
     value_entry2=gtk_entry_new();

     gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 12);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 2);

     gtk_entry_set_text(GTK_ENTRY(value_entry1), "permutations");
     gtk_entry_set_text(GTK_ENTRY(value_entry2), "5");
 
     table=gtk_table_new(4,5,FALSE);
    
     gtk_table_attach(GTK_TABLE(table), label1, 0,1,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,2,3,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_attach(GTK_TABLE(table), value_entry1, 1,5,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry2, 1,2,2,3,GTK_SHRINK,GTK_SHRINK,0,0);
     
     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
         g_print("Build Permutations Table\n");
         int ComboSet=atoi(gtk_entry_get_text(GTK_ENTRY(value_entry2)));
         const gchar *TableName=gtk_entry_get_text(GTK_ENTRY(value_entry1));

         //Cap at 9
         if(ComboSet>=2&&ComboSet<=9)
           {
            build_permutation_table_sql(ComboSet, TableName);
           }
         else
           {
            printf("The permutation set range of is 2<=x<=9.\n");
            simple_message_dialog("The permutation set range is 2<=x<=9.\n");
           }

         
         }
      printf("Permutations Finished\n");
      gtk_widget_destroy(dialog);
  }
static void format_text_dialog(GtkButton *button, gpointer data)
  {
    GtkWidget *dialog, *table, *entry1, *entry2, *entry3, *entry4, *label1, *label2, *label3, *label4, *content_area, *action_area;
    gint result;
    
     g_print("Format Text\n");

     dialog=gtk_dialog_new_with_buttons("Format Platemap", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
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

     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 8);
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 8);
     gtk_entry_set_width_chars(GTK_ENTRY(entry3), 8);
     gtk_entry_set_width_chars(GTK_ENTRY(entry4), 8);

     gtk_entry_set_text(GTK_ENTRY(entry1), "800.0");
     gtk_entry_set_text(GTK_ENTRY(entry2), "200.0");
     gtk_entry_set_text(GTK_ENTRY(entry3), "400.0");
     gtk_entry_set_text(GTK_ENTRY(entry4), "600.0");

     table=gtk_table_new(5,2,FALSE);
     gtk_table_attach_defaults(GTK_TABLE(table), label1, 0, 1, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), label2, 0, 1, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), label3, 0, 1, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), label4, 0, 1, 3, 4);
     gtk_table_attach_defaults(GTK_TABLE(table), entry1, 1, 2, 0, 1);
     gtk_table_attach_defaults(GTK_TABLE(table), entry2, 1, 2, 1, 2);
     gtk_table_attach_defaults(GTK_TABLE(table), entry3, 1, 2, 2, 3);
     gtk_table_attach_defaults(GTK_TABLE(table), entry4, 1, 2, 3, 4);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 20);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
        {
         double dEntry1=atof(gtk_entry_get_text(GTK_ENTRY(entry1)));
         double dEntry2=atof(gtk_entry_get_text(GTK_ENTRY(entry2)));
         double dEntry3=atof(gtk_entry_get_text(GTK_ENTRY(entry3)));
         double dEntry4=atof(gtk_entry_get_text(GTK_ENTRY(entry4)));
 
         format_text_platemap(dEntry1, dEntry2, dEntry3, dEntry4, GTK_TEXT_VIEW(data));
         
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
    GtkWidget *dialog, *table, *label1, *label2, *value_entry1, *value_entry2, *content_area, *action_area;
    int result;
    int rows=0;
    int columns=0;

    dialog=gtk_dialog_new_with_buttons("Plate Map", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("Rows");
     label2=gtk_label_new("      Columns");
    
     value_entry1=gtk_entry_new();
     value_entry2=gtk_entry_new();

     gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 3);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 3);

     gtk_entry_set_text(GTK_ENTRY(value_entry1), "8");
     gtk_entry_set_text(GTK_ENTRY(value_entry2), "12");
 
     table=gtk_table_new(4,3,FALSE);
    
     gtk_table_attach(GTK_TABLE(table), label1, 0,1,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,2,3,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_attach(GTK_TABLE(table), value_entry1, 1,2,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry2, 1,2,2,3,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
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
    GtkWidget *dialog, *table, *label1, *label2, *label3, *value_entry1, *value_entry2, *value_entry3, *content_area, *action_area;
    int result;
    int rows=0;
    int columns=0;
    int digits=0;

    dialog=gtk_dialog_new_with_buttons("Plate Map", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 20);

     label1=gtk_label_new("Rows");
     label2=gtk_label_new("Columns");
     label3=gtk_label_new("Truncate Digits");
    
     value_entry1=gtk_entry_new();
     value_entry2=gtk_entry_new();
     value_entry3=gtk_entry_new();

     gtk_entry_set_width_chars(GTK_ENTRY(value_entry1), 3);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry2), 3);
     gtk_entry_set_width_chars(GTK_ENTRY(value_entry3), 3);

     gtk_entry_set_text(GTK_ENTRY(value_entry1), "8");
     gtk_entry_set_text(GTK_ENTRY(value_entry2), "12");
     gtk_entry_set_text(GTK_ENTRY(value_entry3), "7");
 
     table=gtk_table_new(5,3,FALSE);
    
     gtk_table_attach(GTK_TABLE(table), label1, 0,1,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label2, 0,1,2,3,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), label3, 0,1,3,4,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_attach(GTK_TABLE(table), value_entry1, 1,2,1,2,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry2, 1,2,2,3,GTK_EXPAND,GTK_EXPAND,0,0);
     gtk_table_attach(GTK_TABLE(table), value_entry3, 1,2,3,4,GTK_EXPAND,GTK_EXPAND,0,0);

     gtk_table_set_row_spacings(GTK_TABLE(table), 10);
     gtk_table_set_col_spacings(GTK_TABLE(table), 10);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_add(GTK_CONTAINER(content_area), table); 
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
static void copy_plates_to_clipboard(GtkWidget *copy, GtkWidget *treeview, int iRows, int iColumns, int digits)
  {
    //Copy one dimensional data to two dimensions or plate well format.
    int iPlateSize=atoi(pPlateSizeText);
    int iNumberOfPlates=atoi(pPlateNumberText);
    //int iSetSizeForStatistics=atoi(pPlateStatsText);
    const gchar *title;
    GtkTreeIter iter;
    GtkTreeModel *model;
    guint32 iModelRows=0;
    double dTemp=0;

    g_print("Rows %i Columns %i PlateSize %i\n", iRows, iColumns, iPlateSize);
    if(iRows*iColumns!=iPlateSize)
     {
       simple_message_dialog("The Rows in Plate times the Columns in Plate need\n to equal the Size of Plate!");
     }
    else
     {
       model=gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
       gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");

         do
            {
              iModelRows++;
            }
         while(gtk_tree_model_iter_next(model,&iter));
         gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");
         double dDataArray[iModelRows];
         iModelRows=0;
         do
            {
              gtk_tree_model_get(model, &iter, 1, &dTemp, -1);
              dDataArray[iModelRows]=dTemp;
              iModelRows++;
            }
         while(gtk_tree_model_iter_next(model,&iter));

         title=(gtk_menu_item_get_label(GTK_MENU_ITEM(copy)));
         g_print("Menu is %s\n", title);

         if(iModelRows==(iNumberOfPlates*iRows*iColumns))
           {
            if(g_strcmp0(title, "Copy Plates to Clipboard(int)")==0)
               {
                 PlateMapInt(dDataArray, iNumberOfPlates, iPlateSize, iRows, iColumns);  
               }
            else if(g_strcmp0(title, "Copy Plates to Clipboard(float)")==0)
               {
                 PlateMapDouble(dDataArray, iNumberOfPlates, iPlateSize, iRows, iColumns);
               }
            else
               {
                 PlateMapDoubleTruncate(dDataArray, iNumberOfPlates, iPlateSize, iRows, iColumns, digits);  
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
static void test_data_button_clicked (GtkButton *button, gpointer data, int seed_value, double param, int iRadioButton)
{
   GtkWidget *dialog, *content_area, *treeview, *action_area, *label1, *NextButton, *scrolled_win, *menu, *copyplates1, *copyplates2, *copyplates3, *copyappend;
   GtkTreeSelection *selection;
   GtkAccelGroup *group = NULL;
   int iArrayCount;
   int iArrayNotVoid=0;
   double dTemp;
   GArray *DataArray;
   int check=0;

    //Simple validation checks.
    if(data==NULL)
      {
        int iPlateSize=atoi(pPlateSizeText);
        int iNumberOfPlates=atoi(pPlateNumberText);
        int iSetSizeForStatistics=atoi(pPlateStatsText);
        printf("Plates %i, PlateSize %i, Stats %i\n",iNumberOfPlates,iPlateSize,iSetSizeForStatistics);
        if(iPlateSize<1||iNumberOfPlates<1||iSetSizeForStatistics<1)
          {
            check=1;
          }
        if(iSetSizeForStatistics>0)
          {
            if(iPlateSize%iSetSizeForStatistics!=0)
              {
                check=1;
              }
          }
        if(iPlateSize<2*iSetSizeForStatistics)
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
           DataArray=(GArray*)data;
           iArrayCount=DataArray[0].len;
           dTemp=g_array_index(DataArray, double, 0);
           g_print("Check Array Length %i First Value %f\n", iArrayCount, dTemp);
           iArrayNotVoid=1;
          }

       dialog=gtk_dialog_new();

       gtk_window_set_title(GTK_WINDOW(dialog), "Plate Data");

       content_area=gtk_dialog_get_content_area (GTK_DIALOG(dialog));
       action_area=gtk_dialog_get_action_area (GTK_DIALOG(dialog)); 

       label1=gtk_label_new("");
       gtk_container_add (GTK_CONTAINER (action_area), label1);
       NextButton=gtk_button_new_with_label("  Calculate Percent>  ");
       gtk_container_add (GTK_CONTAINER (action_area), NextButton);
   
       treeview=gtk_tree_view_new();
       g_signal_connect(G_OBJECT(NextButton), "clicked", G_CALLBACK(next_button_clicked), (gpointer) treeview);

       if(iArrayNotVoid==0)
         {
           setup_tree_view_data(GTK_TREE_VIEW(treeview), seed_value, param, iRadioButton);
         }
       else
         {
           setup_tree_view_text(GTK_TREE_VIEW(treeview), DataArray);
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
        g_signal_connect(GTK_WIDGET(dialog), "state_flags_changed", G_CALLBACK(activate_treeview_data_event),NULL);

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
   GtkWidget *dialog2, *content_area2, *treeview2, *action_area2, *label1, *NextButton2, *scrolled_win2, *copyplates2, *copyplatesd2, *copyplatest2, *copyappend2, *menu2;
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
       dialog2 = gtk_dialog_new();
       gtk_window_set_title(GTK_WINDOW(dialog2), "Percent");

       content_area2 = gtk_dialog_get_content_area (GTK_DIALOG (dialog2));
       action_area2 = gtk_dialog_get_action_area (GTK_DIALOG (dialog2));
      
       label1=gtk_label_new("");
       gtk_container_add (GTK_CONTAINER (action_area2), label1);
       NextButton2=gtk_button_new_with_label("Build Auxiliary Table>");
       gtk_container_add (GTK_CONTAINER (action_area2), NextButton2);
       g_signal_connect(G_OBJECT(NextButton2), "clicked", G_CALLBACK(build_aux_table_dialog), NULL);
            
       treeview2=gtk_tree_view_new();

        //setup tree view percent
       setup_tree_view_percent(GTK_TREE_VIEW(treeview2),GTK_TREE_VIEW(treeview));

       scrolled_win2=gtk_scrolled_window_new(NULL, NULL);
       gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scrolled_win2), GTK_POLICY_ALWAYS, GTK_POLICY_AUTOMATIC);
  
       selection2=gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview2));
       gtk_tree_selection_set_mode(selection2, GTK_SELECTION_MULTIPLE);

       group2 = gtk_accel_group_new();
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
       g_signal_connect(GTK_WIDGET(dialog2), "state_flags_changed", G_CALLBACK(activate_treeview_percent_event),NULL);
       
       gtk_widget_show_all(dialog2);

       gtk_window_move(GTK_WINDOW(dialog2), gdk_screen_width()/2 +15, gdk_screen_height()/2-15);
       pWindowTitle=gtk_window_get_title(GTK_WINDOW(dialog2));
    }
 }

void setup_tree_view_text(GtkTreeView *treeview2, GArray *DataArray)
     {
       //Create a TreeView for the imported text file.
       GtkListStore *store;
       GtkTreeIter iter;
       GtkCellRenderer *renderer;
       GtkTreeViewColumn *column;
       //GtkTreeModel *model;
       int iArrayCount;
       guint32 i=0;
       gchar *pRecords=NULL;

       g_print("Set Up Treeview from Text File or SQLite...\n");

       store=gtk_list_store_new(2, G_TYPE_INT, G_TYPE_DOUBLE);
       iArrayCount=DataArray[0].len;
       //Save count array count to global variable.
       iTextArrayCount=DataArray[0].len;
       pRecords=g_strdup_printf("Data (%i records)", iArrayCount); 

            for(i=0; i<(iArrayCount); i++)
                {
                  gtk_list_store_append(store, &iter);
                  gtk_list_store_set(store, &iter, 0 , i+1, 1, g_array_index(DataArray, double, i), -1);
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
       column=gtk_tree_view_column_new_with_attributes(pRecords, renderer, "text", 1, NULL);
       gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);
       g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_edited), (gpointer) treeview2);
       
       g_free(pRecords);
       g_print("TreeView Built\n");
      /*
       After text treeview built check the arguments to make sure they apply. Check in next button.
      */
     }

void setup_tree_view_data(GtkTreeView *treeview, int seed_value, double param, int iRadioButton)
     {
        //Set up the tree model and renderers for the TreeView and call data functions.
        guint32 i=0;
        GtkListStore *store;
        GtkTreeIter iter;
        GtkCellRenderer *renderer;
        GtkTreeViewColumn *column;  
        guint32 iModelRows=0;
        gchar *pRecords=NULL;
        double *dDataArray=NULL;
      
        //Try to handle input errors before function call and atoi.
        int iPlateSize=atoi(pPlateSizeText);
        int iNumberOfPlates=atoi(pPlateNumberText);
        int iSetSizeForStatistics=atoi(pPlateStatsText);

        g_print("Allocate Arrays on Free Store\n");
        dDataArray = (double*)malloc((iPlateSize*iNumberOfPlates) * sizeof(double));
        if(dDataArray==NULL)
          {
            g_print("Array Allocation Failed!\n");
            simple_message_dialog("Couldn't allocate memory for the treeview! Reduce\n the number of records in the dataset.");
          }
        iRandomDataArrayCount=iPlateSize*iNumberOfPlates;
           
        g_print("Arrays Allocated\n");
        g_print("Setting Up Treeview...\n");      
        
        if(iRadioButton==1)
          {
           double gsl_ran_gaussian();
           GenerateRandomValues(dDataArray, iNumberOfPlates, iPlateSize, iSetSizeForStatistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, gsl_ran_gaussian);
          }
        if(iRadioButton==2)
          {
           double gsl_ran_chisq();
           GenerateRandomValues(dDataArray, iNumberOfPlates, iPlateSize, iSetSizeForStatistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, gsl_ran_chisq);
          }
        if(iRadioButton==3)
          {
           double gsl_ran_rayleigh();
           GenerateRandomValues(dDataArray, iNumberOfPlates, iPlateSize, iSetSizeForStatistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, gsl_ran_rayleigh);
          }
        if(iRadioButton==4)
          {
           double wrap_gsl_rng_uniform();
           GenerateRandomValues(dDataArray, iNumberOfPlates, iPlateSize, iSetSizeForStatistics, pPlatePosControlText, pPlateNegControlText, seed_value, param, wrap_gsl_rng_uniform);
          }

        iModelRows=iPlateSize*iNumberOfPlates;
 
        g_print("Model Rows Count %i\n", iModelRows);
        pRecords=g_strdup_printf("Data (%i records)", iModelRows); 

        store=gtk_list_store_new(2, G_TYPE_INT, G_TYPE_DOUBLE);

        for(i=0; i<(iPlateSize*iNumberOfPlates); i++)
           {
             gtk_list_store_append(store, &iter);
             gtk_list_store_set(store, &iter, 0 , i+1, 1, dDataArray[i], -1);
           }
              
        gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(store));
        g_object_unref(store);

        renderer=gtk_cell_renderer_text_new();
        column=gtk_tree_view_column_new_with_attributes("Counter", renderer, "text", 0, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);

        renderer=gtk_cell_renderer_text_new();
        g_object_set(renderer, "editable", TRUE, "editable-set", TRUE, NULL);
        column=gtk_tree_view_column_new_with_attributes(pRecords, renderer, "text", 1, NULL);
        gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column);
        g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_edited), (gpointer) treeview);
          
        g_free(pRecords);
        if(dDataArray!=NULL)
           {
            free(dDataArray);
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
        double dTemp=0;   
        guint32 iModelRows=0;
        gchar *pRecords=NULL;
        double *dDataArray=NULL;
        double *dPercentArray=NULL;
      
        //Try to handle input errors before function call and atoi.
        int iPlateSize=atoi(pPlateSizeText);
        int iNumberOfPlates=atoi(pPlateNumberText);
        int iSetSizeForStatistics=atoi(pPlateStatsText);
        
        g_print("Allocate Arrays\n");
        dDataArray = (double*)malloc((iPlateSize*iNumberOfPlates) * sizeof(double));
        dPercentArray = (double*)malloc((iPlateSize*iNumberOfPlates) * sizeof(double));

        if(dDataArray==NULL||dPercentArray==NULL)
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
           gtk_tree_model_get(model, &iter, 1, &dTemp, -1);
           dDataArray[iModelRows]=dTemp;
           iModelRows++;
          }
        while(gtk_tree_model_iter_next(model,&iter));

        CalculatePercentControl(dDataArray, dPercentArray, iPlateSize, iNumberOfPlates, iSetSizeForStatistics, pPlatePosControlText, pPlateNegControlText);
              
         g_print("Model Rows Count %i\n", iModelRows);
         pRecords=g_strdup_printf("Data (%i records)", iModelRows); 

         store=gtk_list_store_new(2, G_TYPE_INT, G_TYPE_DOUBLE);

         for(i=0; i<(iPlateSize*iNumberOfPlates); i++)
            {
              gtk_list_store_append(store, &iter);
              gtk_list_store_set(store, &iter, 0 , i+1, 1, dPercentArray[i], -1);
             }
              
          gtk_tree_view_set_model(GTK_TREE_VIEW(treeview2), GTK_TREE_MODEL(store));
          g_object_unref(store);

          renderer=gtk_cell_renderer_text_new();
          column=gtk_tree_view_column_new_with_attributes("Counter", renderer, "text", 0, NULL);
          gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);

          renderer=gtk_cell_renderer_text_new();
          g_object_set(renderer, "editable", TRUE, "editable-set", TRUE, NULL);
          column=gtk_tree_view_column_new_with_attributes(pRecords, renderer, "text", 1, NULL);
          gtk_tree_view_append_column(GTK_TREE_VIEW(treeview2), column);
          g_signal_connect(G_OBJECT(renderer), "edited", G_CALLBACK(cell_edited), (gpointer) treeview2);
          
        g_free(pRecords);
        if(dDataArray!=NULL)
           {
            free(dDataArray);
           }
        if(dPercentArray!=NULL)
           {
            free(dPercentArray);
           }
        
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
    GArray *DataArray=g_array_new(FALSE, FALSE, sizeof(double));
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
        g_array_append_val(DataArray, temp);
        sql_return=sqlite3_step(stmt1);
        i++;
      }
    
    sqlite3_finalize(stmt1);   
    sqlite3_close(cnn);
 
    g_print("SQLite Check Array Length %i First %f Last %f ZeroWarning %i\n", i, g_array_index(DataArray, double, 0), g_array_index(DataArray, double, i-1), zero_counter);

    if(i>0)
      {
        test_data_button_clicked(NULL, DataArray,0,0,0);
      }
    else
      {
        g_print("Couldn't fill array with values.\n");
        simple_message_dialog("Couldn't fill array with values.");
      }

    g_array_free(DataArray, TRUE);
    sqlite3_free(sql1);
    if(database!=NULL) g_free(database);
  }






