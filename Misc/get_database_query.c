
/*

Some more test code. For use with the VelociRaptor application. Working on
making the HTML table output a little more flexible. 

Compile with;  gcc -Wall -std=c99 get_database_query.c -lsqlite3 -o table2

C. Eric Cashon
*/

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

void get_database_query(char *sql);

int main()
  {
    char sql[]="SELECT * FROM data;";

    get_database_query(sql);
    return 0;
  }
void get_database_query(char *sql)
  {
    int i=0;
    int column_count=0;
    int column_type=0;
    sqlite3 *handle;
    sqlite3_stmt *stmt;

    sqlite3_open("VelociRaptorData.db",&handle);

    sqlite3_prepare_v2(handle,sql,-1,&stmt,0);

    //Get column count
    column_count=sqlite3_column_count(stmt);
    printf("Column Count %i\n", column_count);
    
    while(sqlite3_step(stmt) == SQLITE_ROW)
       {
         for(i=0;i<column_count;i++)
            {
              column_type=sqlite3_column_type(stmt, i);
              switch(column_type)
                {
                  case SQLITE_INTEGER:
                    {
                      printf("%i  ", sqlite3_column_int(stmt, i));
                      break;
                    }
                  case SQLITE_FLOAT:
                    {
                      printf("%f  ", sqlite3_column_double(stmt, i));
                      break;
                    }
                  case SQLITE_TEXT:
                    {
                      printf("%s  ", sqlite3_column_text(stmt, i));
                      break;
                    }
                  case SQLITE_BLOB:
                    {
                      printf("Blob\n");
                      break;
                    }
                  case SQLITE_NULL:
                    {
                      printf("Null\n");
                      break;
                 }
               }
            }
         printf("\n");
       }
     sqlite3_finalize(stmt);
     sqlite3_close(handle);
  }



