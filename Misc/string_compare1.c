
/*
  Test code for a simple file write and retrieval with some shifted names.

  gcc -Wall -g string_compare1.c -o string_compare1 `pkg-config --cflags --libs gtk+-3.0`
  Valgrind ./string_compare
*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<string.h>

int main()
  {
    int i=0;
    int j=0;
    int length=0;
    char *names1[]={"Fred", "Joe", "Jack", "Dave"};
    char compare[]="Jack";
    int strings=sizeof(names1)/sizeof(char*);
    g_print("Strings %i\n\n", strings);

    //Check shifting letters.
    g_print("Shift Letters in Names\n");
    for(i=0;i<strings;i++)
       {
         g_print("%i, %s, %i ", i, names1[i], strlen(names1[i]));
         length=strlen(names1[i]);
         for(j=0;j<length;j++)
            {
              g_print("%c%u ", names1[i][j]+1, (unsigned int)names1[i][j]+1);
            }
         g_print("\n");
       }
    g_print("\n");

    //Save shifted letters to file.
    g_print("Save Shifted Name to File\n");
    FILE *f = fopen("file.txt", "w");
    for(i=0;i<strings;i++)
       {
         length=strlen(names1[i]);
         fprintf(f, "%i,", i);
         for(j=0;j<length;j++)
            {
              fprintf(f, "%c", names1[i][j]+1);
            }
         fprintf(f, "\n");
       }
    fclose(f);

    //Get names from file. Open new file pointer or else bad things happen with Valgrind.
    char buffer[1000];
    memset(buffer, '\0', 1000);
    FILE *f2=fopen("file.txt", "r");
    fread(buffer, sizeof(char), 1000, f2);
    fclose(f2);
    g_print("%s\n", buffer);

    //Split names.
    g_print("Print Split Names from File\n");
    gchar **names2=g_strsplit_set(buffer, ",\n", -1);
    i=0;
    while(names2[i])
       {
         g_print("%s\n", names2[i]);
         i++;
       }

    //Subtract one from chars to shift back.
    for(i=1;i<(2*strings);i+=2)
       {
         length=strlen(names2[i]);
         for(j=0;j<length;j++)
            {
              names2[i][j]=names2[i][j]-1;
            }
       }

    //Compare name and id or just index here.
    g_print("Names Returned and Compared\n");
    for(i=1;i<(2*strings);i+=2)
       {
         g_print("%s %s %s %i\n", names2[i-1], names2[i], compare, strcmp(names2[i], compare));
       }

    g_strfreev(names2);
  
    return 0;
  }

    		
    		
