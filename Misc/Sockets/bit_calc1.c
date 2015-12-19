
/*
    A simple bit logic calculator.

    Tested on Ubuntu14.04 with GTK3.10 

    gcc -Wall bit_calc1.c -o bit_calc1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>

//Some entry validation.
static void validate_entries(GtkWidget *widget, gpointer *data);
static int validate_shift_entries(gpointer *data);
static void validate_number_entries(gpointer *data);
//Bit logic.
static void get_bits(gpointer *data);
inline static unsigned char bit_value(unsigned char x, unsigned char p);
//Draw program icon.
static GdkPixbuf* draw_icon();
static void about_dialog(GtkWidget *widget, gpointer data);
static void error_message(const gchar *string);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Bit Calc");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Bit Calc");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *entry1=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry1), "255");
    gtk_widget_set_halign(entry1, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", ">>");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "<<");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "~");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo1), "1");

    GtkWidget *entry2=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry2), "0");
    gtk_widget_set_halign(entry2, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 1);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "&");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "|");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "^");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo2), "1");

    GtkWidget *entry3=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry3), "0");
    gtk_widget_set_halign(entry3, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 3);

    GtkWidget *combo3=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", ">>");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "<<");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "~");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo3), "1");

    GtkWidget *entry4=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry4), "0");
    gtk_widget_set_halign(entry4, GTK_ALIGN_START);
    gtk_entry_set_width_chars(GTK_ENTRY(entry4), 1);

    GtkWidget *label1=gtk_label_new("1 1 1 1 1 1 1 1");
    gtk_widget_set_halign(label1, GTK_ALIGN_END);

    GtkWidget *label2=gtk_label_new("0 0 0 0 0 0 0 0");
    gtk_widget_set_halign(label2, GTK_ALIGN_END);

    GtkWidget *label3=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label3), "<span foreground='blue'>0 0 0 0 0 0 0 0</span>");
    gtk_widget_set_halign(label3, GTK_ALIGN_END);

    GtkWidget *label4=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label4), "<span foreground='blue'>0</span>");
    gtk_widget_set_halign(label4, GTK_ALIGN_END);

    GtkWidget *label5=gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label5), "0xff");

    GtkWidget *label6=gtk_label_new(NULL);
    gtk_label_set_text(GTK_LABEL(label6), "0x0");

    GtkWidget *label7=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label7), "<span foreground='blue'>0x0</span>");
    
    gpointer widgets[]={entry1, entry2, entry3, entry4, combo1, combo2, combo3, label1, label2, label3, label4, label5, label6, label7};

    GtkWidget *button1=gtk_button_new_with_label("Get Bits");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(validate_entries), widgets);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 2, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 3, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 2, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 2, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 3, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 3, 5, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 1, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 3, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 1, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 3, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 1, 6, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 3, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 2, 7, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    GError *css_error=NULL;
    gchar css_string[]="GtkWindow{background: rgba(153,204,255,1)} GtkButton{background: cyan}";
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
static void validate_entries(GtkWidget *widget, gpointer *data)
  {
    gint check1=validate_shift_entries(data);
    if(check1==0) validate_number_entries(data);    
  }
static int validate_shift_entries(gpointer *data)
  {
    gint ret_val=0;
    gboolean entry1_zero=FALSE;
    gboolean entry2_zero=FALSE;
    gint entry1_value=g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[1])), NULL, 10);
    gint entry2_value=g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[3])), NULL, 10);
    //Check for 0. strtoll might return a 0 on error.
    if(g_strcmp0(gtk_entry_get_text(GTK_ENTRY(data[1])), "0")==0) entry1_zero=TRUE;
    if(g_strcmp0(gtk_entry_get_text(GTK_ENTRY(data[3])), "0")==0) entry2_zero=TRUE;

    if(entry1_value!=0||entry1_zero)
      {
        if(entry2_value!=0||entry2_zero)
          {
            if(entry1_value<=8&&entry1_value>=0)
              {
                if(entry2_value>8||entry2_value<0)
                  {
                    gchar *error_string=g_strdup("Shift2 Range 0 <= x <= 8");
                    g_print("%s\n", error_string);
                    error_message(error_string);
                    g_free(error_string);
                    ret_val=1; 
                  }
              }
            else
              {
                gchar *error_string=g_strdup("Shift1 Range 0 <= x <= 8");
                g_print("%s\n", error_string);
                error_message(error_string);
                g_free(error_string);
                ret_val=1; 
              }
           }
        else
           {
             gchar *error_string=g_strdup("Shift2 int conversion error.");
             g_print("%s\n", error_string);
             error_message(error_string);
             g_free(error_string);
             ret_val=1;
           }
      }
    else
      {
        gchar *error_string=g_strdup("Shift1 int conversion error.");
        g_print("%s\n", error_string);
        error_message(error_string);
        g_free(error_string);
        ret_val=1;
      }
    return ret_val;
  }
static void validate_number_entries(gpointer *data)
  {
    gboolean entry1_zero=FALSE;
    gboolean entry2_zero=FALSE;
    gint entry1_value=g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[0])), NULL, 10);
    gint entry2_value=g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[2])), NULL, 10);
    //Check for 0. strtoll might return a 0 on error.
    if(g_strcmp0(gtk_entry_get_text(GTK_ENTRY(data[0])), "0")==0) entry1_zero=TRUE;
    if(g_strcmp0(gtk_entry_get_text(GTK_ENTRY(data[2])), "0")==0) entry2_zero=TRUE;

    if(entry1_value!=0||entry1_zero)
      {
        if(entry2_value!=0||entry2_zero)
          {
            if(entry1_value<256&&entry1_value>=0)
              {
                if(entry2_value<256&&entry2_value>=0)
                  {
                    //Fill in labels with bit values.
                    get_bits(data);
                  }
                else
                  {
                    gchar *error_string=g_strdup("Entry2 Range 0 <= x < 256");
                    g_print("%s\n", error_string);
                    error_message(error_string);
                    g_free(error_string); 
                  }
              }
            else
              {
                gchar *error_string=g_strdup("Entry1 Range 0 <= x < 256");
                g_print("%s\n", error_string);
                error_message(error_string);
                g_free(error_string);  
              }
           }
        else
           {
             gchar *error_string=g_strdup("Entry2 int conversion error.");
             g_print("%s\n", error_string);
             error_message(error_string);
             g_free(error_string); 
           }
      }
    else
      {
        gchar *error_string=g_strdup("Entry1 int conversion error.\n");
        g_print("%s\n", error_string);
        error_message(error_string);
        g_free(error_string); 
      }
  }
static void get_bits(gpointer *data)
  {
    gint active_combo_shift1=gtk_combo_box_get_active(GTK_COMBO_BOX(data[4]));
    gint active_combo=gtk_combo_box_get_active(GTK_COMBO_BOX(data[5]));
    gint active_combo_shift2=gtk_combo_box_get_active(GTK_COMBO_BOX(data[6]));
    unsigned char entry1_value=(unsigned char)g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[0])), NULL, 10);
    unsigned char entry2_value=(unsigned char)g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[2])), NULL, 10);
    unsigned char shift_entry1=(unsigned char)g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[1])), NULL, 10);
    unsigned char shift_entry2=(unsigned char)g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[3])), NULL, 10);
    
    //Shift bits.
    if(active_combo_shift1==0) entry1_value=entry1_value>>shift_entry1;
    else if(active_combo_shift1==1) entry1_value=entry1_value<<shift_entry1;
    else entry1_value=~entry1_value;

    if(active_combo_shift2==0) entry2_value=entry2_value>>shift_entry2;
    else if(active_combo_shift2==1) entry2_value=entry2_value<<shift_entry2;
    else entry2_value=~entry2_value;

    //Build bit and hex labels.
    gchar *string1=g_strdup_printf("%i %i %i %i %i %i %i %i", (int)bit_value(entry1_value, 7), (int)bit_value(entry1_value, 6), (int)bit_value(entry1_value, 5), (int)bit_value(entry1_value, 4), (int)bit_value(entry1_value, 3), (int)bit_value(entry1_value, 2), (int)bit_value(entry1_value, 1), (int)bit_value(entry1_value, 0));

    gchar *string2=g_strdup_printf("%i %i %i %i %i %i %i %i", (int)bit_value(entry2_value, 7), (int)bit_value(entry2_value, 6), (int)bit_value(entry2_value, 5), (int)bit_value(entry2_value, 4), (int)bit_value(entry2_value, 3), (int)bit_value(entry2_value, 2), (int)bit_value(entry2_value, 1), (int)bit_value(entry2_value, 0));

    gchar *string3=g_strdup_printf("0x%0x", entry1_value);
    gchar *string4=g_strdup_printf("0x%0x", entry2_value);

    gtk_label_set_text(GTK_LABEL(data[7]), string1);
    gtk_label_set_text(GTK_LABEL(data[8]), string2);
    gtk_label_set_text(GTK_LABEL(data[11]), string3);
    gtk_label_set_text(GTK_LABEL(data[12]), string4);

    //Bitwise and/or/xor.
    unsigned char bit_logic=0;
    if(active_combo==0)
      {
        bit_logic=entry1_value&entry2_value;
      }
    else if(active_combo==1)
      {
        bit_logic=entry1_value|entry2_value;
      }
    else
      {
        bit_logic=entry1_value^entry2_value;
      }

    gchar *string5=g_strdup_printf("<span foreground='blue'>%i %i %i %i %i %i %i %i</span>", (int)bit_value(bit_logic, 7), (int)bit_value(bit_logic, 6), (int)bit_value(bit_logic, 5), (int)bit_value(bit_logic, 4), (int)bit_value(bit_logic, 3), (int)bit_value(bit_logic, 2), (int)bit_value(bit_logic, 1), (int)bit_value(bit_logic, 0));
    gchar *string6=g_strdup_printf("<span foreground='blue'>%i</span>", (gint)bit_logic);
    gchar *string7=g_strdup_printf("<span foreground='blue'>0x%0x</span>", bit_logic);

    gtk_label_set_markup(GTK_LABEL(data[9]), string5);
    gtk_label_set_markup(GTK_LABEL(data[10]), string6);
    gtk_label_set_markup(GTK_LABEL(data[13]), string7);

    g_free(string1);
    g_free(string2);
    g_free(string3);
    g_free(string4); 
    g_free(string5);  
    g_free(string6); 
    g_free(string7);      
  }
inline static unsigned char bit_value(unsigned char x, unsigned char p)
  {
    return (x>>p)&1;
  }
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    gint i=0;
    
    //Paint the background.
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_paint(cr);

    //A blue icon border.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, 256, 256);
    cairo_stroke(cr);

    //Smiley
    cairo_save(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_scale(cr, 0.5, 0.5);
    cairo_translate(cr, 128.0, 215.0);
    cairo_set_line_width(cr, 10);
    //Circle
    cairo_arc(cr, 128, 128, 100, 0, 2*G_PI);
    cairo_stroke(cr);
    //Left eye.
    cairo_arc(cr, 128-35, 90, 7, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke(cr);
    //Right eye.
    cairo_arc(cr, 128+35, 90, 7, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke(cr);
    //Smile
    cairo_arc(cr, 128, 130, 60, 0, G_PI);
    cairo_stroke(cr);
    cairo_restore(cr);

    //Light bulbs.
    for(i=0;i<8;i++)
      {
        cairo_save(cr);
        cairo_translate(cr, i*28.0, 10.0);
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_set_line_width(cr, 3);
        //Top Circle
        cairo_arc(cr, 30, 30, 12, 0, 2*G_PI);
        cairo_fill(cr);
        cairo_stroke(cr);
        //Curve of bulb
        cairo_move_to(cr, 40, 35);
        cairo_curve_to(cr, 30, 65, 37, 65, 37, 65);
        cairo_stroke_preserve(cr);
        cairo_line_to(cr, 23, 65);
        cairo_stroke_preserve(cr);
        cairo_curve_to(cr, 23, 65, 30, 65, 20, 35);
        cairo_stroke_preserve(cr);
        cairo_close_path(cr);
        cairo_stroke_preserve(cr);
        cairo_fill(cr);
        //Bottom square on bulb.
        cairo_set_source_rgb(cr, 0.8, 0.8, 0.8);
        cairo_move_to(cr, 0, 0);
        cairo_rectangle(cr, 22, 55, 16, 20);
        cairo_fill(cr);
        cairo_restore(cr);
      }

    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface, 0, 0, 256, 256);

    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    return icon;
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    //Null will add the program icon to the logo.
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Bit Calc");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A simple bitwise calculator.");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static void error_message(const gchar *string)
  {
    GtkWidget *dialog=gtk_message_dialog_new(NULL, GTK_DIALOG_MODAL, GTK_MESSAGE_INFO, GTK_BUTTONS_CLOSE, "%s", string);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }

