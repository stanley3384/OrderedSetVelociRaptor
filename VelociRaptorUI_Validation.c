
/* Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.*/

#include <gtk/gtk.h>
#include <glib.h>
#include <glib/gprintf.h>
#include <sqlite3.h>
#include <stdlib.h>
#include <string.h>

void simple_message_dialog(const char *str);
void entry_field_validation(const gchar *pPlateSizeText1, const gchar *pPlateStatsText1, GtkWidget *entry);
void control_changed_validation(const gchar *pPlateSizeText, GtkWidget *entry);
int critical_value_changed_validation(GtkWidget *entry);
int group_by_changed_validation(GtkWidget *entry);
int groups_database_validation(GtkWidget *entry);
int picks_database_validation(GtkWidget *entry);
int contrast_matrix_validation(char contrasts[], int *rows, int *columns);

void simple_message_dialog(const char *str)
  {
    //Handle basic errors with a pop-up window.
    GtkWidget *dialog, *label, *content_area;

    dialog=gtk_dialog_new_with_buttons("Information", NULL, GTK_DIALOG_DESTROY_WITH_PARENT, GTK_STOCK_OK, GTK_RESPONSE_OK, NULL);
    label=gtk_label_new(str);
    content_area = gtk_dialog_get_content_area (GTK_DIALOG (dialog));
    gtk_box_set_spacing(GTK_BOX(content_area), 10);
    gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
    gtk_container_add (GTK_CONTAINER (content_area), label);
    g_signal_connect(dialog, "destroy", G_CALLBACK(gtk_widget_destroy), NULL);
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);   
  }
void entry_field_validation(const gchar *pPlateSizeText1, const gchar *pPlateStatsText1, GtkWidget *entry)
  {
    //A couple of simple checks for the entry fields. If the focus is grabbed back into the
   //problem field runtime errors occur when the main window is destroyed.
    const gchar *pTempValue;
    const gchar *pString;
    int iPlateStats=atoi(pPlateStatsText1);
    int iPlateSize=atoi(pPlateSizeText1);
    gchar c;
    int check=0;
    int check2=0;
    int check3=0;
    int iZeroCheck=0;
    guint32 count=0;
    g_print("Focus Changed\n"); 

    pTempValue=gtk_entry_get_text(GTK_ENTRY(entry));
    pString=pTempValue;
    
    if(iPlateStats!=0)
       {
          if(iPlateSize%iPlateStats!=0)
              {
                check2=1;
                  if(2*iPlateStats>iPlateSize)
                      {
                         check2=0;
                         check3=1;
                      }
              }
       }



    while ((c = *pString++) != '\0')
      {
        if (!g_ascii_isdigit(c))
           {
             check=1;
           }
        if (c=='0' && count==0)
           {
             iZeroCheck=1;
           }
        count++;
      }
    g_print("Entry Value %s\n",pTempValue);
 
    if(check==1)
       {
        g_print("Entry Field Needs a Number\n");
        simple_message_dialog("The field needs a positive number!");
       }
    else if(check2==1)
       {
        g_print("Integer Division Remainder\n");
        simple_message_dialog("The Size of Plate is not integer divisible\n by the Set Size for Stats!");
       }
    else if(check3==1)
       {
        g_print("Couldn't Divide PlateSize by PlateStats\n");
        simple_message_dialog("Two times the Set Size for Stats is greater\n than the Plate Size!");
       }
    else if(count==0)
       {
        g_print("Empty String\n");
        simple_message_dialog("The field needs a positive number!");
       }
    else if(iZeroCheck==1)
       {
        g_print("The Value is Zero\n");
        simple_message_dialog("A misplaced 0 or the value needs\n to be greater than 0.");
       }
    else
       {
        g_print("Entry Field OK %s\n", pTempValue);
       }
  }
