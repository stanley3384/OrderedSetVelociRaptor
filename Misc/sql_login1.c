
/*
Test code for sqlite. Try some basic user logins.

Compile with
    gcc -Wall sql_login1.c -o sql_login1 -lsqlite3 

valgrind ./sql_login1

C. Eric Cashon
*/

#include<sqlite3.h>
#include<stdio.h>
#include<stdbool.h>

static bool initialize_admin_login_password();
static bool check_login_password(int *admin, char *login, char *password);
static bool insert_login_password(int admin, char *login, char *password);
static bool delete_login_password(char *login, char *password);
static bool update_login_password(char *login_old, char *password_old, char *login_new, char *password_new);

int main()
  {
    bool table_created=false;
    bool login_exists=false;
    int admin=0;

    table_created=initialize_admin_login_password();
    if(table_created==true) printf("A New Password Table Created\n");
    else printf("Password Table Already Exists\n");

    login_exists=check_login_password(&admin, "Eric", "password1");
    if(login_exists==true) printf("Found Login. Admin %i\n", admin);
    else printf("No Login Found\n");

    login_exists=insert_login_password(1, "Eric", "password1");
    if(login_exists==true) printf("Found Existing Login\n");
    else printf("New Login Created\n");

    login_exists=delete_login_password("Eric", "password2");
    if(login_exists==true) printf("Login Deleted\n");
    else printf("No Login to Delete\n");

    login_exists=update_login_password("Eric", "password1", "Eric", "password3");
    if(login_exists==true) printf("Login Updated\n");
    else printf("No Login to Update\n");

    return 0;
  }
//Initialize a default admin and password table. Don't drop table if it already exists.
static bool initialize_admin_login_password()
  {
    int sql_return=0;
    bool table_created=false;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1="CREATE TABLE IF NOT EXISTS Passwords (admin INTEGER, login TEXT, password TEXT);";
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
static bool check_login_password(int *admin, char *login, char *password)
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
static bool insert_login_password(int admin, char *login, char *password)
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
static bool delete_login_password(char *login, char *password)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login, password);
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
static bool update_login_password(char *login_old, char *password_old, char *login_new, char *password_new)
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






