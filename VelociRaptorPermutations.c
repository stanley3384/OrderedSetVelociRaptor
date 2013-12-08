
/*Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
cecashon@aol.com

Some permutation testing.
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <glib.h>
#include <apop.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_sf_gamma.h>
#include "VelociRaptorUI_Validation.h"

int hash_check=0;
static void key_destroyed(gpointer data)
  {
    //printf("Got a key destroy call for %s\n", (char*)data);
    hash_check=1;
  }
void permutation_sql(int permutations, int iRadioButton, int iControlValue, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton);
static void permutation_calculations(int permutations, double data_control[], double data_test[], int control_count, int test_count, GtkTextView *textview, int *pBreakLoop, int iSeedValue, int iRandomButton);
static void generate_permutations(int permutations, int permutation_length, double data_control[], double data_test[], double mean_difference, int control_count, int test_count, int less, int greater, GtkTextView *textview, int iSeedValue, int iRandomButton);
static void generate_permutations_without_hashing(int ***perm1, int permutations, int permutation_length,  int iSeedValue, int iRandomButton);
static void generate_permutations_with_hashing(int ***perm1, int permutations, int permutation_length, int iSeedValue, int iRandomButton);


void permutation_sql(int permutations, int iRadioButton, int iControlValue, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton)
   {
     int malloc_error=0;
     int iCounter1=0;
     int iCounter2=0;
     int i=0;
     int j=0;
     int k=0;
     double temp1=0;
     int temp2=0;
     int temp3=0;
     int temp4=0;
     double temp5=0;
     double temp6=0;
     int control_count=0;
     int step_plate_count=1;
     double PlateCount=0;
     apop_data *mPermutationData1;
     gsl_vector *vPermutationData1;
     gsl_vector *vPermutationData2;
     gsl_vector *vPermutationData3;
     GtkTextBuffer *buffer;
     buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

     apop_db_open("VelociRaptorData.db");
 
     if(iRadioButton==1)
       {
          mPermutationData1=apop_query_to_data("SELECT T2.plate, T2.groups, count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(mPermutationData1==NULL) malloc_error=1; 
         vPermutationData1=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(vPermutationData1==NULL) malloc_error=1;  
         vPermutationData2=apop_query_to_vector("SELECT count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(vPermutationData2==NULL) malloc_error=1;   
         vPermutationData3=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(vPermutationData3==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==2)
       {
         mPermutationData1=apop_query_to_data("SELECT T2.plate, T2.groups, count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(mPermutationData1==NULL) malloc_error=1; 
         vPermutationData1=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(vPermutationData1==NULL) malloc_error=1;  
         vPermutationData2=apop_query_to_vector("SELECT count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(vPermutationData2==NULL) malloc_error=1;   
         vPermutationData3=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i ORDER BY T2.plate, T2.Groups asc;", iControlValue);
         if(vPermutationData3==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==3)
       {
         mPermutationData1=apop_query_to_data("SELECT T2.plate, T2.picks, count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(mPermutationData1==NULL) malloc_error=1; 
         vPermutationData1=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(vPermutationData1==NULL) malloc_error=1;  
         vPermutationData2=apop_query_to_vector("SELECT count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(vPermutationData2==NULL) malloc_error=1;   
         vPermutationData3=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(vPermutationData3==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==4)
       {
         mPermutationData1=apop_query_to_data("SELECT T2.plate, T2.picks, count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(mPermutationData1==NULL) malloc_error=1; 
         vPermutationData1=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(vPermutationData1==NULL) malloc_error=1;  
         vPermutationData2=apop_query_to_vector("SELECT count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(vPermutationData2==NULL) malloc_error=1;   
         vPermutationData3=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks asc;", iControlValue);
         if(vPermutationData3==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    apop_db_close(0);

    if(malloc_error==1)
      {
        printf("Memory allocation error while getting data from the database.\n");
      }
    else
      {
        //printf("Plate Control Test ControlMean, TestMean, MeanDifference Permutations PermutationLength ControlCount TestCount Values>=MeanDifference Mean StdDevS Side p-value Seconds\n");
        char *string;
        asprintf(&string, "Plate Control Test ControlMean TestMean AbsMeanDifference Permutations PermutationLength ControlCount TestCount CountP PermMean PermStdDevS Side p-value Seconds\n");
        gtk_text_buffer_insert_at_cursor(buffer, string, -1);
        free(string);
        for(i=0;i<mPermutationData1->matrix->size1;i++)
           {
             for(j=0;j<mPermutationData1->matrix->size2;j++)
                {
                   temp1=apop_data_get(mPermutationData1,i,j);
                   //plate number
                   if(j==0)
                     {
                       temp2=(int)temp1;
                       //printf("temp2=%i ", temp2);
                     }
                   //group number
                   if(j==1)
                     {
                       temp3=(int)temp1;
                       //printf("temp3=%i ", temp3);
                     }
                   //group count 
                   if(j==2)
                     {
                       temp4=(int)temp1;
                       //printf("temp4=%i\n", temp4);
                     } 
                 }
                
                 control_count=gsl_vector_get(vPermutationData2,step_plate_count-1);
                 //double data_control[control_count];
                 double *data_control=(double *)malloc(sizeof(double)*control_count);
                 if(step_plate_count!=temp2)
                   {
                     step_plate_count++;
                     iCounter1=iCounter1+control_count;
                   }
                  //Loop the group count number down the control data vector.
                  for(k=0;k<control_count;k++)
                     {
                       temp5=(gsl_vector_get(vPermutationData3, iCounter1));
                       //printf("%i %f\n", iCounter1, temp5);
                       data_control[k]=temp5;
                       iCounter1++;
                     }
                 iCounter1=iCounter1-control_count;
                    
                 //double data_test[temp4];
                 double *data_test=(double *)malloc(sizeof(double)*temp4);
                 //Loop the group count number down the test data vector.
                 for(k=0;k<temp4;k++)
                    {
                      temp6=(gsl_vector_get(vPermutationData1,iCounter2));
                      //printf("%i %f\n", iCounter2, temp6);
                      data_test[k]=temp6;
                      iCounter2++;
                    } 
                 //printf("%i %i %i ", temp2, 1, temp3);
                 char *string2;
                 asprintf(&string2, "%i %i %i ", temp2, iControlValue, temp3);
                 gtk_text_buffer_insert_at_cursor(buffer, string2, -1);
                 free(string2);
 
                 permutation_calculations(permutations, data_control, data_test, control_count, temp4, textview, pBreakLoop, iSeedValue, iRandomButton); 

                 free(data_test);
                 free(data_control);

                 //update progress bar.
                 gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), (double)temp2/PlateCount);
                 gchar *message=g_strdup_printf("Calculating Plate %i out of %i", temp2, (int)PlateCount);
                 gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), message);
                 g_free(message);

                 //Process pending events for UI.
                 while(gtk_events_pending())
                      {
                        gtk_main_iteration();
                      } 
                 if(*pBreakLoop==1)
                      {
                       //Break long running loop on cancel.
                       printf("Break Loop\n");
                       i=mPermutationData1->matrix->size1;
                       *pBreakLoop=0;
                       break;
                      }    
            } 
          
      }

    
     if(mPermutationData1->matrix!=NULL)gsl_matrix_free(mPermutationData1->matrix);
     if(vPermutationData1!=NULL)gsl_vector_free(vPermutationData1);
     if(vPermutationData2!=NULL)gsl_vector_free(vPermutationData2);
     if(vPermutationData3!=NULL)gsl_vector_free(vPermutationData3);      
   }
static void permutation_calculations(int permutations, double data_control[], double data_test[], int control_count, int test_count, GtkTextView *textview, int *pBreakLoop, int iSeedValue, int iRandomButton)
   {
    int greater=0;
    int less=0;
    double check_permutation_count=0;
    double control_mean=gsl_stats_mean(data_control, 1, control_count);
    double test_mean=gsl_stats_mean(data_test, 1, test_count);
    double mean_difference=fabs(control_mean-test_mean);
    int permutation_length=control_count+test_count;
    GtkTextBuffer *buffer;
    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    //check permutations for small sets. 10!=3628800. Restrict number of permutations in UI.
    if(permutation_length<=10)
      {
        check_permutation_count=gsl_sf_gamma(permutation_length+1);
      }
    else
      {
        check_permutation_count=3628800;
      }

    if(test_mean<=control_mean)less=1;
    if(test_mean>control_mean)greater=1;    

    //printf("Max number of permutations=%g\n", check_permutation_count);
    //printf("%f %f %f ", control_mean, test_mean, mean_difference);
    char *string;
    asprintf(&string, "%f %f %f ", control_mean, test_mean, mean_difference);
    gtk_text_buffer_insert_at_cursor(buffer, string, -1);
    free(string);

    if(permutations<=check_permutation_count)
      {
        generate_permutations(permutations, permutation_length, data_control, data_test, mean_difference, control_count, test_count, less, greater, textview, iSeedValue, iRandomButton);
      }
    else
      {
        printf("The maximum number of permutations is %i\n", (int)check_permutation_count);
        simple_message_dialog("The number of permutations exceeded n! for the set.");
        *pBreakLoop=1;
        //Some textview clean-up.
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gtk_text_buffer_delete(buffer, &start, &end);
      }
      
  }
static void generate_permutations(int permutations, int permutation_length, double data_control[], double data_test[], double mean_difference, int control_count, int test_count, int less, int greater, GtkTextView *textview, int iSeedValue, int iRandomButton)
  {
    int i=0;
    int j=0;
    double dtemp1=0;
    double dtemp2=0;
    double dtemp3=0;
    int counter=0;
    int malloc_error=0;
    time_t start;
    time_t end;
    GtkTextBuffer *buffer;
    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    double combine_arrays[control_count+test_count];
    for(i=0;i<control_count;i++)
       {
         combine_arrays[i]=data_control[i];
         //printf("%i ", (int)combine_arrays[i]);
       }
    for(i=control_count;i<control_count+test_count;i++)
       {
         combine_arrays[i]=data_test[i-control_count];
         //printf("%i ", (int)combine_arrays[i]);
       }
    //printf("\n");

    //int perm1[permutations][permutation_length];
    int **perm1=malloc(permutations*sizeof(int*));
    if(perm1==NULL) malloc_error=1;
      for(i=0;i<permutations;i++)
         {
           perm1[i]=malloc(permutation_length * sizeof(int));
           if(perm1[i]==NULL) malloc_error=1;
         }
    //double means[permutations];
    double *means=(double *)malloc(sizeof(double)*permutations);
    if(means==NULL) malloc_error=1;

    if(malloc_error==1)
      {
        printf("Memory allocation error while generating permutations.\n");
      }
    else
      {
        hash_check=0;
        start = time(NULL);
        
        //Use GSL to generate permutations. 9!=362,880
        if(permutation_length<=9&&permutations/gsl_sf_gamma(permutation_length+1)>0.70)
          {
            generate_permutations_without_hashing(&perm1, permutations, permutation_length, iSeedValue, iRandomButton);
          }
        else//Get permutations one at a time with hashing.
          {
            generate_permutations_with_hashing(&perm1, permutations, permutation_length, iSeedValue, iRandomButton);
          }
    
        /*Print permutation array
        printf("Permutation Array\n");
        for(i=0;i<permutations;i++)
           {
             for(j=0;j<permutation_length;j++)
                {
                  printf("%i ", perm1[i][j]);
                }
             printf("\n");
           }
         */

        //Get the difference of means.
        for(i=0;i<permutations;i++)
           {
             for(j=0;j<permutation_length;j++)
                {
                  if(j<control_count)
                    {
                      dtemp1=dtemp1+combine_arrays[perm1[i][j]];
                    }
                  else
                    {
                      dtemp2=dtemp2+combine_arrays[perm1[i][j]];
                    }
                }
             means[i]=(dtemp2/test_count)-(dtemp1/control_count);
             dtemp1=0;
             dtemp2=0;
           }

        //Count how many differences of means are greater or less than the mean_difference.
        counter=0;
        if(greater==1)
          {
            for(i=0;i<permutations;i++)
               {
                 if(means[i]>=mean_difference)
                   {
                     counter++;
                   }
               }
           }
         if(less==1)
          {
            for(i=0;i<permutations;i++)
               {
                 if(means[i]<=-mean_difference)
                   {
                     counter++;
                   }
               }
           }
        

        //printf("%i %i %i %i %i ", permutations, permutation_length, control_count, test_count, counter); 
        char *string;
        asprintf(&string, "%i %i %i %i %i ", permutations, permutation_length, control_count, test_count, counter);
        gtk_text_buffer_insert_at_cursor(buffer, string, -1);
        free(string);

        //Get the mean and stddev of the difference of means
        dtemp3=gsl_stats_mean(means, 1, permutations);
        end=time(NULL);
        if(greater==1)
          {
            //printf("%f %f %s %f %ld\n", dtemp3, gsl_stats_sd_m(means, 1, permutations, dtemp3), "greater", ((double)counter+1)/((double)permutations+1), end-start); 
            char *string2;
            asprintf(&string2, "%f %f %s %f %ld\n", dtemp3, gsl_stats_sd_m(means, 1, permutations, dtemp3), "greater", ((double)counter+1)/((double)permutations+1), end-start);
            gtk_text_buffer_insert_at_cursor(buffer, string2, -1);
            free(string2);
          }
        else
          {
            //printf("%f %f %s %f %ld\n", dtemp3, gsl_stats_sd_m(means, 1, permutations, dtemp3), "less", ((double)counter+1)/((double)permutations+1), end-start);
            char *string3;
            asprintf(&string3, "%f %f %s %f %ld\n", dtemp3, gsl_stats_sd_m(means, 1, permutations, dtemp3), "less", ((double)counter+1)/((double)permutations+1), end-start);
            gtk_text_buffer_insert_at_cursor(buffer, string3, -1);
            free(string3); 
          }
      }

      for(i=0;i<permutations; ++i)
         {
          if(perm1[i]!=NULL)free(perm1[i]);
         }
      if(perm1!=NULL)free(perm1);
      if(means!=NULL)free(means);
      
  }
