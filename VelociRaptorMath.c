
/* Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.*/

//for using popen() 
#define _XOPEN_SOURCE //500

#include <gtk/gtk.h>
#include <glib.h>
#include <sqlite3.h>
#include <apop.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>
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
     printf("Descriptive Stats\n");
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
void anderson_darling_test(GtkTextView *textview, int iRadioButton)
   {
    int i=0;
    int j=0;
    int k=0;
    double temp1=0;
    int temp2=0;
    double temp3=0;
    double temp4=0;
    double temp5=0;
    int temp6=0;
    int temp7=0;
    double A2=0;
    double A2_1=0;
    double A2_2=0;
    double Pvalue=0;
    int iCounter=0;
    apop_data *AndersonsStats1=NULL;
    gsl_vector *vAndersonsData1=NULL;
    gsl_vector *v1=NULL;
    GString *string=g_string_new("");
    GtkTextBuffer *buffer;
    buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    apop_db_open("VelociRaptorData.db");

      if(iRadioButton==1)
        {
          AndersonsStats1=apop_query_to_data("SELECT T4.plate, count(T3.data), avg(T3.data), stddev_samp(T3.data) FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID GROUP BY T4.Groups,T4.Plate ORDER BY T4.Plate,T4.Groups;");
          vAndersonsData1=apop_query_to_vector("SELECT T3.data FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID ORDER BY T4.Plate,T4.Groups,T3.data asc;");
        }
      if(iRadioButton==2)
        {
          AndersonsStats1=apop_query_to_data("SELECT T4.plate, count(T3.percent), avg(T3.percent), stddev_samp(T3.percent) FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID GROUP BY T4.Groups,T4.Plate ORDER BY T4.Plate,T4.Groups;");
          vAndersonsData1=apop_query_to_vector("SELECT T3.percent FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID ORDER BY T4.Plate,T4.Groups,T3.percent asc;");
        }
      if(iRadioButton==3)
        {
          AndersonsStats1=apop_query_to_data("SELECT T4.plate, count(T3.data), avg(T3.data), stddev_samp(T3.data) FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID AND T4.Picks!=0 GROUP BY T4.Picks,T4.Plate ORDER BY T4.Plate,T4.Picks;");
          vAndersonsData1=apop_query_to_vector("SELECT T3.data FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID AND T4.Picks!=0 ORDER BY T4.Plate,T4.Picks,T3.data asc;");
        }
      if(iRadioButton==4)
        {
          AndersonsStats1=apop_query_to_data("SELECT T4.plate, count(T3.percent), avg(T3.percent), stddev_samp(T3.percent) FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID AND T4.Picks!=0 GROUP BY T4.Picks,T4.Plate ORDER BY T4.Plate,T4.Picks;");
          vAndersonsData1=apop_query_to_vector("SELECT T3.percent FROM data T3, aux T4 WHERE T3.KeyID=T4.KeyID AND T4.Picks!=0 ORDER BY T4.Plate,T4.Picks,T3.percent asc;");
        }

      v1=gsl_vector_alloc(vAndersonsData1->size);
   
    apop_db_close(0);

    //Get (x-u)/s and put in v1.
    for(i=0;i<AndersonsStats1->matrix->size1;i++)
       {
          for(j=0;j<AndersonsStats1->matrix->size2;j++)
             {
                temp1=apop_data_get(AndersonsStats1,i,j);
                //group count
                if(j==1)
                  {
                    temp2=(int)temp1;
                  }
                //avg value
                if(j==2)
                  {
                    temp3=temp1;
                  }
                //stdev value
                if(j==3)
                  {
                    temp4=temp1;
                    //Loop the group count number down the data vector.
                    for(k=0;k<temp2;k++)
                       {
                         temp5=(gsl_vector_get(vAndersonsData1,iCounter)-temp3)/temp4;
                         //printf("%i %f\n", iCounter, temp5);
                         gsl_vector_set(v1,iCounter,temp5);
                         iCounter++;
                       }
                  }
             }
       }

    //Get A2 test statistic.
    temp5=0;
    iCounter=0;
    for(i=0;i<AndersonsStats1->matrix->size1;i++)
       {
          for(j=0;j<AndersonsStats1->matrix->size2;j++)
             {
                temp1=apop_data_get(AndersonsStats1,i,j);
                //Plate
                if(j==0)
                  {
                    temp2=(int)temp1;
                  }
                //Group count
                if(j==1)
                  {
                    temp6=(int)temp1;
                  }
                if(j==2)
                  {
                    temp3=temp1;
                  }
                //Loop the group count number down the data vector.
                if(j==3)
                  {
                   temp4=temp1;
                   temp7=iCounter+temp6;
                    for(k=1;k<=temp6;k++)
                       {
                         temp5=temp5+((2.0*(double)k-1.0)*(log(gsl_cdf_ugaussian_P(gsl_vector_get(v1,iCounter)))+log(1.0-gsl_cdf_ugaussian_P(gsl_vector_get(v1,temp7-k)))));  
                         //printf("%i %i %i %f %f %f %f\n", iCounter, k, temp7-k, temp5, gsl_cdf_ugaussian_P(gsl_vector_get(v1,iCounter)), 1.0-gsl_cdf_ugaussian_P(gsl_vector_get(v1,temp7-k)), temp5) ;
                         iCounter++;
                       }
                  }
             }
           //A2
           A2=((1.0/(double)temp6)*temp5);
           A2=-(double)temp6-A2;
           //A2 adjusted from Wikipedia
           A2_1=1.0+(4.0/(double)temp6)-(25.0/pow((double)temp6,2));
           A2_1=A2*A2_1;
           //A2 for small sample sizes 
           A2_2=1.0+0.75/temp6+2.25/pow((double)temp6,2);
           A2_2=A2*A2_2;
           //R.B. D'Agostino and M.A. Stephens, Eds. (1986). Goodness-of-Fit Techniques, Marcel Dekker.
           if(A2_2>=0.6)
             {
               Pvalue=exp(1.2937-5.709*A2_2+0.0186*pow(A2_2,2));
             }
           else if(A2_2>=0.34&&A2_2<0.6)
             {
               Pvalue=exp(0.9177-4.279*A2_2-1.38*pow(A2_2,2));
             }
           else if(A2_2>=0.20&&A2_2<0.34)
             {
               Pvalue=1-exp(-8.318 + 42.796*A2_2-59.938*pow(A2_2,2));
             }
           else //<0.2
             {
               Pvalue=1-exp(-13.436+101.14*A2_2- 223.73*pow(A2_2,2));
             }
           //printf("%i %i %f %f %f %f %f\n", temp2, temp6, temp3, temp4, A2, A2_2, Pvalue); 
           g_string_append_printf(string,"%i %i %f %f %f %f %f\n", temp2, temp6, temp3, temp4, A2, A2_2, Pvalue);
           temp5=0;
       }

    printf("Plate Count Mean StdDevS A2 A2adjusted p_value\n");
    printf("%s", string->str);
    gtk_text_buffer_insert_at_cursor(buffer, "Plate Count Mean StdDevS A2 A2adjusted p_value\n", -1);
    gtk_text_buffer_insert_at_cursor(buffer, string->str, -1);
       
    g_string_free(string,TRUE);

    if(vAndersonsData1!=NULL)
      {
        gsl_vector_free(vAndersonsData1);
      }
    if(AndersonsStats1!=NULL)
      {
        gsl_matrix_free(AndersonsStats1->matrix);
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
      GString *string=g_string_new(" ");
      g_string_append_printf(string, "one = %f\n", one);
      g_string_append_printf(string, " two = %f\n", two);
      g_string_append_printf(string, " three = %f\n\n", three);
      g_string_append_printf(string, "Plate %i\n", iPlates);
      g_string_append_printf(string, "F Critical Value %f with(%f, %i, %i)\n", dCriticalValue, alpha, iBetweenDf, iWithinDf);
      g_string_append_printf(string, "Source         df          SS                    MS                F\n");
      g_string_append_printf(string, "Between        %i     ", iBetweenDf);
      g_string_append_printf(string, "  %f       %f      %f\n", three-two, (three-two)/iBetweenDf, ((three-two)/iBetweenDf)/((one-three)/iWithinDf));
      g_string_append_printf(string, "Within         %i    ", iWithinDf);
      g_string_append_printf(string, "   %f       %f\n", one-three, (one-three)/iWithinDf);
      g_string_append_printf(string, "Total          %i    ", iTotalDf);
      g_string_append_printf(string, "   %f\n\n", one-two);

      gtk_text_buffer_insert_at_cursor(buffer, string->str, -1);
      g_string_free(string,TRUE);
    }
void anova_format_tabular(GtkTextView *textview, int iPlates, int iBetweenDf,int iWithinDf,int iTotalDf, double one, double two, double three, double alpha)
   {
      int as_return=0;
      GtkTextBuffer *buffer1;
      buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
      double dCriticalValue=gsl_cdf_fdist_Qinv(alpha,iBetweenDf,iWithinDf);

       char *string;
       as_return=asprintf(&string, "%i %i %i %i %f %f %f %f %f %f %f\n", iPlates, iBetweenDf, iWithinDf, iTotalDf, three-two, one-three, one-two, (three-two)/iBetweenDf, (one-three)/iWithinDf, ((three-two)/iBetweenDf)/((one-three)/iWithinDf), dCriticalValue  );
       if(as_return!=-1)
         {
           gtk_text_buffer_insert_at_cursor(buffer1, string, -1);
           free(string); 
         }
       else
         {
           printf("Memory allocation error in asprintf.\n");
         }
   }
void database_to_box_graph_sql(int iRadioButton, int iCombo, int lower_bound, int upper_bound)
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

        //apop_query_to_matrix removed from Apophenia library.
        apop_data *db_data=NULL;
        if(iRadioButton==1)
          {
            db_data=apop_query_to_data("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Data AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 4, iCombo, lower_bound, upper_bound);       
            //plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Data AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);       
          }
        if(iRadioButton==2)
          {   
            db_data=apop_query_to_data("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Percent AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 4, iCombo, lower_bound, upper_bound);  
            //plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Percent AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);            
          }
        if(iRadioButton==3)
          { 
            db_data=apop_query_to_data("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Data AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 4, iCombo, lower_bound, upper_bound);  
            //plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Data AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);        
          }
        if(iRadioButton==4)
          { 
            db_data=apop_query_to_data("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Percent AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 4, iCombo, lower_bound, upper_bound);
            //plot_matrix_now(apop_query_to_matrix("SELECT T4.BoxID, T3.D1 FROM (SELECT Plate|| '0' ||Groups AS V1, T2.Percent AS D1 FROM aux AS T1, data AS T2 WHERE T1.KeyID=T2.KeyID) AS T3, BoxGraphTemp AS T4 WHERE T3.V1=T4.Order1 AND BoxID >= %i AND BoxID <= %i ORDER BY T4.Order2;", lower_bound, upper_bound), 4, lower_bound, upper_bound);          
                 }
        apop_query("DROP TABLE IF EXISTS BoxGraphTemp;");
        apop_db_close(0);
        apop_data_free(db_data);
   }
void database_to_error_graph_sql(int iRadioButton1, int iRadioButton2, int iCombo, int lower_bound, int upper_bound)
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
 
        //apop_query_to_matrix removed from Apophenia library.
        apop_data *db_data=NULL;
        if(iRadioButton2==1)
          {
            db_data=apop_query_to_data("SELECT ErrorID, Mean, StandardDev FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 3, iCombo, lower_bound, upper_bound);
            //plot_matrix_now(apop_query_to_matrix("SELECT ErrorID, Mean, StandardDev FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound), 3, lower_bound, upper_bound);   
          }
        if(iRadioButton2==2)
          {  
            db_data=apop_query_to_data("SELECT ErrorID, Mean, (2.0*StandardDev) FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 3, iCombo, lower_bound, upper_bound);
            //plot_matrix_now(apop_query_to_matrix("SELECT ErrorID, Mean, (2.0*StandardDev) FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound), 3, lower_bound, upper_bound);     
          }
        if(iRadioButton2==3)
          {    
            db_data=apop_query_to_data("SELECT ErrorID, Mean, (3.0*StandardDev) FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 3, iCombo, lower_bound, upper_bound);
            //plot_matrix_now(apop_query_to_matrix("SELECT ErrorID, Mean, (3.0*StandardDev) FROM ErrorGraphTemp WHERE ErrorID >= %i AND ErrorID <= %i;", lower_bound, upper_bound), 3, lower_bound, upper_bound);
          }
        
        apop_query("DROP TABLE IF EXISTS ErrorGraphTemp;");
        apop_db_close(0);
        apop_data_free(db_data);
   }
void database_to_scatter_graph_sql(int iRadioButton, int iCombo, int lower_bound, int upper_bound)
   {
      apop_opts.db_engine='l';
      apop_db_open("VelociRaptorData.db");
      //apop_query_to_matrix removed from Apophenia library.
      apop_data *db_data=NULL;
        if(iRadioButton==1)
          {
            db_data=apop_query_to_data("SELECT Data FROM data WHERE KeyID >= %i AND KeyID <= %i;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 1, iCombo, lower_bound, upper_bound);
            //plot_matrix_now(apop_query_to_matrix("SELECT Data FROM data WHERE KeyID >= %i AND KeyID <= %i;", lower_bound, upper_bound), 1, lower_bound, upper_bound);       
          }
        if(iRadioButton==2)
          {  
            db_data=apop_query_to_data("SELECT Percent FROM data WHERE KeyID >= %i AND KeyID <= %i;", lower_bound, upper_bound);
            plot_matrix_now(db_data->matrix, 1, iCombo, lower_bound, upper_bound); 
            //plot_matrix_now(apop_query_to_matrix("SELECT Percent FROM data WHERE KeyID >= %i AND KeyID <= %i;", lower_bound, upper_bound), 2, lower_bound, upper_bound);
          }
              
      apop_db_close(0);
      apop_data_free(db_data);
   }
void plot_matrix_now(gsl_matrix *data, int graph, int iCombo, int lower_bound, int upper_bound)
   {
     FILE *gp=NULL;
     char *terminal=NULL;

     if(iCombo==0) terminal=g_strdup("wxt");
     else terminal=g_strdup("x11");

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
                  fprintf(gp, "set terminal %s title 'Quick Gnuplot Scatter'; set xrange[%i:%i]; plot '-' notitle\n", terminal, lower_bound-1, upper_bound+1);
                }
              if(graph==3)
                {
                  fprintf(gp, "set terminal %s title 'Quick Gnuplot Error'; set xrange[%i:%i]; plot '-' with errorbars notitle\n", terminal, lower_bound-1, upper_bound+1);
                }
              if(graph==4)
                {
                  fprintf(gp, "set terminal %s title 'Quick Gnuplot Box'; set xrange[*:*]; set yrange [*:*]; set style fill solid 0.25 border -1; set style boxplot outliers pointtype 7; set style data boxplot; plot '-' using (1.0):($2):(0):1 notitle\n", terminal);
                  
                }
               
              //apophenia 0.99
              //apop_opts.output_type='p';apop_opts.output_pipe=gp;apop_matrix_print(data, NULL);
              

              //apophenia 0.999
              apop_matrix_print(data, .output_type='p', .output_pipe=gp);

              fflush(gp);
              pclose(gp);
            }
        else
            {
              g_print("No Data Returned From Database\n");
            }
      }

     if(terminal!=NULL) g_free(terminal);
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
         //printf("Table of Sorted Means\n");
         sqlite3_prepare_v2(handle,sql2,-1,&stmt2,0);
            for(i=0;i<iRecordCount;i++)
               {
                 sqlite3_step(stmt2);
                 iPlates[i]=sqlite3_column_int(stmt2, 0);
                 iGroup[i]=sqlite3_column_int(stmt2, 1);
                 iSetSize[i]=sqlite3_column_int(stmt2, 2);
                 dMean[i]=sqlite3_column_double(stmt2, 3);
                 //printf(" %i %i %i %i %f\n", i, iPlates[i], iGroup[i], iSetSize[i], dMean[i]);
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
                     //printf("%i ", CovarianceArray[ii]);
                     iCovSize++;
                   }
               //printf("\n");

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
void GenerateRandomValues(double dDataArray[], int iNumberOfPlates,int iPlateSize,int iSetSizeForStatistics, const gchar *pPlatePosControlText, const gchar *pPlateNegControlText, int seed_value, double param, double pFuncDistribution())
     {
      //Generate random numbers in blocks or based on positional arguments.
      double dRndNumber=0;
      double shift=0;
      double shift_pos=0;
      double shift_neg=0;
      int iGroupCounter=0;
      const gsl_rng_type *T;
      gsl_rng *r;
      gsl_rng_env_setup();
      T = gsl_rng_mt19937;
      r = gsl_rng_alloc(T);
      gsl_rng_set(r,seed_value);

      int PosControlLength=strlen(pPlatePosControlText);
      int NegControlLength=strlen(pPlateNegControlText);
      GString *buffer=g_string_new("");
      GArray *PosArray=g_array_new(FALSE, FALSE, sizeof (int));
      GArray *NegArray=g_array_new(FALSE, FALSE, sizeof (int));
      guint32 iPosControlSize=0;
      guint32 iNegControlSize=0;
      guint32 itemp=0;
      guint32 i=0;
      guint32 j=0;
      guint32 k=0;
      guint32 m=0;

     g_print("Start Random Number Generator\n");
     printf("Mersenne Twister Generator %s\n", gsl_rng_name(r));
     printf("Seed Value %i\n", seed_value);
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
                       if(iGroupCounter==iSetSizeForStatistics)
                         {
                           shift=gsl_rng_uniform(r) * 1000;
                           iGroupCounter=0;
                         }
                       if(j==0)
                         {
                           do
		              {
		                shift_pos=gsl_rng_uniform(r)*1000;
		              }
		           while (shift_pos<800);
                           do
		              {
		                shift_neg=gsl_rng_uniform(r)*1000;
		              }
		           while (shift_neg>200);
                         }

                       if(i*iPlateSize+j==i*iPlateSize+g_array_index(PosArray,int,k)&&k<iPosControlSize)
                          {
                            dRndNumber=shift_pos+pFuncDistribution(r,param);
                            dDataArray[i*iPlateSize+j] = dRndNumber;
                            iGroupCounter++;  
                            if(k<iPosControlSize)
                               {
                                 k++;
                               }
                          }
                       else if(i*iPlateSize+j==i*iPlateSize+g_array_index(NegArray,int,m)&&m<iNegControlSize)
                          {
                            dRndNumber=shift_neg+pFuncDistribution(r,param);
                            dDataArray[i*iPlateSize+j] = dRndNumber;
                            iGroupCounter++;
                            if(m<iNegControlSize)
                              {
                                m++;
                              }
                          }
                       else
                          {
                            dRndNumber=shift+pFuncDistribution(r,param);
                            dDataArray[i*iPlateSize+j] = dRndNumber;   
                            iGroupCounter++;
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
              do
		{
		  shift_pos=gsl_rng_uniform(r)*1000;
		}
	      while (shift_pos<800);
              do
		{
		  shift_neg=gsl_rng_uniform(r)*1000;
	        }
	      while (shift_neg>200);
               for(j=(i*iPlateSize); j<(i*iPlateSize+iSetSizeForStatistics); j++)
	          {
                    dRndNumber=shift_pos+pFuncDistribution(r,param);
                    dDataArray[j] = dRndNumber;
	          }

               for(j=(i*iPlateSize+iSetSizeForStatistics); j<(i*iPlateSize+2*iSetSizeForStatistics); j++)
	            {
                      dRndNumber=shift_neg+pFuncDistribution(r,param);
                      dDataArray[j] = dRndNumber;
	            }

               for(j=(i*iPlateSize+2*iSetSizeForStatistics); j<(i*iPlateSize+iPlateSize); j++)
	            {
                      if(iGroupCounter==iSetSizeForStatistics)
                         {
                           shift=gsl_rng_uniform(r) * 1000;
                           iGroupCounter=0;
                         }
                      dRndNumber=shift+pFuncDistribution(r,param);
                      dDataArray[j] = dRndNumber;
                      iGroupCounter++;
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
void PlateMapDoubleTruncate(double dDataArray[], int iNumberOfPlates, int iPlateSize, int iRows, int iColumns, int digits)
     {
         //Put a column of numbers into row, column format. (double) version.
         guint32 check=0;
         guint32 i=0;
         guint32 j=0;
         guint32 k=0;
         GString *buffer=g_string_new(NULL);
         GString *temp1=g_string_new(NULL);
         GtkClipboard *clipboard=gtk_clipboard_get(GDK_SELECTION_CLIPBOARD);
          
         g_print("Create Plate Map(truncate)\n");  
         for(i=0; i<iNumberOfPlates; i++)
           {
              for(j=0; j<iRows; j++)
                 {
                   for(k=0; k<iColumns; k++)
                        {  
                          
                          g_string_printf(temp1,"%.10f", dDataArray[check]);
                          g_string_truncate(temp1, digits);
                          g_string_append_printf(buffer,"%s ", temp1->str);
                          g_string_truncate(temp1, 0); 
                          check++;
                        }
                     g_string_append_printf(buffer,"\n");  
                 }
                g_string_append_printf(buffer,"\n");
           }
             
           gtk_clipboard_set_text(clipboard, buffer->str, strlen(buffer->str));
           g_string_free(buffer, TRUE);
           g_string_free(temp1, TRUE);
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
               printf("Send Text to Database\n");
               retval=apop_text_to_db("stats_with_pipes.txt", string2,0,1,NULL);
               printf("\n%i Rows Sent to Database\n", retval);
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
     printf("Data Export Finished\n");
  }
void format_text_platemap(double dEntry1, double dEntry2, double dEntry3, double dEntry4, GtkTextView *textview)
  {
    guint32 iBufferCount;
    gunichar Char='~';
    gunichar Char2='~';
    int iSwitch=0;
    int iSwitch2=0;
    guint32 i=0;
    guint32 j=0;
    double dTemp;
    GtkTextBuffer *buffer;
    GtkTextIter start1, end1, start_iter, start_number, end_number;
    GtkTextTagTable *TagTable;
    GtkTextTag *tagtest;

    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    TagTable=gtk_text_buffer_get_tag_table(buffer);
    tagtest=gtk_text_tag_table_lookup(TagTable, "blue_foreground");
           if(tagtest==NULL)
               {
                 gtk_text_buffer_create_tag (buffer, "blue_foreground",
                        "foreground", "#0000ff", NULL); 
                 gtk_text_buffer_create_tag (buffer, "red_foreground",
                        "foreground", "#ff0000", NULL); 
                 gtk_text_buffer_create_tag (buffer, "green_foreground",
                        "foreground", "#00ff00", NULL);     
               }
           else
               {
                 //remove tags.
                 gtk_text_buffer_get_bounds(buffer, &start1, &end1);
                 gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);
               }

           iBufferCount=gtk_text_buffer_get_char_count(buffer); 
           gtk_text_buffer_get_start_iter(buffer, &start_iter);
           for(i=0;i<iBufferCount;i++)
              {
                 Char=gtk_text_iter_get_char(&start_iter);
                       if(Char=='\n' && Char2=='\n')
                         {
                           //A jump of " \n\n" or 3 characters between plates after a " \n".
                           iSwitch=1;
                           iSwitch2=0;
                         }
                       if(Char==' '&& Char2=='\n')
                         {
                            //A jump of " \n" or 2 characters between rows.
                            iSwitch2=1;
                         }

                       if(Char==' ')
                         {
                             if(iSwitch==1)
                                {
                                  gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-2));
                                  gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i);
                                  iSwitch=0;
                                }
                             else if(iSwitch2==1)
                                {
                                  gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-1));
                                  gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i);
                                  iSwitch2=0;
                                }
                             else
                                {
                                  gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-j);
                                  gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i);
                                }

                            //g_print("z%sz\n", gtk_text_iter_get_text(&start_number, &end_number));
                            dTemp=atof(gtk_text_iter_get_text(&start_number, &end_number));

                              if(dTemp>dEntry1)
                                {
                                   gtk_text_buffer_apply_tag_by_name (buffer, "blue_foreground", &start_number, &end_number);
                                }
                              else if(dTemp<dEntry2)
                                {
                                   gtk_text_buffer_apply_tag_by_name(buffer, "red_foreground", &start_number, &end_number);
                                }
                              else if(dTemp>dEntry3 && dTemp<dEntry4)
                                {
                                   gtk_text_buffer_apply_tag_by_name(buffer, "green_foreground", &start_number, &end_number);
                                }
                              else
                                {
                                 //exit
                                }
                                j=0;
                             }
                          else
                             {
                               j++;
                             }
                      Char2=Char;
                      gtk_text_iter_forward_chars(&start_iter, 1);
                 }

  }
void format_text_platemap_heatmap_high_low(GtkTextView *textview, double *high, double *low)
  {
    guint32 iBufferCount;
    gunichar Char='~';
    gunichar Char2='~';
    int iSwitch=0;
    int iSwitch2=0;
    guint32 i=0;
    guint32 j=0;
    double dTemp=0;
    GtkTextBuffer *buffer;
    GtkTextIter start_iter, start_number, end_number;
    buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

    iBufferCount=gtk_text_buffer_get_char_count(buffer); 
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    for(i=0;i<iBufferCount;i++)
       {
         Char=gtk_text_iter_get_char(&start_iter);
            if(Char=='\n' && Char2=='\n')
              {
                //A jump of " \n\n" or 3 characters between plates after a " \n".
                iSwitch=1;
                iSwitch2=0;
              }
            if(Char==' '&& Char2=='\n')
              {
                //A jump of " \n" or 2 characters between rows.
                iSwitch2=1;
              }

            if(Char==' ')
              {
                if(iSwitch==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-2));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i);
                    iSwitch=0;
                  }
                else if(iSwitch2==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-1));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i);
                    iSwitch2=0;
                  }
                else
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-j);
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i);
                  }

                 //g_print("z%sz\n", gtk_text_iter_get_text(&start_number, &end_number));
                 dTemp=atof(gtk_text_iter_get_text(&start_number, &end_number));
                 if(dTemp>*high)
                   {
                     *high=dTemp;
                   }
                 if(dTemp<*low)
                   {
                     *low=dTemp;
                   }
                j=0;
              }  
            else
              {
                j++;
              }
           Char2=Char;
           gtk_text_iter_forward_chars(&start_iter, 1);
        }
    //printf("high %f low %f\n", *high, *low);
  }
