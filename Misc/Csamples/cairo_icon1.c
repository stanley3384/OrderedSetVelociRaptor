
/*
    Test code for drawing a cairo icon on program startup. The icon is then used for the icon
in the launcher and the icon in the about dialog. This way there isn't a seperate icon file 
to go with the program. It is already part of the program.

    Compiled with Ubuntu14.04 and GTK3.10.

    gcc -Wall cairo_icon1.c -o cairo_icon1 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static GdkPixbuf* draw_icon();
static void about_dialog(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Cairo Icon");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    GtkWidget *image=gtk_image_new_from_pixbuf(icon);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Cairo Icon");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;   
  }
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    
    //Paint the background.
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_paint(cr);

    //A green icon border.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, 256, 256);
    cairo_stroke(cr);

    //Smiley
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 10);
    //Circle
    cairo_arc(cr, 128, 128, 100, 0, 2*G_PI);
    cairo_stroke(cr);
    //Left eye.
    cairo_arc(cr, 128-35, 90, 7, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke(cr);
    //Right eye.
    cairo_arc(cr, 128+35, 90, 7, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke(cr);
    //Smile
    cairo_arc(cr, 128, 130, 60, 0, G_PI);
    cairo_stroke(cr);

    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface, 0, 0, 256, 256);

    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    return icon;
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    //Null will add the program icon to the logo.
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Cairo Icon");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Create a program icon with Cairo.");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }





