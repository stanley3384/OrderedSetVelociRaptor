
/*
  Test code for a login UI. Uses mcrypt and two fish encryption for the passwords stored in the database.
  Initialize table first then you can add some names and passwords. This uses a blob to store the byte
  data in the table so if you have used the sql_login_ui1 program drop and re-initialize the table.

Get the mcrypt dev library
  apt-get install libmcrypt-dev

  gcc -Wall sql_login_ui2.c -o sql_login_ui2 `pkg-config --cflags --libs gtk+-3.0` -lsqlite3 -lmcrypt 

  C. Eric Cashon
*/


#include<gtk/gtk.h>
#include<sqlite3.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<stdbool.h>
#include<mcrypt.h> 

//For mcrypt.
char IV[]="AAAAAAAAAAAAAAAA";
char key[]="0123456789abcdef";
int key_len=16; 

typedef struct
  {
    GtkWidget *entry1, *entry2, *combo1, *label3;
  } Widgets;

//UI functions.
static void close_program(void);
static void run_database_command(GtkWidget *widget, Widgets *w);
static void message_dialog(gchar *message);
static void update_login_dialog(gchar *user_name, gchar *password, Widgets *w);
//Encryption functions.
static int allocate_buffer_block(char **buffer, int length);
static int encrypt_string(void *buffer, int buffer_len);
static int decrypt_string(void *buffer, int buffer_len);
//Database functions.
static void print_passwords_table(void);
static bool initialize_admin_login_password(void);
static bool check_login_password(int *admin, const char *login, const char *password);
static bool insert_login_password(int admin, const char *login, const char *password);
static bool delete_login_password(const char *login, const char *password);
static bool update_login_password(const char *login_old, const char *password_old, const char *login_new, const char *password_new);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Login Administrator");
   gtk_window_set_default_size(GTK_WINDOW(window), 300, 200);
   gtk_container_set_border_width(GTK_CONTAINER(window), 15);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   GtkWidget *label1=gtk_label_new("User Name");
   GtkWidget *label2=gtk_label_new("Password");  
         
   GtkWidget *entry1=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry1), 20);

   GtkWidget *entry2=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry2), 20);

   GtkWidget *combo1=gtk_combo_box_text_new();     
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "Create Administrator Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "Create User Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "Update Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "3", "Delete Login");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "4", "Login Exists");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "5", "Initialize Passwords Table");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "6", "Print Passwords Table");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 1);

   GtkWidget *label3=gtk_label_new("Two Fish Encryption");
   gtk_widget_set_hexpand(label1, TRUE);
   gtk_widget_set_vexpand(label1, TRUE);

   Widgets *w;
   w=g_slice_new(Widgets);
   w->entry1=GTK_WIDGET(entry1);
   w->entry2=GTK_WIDGET(entry2);
   w->combo1=GTK_WIDGET(combo1);
   w->label3=GTK_WIDGET(label3);
   GtkWidget *button1=gtk_button_new_with_label("Run");
   g_signal_connect(button1, "clicked", G_CALLBACK(run_database_command), (gpointer) w);  

   GtkWidget *grid1=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label2, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), entry1, 1, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), entry2, 1, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), combo1, 0, 2, 2, 1);
   gtk_grid_attach(GTK_GRID(grid1), button1, 0, 3, 2, 1);
   gtk_grid_attach(GTK_GRID(grid1), label3, 0, 4, 2, 2);
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;
}
static void close_program(void)
 {
   printf("Quit Program\n");
   gtk_main_quit();
 }
