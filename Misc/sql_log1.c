
/*
Test code for sqlite. 

Compile with
    gcc -Wall sql_log1.c -o sql_log1 -lsqlite3 

valgrind ./sql_log1

C. Eric Cashon
*/

#include<sqlite3.h>
#include<stdio.h>
#include<stdbool.h>

static bool insert_login_password(char *login, char *password)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1="CREATE TABLE IF NOT EXISTS Passwords (login TEXT, password TEXT);";
    //check if login and password are already there.
    char *sql2=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login, password);
    char *sql3=sqlite3_mprintf("INSERT INTO Passwords VALUES('%q','%q');", login, password);
    
    sqlite3_open("password.db",&cnn);
    sqlite3_exec(cnn, sql1, 0, 0, 0);
    sqlite3_prepare_v2(cnn,sql2,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    if(sql_return==SQLITE_ROW) login_exists=true;
    else
      {
        login_exists=false;
        sqlite3_exec(cnn, sql3, 0, 0, 0);
      }
    sqlite3_finalize(stmt1);     
    sqlite3_close(cnn); 

    sqlite3_free(sql2);
    sqlite3_free(sql3);

    return login_exists;
  } 
static bool check_login_password(char *login, char *password)
  {
    bool login_exists=false;
    int sql_return=0;
    sqlite3 *cnn=NULL;
    sqlite3_stmt *stmt1=NULL;
    char *sql1="CREATE TABLE IF NOT EXISTS Passwords (login TEXT, password TEXT);";
    char *sql2=sqlite3_mprintf("SELECT login, password FROM Passwords WHERE login=='%q' AND password=='%q';", login, password);

    sqlite3_open("password.db",&cnn);
    sqlite3_exec(cnn, sql1, 0, 0, 0);   
    sqlite3_prepare_v2(cnn,sql2,-1,&stmt1,0);
    sql_return=sqlite3_step(stmt1);
    if(sql_return==SQLITE_ROW) login_exists=true;
    sqlite3_finalize(stmt1);   
    sqlite3_close(cnn); 

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
int main()
  {
    bool login_exists=false;

    login_exists=insert_login_password("Eric", "password");
    if(login_exists==true) printf("Found Existing Login\n");
    else printf("New Login Created\n");

    login_exists=check_login_password("Eric", "password");
    if(login_exists==true) printf("Found Login\n");
    else printf("No Login Found\n");
    
    login_exists=delete_login_password("Eric", "password");
    if(login_exists==true) printf("Login Deleted\n");
    else printf("No Login to Delete\n");

    return 0;
  }






