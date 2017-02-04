
/*

    Test a case in-sensitive search and compare it with the GTK text in-sensitive search.
Follow up to.

    https://mail.gnome.org/archives/gtk-devel-list/2017-January/msg00025.html

which details some language details that cause problems checking the upper and lower case chars.

    This version tries a case-insensitive search with a UTF-8 casefold function. Still some
problems with the ligatures. For example, if you search ff on the "ffl" single char, how do 
you just highlight ff? 

    With a lot of mallocs and frees for each char it runs a little slow. If you could reuse a
small byte array with the casefold function maybe the performance might be better. Another strategy
would be to have a function look for if a char was a ligature and return true/false before
using the casefold function.

    Take a look at search_textbuffer4 for getting rid of the mallocs.
    
    gcc -Wall search_textbuffer3.c -o search_textbuffer3 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include <gtk/gtk.h>

static gboolean text_iter_forward_search(GtkTextIter *start, gchar *search_string, GtkTextIter *start_word, GtkTextIter *end_word)
  {
    //Case in-sensitive search.
    gint i=0;
    gchar *casefold_search=g_utf8_casefold(search_string, -1); 
    gchar *p=casefold_search;
    glong count=g_utf8_strlen(casefold_search, -1);
    glong counter=0;
    gint first_repeat=0;
    gchar first_char=*p;
    gint backwards_chars=0;

    //Check if there are some chars.
    if(count>0)
      {
         /*
          Check for a second occurance of the first char in the string. Need this to "search sa" in
          "search search sand". The previous search ends at 'e'. When searching, the loop
          might have to check some chars more than once.
         */
        p=g_utf8_find_next_char(p, NULL);
        for(i=1;i<count;i++)
          {
            if(g_utf8_get_char(p)==first_char)
              {
                first_repeat=i;
                break;
              }
            p=g_utf8_find_next_char(p, NULL);
          }
        p=casefold_search;
        if(first_repeat>0) backwards_chars=count-first_repeat;

        gtk_text_iter_assign(start_word, start);
        GtkTextIter start_plus;
        gtk_text_iter_assign(&start_plus, start); 

        //Search for the word.
        gint case_len=0;
        gint case_len_forward=0;
        gint max=0;
        gint offset=0;
        gunichar c;
        gboolean run_loop=TRUE;
        do
          {
            gtk_text_iter_forward_char(&start_plus);
            gchar *uni=gtk_text_iter_get_slice(start, &start_plus);
            gchar *casefold=g_utf8_casefold(uni, -1);
            case_len=g_utf8_strlen(casefold, -1);                                
            g_free(uni);

            //Start a multi char ligature.
            if(case_len>1&&case_len_forward==0)
              {  
                //g_print("case %i\n", case_len);             
                case_len_forward=case_len;
                max=case_len_forward;
              }   

            //If we are checking a ligature else just get the char.
            if(case_len_forward>0)
              {
                //count forward.
                offset=max-case_len_forward;
                //g_print("forward %s %i %i\n", casefold, max, case_len_forward);
                c=g_utf8_get_char(g_utf8_offset_to_pointer(casefold, offset));
                //g_print("|%c|%c|%i,%i,%i\n", g_utf8_get_char(p), c, max, case_len_forward, offset);
                case_len_forward--;
              }
            else
              { 
                c=g_utf8_get_char(casefold);
                //g_print("|%c|%c|%i,%i,%i\n", g_utf8_get_char(p), c, max, case_len_forward, offset); 
              }
             
            if(g_utf8_get_char(p)==c)
              {
                gtk_text_iter_assign(end_word, start);
                gtk_text_iter_forward_char(end_word);
                counter++;  
                p=g_utf8_find_next_char(p, NULL); //p++ for utf-8.
                if(counter>=count)
                  {
                    return TRUE;
                  }
              }
            else
              {
                case_len_forward=0;
                gtk_text_iter_assign(start_word, start);
                gtk_text_iter_forward_char(start_word);
                if(counter>0)
                  {
                    if(first_repeat>0&&counter>first_repeat)
                      {
                        gtk_text_iter_backward_chars(start, backwards_chars);
                        gtk_text_iter_backward_chars(&start_plus, backwards_chars);  
                        gtk_text_iter_backward_chars(start_word, backwards_chars); 
                      } 
                    counter=0;
                    p=casefold_search;
                  }                           
              }

            g_free(casefold);

            //Need to hold on the ligature char until it's chars are checked.
            if(case_len_forward>0) gtk_text_iter_backward_char(&start_plus);
            else run_loop=gtk_text_iter_forward_char(start);

          }while(run_loop);
      }    

    g_free(casefold_search);
    return FALSE;
    
  }
