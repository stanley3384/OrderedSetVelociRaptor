
/*

  Change values of several entries at once. Use a structure to pass widget pointers to
  the callback function.

  gcc -Wall focus.c -o focus `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon

*/

#include <gtk/gtk.h>

typedef struct
{
GtkWidget *entry1, *entry2, *entry3, *entry4;
} Entries;

static void update_entries(GtkWidget *widget, Entries *es)
  {
    static int i;
    gchar *value1=g_strdup_printf("Value %i", i++);
    gchar *value2=g_strdup_printf("Value %i", i++);
    gchar *value3=g_strdup_printf("Value %i", i++);
    gchar *value4=g_strdup_printf("Value %i", i++);

    gtk_entry_set_text(GTK_ENTRY(es->entry1), value1);
    gtk_entry_set_text(GTK_ENTRY(es->entry2), value2);
    gtk_entry_set_text(GTK_ENTRY(es->entry3), value3);
    gtk_entry_set_text(GTK_ENTRY(es->entry4), value4);

    g_free(value1);
    g_free(value2);
    g_free(value3);
    g_free(value4);
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    printf("Quit Program\n");
    gtk_main_quit();
  }
int main(int argc, char *argv[])
  {
    GtkWidget *window, *button1, *entry1, *entry2, *entry3, *entry4, *grid;

    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), "Test Entries");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 150);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);
 
    button1=gtk_button_new_with_label("Change Entries");

    entry1=gtk_entry_new();
    entry2=gtk_entry_new();
    entry3=gtk_entry_new();
    entry4=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry1), "1");
    gtk_entry_set_text(GTK_ENTRY(entry2), "2");
    gtk_entry_set_text(GTK_ENTRY(entry3), "3");
    gtk_entry_set_text(GTK_ENTRY(entry4), "4");

    Entries *es;
    es=g_slice_new(Entries);
    es->entry1=entry1;
    es->entry2=entry2;
    es->entry3=entry3;
    es->entry4=entry4;
    g_signal_connect(button1, "clicked", G_CALLBACK(update_entries), (gpointer)es); 

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 0, 4, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
    
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
