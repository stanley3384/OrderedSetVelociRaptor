
/*

    Test code for drag and drop along with storing and retrieving data using json and a checksum. 
You can drag and drop the button widgets to the labels and upon exit the grid locations of the 
widgets are saved. A checksum is used to help with the case of someone changing the data in the
json file. If the data gets changed the program falls back to the default values in the grid_coord
array. From the discussion titled "Using glib to create config file" on the GTK+ Forum.

    gcc -Wall json4.c -o json4 -I/usr/include/json-glib-1.0 `pkg-config --cflags --libs gtk+-3.0` -ljson-glib-1.0 

    C. Eric Cashon
*/

#include <gtk/gtk.h>
#include <json-glib/json-glib.h>
#include <stdlib.h>

enum{TARGET_INT32, TARGET_STRING, TARGET_ROOTWIN};
gint grid_coord[12]={0,0,0,1,0,2,1,0,1,1,1,2};

static void get_program_values()
 {
   gint i=0;
   GError *error=NULL;
   gchar *checksum=NULL;
   const gchar *check_hash;
   gint json_grid[12];
   GString *string=g_string_new("");
   if(g_file_test("data4.json", G_FILE_TEST_EXISTS)) 
     {
       //Get the data from the file.
       JsonParser *parser = json_parser_new();
       json_parser_load_from_file(parser, "data4.json", &error);
       if(error)
         {
           g_print("Error loading file. Use default array.\n");
           g_object_unref(parser);
         }
       else
         {
           JsonReader *reader = json_reader_new(json_parser_get_root(parser));
           json_reader_read_member(reader, "checksum");
           check_hash=json_reader_get_string_value(reader);
           json_reader_end_member(reader);
           json_reader_read_member(reader, "grid_coord");
           for(i=0;i<12;i++)
             {
               json_reader_read_element(reader, i);
               json_grid[i]=json_reader_get_int_value(reader);
               g_string_append_printf(string, "%i", json_grid[i]);
               json_reader_end_element(reader);
             }
           json_reader_end_member(reader);
           checksum=g_compute_checksum_for_string(G_CHECKSUM_SHA1, string->str, string->len);
           g_print("Grid string load %s\n", string->str);
           g_print("Checksum  %s\nCheckHash %s\n", checksum, check_hash);
           if(g_strcmp0(checksum, check_hash)==0)
             {
               for(i=0;i<12;i++)
                 {
                   grid_coord[i]=json_grid[i];
                 }
             }
           else
             {
               g_print("File was tampered with. Use default array.\n");
             }
           g_object_unref(parser);
           g_object_unref(reader);
         }
     }
   else
     {
       g_print("Use Default Array\n");
     }
 
   if(error!=NULL)g_error_free(error);
   g_string_free(string, TRUE);
   if(checksum!=NULL)g_free(checksum);
 }
static void save_program_values(GtkWidget *widget, gpointer data)
 {
   //Load the data.
   gint i=0;
   gchar *checksum=NULL;
   GString *string=g_string_new("");
   JsonBuilder *builder=json_builder_new();
   json_builder_begin_object(builder);
   json_builder_set_member_name(builder, "grid_coord");
   json_builder_begin_array(builder);
   for(i=0;i<12;i++)
     {
       json_builder_add_int_value(builder, grid_coord[i]);
       g_string_append_printf(string, "%i", grid_coord[i]);
     }
   json_builder_end_array(builder);
   checksum=g_compute_checksum_for_string(G_CHECKSUM_SHA1, string->str, string->len);
   g_print("Grid string save %s\n", string->str);
   json_builder_set_member_name(builder, "checksum");
   json_builder_add_string_value (builder, checksum);
   json_builder_end_object(builder);

   //Save the data to the file.
   JsonGenerator *generator = json_generator_new();
   JsonNode *root = json_builder_get_root(builder);
   json_generator_set_root(generator, root);
   json_generator_to_file(generator, "data4.json", NULL);

   json_node_free(root);
   g_object_unref(builder);
   g_object_unref(generator);
   g_string_free(string, TRUE);
   if(checksum!=NULL)g_free(checksum);

   gtk_main_quit();
 }