void format_text_platemap_heatmap(GtkTextView *textview, int high, int low)
  {
    int hexup=0x00;
    int hexdown=0xFF;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    char hex_blue_to_red[65][8];
    char *tag_names[]={"1","2","3","4","5","6","7","8","9","10","11","12","13","14","15","16","17","18","19", "20","21","22","23","24","25","26","27","28","29","30","31","32","33","34","35","36","37","38","39", "40","41","42","43","44","45","46","47","48","49","50","51","52","53","54","55","56","57","58","59","60", "61","62","63","64","65"};
    int tag=0;
    double range=high-low;
    guint32 iBufferCount;
    gunichar Char='~';
    gunichar Char2='~';
    int iSwitch=0;
    int iSwitch2=0;
    guint32 i=0;
    guint32 j=0;
    double dTemp=0;
    GtkTextIter start1, end1, start_iter, start_number, end_number;
    GtkTextTagTable *TagTable;
    GtkTextTag *tagtest=NULL;

    //remove tags.
    gtk_text_buffer_get_bounds(buffer, &start1, &end1);
    gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);

    TagTable=gtk_text_buffer_get_tag_table(buffer);
    tagtest=gtk_text_tag_table_lookup(TagTable, "1");
    if(tagtest==NULL)
      {
        //256/8=32. scale 64 times from red to blue
        //scale blue to green
        for(i=0;i<16;i++)
           {
             snprintf(hex_blue_to_red[i], 8, "#00%02XFF", hexup);
             //printf("hex number:%i %s \n", i+1, hex_blue_to_red[i]);
             gtk_text_buffer_create_tag(buffer,tag_names[i],"background", hex_blue_to_red[i], "foreground", "#000000", NULL);
             hexup=hexup+0x0F;
           }
   
        hexup=0x00;
        hexdown=0xFF;
        for(i=16;i<32;i++)
           {
             snprintf(hex_blue_to_red[i], 8, "#00FF%02X", hexdown);
             //printf("hex number:%i %s \n", i+1, hex_blue_to_red[i]);
             gtk_text_buffer_create_tag(buffer,tag_names[i],"background", hex_blue_to_red[i], "foreground", "#000000", NULL);
             hexdown=hexdown-0x0F;
           }
        
        hexup=0x00;
        hexdown=0xFF;
        for(i=32;i<48;i++)
           {
             snprintf(hex_blue_to_red[i], 8, "#%02XFF00", hexup);
             //printf("hex number:%i %s \n", i+1, hex_blue_to_red[i]);
             gtk_text_buffer_create_tag(buffer,tag_names[i],"background", hex_blue_to_red[i], "foreground", "#000000", NULL);
             hexup=hexup+0x0F;
           }

        //scale green to red
        hexup=0x00;
        hexdown=0xFF;
        for(i=48;i<64;i++)
           {
             snprintf(hex_blue_to_red[i], 8, "#FF%02X00", hexdown);
             //printf("hex number:%i %s \n", i+1, hex_blue_to_red[i]);
             gtk_text_buffer_create_tag(buffer,tag_names[i],"background", hex_blue_to_red[i], "foreground", "#000000", NULL);
             hexdown=hexdown-0x0F;
           } 
        
         gtk_text_buffer_create_tag(buffer, tag_names[64], "background", "#FF0000", "foreground", "#000000", NULL);
       
      }
    
    //Heatmap numbers
    iBufferCount=gtk_text_buffer_get_char_count(buffer); 
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    for(i=0;i<iBufferCount;i++)
       {
         Char=gtk_text_iter_get_char(&start_iter);
            if(Char=='\n' && Char2=='\n')
              {
                //A jump of " \n\n" or 3 characters between plates after a " \n".
                iSwitch=1;
                iSwitch2=0;
              }
            if(Char==' '&& Char2=='\n')
              {
                //A jump of " \n" or 2 characters between rows.
                iSwitch2=1;
              }

            if(Char==' ')
              {
                if(iSwitch==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-2));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch=0;
                  }
                else if(iSwitch2==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-1));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch2=0;
                  }
                else
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-j);
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                  }

                 //g_print("z%sz\n", gtk_text_iter_get_text(&start_number, &end_number));
                 dTemp=atof(gtk_text_iter_get_text(&start_number, &end_number));
                 tag=(int)rint(((dTemp-low)/range)*65.0);
                 //printf("tag %i\n", tag);
                 if(tag<0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[0], &start_number, &end_number);  
                   }    
                 if(tag>64)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[64], &start_number, &end_number); 
                   } 
                 if(tag<=64&&tag>=0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[tag], &start_number, &end_number); 
                   }          

                j=0;
              }  
            else
              {
                j++;
              }
           Char2=Char;
           gtk_text_iter_forward_chars(&start_iter, 1);
        }

  }
