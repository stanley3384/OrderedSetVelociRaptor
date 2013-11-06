
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

int hash_check=0;
static void key_destroyed(gpointer data)
  {
    //printf("Got a key destroy call for %s\n", (char*)data);
    hash_check=1;
  }
void permutation_sql(int permutations, int iRadioButton, int iControlValue, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop);
static void permutation_calculations(int permutations, double data_control[], double data_test[], int control_count, int test_count, GtkTextView *textview);
static void generate_permutations_with_hashing(int permutations, int permutation_length, double data_control[], double data_test[], double mean_difference, int control_count, int test_count, int less, int greater, GtkTextView *textview);

void permutation_sql(int permutations, int iRadioButton, int iControlValue, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop)
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
        asprintf(&string, "Plate Control Test ControlMean TestMean MeanDifference Permutations PermutationLength ControlCount TestCount CountValues PermMean PermStdDevS Side p-value Seconds\n");
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
 
                 permutation_calculations(permutations, data_control, data_test, control_count, temp4, textview); 

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

    if(malloc_error==0) 
      {
        gsl_matrix_free(mPermutationData1->matrix);
        gsl_vector_free(vPermutationData1);
        gsl_vector_free(vPermutationData2);
        gsl_vector_free(vPermutationData3);
      }
      
   }
static void permutation_calculations(int permutations, double data_control[], double data_test[], int control_count, int test_count, GtkTextView *textview)
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
        generate_permutations_with_hashing(permutations, permutation_length, data_control, data_test, mean_difference, control_count, test_count, less, greater, textview);
      }
    else
      {
        printf("The maximum number of permutations is %f\n", check_permutation_count);
        GtkTextIter start, end;
        gtk_text_buffer_get_bounds(buffer, &start, &end);
        gtk_text_buffer_delete(buffer, &start, &end);
      }
      
  }
static void generate_permutations_with_hashing(int permutations, int permutation_length, double data_control[], double data_test[], double mean_difference, int control_count, int test_count, int less, int greater, GtkTextView *textview)
  {
    int i=0;
    int j=0;
    int temp1=0;
    int temp2=0;
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
    //int temp_perm[permutation_length];
    int *temp_perm=(int *)malloc(sizeof(int)*permutation_length);
    if(temp_perm==NULL) malloc_error=1;
    GHashTable* hash = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)key_destroyed, NULL);
    if(hash==NULL) malloc_error=1;
    GString *string=g_string_new("");

    gsl_rng *r;
    const gsl_rng_type *T;
    gsl_rng_env_setup();
    T=gsl_rng_default;
    r=gsl_rng_alloc(T);
    if(r==NULL) malloc_error=1;

    //Set up initial permutation.
    for(i=0;i<permutation_length;i++)
       {
         temp_perm[i]=i;
       }

    if(malloc_error==1)
      {
        printf("Memory allocation error while generating permutations.\n");
      }
    else
      {
        hash_check=0;
        start = time(NULL);
        //printf("Generate Permutation Vector and String to Hash\n");
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
                      perm1[counter][i]=temp_perm[i];
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
             means[i]=(dtemp1/control_count)-(dtemp2/test_count);
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

    if(malloc_error==0) 
      {
        for(i=0;i<permutations; ++i)
           {
            free(perm1[i]);
           }
        free(perm1);
        free(means);
        free(temp_perm);
        g_string_free(string, TRUE);
        gsl_rng_free(r);
        g_hash_table_destroy(hash);
      }
  }






