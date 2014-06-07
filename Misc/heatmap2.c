/*
                                       Heatmaps are Cool2
    For comparison with heatmap1.py. Testing some HTML output code for heatmapping microtiter plates
and 2d arrays in general. The output is a heatmap2.html file that can be opened in a HTML5 web browser.

Compile with;  gcc -g -Wall -std=c99 heatmap2.c -lsqlite3 -o heat2

Check with; valgrind -v --leak-check=yes --track-origins=yes ./heat2

C. Eric Cashon
*/

#include <stdio.h>
#include <stdlib.h>
#include <float.h>
#include <sqlite3.h>

void test_sequence();
void heatmap_to_html_sql(int iRadioButton, int rows, int columns, int precision, int font_size);
void make_heatmap_html(double test_data[], int rows, int columns, int plate_size, int plates, int precision, int font_size);

int main()
  {

    //Comment out one function to test
    test_sequence();
       
    //Need 1 or 2 for UI radio button. Check plate_size and plates in database. Uses VelociRaptorData.db. Change as needed. (radio_button, rows, columns, precision, font_size)
    //heatmap_to_html_sql(1, 8, 13, 2, 10);
       
    return 0;
  }
void test_sequence()
  {
    //For sequence of numbers.
    int i=0;
    double test_data[768];
    int rows = 16;
    int columns = 24+1;
    int plate_size = 384;
    int plates = 2;
    int precision = 1;
    int font_size = 10;
    
    for(i=0;i<768;i++)
        {
          test_data[i]=(double)i;
        }

    make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size);

  }
void heatmap_to_html_sql(int iRadioButton, int rows, int columns, int precision, int font_size)
  {
    int i=0;
    int iRecordCount=0;
    int plate_size=0;
    int plates=0;
    sqlite3 *handle=NULL;
    sqlite3_stmt *stmt1=NULL;
    sqlite3_stmt *stmt2=NULL;
    sqlite3_stmt *stmt3=NULL;
    sqlite3_stmt *stmt4=NULL;
    sqlite3_stmt *stmt5=NULL;
    char sql1[]="SELECT count(data) FROM data;";
    char sql2[]="SELECT max(wells) FROM aux;";
    char sql3[]="SELECT max(plate) FROM aux;";
    char sql4[]="SELECT Data FROM data ORDER BY KeyID;";
    char sql5[]="SELECT Percent FROM data ORDER BY KeyID;";
   
    sqlite3_open("VelociRaptorData.db",&handle);

    //Get record count.
    sqlite3_prepare_v2(handle,sql1,-1,&stmt1,0);
    sqlite3_step(stmt1);
    iRecordCount=sqlite3_column_int(stmt1, 0);
    sqlite3_finalize(stmt1);
    //Get plate_number
    sqlite3_prepare_v2(handle,sql2,-1,&stmt2,0);
    sqlite3_step(stmt2);
    plate_size=sqlite3_column_int(stmt2, 0);
    sqlite3_finalize(stmt2);
    //Get plates.
    sqlite3_prepare_v2(handle,sql3,-1,&stmt3,0);
    sqlite3_step(stmt3);
    plates=sqlite3_column_int(stmt3, 0);
    sqlite3_finalize(stmt3);

    //Do some error checking.
    printf("rows %i columns %i database_records %i plate_size %i plates %i\n", rows, columns, iRecordCount, plate_size, plates);

    if(iRecordCount==0)
      {
        printf("No records returned from database.\n");
      }
    else if(plate_size==0)
      { 
        printf("Couldn't get plate size from aux table.\n");
      }
    else if(rows*(columns-1)!=plate_size)
      {
        printf("Rows times columns doesn't equal the plate_size.\n");
      }
    else
      {
        double *test_data = malloc((iRecordCount) * sizeof(double));
        if(test_data==NULL)
          {
            printf("Couldn't allocate memory for database records.\n");
          } 
        else
          {     
            if(iRadioButton==1)
              {
                sqlite3_prepare_v2(handle,sql4,-1,&stmt4,0);
                for(i=0;i<iRecordCount;i++)
                   {
                     sqlite3_step(stmt4);
                     test_data[i]=sqlite3_column_double(stmt4, 0);
                   }
                 sqlite3_finalize(stmt4); 
                 make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size);   
              }
            if(iRadioButton==2)
              {   
                sqlite3_prepare_v2(handle,sql5,-1,&stmt5,0);
                for(i=0;i<iRecordCount;i++)
                   {
                     sqlite3_step(stmt5);
                     test_data[i]=sqlite3_column_double(stmt5, 0);
                   }
                sqlite3_finalize(stmt5);
                make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size);       
              }
            }
         if(test_data!=NULL)free(test_data);
       }
     
     sqlite3_close(handle);         
     
  }
