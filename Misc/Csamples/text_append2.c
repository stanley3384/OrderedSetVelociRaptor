
/*
   Test code to append some text files. Add a UI to the text_append code. Multi-select files and
combine or append them together into a new file.

   gcc -Wall text_append2.c -o text_append2 `pkg-config --cflags --libs gtk+-3.0`

   C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

static void show_file_dialog(GtkWidget *button, GArray *widgets);
static void append_text_files(GSList *text_file_paths, const gchar *new_file_name);

int main(int argc, char **argv)
  {
    GtkWidget *window, *label, *entry, *button, *grid;
    
    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Select Text Files");
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 100);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    label=gtk_label_new("Combined Text File Name");
    gtk_widget_set_hexpand(label, TRUE);

    entry=gtk_entry_new();
    gtk_widget_set_hexpand(entry, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry), "combined.txt");

    GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
    g_array_append_val(widgets, window);
    g_array_append_val(widgets, entry);

    button=gtk_button_new_with_label("Append Text Files");
    gtk_widget_set_hexpand(button, TRUE);
    g_signal_connect(button, "clicked", G_CALLBACK(show_file_dialog), widgets); 

    grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
    gtk_grid_attach(GTK_GRID(grid), label, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 2, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;
  }
static void show_file_dialog(GtkWidget *button, GArray *widgets)
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
                 fread(pTextBuffer, file_size, 1, fp);
                 fwrite(pTextBuffer, file_size, 1, combined);
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

