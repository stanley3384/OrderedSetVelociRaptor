
/*

  Heatmap a hex sequence and output to text and html for viewing. 

  gcc -Wall hex_rgb.c -o hex_rgb `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>

static void button_clicked(GtkWidget *widget, GArray *widgets);
static int intRGB(int red, int green, int blue);
static void get_rgb_gradient(char hex_values[512][8]);
static void get_rygbb_gradient(char hex_values[512][8]);
static void get_yellow_purple_gradient(char hex_values[512][8]);
static void get_red_yellow_gradient(char hex_values[512][8]);
static void get_yellow_blue_gradient(char hex_values[512][8]);
static void build_hex_text(char hex_values[512][8], const gchar *filename2);
static void build_html_table(char hex_values[512][8], const gchar *filename1);

int main(int argc, char **argv)
 {
   GtkWidget *window, *combo1, *entry1, *entry2, *label1, *label2, *button1, *grid;

   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Heatmap Gradients");
   gtk_window_set_default_size(GTK_WINDOW(window), 250, 200);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   gtk_container_set_border_width(GTK_CONTAINER(window), 20);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "rgb gradient");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "rygbb gradient");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "yellow purple gradient");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "3", "red yellow gradient");
   gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "4", "yellow blue gradient");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 1);

   label1=gtk_label_new("Text File Name");
   gtk_widget_set_hexpand(label1, TRUE);

   entry1=gtk_entry_new();
   gtk_widget_set_hexpand(entry1, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry1), "HexHeatmap.txt");

   label2=gtk_label_new("HTML File Name");
   gtk_widget_set_hexpand(label2, TRUE);

   entry2=gtk_entry_new();
   gtk_widget_set_hexpand(entry2, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry2), "HexHeatmap.html");

   GArray *widgets=g_array_new(FALSE, FALSE, sizeof(GtkWidget*));
   g_array_append_val(widgets, combo1);
   g_array_append_val(widgets, entry1);
   g_array_append_val(widgets, entry2);

   button1=gtk_button_new_with_label("Create Files");
   gtk_widget_set_hexpand(button1, TRUE);
   g_signal_connect(button1, "clicked", G_CALLBACK(button_clicked), widgets); 

   grid=gtk_grid_new();
   gtk_grid_set_row_spacing(GTK_GRID(grid), 10);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry1, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label2, 0, 3, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry2, 0, 4, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 5, 1, 1);
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();
   return 0;
  }
static void button_clicked(GtkWidget *widget, GArray *widgets)
  {
    char hex_values[512][8];
    
    const gchar *index=gtk_combo_box_get_active_id(GTK_COMBO_BOX(g_array_index(widgets, GtkWidget*, 0)));
    const gchar *filename1=gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 1)));
    const gchar *filename2=gtk_entry_get_text(GTK_ENTRY(g_array_index(widgets, GtkWidget*, 2)));
    int gradient=atoi(index);
    int length1=strlen(filename1);
    int length2=strlen(filename2);

    if(length1<5)
      {
        printf("String length %i\n", length1);
        printf("The text file length needs 5 or more characters.\n");
      }
    else if(length2<5)
      {
        printf("String length %i\n", length2);
        printf("The html file length needs 5 or more characters.\n");
      }
    else
      {
        if(gradient==0)
          {
            get_rgb_gradient(hex_values);
          }
        else if(gradient==1)
          {
            get_rygbb_gradient(hex_values);
          }
        else if(gradient==2)
          {
            get_yellow_purple_gradient(hex_values);
          }
        else if(gradient==3)
          {
            get_red_yellow_gradient(hex_values);
          }
        else
          {
            get_yellow_blue_gradient(hex_values);
          }    
        build_hex_text(hex_values, filename1);
        build_html_table(hex_values, filename2);
     }
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
static void get_red_yellow_gradient(char hex_values[512][8])
  {
    int i=0;
    int record=1;
    int red=255;
    int green=0;
    int blue=0;
    
    for(green=0;green<=255;green++)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);       
       }

  }
static void get_yellow_blue_gradient(char hex_values[512][8])
  {
    int i=0;
    int record=1;
    int red=255;
    int green=255;
    int blue=0;
    
    for(red=255,green=255,blue=0;red>=0&&green>=0&&blue<=255;red--,green--,blue++)
       {        
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);
         printf("%i hex #%02x%02x%02x int %i r %i g %i b %i\n", record++, red, green, blue, intRGB(red,green,blue), red, green, blue);
         sprintf(hex_values[i++], "#%02x%02x%02x", red, green, blue);       
       }

  }
static void build_hex_text(char hex_values[512][8], const gchar *filename1)
  {
    printf("Build Text\n");
    int i=0;

    FILE *f=fopen("heatmap3.txt", "w");
      if(f==NULL)
        {
          printf("Error opening file!\n");
        }

    fprintf(f, "{");
    for(i=0;i<512;i++)
       {
         if(i%8==0&&i!=0)
            {
              fprintf(f, "\\\n ");
              fprintf(f, "%s, ", hex_values[i]);
            }
         else if(i==511)
            {
              fprintf(f, "%s", hex_values[i]);
            } 
         else
            {
              fprintf(f, "%s, ", hex_values[i]);
            }
       }
    fprintf(f, "}");

    fclose(f); 
    printf("heatmap3.txt file created.\n");
  }
static void build_html_table(char hex_values[512][8], const gchar *filename2)
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
    FILE *f = fopen(filename2, "w");
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
    printf("%s file created.\n", filename2);
  }