void make_heatmap_html(double test_data[], int rows, int columns, int plate_size, int plates, int precision, int font_size)
  {
    int i=0;
    int j=0;
    int k=0;

    //Get a color gradient from KColorEdit.
    //char *gradient_iris[] = {"#d507e5", "#d60fde", "#d711dc", "#d716d7", "#d819d5", "#d91ed1", "#d920ce", "#da25ca", "#da28c7", "#db2dc3", "#dc2fc1", "#dd34bc", "#dd37ba", "#de3cb5", "#de3eb3", "#df43ae", "#df46ac", "#e04ba8", "#e14da5", "#e252a1", "#e2559e", "#e35c98", "#e46193", "#e46491", "#e5698c", "#e77085", "#e77383", "#e8787f", "#e97f78", "#ea8275", "#ea8771", "#eb896f", "#ec8e6a", "#ec9168", "#ed9663", "#ed9861", "#ee9d5c", "#efa05a", "#efa556", "#f0a753", "#f1ac4f", "#f1af4c", "#f2b448", "#f2b646", "#f3bb41", "#f4be3f", "#f4c33a", "#f5c538", "#f6ca33", "#f6cd31", "#f7d22d", "#f7d42a", "#f8d926", "#f9dc23", "#f9e11f", "#fae31d", "#fbe818", "#fbeb16", "#fcf011", "#fcf20f", "#fdf50d", "#fdf70a", "#fefa08", "#fefc06", "#fefe04"};
//RGB
    char *gradient_iris[] = {"#0000FF", "#000FFF", "#001EFF", "#002DFF", "#003CFF", "#004BFF", "#005AFF", "#0069FF", "#0078FF", "#0087FF", "#0096FF", "#00A5FF", "#00B4FF", "#00C3FF", "#00D2FF", "#00E1FF", "#00F0FF", "#00FFF0", "#00FFE1", "#00FFD2", "#00FFC3", "#00FFB4", "#00FFA5", "#00FF96", "#00FF87", "#00FF78", "#00FF69", "#00FF5A", "#00FF4B", "#00FF3C", "#00FF2D", "#00FF1E", "#00FF0F", "#0FFF00", "#1EFF00", "#2DFF00", "#3CFF00", "#4BFF00", "#5AFF00", "#69FF00", "#78FF00", "#87FF00", "#96FF00", "#A5FF00", "#B4FF00", "#C3FF00", "#D2FF00", "#E1FF00", "#F0FF00", "#FFF000", "#FFE100", "#FFD200", "#FFC300", "#FFB400", "#FFA500", "#FF9600", "#FF8700", "#FF7800", "#FF6900", "#FF5A00", "#FF4B00", "#FF3C00", "#FF2D00", "#FF1E00", "#FF0F00"};
//Sun
    //char *gradient_iris[]={"#fe1005", "#fe1505", "#fe1705", "#fe1b05", "#fe1d05", "#fe2205", "#fe2405", "#fe2905", "#fe2b05", "#fe2f05", "#fe3205", "#fe3605", "#fe3805", "#fe3b05", "#fe4105", "#fe4405", "#fe4805", "#fe4f05", "#fe5105", "#fe5505", "#fe5805", "#fe5e05", "#fe6105", "#fe6505", "#fe6705", "#fe6e05", "#fe7305", "#fe7505", "#fe7c05", "#fe8004", "#fe8204", "#fe8404", "#fe8904", "#fe8b04", "#fe9004", "#fe9204", "#fe9604", "#fe9904", "#fe9d04", "#fe9f04", "#fea204", "#fea604", "#fea804", "#fead04", "#feaf04", "#feb404", "#feb604", "#feba04", "#febc04", "#fec104", "#fec304", "#fec504", "#feca04", "#fecc04", "#fed104", "#fed304", "#fed704", "#feda04", "#fedc04", "#fee004", "#fee304", "#fee504", "#fee904", "#feec04", "#feee04"};

    //High and low for each plate.
    double high[plates];
    double low[plates];
    double temp_high;
    double temp_low;
    for(i=0;i<plates;i++)
       {
         temp_low=DBL_MAX;
         temp_high=DBL_MIN;
         for(j=0;j<plate_size;j++)
            {
              if(temp_high<test_data[i*plate_size+j])
                {
                  temp_high=test_data[i*plate_size+j];
                }
              if(temp_low>test_data[i*plate_size+j])
                {
                  temp_low=test_data[i*plate_size+j];
                }
              
            }
          //printf("high %f low %f\n", temp_high, temp_low);
          high[i]=temp_high;
          low[i]=temp_low;
       }

    //Write HTML to file.
    FILE *f = fopen("heatmap2.html", "w");
      if(f == NULL)
        {
          printf("Error opening file!\n");
        }

    fprintf(f, "<!DOCTYPE html>\n");
    fprintf(f, "<html lang=\"en\">\n");
    fprintf(f, "<head>\n");
    fprintf(f, "<meta charset=\"UTF-8\"/>\n");
    fprintf(f, "<title>Heatmap</title>\n");
    fprintf(f, "</head>\n");
    fprintf(f, "<body style=\"font-size:%ipx\">\n", font_size);
    fprintf(f, "<h1 align=\"center\">Heatmap 384 Well Plates</h1>\n");

    //Use a counter for simplicity.
    int counter = 0;
    double diff = 0;
    double temp1 = 0;
    double temp2 = 0;
    double temp3 = 0;
    for(i=0;i<plates;i++)
       {
        //Write the score table 
        fprintf(f, "<table align=\"center\" style=\"border-collapse:collapse;\"><caption>Score %i</caption><tr>\n", i+1);
        for(j=0;j<8;j++)
           {
             diff = high[i] - low[i];
             temp1 = (low[i] + j*(diff/7));
             temp2 = (((temp1-low[i])/(high[i]-low[i])) *64);
             //printf("high %f low %f temp1 %f temp2 %f\n", high[i], low[i], temp1, temp2);
             if(temp2>64)temp2=64;
             if(temp2<0)temp2=0;
             fprintf(f, "<td bgcolor=\"%s\">%.*f</td>\n", gradient_iris[(int)temp2], precision, temp1);
           }
        fprintf(f, "</tr></table>");

        //The heatmap table
        fprintf(f, "<table align=\"center\" bgcolor=\"silver\"  style=\"margin-bottom:40pt\"><caption>Plate %i</caption><thead><tr>\n", i+1);
        fprintf(f, "<th scope=\"rowgroup\" bgcolor=\"silver\"></th>\n");
        for(j=0; j<columns-1; j++)
           {
             fprintf(f, "<th scope=\"col\" bgcolor=\"silver\">%i</th>\n", j+1);
           }
        fprintf(f, "</tr></thead><tbody>\n");  

        //Write the data and color gradient into the table.
        for(j=0; j<rows; j++)
           {
             fprintf(f, "<tr>\n");
             for(k=0;k<columns;k++)
                {
                  if(k==0)
                    {
                     //This is a label column. Doesn't count for the test_data counter.
                     fprintf(f, "<th scope=\"row\" bgcolor=\"silver\">%i</th>\n", j+1);
                    }
                  else
                    {
                      temp3 = (((test_data[counter]-low[i])/(high[i] - low[i])) *64);
                      if(temp3>64)temp3=64;
                      if(temp3<0)temp3=0;
                      fprintf(f, "<td bgcolor=\"%s\">%.*f</td>\n", gradient_iris[(int)temp3], precision, test_data[counter]);
                      counter+=1;
                    }
                  }
               fprintf(f, "</tr>\n");
             }
        fprintf(f, "</tbody></table>\n");
       }

    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");

    fclose(f); 
    printf("heatmap2.html file created.\n");
  }