static void run_database_command(GtkWidget *widget, Widgets *w)
 {
   gboolean login_exists=FALSE;
   gint admin=0;
   gchar *buffer=NULL;
   gint buffer_len=0;
   guint length1=gtk_entry_get_text_length(GTK_ENTRY(w->entry1));
   guint length2=gtk_entry_get_text_length(GTK_ENTRY(w->entry2));
   gchar *user_name=g_strdup(gtk_entry_get_text(GTK_ENTRY(w->entry1)));
   gchar *password=g_strdup(gtk_entry_get_text(GTK_ENTRY(w->entry2)));
   gint combo_id=atoi(gtk_combo_box_get_active_id(GTK_COMBO_BOX(w->combo1)));
  
   //g_print("%i %s %i %s %i\n", combo_id, user_name, length1, password, length2);
   if(combo_id<5)
     {
       if(length1<=2||length2<=4)
         {
           g_print("The user_name > 2 chars and password > 4 chars\n");
           message_dialog("The user_name > 2 chars and password > 4 chars\n");
         }
       else
         {
           switch(combo_id)
            {
              case 0: //Add admin.
                if(strcmp(password, "password")!=0)
                  {
                    buffer_len=allocate_buffer_block(&buffer, length2);
                    strcpy(buffer, password); 
                    encrypt_string(buffer, buffer_len);
                  }
                login_exists=insert_login_password(1, user_name, buffer);
                if(!login_exists) gtk_label_set_text(GTK_LABEL(w->label3), "New Administrator Created");
                else gtk_label_set_text(GTK_LABEL(w->label3), "Couldn't Create Administrator");
                break;
              case 1: //Add user.
                if(strcmp(password, "password")!=0)
                  {
                    buffer_len=allocate_buffer_block(&buffer, length2);
                    strcpy(buffer, password); 
                    encrypt_string(buffer, buffer_len);
                  }
                login_exists=insert_login_password(0, user_name, buffer);
                if(!login_exists) gtk_label_set_text(GTK_LABEL(w->label3), "New Login Created");
                else gtk_label_set_text(GTK_LABEL(w->label3), "Couldn't Create New Login");
                break;
              case 2: //Update login
                if(strcmp(password, "password")!=0)
                  {
                    buffer_len=allocate_buffer_block(&buffer, length2);
                    strcpy(buffer, password); 
                    encrypt_string(buffer, buffer_len);
                    login_exists=check_login_password(&admin, user_name, buffer);
                  }
                else
                  {
                    login_exists=check_login_password(&admin, user_name, "password");
                  }
                if(!login_exists)
                  {
                    gtk_label_set_text(GTK_LABEL(w->label3), "Login Doesn't Exist");
                  }
                else
                  {
                    if(strcmp(password, "password")!=0)
                      {
                        decrypt_string(buffer, buffer_len);
                        update_login_dialog(user_name, buffer, w);
                      }
                    else
                      {
                        update_login_dialog(user_name, password, w);
                      }
                  }
                break;
              case 3: //Delete login.
                if(strcmp(password, "password")!=0)
                  {
                    buffer_len=allocate_buffer_block(&buffer, length2);
                    strcpy(buffer, password); 
                    encrypt_string(buffer, buffer_len);
                  }
                login_exists=delete_login_password(user_name, buffer);
                if(login_exists) gtk_label_set_text(GTK_LABEL(w->label3), "Login Deleted");
                else gtk_label_set_text(GTK_LABEL(w->label3), "Couldn't Delete Login");
                break;
              case 4: //Check if login exists.
                if(strcmp(password, "password")!=0)
                  {
                    buffer_len=allocate_buffer_block(&buffer, length2);
                    strcpy(buffer, password); 
                    encrypt_string(buffer, buffer_len);
                    login_exists=check_login_password(&admin, user_name, buffer);
                  }
                else
                  {
                    login_exists=check_login_password(&admin, user_name, "password");
                  }
                if(login_exists) gtk_label_set_text(GTK_LABEL(w->label3), "Login Exists");
                else gtk_label_set_text(GTK_LABEL(w->label3), "Login Doesn't Exist");
                break;
            }
        }
     }

   if(combo_id==5) //Initialize table.
     {
       login_exists=initialize_admin_login_password();
       if(login_exists) gtk_label_set_text(GTK_LABEL(w->label3), "New Table Created");
       else gtk_label_set_text(GTK_LABEL(w->label3), "Table Already Exists");
     }
   if(combo_id==6) print_passwords_table(); 

   g_free(user_name);
   g_free(password);
   if(buffer!=NULL) g_free(buffer);
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
static void update_login_dialog(gchar *user_name, gchar *password, Widgets *w)
 {
   GtkWidget *dialog, *grid1, *entry1, *entry2, *entry3, *entry4, *label1, *label2, *label3, *label4, *content_area, *action_area;
   int result;

   dialog=gtk_dialog_new_with_buttons("Login", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
   gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
   gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
     
   label1=gtk_label_new("User Name");
   label2=gtk_label_new("Password"); 
   label3=gtk_label_new("New User Name");
   label4=gtk_label_new("New Password");   
         
   entry1=gtk_entry_new();
   gtk_entry_set_text(GTK_ENTRY(entry1), user_name);
   gtk_entry_set_width_chars(GTK_ENTRY(entry1), 20);

   entry2=gtk_entry_new();
   gtk_entry_set_text(GTK_ENTRY(entry2), password);
   gtk_entry_set_width_chars(GTK_ENTRY(entry2), 20);
   //For GTK3.6 and above.
   //gtk_entry_set_input_purpose(GTK_ENTRY(entry2), GTK_INPUT_PURPOSE_PASSWORD);

   entry3=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry3), 20);

   entry4=gtk_entry_new();
   gtk_entry_set_width_chars(GTK_ENTRY(entry4), 20); 
     
   grid1=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label2, 0, 1, 1, 1); 
   gtk_grid_attach(GTK_GRID(grid1), label3, 0, 2, 1, 1);   
   gtk_grid_attach(GTK_GRID(grid1), label4, 0, 3, 1, 1);      
   gtk_grid_attach(GTK_GRID(grid1), entry1, 1, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), entry2, 1, 1, 1, 1); 
   gtk_grid_attach(GTK_GRID(grid1), entry3, 1, 2, 1, 1);  
   gtk_grid_attach(GTK_GRID(grid1), entry4, 1, 3, 1, 1);     
 
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
       gchar *buffer1=NULL;
       gchar *buffer2=NULL;
       gint buffer_len1=0;
       gint buffer_len2=0;
       gboolean check=FALSE;
       guint length1=gtk_entry_get_text_length(GTK_ENTRY(entry3));
       guint length2=gtk_entry_get_text_length(GTK_ENTRY(entry4));
       guint length3=gtk_entry_get_text_length(GTK_ENTRY(entry2));
       if(length1<=2||length2<=4)
         {
           g_print("The new user_name > 2 chars and new password > 4 chars\n");
           message_dialog("The new user_name > 2 chars and new password > 4 chars\n");
         }
       else
         {
           gchar *new_user_name=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry3)));
           gchar *new_password=g_strdup(gtk_entry_get_text(GTK_ENTRY(entry4)));
           if(strcmp(password, "password")!=0)
             {
               buffer_len1=allocate_buffer_block(&buffer1, length2);
               strcpy(buffer1, password); 
               encrypt_string(buffer1, buffer_len1);
               buffer_len2=allocate_buffer_block(&buffer2, length3);
               strcpy(buffer2, new_password); 
               encrypt_string(buffer2, buffer_len2);
               check=update_login_password(user_name, buffer1, new_user_name, buffer2);
              }
           else
              {
                buffer_len2=allocate_buffer_block(&buffer2, length3);
                strcpy(buffer2, new_password); 
                encrypt_string(buffer2, buffer_len2);
                check=update_login_password(user_name, password, new_user_name, buffer2);
              }
           if(check) gtk_label_set_text(GTK_LABEL(w->label3), "Login Updated");
           else gtk_label_set_text(GTK_LABEL(w->label3), "Couldn't Update Login");
           g_free(new_user_name);
           g_free(new_password);
           if(buffer1!=NULL) g_free(buffer1);
           if(buffer2!=NULL) g_free(buffer2);
         }
     }
    
     gtk_widget_destroy(dialog);
 }   
//Encryption code.
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
int decrypt_string(void *buffer, int buffer_len)
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
   mdecrypt_generic(td, buffer, buffer_len);
   mcrypt_generic_deinit(td);
   mcrypt_module_close(td);
   return 0;
 } 
//Database code.
static void print_passwords_table(void)
  {
    int sql_return=0;
    int length=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1="SELECT admin, login, password FROM Passwords;";

    printf("Password Table\n");
    sqlite3_open("password.db",&cnn);   
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    while(sql_return==SQLITE_ROW)
      {
        length=sqlite3_column_bytes(stmt1, 2);
        if(length>1)
          {
            char *buffer=NULL;
            int buffer_len=allocate_buffer_block(&buffer, length); 
            memcpy(buffer, (char*)sqlite3_column_blob(stmt1, 2), buffer_len);            
            if(memcmp(buffer, "password", 8)!=0)
              {
                decrypt_string(buffer, buffer_len);
                printf("  %i %s %s\n", sqlite3_column_int(stmt1, 0), sqlite3_column_text(stmt1, 1), buffer);
              }
            else
              {
                printf("  %i %s %s\n", sqlite3_column_int(stmt1, 0), sqlite3_column_text(stmt1, 1), "password");
              }
            free(buffer);
          }
        sql_return=sqlite3_step(stmt1);
      }
   
    sqlite3_finalize(stmt1);   
    sqlite3_close(cnn); 
  }
