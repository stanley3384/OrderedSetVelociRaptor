
/*
    A simple GTK+ test app that has desktop settings, icon and gsettings all combined into one file.
It installs with current user permissions so there is no need for sudo. This is a combination of the gsettings2.c test program with the hello_world_app2.c test program. There is a lot here. Not really
trivial. It does put together some ideas on how to integrate with the GNOME desktop and save settings
for multiple instances of an application along with saving settings when an application is closed.
    There is a trivial GTK+ application example on GNOME developer that breaks everything into individual
pieces. Two ways to look at the same sort of problem.

    Tested on Ubuntu14.04 and GTK3.10.

    gcc -Wall trivial_gtk_app1.c -o trivial_gtk_app1 `pkg-config --cflags --libs gtk+-3.0`

    run with: ./trivial_gtk_app1

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<stdio.h>

//Name the xml schema file and schema id saved in the file.
gchar *schema_id="org.test.gsettings1";
gchar *xml_file_name="gsettings1.gschema.xml";

//Setup the GUI.
static void app_activate(GtkApplication *app, gpointer data);
//Setup the menu.
static void app_startup(GtkApplication *app, gpointer data);
//Draw an icon with cairo.
static GdkPixbuf* draw_icon();
//Draw window color. Change it with combo box.
static gboolean draw_window_background(GtkWidget *widget, cairo_t *cr, gpointer data);
static void redraw_window(GtkWidget *widget, gpointer data);
//Change pango label color.
static void change_pango_label(GtkWidget *widget, gpointer data);
//Info dialog.
static void about_dialog(GSimpleAction *action, GVariant *parameter, gpointer data);
//Install icon and app name to desktop.
static void install_desktop(GtkWidget *widget, gpointer data);
static void uninstall_desktop(GtkWidget *widget, gpointer data);
//Compile the schema for gsettings.
static void compile_default_schema(gchar *xml_file_path, gchar *current_path);

int main(int argc, char *argv[])
  {
    gint status=0;
    GtkApplication *app = gtk_application_new("app.example", G_APPLICATION_FLAGS_NONE);
    g_signal_connect(app, "startup", G_CALLBACK(app_startup), NULL);
    g_signal_connect(app, "activate", G_CALLBACK(app_activate), NULL);
    status=g_application_run(G_APPLICATION(app), argc, argv);
    g_object_unref(app);
    return(status);
  }
static void app_activate(GtkApplication *app, gpointer data)
  {
    GtkWidget *window=gtk_application_window_new(GTK_APPLICATION(app));
    gtk_window_set_title(GTK_WINDOW(window), "Trivial GTK+ App");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 300);

    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    GtkWidget *image=gtk_image_new_from_pixbuf(icon);

    GtkWidget *button1=gtk_button_new_with_label("Install Desktop");
    gtk_widget_set_hexpand(button1, TRUE);
    g_signal_connect(button1, "clicked", G_CALLBACK(install_desktop), icon);

    GtkWidget *button2=gtk_button_new_with_label("Uninstall Desktop");
    gtk_widget_set_hexpand(button2, TRUE);
    g_signal_connect(button2, "clicked", G_CALLBACK(uninstall_desktop), NULL);

    //Check if gschemas.compiled file is in the path. If it isn't, build it.
    gchar *current_dir=g_get_current_dir();
    gchar *current_path=g_strdup_printf("%s/", current_dir);
    gchar *compiled_file_path=g_strdup_printf("%sgschemas.compiled", current_path);
    gchar *xml_file_path=g_strdup_printf("%s%s", current_path, xml_file_name);
    if(!g_file_test(compiled_file_path, G_FILE_TEST_EXISTS)) compile_default_schema(xml_file_path, current_path);
    g_free(current_dir);
    g_free(compiled_file_path);
    g_free(xml_file_path);

    //Get the local schema. 
    GError *error=NULL;
    GSettingsSchemaSource *schema_source=NULL;
    GSettingsSchema *schema=NULL;
    GSettings *settings=NULL;
    schema_source=g_settings_schema_source_new_from_directory(current_path, g_settings_schema_source_get_default(), FALSE, &error);
    if(error==NULL)
      {
        schema=g_settings_schema_source_lookup(schema_source, schema_id, FALSE);
        if(schema!=NULL) settings=g_settings_new_full(schema, NULL, current_path);
        else g_print("Error: Invalid Schema\n");
        g_settings_schema_source_unref(schema_source);
      }
    else g_print("Error: %s\n", error->message);
    g_free(current_path);
   
    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "3");
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_widget_set_vexpand(combo1, TRUE);
    if(error==NULL&&schema!=NULL)
      {
        g_settings_bind(settings, "number", combo1, "active_id", G_SETTINGS_BIND_DEFAULT);
        g_settings_schema_unref(schema);
      }
    else gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);

    GtkWidget *label1=gtk_label_new(NULL);
    gint active=gtk_combo_box_get_active(GTK_COMBO_BOX(combo1));
    if(active==0) gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='yellow' weight='bold' font='20'>Saved Settings</span>");
    else if(active==1) gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='purple' weight='bold' font='20'>Saved Settings</span>");
    else gtk_label_set_markup(GTK_LABEL(label1), "<span foreground='cyan' weight='bold' font='20'>Saved Settings</span>");
    gtk_widget_set_hexpand(label1, TRUE);
    g_signal_connect(combo1, "changed", G_CALLBACK(change_pango_label), label1);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "red");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "green");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "blue");
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_widget_set_vexpand(combo2, TRUE);
    if(error==NULL&&schema!=NULL)
      {
        g_settings_bind(settings, "color", combo2, "active_id", G_SETTINGS_BIND_DEFAULT);
        g_settings_schema_unref(schema);
      }
    else gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
    g_signal_connect(combo2, "changed", G_CALLBACK(redraw_window), window);

    if(error!=NULL) g_error_free(error);

    gtk_widget_set_app_paintable(window, TRUE);
    g_signal_connect(window, "draw", G_CALLBACK(draw_window_background), combo2);

    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 10);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_attach(GTK_GRID(grid), image, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 5, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);
  }
static void app_startup(GtkApplication *app, gpointer data)
 {
    //Setup menu callback.
    GSimpleAction *about = g_simple_action_new("about", NULL);
    g_signal_connect(about, "activate", G_CALLBACK(about_dialog), app);
    g_action_map_add_action(G_ACTION_MAP(app), G_ACTION(about));
    g_object_unref(about);

    //Setup menu.
    GMenu *menu = g_menu_new();
    GMenu *submenu = g_menu_new();
    g_menu_append_submenu(menu, "About", G_MENU_MODEL(submenu));
    GMenu *section = g_menu_new();
    g_menu_append_section(submenu, NULL, G_MENU_MODEL(section));
    g_menu_append(section, "Trivial GTK+ App", "app.about");
    g_object_unref(submenu);
    g_object_unref(section);

    gtk_application_set_menubar(GTK_APPLICATION(app), G_MENU_MODEL(menu));
    g_object_unref(menu);
  }
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    
    //Paint the background blue.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    //A blue world.
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
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
static gboolean draw_window_background(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==0) cairo_set_source_rgb(cr, 1.0, 0.0, 0.0);
    else if(gtk_combo_box_get_active(GTK_COMBO_BOX(data))==1) cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    else cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_paint(cr);
    return FALSE;
  }
static void redraw_window(GtkWidget *widget, gpointer data)
  {
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void change_pango_label(GtkWidget *widget, gpointer data)
  {
    g_print("Change Pango\n");
    if(gtk_combo_box_get_active(GTK_COMBO_BOX(widget))==0) gtk_label_set_markup(GTK_LABEL(data), "<span foreground='yellow' weight='bold' font='20'>Saved Settings</span>");
    else if(gtk_combo_box_get_active(GTK_COMBO_BOX(widget))==1) gtk_label_set_markup(GTK_LABEL(data), "<span foreground='purple' weight='bold' font='20'>Saved Settings</span>");
    else gtk_label_set_markup(GTK_LABEL(data), "<span foreground='cyan' weight='bold' font='20'>Saved Settings</span>");
  }
static void about_dialog(GSimpleAction *action, GVariant *parameter, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    //Null will add the program icon to the logo.
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Trivial GTK+ App");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Setup a basic application.");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static void install_desktop(GtkWidget *widget, gpointer data)
  {
    //Install paths for the current user. No sudo required.
    gchar *desktop_path=g_strdup_printf("%s/.local/share/applications/trivial_gtk_app1.desktop", g_get_home_dir());
    gchar *icon_path=g_strdup_printf("%s/.local/share/icons/hicolor/48x48/apps/trivial_gtk_app1.png", g_get_home_dir());

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
    gchar *desktop=g_strdup_printf("[Desktop Entry]\nEncoding=UTF-8\nType=Application\nName=Trivial GTK+ App\nIcon=%s\nStartupNotify=true\nExec=%s/trivial_gtk_app1", icon_path, current_dir);

    //Save desktop to file
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

    //Install for the current user. No sudo required.
    gchar *desktop_path=g_strdup_printf("%s/.local/share/applications/trivial_gtk_app1.desktop", g_get_home_dir());
    gchar *icon_path=g_strdup_printf("%s/.local/share/icons/hicolor/48x48/apps/trivial_gtk_app1.png", g_get_home_dir());

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
static void compile_default_schema(gchar *xml_file_path, gchar *current_path)
  {
    gchar *xml_string=g_strdup_printf("<?xml version='1.0' encoding='UTF-8'?>\n"
                                      "<schemalist>\n"
                                      "  <schema path='%s' id='%s'>\n"
                                      "    <key name='number' type='s'>\n"
                                      "      <choices>\n"
                                      "        <choice value='1'/>\n"
                                      "        <choice value='2'/>\n"
                                      "        <choice value='3'/>\n"
                                      "      </choices>\n"
                                      "      <default>'1'</default>\n"
                                      "      <summary>Number</summary>\n"
                                      "      <description>Change a number.</description>\n"
                                      "    </key>\n"
                                      "    <key name='color' type='s'>\n"
                                      "      <choices>\n"
                                      "        <choice value='1'/>\n"
                                      "        <choice value='2'/>\n"
                                      "        <choice value='3'/>\n"
                                      "      </choices>\n"
                                      "      <default>'1'</default>\n"
                                      "      <summary>Color</summary>\n"
                                      "      <description>Change a color.</description>\n"
                                      "    </key>\n"
                                      "  </schema>\n"
                                      "</schemalist>\n", current_path, schema_id);
    //g_print("%s\n", xml_string);

    //Write XML file.
    GError *error1=NULL;
    g_file_set_contents(xml_file_path, xml_string, -1, &error1);
    if(error1!=NULL)
      {
        g_print("File Write Error: %s\n", error1->message);
        g_error_free(error1);
      }

    //Compile XML file.
    gchar *standard_output=NULL;
    gchar *standard_error=NULL;
    gint exit_status=0;
    GError *error2=NULL;
    gchar *command_line=g_strdup_printf("glib-compile-schemas %s", current_path);
    g_print("Compile Schema: %s\n", command_line);
    g_spawn_command_line_sync(command_line, &standard_output, &standard_error, &exit_status, &error2);
    g_print("Command Line Return: %i %s %s\n", exit_status, standard_output, standard_error);
    if(error2!=NULL)
      {
        g_print("XML Compile Error: %s\n", error2->message);
        g_error_free(error2);
      }
    g_free(command_line);
    if(standard_output!=NULL) g_free(standard_output);
    if(standard_error!=NULL) g_free(standard_error);

    g_free(xml_string);
  }