void format_text_platemap_heatmap_iris(GtkTextView *textview, int high, int low)
  {
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    char *tag_names[]={"66", "67", "68", "69", "70", "71", "72", "73", "74", "75", "76", "77", "78", "79", "80", "81", "82", "83", "84" , "85", "86" , "87" , "88" , "89", "90" , "91" , "92" , "93" , "94" , "95" , "96", "97", "98", "99", "100", "101", "102", "103", "104",  "105", "106", "107" , "108", "109", "110", "111", "112", "113", "114", "115", "116", "117", "118", "119", "120", "121", "122", "123", "124", "125",  "126", "127", "128", "129", "130"};
    char *iris[]={"#d507e5", "#d60fde", "#d711dc", "#d716d7", "#d819d5", "#d91ed1", "#d920ce", "#da25ca", "#da28c7", "#db2dc3", "#dc2fc1", "#dd34bc", "#dd37ba", "#de3cb5", "#de3eb3", "#df43ae", "#df46ac", "#e04ba8", "#e14da5", "#e252a1", "#e2559e", "#e35c98", "#e46193", "#e46491", "#e5698c", "#e77085", "#e77383", "#e8787f", "#e97f78", "#ea8275", "#ea8771", "#eb896f", "#ec8e6a", "#ec9168", "#ed9663", "#ed9861", "#ee9d5c", "#efa05a", "#efa556", "#f0a753", "#f1ac4f", "#f1af4c", "#f2b448", "#f2b646", "#f3bb41", "#f4be3f", "#f4c33a", "#f5c538", "#f6ca33", "#f6cd31", "#f7d22d", "#f7d42a", "#f8d926", "#f9dc23", "#f9e11f", "#fae31d", "#fbe818", "#fbeb16", "#fcf011", "#fcf20f", "#fdf50d", "#fdf70a", "#fefa08", "#fefc06", "#fefe04"};
    int tag=0;
    double range=high-low;
    guint32 iBufferCount;
    gunichar Char='~';
    gunichar Char2='~';
    int iSwitch=0;
    int iSwitch2=0;
    guint32 i=0;
    guint32 j=0;
    double dTemp=0;
    GtkTextIter start1, end1, start_iter, start_number, end_number;
    GtkTextTagTable *TagTable;
    GtkTextTag *tagtest=NULL;

    //remove tags.
    gtk_text_buffer_get_bounds(buffer, &start1, &end1);
    gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);

    TagTable=gtk_text_buffer_get_tag_table(buffer);
    tagtest=gtk_text_tag_table_lookup(TagTable, "66");
    if(tagtest==NULL)
      {
        for(i=0;i<65;i++) 
           {
         gtk_text_buffer_create_tag(buffer, tag_names[i], "background", iris[i], "foreground", "#000000", NULL);
           }
      }
    
    //Heatmap numbers
    iBufferCount=gtk_text_buffer_get_char_count(buffer); 
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    for(i=0;i<iBufferCount;i++)
       {
         Char=gtk_text_iter_get_char(&start_iter);
            if(Char=='\n' && Char2=='\n')
              {
                //A jump of " \n\n" or 3 characters between plates after a " \n".
                iSwitch=1;
                iSwitch2=0;
              }
            if(Char==' '&& Char2=='\n')
              {
                //A jump of " \n" or 2 characters between rows.
                iSwitch2=1;
              }

            if(Char==' ')
              {
                if(iSwitch==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-2));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch=0;
                  }
                else if(iSwitch2==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-1));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch2=0;
                  }
                else
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-j);
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                  }

                 //g_print("z%sz\n", gtk_text_iter_get_text(&start_number, &end_number));
                 dTemp=atof(gtk_text_iter_get_text(&start_number, &end_number));
                 tag=(int)rint(((dTemp-low)/range)*65.0);
                 //printf("tag %i\n", tag);
                 if(tag<0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[0], &start_number, &end_number);  
                   }    
                 if(tag>64)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[64], &start_number, &end_number); 
                   } 
                 if(tag<=64&&tag>=0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[tag], &start_number, &end_number); 
                   }          

                j=0;
              }  
            else
              {
                j++;
              }
           Char2=Char;
           gtk_text_iter_forward_chars(&start_iter, 1);
        }

  }