//If the number of permutations is reasonable just create all the possibilities. That way there isn't 
//a bottle neck for trying to get complete sets of permutations for "small" values(<=9!). Should probably
//just create an array with permutations and directly access "rows". Give GSL permutations a try first.
//Should also figure out which one is better at a percentage of the complete permutation set. Guess
//without hashing at >0.70 for now. Still testing this out. 
static void generate_permutations_without_hashing(int ***perm1, int permutations, int permutation_length,  int iSeedValue, int iRandomButton)
  {
    int i=0;
    int j=0;
    int difference=0;
    const size_t N=permutation_length;

    gsl_rng *r;
    const gsl_rng_type *T;
    gsl_rng_env_setup();
    if(iRandomButton==1)T=gsl_rng_mt19937;
    if(iRandomButton==2)T=gsl_rng_taus2;
    if(iRandomButton==3)T=gsl_rng_ranlux389;
    r=gsl_rng_alloc(T);
    gsl_rng_set(r,iSeedValue);

    int check_permutation_count=gsl_sf_gamma(permutation_length+1);
    //int permutation_shuffled_index[check_permutation_count];
    int *permutation_shuffled_index=(int *)malloc(sizeof(int)*check_permutation_count);

    for(i=0;i<check_permutation_count;i++)
       {
         permutation_shuffled_index[i]=i;
       }

    gsl_permutation *p=gsl_permutation_alloc(N);

    gsl_permutation_init(p);
    
    gsl_ran_shuffle(r, permutation_shuffled_index, check_permutation_count, sizeof(int));

    for(i=0;i<permutations;i++)
       {
         difference=difference-permutation_shuffled_index[i];
         if(difference>0)
           {
             for(j=0;j<difference;j++)
                {
                  gsl_permutation_prev(p);
                }
           }
         if(difference<0)
           {
             for(j=0;j<abs(difference);j++)
                {
                  gsl_permutation_next(p);
                }
           }
     
          for(j=0;j<permutation_length;j++)
             {
               (*perm1)[i][j]=gsl_permutation_get(p,j);
             }
          difference=permutation_shuffled_index[i];//Current index location of p. 
       }
   
    gsl_permutation_free(p);
    free(permutation_shuffled_index);

  }
