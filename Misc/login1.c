
/*
  Test code for a login. Trouble with starting and stopping the main loop. Needs a better solution.

  gcc -Wall login1.c -o login1 `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon
*/


#include<gtk/gtk.h>
#include<string.h>

gboolean valid_login=FALSE;
//The user name and password to match. Case sensitive. 
gchar *admin="admin";
gchar *password="password";

static void close_program()
 {
   printf("Quit Program\n");
   gtk_main_quit();
 }
static void button_clicked(GtkWidget *widget, gpointer data)
 {
   //A few clicks to check the main event loop.
   g_print("Click\n");
 }
static void message_dialog(gchar *message)
 {
   GtkWidget *dialog, *label, *content_area;
   dialog=gtk_dialog_new_with_buttons("Information", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
   gtk_window_set_default_size(GTK_WINDOW(dialog), 300, 100);
   label=gtk_label_new(message);
   content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
   gtk_box_set_spacing(GTK_BOX(content_area), 10);
   gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
   gtk_container_add (GTK_CONTAINER (content_area), label);
   gtk_widget_show_all(dialog);
   gtk_dialog_run(GTK_DIALOG(dialog));
   gtk_widget_destroy(dialog);
 }   
static void login_dialog(GtkWidget *widget, gpointer data)
 {
   GtkWidget *dialog, *grid1, *entry1, *entry2, *label1, *label2, *content_area, *action_area;
   int result;

   dialog=gtk_dialog_new_with_buttons("Login", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
   gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
   gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
     
   label1=gtk_label_new("User Name");
   label2=gtk_label_new("Password");  
         
   entry1=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry1), 20);

   entry2=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry2), 20);
   //For GTK3.6 and above.
   //gtk_entry_set_input_purpose(GTK_ENTRY(entry2), GTK_INPUT_PURPOSE_PASSWORD); 
     
   grid1=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label2, 0, 1, 1, 1);    
   gtk_grid_attach(GTK_GRID(grid1), entry1, 1, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), entry2, 1, 1, 1, 1);    
 
   gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
   gtk_grid_set_column_spacing(GTK_GRID(grid1), 30);

   content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
   action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
   gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
   gtk_container_add(GTK_CONTAINER(content_area), grid1); 
   gtk_container_set_border_width(GTK_CONTAINER(action_area), 10);

   gtk_widget_show_all(dialog);
   result=gtk_dialog_run(GTK_DIALOG(dialog));

   if(result==GTK_RESPONSE_OK)
     {
       const gchar *name1=gtk_entry_get_text(GTK_ENTRY(entry1));
       const gchar *password1=gtk_entry_get_text(GTK_ENTRY(entry2));
       if(strcmp(name1, admin)==0&&strcmp(password, password1)==0)
         {
           g_print("Show Main Window\n");
           valid_login=TRUE;
         }
       else
         {
           valid_login=FALSE;
           gchar *message="Login failed!";
           message_dialog(message);
         }
     }
    
     gtk_widget_destroy(dialog);
 }
int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Login Test");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);
   g_signal_connect(window, "realize", G_CALLBACK(login_dialog), NULL);

   GtkWidget *label1=gtk_label_new("Main Loop");
   gtk_widget_set_hexpand(label1, TRUE);
   gtk_widget_set_vexpand(label1, TRUE);

   GtkWidget *button1=gtk_button_new_with_label("button1");
   gtk_widget_set_hexpand(label1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(button_clicked), NULL); 

   GtkWidget *grid1=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 1, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_widget_show_all(window);

   //Probably not the best way to go about this.
   if(valid_login) gtk_main();

   return 0;
}

