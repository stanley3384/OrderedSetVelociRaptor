
/* Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.*/

//for using popen() 
#define _XOPEN_SOURCE //500

#include <gtk/gtk.h>
#include <glib.h>
#include <sqlite3.h>
#include <apop.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_statistics_double.h>
#include <gsl/gsl_combination.h>
#include <gsl/gsl_permutation.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_vector.h>
#include <math.h>
#include "VelociRaptorUI_Validation.h"
#include "VelociRaptorMath.h"
#include "VelociRaptorGlobal.h"


void basic_statistics_sql(GtkTextView *textview, int iRadioButton)
   {
     printf("Basic Stats\n");
     int i=0;
     int j=0;
     int check=0;
     int iGroups=0;
     int iCounter=0;
     GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
     apop_data *StatsData=NULL;
     apop_data *MedianData=NULL;
     gsl_vector *vGroupCount=NULL;
     gsl_vector *v=NULL;
     gsl_vector *vMedian=NULL;
     double *pctiles=NULL;
     double median=0;

     apop_db_open("VelociRaptorData.db");

     if(iRadioButton==1)
       {
         StatsData=apop_query_to_data("SELECT T2.plate, count(T1.data), avg(T1.data), stddev(T1.data), var_samp(T1.data), skew(T1.data), kurt(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate;");
         MedianData=apop_query_to_data("SELECT T2.plate, T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID ORDER BY T2.Plate;");
         vGroupCount=apop_query_to_vector("SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate;");
         iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate);");
         vMedian=gsl_vector_alloc(iGroups);
       }
     if(iRadioButton==2)
       {
         StatsData=apop_query_to_data("SELECT T2.plate, count(T1.percent), avg(T1.percent), stddev(T1.percent), var_samp(T1.percent), skew(T1.percent), kurt(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate;");
         MedianData=apop_query_to_data("SELECT T2.plate, T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID ORDER BY T2.Plate;");
         vGroupCount=apop_query_to_vector("SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate;");
         iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate);");
         vMedian=gsl_vector_alloc(iGroups);
       }
     if(iRadioButton==3)
       {
         StatsData=apop_query_to_data("SELECT T2.plate, count(T1.data), avg(T1.data), stddev(T1.data), var_samp(T1.data), skew(T1.data), kurt(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate;");
         MedianData=apop_query_to_data("SELECT T2.plate, T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID ORDER BY T2.Plate;");
         vGroupCount=apop_query_to_vector("SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate;");
         iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate);");
         vMedian=gsl_vector_alloc(iGroups);
       }
     if(iRadioButton==4)
       {
         StatsData=apop_query_to_data("SELECT T2.plate, count(T1.percent), avg(T1.percent), stddev(T1.percent), var_samp(T1.percent), skew(T1.percent), kurt(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate;");
         MedianData=apop_query_to_data("SELECT T2.plate, T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID ORDER BY T2.Plate;");
         vGroupCount=apop_query_to_vector("SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate;");
         iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate);");
         vMedian=gsl_vector_alloc(iGroups);
       }

     apop_db_close(0);
  
     //find the median values.
     for(i=0;i<iGroups;i++)
        {
         for(j=0;j<gsl_vector_get(vGroupCount,i);j++)
            {
              if(j==0)
                {
                  v=gsl_vector_alloc(gsl_vector_get(vGroupCount,i));
                }
              gsl_vector_set(v,j,apop_data_get(MedianData,iCounter,1));
              if(j==gsl_vector_get(vGroupCount,i)-1)
                {
                  pctiles=apop_vector_percentiles(v,'a');
                  median=pctiles[50];
                  //printf("Median=%f\n",median);
                  gsl_vector_set(vMedian,i,median);
                  gsl_vector_free(v);
                  v=NULL;
                }
              iCounter++;
            }
         }

     if(StatsData==NULL)
       {
         printf("No data returned from the database.\n");
         simple_message_dialog("No data returned from the database.\n Make sure to make an auxiliary table.");
         check=1;
       }
     //i=StatsData->matrix->size1;
     //j=StatsData->matrix->size2;
     //printf("rows=%i columns=%i\n", i, j);

     if(check==0)
       {
        GString *string=g_string_new("");
        gtk_text_buffer_insert_at_cursor(buffer, "Plate Count Median Mean StdDevS VarianceS Skew Kurtosis\n", -1);
         for(i=0;i<StatsData->matrix->size1;i++)
            {
              for(j=0;j<StatsData->matrix->size2;j++)
                 {
                   //printf("%f ", apop_data_get(StatsData,i,j));
                   if(j==0||j==1)
                     {
                       g_string_append_printf(string,"%i ", (int)apop_data_get(StatsData,i,j));
                     }
                   //get the median value.
                   if(j==1)
                     {
                       g_string_append_printf(string,"%f ", gsl_vector_get(vMedian,i));
                     }
                   if(j==2)
                     {
                       g_string_append_printf(string,"%f ", apop_data_get(StatsData,i,j));
                     }
                   if(j==3)
                     {
                       g_string_append_printf(string,"%f ", apop_data_get(StatsData,i,j));
                     }
                   if(j==4)
                     {
                       g_string_append_printf(string,"%f ", apop_data_get(StatsData,i,j));
                     }
                   if(j==5)
                     {
                       g_string_append_printf(string,"%f ", apop_data_get(StatsData,i,j));
                     }
                   if(j==6)
                     {
                       g_string_append_printf(string,"%f", apop_data_get(StatsData,i,j));
                     }
                 }
              g_string_append_printf(string, "\n");
            }
         gtk_text_buffer_insert_at_cursor(buffer, string->str, -1);
         g_string_free(string,TRUE);
       }

    if(StatsData!=NULL)
      {
        gsl_matrix_free(StatsData->matrix);
      }
    if(MedianData!=NULL)
      {
        gsl_matrix_free(MedianData->matrix);
      }
    if(v!=NULL)
      {
        gsl_vector_free(v);
      }
    if(vGroupCount!=NULL)
      {
        gsl_vector_free(vGroupCount);
      }
    if(vMedian!=NULL)
      {
        gsl_vector_free(vMedian);
      }
   }
