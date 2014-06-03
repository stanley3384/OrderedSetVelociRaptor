/*
                                       
Test code for generating HTML tables from a SQLite database. Test on a small dataset. This one
is better with the data format but doesn't load the values into a structure for further calculations.
If you use Ben Klemens Apophenia library you can get some statistical calculations done with SQlite
by adding the statistical functions to the SQL string. This can be a big help and add funtionality to
SQLite.

Compile with;  gcc -Wall -std=c99 -g make_html_table2.c -lsqlite3 -o table3

Check with; valgrind -v --leak-check=yes --track-origins=yes ./table3

C. Eric Cashon
*/

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sqlite3.h>


int check_sql_for_select(char *sql);
void parse_sql_field_names(char *html_file_name, char *database_name, char *sql, int precision, int font_size, char *bg_color, char *field_bg_color);

static void make_html_table(char *html_file_name, char *database_name, char *sql, char *field_names[], int fields, int precision, int font_size, char *bg_color, char *field_bg_color, bool fields_found);
static void get_data_for_query(FILE *f, char *database_name, char *sql, int precision, char *bg_color);
static int get_column_count(char *database_name, char *sql);

int main()
  {
    int check=0;
    //Test some SQL strings including some bad ones.
    //char *sql=NULL;
    //char sql[]="selec fro;";
    //char sql[]="SELECT FROM;";
    //char sql[]="SELECT * FROM data;";
    //char sql[]="SELECT percent FROM data;";
    char sql[]="SELECT T1.KeyID, T1.plate, T1.Groups AS G1, T2.data, T2.percent FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID;";
    
    //Check if file exists? SQLite will create it automatically if it doesn't. Use the VelociRaptor db for testing.
    char database_name[]="VelociRaptorData.db";
    char html_file_name[]="table3.html";
    //For returned floats.
    int precision=2;
    int font_size=12;
    char bg_color[]="white";
    char field_bg_color[]="silver";

    check=check_sql_for_select(sql);
    
    if(check==0)
      {
        printf("%s\n", sql);
        parse_sql_field_names(html_file_name, database_name, sql, precision, font_size, bg_color, field_bg_color);
      }
    else
      {
        printf("SQL statement doesn't check out.\n");
      }
       
    return 0;
  }
int check_sql_for_select(char *sql)
  {
    //Some basic SQL statement checking and string replacement.
    int check=0;
    char replace1[]="SELECT ";
    char replace2[]=" FROM ";
    char replace3[]=" AS ";
    char *sp=NULL;
    char *fp=NULL;
    char *ap=NULL;
    
    if(sql!=NULL&&strlen(sql)>12)
      {
        //Capitalize Select.
        if((sp=strstr(sql, "select ")))
          {
             strncpy(sp, replace1 , 7);
          }
        if((sp=strstr(sql, "Select ")))
          {
             strncpy(sp, replace1 , 7);
          }
        //Capitalize From.
        if((fp=strstr(sql, " from ")))
          {
             strncpy(fp, replace2 , 6);
          }
        if((fp=strstr(sql, " From ")))
          {
             strncpy(fp, replace2 , 6);
          }
        //Capitalize As
        if((ap=strstr(sql, " As ")))
          {
             strncpy(ap, replace3 , 4);
          }
        if((ap=strstr(sql, " as ")))
          {
             strncpy(ap, replace3 , 4);
          }
      }

    if(sql!=NULL&&strlen(sql)>12)
      {
        if(strstr(sql, "SELECT ")&&strstr(sql, " FROM"))
          {
            check=0;
          }
      }
    else
      {
        check=1;
        printf("The SQL statement needs SELECT and FROM keywords with fields and tables.\n");
      }
      
    return check;

  }
void parse_sql_field_names(char *html_file_name, char *database_name, char *sql, int precision, int font_size, char *bg_color, char *field_bg_color)
  {
    int i=0;
    int commas=0;
    int fields=0;
    bool fields_found=true;
    int field_count=0;
    int as_return=0;
    int sql_return=0;
    char *start;
    char *end;
    char c1='a';
    char c2='b';
    char c3='c';
    char c4='d';
    sqlite3 *handle=NULL;
    sqlite3_stmt *stmt=NULL;

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
        for(i=0;i<commas+1;i++)field_names[i]=NULL;
        
        //No field names in SQL statement so get them from the database.
        if(fields_found==false)
           {
             sqlite3_open(database_name,&handle);         
             sql_return=sqlite3_prepare_v2(handle,sql,-1,&stmt,0);
           
             if(sql_return!=SQLITE_OK)
               {
                 printf("SQLite couldn't parse SQL string.\n");
               }
             else
               {
                 for(i=0;i<field_count;i++)
                    {
                      as_return=asprintf(&field_names[i], "%s", sqlite3_column_name(stmt, i));
                      if(as_return==-1)printf("Memory allocation failed in asprintf.\n");
                    }
                }
             sqlite3_finalize(stmt);
             sqlite3_close(handle);
           }

        //Field names in the SQL statement so use those.
        i=0;
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
                      if(field_names[i]!=NULL)memset(field_names[i], '\0', j-k+1);
                      if(field_names[i]==NULL)printf("Malloc error.\n");
                      strncpy(field_names[i], sql+k, j-k);
                      //printf("Count %i Length %i String %s\n", i, strlen(field_names[i]), field_names[i]);
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
static void make_html_table(char *html_file_name, char *database_name, char *sql, char *field_names[], int fields, int precision, int font_size, char *bg_color, char *field_bg_color, bool fields_found)
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
         fprintf(f, "<th scope=\"col\" bgcolor=\"%s\" style=\"border:1px solid #000000\">&nbsp%s&nbsp</th>\n", field_bg_color, field_names[i]);
       }
    fprintf(f, "</tr></thead><tbody>\n");  

    get_data_for_query(f, database_name, sql, precision, bg_color);

    fprintf(f, "</tbody></table>");
    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");

    fclose(f); 
    printf("%s file created.\n", html_file_name);
  }
static void get_data_for_query(FILE *f, char *database_name, char *sql, int precision, char *bg_color)
  {
    int i=0;
    int column_count=0;
    int column_type=0;
    int sql_return=0;
    sqlite3 *handle=NULL;
    sqlite3_stmt *stmt=NULL;

    sqlite3_open(database_name,&handle);
    sql_return=sqlite3_prepare_v2(handle,sql,-1,&stmt,0);

    if(sql_return!=SQLITE_OK)
      {
         printf("SQLite couldn't parse SQL string.\n");
      }
    else
      {
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
  
  }
static int get_column_count(char *database_name, char *sql)
  {
    int column_count=0;
    int sql_return=0;
    sqlite3 *handle=NULL;
    sqlite3_stmt *stmt=NULL;

    sqlite3_open(database_name,&handle);   
    sql_return=sqlite3_prepare_v2(handle,sql,-1,&stmt,0);

    if(sql_return!=SQLITE_OK)
      {
         printf("SQLite couldn't parse SQL string.\n");
      }
    else
      {
        column_count=sqlite3_column_count(stmt);
      }

    sqlite3_finalize(stmt);
    sqlite3_close(handle);
    return column_count;
  }
