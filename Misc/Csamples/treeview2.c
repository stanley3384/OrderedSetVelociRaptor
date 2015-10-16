
/*
    Test code to see how transitioning a treeview row color looks.

    With Ubuntu14.04 and GTK3.10.
    gcc -Wall treeview2.c -o treeview2 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/
#include<gtk/gtk.h>

enum
{
   ID,
   PROGRAM,
   IMAGE,
   COLUMNS
};

static gboolean fade_color(gpointer data)
  {
    static int i=0;
    GdkRGBA color;
    gboolean check=TRUE;

    if(i==0) gdk_rgba_parse(&color, "rgba(255, 0, 255, 255)");
    else if(i==1) gdk_rgba_parse(&color, "rgba(255, 25, 225, 255)");
    else if(i==2) gdk_rgba_parse(&color, "rgba(255, 50, 200, 255)");
    else if(i==3) gdk_rgba_parse(&color, "rgba(255, 75, 175, 255)");
    else if(i==4) gdk_rgba_parse(&color, "rgba(255, 100, 150, 255)");
    else if(i==5) gdk_rgba_parse(&color, "rgba(255, 125, 125, 255)");
    else if(i==6) gdk_rgba_parse(&color, "rgba(255, 150, 100, 255)");
    else if(i==7) gdk_rgba_parse(&color, "rgba(255, 175, 75, 255)");
    else if(i==8) gdk_rgba_parse(&color, "rgba(255, 200, 50, 255)");
    else if(i==8) gdk_rgba_parse(&color, "rgba(255, 225, 25, 255)");
    else
      {
        gdk_rgba_parse(&color, "rgba(255, 255, 0, 255)");
        check=FALSE;
      }

    gtk_widget_override_background_color(GTK_WIDGET(data), GTK_STATE_FLAG_SELECTED, &color);
    g_print("Fade %i\n", i);
    i++;
    if(!check) i=0;

    return check;
  }
static void changed_row(GtkTreeSelection *treeselection, gpointer data)
  {
    g_timeout_add(10, (GSourceFunc)fade_color, data);   
  }
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Tree View");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkTreeIter iter;
    GtkListStore *store = gtk_list_store_new(COLUMNS, G_TYPE_UINT, G_TYPE_STRING, G_TYPE_STRING);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, ID, 0, PROGRAM, "Gedit", IMAGE, "Image1",  -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, ID, 1, PROGRAM, "Gimp", IMAGE,  "Image2", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, ID, 2, PROGRAM, "Inkscape", IMAGE, "image3", -1);
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter, ID, 3, PROGRAM, "Firefox", IMAGE, "Image4", -1);

    GtkTreeModel *sortmodel=gtk_tree_model_sort_new_with_model(GTK_TREE_MODEL(store));

    GtkWidget *tree = gtk_tree_view_new_with_model(sortmodel);
    gtk_tree_view_set_hover_selection(GTK_TREE_VIEW(tree), TRUE);
    gtk_widget_set_hexpand(tree, TRUE);
    gtk_widget_set_name(tree, "tree");

    GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(tree));
    gtk_tree_selection_set_mode(selection, GTK_SELECTION_BROWSE);
    g_signal_connect(selection, "changed", G_CALLBACK(changed_row), tree); 

    GdkRGBA color1;
    gdk_rgba_parse(&color1, "rgba(255,0,255,255)");
    GtkCellRenderer *renderer1 = gtk_cell_renderer_text_new();
    g_object_set(renderer1, "xalign", 0.5, "editable", TRUE, "cell-background-rgba", &color1, NULL);
   
    GtkTreeViewColumn *column1 = gtk_tree_view_column_new_with_attributes("ID", renderer1, "text", ID, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column1);    
    GtkTreeViewColumn *column2 = gtk_tree_view_column_new_with_attributes("Program", renderer1, "text", PROGRAM, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column2);
    GtkTreeViewColumn *column3 = gtk_tree_view_column_new_with_attributes("Image", renderer1, "text", IMAGE, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column3);

    GtkWidget *button=gtk_button_new_with_label("Fade");
    gtk_widget_set_hexpand(button, TRUE);
    gtk_widget_set_vexpand(button, TRUE);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), tree, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 1, 1, 1);
    gtk_container_add(GTK_CONTAINER(window), grid);

    GError *css_error=NULL;
    gchar css_string[]="GtkButton:hover {transition: 500ms ease-in; background: purple;} GtkWidget#tree{background: purple; color: black}";
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