void format_text_platemap_heatmap_iris2(GtkTextView *textview, int high, int low)
  {
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    char *tag_names[]={"131", "132", "133", "134", "135", "136", "137", "138", "139", "140", "141", "142", "143", "144", "145", "146", "147", "148", "149" , "150", "151" , "152" , "153" , "154", "155" , "156" , "157" , "158" , "159" , "160" , "161", "162", "163", "164", "165", "166", "167", "168", "169",  "170", "171", "172" , "173", "174", "175", "176", "177", "178", "179", "180", "181", "182", "183", "184", "185", "186", "187", "188", "189", "190",  "191", "192", "193", "194", "195"};
    char *iris[]={"#0202fd", "#0707f8", "#0a0af5", "#0f0ff0", "#1111ee", "#1717e8", "#1919e6", "#2121de", "#2323dc", "#2828d7", "#2b2bd4", "#3030cf", "#3838c7", "#3a3ac5", "#4040bf", "#4242bd", "#4545ba", "#4a4ab5", "#4c4cb3", "#5151ae", "#5454ab", "#5c5ca3", "#5e5ea1", "#666699", "#686897", "#6e6e91", "#75758a", "#787887", "#7d7d82", "#808080", "#82827d", "#878778", "#8a8a75", "#8f8f70", "#91916e", "#94946b", "#999966", "#9c9c63", "#a1a15e", "#a3a35c", "#a8a857", "#abab54", "#b0b04f", "#b3b34c", "#b5b54a", "#baba45", "#bdbd42", "#c2c23d", "#c5c53a", "#caca35", "#cccc33", "#cfcf30", "#d4d42b", "#d7d728", "#dcdc23", "#dede21", "#e1e11e", "#e6e619", "#e8e817", "#ebeb14", "#f0f00f", "#f3f30c", "#f8f807", "#fafa05", "#ffff00"};
    int tag=0;
    double range=high-low;
    guint32 iBufferCount;
    gunichar Char='~';
    gunichar Char2='~';
    int iSwitch=0;
    int iSwitch2=0;
    guint32 i=0;
    guint32 j=0;
    double dTemp=0;
    GtkTextIter start1, end1, start_iter, start_number, end_number;
    GtkTextTagTable *TagTable;
    GtkTextTag *tagtest=NULL;

    //remove tags.
    gtk_text_buffer_get_bounds(buffer, &start1, &end1);
    gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);

    TagTable=gtk_text_buffer_get_tag_table(buffer);
    tagtest=gtk_text_tag_table_lookup(TagTable, "131");
    if(tagtest==NULL)
      {
        for(i=0;i<65;i++) 
           {
         gtk_text_buffer_create_tag(buffer, tag_names[i], "background", iris[i], "foreground", "#000000", NULL);
           }
      }
    
    //Heatmap numbers
    iBufferCount=gtk_text_buffer_get_char_count(buffer); 
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    for(i=0;i<iBufferCount;i++)
       {
         Char=gtk_text_iter_get_char(&start_iter);
            if(Char=='\n' && Char2=='\n')
              {
                //A jump of " \n\n" or 3 characters between plates after a " \n".
                iSwitch=1;
                iSwitch2=0;
              }
            if(Char==' '&& Char2=='\n')
              {
                //A jump of " \n" or 2 characters between rows.
                iSwitch2=1;
              }

            if(Char==' ')
              {
                if(iSwitch==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-2));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch=0;
                  }
                else if(iSwitch2==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-1));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch2=0;
                  }
                else
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-j);
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                  }

                 //g_print("z%sz\n", gtk_text_iter_get_text(&start_number, &end_number));
                 dTemp=atof(gtk_text_iter_get_text(&start_number, &end_number));
                 tag=(int)rint(((dTemp-low)/range)*65.0);
                 //printf("tag %i\n", tag);
                 if(tag<0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[0], &start_number, &end_number);  
                   }    
                 if(tag>64)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[64], &start_number, &end_number); 
                   } 
                 if(tag<=64&&tag>=0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[tag], &start_number, &end_number); 
                   }          

                j=0;
              }  
            else
              {
                j++;
              }
           Char2=Char;
           gtk_text_iter_forward_chars(&start_iter, 1);
        }

  }
