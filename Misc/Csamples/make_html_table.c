/*
                                       
Test code for generating HTML tables from a database. Still playing around with this. It needs 
more work. Test on a small dataset. For use with the VelociRaptor data.

Compile with;  gcc -Wall -std=c99 make_html_table.c -lapophenia -lsqlite3 -lgsl -lgslcblas -o table

C. Eric Cashon
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <float.h>
#include <stdbool.h>
#include <sqlite3.h>
#include <apop.h>
#include <gsl/gsl_vector.h>

void make_html_table(apop_data *mTable, char *field_names[], int fields, int precision, int font_size, char *bg_color, char *field_bg_color, bool fields_found);
void get_table_sql(char *sql, int precision, int font_size, char *bg_color, char *field_bg_color);

int main()
  {
    //Test some SQL strings. Case sensitive for capital SQL keywords(SELECT, AS and FROM).

    //char sql[]="SELECT * FROM data;";
    //char sql[]="SELECT percent FROM data;";
    char sql[]="SELECT T1.KeyID, T1.plate, T1.Groups, T2.data, T2.percent FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID;";
    //char sql[]="SELECT T4.plate AS plate1, count(T3.percent), avg(T3.percent) AS mean, stddev_samp(T3.percent) FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID AND T4.Picks!=0 GROUP BY T4.Picks,T4.Plate ORDER BY T4.Plate,T4.Picks;";
    int precision=2;
    int font_size=12;
    char bg_color[]="white";
    char field_bg_color[]="silver";

    get_table_sql(sql, precision, font_size, bg_color, field_bg_color);
       
    return 0;
  }
void make_html_table(apop_data *mTable, char *field_names[], int fields, int precision, int font_size, char *bg_color, char *field_bg_color, bool fields_found)
  {
    int i=0;
    int j=0;

    FILE *f = fopen("table.html", "w");
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
    fprintf(f, "<h1 align=\"center\">Database Table</h1>\n");
    fprintf(f, "<table align=\"center\" style=\"border-collapse:collapse;\">\n");
    fprintf(f, "<thead><tr>\n");

    //Print the field names.
    if(fields_found==true)
      {
        for(i=0; i<fields; i++)
           {
             fprintf(f, "<th scope=\"col\" bgcolor=\"%s\" style=\"border:1px solid #000000\">%s</th>\n", field_bg_color, field_names[i]);
           }
        fprintf(f, "</tr></thead><tbody>\n");  
      }
    else
      {
        for(i=0;i<mTable->matrix->size2;i++)
           {
             fprintf(f, "<th scope=\"col\" bgcolor=\"%s\" style=\"border:1px solid #000000\">Field%i</th>\n", field_bg_color, i+1);
           }
        fprintf(f, "</tr></thead><tbody>\n"); 
      }

    //Print the data.
    for(i=0;i<mTable->matrix->size1;i++)
       {
         fprintf(f, "<tr>\n");
         for(j=0;j<mTable->matrix->size2;j++)
            {
              fprintf(f, "<td style=\"border:1px solid #000000\" bgcolor=\"%s\">%.*f</td>\n", bg_color, precision, apop_data_get(mTable,i,j));
            }
         fprintf(f, "</tr>\n");
       }

    fprintf(f, "</tbody></table>");
    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");

    fclose(f); 
    printf("table.html file created.\n");
  }
void get_table_sql(char *sql, int precision, int font_size, char *bg_color, char *field_bg_color)
  {
    int i=0;
    int commas=0;
    int fields=0;
    bool fields_found=true;
    char *start;
    char *end;
    char c1='a';
    char c2='b';
    char c3='c';
    char c4='d';
    apop_data *mTable=NULL;

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
        //printf("Commas before FROM %i\n", commas);

        //allocate array for fields.
        char *field_names[commas+1];
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

        for(i=0;i<commas+1;i++)
           {
             printf("Field %s\n", field_names[i]);
           }

        apop_db_open("VelociRaptorData.db");
        mTable=apop_query_to_data("%s", sql);
        apop_db_close(0);

        fields=commas+1;
        make_html_table(mTable, field_names, fields, precision, font_size, bg_color, field_bg_color, fields_found);

        if(mTable!=NULL)gsl_matrix_free(mTable->matrix);
        for(i=0;i<commas+1;i++)
           {
             if(field_names[i]!=NULL)free(field_names[i]);
           }
    }
          
  }