void levenes_variance_test(GtkTextView *textview, int iRadioButton, double alpha)
   {
    //See references file for more information.
    int i=0;
    int j=0;
    double temp1=0;
    double temp2=0;
    double temp3=0;
    double temp4=0;
    int iGroups=0;
    int iCounter=0;
    int iGroupsPerPlate=0;
    int iRecordsPerPlate=0;
    int iTotalPlates=0;
    gsl_vector *vPlateData=NULL;
    gsl_vector *vZ=NULL;
    gsl_vector *vMM=NULL;
    gsl_vector *vGroupCount=NULL;
    gsl_vector *vMedian=NULL;
    gsl_vector *vAvgZ=NULL;
    gsl_vector *v=NULL;
    gsl_vector *vPlateZ=NULL;
    gsl_vector *vT1=NULL;
    gsl_vector *vT2=NULL;
    double *pctiles=NULL;
    double AvgZ=0;
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    apop_db_open("VelociRaptorData.db");

    //Groups and Data
    if(iRadioButton==1)
      {
        vPlateData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID ORDER BY T2.Plate;");
        iTotalPlates=(int)apop_query_to_float("SELECT max(T2.plate) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID;");
        iGroupsPerPlate=(int)apop_query_to_float("SELECT max(T2.Groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1;");
        iRecordsPerPlate=(int)apop_query_to_float("SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1 AND T2.Groups!=0;");
        vGroupCount=apop_query_to_vector("SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate;");
        iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate);");
        vMedian=gsl_vector_alloc(iGroups);
        vAvgZ=gsl_vector_alloc(iGroups);
        vZ=gsl_vector_alloc(vPlateData->size);
        vMM=gsl_vector_alloc(vPlateData->size);
        vPlateZ=gsl_vector_alloc(iTotalPlates);
        vT1=gsl_vector_alloc(iTotalPlates);
        vT2=gsl_vector_alloc(iTotalPlates);
      }
    //Groups and Percent
    if(iRadioButton==2)
      {
        vPlateData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID ORDER BY T2.Plate;");
        iTotalPlates=(int)apop_query_to_float("SELECT max(T2.plate) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID;");
        iGroupsPerPlate=(int)apop_query_to_float("SELECT max(T2.Groups) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1;");
        iRecordsPerPlate=(int)apop_query_to_float("SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1 AND T2.Groups!=0;");
        vGroupCount=apop_query_to_vector("SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate;");
        iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID GROUP BY T2.Plate,T2.Groups ORDER BY T2.Plate);");
        vMedian=gsl_vector_alloc(iGroups);
        vAvgZ=gsl_vector_alloc(iGroups);
        vZ=gsl_vector_alloc(vPlateData->size);
        vMM=gsl_vector_alloc(vPlateData->size);
        vPlateZ=gsl_vector_alloc(iTotalPlates);
        vT1=gsl_vector_alloc(iTotalPlates);
        vT2=gsl_vector_alloc(iTotalPlates);
      }
   
    //Picks and Data
    if(iRadioButton==3)
      {
        vPlateData=apop_query_to_vector("SELECT T1.data FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 ORDER BY T2.Plate;");
        iTotalPlates=(int)apop_query_to_float("SELECT max(T2.plate) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID;");
        iGroupsPerPlate=(int)apop_query_to_float("SELECT max(T2.Picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1;");
        iRecordsPerPlate=(int)apop_query_to_float("SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1 AND T2.Picks!=0;");
        vGroupCount=apop_query_to_vector("SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate;");
        iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.data) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate);");
        vMedian=gsl_vector_alloc(iGroups);
        vAvgZ=gsl_vector_alloc(iGroups);
        vZ=gsl_vector_alloc(vPlateData->size);
        vMM=gsl_vector_alloc(vPlateData->size);
        vPlateZ=gsl_vector_alloc(iTotalPlates);
        vT1=gsl_vector_alloc(iTotalPlates);
        vT2=gsl_vector_alloc(iTotalPlates);
      }
    //Picks and Percent
    if(iRadioButton==4)
      {
        vPlateData=apop_query_to_vector("SELECT T1.percent FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 ORDER BY T2.Plate;");
        iTotalPlates=(int)apop_query_to_float("SELECT max(T2.plate) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID;");
        iGroupsPerPlate=(int)apop_query_to_float("SELECT max(T2.Picks) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1;");
        iRecordsPerPlate=(int)apop_query_to_float("SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Plate=1 AND T2.Picks!=0;");
        vGroupCount=apop_query_to_vector("SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate;");
        iGroups=(int)apop_query_to_float("SELECT count() FROM (SELECT count(T1.percent) FROM data T1, aux T2 WHERE T1.KeyID=T2.KeyID AND T2.Picks!=0 GROUP BY T2.Plate,T2.Picks ORDER BY T2.Plate);");
        vMedian=gsl_vector_alloc(iGroups);
        vAvgZ=gsl_vector_alloc(iGroups);
        vZ=gsl_vector_alloc(vPlateData->size);
        vMM=gsl_vector_alloc(vPlateData->size);
        vPlateZ=gsl_vector_alloc(iTotalPlates);
        vT1=gsl_vector_alloc(iTotalPlates);
        vT2=gsl_vector_alloc(iTotalPlates);
      }

    apop_db_close(0);

    //find the median values.
     for(i=0;i<iGroups;i++)
        {
         for(j=0;j<gsl_vector_get(vGroupCount,i);j++)
            {
              if(j==0)
                {
                  v=gsl_vector_alloc(gsl_vector_get(vGroupCount,i));
                }
              gsl_vector_set(v,j,gsl_vector_get(vPlateData,iCounter));
              if(j==gsl_vector_get(vGroupCount,i)-1)
                {
                  pctiles=apop_vector_percentiles(v,'a');
                  temp1=pctiles[50];
                  //printf("%i GroupCount %i Median %f\n", i+1 , (int)gsl_vector_get(vGroupCount,i), temp1);
                  gsl_vector_set(vMedian,i,temp1);
                  gsl_vector_free(v);
                  v=NULL;
                }
              iCounter++;
            }
         }
    
    //find |data-median| and avg|data-median|. Load the values into vectors.
    iCounter=0;
    for(i=0;i<iGroups;i++)
        {
         for(j=0;j<gsl_vector_get(vGroupCount,i);j++)
            {
              if(j==0)
                {
                  temp1=gsl_vector_get(vMedian,i);
                }
              temp2=fabs((gsl_vector_get(vPlateData,iCounter))-temp1);
              gsl_vector_set(vZ,iCounter,temp2);
              temp3=temp3+temp2;
              iCounter++;
            }
           temp4=temp3/gsl_vector_get(vGroupCount,i);
           //printf("%i GroupCount %i avgZ %f %i\n", i+1 , (int)gsl_vector_get(vGroupCount,i), temp4,j);
           gsl_vector_set(vAvgZ,i,temp4);
           temp2=0;
           temp3=0;
           temp4=0;
         }

    //find (vZ - vAvgZ)^2 and load into vector vMM.
    iCounter=0;
    for(i=0;i<iGroups;i++)
        {
         for(j=0;j<gsl_vector_get(vGroupCount,i);j++)
            {
              if(j==0)
                {
                  temp1=gsl_vector_get(vAvgZ,i);
                }
              temp2=pow(gsl_vector_get(vZ,iCounter)-temp1,2);
              //printf("%i GroupCount %i squareZ %f\n", i+1 , (int)gsl_vector_get(vGroupCount,i), temp2);
              gsl_vector_set(vMM,iCounter,temp2);
              iCounter++;
            }
           //printf("\n");
         }

    //Find avg of z per plate.
    temp1=0;
    iCounter=0;
    for(i=0;i<iTotalPlates;i++)
       {
        for(j=0;j<iGroupsPerPlate;j++)
           {
             temp1=temp1+gsl_vector_get(vAvgZ,iCounter);
             iCounter++;
           }
        AvgZ=temp1/iGroupsPerPlate;
        gsl_vector_set(vPlateZ, i, AvgZ);
        temp1=0;
       }
    
    //Get T1 and T2.
    temp1=0;
    temp2=0;
    temp3=0;
    temp4=0;
    iCounter=0;
    for(i=0;i<iTotalPlates;i++)
       {
        for(j=0;j<iRecordsPerPlate;j++)
           {
             temp1=gsl_vector_get(vZ,iCounter);
             temp2=pow((temp1-gsl_vector_get(vPlateZ,i)),2);
             temp3=temp3+gsl_vector_get(vMM,iCounter);
             temp4=temp4+temp2;
             iCounter++;
           }
         gsl_vector_set(vT1,i,temp3);
         gsl_vector_set(vT2,i,temp4);
         temp3=0;
         temp4=0;
       }

    printf("Plate Groups Records Z T1 T2 Levenes CriticalValue\n");
    gtk_text_buffer_insert_at_cursor(buffer, "Plate Groups Records Z T1 T2 Levenes CriticalValue\n", -1);
    GString *string=g_string_new("");
    for(i=0;i<iTotalPlates;i++)
       {
         printf("%i %i %i %f %f %f %f %f\n", i+1, iGroupsPerPlate, iRecordsPerPlate, gsl_vector_get(vPlateZ,i), gsl_vector_get(vT1,i), gsl_vector_get(vT2,i), ((gsl_vector_get(vT2,i)-gsl_vector_get(vT1,i))/(iGroupsPerPlate-1))/(gsl_vector_get(vT1,i)/(iRecordsPerPlate-iGroupsPerPlate)), gsl_cdf_fdist_Qinv(alpha,(iGroupsPerPlate-1),(iRecordsPerPlate-iGroupsPerPlate)));

         g_string_append_printf(string,"%i %i %i %f %f %f %f %f\n", i+1, iGroupsPerPlate, iRecordsPerPlate, gsl_vector_get(vPlateZ,i), gsl_vector_get(vT1,i), gsl_vector_get(vT2,i), ((gsl_vector_get(vT2,i)-gsl_vector_get(vT1,i))/(iGroupsPerPlate-1))/(gsl_vector_get(vT1,i)/(iRecordsPerPlate-iGroupsPerPlate)), gsl_cdf_fdist_Qinv(alpha,(iGroupsPerPlate-1),(iRecordsPerPlate-iGroupsPerPlate)));
       } 

    gtk_text_buffer_insert_at_cursor(buffer, string->str, -1);
    g_string_free(string,TRUE);

    if(vPlateData!=NULL)
      {
        gsl_vector_free(vPlateData);
      }
    if(vZ!=NULL)
      {
        gsl_vector_free(vZ);
      }
    if(vMM!=NULL)
      {
        gsl_vector_free(vMM);
      }
    if(vGroupCount!=NULL)
      {
        gsl_vector_free(vGroupCount);
      }
    if(vAvgZ!=NULL)
      {
        gsl_vector_free(vAvgZ);
      }
    if(v!=NULL)
      {
        gsl_vector_free(v);
      }
    if(vMedian!=NULL)
      {
        gsl_vector_free(vMedian);
      }
    if(vPlateZ!=NULL)
      {
        gsl_vector_free(vPlateZ);
      }
   if(vT1!=NULL)
      {
        gsl_vector_free(vT1);
      }
   if(vT2!=NULL)
      {
        gsl_vector_free(vT2);
      }

   }
