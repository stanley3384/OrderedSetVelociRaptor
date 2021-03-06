
/*Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
cecashon@aol.com

Some permutation testing. Check the Test_SQL_minP.R script for an example to test results with. Different random number generators produce slightly different values.

C. Eric Cashon
*/


#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <float.h>
#include <math.h>
#include <time.h>
#include <apop.h>
#include <omp.h>
#include <sqlite3.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_randist.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_sf_gamma.h>
#include <gsl/gsl_vector.h>
#include "VelociRaptorUI_Validation.h"


int hash_check=0;
int new_plate=1;

void unadjusted_p_sql(int permutations, int iRadioButton, int iControlValue, int iTail, int iTest, int iFunction, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton);
static void unadjusted_p_data(int permutations, int iControlValue, int iTail, int iTest, apop_data *mTestGroups, gsl_vector *vTestData, gsl_vector *vControlGroups, gsl_vector *vControlData, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton, double PlateCount);

static void send_tvalues_to_database(int iControlValue, int iTest, apop_data *mTestGroups, gsl_vector *vTestData, gsl_vector *vControlGroups, gsl_vector *vControlData, double PlateCount);
//minP functions for maxT also. 
void minP_sql(int permutations, int iRadioButton, int iControlValue, int iTail, int iTest, int iFunction, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton);
static void minP_data(int permutations, int iControlValue, int iTail, int iTest, int iFunction, apop_data *mTestGroups, gsl_vector *vTestData, gsl_vector *vControlGroups, gsl_vector *vControlData, apop_data *mPvaluesSorted, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton, double PlateCount);

static void send_raw_pvalues_to_database(int rows, GArray *pValues);

static void generate_permutations_test_statistics(int comparison, int plate, int permutations, double data_control[], double data_test[], int control_count, int test_count, GArray *pValues, int iTail, int iTest, GtkTextView *textview, int ***perm1);

static void generate_permutations_test_statistics_minP(int comparison, int plate, int permutations, double data_control[], double data_test[], int control_count, int test_count, apop_data *mPvaluesSorted, int iTail, int iTest, int iFunction, GtkTextView *textview, int ***perm1, GString *PrintOutput, GPtrArray *sArray, double *monotonicity, double *prob_prev_row);

static void generate_permutations_without_hashing(int ***perm1, int permutations, int permutation_length, int iSeedValue, int iRandomButton);
static void generate_permutations_with_hashing(int ***perm1, int permutations, int permutation_length, int iSeedValue, int iRandomButton);
static void hash_key_destroyed(gpointer data);
static void print_monotone_pvalues(GtkTextView *textview, GPtrArray *sArray, double *monotonicity, apop_data *mPvaluesSorted, int iFunction);


