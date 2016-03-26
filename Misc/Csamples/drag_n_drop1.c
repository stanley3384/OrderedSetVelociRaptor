
/*

    Test some drag and drop out. Drag a desktop icon to the top button widget. If there is
a desktop file associated with the item then look in it and find the name of the icon. Then 
locate the icon and put it in the image widget.

    Tested on Ubuntu14.04 and GTK3.10 

    gcc -Wall drag_n_drop1.c -o drag_n_drop1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/
 
#include<gtk/gtk.h>

static gboolean drop_widget(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, gpointer data)
  {
    g_print("Drop Widget\n");
    GdkAtom target;
    target=gtk_drag_dest_find_target(widget, context, NULL);
    if(target!=GDK_NONE)
      {
        gchar *string=gdk_atom_name(target);
        g_print("Found Target %s\n", string);
        g_free(string);
        gtk_drag_get_data(widget, context, target, time);
        gtk_drag_finish(context, TRUE, FALSE, time);
        return TRUE;
      }
  
   return FALSE;
  }
static void data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *data, guint info, guint time, gpointer user_data)
  {
    g_print("Get Data\n");    
    guchar *selection_string=gtk_selection_data_get_text(data);
    gtk_button_set_label(GTK_BUTTON(widget), (gchar*)selection_string);
   
    //Remove newlines and uri "file".
    gint i=0;
    gboolean newline=FALSE;
    gboolean file_string=TRUE;
    gchar *file_path1=NULL;
    gchar *file_path2=NULL;
    GError *error1=NULL;
    gchar *p=(gchar*)selection_string;
    while(g_utf8_get_char(p)!='\0')
      {
        if(g_utf8_get_char(p)=='\n')
          {
            newline=TRUE;
            break;
          }
        i++;
        p=g_utf8_next_char(p);
      }
    g_print("%s", (gchar*)selection_string);
    if(newline)
      {
        file_path1=g_utf8_substring((gchar*)selection_string, 0, i-1);
        file_path2=g_filename_from_uri(file_path1, NULL, &error1);
        if(error1!=NULL)
          {
            g_print("Error1 %s\n", error1->message);
            file_string=FALSE;
            g_error_free(error1);
          }
      }
    else
      {
        file_path2=g_filename_from_uri((gchar*)selection_string, NULL, &error1);
        if(error1!=NULL)
          {
            g_print("Error1 %s\n", error1->message);
            file_string=FALSE;
            g_error_free(error1);
          }
      }
    g_print("%s\n", file_path2);

    //Try to get the icon from the file string.
    if(file_string)
      {
        //Check if it is a desktop file.
        GRegex *reg=g_regex_new(".desktop", 0, 0, NULL);
        if(!g_regex_match(reg, file_path2, 0, NULL)) 
          {
            g_print("No Desktop File\n");
            gtk_image_clear(GTK_IMAGE(user_data));
          }
        else
          {  
            //Get the name of the icon in the desktop file if there is one.
            GKeyFile *key_file=g_key_file_new();
            gchar *icon_name=NULL;
            GError *error2=NULL;
            if(g_key_file_load_from_file(key_file, file_path2, G_KEY_FILE_NONE, &error2))
              {
                GError *error3=NULL;
                icon_name=g_key_file_get_string(key_file, "Desktop Entry", "Icon", &error3);
                if(error3!=NULL) g_print("Error3 %s\n", error3->message);
                if(error3!=NULL) g_error_free(error3);
                if(icon_name!=NULL) g_print("Icon Name: %s\n", icon_name);        
              }
            else
              {
                g_print("Error2 %s\n", error2->message);
                g_error_free(error2);
              }   

            //Add the icon to the image widget.
            if(icon_name!=NULL)
              {
                GtkIconTheme *icon_theme=gtk_icon_theme_get_default();
                GtkIconInfo *icon_info=gtk_icon_theme_lookup_icon(icon_theme, icon_name, 256, GTK_ICON_LOOKUP_USE_BUILTIN);
                if(icon_info!=NULL)
                  {
                    g_print("Set Image\n");
                    gtk_image_set_from_file(GTK_IMAGE(user_data), gtk_icon_info_get_filename(icon_info));
                  }
                else
                  {
                    g_print("No Image in Desktop File\n");
                  }
                //Don't free icon_theme.
                if(icon_info!=NULL) g_object_unref(icon_info);
              }

            if(key_file!=NULL) g_key_file_free(key_file);   
            if(icon_name!=NULL) g_free(icon_name);
          }
      }
    else g_print("No File String\n");

    if(selection_string!=NULL) g_free(selection_string);
    if(file_path1!=NULL) g_free(file_path1);
    if(file_path2!=NULL) g_free(file_path2);
  }
int main(int argc, char **argv)
  {
    gtk_init(&argc, &argv);

    GtkTargetList *target_list=gtk_target_list_new(NULL, 0);
    gtk_target_list_add_text_targets(target_list, 0);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    gtk_window_set_title(GTK_WINDOW(window), "GTK+ Drag and Drop");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *button1= gtk_button_new_with_label("Drag Desktop Item Here");
    gtk_widget_set_hexpand(button1, TRUE);
    gtk_drag_dest_set(button1, GTK_DEST_DEFAULT_MOTION|GTK_DEST_DEFAULT_HIGHLIGHT, NULL, 0, GDK_ACTION_COPY);
    gtk_drag_dest_set_target_list(button1, target_list);
    g_signal_connect(button1, "drag-drop", G_CALLBACK(drop_widget), NULL);

    GtkWidget *image=gtk_image_new();
    g_signal_connect(button1, "drag-data-received", G_CALLBACK(data_received), image);
       
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);
   
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }




