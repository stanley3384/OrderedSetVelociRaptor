
/*

    A simple test program using gsettings. Embed the XML into the C code and compile the schema. If
gschemas.compiled isn't in the path location create or recreate it.
    The program uses the current user so there is no need for sudo in this example. If you copy the xml file to /usr/share/glib-2.0/schemas and run glib-compile-schemas you need sudo. A warning, do not copy
the gschemas.compiled file over to that location! If you do, your system won't work and you will have to boot the kernel and use the command prompt to rebuild the gschemas.compiled file there in order to get your computer operational again. Don't ask.
    Try starting two instances of the program and see if values change in both instances. Delete the LOCAL!!!
(and not the system!) gschemas.compiled file and it will get rebuilt automatically at the program start.
    In order to delete entries and files, delete the local xml and gschemas.compiled file. Then
remove the entry in the dconf database using

    dconf reset -f program_dir_path

remove the program and that should just leave the gsettings2.c file left. 

    Tested on Ubuntu14.04 and GTK3.10. 

    Compile program: gcc -Wall gsettings2.c -o gsettings2 `pkg-config --cflags --libs gtk+-3.0`

    C. Eric Cashon

*/

#include<gtk/gtk.h>

gchar *schema_id="org.test.gsettings1";
gchar *xml_file_name="gsettings1.gschema.xml";

static void compile_default_schema(gchar *xml_file_path, gchar *current_path);

int main(int argc, char **argv)
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 200, 100);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

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

    if(error!=NULL) g_error_free(error);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);

    gtk_main();
    return 0;   
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







