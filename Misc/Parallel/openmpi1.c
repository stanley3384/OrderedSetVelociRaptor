/* 
    Test some numbers using MPI. Start four processes, sum the numbers in each process and then
add those sums together for the total sum. A little different strategy than openMP. 

    Compile with:
    mpicc -Wall openmpi1.c -o openmpi1 -lm `pkg-config --cflags --libs glib-2.0`
    Run with:
    mpirun -n 4 openmpi1

    Tested on an Ubuntu14.04 netbook with two cores. 

    C. Eric Cashon
*/

#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include<math.h>
#include<mpi.h>
#include<glib.h>

int main (int argc, char *argv[])
{
  int process_id=0;
  int processes=0;
  int i=0;
  int numbers=1000000;
  double local_sum=0;
  double sum=0;
  double mean=0;
  double local_squares=0;
  double squares=0;
  double var=0;
  double std=0;

  double *num1=(double*)malloc(numbers*sizeof(double));

  if(num1!=NULL)
    {
      GTimer *time_it1=g_timer_new();

      MPI_Init(&argc, &argv);
      MPI_Comm_size(MPI_COMM_WORLD, &processes);
      MPI_Comm_rank(MPI_COMM_WORLD, &process_id);
      printf ("process %d of %d\n", process_id, processes);

      srand(time(NULL)*process_id);
      for(i=0;i<numbers;i++)
        {
          num1[i]=(double)rand()/(double)RAND_MAX;
        }

      for(i=0;i<numbers;i++)
        {
          local_sum+=num1[i];
        }
      printf("local sum for process %d %f, mean %f\n", process_id, local_sum, local_sum/(double)numbers);

      MPI_Allreduce(&local_sum, &sum, 1, MPI_DOUBLE, MPI_SUM, MPI_COMM_WORLD);
      mean=sum/((double)numbers*processes);

      for(i=0;i<numbers;i++)
        {        
          local_squares+=(num1[i]-mean)*(num1[i]-mean);
        }
      printf("local squares for process %d %f\n", process_id, local_squares);

      MPI_Reduce(&local_squares, &squares, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

      if(process_id==0)
        {
          mean=sum/((double)numbers*processes);
          var=squares/((double)numbers*processes-1);
          std=sqrt(var);
          printf("total sum %f mean %f var %f stddev %f\n", sum, mean, var, std);
          printf("Time %f\n", g_timer_elapsed(time_it1, NULL));
        }

      free(num1);
      g_timer_destroy(time_it1);
      MPI_Finalize();
    }
  else printf("Malloc Failed\n");
 
  return 0;
}