void format_text_platemap_heatmap_sun(GtkTextView *textview, int high, int low)
  {
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    char *tag_names[]={"196", "197", "198", "199", "200", "201", "202", "203", "204", "205", "206", "207", "208", "209", "210", "211", "212", "213", "214" , "215", "216" , "217" , "218" , "219", "220" , "221" , "222" , "223" , "224" , "225" , "226", "227", "228", "229", "230", "231", "232", "233", "234",  "235", "236", "237" , "238", "239", "240", "241", "242", "243", "244", "245", "246", "247", "248", "249", "250", "251", "252", "253", "254", "255",  "256", "257", "258", "259", "260"};
     char *sun[]={"#fe1005", "#fe1505", "#fe1705", "#fe1b05", "#fe1d05", "#fe2205", "#fe2405", "#fe2905", "#fe2b05", "#fe2f05", "#fe3205", "#fe3605", "#fe3805", "#fe3b05", "#fe4105", "#fe4405", "#fe4805", "#fe4f05", "#fe5105", "#fe5505", "#fe5805", "#fe5e05", "#fe6105", "#fe6505", "#fe6705", "#fe6e05", "#fe7305", "#fe7505", "#fe7c05", "#fe8004", "#fe8204", "#fe8404", "#fe8904", "#fe8b04", "#fe9004", "#fe9204", "#fe9604", "#fe9904", "#fe9d04", "#fe9f04", "#fea204", "#fea604", "#fea804", "#fead04", "#feaf04", "#feb404", "#feb604", "#feba04", "#febc04", "#fec104", "#fec304", "#fec504", "#feca04", "#fecc04", "#fed104", "#fed304", "#fed704", "#feda04", "#fedc04", "#fee004", "#fee304", "#fee504", "#fee904", "#feec04", "#feee04"};
    int tag=0;
    double range=high-low;
    guint32 iBufferCount;
    gunichar Char='~';
    gunichar Char2='~';
    int iSwitch=0;
    int iSwitch2=0;
    guint32 i=0;
    guint32 j=0;
    double dTemp=0;
    GtkTextIter start1, end1, start_iter, start_number, end_number;
    GtkTextTagTable *TagTable;
    GtkTextTag *tagtest=NULL;

    //remove tags.
    gtk_text_buffer_get_bounds(buffer, &start1, &end1);
    gtk_text_buffer_remove_all_tags(buffer, &start1, &end1);

    TagTable=gtk_text_buffer_get_tag_table(buffer);
    tagtest=gtk_text_tag_table_lookup(TagTable, "196");
    if(tagtest==NULL)
      {
        for(i=0;i<65;i++) 
           {
         gtk_text_buffer_create_tag(buffer, tag_names[i], "background", sun[i], "foreground", "#000000", NULL);
           }
      }
    
    //Heatmap numbers
    iBufferCount=gtk_text_buffer_get_char_count(buffer); 
    gtk_text_buffer_get_start_iter(buffer, &start_iter);
    for(i=0;i<iBufferCount;i++)
       {
         Char=gtk_text_iter_get_char(&start_iter);
            if(Char=='\n' && Char2=='\n')
              {
                //A jump of " \n\n" or 3 characters between plates after a " \n".
                iSwitch=1;
                iSwitch2=0;
              }
            if(Char==' '&& Char2=='\n')
              {
                //A jump of " \n" or 2 characters between rows.
                iSwitch2=1;
              }

            if(Char==' ')
              {
                if(iSwitch==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-2));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch=0;
                  }
                else if(iSwitch2==1)
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-(j-1));
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                    iSwitch2=0;
                  }
                else
                  {
                    gtk_text_buffer_get_iter_at_offset (buffer, &start_number, i-j);
                    gtk_text_buffer_get_iter_at_offset (buffer, &end_number, i+1);
                  }

                 //g_print("z%sz\n", gtk_text_iter_get_text(&start_number, &end_number));
                 dTemp=atof(gtk_text_iter_get_text(&start_number, &end_number));
                 tag=(int)rint(((dTemp-low)/range)*65.0);
                 //printf("tag %i\n", tag);
                 if(tag<0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[0], &start_number, &end_number);  
                   }    
                 if(tag>64)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[64], &start_number, &end_number); 
                   } 
                 if(tag<=64&&tag>=0)
                   {
                     gtk_text_buffer_apply_tag_by_name(buffer, tag_names[tag], &start_number, &end_number); 
                   }          

                j=0;
              }  
            else
              {
                j++;
              }
           Char2=Char;
           gtk_text_iter_forward_chars(&start_iter, 1);
        }

  }
