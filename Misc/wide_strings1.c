

/*

Test some wide strings.

gcc -Wall -std=c99 wide_strings1.c -o wide_strings1

valgrind ./wide_strings1

http://stackoverflow.com/questions/15573104/wchar-t-valgrind-issue-invalid-read-of-size-8

C. Eric Cashon

*/

#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<locale.h>
#include<wchar.h>

void print_wide_chars()
 {
    printf("Wide Chars\n");
    wchar_t wc1[]={L'U', L'n', L'i', L'c', L'o', L'd', L'e', L'1', L' ', L'\u00EB', L'\0'};
    wchar_t wc2[]=L"Unicode2 \u00EB";
    wchar_t *wc3=L"Unicode3 \u00EB";
    wchar_t *wc4=(wchar_t*)malloc(11 * sizeof(wchar_t));
    wcscpy(wc4, L"Unicode4 \u00EB");
    wchar_t *wc5[]={L"Test", L"Some", L"Strings"};
    printf("WChar %lc\n", wc1[9]); 
    printf("String wc lengths %i %i %i %i\n", wcslen(wc1), wcslen(wc2), wcslen(wc3), wcslen(wc4));
    printf("String wc1 %ls\n", wc1);
    printf("String wc2 %ls\n", wc2);
    printf("String wc3 %ls\n", wc3); 
    printf("String wc4 %ls\n", wc4);
    printf("%ls %ls %ls\n", wc5[0], wc5[1], wc5[2]);
    free(wc4);
 }
void print_chars()
 {
    printf("Regular Chars\n");
    char c1[]={'T', 'e', 'x', 't', '1', '\0'};
    char c2[]="Text2";
    char *c3="Text3";
    char *c4=(char*)malloc(6 * sizeof(char));
    strcpy(c4, "Text4");
    char *c5[]={"Test", "Some", "Strings"};
    printf("String c lengths %i %i %i %i\n", strlen(c1), strlen(c2), strlen(c3), strlen(c4));
    printf("String c1 %s\n", c1);
    printf("String c2 %s\n", c2);
    printf("String c3 %s\n", c3);
    printf("String c4 %s\n", c4);
    printf("%s %s %s\n", c5[0], c5[1], c5[2]);
    free(c4); 
 }
int main()
 {
    char *locale=setlocale(LC_ALL,"");
    printf("Locale %s\n", locale); //Locale "en_US.UTF-8" on Ubuntu 12.04
    print_wide_chars();
    print_chars();
    return 0;
 }


