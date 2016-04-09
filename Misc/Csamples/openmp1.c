
/*
   Test some different compiler settings and optimization. Most of the time is spent getting
the random numbers. Very little time spent summing them.

   gcc -Wall -fopenmp -O3 openmp1.c -o openmp1 -lgomp `pkg-config --cflags --libs glib-2.0`

   Look at some assembly output.
   gcc -Wall -fopenmp -O3 -S openmp1.c -o asm_openmpO3 -lgomp `pkg-config --cflags --libs glib-2.0`
   gcc -Wall -fopenmp -O2 -S openmp1.c -o asm_openmpO2 -lgomp `pkg-config --cflags --libs glib-2.0`
   gcc -Wall -fopenmp -S openmp1.c -o asm_openmp -lgomp `pkg-config --cflags --libs glib-2.0`
   
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<glib.h>

int main()
  {
    time_t t;
    srand((unsigned) time(&t));
    int i=0;
    double sum=0;

    //Request a big array for 10 million numbers. 80mb here.
    double *num1=(double*)malloc(10000000*sizeof(double));

    if(num1!=NULL)
      {
        //rand() isn't thread safe.
        for(i=0;i<10000000;i++)
          {
            num1[i]=(double)rand()/(double)RAND_MAX;
            //if(i<100) printf("%f ", num1[i]);
          }
    
        //Time parallel section.
        GTimer *time_it=g_timer_new();    
        #pragma omp parallel for private(i) reduction(+:sum)
        for(i=0;i<10000000;i++)
          {        
            sum+=num1[i]-0.5;
          }

        printf("sum %f, time %f\n", sum, g_timer_elapsed(time_it, NULL));
        g_timer_destroy(time_it);
        free(num1);
      }
    else printf("Malloc Failed\n");
 
    return 0;
  }
