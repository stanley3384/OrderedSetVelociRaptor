
/*
    Tested on Ubuntu14.04 and GTK3.10.

    gcc -Wall hello_world_app1.c -o hello_world_app1 `pkg-config --cflags --libs gtk+-3.0`

    run with: ./hello_world_app1
*/

#include<gtk/gtk.h>
#include<stdio.h>

//Draw an icon with cairo.
static GdkPixbuf* draw_icon();
static void about_dialog(GtkWidget *widget, gpointer data);
//Install icon and app name to desktop.
static void install_desktop(GtkWidget *widget, gpointer data);
static void uninstall_desktop(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Hello World App1");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 300);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    GtkWidget *image=gtk_image_new_from_pixbuf(icon);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Hello World App1");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *button1=gtk_button_new_with_label("Install Desktop");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(install_desktop), icon);

    GtkWidget *button2=gtk_button_new_with_label("Uninstall Desktop");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(button2, "clicked", G_CALLBACK(uninstall_desktop), NULL);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 3, 1, 1);

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
    
    //Paint the background blue.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    //A green world.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_arc(cr, 128, 128, 100, 0, 2*G_PI);
    cairo_fill(cr);
    cairo_stroke(cr);

    //Purple text.
    cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
    cairo_move_to(cr, 32, 138);
    cairo_set_font_size(cr, 34);
    cairo_select_font_face(cr, "arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_show_text(cr, "Hello World");
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
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Hello World App1");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Try without an application object.");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static void install_desktop(GtkWidget *widget, gpointer data)
  {
    /*
      Where to install the desktop and icon. For all users use the following paths.
      You need to run the program with sudo ./hello_world_app1 to have permission to
      write to the following locations.
    */
    //gchar *desktop_path=g_strdup("/usr/share/applications/hello-world-app1.desktop");
    //gchar *icon_path=g_strdup("/usr/share/icons/hicolor/256x256/apps/hello_world_app1.png");

    //Install for the current user. No sudo required.
    gchar *desktop_path=g_strdup_printf("%s/.local/share/applications/hello-world-app1.desktop", g_get_home_dir());
    gchar *icon_path=g_strdup_printf("%s/.local/share/icons/hicolor/48x48/apps/hello_world_app1.png", g_get_home_dir());

    //Save icon to file.
    GError *error1=NULL;
    gdk_pixbuf_save((GdkPixbuf*)data, icon_path, "png", &error1, NULL);
    if(error1!=NULL)
      {
        g_print("Pixbuf Error: %s\n", error1->message);
        g_error_free(error1);
      }

    //Setup desktop strings.
    gchar *current_dir=g_get_current_dir();
    gchar *desktop=g_strdup_printf("[Desktop Entry]\nEncoding=UTF-8\nType=Application\nName=Hello World App1\nIcon=%s\nStartupNotify=true\nExec=%s/hello_world_app1", icon_path, current_dir);

    //Save desktop to file.
    g_print("%s\n", desktop);
    GError *error2=NULL;
    g_file_set_contents(desktop_path, desktop, -1, &error2);
    if(error2!=NULL)
      {
        g_print("Save Desktop Error: %s\n", error2->message);
        g_error_free(error2);
      }
    
    g_free(current_dir);
    g_free(desktop);
    g_free(desktop_path);
    g_free(icon_path);
  }
static void uninstall_desktop(GtkWidget *widget, gpointer data)
  {
    g_print("Uninstall Desktop\n");
    gint error=0;
    //For all users. Requires sudo.
    //gchar *desktop_path=g_strdup("/usr/share/applications/hello-world-app1.desktop");
    //gchar *icon_path=g_strdup("/usr/share/icons/hicolor/256x256/apps/hello_world_app1.png");

    //Install for the current user. No sudo required.
    gchar *desktop_path=g_strdup_printf("%s/.local/share/applications/hello-world-app1.desktop", g_get_home_dir());
    gchar *icon_path=g_strdup_printf("%s/.local/share/icons/hicolor/48x48/apps/hello_world_app1.png", g_get_home_dir());

    //Remove desktop file.
    error=remove(desktop_path);
    if(error==0) g_print("Desktop File Removed\n");
    else g_print("Couldn't Remove Desktop File\n");

    //Remove icon file.
    error=0;
    error=remove(icon_path);
    if(error==0) g_print("Icon File Removed\n");
    else g_print("Couldn't Remove Icon File\n");

    g_free(desktop_path);
    g_free(icon_path);
  }





