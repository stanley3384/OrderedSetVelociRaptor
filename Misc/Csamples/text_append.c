
/*
   Test code to append some text files. Make a couple of test text files to test out.

   gcc -Wall text_append.c -o text_append
   valgrind ./text_append

   C. Eric Cashon
*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>

static void append_text_files();

int main(int argc, char **argv)
  {
    append_text_files();
    return 0;
  }
static void append_text_files()
  {
    char *text_files[]={"TestText1.txt", "TestText2.txt"};
    int selected_files=2;
    int file_size=0;
    int i=0;
    FILE *combined=NULL;
 
    combined=fopen("combined.txt", "a");
    for(i=0;i<selected_files;i++)
       {
         char *pTextBuffer=NULL;
         FILE *fp=NULL;
         fp=fopen(text_files[i], "r");
         fseek(fp , 0 , SEEK_END);
         file_size=ftell(fp);
         rewind(fp);
         printf("Filesize %i\n", file_size);
         pTextBuffer=(char *)malloc(sizeof(char) * file_size);
         //Just read and write to a file. No string functions.
         fread(pTextBuffer, file_size, 1, fp);
         fwrite(pTextBuffer, file_size, 1, combined);
         fclose(fp);
         free(pTextBuffer);
       }
    fclose(combined);   
  }

