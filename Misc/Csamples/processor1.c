
/*
    Test code for getting percentage of processor use.

    gcc -Wall -D_GNU_SOURCE processor1.c -o processor1

    C. Eric Cashon
*/

#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<unistd.h>

static int procs=0;

void init_prev(unsigned long int prev[]);
void processor_percent(double results[], unsigned long int prev[]);

int main()
  {
    procs=sysconf(_SC_NPROCESSORS_ONLN);
    printf("Procs %i\n", procs);
    unsigned long int prev[4*(procs+1)];
    double results[procs+1];
    int i=0;

    init_prev(prev);
    sleep(1);

    //Get a few results.
    for(i=0;i<5;i++)
      {
        //system("head -n 3 /proc/stat");
        processor_percent(results, prev);
        printf("cpu %f cpu0 %f cpu1 %f\n", results[0], results[1], results[2]);
        sleep(1);
      }
    
    return 0;
  }
void init_prev(unsigned long int prev[])
  {
    int i=0;
    char buffer[100];
    FILE *file=fopen("/proc/stat", "r");
    for(i=0;i<procs+1;i++)
      {
        if(fgets(buffer, 100, file)==NULL) break;
        char *string=NULL;
        if(i==0) string=strdup("cpu %lu %lu %lu %lu");
        else asprintf(&string, "cpu%i %%lu %%lu %%lu %%lu", i-1);
        sscanf(buffer, string, &prev[4*i], &prev[4*i+1], &prev[4*i+2], &prev[4*i+3]);
        //printf("Start %lu %lu %lu %lu\n", prev[4*i], prev[4*i+1], prev[4*i+2], prev[4*i+3]); 
        free(string);
      }
    fclose(file);
  }
void processor_percent(double results[], unsigned long int prev[])
  {
    double percent=0;
    double total=0;
    FILE *file;
    char buffer[100];
    int i=0;
    unsigned long int cur[4*(procs+1)];

    file=fopen("/proc/stat", "r");
    for(i=0;i<procs+1;i++)
      {
        if(fgets(buffer, 100, file)==NULL) break;
        char *string=NULL;
        if(i==0) string=strdup("cpu %lu %lu %lu %lu");
        else asprintf(&string, "cpu%i %%lu %%lu %%lu %%lu", i-1);
        sscanf(buffer, string, &cur[4*i], &cur[4*i+1], &cur[4*i+2], &cur[4*i+3]);
        //printf("    %lu %lu %lu %lu\n", cur[4*i], cur[4*i+1], cur[4*i+2], cur[4*i+3]); 
        free(string);
      }
    fclose(file);

    //Get results.
    for(i=0;i<procs+1;i++)
      {
        total=(double)((cur[i*4]-prev[i*4])+(cur[i*4+1]-prev[i*4+1])+(cur[i*4+2]-prev[i*4+2]));
        percent=total;
        total+=(double)((cur[i*4+3]-prev[i*4+3]));
        percent/=total;
        percent*=100;
        results[i]=percent;
      }

    //Update prev with cur values.
    int len=4*(procs+1);
    for(i=0;i<len;i++)
      {
        prev[i]=cur[i];
      }
     
}

