
/*

    This is from the discussion "[Could end up being a bug] GtkFileChooser insolently expands"
with AI390 on the GTK forum. The goal is to add a textview to a file chooser widget. There was
some problems in doing so. One was the GtkBox the textview gets put in doesn't want to expand
horizontally. To get the box the textview is in, there is a recursive function that looks through
all of the widgets in the file chooser and finds the box the textview is in. Then several methods
of trying to get the box to expand were tried. None of which seemed to work. So a spacer label was
added to force the widget to expand.

    gcc -Wall recursion1.c -o recursion1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include <gtk/gtk.h>

static void internal_find_func (GtkWidget* widget, gint *count)
  {
    const char *name = gtk_widget_get_name(widget);
    printf("% *d. %s\n", (*count)*2, *count, name);

    //By trial and error the box that we are looking is the only box at 7.
    if(*count==7&&!g_strcmp0(name, "GtkBox"))
      {
        //Force expansion of box. Try some different methods to try to get the box to expand.
        GtkWidget *spacer=gtk_label_new("........................................");
        gtk_widget_set_hexpand(spacer, TRUE);
        gtk_widget_set_halign(spacer, GTK_ALIGN_FILL);
        gtk_box_pack_end(GTK_BOX(widget), spacer, FALSE, FALSE, 0);
        gtk_widget_show(spacer);
      }

    //Recursive call for each container widget in the file chooser dialog.
    if(GTK_IS_CONTAINER(widget))
      {
        (*count)++;
        gtk_container_foreach(GTK_CONTAINER(widget), (GtkCallback)internal_find_func, count);
        (*count)--;
      }

  }
static void show_file_chooser(GtkWidget *widget, gpointer user_data)
  {
    GtkWidget *textview=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    gtk_widget_set_hexpand(textview, TRUE);
    gtk_widget_set_vexpand(textview, TRUE);
    gtk_widget_set_name(textview, "GtkTextView_Fred");

    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, "Some test text that will be too long for the textview in the filechooser.", -1);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), textview);

    GtkWidget *dialog=gtk_file_chooser_dialog_new ("Open File", GTK_WINDOW(user_data), GTK_FILE_CHOOSER_ACTION_OPEN, "Cancel", GTK_RESPONSE_CANCEL, NULL);

    gtk_file_chooser_set_preview_widget(GTK_FILE_CHOOSER(dialog), scroll);
    gtk_file_chooser_set_preview_widget_active(GTK_FILE_CHOOSER(dialog), TRUE);

    gtk_widget_show(textview);
    gint count=0;
    gtk_container_forall(GTK_CONTAINER(dialog), (GtkCallback)internal_find_func, &count);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);                                       
  }
int main (int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "File Chooser");
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *button1=gtk_button_new_with_label("Show File Chooser");
    g_signal_connect(button1, "clicked", G_CALLBACK(show_file_chooser), window);

    gtk_container_add(GTK_CONTAINER(window), button1);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