/*
Initialize a default admin and password table. Don't drop table if it already exists.
Set admin=2. Don't allow admin=2 to be deleted. The default admin and password can
be changed but admin=2 can't be deleted. Always one admin present in table.
*/
static bool initialize_admin_login_password(void)
  {
    int sql_return=0;
    bool table_created=false;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1="CREATE TABLE IF NOT EXISTS Passwords (admin INTEGER, login TEXT, password BLOB);";
    char *sql2="SELECT admin FROM Passwords WHERE admin==2;";
    char *sql3="INSERT INTO Passwords VALUES(2, 'admin', 'password');";

    sqlite3_open("password.db",&cnn);
    sqlite3_exec(cnn, sql1, 0, 0, 0);
    sqlite3_prepare_v2(cnn,sql2,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    if(sql_return!=SQLITE_ROW)
      {
        table_created=true;
        sqlite3_exec(cnn, sql3, 0, 0, 0);
      }
    sqlite3_finalize(stmt1);  
    sqlite3_close(cnn);  

    return table_created; 
  }
//Check if there is already a login and return the admin status of the login.
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
static bool insert_login_password(int admin, const char *login, const char *password)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    //check if login and password are already there.
    char *sql1=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login, password);
    char *sql2=sqlite3_mprintf("INSERT INTO Passwords VALUES('%i','%q','%q');", admin, login, password);
    
    sqlite3_open("password.db",&cnn);
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    if(sql_return==SQLITE_ROW) login_exists=true;
    else
      {
        login_exists=false;
        sqlite3_exec(cnn, sql2, 0, 0, 0);
      }
    sqlite3_finalize(stmt1);     
    sqlite3_close(cnn); 

    sqlite3_free(sql1);
    sqlite3_free(sql2);

    return login_exists;
  } 
static bool delete_login_password(const char *login, const char *password)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    //Don't delete if admin=2. Always have one login.
    char *sql1=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q' AND admin!=2;", login, password);
    char *sql2=sqlite3_mprintf("DELETE FROM Passwords WHERE login=='%q' AND password=='%q';", login, password);

    sqlite3_open("password.db",&cnn);   
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    if(sql_return==SQLITE_ROW) 
      {
        login_exists=true;
        sqlite3_exec(cnn, sql2, 0, 0, 0);   
      }
    else login_exists=false;
    sqlite3_finalize(stmt1);   
    sqlite3_close(cnn); 

    sqlite3_free(sql1);
    sqlite3_free(sql2);
    
    return login_exists;
  }
static bool update_login_password(const char *login_old, const char *password_old, const char *login_new, const char *password_new)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_stmt *stmt2=NULL;
    char *sql1=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login_old, password_old);
    char *sql2=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login_new, password_new);
    char *sql3=sqlite3_mprintf("UPDATE Passwords SET login='%q', password='%q' WHERE login=='%q' AND password=='%q';", login_new, password_new, login_old, password_old);
   
    sqlite3_open("password.db",&cnn);   
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sqlite3_prepare_v2(cnn,sql2,-1,&stmt2,0);
    sql_return=sqlite3_step(stmt1);
    //First check for old login and password.
    if(sql_return==SQLITE_ROW) 
      {
        //Check for new login and password.
        sql_return=sqlite3_step(stmt2);
        if(sql_return!=SQLITE_ROW) 
          {
            login_exists=true;
            sqlite3_exec(cnn, sql3, 0, 0, 0); 
          }  
      }
    else login_exists=false;
    sqlite3_finalize(stmt1); 
    sqlite3_finalize(stmt2);    
    sqlite3_close(cnn); 

    sqlite3_free(sql1);
    sqlite3_free(sql2);
    sqlite3_free(sql3);
    
    return login_exists;
  }