void one_way_anova_sql(GtkTextView *textview, int iRadioButton, int check_box, double alpha)
   {
      sqlite3 *handle;
      sqlite3_stmt *stmt1;
      int iPlates=0;
      int iBetweenDf=0;
      int iWithinDf=0;
      int iTotalDf=0;
      double one=0;
      double two=0;
      double three=0; 

      sqlite3_open("VelociRaptorData.db",&handle);
         
         if(iRadioButton==1)
            {
              //Groups and Data
              char sql1[]="SELECT Plate, k-1, N-k, N-1, one, two, three FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Groups) AS k1, count(data) AS N1, sum(data * data) AS one1, sum(data) AS two2, (sum(data)*sum(data))/count(Groups) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID GROUP BY Plate, Groups) GROUP BY Plate ORDER BY Plate);";

             if(check_box==1)
               {
                 //anova for groups and data
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char sql_anova1[]="SELECT Plate, k-1, N-k, N-1, (three-two) AS SSb, (one-three) AS SSw, (one-two) AS SSt, ((three-two)/(k-1)) AS MSb, ((one-three)/(N-k)) AS MSw, (((three-two)/(k-1))/((one-three)/(N-k))) AS F FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Groups) AS k1, count(data) AS N1, sum(data * data) AS one1, sum(data) AS two2, (sum(data)*sum(data))/count(Groups) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID GROUP BY Plate, Groups) GROUP BY Plate ORDER BY Plate);\n\n";
                 gtk_text_buffer_insert_at_cursor(buffer, sql_anova1, -1);
               }
             else
               {
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char ColumnTitles[]="Plate k-1 N-k N-1 SSb SSw SSt MSb MSw F CriticalValueF\n";
                 gtk_text_buffer_insert_at_cursor(buffer, ColumnTitles, -1);
               }

             sqlite3_prepare_v2(handle,sql1,-1,&stmt1,0);
             while(sqlite3_step(stmt1) == SQLITE_ROW)
               {
                 iPlates=sqlite3_column_int(stmt1, 0);
                 iBetweenDf=sqlite3_column_int(stmt1, 1);
                 iWithinDf=sqlite3_column_int(stmt1, 2);
                 iTotalDf=sqlite3_column_int(stmt1, 3);
                 one=sqlite3_column_double(stmt1, 4);
                 two=sqlite3_column_double(stmt1, 5);
                 three=sqlite3_column_double(stmt1, 6);
                 //printf("%i %i %i %i %f %f %f\n",iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three);
                 if(check_box==1)
                  {
                    anova_format(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
                 if(check_box==0)
                  {
                    anova_format_tabular(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
               }
              sqlite3_finalize(stmt1);  
            }
          else if(iRadioButton==2)
            {
              //Groups and Percent
              char sql2[]="SELECT Plate, k-1, N-k, N-1, one, two, three FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Groups) AS k1, count(Percent) AS N1, sum(Percent * Percent) AS one1, sum(Percent) AS two2, (sum(Percent)*sum(Percent))/count(Groups) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID GROUP BY Plate, Groups) GROUP BY Plate ORDER BY Plate);";

             if(check_box==1)
               {
                 //anova for groups and percent
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char sql_anova2[]="SELECT Plate, k-1, N-k, N-1, (three-two) AS SSb, (one-three) AS SSw, (one-two) AS SSt, ((three-two)/(k-1)) AS MSb, ((one-three)/(N-k)) AS MSw, (((three-two)/(k-1))/((one-three)/(N-k))) AS F FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Groups) AS k1, count(Percent) AS N1, sum(Percent * Percent) AS one1, sum(Percent) AS two2, (sum(Percent)*sum(Percent))/count(Groups) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID GROUP BY Plate, Groups) GROUP BY Plate ORDER BY Plate);\n\n";
                 gtk_text_buffer_insert_at_cursor(buffer, sql_anova2, -1);
               }
             if(check_box==0)
               {
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char ColumnTitles[]="Plate k-1 N-k N-1 SSb SSw SSt MSb MSw F CriticalValueF\n";
                 gtk_text_buffer_insert_at_cursor(buffer, ColumnTitles, -1);
               }

             sqlite3_prepare_v2(handle,sql2,-1,&stmt1,0);
             while(sqlite3_step(stmt1) == SQLITE_ROW)
               {
                 iPlates=sqlite3_column_int(stmt1, 0);
                 iBetweenDf=sqlite3_column_int(stmt1, 1);
                 iWithinDf=sqlite3_column_int(stmt1, 2);
                 iTotalDf=sqlite3_column_int(stmt1, 3);
                 one=sqlite3_column_double(stmt1, 4);
                 two=sqlite3_column_double(stmt1, 5);
                 three=sqlite3_column_double(stmt1, 6);
                 //printf("%i %i %i %i %f %f %f\n",iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three);
                 if(check_box==1)
                  {
                    anova_format(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
                 if(check_box==0)
                  {
                    anova_format_tabular(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
               }
              sqlite3_finalize(stmt1);  
            }
          else if(iRadioButton==3)
            {
              //Picks and Data
              char sql3[]="SELECT Plate, k-1, N-k, N-1, one, two, three FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Picks) AS k1, count(data) AS N1, sum(data * data) AS one1, sum(data) AS two2, (sum(data)*sum(data))/count(Picks) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate ORDER BY Plate);";

             if(check_box==1)
               {
                 //anova for picks and data
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char sql_anova3[]="SELECT Plate, k-1, N-k, N-1, (three-two) AS SSb, (one-three) AS SSw, (one-two) AS SSt, ((three-two)/(k-1)) AS MSb, ((one-three)/(N-k)) AS MSw, (((three-two)/(k-1))/((one-three)/(N-k))) AS F FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Picks) AS k1, count(data) AS N1, sum(data * data) AS one1, sum(data) AS two2, (sum(data)*sum(data))/count(Picks) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate ORDER BY Plate);\n\n";
                 gtk_text_buffer_insert_at_cursor(buffer, sql_anova3, -1);
               }
             if(check_box==0)
               {
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char ColumnTitles[]="Plate k-1 N-k N-1 SSb SSw SSt MSb MSw F CriticalValueF\n";
                 gtk_text_buffer_insert_at_cursor(buffer, ColumnTitles, -1);
               }

             sqlite3_prepare_v2(handle,sql3,-1,&stmt1,0);
             while(sqlite3_step(stmt1) == SQLITE_ROW)
               {
                 iPlates=sqlite3_column_int(stmt1, 0);
                 iBetweenDf=sqlite3_column_int(stmt1, 1);
                 iWithinDf=sqlite3_column_int(stmt1, 2);
                 iTotalDf=sqlite3_column_int(stmt1, 3);
                 one=sqlite3_column_double(stmt1, 4);
                 two=sqlite3_column_double(stmt1, 5);
                 three=sqlite3_column_double(stmt1, 6);
                 //printf("%i %i %i %i %f %f %f\n",iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three);
                 if(check_box==1)
                  {
                    anova_format(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
                 if(check_box==0)
                  {
                    anova_format_tabular(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
               }
              sqlite3_finalize(stmt1);  
            }
          else if(iRadioButton==4)
            {
              //Picks and Percent
              char sql4[]="SELECT Plate, k-1, N-k, N-1, one, two, three FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Picks) AS k1, count(Percent) AS N1, sum(Percent * Percent) AS one1, sum(Percent) AS two2, (sum(Percent)*sum(Percent))/count(Picks) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate ORDER BY Plate);";

              if(check_box==1)
               {
                 //anova for picks and percent
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char sql_anova4[]="SELECT Plate, k-1, N-k, N-1, (three-two) AS SSb, (one-three) AS SSw, (one-two) AS SSt, ((three-two)/(k-1)) AS MSb, ((one-three)/(N-k)) AS MSw, (((three-two)/(k-1))/((one-three)/(N-k))) AS F FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, (sum(two2)*sum(two2))/sum(N1) AS two, sum(three3) AS three FROM(SELECT Plate, count(Picks) AS k1, count(Percent) AS N1, sum(Percent * Percent) AS one1, sum(Percent) AS two2, (sum(Percent)*sum(Percent))/count(Picks) AS three3 FROM data, aux WHERE data.KeyID=aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate ORDER BY Plate);\n\n";
                 gtk_text_buffer_insert_at_cursor(buffer, sql_anova4, -1);
               }
              if(check_box==0)
               {
                 GtkTextBuffer *buffer;
                 buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
                 char ColumnTitles[]="Plate k-1 N-k N-1 SSb SSw SSt MSb MSw F CriticalValueF\n";
                 gtk_text_buffer_insert_at_cursor(buffer, ColumnTitles, -1);
               }

              sqlite3_prepare_v2(handle,sql4,-1,&stmt1,0);
              while(sqlite3_step(stmt1) == SQLITE_ROW)
               {
                 iPlates=sqlite3_column_int(stmt1, 0);
                 iBetweenDf=sqlite3_column_int(stmt1, 1);
                 iWithinDf=sqlite3_column_int(stmt1, 2);
                 iTotalDf=sqlite3_column_int(stmt1, 3);
                 one=sqlite3_column_double(stmt1, 4);
                 two=sqlite3_column_double(stmt1, 5);
                 three=sqlite3_column_double(stmt1, 6);
                 //printf("%i %i %i %i %f %f %f\n",iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three);
                if(check_box==1)
                  {
                    anova_format(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
                if(check_box==0)
                  {
                    anova_format_tabular(textview, iPlates, iBetweenDf, iWithinDf, iTotalDf, one, two, three, alpha);
                  }
               }
              sqlite3_finalize(stmt1);  
            }
         else
            {
              //exit
            }

           sqlite3_close(handle);

       }
void anova_format(GtkTextView *textview, int iPlates, int iBetweenDf,int iWithinDf,int iTotalDf, double one, double two, double three, double alpha)
   {
      GtkTextBuffer *buffer;
      buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
      double dCriticalValue=0;

            //Format results for each set.
              printf("Plate %i\n", iPlates);
              dCriticalValue=gsl_cdf_fdist_Qinv(alpha,iBetweenDf,iWithinDf);
              printf("F Critical Value %f with(%f, %i, %i)\n", dCriticalValue, alpha, iBetweenDf, iWithinDf);
              printf("Source     df           SS             MS             F\n");
              printf("Between    %i     ", iBetweenDf);
              printf("   %f       %f      %f\n", three-two, (three-two)/iBetweenDf, ((three-two)/iBetweenDf)/((one-three)/iWithinDf));
              printf("Within     %i    ", iWithinDf);
              printf("   %f      %f\n", one-three, (one-three)/iWithinDf);
              printf("Total      %i    ", iTotalDf);
              printf("   %f\n\n", one-two);

               //Print to textview
                 char *string;
                 asprintf(&string, " ");
                 char *temp1=string;
                 asprintf(&string, " one = %f\n", one);
                 free(temp1);
                 char *temp2=string;
                 asprintf(&string, "%s two = %f\n",string, two);
                 free(temp2);
                 char *temp3=string;
                 asprintf(&string, "%s three = %f\n\n",string, three);
                 free(temp3);

                 //Format results for each set.
                 char *temp4=string;
                 asprintf(&string, "%sPlate %i\n",string, iPlates);
                 free(temp4);
                 char *temp5=string;
                 asprintf(&string, "%sF Critical Value %f with(%f, %i, %i)\n", string, dCriticalValue, alpha, iBetweenDf, iWithinDf);
                 free(temp5);
                 char *temp6=string;
                 asprintf(&string, "%sSource         df                 SS                                     MS                           F\n", string);
                 free(temp6);
                 char *temp7=string;
                 asprintf(&string, "%sBetween     %i     ",string, iBetweenDf);
                 free(temp7);
                 char *temp8=string;
                 asprintf(&string, "%s   %f       %f      %f\n",string, three-two, (three-two)/iBetweenDf, ((three-two)/iBetweenDf)/((one-three)/iWithinDf));
                 free(temp8);
                 char *temp9=string;
                 asprintf(&string, "%sWithin         %i    ",string, iWithinDf);
                 free(temp9);
                 char *temp10=string;
                 asprintf(&string, "%s   %f      %f\n",string, one-three, (one-three)/iWithinDf);
                 free(temp10);
                 char *temp11=string;
                 asprintf(&string, "%sTotal            %i    ",string, iTotalDf);
                 free(temp11);
                 char *temp12=string;
                 asprintf(&string, "%s   %f\n\n",string, one-two);
                 free(temp12);
            
                 gtk_text_buffer_insert_at_cursor(buffer, string, -1);
                 free(string);
    }
void anova_format_tabular(GtkTextView *textview, int iPlates, int iBetweenDf,int iWithinDf,int iTotalDf, double one, double two, double three, double alpha)
   {
      GtkTextBuffer *buffer1;
      buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
      double dCriticalValue=gsl_cdf_fdist_Qinv(alpha,iBetweenDf,iWithinDf);

       char *string;
       asprintf(&string, "%i %i %i %i %f %f %f %f %f %f %f\n", iPlates, iBetweenDf, iWithinDf, iTotalDf, three-two, one-three, one-two, (three-two)/iBetweenDf, (one-three)/iWithinDf, ((three-two)/iBetweenDf)/((one-three)/iWithinDf), dCriticalValue  );
       gtk_text_buffer_insert_at_cursor(buffer1, string, -1);
       free(string); 
   }
void database_to_box_graph_sql(int iRadioButton, int lower_bound, int upper_bound)
   {
        apop_opts.db_engine='l';
        apop_db_open("VelociRaptorData.db");
        apop_query("BEGIN;");
        apop_query("CREATE TABLE BoxGraphTemp(BoxID INTEGER PRIMARY KEY NOT NULL, Order1 INTEGER, Order2 INTEGER);");
        if(iRadioButton==1||iRadioButton==2)
          {
            apop_query("INSERT INTO BoxGraphTemp(Order1, Order2) SELECT Box1ID , sum(OrderID) AS O1 FROM (SELECT Plate|| '0' ||Groups AS Box1ID, Data.KeyID AS OrderID FROM aux, data WHERE aux.KeyID=data.KeyID ORDER BY data.KeyID) GROUP BY Box1ID ORDER BY O1;");
          }
        if(iRadioButton==3||iRadioButton==4)
          {
            apop_query("INSERT INTO BoxGraphTemp(Order1, Order2) SELECT Box1ID , sum(OrderID) AS O1 FROM (SELECT Plate|| '0' ||Picks AS Box1ID, Data.KeyID AS OrderID FROM aux, data WHERE aux.KeyID=data.KeyID ORDER BY data.KeyID) GROUP BY Box1ID ORDER BY O1;");
          }     
        apop_query("COMMIT;");

        if(iRadioButton==1)
          {
            plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Data AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);       
          }
        if(iRadioButton==2)
          {           
            plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Percent AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);            
          }
        if(iRadioButton==3)
          { 
            plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Data AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);        
          }
        if(iRadioButton==4)
          { 
            plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Percent AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);          
                 }
        apop_query("DROP TABLE IF EXISTS BoxGraphTemp;");
        apop_db_close(0);
   }
void database_to_error_graph_sql(int iRadioButton1, int iRadioButton2, int lower_bound, int upper_bound)
   {
     apop_opts.db_engine='l';
        apop_db_open("VelociRaptorData.db");
        apop_query("BEGIN;");
        apop_query("CREATE TABLE ErrorGraphTemp(ErrorID INTEGER PRIMARY KEY NOT NULL, Mean DOUBLE, StandardDev DOUBLE);");

        if(iRadioButton1==1)
          {
             apop_query("INSERT INTO ErrorGraphTemp(Mean, StandardDev) SELECT avg(data), stddev(data) FROM data,aux WHERE data.KeyID=aux.KeyID GROUP BY Plate,Groups ORDER BY Plate,Groups;");
          }
        if(iRadioButton1==2)
          {
             apop_query("INSERT INTO ErrorGraphTemp(Mean, StandardDev) SELECT avg(percent), stddev(percent) FROM data,aux WHERE data.KeyID=aux.KeyID GROUP BY Plate,Groups ORDER BY Plate,Groups;");
          }
        if(iRadioButton1==3)
          {
             apop_query("INSERT INTO ErrorGraphTemp(Mean, StandardDev) SELECT avg(data), stddev(data) FROM data,aux WHERE data.KeyID=aux.KeyID AND aux.Picks!=0 GROUP BY Plate,Picks ORDER BY Plate,Picks;");
          }
        if(iRadioButton1==4)
          {
             apop_query("INSERT INTO ErrorGraphTemp(Mean, StandardDev) SELECT avg(percent), stddev(percent) FROM data,aux WHERE data.KeyID=aux.KeyID AND aux.Picks!=0 GROUP BY Plate,Picks ORDER BY Plate,Picks;");
          }
        apop_query("COMMIT;");
 

        if(iRadioButton2==1)
          {
            plot_matrix_now(apop_query_to_matrix("SELECT ErrorID, Mean, StandardDev FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound), 3, lower_bound, upper_bound);   
          }
        if(iRadioButton2==2)
          {   
            plot_matrix_now(apop_query_to_matrix("SELECT ErrorID, Mean, (2.0*StandardDev) FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound), 3, lower_bound, upper_bound);     
          }
        if(iRadioButton2==3)
          {    
            plot_matrix_now(apop_query_to_matrix("SELECT ErrorID, Mean, (3.0*StandardDev) FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound), 3, lower_bound, upper_bound);
          }
        
        apop_query("DROP TABLE IF EXISTS ErrorGraphTemp;");
        apop_db_close(0);
   }
void database_to_scatter_graph_sql(int iRadioButton, int lower_bound, int upper_bound)
   {
      apop_opts.db_engine='l';
      apop_db_open("VelociRaptorData.db");

        if(iRadioButton==1)
          {
            plot_matrix_now(apop_query_to_matrix("SELECT Data FROM data WHERE KeyID >= %i AND KeyID <= %i;", lower_bound, upper_bound), 1, lower_bound, upper_bound);       
          }
        if(iRadioButton==2)
          {   
             plot_matrix_now(apop_query_to_matrix("SELECT Percent FROM data WHERE KeyID >= %i AND KeyID <= %i;", lower_bound, upper_bound), 2, lower_bound, upper_bound);
          }
              
      apop_db_close(0);
   }
void plot_matrix_now(gsl_matrix *data, int graph, int lower_bound, int upper_bound)
   {
     FILE *gp=NULL;

     gp=popen("gnuplot -persist", "w");
      
     if(gp==NULL)
       {
        printf("Couldn't Open Gnuplot\n");
       }
     else
       {
         if(data!=NULL)
            {
              if(graph==1||graph==2)
                {
                  fprintf(gp, "set xrange[%i:%i]; plot '-'\n", lower_bound-1, upper_bound+1);
                }
              if(graph==3)
                {
                  fprintf(gp, "set xrange[%i:%i]; plot '-' with errorbars\n", lower_bound-1, upper_bound+1);
                }
              if(graph==4)
                {
                  fprintf(gp, "set xrange[*:*]; set yrange [*:*]; set style fill solid 0.25 border -1; set style boxplot outliers pointtype 7; set style data boxplot; plot '-' using (1.0):($2):(0):1\n");
                  
                }
               
              apop_opts.output_type='p';
              apop_opts.output_pipe=gp;
              apop_matrix_print(data, NULL);
              fflush(gp);
              pclose(gp);
            }
        else
            {
              g_print("No Data Returned From Database\n");
            }
      }
   }
void comparison_with_control_sql(int iRadioButton, int iControlValue, double alpha, int iRadioCritVal, GtkTextView *textview, GtkWidget *progress, int *pBreakLoop)
   {
     

      if(iRadioButton==1)
             //Groups and Data
             {
              char sql1[]="SELECT count(*) FROM (SELECT  Plate, Groups, count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups ORDER BY Plate, Mean);";
              char sql2[]="SELECT  Plate, Groups, count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups ORDER BY Plate, Mean;";
              char sql3[]="SELECT max(Plate) FROM (SELECT  Plate, Groups, count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups ORDER BY Plate, Mean);";
              char sql4[]="SELECT count(Groups) FROM (SELECT Plate, Groups FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate,Groups) GROUP BY Plate;";
              char sql5[]="SELECT N-k, ((one-three)/(N-k)) AS MSw FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, sum(three3) AS three FROM(SELECT Plate, count(Groups) AS k1, count(Data) AS N1, sum(Data * Data) AS one1,(sum(Data)*sum(Data))/count(Groups) AS three3 FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups) GROUP BY Plate ORDER BY Plate);";

              char *sql6 = sqlite3_mprintf("SELECT count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Groups=%i GROUP BY Plate, Groups ORDER BY Plate, Mean;", iControlValue);
              //char sql6[]="SELECT count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Groups=1 GROUP BY Plate, Groups ORDER BY Plate, Mean;";

              multiple_comparison_with_controls(sql1, sql2, sql3, sql4, sql5, sql6, iControlValue, alpha, iRadioCritVal, textview, progress, pBreakLoop);
              sqlite3_free(sql6);
             }

            if(iRadioButton==2)
              {
               //Groups and Percent
               char sql1[]="SELECT count(*) FROM (SELECT  Plate, Groups, count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups ORDER BY Plate, Mean);";
               char sql2[]="SELECT  Plate, Groups, count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups ORDER BY Plate, Mean;";
               char sql3[]="SELECT max(Plate) FROM (SELECT  Plate, Groups, count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups ORDER BY Plate, Mean);";
               char sql4[]="SELECT count(Groups) FROM (SELECT Plate, Groups FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate,Groups) GROUP BY Plate;";
               char sql5[]="SELECT N-k, ((one-three)/(N-k)) AS MSw FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, sum(three3) AS three FROM(SELECT Plate, count(Groups) AS k1, count(Percent) AS N1, sum(Percent * Percent) AS one1,(sum(Percent)*sum(Percent))/count(Groups) AS three3 FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Groups) GROUP BY Plate ORDER BY Plate);";

               char *sql6 = sqlite3_mprintf("SELECT count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Groups=%i GROUP BY Plate, Groups ORDER BY Plate, Mean;", iControlValue);
               //char sql6[]="SELECT count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Groups=1 GROUP BY Plate, Groups ORDER BY Plate, Mean;";

               multiple_comparison_with_controls(sql1, sql2, sql3, sql4, sql5, sql6, iControlValue, alpha, iRadioCritVal, textview, progress, pBreakLoop);
               sqlite3_free(sql6);
             }
       
           if(iRadioButton==3)
             {
               //Picks and Data
               char sql1[]="SELECT count(*) FROM (SELECT  Plate, Picks, count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks ORDER BY Plate, Mean);";
               char sql2[]="SELECT  Plate, Picks, count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks ORDER BY Plate, Mean;";
               char sql3[]="SELECT max(Plate) FROM (SELECT  Plate, Picks, count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks ORDER BY Plate, Mean);";
               char sql4[]="SELECT count(Picks) FROM (SELECT Plate, Picks FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate;";
               char sql5[]="SELECT N-k, ((one-three)/(N-k)) AS MSw FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, sum(three3) AS three FROM(SELECT Plate, count(Picks) AS k1, count(Data) AS N1, sum(Data * Data) AS one1,(sum(Data)*sum(Data))/count(Picks) AS three3 FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate ORDER BY Plate);";

               char *sql6 = sqlite3_mprintf("SELECT count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0  AND aux.Picks=%i GROUP BY Plate, Picks ORDER BY Plate, Mean;", iControlValue);
               //char sql6[]="SELECT count(Data) AS SetSize, sum(Data)/count(Data) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0  AND aux.Picks=1 GROUP BY Plate, Picks ORDER BY Plate, Mean;";

               multiple_comparison_with_controls(sql1, sql2, sql3, sql4, sql5, sql6, iControlValue, alpha, iRadioCritVal, textview, progress, pBreakLoop);
               sqlite3_free(sql6);
              }

           if(iRadioButton==4)
             {
               //Picks and Percent
               char sql1[]="SELECT count(*) FROM (SELECT  Plate, Picks, count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID GROUP BY Plate, Picks ORDER BY Plate, Mean);";
               char sql2[]="SELECT  Plate, Picks, count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks ORDER BY Plate, Mean;";
               char sql3[]="SELECT max(Plate) FROM (SELECT  Plate, Picks, count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID  AND aux.Picks!=0 GROUP BY Plate, Picks ORDER BY Plate, Mean);";
               char sql4[]="SELECT count(Picks) FROM (SELECT Plate, Picks FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate;";
               char sql5[]="SELECT N-k, ((one-three)/(N-k)) AS MSw FROM(SELECT Plate, sum(N1) AS N, count(k1) AS k, sum(one1) AS one, sum(three3) AS three FROM(SELECT Plate, count(Picks) AS k1, count(Percent) AS N1, sum(Percent * Percent) AS one1,(sum(Percent)*sum(Percent))/count(Picks) AS three3 FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 GROUP BY Plate, Picks) GROUP BY Plate ORDER BY Plate);";

               char *sql6 = sqlite3_mprintf("SELECT count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 AND aux.Picks=%i GROUP BY Plate, Picks ORDER BY Plate, Mean;", iControlValue);
              //char sql6[]="SELECT count(Percent) AS SetSize, sum(Percent)/count(Percent) AS Mean FROM Data, Aux WHERE Data.KeyID=Aux.KeyID AND aux.Picks!=0 AND aux.Picks=1 GROUP BY Plate, Picks ORDER BY Plate, Mean;";
              multiple_comparison_with_controls(sql1, sql2, sql3, sql4, sql5, sql6, iControlValue, alpha, iRadioCritVal, textview, progress, pBreakLoop);
              sqlite3_free(sql6);
            }    
   }
void multiple_comparison_with_controls(char *sql1, char *sql2, char *sql3, char *sql4, char *sql5, char *sql6, int iControlValue, double alpha, int iRadioCritVal, GtkTextView *textview, GtkWidget *progress, int *pBreakLoop)
     {
         sqlite3 *handle;
         sqlite3_stmt *stmt1;
         sqlite3_stmt *stmt2;
         sqlite3_stmt *stmt3;
         sqlite3_stmt *stmt4;
         sqlite3_stmt *stmt5;
         sqlite3_stmt *stmt6;
         double dTemp=0;
         double dTemp2=0;
         double dTemp3=0;
         double dTemp4=0;
         //int retval=0;
         int i=0;
         int j=0;
         int ii=0;
         int iRecordCount=0;
         int iPlateCount=0;
         double dControl=0;
         int iCounter=0;
         int iPosition=1;
         int iCounter1=0;
         double CriticalValue1=0;
         int iCovSize=1;
         int iStart=0;

         //should check return values
         sqlite3_open("VelociRaptorData.db",&handle);

         //Get record count of means table
         sqlite3_prepare_v2(handle,sql1,-1,&stmt1,0);
         sqlite3_step(stmt1);
         iRecordCount=sqlite3_column_int(stmt1, 0);
         sqlite3_finalize(stmt1);
         //if no records returned exit with dialog.
         //printf("Record Count %i\n", iRecordCount);

         //Get plate count
         sqlite3_prepare_v2(handle,sql3,-1,&stmt3,0);
         sqlite3_step(stmt3);
         iPlateCount=sqlite3_column_int(stmt3, 0);
         sqlite3_finalize(stmt3);

         //Allocate arrays on the heap.
         int *iPlates = malloc((iRecordCount) * sizeof(int));
         int *iGroup = malloc((iRecordCount) * sizeof(int));
         int *iSetSize = malloc((iRecordCount) * sizeof(int));
         double *dMean = malloc((iRecordCount) * sizeof(double));
         int *iGroupCount = malloc((iPlateCount) * sizeof(int));
         double *dMSw = malloc((iPlateCount) * sizeof(double));
         double *dControls = malloc((iPlateCount) * sizeof(double));
         int *iControlCount = malloc((iPlateCount) * sizeof(int));
         int *iControlLocation = malloc((iPlateCount) * sizeof(int));
         int *iDF = malloc((iPlateCount) * sizeof(int));

         //Load mean table into arrays.
         printf("Table of Sorted Means\n");
         sqlite3_prepare_v2(handle,sql2,-1,&stmt2,0);
            for(i=0;i<iRecordCount;i++)
               {
                 sqlite3_step(stmt2);
                 iPlates[i]=sqlite3_column_int(stmt2, 0);
                 iGroup[i]=sqlite3_column_int(stmt2, 1);
                 iSetSize[i]=sqlite3_column_int(stmt2, 2);
                 dMean[i]=sqlite3_column_double(stmt2, 3);
                 printf(" %i %i %i %i %f\n", i, iPlates[i], iGroup[i], iSetSize[i], dMean[i]);
               }
         sqlite3_finalize(stmt2);  
         //printf("\n");   
         
         //Get group counts. 
         sqlite3_prepare_v2(handle,sql4,-1,&stmt4,0);
           for(i=0;i<iPlateCount;i++)
               {
                 sqlite3_step(stmt4);
                 iGroupCount[i]=sqlite3_column_int(stmt4, 0);
                 //printf("%i\n", iGroupCount[i]);
               }
         sqlite3_finalize(stmt4);

         //Load data table in an array. Use this array for building MSw array.
         sqlite3_prepare_v2(handle,sql5,-1,&stmt5,0);
            for(i=0;i<iPlateCount;i++)
               {
                 sqlite3_step(stmt5);
                 iDF[i]=sqlite3_column_int(stmt5, 0);
                 dMSw[i]=sqlite3_column_double(stmt5, 1);
                 //printf("%i  %f\n",iDF[i], dMSw[i]);
               }
         sqlite3_finalize(stmt5);  

         sqlite3_prepare_v2(handle,sql6,-1,&stmt6,0);
            for(i=0;i<iPlateCount;i++)
               {
                 sqlite3_step(stmt6);
                 iControlCount[i]=sqlite3_column_int(stmt6, 0);
                 dControls[i]=sqlite3_column_double(stmt6, 1);
               }
         sqlite3_finalize(stmt6);  

         sqlite3_close(handle);
         
         //Get Control Locations for Dunnett's critical values.
         for(i=0;i<iPlateCount;i++)
            {
              for(j=0;j<iGroupCount[i];j++)
                 {
                  if(iGroup[iCounter]==iControlValue)//from UI
                     {
                      iControlLocation[i]=iPosition;
                      //printf("Group Position %i\n", iControlLocation[i]);
                      iCounter++;
                      iPosition++;
                     }
                  else
                     {
                      iCounter++;
                      iPosition++;
                     }
                  }
              iPosition=1;
            }

         //Build confidence interval for Dunnett's test.
         GtkTextBuffer *buffer;
         buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
         char ColumnTitles[]="Plate CriticalValue Alpha BetweenDF WithinDF Control Test Lower Contrast Upper ControlSort TestSort MeanSortDistance\n";
         gtk_text_buffer_insert_at_cursor(buffer, ColumnTitles, -1);
         //Make array same size for all sets. Might want to change this. For testing.
         int CovarianceArray[iGroupCount[0]];

         for(i=0;i<iPlateCount;i++)//Number of plates
            { 
              dControl=dControls[i];

              //Set up array for covariance\correlation matrix.
              iStart=iCounter1;
              iCovSize=1;
               for(ii=0;ii<iGroupCount[i];ii++) //put values into an array for each plate.
                   {
                     CovarianceArray[ii]=iSetSize[iStart+ii];
                     printf("%i ", CovarianceArray[ii]);
                     iCovSize++;
                   }
               printf("\n");

               //update progress bar.
               gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), (double)i/(double)iPlateCount);
               gchar *message=g_strdup_printf("Calculating Plate %i out of %i", i+1,iPlateCount);
               gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), message);
               g_free(message);
               
               for(j=0;j<iGroupCount[i];j++)//Number of sets in a plate
                  {
                     if(iGroup[iCounter1]!=iControlValue)//group label of control. From the UI
                       {
                         GString *string=g_string_new("");
                         g_string_append_printf(string,"%i ", i+1);
                         if(iRadioCritVal==1)
                           {
                            CriticalValue1=bonferroni_critical_value(alpha, iGroupCount[i]-1, iDF[i]);
                           }
                         else if(iRadioCritVal==2)
                           {
                            CriticalValue1=dunn_sidak_critical_value(alpha, iGroupCount[i]-1, iDF[i]);
                           }
                         else
                           {
                            CriticalValue1=dunnetts_critical_value(alpha,CovarianceArray,iCovSize,(int)fabs(iControlLocation[i]-(j+1))+1,iDF[i]);
                           }

                         g_string_append_printf(string,"%f %f %i %i ", CriticalValue1, alpha, iGroupCount[i]-1, iDF[i]);
                         dTemp=dControl-dMean[iCounter1];
                         g_string_append_printf(string, "%f %f ", dControl, dMean[iCounter1]);
                         dTemp2=(1/(double)iControlCount[i])+(1/(double)iSetSize[j]);
                         dTemp3=dTemp-(CriticalValue1*sqrt(dMSw[i]*dTemp2));
                         dTemp4=dTemp+(CriticalValue1*sqrt(dMSw[i]*dTemp2));           
                         g_string_append_printf(string, "%f <uc%i-ut%i< %f ", dTemp3, iControlValue, iGroup[iCounter1], dTemp4); 
                         g_string_append_printf(string, "%i %i ", iControlLocation[i], j+1); 
                         g_string_append_printf(string, "%i\n", (int)fabs(iControlLocation[i]-(j+1))+1);
                         
                         gtk_text_buffer_insert_at_cursor(buffer, string->str, -1);
                         g_string_free(string,TRUE);
                       }
                     iCounter1++;//index for current record

                     //Process pending events for UI.
                  while(gtk_events_pending())
                       {
                         gtk_main_iteration();
                       }
                  if(*pBreakLoop==1)
                      {
                       //Break long running loop on cancel.
                       printf("Break Loop\n");
                       i=iPlateCount;
                       *pBreakLoop=0;
                       break;
                      }
                  }
            }

         free(iPlates);
         free(iGroup);
         free(iSetSize);
         free(dMean);
         free(dMSw);
         free(iDF);
         free(iGroupCount);
         free(dControls);  
         free(iControlCount);
         free(iControlLocation);
        }
double dunn_sidak_critical_value(double alpha, int Comparisons, int DF)
        {
          //Two tail students t
          //double alpha=0.05/2;//Divide by 2 for two tails
          alpha=alpha/2;
          double CritVal=0;
          double AlphaPrime=1.0-pow((1.0-alpha),(1.0/(double)Comparisons));

          CritVal=gsl_cdf_tdist_Qinv(AlphaPrime, DF);
          return CritVal;
        }
double bonferroni_critical_value(double alpha, int Comparisons, int DF)
        {
          //Two tail students t
          //double alpha=0.05/2;//Divide by 2 for two tails
          alpha=alpha/2;
          double CritVal=0;
          double AlphaPrime=alpha/(double)Comparisons;

          CritVal=gsl_cdf_tdist_Qinv(AlphaPrime, DF);
          return CritVal;
        }
double dunnetts_critical_value(double alpha1,int CovarianceArray[],int iCovSize,int iMeanDistance,int iDF)
        {
          //Match variables to Alan Genz's function.
      int iPlates=1;//Left over from test version. Set to 1.
      int N=iMeanDistance;
      int NU=iDF;
      int M=iMeanDistance;
      double COVRNCV1[iPlates*N*N];
      double CONSTR[N*M];
      double LOWER[M];//Set in Fortran
      double UPPER[M];//Set in Fortran
      int INFIN[M];//=2; Set in Fortran
      double DELTA[M];//=0.0; Set in Fortran
      //int MAXPTS=10000;
      //double ABSEPS=0.001;
      double ALPHA=alpha1;
      double RELEPS=0.0;
      int INFORM=0;
      int NEVALS=0;
      double TALPHA=0;
      double ERROR=0;
      double VALUE=0;
      
      double rho=0;
      int i=0;
      int j=0;
      int k=0;
      int iCounter1=0;
      int iCounter2=0;

      //Correlation matrix for Dunnett's.
      for(k=0;k<iPlates;k++)
         {
          //offset for the control or new set.
          iCounter1=N*k;
          for(i=0;i<N;i++)
             {
              for(j=0;j<N;j++)
                 {
                   if(j>i)
                     {
                       COVRNCV1[iCounter2]=0;
                     }
                   else if(i!=j)
                     {
                       rho=(1/(sqrt(CovarianceArray[iCounter1]/CovarianceArray[iCounter1+i]+1))*(1/sqrt(CovarianceArray[iCounter1]/CovarianceArray[iCounter1+j]+1)));
                       COVRNCV1[iCounter2]=rho;
                       printf("%f ", rho);
                     }
                   else
                     {
                       printf("1 ");
                       COVRNCV1[iCounter2]=1;
                     }
                   iCounter2++;
                 }
               printf("\n");
             }             
          }
      
      printf("\n");
      
      //printf("Bonferonni bound %f\n", gsl_cdf_tdist_Qinv(ALPHA/(2*N), NU));
      
      iCounter1=0;
      for(i=0;i<iPlates;i++)
         {
           double *COVRNCTEMP;//Had trouble sending multiple arrays but the problem was in the fortran code. Needed to reset K=1 in fortran. 
           COVRNCTEMP = (double *)malloc((N*N) * sizeof(double));
           for(j=0;j<(N*N);j++)
              {
                COVRNCTEMP[j]=COVRNCV1[iCounter1];
                //printf("%f ", COVRNCTEMP[j]);
                iCounter1++;
              }
           printf("\n");
           mvdistdun(&N, COVRNCTEMP, &NU, &M, LOWER, CONSTR, UPPER, INFIN, DELTA, &MAXPTS_C, &ABSEPS_C, &RELEPS, &ERROR, &VALUE, &NEVALS, &INFORM, &ALPHA, &TALPHA);
           printf("Critical Value %f Alpha %f\n\n", TALPHA, ALPHA);
           free(COVRNCTEMP);
         }          
    
      return TALPHA;
        }
void GenerateRandomValues(double dDataArray[], int iNumberOfPlates,int iPlateSize,int iSetSizeForStatistics, const gchar *pPlatePosControlText, const gchar *pPlateNegControlText)
     {
      //Generate random numbers in blocks or based on positional arguments.
      double dRndNumber;
      const gsl_rng_type *T;
      gsl_rng *r;
      gsl_rng_env_setup();
      T = gsl_rng_default;
      r = gsl_rng_alloc(T);

      int PosControlLength=strlen(pPlatePosControlText);
      int NegControlLength=strlen(pPlateNegControlText);
      GString *buffer=g_string_new("");
      GArray *PosArray=g_array_new(FALSE, FALSE, sizeof (int));
      GArray *NegArray=g_array_new(FALSE, FALSE, sizeof (int));
      guint32 iPosControlSize=0;
      guint32 iNegControlSize=0;
      //guint32 iblock=0;
      guint32 itemp=0;
      guint32 i=0;
      guint32 j=0;
      guint32 k=0;
      guint32 m=0;

     g_print("Start Random Number Generator\n");

       if(PosControlLength>0)
          {
            for(i=0;i<PosControlLength;i++)
              {
                if(pPlatePosControlText[i]!=','|| i==PosControlLength-1)
                   {
                    g_string_append_printf(buffer,"%c", pPlatePosControlText[i]);
                   }
                else
                   {
                    itemp=atoi(buffer->str);
                    itemp=itemp-1;
                    g_array_append_val(PosArray,itemp);
                    g_string_truncate(buffer,0);
                   }
               }
              itemp=atoi(buffer->str);
              itemp=itemp-1;
              g_array_append_val(PosArray,itemp);
              iPosControlSize=PosArray[0].len;
              g_string_truncate(buffer,0);
            }
               
        if(NegControlLength>0)
          {
            for(i=0;i<NegControlLength;i++)
              {
                if(pPlateNegControlText[i]!=','|| i==NegControlLength-1)
                   {
                    g_string_append_printf(buffer,"%c", pPlateNegControlText[i]);
                   }
                else
                   {
                    itemp=atoi(buffer->str);
                    itemp=itemp-1;
                    g_array_append_val(NegArray,itemp);
                    g_string_truncate(buffer,0);
                   }
               }
              itemp=atoi(buffer->str);
              itemp=itemp-1;
              g_array_append_val(NegArray,itemp);
              iNegControlSize=NegArray[0].len;
              g_string_truncate(buffer,0);
            }
         
      if(iPosControlSize>=1 && iNegControlSize>=1)
        {
          g_print("Random Number Generator with Controls\n");
          for (i=0; i<iNumberOfPlates; i++)
	       {
               for(j=0; j<iPlateSize; j++)
	           {
                       if(i*iPlateSize+j==i*iPlateSize+g_array_index(PosArray,int,k)&&k<iPosControlSize)
                         {
		            do
		              {
		                dRndNumber = gsl_rng_uniform(r) * 1000;
		              }
		            while (dRndNumber < 800);
	                   dDataArray[i*iPlateSize+j] = dRndNumber;
                           if(k<iPosControlSize)
                             k++;
                          }
                      else if(i*iPlateSize+j==i*iPlateSize+g_array_index(NegArray,int,m)&&m<iNegControlSize)
                          {
		            do
		              {
		                dRndNumber = gsl_rng_uniform(r) * 1000;
		              }
		            while (dRndNumber > 200);
	                   dDataArray[i*iPlateSize+j] = dRndNumber;
                           if(m<iNegControlSize)
                             m++;
                          }
                       else
                          {
                          dDataArray[i*iPlateSize+j] = gsl_rng_uniform(r) * 1000;
                          }
                     }
                   k=0;
                   m=0;
	         }
        }
        else
        {
         g_print("Random Number Generator with Stats Value\n");
         for (i=0; i<iNumberOfPlates; i++)
	       {
            for(j=(i*iPlateSize); j<(i*iPlateSize+iSetSizeForStatistics); j++)
	            {
		         do
		           {
		             dRndNumber = gsl_rng_uniform(r) * 1000;
		           }
		         while (dRndNumber < 800);
	             dDataArray[j] = dRndNumber;
	            }

            for(j=(i*iPlateSize+iSetSizeForStatistics); j<(i*iPlateSize+2*iSetSizeForStatistics); j++)
	            {
		         do
		           {
		             dRndNumber = gsl_rng_uniform(r) * 1000;
		           }
		         while (dRndNumber > 200);
	            dDataArray[j] = dRndNumber;
	            }

            for(j=(i*iPlateSize+2*iSetSizeForStatistics); j<(i*iPlateSize+iPlateSize); j++)
	            {
	            dDataArray[j] = gsl_rng_uniform(r) * 1000;
	            }

            }

        }
       gsl_rng_free(r);
       g_array_free(PosArray,TRUE);
       g_array_free(NegArray,TRUE);
       g_string_free(buffer,TRUE);
      }

void CalculatePercentControl(double dDataArray[], double dPercentArray[], int iPlateSize, int iNumberOfPlates, int iSetSizeForStatistics, const gchar *pPlatePosControlText, const gchar *pPlateNegControlText)
      {
         //A percent of control function that can take a block size or positional arguments.
	 double dPosControlSum=0;
	 double dPosControlAvg;
	 double dNegControlSum=0;
	 double dNegControlAvg;
        
         int PosControlLength=strlen(pPlatePosControlText);
         int NegControlLength=strlen(pPlateNegControlText);
         GString *buffer=g_string_new("");
         GArray *PosArray=g_array_new(FALSE, FALSE, sizeof (int));
         GArray *NegArray=g_array_new(FALSE, FALSE, sizeof (int));
         int iPosControlSize=0;
         int iNegControlSize=0;
         guint32 i=0;
         guint32 k=0;
         guint32 itemp=0;
          
         g_print("Calculate Percent of Control\n");
         if(PosControlLength>0)
          {
            for(i=0;i<PosControlLength;i++)
              {
                if(pPlatePosControlText[i]!=','|| i==PosControlLength-1)
                   {
                    g_string_append_printf(buffer,"%c", pPlatePosControlText[i]);
                   }
                else
                   {
                    itemp=atoi(buffer->str);
                    itemp=itemp-1;
                    g_array_append_val(PosArray,itemp);
                    g_string_truncate(buffer,0);
                   }
               }
              itemp=atoi(buffer->str);
              itemp=itemp-1;
              g_array_append_val(PosArray,itemp);
              iPosControlSize=PosArray[0].len;
              g_string_truncate(buffer,0);
            }
               
        if(NegControlLength>0)
          {
            for(i=0;i<NegControlLength;i++)
              {
                if(pPlateNegControlText[i]!=','|| i==NegControlLength-1)
                   {
                    g_string_append_printf(buffer,"%c", pPlateNegControlText[i]);
                   }
                else
                   {
                    itemp=atoi(buffer->str);
                    itemp=itemp-1;
                    g_array_append_val(NegArray,itemp);
                    g_string_truncate(buffer,0);
                   }
               }
              itemp=atoi(buffer->str);
              itemp=itemp-1;
              g_array_append_val(NegArray,itemp);
              iNegControlSize=NegArray[0].len;
              g_string_truncate(buffer,0);
            }
         
        if(iPosControlSize>=1 && iNegControlSize>=1)
         {
          g_print("Percent with Control Values\n");
	  for(int i=0; i<iNumberOfPlates; i++)
           {
             for(int j=(i*iPlateSize); j<(i*iPlateSize+iPosControlSize); j++)
		 {
			 dPosControlSum += dDataArray[(i*iPlateSize)+g_array_index(PosArray,int,k)];
                         k++;
		 }
	       dPosControlAvg = dPosControlSum/iPosControlSize;
               k=0;
             for(int j=(i*iPlateSize); j<(i*iPlateSize+iNegControlSize); j++)
		 {
			 dNegControlSum += dDataArray[(i*iPlateSize)+g_array_index(NegArray,int,k)];
                         k++;
		 }
	       dNegControlAvg = dNegControlSum/iNegControlSize;
               k=0;
             for(int j=(i*iPlateSize); j<(i*iPlateSize+iPlateSize); j++)
		 {
		         dPercentArray[j] = ((dDataArray[j]-dNegControlAvg)/(dPosControlAvg-dNegControlAvg))*100;
		 }
			 dPosControlSum=0;
			 dNegControlSum=0;
          }
         }
         else
          {
           g_print("Percent with Set Size for Stats\n");
            for(int i=0; i<iNumberOfPlates; i++)
           {
             for(int j=(i*iPlateSize); j<(i*iPlateSize+iSetSizeForStatistics); j++)
		 {
			 dPosControlSum += dDataArray[j];
		 }
	       dPosControlAvg = dPosControlSum/iSetSizeForStatistics;
             for(int j=(i*iPlateSize+iSetSizeForStatistics); j<(i*iPlateSize+2*iSetSizeForStatistics); j++)
		 {
			 dNegControlSum += dDataArray[j];
		 }
	       dNegControlAvg = dNegControlSum/iSetSizeForStatistics;
             for(int j=(i*iPlateSize); j<(i*iPlateSize+iPlateSize); j++)
		 {
		         dPercentArray[j] = ((dDataArray[j]-dNegControlAvg)/(dPosControlAvg-dNegControlAvg))*100;
		 }
			 dPosControlSum=0;
			 dNegControlSum=0;
          }
         }
        g_array_free(PosArray,TRUE);
        g_array_free(NegArray,TRUE);
        g_string_free(buffer,TRUE);
       }
void PlateMapInt(double dDataArray[], int iNumberOfPlates, int iPlateSize, int iRows, int iColumns)
     {
         //Put a column of numbers into row, column format. (int) version.
         guint32 check=0;
         guint32 i=0;
         guint32 j=0;
         guint32 k=0;
         GString *buffer=g_string_new(NULL);
         GtkClipboard *clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
          
         g_print("Create Plate Map(int)\n"); 
         for(i=0; i<iNumberOfPlates; i++)
           {
              for(j=0; j<iRows; j++)
                 {
                   for(k=0; k<iColumns; k++)
                        {
                          g_string_append_printf(buffer,"%i ", (int)dDataArray[check]); 
                          check++;
                        }
                     g_string_append_printf(buffer,"\n");  
                 }
                g_string_append_printf(buffer,"\n");
           }
             
           gtk_clipboard_set_text(clipboard, buffer->str, strlen(buffer->str));
           g_string_free(buffer, TRUE);
     }

void PlateMapDouble(double dDataArray[], int iNumberOfPlates, int iPlateSize, int iRows, int iColumns)
     {
         //Put a column of numbers into row, column format. (double) version.
         guint32 check=0;
         guint32 i=0;
         guint32 j=0;
         guint32 k=0;
         GString *buffer=g_string_new(NULL);
         GtkClipboard *clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
          
         g_print("Create Plate Map(float)\n");  
         for(i=0; i<iNumberOfPlates; i++)
           {
              for(j=0; j<iRows; j++)
                 {
                   for(k=0; k<iColumns; k++)
                        {
                          g_string_append_printf(buffer,"%f ", dDataArray[check]); 
                          check++;
                        }
                     g_string_append_printf(buffer,"\n");  
                 }
                g_string_append_printf(buffer,"\n");
           }
             
           gtk_clipboard_set_text(clipboard, buffer->str, strlen(buffer->str));
           g_string_free(buffer, TRUE);
     }
void send_text_to_database(const gchar *pTableName, GtkWidget *textview)
     {
       GtkTextBuffer *buffer1;
       GtkTextIter start1;
       GtkTextIter end1;
       
       buffer1=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
       gtk_text_buffer_get_bounds(buffer1, &start1, &end1);

       gchar *string1=g_strdup(gtk_text_buffer_get_text(buffer1, &start1, &end1, TRUE));
       gchar *string2=g_strdup(pTableName);
       char *p1;
       char prev='~';
       p1=string1;
       int check1=0;

       if(string1!=NULL)
         {
           while(*p1!='\0')
             {
               if(*p1==' '&&prev==' ')
                 {
                   printf("Two or more spaces together in text!\n");
                   simple_message_dialog("The text needs to be in tablular format\n with single spaces between values.");
                   check1=1;
                   break;
                 }
               else if(*p1=='\n'&&prev=='\n')
                 {
                   printf("There is a space between rows!\n");
                   simple_message_dialog("The text needs to be in tablular format\n with single spaces between values.");
                   check1=1;
                   break;
                 }
               else if(*p1==' ')
                 {
                   prev=*p1;
                   *p1='|';
                 }
               else
                 {
                   prev=*p1;
                 }
               p1++;
             }

           if(check1==0)
             {
               FILE *fp;
               fp=fopen("stats_with_pipes.txt", "w");
               fprintf(fp, "%s", string1);
               fclose(fp);
           
               int retval=0;
               apop_db_open("VelociRaptorData.db");
               apop_query("DROP TABLE IF EXISTS %s;", string2);
               printf("Send Text to Database.\n");
               retval=apop_text_to_db("stats_with_pipes.txt", string2,0,1,NULL);
               printf("\n%i Rows Sent to Database.\n", retval);
               apop_db_close(1);
             }
         }
         

       if(string1!=NULL)
         {
           g_free(string1);
         }
       if(string2!=NULL)
         {
           g_free(string2);
         }

     }
void build_combo_table_sql(int ComboSet, int ComboSubSet, const gchar *TableName)
  {
         gsl_combination *Set1;
         gsl_combination *Set2;
         FILE *f=NULL;
         char FileName[]="VelociRaptorCombos.sql";
         int FileError=0;
         int i=0;;

          //Send the SQL combo script to the screen
          printf("Write Combos to Screen\n");
          Set1=gsl_combination_calloc(ComboSet,ComboSubSet);
          printf("BEGIN TRANSACTION;\n");
              do
                {
                  printf("INSERT INTO %s VALUES(", TableName);
                  gsl_combination_fprintf(stdout, Set1, "%u,");
                  printf("\b);\n");
                }
              while(gsl_combination_next(Set1)==GSL_SUCCESS);
          printf("END TRANSACTION;\n");
          gsl_combination_free(Set1);


      /*
      Print a subset of combinations to a file. Problem with formating to a file. The last comma gets left. Can't use \b. \b is only good for the terminal. Put a * character in so that ,* can be found and trimmed off in the file. Create a table and load data in.
      */
          printf("Write Combos to %s File\n", FileName);
          Set2=gsl_combination_calloc(ComboSet,ComboSubSet);
          f = fopen(FileName, "w");
          if(f)
             {
              fprintf(f,"DROP TABLE IF EXISTS %s;", TableName);
              fprintf(f,"CREATE TABLE IF NOT EXISTS %s (", TableName);
                for(i=1;i<=ComboSubSet;i++)
                  {
                    fprintf(f,"combos%i int,", i);
                  }
              fprintf(f,"*);\n");

              fprintf(f, "BEGIN TRANSACTION;\n");
                 do
                  {
                    fprintf(f, "INSERT INTO %s VALUES(", TableName);
                    gsl_combination_fprintf(f, Set2, "%u,");
                    fprintf(f, "*);\n");
                  }
                while(gsl_combination_next(Set2)==GSL_SUCCESS);
              fprintf(f, "END TRANSACTION;\n");
              fclose(f);
             }
          else
             {
              printf("Couldn't Open %s\n", FileName);
              FileError=1;
             }
          gsl_combination_free(Set2);

      /*
       Open the file, read into an array and remove the ,* characters.
      */
          printf("Read File into Buffer\n");
          int size=1;
          f = fopen(FileName, "r");
          fseek(f, 0, SEEK_END);
          size = ftell(f); 
          fseek(f, 0, SEEK_SET);
          char *buffer=malloc(size*sizeof(char));
          int ch;
          char *p;
          p=buffer; 
          if(f)
            {
              ch = getc(f);
                 while (ch != EOF)
                    {
                         if(ch==42)
                           {
                            //find *, ascii character 42 and back up the pointer by 2. 
                             p=p-2;                   
                           }
                         else
                           {
                            //printf("%c", ch);
                             *p=ch;
                           }
                       p++;
                       ch = getc(f);
                    }
              //printf("Buffer to File\n%s\n", buffer);
              fclose(f);
            }
          else
            {
              printf("Couldn't Open %s\n", FileName);
              FileError=1;
            }

          //Load the buffer array back into the file or delete the file.
          f = fopen(FileName, "w");
          p=buffer;
          if(f)
            {
              while (*p != '\0')
                 {
                   fprintf(f, "%c", *p);
                   p++;
                 }
              fclose(f);
             }
          else
             {
               printf("Couldn't Open %s\n", FileName);
               FileError=1;
             }

          //Send the buffer to the database
          if(FileError==0)
             {
               printf("Send Buffer to Database\n");
               //int retval;
               sqlite3 *handle;

               sqlite3_open("VelociRaptorData.db",&handle);
               //char *zSQL = sqlite3_mprintf("%s", buffer);

               sqlite3_exec(handle, buffer, 0, 0, 0);
               //sqlite3_free(zSQL);

               sqlite3_close(handle);
             }
          free(buffer);
  }
void build_permutation_table_sql(int ComboSet, const gchar *TableName)
  {
         gsl_permutation *Set1;
         gsl_permutation *Set2;
         FILE *f=NULL;
         char FileName[]="permutations.sql";
         int FileError=0;
         int i=0;

          //Send the SQL Permutations script to the screen
          printf("Write Permutations to Screen\n");
          Set1=gsl_permutation_calloc(ComboSet);
          printf("BEGIN TRANSACTION;\n");
              do
                {
                  printf("INSERT INTO %s VALUES(", TableName);
                  gsl_permutation_fprintf(stdout, Set1, "%u,");
                  printf("\b);\n");
                }
              while(gsl_permutation_next(Set1)==GSL_SUCCESS);
          printf("END TRANSACTION;\n");
          gsl_permutation_free(Set1);


      /*
      Print a subset of combinations to a file. Problem with formating to a file. The last comma gets left. Can't use \b. \b is only good for the terminal. Put a * character in so that ,* can be found and trimmed off in the file. Create a table and load data in.
      */
          printf("Write Permutations to %s File\n", FileName);
          Set2=gsl_permutation_calloc(ComboSet);
          f = fopen(FileName, "w");
          if(f)
             {
              fprintf(f,"DROP TABLE IF EXISTS %s;", TableName);
              fprintf(f,"CREATE TABLE IF NOT EXISTS %s (", TableName);
                for(i=1;i<=ComboSet;i++)
                  {
                    fprintf(f,"permuts%i int,", i);
                  }
              fprintf(f,"*);\n");

              fprintf(f, "BEGIN TRANSACTION;\n");
                 do
                  {
                    fprintf(f, "INSERT INTO %s VALUES(", TableName);
                    gsl_permutation_fprintf(f, Set2, "%u,");
                    fprintf(f, "*);\n");
                  }
                while(gsl_permutation_next(Set2)==GSL_SUCCESS);
              fprintf(f, "END TRANSACTION;\n");
              fclose(f);
             }
          else
             {
              printf("Couldn't Open %s\n", FileName);
              FileError=1;
             }
          gsl_permutation_free(Set2);

      /*
       Open the file, read into an array and remove the ,* characters.
      */
          printf("Read File into Buffer\n");
          int size=1;
          f = fopen(FileName, "r");
          fseek(f, 0, SEEK_END);
          size = ftell(f); 
          fseek(f, 0, SEEK_SET);
          char *buffer=malloc(size*sizeof(char));
          int ch;
          char *p;
          p=buffer; 
          if(f)
            {
              ch = getc(f);
                 while (ch != EOF)
                    {
                         if(ch==42)
                           {
                             //find *, ascii character 42 and back up the pointer by 2. 
                             p=p-2;                   
                           }
                         else
                           {
                             //printf("%c", ch);
                             *p=ch;
                           }
                       p++;
                       ch = getc(f);
                    }
              //printf("Buffer to File\n%s\n", buffer);
              fclose(f);
            }
          else
            {
              printf("Couldn't Open %s\n", FileName);
              FileError=1;
            }

      //Load the buffer array back into the file or delete the file.
          f = fopen(FileName, "w");
          p=buffer;
          if(f)
            {
              while (*p != '\0')
                 {
                   fprintf(f, "%c", *p);
                   p++;
                 }
              fclose(f);
             }
          else
             {
               printf("Couldn't Open %s\n", FileName);
               FileError=1;
             }

      //Send the buffer to the database
          if(FileError==0)
             {
               printf("Send Buffer to Database\n");
               //int retval;
               sqlite3 *handle;

               sqlite3_open("VelociRaptorData.db",&handle);
               //char *zSQL = sqlite3_mprintf("%s", buffer);

               sqlite3_exec(handle, buffer, 0, 0, 0);
               //sqlite3_free(zSQL);

               sqlite3_close(handle);
             }

          free(buffer);
  }
void copy_treeview_to_database_sql(GtkWidget *copy, GtkWidget *treeview, const gchar *pWindowTitle)
  {
    //Save TreeView data to a SQLite database.
    GtkTreeIter iter;
    GtkTreeModel *model;
    guint32 iModelRows=0;
    double dTemp=0;
    guint32 DataCount=0; 
    sqlite3_stmt *stmt;
    sqlite3 *handle;
    GtkTreeViewColumn *column;
    const char *column_name;

    column=gtk_tree_view_get_column(GTK_TREE_VIEW(treeview), 1);
    column_name=gtk_tree_view_column_get_title(column);
    printf("Column Title %s\n",column_name); 
    
    sqlite3_open("VelociRaptorData.db",&handle);
    // Create the SQL query for creating a table
    char create_table[200] = "CREATE TABLE IF NOT EXISTS Data (KeyID int PRIMARY KEY, Data Real NOT NULL, Percent Real NOT NULL)";
    
    // Execute the query for creating the table
    sqlite3_exec(handle,create_table,0,0,0);

    char *zSQL = sqlite3_mprintf("SELECT COUNT(*) FROM Data;");
    sqlite3_prepare_v2(handle,zSQL,-1,&stmt,0);
    sqlite3_step(stmt);
    DataCount = sqlite3_column_int(stmt, 0);
    printf("Data Table Count = %i\n", DataCount);
    sqlite3_free(zSQL);

    model=gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
    gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");

         do
            {
              iModelRows++;
            }
         while(gtk_tree_model_iter_next(model,&iter));
         gtk_tree_model_get_iter_from_string(GTK_TREE_MODEL(model), &iter, "0");

         if(DataCount!=iModelRows && DataCount!=0)
           {
             if((g_strcmp0(pWindowTitle, "Plate Data")==0)||(g_strcmp0(pWindowTitle, "Percent")==0))
                {
                 //truncate data table
                 char *zSQL3 = sqlite3_mprintf("DELETE FROM Data;");
                 sqlite3_exec(handle, zSQL3, 0, 0, 0);
                 printf("Data Table Truncated.\n");
                 DataCount=0;
                 sqlite3_free(zSQL3);
                }
            }

         iModelRows=0;
         printf("Window Title is %s\n", pWindowTitle);
          if(DataCount==0 && (g_strcmp0(pWindowTitle, "Plate Data")==0))
            {
             sqlite3_stmt *stmt5;
             sqlite3_prepare(handle, "INSERT INTO Data VALUES (?,?,0);", -1, &stmt5, 0);
             sqlite3_exec(handle, "BEGIN TRANSACTION", NULL, NULL, NULL);
              do
                 {
                  gtk_tree_model_get(model, &iter, 1, &dTemp, -1);
                  printf("INSERT INTO Data VALUES(%i,%f,0)\n", iModelRows+1, dTemp);
                  sqlite3_bind_int(stmt5, 1, iModelRows+1);
                  sqlite3_bind_double(stmt5, 2, dTemp); 
                  sqlite3_step(stmt5);
                  sqlite3_reset(stmt5);
                  iModelRows++; 
                 }
              while(gtk_tree_model_iter_next(model,&iter));
             sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
             sqlite3_finalize(stmt5);
            }
          if(DataCount!=0 && (g_strcmp0(pWindowTitle, "Plate Data")==0))
            {
             sqlite3_stmt *stmt6;
             sqlite3_prepare(handle, "UPDATE Data SET Data=? WHERE KeyID =?;", -1, &stmt6, 0);
             sqlite3_exec(handle, "BEGIN TRANSACTION", NULL, NULL, NULL);
              do
                 {
                  gtk_tree_model_get(model, &iter, 1, &dTemp, -1);
                  printf("UPDATE Data SET Data=%f WHERE KeyID =%i;\n", dTemp, iModelRows+1);
                  sqlite3_bind_double(stmt6, 1, dTemp); 
                  sqlite3_bind_int(stmt6, 2, iModelRows+1);
                  sqlite3_step(stmt6);
                  sqlite3_reset(stmt6);
                  iModelRows++;
                 }
              while(gtk_tree_model_iter_next(model,&iter));
             sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
             sqlite3_finalize(stmt6);
            }
          if(DataCount==0 && (g_strcmp0(pWindowTitle, "Percent")==0))
            {
             sqlite3_stmt *stmt7;
             sqlite3_prepare(handle, "INSERT INTO Data VALUES(?,0,?);", -1, &stmt7, 0);
             sqlite3_exec(handle, "BEGIN TRANSACTION", NULL, NULL, NULL);
              do
                 {
                  gtk_tree_model_get(model, &iter, 1, &dTemp, -1);
                  printf("INSERT INTO Data VALUES(%i,0,%f);\n", iModelRows+1, dTemp);
                  sqlite3_bind_int(stmt7, 1, iModelRows+1);
                  sqlite3_bind_double(stmt7, 2, dTemp);
                  sqlite3_step(stmt7);
                  sqlite3_reset(stmt7); 
                  iModelRows++;
                 }
              while(gtk_tree_model_iter_next(model,&iter));
             sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
             sqlite3_finalize(stmt7);
            }
          if(DataCount!=0 && (g_strcmp0(pWindowTitle, "Percent")==0))
            {
             sqlite3_stmt *stmt8;
             sqlite3_prepare(handle, "UPDATE Data SET Percent=? WHERE KeyID=?;", -1, &stmt8, 0);
             sqlite3_exec(handle, "BEGIN TRANSACTION", NULL, NULL, NULL);
              do
                 {
                  gtk_tree_model_get(model, &iter, 1, &dTemp, -1);
                  printf("UPDATE Data SET Percent=%f WHERE KeyID=%i;\n", dTemp, iModelRows+1);
                  sqlite3_bind_double(stmt8, 1, dTemp);
                  sqlite3_bind_int(stmt8, 2, iModelRows+1);
                  sqlite3_step(stmt8);
                  sqlite3_reset(stmt8); 
                  iModelRows++;
                 }
              while(gtk_tree_model_iter_next(model,&iter));
             sqlite3_exec(handle, "END TRANSACTION", NULL, NULL, NULL);
             sqlite3_finalize(stmt8);
            }
     
     sqlite3_finalize(stmt);
     sqlite3_close(handle);
     printf("Data Export Finished!\n");
  }




