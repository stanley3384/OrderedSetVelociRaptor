/*
                                       Heatmaps are Cool2
    For comparison with heatmap1.py. Testing some HTML output code for heatmapping microtiter plates
and 2d arrays in general. The output is a heatmap2.html file that can be opened in a HTML5 web browser.

Compile with;  gcc -Wall -std=c99 heatmap2.c -o heat

C. Eric Cashon
*/

#include <stdio.h>
#include <float.h>

void make_heatmap(double test_data[], int rows, int columns, int plate_size, int plates)
  {
    int i=0;
    int j=0;
    int k=0;

    //Get a color gradient from KColorEdit.
    char *gradient_iris[] = {"#d507e5", "#d60fde", "#d711dc", "#d716d7", "#d819d5", "#d91ed1", "#d920ce", "#da25ca", "#da28c7", "#db2dc3", "#dc2fc1", "#dd34bc", "#dd37ba", "#de3cb5", "#de3eb3", "#df43ae", "#df46ac", "#e04ba8", "#e14da5", "#e252a1", "#e2559e", "#e35c98", "#e46193", "#e46491", "#e5698c", "#e77085", "#e77383", "#e8787f", "#e97f78", "#ea8275", "#ea8771", "#eb896f", "#ec8e6a", "#ec9168", "#ed9663", "#ed9861", "#ee9d5c", "#efa05a", "#efa556", "#f0a753", "#f1ac4f", "#f1af4c", "#f2b448", "#f2b646", "#f3bb41", "#f4be3f", "#f4c33a", "#f5c538", "#f6ca33", "#f6cd31", "#f7d22d", "#f7d42a", "#f8d926", "#f9dc23", "#f9e11f", "#fae31d", "#fbe818", "#fbeb16", "#fcf011", "#fcf20f", "#fdf50d", "#fdf70a", "#fefa08", "#fefc06", "#fefe04"};

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
    fprintf(f, "<body style=\"font-size:12px\">\n");
    fprintf(f, "<h1 align=\"center\">Heatmap 96 Well Plates</h1>\n");

    //Use a counter for simplicity.
    int counter = 0;
    double diff = 0;
    double temp1 = 0;
    double temp2 = 0;
    double temp3 = 0;
    for(i=0;i<plates;i++)
       {
        //Write the score table 
        fprintf(f, "<table align=\"center\"><caption>Score %i</caption><tr>\n", i+1);
        for(j=0;j<8;j++)
           {
             diff = high[i] - low[i];
             temp1 = (low[i] + j*(diff/7));
             temp2 = (((temp1-low[i])/(high[i]-low[i])) *64);
             printf("high %f low %f temp1 %f temp2 %f\n", high[i], low[i], temp1, temp2);
             if(temp2>64)temp2=64;
             if(temp2<0)temp2=0;
             fprintf(f, "<td bgcolor=\"%s\">%8.2f</td>\n", gradient_iris[(int)temp2], temp1);
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
                      fprintf(f, "<td bgcolor=\"%s\">%8.2f</td>\n", gradient_iris[(int)temp3], test_data[counter]);
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
    printf("File Created\n");
  }

int main()
  {
    int i=0;
    double test_data[288];
    int rows = 8;
    int columns = 12+1;
    int plate_size = 96;
    int plates = 3;
    
    for(i=0;i<288;i++)
        {
          test_data[i]=(double)i;
        }

    make_heatmap(test_data, rows, columns, plate_size, plates);

    return 0;
  }









