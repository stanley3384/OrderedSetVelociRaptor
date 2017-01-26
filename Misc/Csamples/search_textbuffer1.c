
/*

    Test a case insensitive search in GTK2.
    
    gcc -Wall search_textbuffer1.c -o search_textbuffer1 `pkg-config --cflags --libs gtk+-2.0`

    C. Eric Cashon
*/

#include <gtk/gtk.h>

static void button_clicked(GtkWidget *button, gpointer *data)
  {
    GtkTextIter start;
    GtkTextIter end;
    GtkTextIter start_word;
    GtkTextIter end_word;
    gchar *search_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(data[1])));
    gchar *p=search_string;
    glong count=g_utf8_strlen(search_string, -1);
    glong counter=0;
    //Might want to put the temp_string on the heap.
    gchar temp_string[count+1];
    temp_string[count]='\0';
    gchar *temp_p=temp_string;

    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(data[0]), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(data[0]), &end);
    gtk_text_buffer_remove_all_tags(GTK_TEXT_BUFFER(data[0]), &start, &end);
    start_word=start;

    //Check if there are some chars.
    if(count>0&&gtk_text_buffer_get_char_count(GTK_TEXT_BUFFER(data[0]))>0)
      {
        do
          {
            if('\n'==gtk_text_iter_get_char(&start))
              {
                //Skip over newlines.
              }
            //Should be more lower chars so check first.
            else if(g_unichar_tolower(g_utf8_get_char(p))==gtk_text_iter_get_char(&start)||g_unichar_toupper(g_utf8_get_char(p))==gtk_text_iter_get_char(&start))
              {
                end_word=start;
                gtk_text_iter_forward_char(&end_word);
                *temp_p=gtk_text_iter_get_char(&start);
                counter++;
                temp_p=g_utf8_offset_to_pointer(temp_string, counter); 
                p=g_utf8_find_next_char(p, NULL); //p++ for utf-8.
                if(counter>=count)
                  {
                    if(!gtk_text_iter_is_start(&start_word)) gtk_text_iter_forward_char(&start_word);
                    gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(data[0]), "yellow-tag", &start_word, &end_word);
                    g_print("%s\n", temp_string);
                    start_word=start;
                    counter=0;
                    p=search_string;
                    temp_p=temp_string;
                  }
              }
            else
              {
                if(counter>0)
                  {
                    counter=0;
                    p=search_string;
                    temp_p=temp_string;
                  }
                start_word=start;
              }
          }while(gtk_text_iter_forward_char(&start));
      }    

    g_free(search_string);
    
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Search Textbuffer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *textview=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_CHAR);
    gtk_widget_set_size_request(textview, 400, 300);

    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, "SEArch Add some words to Search search SearCH SEaRch search search Search and a few extra s ss sssr Sea Sear.", -1);
    gtk_text_buffer_create_tag(buffer, "yellow-tag", "background", "yellow", NULL); 

    GtkWidget *entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "search");

    GtkWidget *button=gtk_button_new_with_label("Search");
    gpointer test[]={buffer, entry};
    g_signal_connect(button, "clicked", G_CALLBACK(button_clicked), test);

    GtkWidget *table=gtk_table_new(3, 1, FALSE);
    gtk_table_attach(GTK_TABLE(table), textview, 0, 1, 0, 1, GTK_EXPAND, GTK_EXPAND, 0, 0);
    gtk_table_attach(GTK_TABLE(table), entry, 0, 1, 1, 2, GTK_EXPAND, GTK_SHRINK, 0, 0);
    gtk_table_attach(GTK_TABLE(table), button, 0, 1, 2, 3, GTK_EXPAND, GTK_SHRINK, 0, 0);

    gtk_container_add(GTK_CONTAINER(window), table);

    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
  }

