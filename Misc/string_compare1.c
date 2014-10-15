
/*
  Test code for a simple file write and retrieval.

  gcc -Wall -g string_compare1.c -o string_compare1 `pkg-config --cflags --libs gtk+-3.0`
  Valgrind ./string_compare
*/

#include<gtk/gtk.h>
#include<stdio.h>
#include<string.h>

int main()
  {
    int i=0;
    gchar *names1[]={"Fred", "Joe", "Jack", "Dave"};
    gchar compare[]="Jack";
    int strings=sizeof(names1)/sizeof(char*);
    g_print("Strings %i\n\n", strings);

    //Write names to a file for testing. 
    FILE *f = fopen("file.txt", "w");
    for(i=0;i<strings;i++)
       {
         fprintf(f, "%i,%s\n", i, names1[i]);
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
    gchar **names2=g_strsplit_set(buffer, ",\n", -1);
    i=0;
    while(names2[i])
       {
         g_print("%s\n", names2[i]);
         i++;
       }

    //Compare name and id or just index here.
    for(i=1;i<(2*strings);i+=2)
       {
         g_print("%s %s %s %i\n", names2[i-1], names2[i], compare, strcmp(names2[i], compare));
       }

    g_strfreev(names2);
  
    return 0;
  }

    		
    		