static void search1(GtkWidget *button, gpointer *data)
  {
    GtkTextIter start;
    GtkTextIter end;
    GtkTextIter start_word;
    GtkTextIter end_word;
    gchar *search_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(data[1])));
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(data[0]), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(data[0]), &end);
    gtk_text_buffer_remove_all_tags(GTK_TEXT_BUFFER(data[0]), &start, &end);

    while(text_iter_forward_search(&start, search_string, &start_word, &end_word))
      {
        gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(data[0]), "yellow-tag", &start_word, &end_word);
        start=end_word;
      }

    g_free(search_string);
  }
static void search2(GtkWidget *button, gpointer *data)
  {
    GtkTextIter start;
    GtkTextIter end;
    GtkTextIter start_word;
    GtkTextIter end_word;
    gchar *search_string=g_strdup(gtk_entry_get_text(GTK_ENTRY(data[1])));
    gtk_text_buffer_get_start_iter(GTK_TEXT_BUFFER(data[0]), &start);
    gtk_text_buffer_get_end_iter(GTK_TEXT_BUFFER(data[0]), &end);
    gtk_text_buffer_remove_all_tags(GTK_TEXT_BUFFER(data[0]), &start, &end);

    while(gtk_text_iter_forward_search(&start, search_string, GTK_TEXT_SEARCH_CASE_INSENSITIVE, &start_word, &end_word, NULL))
      {
        gtk_text_buffer_apply_tag_by_name(GTK_TEXT_BUFFER(data[0]), "yellow-tag", &start_word, &end_word);
        start=end_word;
      }

    g_free(search_string);
  }
static void button1_clicked(GtkWidget *button, gpointer *data)
  {
    GTimer *timer=g_timer_new();
    gint i=0;

    for(i=0;i<1000;i++) search1(button, data);

    g_print("Timer1 %f\n", g_timer_elapsed(timer, NULL));
    g_timer_destroy(timer);
  }
static void button2_clicked(GtkWidget *button, gpointer *data)
  {
    GTimer *timer=g_timer_new();
    gint i=0;

    for(i=0;i<1000;i++) search2(button, data);

    g_print("Timer2 %f\n", g_timer_elapsed(timer, NULL));
    g_timer_destroy(timer);
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Search Textbuffer Test");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *textview=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_CHAR);
    gtk_widget_set_size_request(textview, 400, 300);

    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, "SEArch Add search a some words to Search  search SearCH a SEaRch search search Search sand a few extra s ss      sssr Sea Search zzzzzzz compare Straße and STRASSE ﬁeld FIELD Straße and STRASSE ﬁeld FIELD ﬄ  ﬄ testﬄ.", -1);
    gtk_text_buffer_create_tag(buffer, "yellow-tag", "background", "yellow", NULL); 

    GtkWidget *entry=gtk_entry_new();
    gtk_entry_set_text(GTK_ENTRY(entry), "search");

    GtkWidget *button1=gtk_button_new_with_label("Test Search");
    gpointer test[]={buffer, entry};
    g_signal_connect(button1, "clicked", G_CALLBACK(button1_clicked), test);

    GtkWidget *button2=gtk_button_new_with_label("Gtk Search");
    g_signal_connect(button2, "clicked", G_CALLBACK(button2_clicked), test);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), textview, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 3, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
  }


