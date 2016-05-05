
/*
   Test some different compiler settings and optimization. Most of the time is spent getting
the random numbers. Very little time spent summing them.

   gcc -Wall -fopenmp -O3 openmp1.c -o openmp1 -lgomp -lm `pkg-config --cflags --libs glib-2.0`

   Look at some assembly output.
   gcc -Wall -fopenmp -O3 -S openmp1.c -o asm_openmpO3 -lgomp -lm `pkg-config --cflags --libs glib-2.0`
   gcc -Wall -fopenmp -O2 -S openmp1.c -o asm_openmpO2 -lgomp -lm `pkg-config --cflags --libs glib-2.0`
   gcc -Wall -fopenmp -S openmp1.c -o asm_openmp -lgomp -lm `pkg-config --cflags --libs glib-2.0`
   
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<glib.h>

int main()
  {
    time_t t;
    srand((unsigned)time(&t));
    int i=0;
    double sum=0;
    double offset=0;
    double mean=0;
    double squares=0;
    double var=0;
    double std=0;
    //Can test glib random numbers also.
    //GRand *g_rand=g_rand_new();

    /*
      Request a big array for ~10 million numbers. 80mb here. Make sure you have the space
      memory so it doesn't start paging to disc.
    */
    double *num1=(double*)malloc(10000000*sizeof(double));

    if(num1!=NULL)
      {
        GTimer *time_it1=g_timer_new(); 
        //rand() isn't thread safe.
        for(i=0;i<10000000;i++)
          {
            num1[i]=(double)rand()/(double)RAND_MAX;
            //num1[i]=g_rand_double(g_rand);
          }

        //Time parallel section.
        GTimer *time_it2=g_timer_new(); 
        //Get offset and sum of array. Offset is accumulation away from 0 or expected offset mean.   
        #pragma omp parallel for private(i) reduction(+:sum, offset)
        for(i=0;i<10000000;i++)
          {        
            offset+=num1[i]-0.5;
            sum+=num1[i];
          }

        mean=sum/10000000.0;

        //Get the squares.
        #pragma omp parallel for private(i) reduction(+:squares)
        for(i=0;i<10000000;i++)
          {        
            squares+=(num1[i]-mean)*(num1[i]-mean);
          }

        //Get the variance of the sample.
        var=squares/(double)(10000000-1);

        //Get the StdDevS.
        std=sqrt(var);

        printf("offset %f, mean %f, var %f, std %f time1 %f time2 %f\n", offset, mean, var, std, g_timer_elapsed(time_it1, NULL), g_timer_elapsed(time_it2, NULL));

        g_timer_destroy(time_it1);
        g_timer_destroy(time_it2);
        free(num1);
      }
    else printf("Malloc Failed\n");
 
    //g_rand_free(g_rand);
    return 0;
  }
