
/*
  Test code for a login. To use, create a Passwords table first with the sql_login_ui2.c
  Login Administrator program. 

  gcc -Wall login1.c -o login1 `pkg-config --cflags --libs gtk+-3.0` -lsqlite3 -lmcrypt 

  C. Eric Cashon
*/


#include<gtk/gtk.h>
#include<sqlite3.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<mcrypt.h> 

//Switch for exiting main event loop.
gboolean valid_login=FALSE;
//For mcrypt. They need to match values in sql_login_ui2.c.
char IV[]="AAAAAAAAAAAAAAAA";
char key[]="0123456789abcdef";
int key_len=16; 

//UI functions.
static void close_program();
static void button_clicked(GtkWidget *widget, gpointer data);
static void message_dialog(gchar *message);
static void login_dialog(GtkWidget *widget, gpointer data);
//Encryption functions.
static bool check_login_password(int *admin, const char *login, const char *password);
static int allocate_buffer_block(char **buffer, int length);
static int encrypt_string(void *buffer, int buffer_len);

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
   gtk_widget_set_hexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(button_clicked), NULL); 

   GtkWidget *grid1=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 1, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_widget_show_all(window);

   if(valid_login) gtk_main();

   return 0;
 }
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
       const gchar *user_name=gtk_entry_get_text(GTK_ENTRY(entry1));
       const gchar *password=gtk_entry_get_text(GTK_ENTRY(entry2));
       guint length=gtk_entry_get_text_length(GTK_ENTRY(entry2));
       gchar *buffer=NULL;
       int admin=0;
       bool login_exists=false;
       if(strcmp(password, "password")!=0)
         {
           int buffer_len=allocate_buffer_block(&buffer, length);
           strcpy(buffer, password); 
           encrypt_string(buffer, buffer_len);
           login_exists=check_login_password(&admin, user_name, buffer);
           
         }
       else
         {
           login_exists=check_login_password(&admin, user_name, "password");
         }    

       if(login_exists)
         {
           valid_login=TRUE;
           g_print("Show Main Window\n");
         }
       else
         {
           valid_login=FALSE;
           gchar *message="Login failed!";
           message_dialog(message);
         }
       if(buffer!=NULL) free(buffer);
     }
    
     gtk_widget_destroy(dialog);
 }
static bool check_login_password(int *admin, const char *login, const char *password)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1=sqlite3_mprintf("SELECT admin, login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login, password);

    sqlite3_open("password.db",&cnn);   
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    if(sql_return==SQLITE_ROW)
      {
         login_exists=true;
         *admin=sqlite3_column_int(stmt1, 0);
      }
    sqlite3_finalize(stmt1);   
    sqlite3_close(cnn); 

    sqlite3_free(sql1);
    return login_exists;
  }
static int allocate_buffer_block(char **buffer, int length)
 {
  int i=0;
  int j=0;
   
  //Pad string for a block of 16.
  if(length<16) 
    {
      length=16;
      *buffer=(char*)malloc((length+1) * sizeof(char));
      if(*buffer==NULL)
        {
          printf("Malloc Error\n");
          return 0;
        }
      memset(*buffer, '\0', length+1);
    }
  else
    {
      i=length/16;
      j=length%16;
      if(j>0)
        {
          length=i*16+16;
          *buffer=(char*)malloc((length+1) * sizeof(char));
          if(*buffer==NULL)
            {
              printf("Malloc Error\n");
              return 0;
            }
          memset(*buffer, '\0', length+1);
        }
      else
        {
          length=i*16;
          *buffer=(char*)malloc((length+1) * sizeof(char));
          if(*buffer==NULL)
            {
              printf("Malloc Error\n");
              return 0;
            }
          memset(*buffer, '\0', length+1);
        }
    }
  
  return length;
 }
int encrypt_string(void *buffer, int buffer_len)
 {
   int i=0;
   MCRYPT td = mcrypt_module_open("twofish", NULL, "cfb", NULL);
   //MCRYPT td = mcrypt_module_open("rijndael-128", NULL, "cbc", NULL);
   int blocksize = mcrypt_enc_get_block_size(td);
   if(buffer_len % blocksize != 0)
     {
       printf("Blocksize Error %i %i\n", buffer_len, blocksize);
       return 1;
     }
   i=mcrypt_generic_init(td, key, key_len, IV);
   if(i<0)
     {
       mcrypt_perror(i);
       return 1;
     }
   mcrypt_generic(td, buffer, buffer_len);
   mcrypt_generic_deinit(td);
   mcrypt_module_close(td);
   return 0;
 } 