void make_heatmap_html(double test_data[], int rows, int columns, int plate_size, int plates, int precision, int font_size, char **gradient_array)
  {
    int i=0;
    int j=0;
    int k=0;

    //High and low for each plate.
    double high[plates];
    double low[plates];
    double temp_high;
    double temp_low;
    for(i=0;i<plates;i++)
       {
         temp_low=DBL_MAX;
         temp_high=DBL_MIN;
         for(j=0;j<plate_size;j++)
            {
              if(temp_high<test_data[i*plate_size+j])
                {
                  temp_high=test_data[i*plate_size+j];
                }
              if(temp_low>test_data[i*plate_size+j])
                {
                  temp_low=test_data[i*plate_size+j];
                }
              
            }
          //printf("high %f low %f\n", temp_high, temp_low);
          high[i]=temp_high;
          low[i]=temp_low;
       }

    //Write HTML to file.
    FILE *f = fopen("heatmap.html", "w");
      if(f == NULL)
        {
          printf("Error opening file!\n");
        }

    fprintf(f, "<!DOCTYPE html>\n");
    fprintf(f, "<html lang=\"en\">\n");
    fprintf(f, "<head>\n");
    fprintf(f, "<meta charset=\"UTF-8\"/>\n");
    fprintf(f, "<title>Heatmap</title>\n");
    fprintf(f, "</head>\n");
    fprintf(f, "<body style=\"font-size:%ipx;\">\n", font_size);
    fprintf(f, "<h1 align=\"center\">Heatmap %i Well Plates</h1>\n", plate_size);

    //Use a counter for simplicity.
    int counter = 0;
    double diff = 0;
    double temp1 = 0;
    double temp2 = 0;
    double temp3 = 0;
    for(i=0;i<plates;i++)
       {
        //Write the score table 
        fprintf(f, "<table align=\"center\" style=\"border-collapse:collapse;\"><caption>Score %i</caption><tr>\n", i+1);
        for(j=0;j<8;j++)
           {
             diff = high[i] - low[i];
             temp1 = (low[i] + j*(diff/7));
             temp2 = (((temp1-low[i])/(high[i]-low[i])) *64);
             //printf("high %f low %f temp1 %f temp2 %f\n", high[i], low[i], temp1, temp2);
             if(temp2>64)temp2=64;
             if(temp2<0)temp2=0;
             fprintf(f, "<td bgcolor=\"%s\">%.*f</td>\n", gradient_array[(int)temp2], precision, temp1);
           }
        fprintf(f, "</tr></table>");

        //The heatmap table
        fprintf(f, "<table align=\"center\" bgcolor=\"silver\"  style=\"margin-bottom:40pt\"><caption>Plate %i</caption><thead><tr>\n", i+1);
        fprintf(f, "<th scope=\"rowgroup\" bgcolor=\"silver\"></th>\n");
        for(j=0; j<columns-1; j++)
           {
             fprintf(f, "<th scope=\"col\" bgcolor=\"silver\">%i</th>\n", j+1);
           }
        fprintf(f, "</tr></thead><tbody>\n");  

        //Write the data and color gradient into the table.
        for(j=0; j<rows; j++)
           {
             fprintf(f, "<tr>\n");
             for(k=0;k<columns;k++)
                {
                  if(k==0)
                    {
                     //This is a label column. Doesn't count for the test_data counter.
                     fprintf(f, "<th scope=\"row\" bgcolor=\"silver\">%i</th>\n", j+1);
                    }
                  else
                    {
                      temp3 = (((test_data[counter]-low[i])/(high[i] - low[i])) *64);
                      if(temp3>64)temp3=64;
                      if(temp3<0)temp3=0;
                      fprintf(f, "<td bgcolor=\"%s\">%.*f</td>\n", gradient_array[(int)temp3], precision, test_data[counter]);
                      counter+=1;
                    }
                  }
               fprintf(f, "</tr>\n");
             }
        fprintf(f, "</tbody></table>\n");
       }

    fprintf(f, "</body>\n");
    fprintf(f, "</html>\n");

    fclose(f); 
    printf("heatmap.html file created.\n");
    simple_message_dialog("heatmap.html file created.");
  }