static gboolean drop_widget(GtkWidget *widget, GdkDragContext *context, gint x, gint y, guint time, GtkWidget *grid)
 {
   GtkWidget *button=gtk_drag_get_source_widget(context);
   g_print("Widget %s dropped on %s\n", gtk_widget_get_name(button), gtk_widget_get_name(widget));
   gint id1=atoi(gtk_widget_get_name(button));
   gint id2=atoi(gtk_widget_get_name(widget));
   gint r1=grid_coord[2*id1];
   gint c1=grid_coord[2*id1+1];
   gint r2=grid_coord[2*id2];
   gint c2=grid_coord[2*id2+1];
   g_print("id %i button %i %i id %i label %i %i\n", id1, r1, c1, id2, r2, c2);
   //Swap widgets.
   g_object_ref(button);
   g_object_ref(widget);
   gtk_container_remove(GTK_CONTAINER(grid), button);
   gtk_container_remove(GTK_CONTAINER(grid), widget);
   gtk_grid_attach(GTK_GRID(grid), button, r2, c2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), widget, r1, c1, 1, 1);
   gtk_widget_show(button);
   gtk_widget_show(widget); 
   //Swap coordinates.
   grid_coord[2*id1]=r2;
   grid_coord[2*id1+1]=c2;
   grid_coord[2*id2]=r1;
   grid_coord[2*id2+1]=c1;
  
   return TRUE;
 }
int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkTargetEntry target_list[] = {{"INTEGER", 0, TARGET_INT32},{"STRING", 0, TARGET_STRING},{"text/plain", 0, TARGET_STRING }, {"application/x-rootwindow-drop", 0, TARGET_ROOTWIN}};

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_default_size(GTK_WINDOW(window), 350, 100);
   gtk_window_set_title(GTK_WINDOW(window), "JSON with GTK+ Drag and Drop");

   GtkWidget *button1 = gtk_button_new_with_label("button1");
   GtkWidget *button2 = gtk_button_new_with_label("button2");
   GtkWidget *button3 = gtk_button_new_with_label("button3");
   gtk_widget_set_name(button1, "0");
   gtk_widget_set_name(button2, "1");
   gtk_widget_set_name(button3, "2");
   gtk_drag_source_set(button1, GDK_BUTTON1_MASK, target_list, 4, GDK_ACTION_COPY);
   gtk_drag_source_set(button2, GDK_BUTTON1_MASK, target_list, 4, GDK_ACTION_COPY);
   gtk_drag_source_set(button3, GDK_BUTTON1_MASK, target_list, 4, GDK_ACTION_COPY);

   GtkWidget *label1= gtk_label_new("label1");
   GtkWidget *label2= gtk_label_new("label2");
   GtkWidget *label3= gtk_label_new("label3");
   gtk_widget_set_name(label1, "3");
   gtk_widget_set_name(label2, "4");
   gtk_widget_set_name(label3, "5");
   gtk_drag_dest_set(label1, GTK_DEST_DEFAULT_MOTION|GTK_DEST_DEFAULT_HIGHLIGHT, target_list, 4, GDK_ACTION_COPY);
   gtk_drag_dest_set(label2, GTK_DEST_DEFAULT_MOTION|GTK_DEST_DEFAULT_HIGHLIGHT, target_list, 4, GDK_ACTION_COPY);
   gtk_drag_dest_set(label3, GTK_DEST_DEFAULT_MOTION|GTK_DEST_DEFAULT_HIGHLIGHT, target_list, 4, GDK_ACTION_COPY);

   GtkWidget *grid=gtk_grid_new();
   g_signal_connect(label1, "drag-drop", G_CALLBACK(drop_widget), grid);
   g_signal_connect(label2, "drag-drop", G_CALLBACK(drop_widget), grid);
   g_signal_connect(label3, "drag-drop", G_CALLBACK(drop_widget), grid);

   g_signal_connect(window, "destroy", G_CALLBACK(save_program_values), NULL);
   get_program_values();
       
   gtk_grid_set_row_homogeneous(GTK_GRID(grid), TRUE);
   gtk_grid_set_column_homogeneous(GTK_GRID(grid), TRUE);
   gtk_container_add(GTK_CONTAINER(window), grid);
   gtk_grid_attach(GTK_GRID(grid), button1, grid_coord[0], grid_coord[1], 1, 1); 
   gtk_grid_attach(GTK_GRID(grid), button2, grid_coord[2], grid_coord[3], 1, 1); 
   gtk_grid_attach(GTK_GRID(grid), button3, grid_coord[4], grid_coord[5], 1, 1); 
   gtk_grid_attach(GTK_GRID(grid), label1, grid_coord[6], grid_coord[7], 1, 1); 
   gtk_grid_attach(GTK_GRID(grid), label2, grid_coord[8], grid_coord[9], 1, 1); 
   gtk_grid_attach(GTK_GRID(grid), label3, grid_coord[10], grid_coord[11], 1, 1); 
   gtk_widget_show_all(window);

   gtk_main();

   return 0;
  }



