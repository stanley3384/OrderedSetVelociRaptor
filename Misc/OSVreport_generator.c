
/*

    Re-write the report_generator.py program in C. A ways to go yet. Just some of the UI has been copied
over so far along with a few functions. Work in progress.
 
    gcc -Wall OSVreport_generator.c -o OSVreport_generator `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<stdlib.h>

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
    else return 0;
  }
static void save_json_file(gchar *string, GtkWidget *ws[])
  {
    gint ret_val=validate_entries(ws);
    g_print("Return %i\n", ret_val);
  }
static void save_report(GtkWidget *widget, GtkWidget *ws[])
  {
    GtkWidget *dialog=gtk_file_chooser_dialog_new("Save Report", GTK_WINDOW(ws[21]), GTK_FILE_CHOOSER_ACTION_SAVE, "Cancel", GTK_RESPONSE_CANCEL, "Save", GTK_RESPONSE_ACCEPT, NULL);
    //gtk_widget_show_all(dialog);
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

    GtkWidget *button6=gtk_button_new_with_label("Set Table Labels");
    gtk_widget_set_sensitive(button6, FALSE);
    
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

    GtkWidget *ws[]={entry1, entry2, entry3, entry4, entry5, entry6, entry7, entry8, entry9, entry10, entry11, combo1, combo2, combo3, combo4, combo5, combo6, combo7, check1, check2, textview1, window};
    g_signal_connect(menu1item2, "activate", G_CALLBACK(save_report), ws);
 
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
    
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_size_request(da1, 10000, 10000);

    GtkWidget *scroll2=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scroll2, TRUE);
    gtk_widget_set_vexpand(scroll2, TRUE);

    GtkWidget *layout=gtk_layout_new(NULL, NULL);
    gtk_layout_set_size(GTK_LAYOUT(layout), 10000, 10000);
    gtk_widget_set_hexpand(layout, TRUE);
    gtk_widget_set_vexpand(layout, TRUE);
    gtk_layout_put(GTK_LAYOUT(layout), da1, 0, 0);
    gtk_container_add(GTK_CONTAINER(scroll2), layout);

    GtkWidget *notebook=gtk_notebook_new();
    GtkWidget *nb_label1=gtk_label_new("Setup");
    GtkWidget *nb_label2=gtk_label_new("Drawing");
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid, nb_label1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), scroll2, nb_label2);

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
