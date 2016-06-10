
/*
    Test a GTK source view widget. 

    On Ubuntu14.04 get headers with:
        sudo apt-get install libgtksourceview-3.0-dev

    Compile with:
        gcc -Wall source_view1.c -o source_view1 `pkg-config --cflags --libs gtk+-3.0 gtksourceview-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<gtksourceview/gtksource.h>

int main (int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Source View");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    //Highlight for the C language.
    GtkSourceLanguageManager *lm=gtk_source_language_manager_get_default();
    GtkSourceLanguage *language=gtk_source_language_manager_get_language(lm, "c");
    GtkSourceBuffer *s_buffer=gtk_source_buffer_new_with_language(language);
    gtk_source_buffer_set_language(s_buffer, language);
    gtk_source_buffer_set_highlight_syntax(s_buffer, TRUE);

    GtkWidget *source_view=gtk_source_view_new_with_buffer(s_buffer);
    gtk_source_view_set_show_line_numbers(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_show_line_marks(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_source_view_set_highlight_current_line(GTK_SOURCE_VIEW(source_view), TRUE);
    gtk_widget_set_vexpand(source_view, TRUE);
    gtk_widget_set_hexpand(source_view, TRUE);

    //Create a mark on the first line.
    GtkTextIter start;
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(s_buffer), &start);
    gtk_source_buffer_create_source_mark(s_buffer, "mark_name", "mark_category", &start);
    
    //Color the first line green.
    GdkRGBA bg;
    gdk_rgba_parse(&bg, "#00FF00");
    GtkSourceMarkAttributes *att=gtk_source_mark_attributes_new();
    gtk_source_mark_attributes_set_background(att, &bg);
    gtk_source_view_set_mark_attributes(GTK_SOURCE_VIEW(source_view), "mark_category", att, 1);

    gtk_text_buffer_set_text(GTK_TEXT_BUFFER(s_buffer), "//gcc -Wall hello1.c -o hello1\n#include<stdio.h>\n\nint main()\n  {\n    printf(\"Hello World\\n\");\n    return 0;\n  }\n", -1);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), source_view, 0, 0, 1, 1);
    
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