void control_changed_validation(const gchar *pPlateSizeText, GtkWidget *entry)
  {
     //A couple of checks for correct formatting in the two control fields.
    const gchar *pTempValue;
    const gchar *pString;
    GString *Buffer=g_string_new(NULL);
    gchar c;
    gchar d='~';
    int iPlateSize;
    int check=0;
    int check2=0;
    int check3=0;
    int check4=0;
    int check5=0;
    guint32 count=0;
    guint32 length=0;
    int iValue;

    g_print("Focus Changed\n"); 
    iPlateSize=atoi(pPlateSizeText);
    pTempValue=gtk_entry_get_text(GTK_ENTRY(entry));
    pString=pTempValue;
    length=strlen(pTempValue);
    
    while(count<length+1)
      {
       c=*pString++;
        if(c != ',' && c != '\0')
          {
            if(!g_ascii_isdigit(c))
              {
                check=1;
              }
          }
        if(c==',' && d==',')
           {
             check3=1;
           }
        else if((c=='0' && d==',')||(c=='0' && d=='~'))
           {
             check4=1;
           }
        else if(c==',' && count==length-1)
           {
             check5=1;
           }
        else if(c==',' && d=='~')
           {
             check5=1;
           }
        else if(c==','|| count==length)
           {
             iValue=atoi(Buffer->str);  
             g_print("Buffer Value %i\n", iValue);
                if(iValue>iPlateSize)
                  {
                   check2=1;
                  }
              g_string_truncate(Buffer,0);
           }
        else
           {
            g_string_append_printf(Buffer, "%c", c);
           }
       count++;
       d=c;
      }  

    g_print("Entry Value %s\n",pTempValue);
 
    if(check==1)
       {
        g_print("Wrong Format\n");
        simple_message_dialog("A control field needs correct formatting!\nExample 1,2,3,4 or 4,7,9");
       }
    else if(check2==1)
       {
        g_print("A Number Greater than the Plate Size\n");
        simple_message_dialog("Values for the controls need to be less\n than or equal to the Size of Plate!");
       }
    else if(check3==1)
       {
        g_print("Two or More Commas\n");
        simple_message_dialog("Two or more commas in a row!");
       }
    else if(check4==1)
       {
        g_print("A Zero in a Plate Control Field\n");
        simple_message_dialog("A misplaced zero in a control field! Valid\n numbers range from 1 to Size of Plate");
       }
    else if(check5==1)
       {
        g_print("A Comma at the Start or End\n");
        simple_message_dialog("A comma at the start or end of a\n control field!");
       }
    else
       {
        g_print("Entry Field OK %s\n", pTempValue);
       }

    if(Buffer!=NULL)
       {
        g_string_free(Buffer,TRUE);
       }
  }
int critical_value_changed_validation(GtkWidget *entry)
  {
    int check=0;
    const gchar *pTempValue;
    pTempValue=gtk_entry_get_text(GTK_ENTRY(entry));

    if(atof(pTempValue)>0.30 || atof(pTempValue)<0.000001)
       {
         g_print("Critical Values 0.000001<x<0.30\n");
         simple_message_dialog("Critical Values 0.000001<x<0.30");
         check=1;
       }  
    return check;  
  }
int group_by_changed_validation(GtkWidget *entry)
  {
    int check=0;
    const gchar *pTempValue;
    pTempValue=gtk_entry_get_text(GTK_ENTRY(entry));

    if(atoi(pTempValue)<=0)
      {
        g_print("GROUP BY Values x>0\n");
        simple_message_dialog("GROUP BY Values x>0");
        check=1;
      }
    return check;
  }
int groups_database_validation(GtkWidget *entry)
  {
    sqlite3 *db;
    sqlite3_stmt *stmt1;
    int check=0;
    int iMax=0;
    const gchar *pTempValue;
    pTempValue=gtk_entry_get_text(GTK_ENTRY(entry));
    char *pSQL="SELECT max(Groups) FROM aux;";

    sqlite3_open("VelociRaptorData.db",&db);
    sqlite3_prepare_v2(db,pSQL,-1,&stmt1,0);
    sqlite3_step(stmt1);
    iMax=sqlite3_column_int(stmt1, 0);
    sqlite3_finalize(stmt1); 
    sqlite3_close(db);

    if(atoi(pTempValue)>iMax)
      {
        g_print("GROUP BY Groups Value not in Database\n");
        simple_message_dialog("GROUP BY Value not in Database. Check the\n Groups Field in the Auxiliary Table.");
        check=1;
      }
    return check;
  }
