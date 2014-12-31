
/*

    Test code for putting a text file of numbers into a column. For example a text file in
a platemap or grid format into a column for loading a database. Change around as needed.

  gcc -Wall text_linearize.c -o text_linearize `pkg-config --cflags --libs gtk+-3.0`

C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<stdlib.h>
#include<string.h>

static void linearize_text_file(const gchar *text_name);

int main(int argc, char **argv)
  {
    gtk_init(&argc, &argv);
    linearize_text_file("TestText1.txt");
    return 0;
  }
static void linearize_text_file(const gchar *text_name)
  {
    gint i=0;
    GFile *TextFile=NULL;
    GFileInputStream *FileStream=NULL;
    gssize length;
    GFileInfo *FileInfo;
    gint iFileSize = -1;
    gchar *pTextBuffer=NULL;
    GString *TempBuffer=g_string_new(NULL);
    GArray *DataArray;
    gchar *pChar=NULL;
    guint32 counter=0;
    gint iTextPresent=0;
    gint iLineNumber=0;
    gdouble dTemp=0;
    gboolean new_number=FALSE;

    TextFile=g_file_new_for_path(text_name);
    FileStream=g_file_read(TextFile, NULL, NULL);
    FileInfo=g_file_input_stream_query_info (G_FILE_INPUT_STREAM(FileStream), G_FILE_ATTRIBUTE_STANDARD_SIZE, NULL, NULL);     
    iFileSize=g_file_info_get_size(FileInfo);
    g_print("Text Length = %d\n", iFileSize);
    g_object_unref(FileInfo);
    pTextBuffer=(char *) malloc(sizeof(char) * iFileSize);
    memset(pTextBuffer, 0, iFileSize);
    length=g_input_stream_read(G_INPUT_STREAM(FileStream), pTextBuffer, iFileSize, NULL, NULL);
    g_print("Length of Buffer = %i\n", length);
    DataArray=g_array_new(FALSE, FALSE, sizeof (double));
    pChar=pTextBuffer;

    //Linearize the text. Load numbers into an array.
    while(counter<(length-1))
         {
           if(g_ascii_isdigit(*pChar)||*pChar =='.'||*pChar=='-')
             {
               g_string_append_printf(TempBuffer, "%c", *pChar);
               new_number=TRUE;
             }
           else if(*pChar=='\n'||*pChar==' '|| *pChar==',')
             {
               if(new_number==TRUE)
                 {
                   dTemp=g_ascii_strtod(TempBuffer->str, NULL);
                   g_array_append_val(DataArray, dTemp);
                   g_string_truncate(TempBuffer, 0);
                   new_number=FALSE;
                 }
             }
           else
             {
               iLineNumber=DataArray[0].len + 1;
               g_print("Not a Number at Number%i\n", iLineNumber);
               iTextPresent=1;
             }
           pChar++;
           counter++;
         }
     g_free(pTextBuffer);
     g_print("Counter=%i and Length=%i\n", counter, length);

     if(iTextPresent==0)
       {
         g_print("Imported Text File\n");
         for(i=0;i<DataArray[0].len;i++)
            {
               g_print("%i %f\n", i, g_array_index(DataArray, gdouble, i));
            }
       }
     else
       {
         g_print("Unsuccessful Import\n");
       }

    g_array_free(DataArray, TRUE);    
    g_string_free(TempBuffer, TRUE);
       
  }