/*
    Pull data from the database to calculate unadjusted p-values.
*/
void unadjusted_p_sql(int permutations, int iRadioButton, int iControlValue, int iTail, int iTest, int iFunction, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton)
   {
     int malloc_error=0;
     double PlateCount=0;
     apop_data *mTestGroups=NULL;
     gsl_vector *vTestData=NULL;
     gsl_vector *vControlGroups=NULL;
     gsl_vector *vControlData=NULL;
     GString *order=NULL;

     //For minP
     if(iFunction==1||iFunction==2)
       {
         order=g_string_new("asc"); 
       }
     //For maxT
     if(iFunction==3)
       {
         order=g_string_new("asc"); 
       }

     apop_db_open("VelociRaptorData.db");   
 
     if(iRadioButton==1)
       {
         mTestGroups=apop_query_to_data("SELECT T2.plate, T2.groups, count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==2)
       {
         mTestGroups=apop_query_to_data("SELECT T2.plate, T2.groups, count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups!=%i ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==3)
       {
         mTestGroups=apop_query_to_data("SELECT T2.plate, T2.picks, count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==4)
       {
         mTestGroups=apop_query_to_data("SELECT T2.plate, T2.picks, count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks!=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 GROUP BY T2.plate, T2.picks ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.picks=%i AND T2.picks!=0 ORDER BY T2.plate, T2.picks %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    apop_db_close(0);

    if(malloc_error==1)
      {
        printf("Memory allocation error in function unadjusted_p_sql().\n");
        assert(malloc_error==0);
      }
    else
      {
        if(iFunction==1||iFunction==2)
          {
            unadjusted_p_data(permutations, iControlValue, iTail, iTest, mTestGroups, vTestData, vControlGroups, vControlData, textview, progress, pBreakLoop, iSeedValue, iRandomButton, PlateCount);
          }
        if(iFunction==3)
          {
            printf("Send T-values to Database\n");
            send_tvalues_to_database(iControlValue, iTest, mTestGroups, vTestData, vControlGroups, vControlData, PlateCount);
          }
      }

    if(mTestGroups!=NULL) apop_data_free(mTestGroups);
    if(vTestData!=NULL)gsl_vector_free(vTestData);
    if(vControlGroups!=NULL)gsl_vector_free(vControlGroups);
    if(vControlData!=NULL)gsl_vector_free(vControlData);
    g_string_free(order, TRUE);

  }
static void unadjusted_p_data(int permutations, int iControlValue, int iTail, int iTest, apop_data *mTestGroups, gsl_vector *vTestData, gsl_vector *vControlGroups, gsl_vector *vControlData, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton, double PlateCount)
  {
    int iCounter1=0;
    int iCounter2=0;
    int malloc_error=0;
    int i=0;
    int j=0;
    int k=0;
    int l=0;
    double temp1=0;
    int plate=0;
    int group=0;
    int as_return=0;
    double control_value=0;
    double test_value=0;
    int control_count=0;
    int test_count=0;
    int previous_count=0;
    int step_plate_count=1;
    int check_permutations_count=0;
    int broken_loop=0;
    int **perm1=NULL;
    GtkTextBuffer *buffer;
    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

     //Need an array to store raw p-values for the minP calculations.
    GArray *pValues=g_array_sized_new(FALSE,FALSE,sizeof(gdouble),mTestGroups->matrix->size1*3);

    //printf("Plate Control Test ControlMean TestMean Difference Permutations PermutationLength ControlCount Count1 PermutationMean PermutationStdDevS Side p-value\n");
    char *string;
    as_return=asprintf(&string, "Plate Control Test ControlMean TestMean Difference Permutations PermutationLength ControlCount TestCount CountP PermMean PermStdDevS Side p-value\n");
    if(as_return!=-1)
      {
        gtk_text_buffer_insert_at_cursor(buffer, string, -1);
        free(string);
      }
    else
      {
        printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
      }

    for(i=0;i<mTestGroups->matrix->size1;i++)
       {
         for(j=0;j<mTestGroups->matrix->size2;j++)
            {
               temp1=apop_data_get(mTestGroups,i,j);
               //plate number
               if(j==0)
                 {
                   plate=(int)temp1;
                   //printf("plate=%i ", plate);
                   g_array_append_val(pValues, temp1);
                 }
               //group number
               if(j==1)
                 {
                   group=(int)temp1;
                   //printf("group=%i ", group);
                   g_array_append_val(pValues, temp1);
                 }
               //group count of test values
               if(j==2)
                 {
                   test_count=(int)temp1;
                 } 
             }            
                
             control_count=gsl_vector_get(vControlGroups,step_plate_count-1);
             double *data_control=(double *)calloc(control_count, sizeof(double));
             if(step_plate_count!=plate)
               {
                 step_plate_count++;
                 iCounter1=iCounter1+control_count;
               }
              //Loop the group count number down the control data vector.
              for(k=0;k<control_count;k++)
                 {
                   control_value=(gsl_vector_get(vControlData, iCounter1));
                   //printf("%i %f\n", iCounter1, control_value);
                   data_control[k]=control_value;
                   iCounter1++;
                  }
              iCounter1=iCounter1-control_count;
              double *data_test=(double *)calloc(test_count, sizeof(double));
              //Loop the group count number down the test data vector.
              for(k=0;k<test_count;k++)
                 {
                   test_value=(gsl_vector_get(vTestData,iCounter2));
                   //printf("%i %f\n", iCounter2, test_value);
                   data_test[k]=test_value;
                   iCounter2++;
                 }
 
              //printf("%i %i %i ", plate, iControlValue, group);
              char *string2;
              as_return=asprintf(&string2, "%i %i %i ", plate, iControlValue, group);
              if(as_return!=-1)
                {
                  gtk_text_buffer_insert_at_cursor(buffer, string2, -1);
                  free(string2); 
                }
              else
                {
                  printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
                }

              //Check if a new permutation set is needed.
              if((control_count+test_count!=previous_count))
                {
                  previous_count=control_count+test_count;
                 /*
                    Free the permutations. The permutation array is allocated and freed
                    with the number of permutations which should always be greater than
                    check_permutations_count. An ineffeciency but shouldn't seg fault.
                    This allows the code to automatically resize to a smaller permutation
                    set if needed.
                  */
                  if(perm1!=NULL)
                    {
                      for(l=0;l<permutations; l++)
                         {
                           if(perm1[l]!=NULL)free(perm1[l]);
                         }
                      if(perm1!=NULL) free(perm1);
                    }
                  //Build a new permutation set.
                  int **perm1=malloc(permutations*sizeof(int*));
                  if(perm1==NULL) malloc_error=1;
                  for(l=0;l<permutations;l++)
                     {
                       perm1[l]=malloc((control_count+test_count) * sizeof(int));
                       if(perm1[l]==NULL) malloc_error=1;
                     }
                  if(malloc_error==1)
                    {
                      printf("Memory allocation error for the permutation array.");
                      assert(malloc_error==0);
                    }
                  //check the possible permutations. 
                  if((control_count+test_count)<=9)
                    {
                      check_permutations_count=(int)gsl_sf_gamma((control_count+test_count)+1);
                      if(permutations<check_permutations_count)
                        {
                          check_permutations_count=permutations;
                        }
                    }
                  else
                    {
                      check_permutations_count=permutations;
                    }
                  if(permutations>check_permutations_count)
                    {
                      printf("Permutation count adjusted to %i for a complete set.\n", (int)check_permutations_count);
                    }
                  //Get random permutations without hashing.
                  if((control_count+test_count)<=9)
                    {
                      generate_permutations_without_hashing(&perm1, check_permutations_count, (control_count+test_count), iSeedValue, iRandomButton);
                    }
                  //Get random permutations with hashing.
                  else
                    {
                      generate_permutations_with_hashing(&perm1, check_permutations_count, (control_count+test_count), iSeedValue, iRandomButton);
                    }
                   scope_problem: generate_permutations_test_statistics(i, plate, check_permutations_count, data_control, data_test, control_count, test_count, pValues, iTail, iTest, textview, &perm1);
                   //If last set done free permutation array.
                   if(i==mTestGroups->matrix->size1-1)
                     {
                       //Free the permutations
                       printf("Free Permutation Array\n");
                       if(perm1!=NULL)
                         {
                           for(l=0;l<permutations; l++)
                              {
                                if(perm1[l]!=NULL)free(perm1[l]);
                              }
                            if(perm1!=NULL) free(perm1);
                          }
                     }
 
                  }
               else
                  {
                    //go a couple lines up. The permutation array is out of scope here.
                    goto scope_problem; 
                  }
               
               //update progress bar.
               gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), (double)plate/PlateCount);
               gchar *message=g_strdup_printf("Calculating Plate %i out of %i", plate, (int)PlateCount);
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
                     //printf("Break Loop\n");
                     i=mTestGroups->matrix->size1;
                     //reset BreakLoop in UI.
                     //*pBreakLoop=0;
                     broken_loop=1;
                     printf("Free Permutation Array\n");
                       if(perm1!=NULL)
                         {
                           for(l=0;l<permutations; l++)
                              {
                                if(perm1[l]!=NULL)free(perm1[l]);
                              }
                            if(perm1!=NULL) free(perm1);
                          }
                     break;
                    } 
     
              free(data_test);
              free(data_control);     
            } 

      //Send the p-values to the database to be sorted.
      if(broken_loop==0)
        {
          printf("Send P-values to Database\n");
          send_raw_pvalues_to_database(mTestGroups->matrix->size1, pValues);
        }
      g_array_free(pValues, TRUE);
      
   }
static void send_tvalues_to_database(int iControlValue, int iTest, apop_data *mTestGroups, gsl_vector *vTestData, gsl_vector *vControlGroups, gsl_vector *vControlData, double PlateCount)
   {
    int iCounter1=0;
    int iCounter2=0;
    int i=0;
    int j=0;
    int k=0;
    double temp1=0;
    int plate=0;
    double control_value=0;
    double test_value=0;
    double control_mean=0;
    double test_mean=0;
    double test_stat=0;
    int control_count=0;
    int test_count=0;
    int step_plate_count=1;

     //Need an array to store T-values.
    GArray *pValues=g_array_sized_new(FALSE,FALSE,sizeof(gdouble),mTestGroups->matrix->size1*3);

    for(i=0;i<mTestGroups->matrix->size1;i++)
       {
         for(j=0;j<mTestGroups->matrix->size2;j++)
            {
               temp1=apop_data_get(mTestGroups,i,j);
               //plate number
               if(j==0)
                 {
                   plate=(int)temp1;
                   //printf("plate=%i ", plate);
                   g_array_append_val(pValues, temp1);
                 }
               //group number
               if(j==1)
                 {
                   //group=(int)temp1;
                   //printf("group=%i ", group);
                   g_array_append_val(pValues, temp1);
                 }
               //group count of test values
               if(j==2)
                 {
                   test_count=(int)temp1;
                 } 
             }            
                
             control_count=gsl_vector_get(vControlGroups,step_plate_count-1);
             double *data_control=(double *)calloc(control_count, sizeof(double));
             if(step_plate_count!=plate)
               {
                 step_plate_count++;
                 iCounter1=iCounter1+control_count;
               }
              //Loop the group count number down the control data vector.
              for(k=0;k<control_count;k++)
                 {
                   control_value=(gsl_vector_get(vControlData, iCounter1));
                   //printf("%i %f\n", iCounter1, control_value);
                   data_control[k]=control_value;
                   iCounter1++;
                  }
              iCounter1=iCounter1-control_count;
              double *data_test=(double *)calloc(test_count, sizeof(double));
              //Loop the group count number down the test data vector.
              for(k=0;k<test_count;k++)
                 {
                   test_value=(gsl_vector_get(vTestData,iCounter2));
                   //printf("%i %f\n", iCounter2, test_value);
                   data_test[k]=test_value;
                   iCounter2++;
                 }
         
              control_mean=gsl_stats_mean(data_control, 1, control_count);
              test_mean=gsl_stats_mean(data_test, 1, test_count);
              //Calculate test statistic.
              if(iTest==1)
                {
                  test_stat=control_mean-test_mean;
                  g_array_append_val(pValues, test_stat);
                }
              //Welch's t-statistic. u1-u2/sqrt(var1/count1+var2/count2).
              if(iTest==2)
                {
                  test_stat=(control_mean-test_mean)/sqrt(gsl_stats_variance(data_control, 1, control_count)/control_count+gsl_stats_variance(data_test, 1, test_count)/test_count);
                  g_array_append_val(pValues, test_stat);
                }
                        
              free(data_test);
              free(data_control);     
            } 

      //Send the test statistics to the database to be sorted.
      send_raw_pvalues_to_database(mTestGroups->matrix->size1, pValues);
    
      g_array_free(pValues, TRUE);
      
   }
/*
     Pull the data from the database sorted by p-values. For the second round of permutations for minP.
     This function is also for maxT.
*/
void minP_sql(int permutations, int iRadioButton, int iControlValue, int iTail, int iTest, int iFunction, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton)
  {
     int malloc_error=0;
     double PlateCount=0;
     apop_data *mTestGroups=NULL;
     gsl_vector *vTestData=NULL;
     gsl_vector *vControlGroups=NULL;
     gsl_vector *vControlData=NULL;
     apop_data *mPvaluesSorted=NULL;
     GString *order=NULL;
     GString *maxTsort=NULL;

     //For minP
     if(iFunction==2)
       {
         order=g_string_new("desc"); 
         maxTsort=g_string_new("T2.pValue");
       }
     //For maxT
     if(iFunction==3)
       { 
         if(iTail==1)
           {
             order=g_string_new("asc"); 
             maxTsort=g_string_new("abs(T2.pValue)");
           }
         else if(iTail==2)
           {
             order=g_string_new("desc"); 
             maxTsort=g_string_new("T2.pValue");
           }
         else if(iTail==3)
           {
             order=g_string_new("asc"); 
             maxTsort=g_string_new("T2.pValue");
           }
       }

     apop_db_open("VelociRaptorData.db");   
     //Order the test values by the p-values or t-values.
     if(iRadioButton==1)
       {
         mTestGroups=apop_query_to_data("SELECT T4.plate1, T4.groups1, count(T4.groups1) FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Groups AS Groups1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Groups=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Groups1!=%i GROUP BY T4.plate1, T4.Groups1 ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T3.data FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Groups AS Groups1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Groups=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Groups1!=%i ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         //Don't have to worry about ordering the control values. Only one per plate.
         vControlGroups=apop_query_to_vector("SELECT count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         mPvaluesSorted=apop_query_to_data("SELECT plate, pValue FROM TempPvalues ORDER BY Plate, pValue %s;", order->str);
         if(mPvaluesSorted==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==2)
       {
         mTestGroups=apop_query_to_data("SELECT T4.plate1, T4.groups1, count(T4.groups1) FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Groups AS Groups1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Groups=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Groups1!=%i GROUP BY T4.plate1, T4.Groups1 ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T3.percent FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Groups AS Groups1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Groups=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Groups1!=%i ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i GROUP BY T2.plate, T2.Groups ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Groups=%i ORDER BY T2.plate, T2.Groups %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         mPvaluesSorted=apop_query_to_data("SELECT plate, pValue FROM TempPvalues ORDER BY Plate, pValue %s;", order->str);
         if(mPvaluesSorted==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==3)
       {
         mTestGroups=apop_query_to_data("SELECT T4.plate1, T4.Picks1, count(T4.Picks1) FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Picks AS Picks1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Picks=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Picks1!=%i AND T4.Picks1!=0 GROUP BY T4.plate1, T4.Picks1 ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T3.data FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Picks AS Picks1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Picks=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Picks1!=%i AND T4.Picks1!=0 ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.Picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks=%i AND T2.Picks!=0 GROUP BY T2.plate, T2.Picks ORDER BY T2.plate, T2.Picks %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks=%i AND T2.Picks!=0 ORDER BY T2.plate, T2.Picks %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         mPvaluesSorted=apop_query_to_data("SELECT plate, pValue FROM TempPvalues ORDER BY Plate, pValue %s;", order->str);
         if(mPvaluesSorted==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    if(iRadioButton==4)
       {
         mTestGroups=apop_query_to_data("SELECT T4.plate1, T4.Picks1, count(T4.Picks1) FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Picks AS Picks1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Picks=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Picks1!=%i AND T4.Picks1!=0 GROUP BY T4.plate1, T4.Picks1 ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(mTestGroups==NULL) malloc_error=1; 
         vTestData=apop_query_to_vector("SELECT T3.Percent FROM data T3, (SELECT T1.KeyID AS KeyID1, T1.Plate AS Plate1, T1.Picks AS Picks1, %s AS pValue1 FROM aux T1, temppvalues T2 WHERE T1.Plate=T2.Plate AND T1.Picks=T2.Groups) T4 WHERE T3.KeyID=T4.KeyID1 AND T4.Picks1!=%i AND T4.Picks1!=0 ORDER BY T4.plate1, T4.pValue1 %s;", maxTsort->str, iControlValue, order->str);
         if(vTestData==NULL) malloc_error=1;  
         vControlGroups=apop_query_to_vector("SELECT count(T2.Picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks=%i AND T2.Picks!=0 GROUP BY T2.plate, T2.Picks ORDER BY T2.plate, T2.Picks %s;", iControlValue, order->str);
         if(vControlGroups==NULL) malloc_error=1;   
         vControlData=apop_query_to_vector("SELECT T1.Percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks=%i AND T2.Picks!=0 ORDER BY T2.plate, T2.Picks %s;", iControlValue, order->str);
         if(vControlData==NULL) malloc_error=1;
         mPvaluesSorted=apop_query_to_data("SELECT plate, pValue FROM TempPvalues ORDER BY Plate, pValue %s;", order->str);
         if(mPvaluesSorted==NULL) malloc_error=1;
         PlateCount=apop_query_to_float("SELECT max(plate) FROM aux;");
       }
    apop_db_close(0);

    if(malloc_error==1)
      {
        printf("Memory allocation error in function minP_sql().\n");
        assert(malloc_error==0);
      }
    else
      {
        minP_data(permutations, iControlValue, iTail, iTest, iFunction, mTestGroups, vTestData, vControlGroups, vControlData, mPvaluesSorted, textview, progress, pBreakLoop, iSeedValue, iRandomButton, PlateCount);
      }

    if(mTestGroups!=NULL) apop_data_free(mTestGroups);
    if(vTestData!=NULL)gsl_vector_free(vTestData);
    if(vControlGroups!=NULL)gsl_vector_free(vControlGroups);
    if(vControlData!=NULL)gsl_vector_free(vControlData);
    if(mPvaluesSorted!=NULL) apop_data_free(mPvaluesSorted);
    g_string_free(order, TRUE);
    g_string_free(maxTsort, TRUE);

  }
static void minP_data(int permutations, int iControlValue, int iTail, int iTest, int iFunction, apop_data *mTestGroups, gsl_vector *vTestData, gsl_vector *vControlGroups, gsl_vector *vControlData, apop_data *mPvaluesSorted, GtkTextView *textview, GtkProgressBar *progress, int *pBreakLoop, int iSeedValue, int iRandomButton, double PlateCount)
  {
    int malloc_error=0;
    int iCounter1=0;
    int iCounter2=0;
    int i=0;
    int j=0;
    int k=0;
    int l=0;
    double temp1=0;
    int plate=0;
    int group=0;
    double control_value=0;
    double test_value=0;
    int control_count=0;
    int test_count=0;
    int previous_count=0;
    int step_plate_count=1;
    int check_permutations_count=0;
    int broken_loop=0;
    int **perm1=NULL;
    GString *PrintOutput=g_string_new(NULL);
    GPtrArray *sArray = g_ptr_array_sized_new(mPvaluesSorted->matrix->size1);
    
    double *monotonicity=(double *)calloc(mPvaluesSorted->matrix->size1, sizeof(double));
    double *prob_prev_row=(double *)calloc(permutations, sizeof(double));
    if(prob_prev_row==NULL) malloc_error=1;

    //Initialize prob_prev_row to 1 for minP.
    if(iFunction==2)
      {
        for(i=0;i<permutations;i++)
           {
             prob_prev_row[i]=1.0;
           }
      }
    //Initialize prob_prev_row for maxT.
    if(iFunction==3)
      {
        if(iTail==3)
          {
            for(i=0;i<permutations;i++)
               {
                 prob_prev_row[i]=DBL_MAX;
               }
          }
        else
          {
            for(i=0;i<permutations;i++)
               {
                 prob_prev_row[i]=-DBL_MAX;
               }
          }
      } 

    for(i=0;i<mTestGroups->matrix->size1;i++)
       { 
         for(j=0;j<mTestGroups->matrix->size2;j++)
            {
               temp1=apop_data_get(mTestGroups,i,j);
               //plate number
               if(j==0)
                 {
                   plate=(int)temp1;
                   //printf("plate=%i ", plate);
                 }
               //group number
               if(j==1)
                 {
                   group=(int)temp1;
                   //printf("group=%i ", group);
                 }
               //group count for test values
               if(j==2)
                 {
                   test_count=(int)temp1;
                 } 
             }            
                
             control_count=gsl_vector_get(vControlGroups,step_plate_count-1);
             double *data_control=(double *)calloc(control_count, sizeof(double));
             if(step_plate_count!=plate)
               {
                 step_plate_count++;
                 iCounter1=iCounter1+control_count;
               }
              //Loop the group count number down the control data vector.
              for(k=0;k<control_count;k++)
                 {
                   control_value=(gsl_vector_get(vControlData, iCounter1));
                   //printf("%i %f\n", iCounter1, control_value);
                   data_control[k]=control_value;
                   iCounter1++;
                  }
              iCounter1=iCounter1-control_count;
                    
              double *data_test=(double *)calloc(test_count, sizeof(double));
              //Loop the group count number down the test data vector.
              for(k=0;k<test_count;k++)
                 {
                   test_value=(gsl_vector_get(vTestData,iCounter2));
                   //printf("%i %f\n", iCounter2, test_value);
                   data_test[k]=test_value;
                   iCounter2++;
                 } 
              //printf("%i %i %i ", plate, iControlValue, group);
              g_string_append_printf(PrintOutput, "%i %i %i ", plate, iControlValue, group); 

              //Check if a new permutation set is needed.
              if((control_count+test_count)!=previous_count)
                {
                  previous_count=control_count+test_count;
                  //Free the permutations
                  if(perm1!=NULL)
                    {
                      for(l=0;l<permutations; l++)
                         {
                           if(perm1[l]!=NULL)free(perm1[l]);
                         }
                      if(perm1!=NULL) free(perm1);
                    }
                  /*
                    Free the permutations. The permutation array is allocated and freed
                    with the number of permutations which should always be greater than
                    check_permutations_count. An ineffeciency but shouldn't seg fault.
                    This allows the code to automatically resize to a smaller permutation
                    set if needed.
                  */
                  int **perm1=malloc(permutations*sizeof(int*));
                  if(perm1==NULL) malloc_error=1;
                  for(l=0;l<permutations;l++)
                     {
                       perm1[l]=malloc((control_count+test_count) * sizeof(int));
                       if(perm1[l]==NULL) malloc_error=1;
                     }
                  if(malloc_error==1)
                    {
                      printf("Memory allocation error for the permutation array.");
                      assert(malloc_error==0);
                    }
                  //check possible number of permutations. 
                  if((control_count+test_count)<=9)
                    {
                      check_permutations_count=(int)gsl_sf_gamma((control_count+test_count)+1);
                      if(permutations<check_permutations_count)
                        {
                          check_permutations_count=permutations;
                        }
                    }
                  else
                    {
                      check_permutations_count=permutations;
                    }
                  if(permutations>check_permutations_count)
                    {
                      printf("Permutation count adjusted to %i for a complete set.\n", (int)check_permutations_count);
                    }
                  //Get random permutations without hashing.
                  if((control_count+test_count)<=9)
                    {
                      generate_permutations_without_hashing(&perm1, check_permutations_count, (control_count+test_count), iSeedValue, iRandomButton);
                    }
                  //Get random permutations with hashing.
                  else
                    {
                      generate_permutations_with_hashing(&perm1, check_permutations_count, (control_count+test_count), iSeedValue, iRandomButton);
                    }
                   scope_problem2: generate_permutations_test_statistics_minP(i, plate, check_permutations_count, data_control, data_test, control_count, test_count, mPvaluesSorted, iTail, iTest, iFunction, textview, &perm1, PrintOutput, sArray, monotonicity, prob_prev_row);
                   //If the last set is done, free the permutation array. 
                   if(i==mTestGroups->matrix->size1-1)
                     {
                       //Free the permutations
                       printf("Free Permutation Array\n");
                       if(perm1!=NULL)
                         {
                           for(l=0;l<permutations; l++)
                              {
                                if(perm1[l]!=NULL)free(perm1[l]);
                              }
                            if(perm1!=NULL) free(perm1);
                          }
                      }
                }
              else
                {
                  //Go a couple lines up. The permutation array is out of scope here.
                  goto scope_problem2;
                }

              //update progress bar.
               gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), (double)plate/PlateCount);
               gchar *message=g_strdup_printf("Calculating Plate %i out of %i", plate, (int)PlateCount);
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
                     //printf("Break Loop\n");
                     i=mTestGroups->matrix->size1;
                     *pBreakLoop=0;
                     broken_loop=1;
                     printf("Free Permutation Array\n");
                       if(perm1!=NULL)
                         {
                           for(l=0;l<permutations; l++)
                              {
                                if(perm1[l]!=NULL)free(perm1[l]);
                              }
                            if(perm1!=NULL) free(perm1);
                          }
                     break;
                    } 
  
              free(data_test);
              free(data_control);     
         } 

     //Enforce monotonicity.
     if(broken_loop==0)
       {
         print_monotone_pvalues(textview, sArray, monotonicity, mPvaluesSorted, iFunction);
       }

     if(monotonicity!=NULL) free(monotonicity);
     if(prob_prev_row!=NULL) free(prob_prev_row);
     g_string_free(PrintOutput, TRUE);
     g_ptr_array_free(sArray, TRUE);  

  }  
/*
    Send unadjusted p-values to the database so that the data can be re-sorted in the order
    of the p-values for the minP calculation.
*/
static void send_raw_pvalues_to_database(int rows, GArray *pValues)
  {
    int i=0;
    int j=0;
    sqlite3 *handle;
    sqlite3_stmt *stmt1;

    sqlite3_open("VelociRaptorData.db",&handle);
      
    sqlite3_exec(handle, "BEGIN;", NULL, NULL, NULL);
    sqlite3_exec(handle, "DROP TABLE IF EXISTS TempPvalues;", NULL, NULL, NULL);
    sqlite3_exec(handle, "CREATE TABLE TempPvalues(Plate INTEGER, Groups INTEGER, pValue REAL);", NULL, NULL, NULL);
    sqlite3_prepare(handle, "INSERT INTO TempPvalues(Plate,Groups,pValue) VALUES(?,?,?);", -1, &stmt1, 0);

    for(i=0;i<rows;i++)
         {
           for(j=0;j<3;j++)
              {
                if(j==0)sqlite3_bind_int(stmt1, 1, (int)g_array_index(pValues, gdouble, 3*i+j));
                if(j==1)sqlite3_bind_int(stmt1, 2, (int)g_array_index(pValues, gdouble, 3*i+j));
                if(j==2)sqlite3_bind_double(stmt1, 3, g_array_index(pValues, gdouble, 3*i+j));
              }
           sqlite3_step(stmt1);
           sqlite3_reset(stmt1); 
         }
     sqlite3_exec(handle, "COMMIT;", NULL, NULL, NULL);
     sqlite3_finalize(stmt1);

     sqlite3_close(handle);
     printf("New Table Built\n");
  }
static void generate_permutations_test_statistics(int comparison, int plate, int permutations, double data_control[], double data_test[], int control_count, int test_count, GArray *pValues, int iTail, int iTest, GtkTextView *textview, int ***perm1)
  {
    int i=0;
    int j=0;
    double dPermutationMean=0;
    int counter=0;
    double pValueTemp=0;
    int malloc_error=0;
    int as_return=0;
    double control_mean=gsl_stats_mean(data_control, 1, control_count);
    double test_mean=gsl_stats_mean(data_test, 1, test_count);
    int permutation_length=control_count+test_count;
    double mean_difference=0;
    GtkTextBuffer *buffer;
    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    
    //Calculate test statistic.
    if(iTest==1)
      {
       mean_difference=test_mean-control_mean;
      }
    //Welch's t-statistic. u1-u2/sqrt(var1/count1+var2/count2).
    if(iTest==2)
      {
        mean_difference=(test_mean-control_mean)/sqrt(gsl_stats_variance(data_control, 1, control_count)/control_count+gsl_stats_variance(data_test, 1, test_count)/test_count);
      }

    double dControl[control_count];
    double dTest[test_count];
    double combine_arrays[control_count+test_count];

    for(i=0;i<control_count;i++)
       {
         combine_arrays[i]=data_control[i];
       }
    for(i=control_count;i<control_count+test_count;i++)
       {
         combine_arrays[i]=data_test[i-control_count];
       }
      
    //Array for permutation test statistics.
    double *means=(double *)calloc(permutations, sizeof(double));
    if(means==NULL) malloc_error=1;
    
    if(malloc_error==1)
      {
        printf("Memory allocation error in function generate_permutation_test_statistics().\n");
        assert(malloc_error==0);
      }
    else
      {
        hash_check=0;

        //Get the test statistic for each random permutation.
        for(i=0;i<permutations;i++)
           {
             for(j=0;j<permutation_length;j++)
                {
                  if(j<control_count)
                    {
                      dControl[j]=combine_arrays[(*perm1)[i][j]];
                    }
                  else
                    {
                      dTest[j-control_count]=combine_arrays[(*perm1)[i][j]];
                    }
                }
             if(iTest==1)
               {
                 means[i]=gsl_stats_mean(dTest, 1, test_count)-gsl_stats_mean(dControl, 1, control_count); 
               }
             if(iTest==2)
               {
                 means[i]=(gsl_stats_mean(dTest, 1, test_count)-gsl_stats_mean(dControl, 1, control_count))/sqrt(gsl_stats_variance(dTest, 1, test_count)/test_count+gsl_stats_variance(dControl, 1, control_count)/control_count);
               }
           }

        //Count test statistics in tails.
        counter=0;
        if(iTail==1)//abs
          {
            #pragma omp parallel for private(i) reduction(+:counter)
            for(i=0;i<permutations;i++)
               {
                 if(fabs(means[i])>=fabs(mean_difference))
                   {
                     counter++;
                   }
               }
           }
        if(iTail==2)//upper
          {
            #pragma omp parallel for private(i) reduction(+:counter)
            for(i=0;i<permutations;i++)
               {
                 if(means[i]>=mean_difference)
                   {
                     counter++;
                   }
               }
           }
         if(iTail==3)//lower
          {
            #pragma omp parallel for private(i) reduction(+:counter)
            for(i=0;i<permutations;i++)
               {
                 if(means[i]<=mean_difference)
                   {
                     counter++;
                   }
               }
           }
        

        //printf("%f %f %f ", control_mean, test_mean, mean_difference);
        char *string;
        as_return=asprintf(&string, "%f %f %f ", control_mean, test_mean, mean_difference);
        if(as_return!=-1)
          {
            gtk_text_buffer_insert_at_cursor(buffer, string, -1);
            free(string);
          }
        else
          {
            printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
          }

        //printf("%i %i %i %i %i ", permutations, permutation_length, control_count, test_count, counter);
        char *string2;
        as_return=asprintf(&string2, "%i %i %i %i %i ", permutations, permutation_length, control_count, test_count, counter);
        if(as_return!=-1)
          {
            gtk_text_buffer_insert_at_cursor(buffer, string2, -1);
            free(string2);
          }
        else
          {
            printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
          } 

        //Get the mean of the permutation test statistics.
        dPermutationMean=gsl_stats_mean(means, 1, permutations);
        //Calculate p-value.
        pValueTemp=((double)counter+1)/((double)permutations+1);
        g_array_append_val(pValues, pValueTemp);

        if(iTail==1)//abs
          {
            //printf("%f %f %s %f\n", dPermutationMean, gsl_stats_sd_m(means, 1, permutations, dPermutationMean), "abs", pValueTemp);
            char *string3;
            as_return=asprintf(&string3, "%f %f %s %f\n", dPermutationMean, gsl_stats_sd_m(means, 1, permutations, dPermutationMean), "abs", pValueTemp);
            if(as_return!=-1)
              {
                gtk_text_buffer_insert_at_cursor(buffer, string3, -1);
                free(string3);
              }
            else
              {
                printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
              }
                  
          }
        if(iTail==2)//upper
          {
            //printf("%f %f %s %f\n", dPermutationMean, gsl_stats_sd_m(means, 1, permutations, dPermutationMean), "upper", pValueTemp);
            char *string4;
            as_return=asprintf(&string4, "%f %f %s %f\n", dPermutationMean, gsl_stats_sd_m(means, 1, permutations, dPermutationMean), "upper", pValueTemp);
            if(as_return!=-1)
              {
                gtk_text_buffer_insert_at_cursor(buffer, string4, -1);
                free(string4);
              }
            else
              {
                printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
              } 
          }
        if(iTail==3)//lower
          {
            //printf("%f %f %s %f\n", dPermutationMean, gsl_stats_sd_m(means, 1, permutations, dPermutationMean), "lower", pValueTemp);
            char *string5;
            as_return=asprintf(&string5, "%f %f %s %f\n", dPermutationMean, gsl_stats_sd_m(means, 1, permutations, dPermutationMean), "lower", pValueTemp);
            if(as_return!=-1)
              {
                gtk_text_buffer_insert_at_cursor(buffer, string5, -1);
                free(string5);
              }
            else
              {
                printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
              } 
          }
       }

      if(means!=NULL) free(means);
      
  }
/*
   Second pass through the permutations to calculate the minP adjusted p-values.
*/
static void generate_permutations_test_statistics_minP(int comparison, int plate, int permutations, double data_control[], double data_test[], int control_count, int test_count, apop_data *mPvaluesSorted, int iTail, int iTest, int iFunction, GtkTextView *textview, int ***perm1, GString *PrintOutput, GPtrArray *sArray, double *monotonicity, double *prob_prev_row)
  {
    int i=0;
    int j=0;
    double dPermutationMean=0;
    int counter=0;
    int counter2=0;
    int malloc_error=0;
    double rawP=0;
    double adjP=0;
    double control_mean=gsl_stats_mean(data_control, 1, control_count);
    double test_mean=gsl_stats_mean(data_test, 1, test_count);
    int permutation_length=control_count+test_count;
    double difference=0;
    
    //Calculate difference.
    if(iTest==1)
      {
        difference=test_mean-control_mean;
      }
    //Welch's t-statistic. u1-u2/sqrt(var1/count1+var2/count2).
    if(iTest==2)
      {
        difference=(test_mean-control_mean)/sqrt(gsl_stats_variance(data_control, 1, control_count)/control_count+gsl_stats_variance(data_test, 1, test_count)/test_count);
      }

    double dControl[control_count];
    double dTest[test_count];
    double combine_arrays[control_count+test_count];

    for(i=0;i<control_count;i++)
       {
         combine_arrays[i]=data_control[i];
       }
    for(i=control_count;i<control_count+test_count;i++)
       {
         combine_arrays[i]=data_test[i-control_count];
       }

    //Index for sorted test statistics.
    size_t *index=(size_t *)calloc(permutations, sizeof(size_t));
    if(index==NULL) malloc_error=1;
    //Array for probabilities.
    double *prob=(double *)calloc(permutations, sizeof(double));
    if(prob==NULL) malloc_error=1;
    //Array for minP values.
    double *minP=(double *)calloc(permutations, sizeof(double));
    if(minP==NULL) malloc_error=1;    
    //Array for permutation test statistics.
    double *perm_test_stat=(double *)calloc(permutations, sizeof(double));
    if(perm_test_stat==NULL) malloc_error=1;

    if(malloc_error==1)
      {
        printf("Memory allocation error in function generate_permutations_test_statistics_minP().\n");
        assert(malloc_error==0);
      }
    else
      {
        hash_check=0;
    
        //Get the test statistic for each random permutation.
        for(i=0;i<permutations;i++)
           {
             for(j=0;j<permutation_length;j++)
                {
                  if(j<control_count)
                    {
                      dControl[j]=combine_arrays[(*perm1)[i][j]];
                    }
                  else
                    {
                      dTest[j-control_count]=combine_arrays[(*perm1)[i][j]];
                    }
                }
             if(iTest==1)
               {
                 perm_test_stat[i]=gsl_stats_mean(dTest, 1, test_count)-gsl_stats_mean(dControl, 1, control_count);
               }
             if(iTest==2)
               {
                 perm_test_stat[i]=(gsl_stats_mean(dTest, 1, test_count)-gsl_stats_mean(dControl, 1, control_count))/sqrt(gsl_stats_variance(dTest, 1, test_count)/test_count+gsl_stats_variance(dControl, 1, control_count)/control_count);
               }
           }

        //Count test statistics in tails.
        counter=0;
        if(iTail==1)//abs
          {
            #pragma omp parallel for private(i) reduction(+:counter)
            for(i=0;i<permutations;i++)
               {
                 if(fabs(perm_test_stat[i])>=fabs(difference))
                   {
                     counter++;
                   }
               }
          }
        if(iTail==2)//upper
          {
            #pragma omp parallel for private(i) reduction(+:counter)
            for(i=0;i<permutations;i++)
               {
                 if(perm_test_stat[i]>=difference)
                   {
                     counter++;
                   }
               }
           }
         if(iTail==3)//lower
          {
            #pragma omp parallel for private(i) reduction(+:counter)
            for(i=0;i<permutations;i++)
               {
                 if(perm_test_stat[i]<=difference)
                   {
                     counter++;
                   }
               }
           }

        rawP=((double)counter+1)/((double)permutations+1);

        if(iFunction==2)//minP
          {
            //Indirect sort and get the probabilities. Account for ties???.
            gsl_sort_index(index, perm_test_stat, 1, permutations);
            #pragma omp parallel for private(i)
            for(i=0;i<permutations;i++)
               {
                 prob[index[i]]=(double)(i)/(double)permutations;
               }
            
            //Calculate minP array.
            if((int)apop_data_get(mPvaluesSorted,comparison,0)==new_plate)
              {
                #pragma omp parallel for private(i)
                for(i=0;i<permutations;i++)
                   {
                     minP[i]=fmin(prob_prev_row[i], prob[i]);
                     prob_prev_row[i]=minP[i];
                   }
              }
            else
              {
                #pragma omp parallel for private(i)
                for(i=0;i<permutations;i++)
                   {
                     minP[i]=fmin(1.0, prob[i]);
                     prob_prev_row[i]=1.0;
                   }
                new_plate=(int)apop_data_get(mPvaluesSorted,comparison,0);
              }
  
            counter2=0;
            //printf("\n%f %i\n", apop_data_get(mPvaluesSorted, comparison, 1)), comparison);
            #pragma omp parallel for private(i) reduction(+:counter2)
            for(i=0;i<permutations;i++)
               {
                 if(minP[i]<=apop_data_get(mPvaluesSorted, comparison, 1))
                   {
                     counter2++;
                   }
               }
          } 

        if(iFunction==3)//maxT
          {
            //Calculate maxT array.
            if((int)apop_data_get(mPvaluesSorted,comparison,0)==new_plate)
              {
                if(iTail==1)//abs
                  {
                    #pragma omp parallel for private(i)
                    for(i=0;i<permutations;i++)
                       {
                         minP[i]=fmax(prob_prev_row[i], fabs(perm_test_stat[i]));
                         prob_prev_row[i]=minP[i];
                       }
                  }
                if(iTail==2)//upper
                  {
                    #pragma omp parallel for private(i)
                    for(i=0;i<permutations;i++)
                       {
                         minP[i]=fmax(prob_prev_row[i], perm_test_stat[i]);
                         prob_prev_row[i]=minP[i];
                       }
                  }
                if(iTail==3)//lower
                  {
                    #pragma omp parallel for private(i)
                    for(i=0;i<permutations;i++)
                       {
                         minP[i]=fmin(prob_prev_row[i], perm_test_stat[i]);
                         prob_prev_row[i]=minP[i];
                       }
                  }
              }
            else
              {
                if(iTail==3)
                  {
                    #pragma omp parallel for private(i)
                    for(i=0;i<permutations;i++)
                       {
                         minP[i]=fmin(DBL_MAX, fabs(perm_test_stat[i]));
                         prob_prev_row[i]=DBL_MAX;
                       }
                  }
                else
                  {
                    #pragma omp parallel for private(i)
                    for(i=0;i<permutations;i++)
                       {
                         minP[i]=fmax(-DBL_MAX, fabs(perm_test_stat[i]));
                         prob_prev_row[i]=-DBL_MAX;
                       }
                   }
                new_plate=(int)apop_data_get(mPvaluesSorted,comparison,0);
              }
 
            //Calculate the adjusted maxT p-value.
            counter2=0;
            //printf("\n%f %i\n", apop_data_get(mPvaluesSorted, comparison, 1)), comparison);
            if(iTail==1)//abs
              {
                #pragma omp parallel for private(i) reduction(+:counter2)
                for(i=0;i<permutations;i++)
                   {
                     if(minP[i]>=fabs(difference))
                       {
                         counter2++;
                       }
                   }
              }
            if(iTail==2)//upper
              {
                #pragma omp parallel for private(i) reduction(+:counter2)
                for(i=0;i<permutations;i++)
                   {
                     if(minP[i]>=difference)
                       {
                         counter2++;
                       }
                   }
              }
            if(iTail==3)//lower
              {
                #pragma omp parallel for private(i) reduction(+:counter2)
                for(i=0;i<permutations;i++)
                   {
                     if(minP[i]<=difference)
                       {
                         counter2++;
                       }
                   }
              }
              
          }        
  
        //Adjusted p-value.
        adjP=((double)counter2+1)/((double)permutations+1);

        //Save the p-value for monotonicity.
        monotonicity[comparison]=adjP;
        
        //Get the mean of the permutation test statistic.
        dPermutationMean=gsl_stats_mean(perm_test_stat, 1, permutations);

        //printf("%f %f %f ", control_mean, test_mean, difference);
        g_string_append_printf(PrintOutput, "%f %f %f ", control_mean, test_mean, difference);

        //printf("%i %i %i %i %i ", permutations, permutation_length, control_count, test_count, counter);
        g_string_append_printf(PrintOutput, "%i %i %i %i %i ", permutations, permutation_length, control_count, test_count, counter);

        //Store the strings so that they can be printed with the monotone p-values.
        if(iTail==1)//abs
          {
            //printf("%i %f %f %s %f ", counter2, dPermutationMean, gsl_stats_sd_m(perm_test_stat, 1, permutations, dPermutationMean), "abs", ((double)counter+1)/((double)permutations+1));
             g_string_append_printf(PrintOutput, "%i %f %f %s %f %f ", counter2, dPermutationMean, gsl_stats_sd_m(perm_test_stat, 1, permutations, dPermutationMean), "abs", rawP, adjP);
          }
        if(iTail==2)//upper
          {
            //printf("%i %f %f %s %f ", counter2, dPermutationMean, gsl_stats_sd_m(perm_test_stat, 1, permutations, dPermutationMean), "upper", ((double)counter+1)/((double)permutations+1));
            g_string_append_printf(PrintOutput, "%i %f %f %s %f %f ", counter2, dPermutationMean, gsl_stats_sd_m(perm_test_stat, 1, permutations, dPermutationMean), "upper", rawP, adjP); 
          }
        if(iTail==3)//lower
          {
            //printf("%i %f %f %s %f ", counter2, dPermutationMean, gsl_stats_sd_m(perm_test_stat, 1, permutations, dPermutationMean), "lower", ((double)counter+1)/((double)permutations+1));
            g_string_append_printf(PrintOutput, "%i %f %f %s %f %f ", counter2, dPermutationMean, gsl_stats_sd_m(perm_test_stat, 1, permutations, dPermutationMean), "lower", rawP, adjP); 
          }
       }

      g_ptr_array_add(sArray, g_strdup(PrintOutput->str));
      g_string_truncate(PrintOutput,0); 

      if(index!=NULL) free(index);
      if(minP!=NULL) free(minP);
      if(prob!=NULL) free(prob);
      if(perm_test_stat!=NULL) free(perm_test_stat);

  }
/*
   If the number of permutations is reasonable just create all the possibilities and get a random
   subset. That way there isn't a bottle neck for trying to get complete sets of permutations for "small"
   values <=9!. 
*/
static void generate_permutations_without_hashing(int ***perm1, int permutations, int permutation_length, int iSeedValue, int iRandomButton) 
  {
    printf("New Permutation Array Made Without Hashing\n");
    int i=0;
    int j=0;
    int malloc_error=0;
    const size_t N=permutation_length;

    gsl_rng *r;
    const gsl_rng_type *T=gsl_rng_mt19937;
    gsl_rng_env_setup();
    if(iRandomButton==1)T=gsl_rng_mt19937;
    if(iRandomButton==2)T=gsl_rng_taus2;
    if(iRandomButton==3)T=gsl_rng_ranlux389;
    r=gsl_rng_alloc(T);
    gsl_rng_set(r,iSeedValue);
    if(r==NULL) malloc_error=1;

    int check_permutation_count=(int)gsl_sf_gamma(permutation_length+1);
    int *permutation_shuffled_index=(int *)calloc(check_permutation_count, sizeof(int));
    if(permutation_shuffled_index==NULL) malloc_error=1;

    int **AllPermutations=malloc(check_permutation_count*sizeof(int*));
    if(AllPermutations==NULL) malloc_error=1;
    for(i=0;i<check_permutation_count;i++)
         {
           AllPermutations[i]=malloc(permutation_length * sizeof(int));
           if(AllPermutations[i]==NULL) malloc_error=1;
         }

    gsl_permutation *p=gsl_permutation_alloc(N);
    gsl_permutation_init(p);

    if(malloc_error==1)
      {
        printf("Memory allocation error in function generate_permutations_without_hashing().");
        assert(malloc_error==0);
      }
    else
      {
        //Add some values to be shuffled.
        for(i=0;i<check_permutation_count;i++)
           {
             permutation_shuffled_index[i]=i;
           }

        //shuffle the index.
        gsl_ran_shuffle(r, permutation_shuffled_index, check_permutation_count, sizeof(int));

        //Get all the permutations in an array.
        for(i=0;i<check_permutation_count;i++)
           {
            for(j=0;j<permutation_length;j++)
               {
                 AllPermutations[i][j]=gsl_permutation_get(p,j);
               }
            gsl_permutation_next(p);
           }

        //load shuffled permutations.
        for(i=0;i<permutations;i++)
           {
             for(j=0;j<permutation_length;j++)
                {
                  (*perm1)[i][j]=AllPermutations[permutation_shuffled_index[i]][j];
                }
           }
       }

    for(i=0;i<check_permutation_count; i++)
         {
           if(AllPermutations[i]!=NULL)free(AllPermutations[i]);
         }
    if(AllPermutations!=NULL) free(AllPermutations);
    if(permutation_shuffled_index!=NULL) free(permutation_shuffled_index);   
    gsl_permutation_free(p);
    gsl_rng_free(r);

  }
/*
  If there are too many permutations to just store the entire set in memory, create them one at a time. Make sure they are unique by hashing the values. If two or more hash keys are the same it shouldn't be a problem as long as there is a large pool of permutations and no duplicate hash keys are allowed. The UI limits the number of permutations to 500,000 which is safe for 10! and above.
*/
static void generate_permutations_with_hashing(int ***perm1, int permutations, int permutation_length, int iSeedValue, int iRandomButton)
  {
    printf("New Permutation Array Made With Hashing\n");
    int i=0;
    int temp1=0;
    int temp2=0;
    int counter=0;
    int malloc_error=0;
    int *temp_perm=(int *)calloc(permutation_length, sizeof(int));
    if(temp_perm==NULL) malloc_error=1;
    GHashTable* hash = g_hash_table_new_full(g_str_hash, g_str_equal, (GDestroyNotify)hash_key_destroyed, NULL);
    if(hash==NULL) malloc_error=1;
    GString *string=g_string_new("");
    
    gsl_rng *r=NULL;
    const gsl_rng_type *T=gsl_rng_mt19937;
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
        assert(malloc_error==0);
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
static void hash_key_destroyed(gpointer data)
  {
    //printf("Got a key destroy call for %s\n", (char*)data);
    hash_check=1;
  }
static void print_monotone_pvalues(GtkTextView *textview, GPtrArray *sArray, double *monotonicity, apop_data *mPvaluesSorted, int iFunction)
  {
    printf("Enforce Monotonicity\n");
    int i=0;
    int plate=0;
    int size=0;
    int as_return=0;
    double ProbValue=-1.0;
    GString *sAdjP=g_string_new(NULL);
    GtkTextBuffer *buffer;
    GtkTextIter start;
    GtkTextIter end;  
    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW (textview));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    gtk_text_buffer_delete(buffer, &start, &end);

    char *string;
    if(iFunction==2)
      {
        as_return=asprintf(&string, "Plate Control Test ControlMean TestMean Difference Permutations PermutationLength ControlCount TestCount Count1 Count2 PermutationMean PermutationsStdDevS Side p-value minP monotone_minP\n");
      }
    else if(iFunction==3)
      {
        as_return=asprintf(&string, "Plate Control Test ControlMean TestMean Difference Permutations PermutationLength ControlCount TestCount Count1 Count2 PermutationMean PermutationsStdDevS Side p-value maxT monotone_maxT\n");
      }

    if(as_return!=-1)
      {
        gtk_text_buffer_insert_at_cursor(buffer, string, -1);
        free(string);
      }
    else
      {
        printf("Error: asprintf couldn't allocate string in VelociRaptorPermutations.c\n");
      }

    size=mPvaluesSorted->matrix->size1;

    //Enforce monotonicity. Start from the bottom of the array and adjust within each plate.
    for(i=size-1;i>=0;i--)
       {
         if(plate!=(int)apop_data_get(mPvaluesSorted,i,0))
           {
             plate=(int)apop_data_get(mPvaluesSorted,i,0);
             ProbValue=-1.0;
           }
         if(ProbValue>=0)
           {
             //printf("%i %i %i %f %f\n", i, size, plate, monotonicity[i], monotonicity[i+1]);
             monotonicity[i]=fmax(monotonicity[i], monotonicity[i+1]);
           }
         else
           {
             //printf("%i %i %i %f\n", i, size, plate, monotonicity[i]);
           }
         ProbValue=monotonicity[i];
       }

    //Print the results.
    for(i=0;i<size;i++)
       {
         g_string_append_printf(sAdjP, "%f\n", monotonicity[i]);
         gtk_text_buffer_insert_at_cursor(buffer, g_ptr_array_index(sArray,i), -1);
         gtk_text_buffer_insert_at_cursor(buffer, sAdjP->str, -1);
         g_string_truncate(sAdjP,0); 
       }
  }




