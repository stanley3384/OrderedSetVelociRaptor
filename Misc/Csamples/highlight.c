
/*

  Add a string to a textview and highlight the string. Use a GArray to pass widget pointers
  to the callback function.

  gcc -Wall highlight.c -o highlight `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon

*/

#include <gtk/gtk.h>

static void append_test_string(GtkWidget *button, GArray *widgets)
  {
    g_print("Append Test String\n");
    GtkTextIter start, end;

    gchar *string=g_strdup_printf("%s\n", gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 0))));
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_array_index(widgets, GtkWidget*, 1)));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &end, string, -1, "gray_bg", NULL);
    g_free(string);
  }
static void insert_test_string(GtkWidget *button, GArray *widgets)
  {
    g_print("Insert Test String\n");
    GtkTextIter start, end, cursor;

    const gchar *string=gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 0)));
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(g_array_index(widgets, GtkWidget*, 1)));
    GtkTextMark *mark=gtk_text_buffer_get_insert(buffer);
    gtk_text_buffer_get_iter_at_mark(buffer, &cursor, mark); 
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_remove_all_tags(buffer, &start, &end);
    gtk_text_buffer_insert_with_tags_by_name(buffer, &cursor, string, -1, "gray_bg", NULL);
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    printf("Quit Program\n");
    gtk_main_quit();
  }
int main(int argc, char *argv[])
  {
    GtkWidget *window, *button1, *button2, *entry, *textview, *grid;

    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), "Test Highlight");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    button1=gtk_button_new_with_label("Append Test String");
    gtk_widget_set_hexpand(button1, TRUE);
    button2=gtk_button_new_with_label("Insert Test String");
    gtk_widget_set_hexpand(button2, TRUE);

    textview=gtk_text_view_new();
    gtk_widget_set_hexpand(textview, TRUE);
    gtk_widget_set_vexpand(textview, TRUE);
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_create_tag(buffer, "gray_bg", "background", "gray", NULL);

    entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "Test String");

    GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
    g_array_append_val(widgets, entry);
    g_array_append_val(widgets, textview);
    g_signal_connect(button1, "clicked", G_CALLBACK(append_test_string), widgets);
    g_signal_connect(button2, "clicked", G_CALLBACK(insert_test_string), widgets);

    grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 5);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), textview, 0, 3, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);
    gtk_main();
    return 0;   
  }
