
/*

    A simple bit logic calculator.
    Tested on Ubuntu14.04 with GTK3.10 

    gcc -Wall bit_calc1.c -o bit_calc1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>

static unsigned char bit_value(unsigned char x, unsigned char p)
  {
    return (x>>p)&1;
  }
static void get_bits(gpointer *data)
  {
    g_print("Get Bits\n");
    gint active_combo=gtk_combo_box_get_active(GTK_COMBO_BOX(data[1]));
    unsigned char entry1_value=(unsigned char)g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[0])), NULL, 10);
    unsigned char entry2_value=(unsigned char)g_ascii_strtoll(gtk_entry_get_text(GTK_ENTRY(data[2])), NULL, 10);
    
    gchar *string1=g_strdup_printf("%i %i %i %i %i %i %i %i\n", (int)bit_value(entry1_value, 7), (int)bit_value(entry1_value, 6), (int)bit_value(entry1_value, 5), (int)bit_value(entry1_value, 4), (int)bit_value(entry1_value, 3), (int)bit_value(entry1_value, 2), (int)bit_value(entry1_value, 1), (int)bit_value(entry1_value, 0));

    gchar *string2=g_strdup_printf("%i %i %i %i %i %i %i %i\n", (int)bit_value(entry2_value, 7), (int)bit_value(entry2_value, 6), (int)bit_value(entry2_value, 5), (int)bit_value(entry2_value, 4), (int)bit_value(entry2_value, 3), (int)bit_value(entry2_value, 2), (int)bit_value(entry2_value, 1), (int)bit_value(entry2_value, 0));

    gtk_label_set_text(GTK_LABEL(data[3]), string1);
    gtk_label_set_text(GTK_LABEL(data[4]), string2);

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

    gchar *string3=g_strdup_printf("%i %i %i %i %i %i %i %i\n", (int)bit_value(bit_logic, 7), (int)bit_value(bit_logic, 6), (int)bit_value(bit_logic, 5), (int)bit_value(bit_logic, 4), (int)bit_value(bit_logic, 3), (int)bit_value(bit_logic, 2), (int)bit_value(bit_logic, 1), (int)bit_value(bit_logic, 0));
    gchar *string4=g_strdup_printf("%i", (gint)bit_logic);

    gtk_label_set_text(GTK_LABEL(data[5]), string3);
    gtk_label_set_text(GTK_LABEL(data[6]), string4);

    g_free(string1);
    g_free(string2);
    g_free(string3);
    g_free(string4);     
  }
static void validate_entries(GtkWidget *widget, gpointer *data)
  {
    g_print("Validate Entries\n");
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
                    g_print("Entry2 Range 0 <= x < 256\n"); 
                  }
              }
            else
              {
                g_print("Entry1 Range 0 <= x < 256\n"); 
              }
           }
        else
           {
             g_print("Entry2 int conversion error.\n");
           }
      }
    else
      {
        g_print("Entry1 int conversion error.\n");
      }

  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Bit Calc");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 300);    
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *entry1=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry1), "255");
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "&");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "|");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "^");
    gtk_combo_box_set_active_id(GTK_COMBO_BOX(combo1), "1");

    GtkWidget *entry2=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry2), "0");
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 3);

    GtkWidget *label1=gtk_label_new("1 1 1 1 1 1 1 1");
    gtk_widget_set_hexpand(label1, TRUE);

    GtkWidget *label2=gtk_label_new("0 0 0 0 0 0 0 0");
    gtk_widget_set_hexpand(label2, TRUE);

    GtkWidget *label3=gtk_label_new("0 0 0 0 0 0 0 0");
    gtk_widget_set_hexpand(label3, TRUE);

    GtkWidget *label4=gtk_label_new("0");
    gtk_widget_set_hexpand(label4, TRUE);
    
    gpointer widgets[]={entry1, combo1, entry2, label1, label2, label3, label4};

    GtkWidget *button1=gtk_button_new_with_label("Get Bits");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(validate_entries), widgets);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_container_set_border_width(GTK_CONTAINER(grid), 20);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 3, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 4, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 6, 3, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 7, 3, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;   
  }