int picks_database_validation(GtkWidget *entry)
  {
    sqlite3 *db;
    sqlite3_stmt *stmt1;
    int check=0;
    int iMax=0;
    const gchar *pTempValue;
    pTempValue=gtk_entry_get_text(GTK_ENTRY(entry));
    char *pSQL="SELECT max(Picks) FROM aux;";

    sqlite3_open("VelociRaptorData.db",&db);
    sqlite3_prepare_v2(db,pSQL,-1,&stmt1,0);
    sqlite3_step(stmt1);
    iMax=sqlite3_column_int(stmt1, 0);
    sqlite3_finalize(stmt1); 
    sqlite3_close(db);

    if(atoi(pTempValue)>iMax)
      {
        g_print("GROUP BY Picks Value not in Database\n");
        simple_message_dialog("GROUP BY Value not in Database. Check the\n Picks Field in the Auxiliary Table.");
        check=1;
      }
    return check;
  }
int contrast_matrix_validation(char contrasts[], int *rows, int *columns)
  {
    //Check the text values from the textview.
      int i=0;
      int iCountNumbers=0;
      int iCountNewlines=0;
      int iCountColumns=0;
      char TwoChars='2';
      char FoundNewline='0';
      int CheckContrasts=0;
      int length=0;
      
      length=strlen(contrasts);
      for(i=0;i<length;i++)
         {
           if(contrasts[i]=='\n'||contrasts[i]=='-'||contrasts[i]=='1'||contrasts[i]==' '||contrasts[i]=='0')
             {
               if(contrasts[i]=='\n')
                 {
                   if(contrasts[i]=='\n'&&TwoChars=='\n')
                     {
                       //Additional newlines are OK.
                       //printf("found a space between rows or additional newlines at the end\n");
                     }
                   TwoChars=contrasts[i];
                 }
               else if(contrasts[i]=='-')
                 {
                   if(contrasts[i]=='-'&&TwoChars=='-')
                     {
                       printf("found two - signs\n");
                       CheckContrasts=1;
                     } 
                   TwoChars=contrasts[i];
                 }
               else if(contrasts[i]=='1')
                 {
                   if(contrasts[i]=='1'&&TwoChars=='1')
                     {
                       printf("found two 1's together\n");
                       CheckContrasts=1;
                     } 
                   TwoChars=contrasts[i];
                 }
               else if(contrasts[i]==' ')
                 {
                   if(contrasts[i]==' '&&TwoChars==' ')
                     {
                       printf("found two spaces together\n");
                       CheckContrasts=1;
                     } 
                   TwoChars=contrasts[i];
                 }
               else if(contrasts[i]=='0')
                 {
                   if(contrasts[i]=='0'&&TwoChars=='0')
                     {
                       printf("found two 0's together\n");
                       CheckContrasts=1;
                     } 
                   TwoChars=contrasts[i];
                 }
               else
                 {
                   //all good
                 } 
             }
           else
             {
               printf("error at position %i character %c\n",i, contrasts[i]);
               TwoChars=contrasts[i];
               CheckContrasts=1;
             }
         }

      //Count the numbers in the string. Get columns by finding the first newline.
      for(i=0;i<=strlen(contrasts);i++)
         {
           FoundNewline=contrasts[i];
           if(FoundNewline=='\n')
             {
               iCountNewlines++;
             }
           if(contrasts[i]=='1'||contrasts[i]=='0')
             {
               iCountNumbers++;
             }
           if(iCountNewlines==1&&FoundNewline=='\n')
             {
               iCountColumns=iCountNumbers;
               FoundNewline='0';
             }
         }
      printf("Number Count = %i\n", iCountNumbers);
      printf("Columns = %i\n", iCountColumns);
      *columns=iCountColumns;

      //rows should be iCountNumbers/iCountColumns
      if(iCountColumns==0)
        {
          printf("There are no columns. If there is just one row press enter to start a newline and try again.\n");
          simple_message_dialog("If there is just one row press enter\nto start a newline and try again.");
          CheckContrasts=1;
        }
      else if(iCountNumbers%iCountColumns!=0)
        {
          printf("Are there too many numbers or incorrect numbers in a row?\n");
          simple_message_dialog("Are there too many numbers or\nincorrect numbers in a row?");
          CheckContrasts=1;
        }
      else
        {
          printf("Rows = %i\n", iCountNumbers/iCountColumns);
          *rows=iCountNumbers/iCountColumns;
        }

      return CheckContrasts;
  }







