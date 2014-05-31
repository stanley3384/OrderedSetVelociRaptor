/*
                                       
Test code for generating HTML tables from a database. Still playing around with this. It needs 
more work. Test on a small dataset. This one is better with the data format but doesn't load
the values into a structure for further calculations.

Compile with;  gcc -Wall -std=c99 -g make_html_table2.c -lsqlite3 -o table3

Check with; valgrind --leak-check=yes ./table3

C. Eric Cashon
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sqlite3.h>


void make_html_table(char *html_file_name, char *database_name, char *sql, char *field_names[], int fields, int precision, int font_size, char *bg_color, char *field_bg_color, bool fields_found);
void parse_sql_field_names(char *html_file_name, char *database_name, char *sql, int precision, int font_size, char *bg_color, char *field_bg_color);
void get_data_for_query(FILE *f, char *database_name, char *sql, int precision, char *bg_color);
int get_column_count(char *database_name, char *sql);

int main()
  {
    //Test some SQL strings. Case sensitive for capital SQL keywords(SELECT, AS and FROM).

    //char sql[]="SELECT * FROM data;";
    //char sql[]="SELECT percent FROM data;";
    char sql[]="SELECT T1.KeyID, T1.plate, T1.Groups, T2.data, T2.percent FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID;";
    //char sql[]="SELECT T4.plate AS plate1, count(T3.percent), avg(T3.percent) AS mean, stddev_samp(T3.percent) FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID AND T4.Picks!=0 GROUP BY T4.Picks,T4.Plate ORDER BY T4.Plate,T4.Picks;";
    char database_name[]="VelociRaptorData.db";
    char html_file_name[]="table3.html";
    //For floats.
    int precision=2;
    int font_size=12;
    char bg_color[]="white";
    char field_bg_color[]="silver";

    parse_sql_field_names(html_file_name, database_name, sql, precision, font_size, bg_color, field_bg_color);
       
    return 0;
  }
void make_html_table(char *html_file_name, char *database_name, char *sql, char *field_names[], int fields, int precision, int font_size, char *bg_color, char *field_bg_color, bool fields_found)
  {
    int i=0;

    FILE *f = fopen(html_file_name, "w");
      if(f == NULL)
        {
          printf("Error opening file!\n");
        }

    fprintf(f, "<!DOCTYPE html>\n");
    fprintf(f, "<html lang=\"en\">\n");
    fprintf(f, "<head>\n");
    fprintf(f, "<meta charset=\"UTF-8\"/>\n");
    fprintf(f, "<title>Table Test</title>\n");
    fprintf(f, "</head>\n");
    fprintf(f, "<body style=\"font-size:%ipx\">\n", font_size);
    fprintf(f, "<h1 align=\"center\">Database Query</h1>\n");
    fprintf(f, "<h4 align=\"center\">%s</h4>\n", sql);
    fprintf(f, "<table align=\"center\" style=\"border-collapse:collapse;\">\n");
    fprintf(f, "<thead><tr>\n");

    //Print the field names.
    for(i=0; i<fields; i++)
       {
         fprintf(f, "<th scope=\"col\" bgcolor=\"%s\" style=\"border:1px solid #000000\">%s</th>\n", field_bg_color, field_names[i]);
       }
    fprintf(f, "</tr></thead><tbody>\n");  

    get_data_for_query(f, database_name, sql, precision, bg_color);

    fprintf(f, "</tbody></table>");
    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");

    fclose(f); 
    printf("%s file created.\n", html_file_name);
  }
void parse_sql_field_names(char *html_file_name, char *database_name, char *sql, int precision, int font_size, char *bg_color, char *field_bg_color)
  {
    int i=0;
    int commas=0;
    int fields=0;
    bool fields_found=true;
    int field_count=0;
    char *start;
    char *end;
    char c1='a';
    char c2='b';
    char c3='c';
    char c4='d';
    sqlite3 *handle;
    sqlite3_stmt *stmt;

    //Count commas between SELECT and FROM for number of fields.
    start=sql;
    end=strstr(sql,"FROM");
    if(end==NULL)
      {
        printf("Can't find FROM in the SQL statement.\n");
      }
    else
      {
        while(start<end)
          {
            c3=c2;
            c2=c1;
            c1=*start;
            if(c3==' '&&c2=='*'&&c1==' ')
              {
                printf("No field names in sql string.\n");
                fields_found=false;
              }
            if(*start==',')
              {
                commas++;
              }
            start++;
          }
   
        if(fields_found==false)
           {
             field_count=get_column_count(database_name, sql);
             commas=field_count-1;
           }

        //allocate array for fields.
        char *field_names[commas+1];

        //No field names in SQL statement so get them from the database.
        if(fields_found==false)
           {
             sqlite3_open(database_name,&handle);
             sqlite3_prepare_v2(handle,sql,-1,&stmt,0);
             for(i=0;i<field_count;i++)
                {
                  asprintf(&field_names[i], "%s", sqlite3_column_name(stmt, i));
                }
             sqlite3_finalize(stmt);
             sqlite3_close(handle);
           }

        //Field names in the SQL statement so use those.
        if(fields_found==true)
           {
             int j=7;
             int k=7;
             start=sql+7;

             while(start<end)
                {
                  c4=c3;
                  c3=c2;
                  c2=c1;
                  c1=*start;
                  //printf("%c%c%c%c\n", c4,c3,c2,c1);
                  if(c4==' '&&c3=='A'&&c2=='S'&&c1==' ')
                    {
                      k=j+1;
                      printf("Found ' AS '\n");
                    }
                  if(*start==','||start==end-1)
                    {
                      field_names[i] = (char*)malloc((j-k+1) * sizeof(char));
                      strncpy(field_names[i], sql+k, j-k);
                      i++;
                      k=j+2;
                    }
                  j++;
                  start++;
                }
           }

        for(i=0;i<commas+1;i++)
           {
             printf("Field %s\n", field_names[i]);
           }

        fields=commas+1;
        make_html_table(html_file_name, database_name, sql, field_names, fields, precision, font_size, bg_color, field_bg_color, fields_found);

        for(i=0;i<commas+1;i++)
           {
             if(field_names[i]!=NULL)free(field_names[i]);
           }
    }
          
  }
void get_data_for_query(FILE *f, char *database_name, char *sql, int precision, char *bg_color)
  {
    int i=0;
    int column_count=0;
    int column_type=0;
    sqlite3 *handle;
    sqlite3_stmt *stmt;

    sqlite3_open(database_name,&handle);

    sqlite3_prepare_v2(handle,sql,-1,&stmt,0);

    //Get column count
    column_count=sqlite3_column_count(stmt);
    printf("Column Count %i\n", column_count);
    
    while(sqlite3_step(stmt) == SQLITE_ROW)
       {
         fprintf(f, "<tr>\n");
         for(i=0;i<column_count;i++)
            {
              column_type=sqlite3_column_type(stmt, i);
              switch(column_type)
                {
                  case SQLITE_INTEGER:
                    {
                      //printf("%i  ", sqlite3_column_int(stmt, i));
                      fprintf(f, "<td style=\"border:1px solid #000000\" bgcolor=\"%s\">%i</td>\n", bg_color, sqlite3_column_int(stmt, i));
                      break;
                    }
                  case SQLITE_FLOAT:
                    {
                      //printf("%f  ", sqlite3_column_double(stmt, i));
                      fprintf(f, "<td style=\"border:1px solid #000000\" bgcolor=\"%s\">%.*f</td>\n", bg_color, precision, sqlite3_column_double(stmt, i));
                      break;
                    }
                  case SQLITE_TEXT:
                    {
                      //printf("%s  ", sqlite3_column_text(stmt, i));
                      fprintf(f, "<td style=\"border:1px solid #000000\" bgcolor=\"%s\">%s</td>\n", bg_color, sqlite3_column_text(stmt, i));
                      break;
                    }
                  case SQLITE_BLOB:
                    {
                      //printf("Blob\n");
                      fprintf(f, "<td style=\"border:1px solid #000000\" bgcolor=\"%s\">%s</td>\n", bg_color, "Blob");
                      break;
                    }
                  case SQLITE_NULL:
                    {
                      //printf("Null\n");
                      fprintf(f, "<td style=\"border:1px solid #000000\" bgcolor=\"%s\">%s</td>\n", bg_color, "Null");
                      break;
                 }
               }
            }
         //printf("\n");
         fprintf(f, "</tr>\n");
       }
     sqlite3_finalize(stmt);
     sqlite3_close(handle);
  }
int get_column_count(char *database_name, char *sql)
  {
    int column_count=0;
    sqlite3 *handle;
    sqlite3_stmt *stmt;

    sqlite3_open(database_name,&handle);
    sqlite3_prepare_v2(handle,sql,-1,&stmt,0);
    column_count=sqlite3_column_count(stmt);
    sqlite3_finalize(stmt);
    sqlite3_close(handle);

    return column_count;
  }

