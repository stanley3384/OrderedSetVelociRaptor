
/*
    A simple test program using gsettings. Tested on Ubuntu14.04 and GTK3.10. 

    Save the xml at the end to a file named: gsettings1.gschema.xml

    Compile with the path to the xml file: glib-compile-schemas /home/owner/eric/Rectangle4/

    Compile program: gcc -Wall gsettings1.c -o gsettings1 `pkg-config --cflags --libs gtk+-3.0`

    The program uses the current user so there is no need for sudo in this example. If you copy the xml file to /usr/share/glib-2.0/schemas and run glib-compile-schemas you need sudo. A warning, do not copy
the gschemas.compiled file over to that location! If you do, your system won't work and you will have to boot the kernel and use the command prompt to rebuild the gschemas.compiled file in order to get your computer operational again. Don't ask.
    Try starting two instances of the program and see if values change in both instances.

    C. Eric Cashon

*/

#include<gtk/gtk.h>

gchar *path="/home/owner/eric/Rectangle4/";
gchar *schema_id="org.test.gsettings1";

int main(int argc, char **argv)
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GError *error=NULL;
    GSettingsSchemaSource *schema_source=NULL;
    GSettingsSchema *schema=NULL;
    GSettings *settings=NULL;
    schema_source=g_settings_schema_source_new_from_directory(path, g_settings_schema_source_get_default(), FALSE, &error);

    if(error==NULL)
      {
        schema=g_settings_schema_source_lookup(schema_source, schema_id, FALSE);
        settings=g_settings_new_full(schema, NULL, path);
        g_settings_schema_source_unref(schema_source);
        g_settings_schema_unref(schema);
      }
    else g_print("Error: %s\n", error->message);
   
    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "3");
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_widget_set_vexpand(combo1, TRUE);
    if(error==NULL) g_settings_bind(settings, "number", combo1, "active_id", G_SETTINGS_BIND_DEFAULT);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "red");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "green");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "blue");
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_widget_set_vexpand(combo2, TRUE);
    if(error==NULL) g_settings_bind(settings, "color", combo2, "active_id", G_SETTINGS_BIND_DEFAULT);
    else gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);

    if(error!=NULL) g_error_free(error);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;   
  }
/*
<?xml version="1.0" encoding="UTF-8"?>
<schemalist>
  <schema path="/home/owner/eric/Rectangle4/" id="org.test.gsettings1">
    <key name="number" type="s">
      <choices>
        <choice value='1'/>
        <choice value='2'/>
        <choice value='3'/>
      </choices>
      <default>'1'</default>
      <summary>Number</summary>
      <description>Change a number.</description>
    </key>
    <key name="color" type="s">
      <choices>
        <choice value='1'/>
        <choice value='2'/>
        <choice value='3'/>
      </choices>
      <default>'1'</default>
      <summary>Color</summary>
      <description>Change a color.</description>
    </key>
  </schema>
</schemalist>
*/
