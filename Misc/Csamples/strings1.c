
/*
    Some basic C strings.

    gcc -Wall strings1.c -o strings1

    C. Eric Cashon
*/

#include<stdio.h>
#include<string.h>

int main()
  {
    int i=0;
    int j=0;
    int length=0;
    char apple[]={'a', 'p', 'p', 'l', 'e', '\0'};
    char banana[]={'b', 'a', 'n', 'a', 'n', 'a', '\0'};
    char orange[]="orange";
    char tomato[]="tomato";
    char *fruits[5]={apple, banana, NULL, orange, tomato};
    char *names1[]={"Fred", "Joe", "Jack", "Dave"};
    char compare[]="Jack";
    int strings=sizeof(names1)/sizeof(char*);
    int num_fruits=sizeof(fruits)/sizeof(char*);

    printf("Print Fruits %i\n", num_fruits);
    for(i=0; i<num_fruits; i++)
       {
         if(fruits[i]!=NULL) printf("%s\n", fruits[i]);
         else printf("NULL\n");
       }
    printf("\n");

    char pear[]="pear";
    fruits[2]=pear;

    printf("Print Fruits2 %i\n", num_fruits);
    for(i=0; i<num_fruits; i++)
       {
         if(fruits[i]!=NULL) printf("%s\n", fruits[i]);
       }
    printf("\n");

    fruits[2][3]='s';
    printf("Pear to Veggie \n%s\n", fruits[2]);
    printf("\n");

    printf("Print Names Letter by Letter\n");
    for(i=0;i<strings;i++)
       {
         printf("%i, ", i);
         length=strlen(names1[i]);
         for(j=0;j<length;j++)
            {
              printf("%c", names1[i][j]);
            }
         printf("\n");
       }
    printf("\n");

    printf("Compare Names with Jack\n");
    for(i=0;i<strings;i++)
       {
         printf("%s %s %i\n", names1[i], compare, strcmp(names1[i], compare));
       }

    return 0;
  }






