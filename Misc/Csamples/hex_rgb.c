
/*

  Heatmap a hex sequence and output to html for viewing. Three gradients for testing. A rgb, 
rgb with yellow and light blue and a yellow to purple gradient.

  gcc -Wall hex_rgb.c -o hex_rgb

C. Eric Cashon

*/

#include<stdio.h>

static int intRGB(int red, int green, int blue);
static void get_rgb_gradient(char hex_values[512][8]);
static void get_rygbb_gradient(char hex_values[512][8]);
static void get_yellow_purple_gradient(char hex_values[512][8]);
static void build_html(char hex_values[512][8]);

int main()
  {
    char hex_values[512][8];
    int gradient=1;
    
    if(gradient==0)
      {
        get_rgb_gradient(hex_values);
      }
    else if(gradient==1)
      {
        get_rygbb_gradient(hex_values);
      }
    else
      {
        get_yellow_purple_gradient(hex_values);
      }
    build_html(hex_values);
    return 0;
  }
static int intRGB(int red, int green, int blue)
  {
    int value=0;
    return value=65536*red+256*green+blue;
  }
static void get_rgb_gradient(char hex_values[512][8])
  {
    int i=0;
    int record=1;
    int red=0;
    int green=0;
    int blue=0;
    
    for(red=255,green=0;red>=0&&green<=255;red--,green++)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);         
       }

    red=0;
    printf("Break\n");

    for(green=255,blue=0;green>=0&&blue<=255;green--,blue++)
       {         
         printf("%i, hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);         
       } 
 
    printf("First %s Last %s\n", hex_values[0], hex_values[511]);

  }
static void get_rygbb_gradient(char hex_values[512][8])
  {
    int i=0;
    int record=1;
    int red=255;
    int green=0;
    int blue=0;
    
    for(green=0;green<=255;green+=2)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);          
       }

    green=255;
    for(red=255;red>=0;red-=2)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);          
       }

    red=0;
    for(blue=0;blue<=255;blue+=2)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);          
       }

    blue=255;
    for(green=255;green>=0;green-=2)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);          
       }
 
    printf("First %s Last %s\n", hex_values[0], hex_values[511]);

  }
static void get_yellow_purple_gradient(char hex_values[512][8])
  {
    int i=0;
    int record=1;
    int red=255;
    int green=255;
    int blue=0;
    
    for(green=255,blue=0;green>=0&&blue<=255;green--,blue++)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);       
       }

  }
static void build_html(char hex_values[512][8])
  {
    printf("Build HTML\n");
    int i=0;
    int j=0;
    int k=0;
    int plates=1;
    int rows=64;
    int columns=8+1;
    int counter=0;

    //Write HTML to file.
    FILE *f = fopen("heatmap3.html", "w");
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
    fprintf(f, "<body style=\"font-size:%ipx\">\n", 22);
    fprintf(f, "<h1 align=\"center\">Heatmap Hex Sequence</h1>\n");
    
    for(i=0;i<plates;i++)
       {
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
                      fprintf(f, "<td bgcolor=\"%s\">%s</td>\n", hex_values[counter], hex_values[counter]);
                      counter++;
                      //printf("Counter %i\n", counter);
                    }
                  }
               fprintf(f, "</tr>\n");
             }
        fprintf(f, "</tbody></table>\n");
       }

    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");

    fclose(f); 
    printf("heatmap3.html file created.\n");
  }


