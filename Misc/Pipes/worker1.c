
/*

    A worker program to use with driver1.c. It sends the messages back to the driver program and 
updates a label.

    gcc -Wall worker1.c -o worker1

    C. Eric Cashon

*/

#include<stdio.h>
#include<unistd.h>

int main()
  {
    fprintf(stdout, "Worker Started\n");
    fflush(stdout);
    fprintf(stderr, "Worker Started\n");
    sleep(2);
    fprintf(stdout, "Get Data\n");
    fflush(stdout);
    fprintf(stderr, "Get Data\n");
    sleep(2);
    fprintf(stdout, "Do Analysis\n");
    fflush(stdout);
    fprintf(stderr, "Do Analysis\n");
    sleep(2);
    fprintf(stdout, "Graph\n");
    fflush(stdout);
    fprintf(stderr, "Graph\n");
    sleep(2);
    fprintf(stdout, "Close Worker\n");
    fflush(stdout);
    fprintf(stderr, "Close Worker\n");
    return 0;
  }