void heatmap_to_html_gradient(double test_data[], int rows, int columns, int plate_size, int plates, int precision, int font_size, int gradient)
  {
    if(gradient==0)
      {
        char *gradient_rgb[] = {"#0000FF", "#000FFF", "#001EFF", "#002DFF", "#003CFF", "#004BFF", "#005AFF", "#0069FF", "#0078FF", "#0087FF", "#0096FF", "#00A5FF", "#00B4FF", "#00C3FF", "#00D2FF", "#00E1FF", "#00F0FF", "#00FFF0", "#00FFE1", "#00FFD2", "#00FFC3", "#00FFB4", "#00FFA5", "#00FF96", "#00FF87", "#00FF78", "#00FF69", "#00FF5A", "#00FF4B", "#00FF3C", "#00FF2D", "#00FF1E", "#00FF0F", "#0FFF00", "#1EFF00", "#2DFF00", "#3CFF00", "#4BFF00", "#5AFF00", "#69FF00", "#78FF00", "#87FF00", "#96FF00", "#A5FF00", "#B4FF00", "#C3FF00", "#D2FF00", "#E1FF00", "#F0FF00", "#FFF000", "#FFE100", "#FFD200", "#FFC300", "#FFB400", "#FFA500", "#FF9600", "#FF8700", "#FF7800", "#FF6900", "#FF5A00", "#FF4B00", "#FF3C00", "#FF2D00", "#FF1E00", "#FF0F00"};
        make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size, gradient_rgb);
      }
    if(gradient==1)
      {
        char *gradient_iris[] = {"#d507e5", "#d60fde", "#d711dc", "#d716d7", "#d819d5", "#d91ed1", "#d920ce", "#da25ca", "#da28c7", "#db2dc3", "#dc2fc1", "#dd34bc", "#dd37ba", "#de3cb5", "#de3eb3", "#df43ae", "#df46ac", "#e04ba8", "#e14da5", "#e252a1", "#e2559e", "#e35c98", "#e46193", "#e46491", "#e5698c", "#e77085", "#e77383", "#e8787f", "#e97f78", "#ea8275", "#ea8771", "#eb896f", "#ec8e6a", "#ec9168", "#ed9663", "#ed9861", "#ee9d5c", "#efa05a", "#efa556", "#f0a753", "#f1ac4f", "#f1af4c", "#f2b448", "#f2b646", "#f3bb41", "#f4be3f", "#f4c33a", "#f5c538", "#f6ca33", "#f6cd31", "#f7d22d", "#f7d42a", "#f8d926", "#f9dc23", "#f9e11f", "#fae31d", "#fbe818", "#fbeb16", "#fcf011", "#fcf20f", "#fdf50d", "#fdf70a", "#fefa08", "#fefc06", "#fefe04"};
        make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size, gradient_iris);
      }
    if(gradient==2)
      {
        char *gradient_iris2[]={"#0202fd", "#0707f8", "#0a0af5", "#0f0ff0", "#1111ee", "#1717e8", "#1919e6", "#2121de", "#2323dc", "#2828d7", "#2b2bd4", "#3030cf", "#3838c7", "#3a3ac5", "#4040bf", "#4242bd", "#4545ba", "#4a4ab5", "#4c4cb3", "#5151ae", "#5454ab", "#5c5ca3", "#5e5ea1", "#666699", "#686897", "#6e6e91", "#75758a", "#787887", "#7d7d82", "#808080", "#82827d", "#878778", "#8a8a75", "#8f8f70", "#91916e", "#94946b", "#999966", "#9c9c63", "#a1a15e", "#a3a35c", "#a8a857", "#abab54", "#b0b04f", "#b3b34c", "#b5b54a", "#baba45", "#bdbd42", "#c2c23d", "#c5c53a", "#caca35", "#cccc33", "#cfcf30", "#d4d42b", "#d7d728", "#dcdc23", "#dede21", "#e1e11e", "#e6e619", "#e8e817", "#ebeb14", "#f0f00f", "#f3f30c", "#f8f807", "#fafa05", "#ffff00"};
        make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size, gradient_iris2);
      }
    if(gradient==3)
      {
        char *gradient_sun[]={"#fe1005", "#fe1505", "#fe1705", "#fe1b05", "#fe1d05", "#fe2205", "#fe2405", "#fe2905", "#fe2b05", "#fe2f05", "#fe3205", "#fe3605", "#fe3805", "#fe3b05", "#fe4105", "#fe4405", "#fe4805", "#fe4f05", "#fe5105", "#fe5505", "#fe5805", "#fe5e05", "#fe6105", "#fe6505", "#fe6705", "#fe6e05", "#fe7305", "#fe7505", "#fe7c05", "#fe8004", "#fe8204", "#fe8404", "#fe8904", "#fe8b04", "#fe9004", "#fe9204", "#fe9604", "#fe9904", "#fe9d04", "#fe9f04", "#fea204", "#fea604", "#fea804", "#fead04", "#feaf04", "#feb404", "#feb604", "#feba04", "#febc04", "#fec104", "#fec304", "#fec504", "#feca04", "#fecc04", "#fed104", "#fed304", "#fed704", "#feda04", "#fedc04", "#fee004", "#fee304", "#fee504", "#fee904", "#feec04", "#feee04"};
        make_heatmap_html(test_data, rows, columns, plate_size, plates, precision, font_size, gradient_sun);
      }
  }
void heatmap_to_html_sql(int iRadioButton, int rows, int columns, int precision, int font_size, int gradient)
  {
    int i=0;
    int iRecordCount=0;
    int plate_size=0;
    int plates=0;
    sqlite3 *handle;
    sqlite3_stmt *stmt1;
    sqlite3_stmt *stmt2;
    sqlite3_stmt *stmt3;
    sqlite3_stmt *stmt4;
    sqlite3_stmt *stmt5;
    char sql1[]="SELECT count(data) FROM data;";
    char sql2[]="SELECT max(wells) FROM aux;";
    char sql3[]="SELECT max(plate) FROM aux;";
    char sql4[]="SELECT Data FROM data ORDER BY KeyID;";
    char sql5[]="SELECT Percent FROM data ORDER BY KeyID;";
   
    sqlite3_open("VelociRaptorData.db",&handle);

    //Get record count.
    sqlite3_prepare_v2(handle,sql1,-1,&stmt1,0);
    sqlite3_step(stmt1);
    iRecordCount=sqlite3_column_int(stmt1, 0);
    sqlite3_finalize(stmt1);
    //Get plate_number
    sqlite3_prepare_v2(handle,sql2,-1,&stmt2,0);
    sqlite3_step(stmt2);
    plate_size=sqlite3_column_int(stmt2, 0);
    sqlite3_finalize(stmt2);
    //Get plates.
    sqlite3_prepare_v2(handle,sql3,-1,&stmt3,0);
    sqlite3_step(stmt3);
    plates=sqlite3_column_int(stmt3, 0);
    sqlite3_finalize(stmt3);

    //Do some error checking.
    printf("rows %i columns %i database_records %i plate_size %i plates %i\n", rows, columns, iRecordCount, plate_size, plates);

    if(iRecordCount==0)
      {
        printf("No records returned from database.\n");
        simple_message_dialog("No records returned from database.");
      }
    else if(plate_size==0)
      { 
        printf("Couldn't get plate size from aux table.\n");
        simple_message_dialog("Couldn't get plate size from aux table.");
      }
    else if(rows*(columns-1)!=plate_size)
      {
        printf("Rows times columns doesn't equal the plate size.\n");
        simple_message_dialog("Rows time columns doesn't equal the plate size.");
      }
    else
      {
        double *test_data = malloc((iRecordCount) * sizeof(double));
        if(test_data==NULL)
          {
            printf("Couldn't allocate memory for database records.\n");
          } 
        else
          {     
            if(iRadioButton==1)
              {
                sqlite3_prepare_v2(handle,sql4,-1,&stmt4,0);
                for(i=0;i<iRecordCount;i++)
                   {
                     sqlite3_step(stmt4);
                     test_data[i]=sqlite3_column_double(stmt4, 0);
                   }
                 sqlite3_finalize(stmt4); 
                 heatmap_to_html_gradient(test_data, rows, columns, plate_size, plates, precision, font_size, gradient);   
              }
            if(iRadioButton==2)
              {   
                sqlite3_prepare_v2(handle,sql5,-1,&stmt5,0);
                for(i=0;i<iRecordCount;i++)
                   {
                     sqlite3_step(stmt5);
                     test_data[i]=sqlite3_column_double(stmt5, 0);
                   }
                sqlite3_finalize(stmt5);
                heatmap_to_html_gradient(test_data, rows, columns, plate_size, plates, precision, font_size, gradient);       
              }
            }
         if(test_data!=NULL)free(test_data);
       }
     
     sqlite3_close(handle);         
     
  }