//If there are too many permutations to just store in memory create them one at a time. Make sure
//they are unique by hashing the values. 
static void generate_permutations_with_hashing(int ***perm1, int permutations, int permutation_length,  int iSeedValue, int iRandomButton)
  {
    int i=0;
    int temp1=0;
    int temp2=0;
    int counter=0;
    int malloc_error=0;
    //int temp_perm[permutation_length];
    int *temp_perm=(int *)malloc(sizeof(int)*permutation_length);
    if(temp_perm==NULL) malloc_error=1;
    GHashTable* hash = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)key_destroyed, NULL);
    if(hash==NULL) malloc_error=1;
    GString *string=g_string_new("");

    gsl_rng *r;
    const gsl_rng_type *T;
    gsl_rng_env_setup();
    if(iRandomButton==1)T=gsl_rng_mt19937;
    if(iRandomButton==2)T=gsl_rng_taus2;
    if(iRandomButton==3)T=gsl_rng_ranlux389;
    r=gsl_rng_alloc(T);
    gsl_rng_set(r,iSeedValue);
    if(r==NULL) malloc_error=1;

     for(i=0;i<permutation_length;i++)
       {
         temp_perm[i]=i;
       }
   
    if(malloc_error==1)
      {
        printf("Memory allocation error in function generate_permutations_with_hashing().");
      }
    else
      {
       do
       {
         //Generate a permutation vector and string to hash.
         for(i=0; i<permutation_length; i++)
            {
              temp1=gsl_rng_get(r)%(permutation_length);
              temp2=temp_perm[i]; 
              temp_perm[i]=temp_perm[temp1];
              temp_perm[temp1]=temp2;
            }
         for(i=0;i<permutation_length;i++)
            {
              g_string_append_printf(string, "%i", temp_perm[i]);
            }

         //Hash the string.
         g_hash_table_insert(hash, string->str, NULL);

         //Check if permutation has already been generated.
         if(hash_check==0)
           {
             //Add permutation to array.
             for(i=0;i<permutation_length;i++)
                {
                  (*perm1)[counter][i]=temp_perm[i];
                }
             counter++;
            }
         else
            {
              //printf("Duplicate Value %s\n", string->str);
              hash_check=0;
            }
         //Clear the string
         g_string_truncate(string,0);     
      }while(counter<permutations);
    //printf("There are %d keys in the hash table\n", g_hash_table_size(hash));
    }

   if(temp_perm!=NULL) free(temp_perm);
   g_string_free(string, TRUE);
   g_hash_table_destroy(hash);
   gsl_rng_free(r);
  }






