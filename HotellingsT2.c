
/*
 Additional functions for the VelociRaptor application.
     
 Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
*/

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <gsl/gsl_rng.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_statistics.h>
#include <gsl/gsl_cdf.h>
#include <gsl/gsl_blas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_errno.h>
#include <sqlite3.h>
#include <apop.h>
#include <gsl/gsl_vector.h>
#include "VelociRaptorUI_Validation.h"

void hotellings_T2(int iRadioButton, double alpha, int ShowAll, int control, GtkTextView *textview, GtkWidget *progress, gsl_matrix *SuppliedContrasts, int MatrixColumns);
void build_contrast_matrix(int columns, int control, gsl_matrix *Contrasts);
void cov_calculate(gsl_matrix *Covariance, gsl_matrix *DataMatrix);
void tsquared_multiple_comparison(double alpha, int ShowAll, int plate, int rows, int columns, int control, double dData[], GtkTextView *textview, gsl_matrix *SuppliedContrasts);
void print_t2_confidence_intervals_control(gsl_matrix *CSC2,gsl_matrix *Cx,double alpha, double T2, double fdist, int rows, int columns, int plate, int control, GtkTextView *textview);
void print_t2_confidence_intervals_supplied(gsl_matrix *CSC2, gsl_matrix *Cx, gsl_matrix *SuppliedContrasts, double alpha, double T2, double fdist, int row1, int rows, int plate, GtkTextView *textview);
int build_contrast_matrix_from_array(char contrasts[], gsl_matrix *Contrasts, int rows, int columns, int numbers);
void z_factor(int iRadioButton, int iControl, GtkTextView *textview);
void build_temp_table_for_contingency(int plates,int rows,int columns);
void calculate_contingency_values(double alpha, GtkTextView *textview, int check_box1, int check_box2, int check_box3, int check_box4, int check_box5, int check_box6);



void hotellings_T2(int iRadioButton, double alpha, int ShowAll, int control, GtkTextView *textview, GtkWidget *progress, gsl_matrix *SuppliedContrasts, int MatrixColumns)
    {
     sqlite3 *handle;
     sqlite3_stmt *stmt1;
     sqlite3_stmt *stmt2;
     sqlite3_stmt *stmt3;
     sqlite3_stmt *stmt4;
     sqlite3_stmt *stmt5;
     int iRecordCount=0;
     int rows=0;
     int columns=0;
     int iPlateSize=0;
     int iNumberOfPlates=0;
     //int retval=0;
     int i=0;
     int j=0;
     int iCounter=0;
     int error=0;
     char *sql1;
     char *sql2;
     char *sql3;
     char *sql4;
     char *sql5;
     GtkTextBuffer *buffer1 = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
     
     //iRadioButton=1 data groups,2 percent groups, 3 data picks, 4 percent picks and for iRadioCritVal=3;
     //iRadioButton==1 data and groups
     if(iRadioButton==1)
       {
         //A record count of the data
         asprintf(&sql1, "SELECT count(*) FROM Data;");
         //Need a column count or group count
         asprintf(&sql2, "SELECT max(Groups) FROM aux;");
         //Need number of plates
         asprintf(&sql3, "SELECT max(Plate) FROM aux;");
         //Need plate size
         asprintf(&sql4, "SELECT max(Wells) FROM aux;");
         //Need data for data array
         asprintf(&sql5, "SELECT Data FROM Data;");
       }

     //iRadioButton==2 percent and groups
     if(iRadioButton==2)
       {
         //A record count of the data
         asprintf(&sql1, "SELECT count(*) FROM Data;");
         //Need a column count or group count
         asprintf(&sql2, "SELECT max(Groups) FROM aux;");
         //Need number of plates
         asprintf(&sql3, "SELECT max(Plate) FROM aux;");
         //Need plate size
         asprintf(&sql4, "SELECT max(Wells) FROM aux;");
         //Need data for data array
         asprintf(&sql5, "SELECT Percent FROM Data;");
       }

     //iRadioButton==3 data and picks
     if(iRadioButton==3)
       {
         //A record count of the data
         asprintf(&sql1, "SELECT count(*) FROM aux WHERE Picks!=0;");
         //Need a column count or group count
         asprintf(&sql2, "SELECT count(*) FROM (SELECT count(Picks) FROM aux WHERE Plate==1 AND Picks!=0 GROUP BY Picks);");
         //Need number of plates
         asprintf(&sql3, "SELECT max(Plate) FROM aux;");
         //Need plate size
         asprintf(&sql4, "SELECT count(*) FROM aux WHERE Plate==1 AND Picks!=0;");
         //Need data for data array
         asprintf(&sql5, "SELECT Data FROM Data AS D1, Aux AS D2 WHERE D1.KeyID==D2.KeyID AND Picks!=0;");
       }

     //iRadioButton==4 percent and picks
     if(iRadioButton==4)
       {
         //A record count of the data
         asprintf(&sql1, "SELECT count(*) FROM aux WHERE Picks!=0;");
         //Need a column count or group count
         asprintf(&sql2, "SELECT count(*) FROM (SELECT count(Picks) FROM aux WHERE Plate==1 AND Picks!=0 GROUP BY Picks);");
         //Need number of plates
         asprintf(&sql3, "SELECT max(Plate) FROM aux;");
         //Need plate size
         asprintf(&sql4, "SELECT count(*) FROM aux WHERE Plate==1 AND Picks!=0;");
         //Need data for data array
         asprintf(&sql5, "SELECT Percent FROM Data AS D1, Aux AS D2 WHERE D1.KeyID==D2.KeyID AND Picks!=0 ;");
       }

     printf("Connect to Database\n");
     sqlite3_open("VelociRaptorData.db",&handle);

         //Get record count
         sqlite3_prepare_v2(handle,sql1,-1,&stmt1,0);
         sqlite3_step(stmt1);
         iRecordCount=sqlite3_column_int(stmt1, 0);
         sqlite3_finalize(stmt1);

         //Get column count
         sqlite3_prepare_v2(handle,sql2,-1,&stmt2,0);
         sqlite3_step(stmt2);
         columns=sqlite3_column_int(stmt2, 0);
         sqlite3_finalize(stmt2);

         //Get number of plates
         sqlite3_prepare_v2(handle,sql3,-1,&stmt3,0);
         sqlite3_step(stmt3);
         iNumberOfPlates=sqlite3_column_int(stmt3, 0);
         sqlite3_finalize(stmt3);

         //Get plate size
         sqlite3_prepare_v2(handle,sql4,-1,&stmt4,0);
         sqlite3_step(stmt4);
         iPlateSize=sqlite3_column_int(stmt4, 0);
         sqlite3_finalize(stmt4);
         
         rows=iPlateSize/columns;

         printf("Record Count=%i\n", iRecordCount);
         printf("Plate Size=%i\n", iPlateSize);
         printf("Number of Plates=%i\n", iNumberOfPlates);
         printf("Rows=%i\n", rows);
         printf("Columns=%i\n", columns);

         //Check mod. If not 0 exit. Check all values are greater than 0.
         //rows-columns+1>=1
         if(iPlateSize%rows!=0 || rows-columns+1<1 || iRecordCount==0 || iPlateSize==0 || iNumberOfPlates==0 || rows==0 || columns==0)
           {
             printf("An error occurred! Check for 0 values.\n");
             simple_message_dialog("An error occurred! Check for 0 values\nin the terminal output.");
             error=1;
           }

         if(SuppliedContrasts!=NULL)
           {
             if(MatrixColumns!=columns)
               {
                 printf("The contrast matrix columns(%i) don't equal the covariance matrix rows(%i).\n", MatrixColumns, columns);
                 simple_message_dialog("The contrast matrix columns don't equal\nthe covariance matrix rows.");
                 error=1;
               }
           }

         //Reference p.133 result 3.3 of the Johnson and Wichern text.
         if(rows<=columns)
           {
             printf("If the sample size<=number of variables, then |S|=0.\n");
             simple_message_dialog("If the sample size <= number of variables,\nthen |S|=0.");
             error=1;
           }

         //Need balanced sets for Hotelling's T2 calculation.
         if(iPlateSize%(rows*columns)!=0)
           {
             printf("Check that the sets are all the same size. Hotelling's T2 calculation needs balanced sets.\n");
             simple_message_dialog("Check that the sets are all the same size. Hotelling's T2\ncalculation needs balanced sets.");
           }

         if(error==0)
           {
             //Allocate arrays on the heap.
             double *dData = malloc((iRecordCount) * sizeof(double));
             double *dPlateData = malloc((iPlateSize) * sizeof(double));

             //Load the data array
             sqlite3_prepare_v2(handle,sql5,-1,&stmt5,0);
                for(i=0;i<iRecordCount;i++)
                   {
                     sqlite3_step(stmt5);
                     dData[i]=sqlite3_column_double(stmt5, 0);
                   }
             sqlite3_finalize(stmt5);
      
             if(SuppliedContrasts==NULL)
               {
                 gtk_text_buffer_insert_at_cursor(buffer1, "Plate Control Test Alpha T2 F Lower Contrast Upper\n", -1);
               }
             else
               {
                 gtk_text_buffer_insert_at_cursor(buffer1, "Plate Row Alpha T2 F Lower Contrast Upper\n", -1);
               }
             
             //Calculate confidence intervals
             for(i=0;i<iNumberOfPlates;i++)
                {
                  for(j=0;j<iPlateSize;j++)
                     {
                       dPlateData[j]=dData[iCounter];
                       iCounter++;
                     }
                  //call funtion for each plate.
                  tsquared_multiple_comparison(alpha, ShowAll, i+1 , rows , columns, control, dPlateData, textview, SuppliedContrasts);
            
                 //update progress bar.
                 gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress), (double)i/(double)iNumberOfPlates);
                 gchar *message=g_strdup_printf("Calculating Plate %i out of %i", i+1,iNumberOfPlates);
                 gtk_progress_bar_set_text(GTK_PROGRESS_BAR(progress), message);
                 g_free(message);
                }
             free(dData);
             free(dPlateData);
           }

         sqlite3_close(handle);
         free(sql1);
         free(sql2);
         free(sql3);
         free(sql4);
         free(sql5);
    }
void tsquared_multiple_comparison(double alpha, int ShowAll, int plate, int rows, int columns, int control, double dData[], GtkTextView *textview, gsl_matrix *SuppliedContrasts)
    {
     gsl_matrix *DataMatrix = gsl_matrix_alloc (rows, columns);
     gsl_matrix *Mean=gsl_matrix_alloc(columns,1);
     gsl_matrix *Covariance = gsl_matrix_alloc (columns, columns);
     gsl_matrix *Contrasts;
     gsl_matrix *CS;
     gsl_matrix *CSC;
     gsl_matrix *CSC2;
     gsl_matrix *Cx;
     gsl_matrix *CSCinverse;
     gsl_matrix *Cholesky;
     gsl_matrix *T2_temp1;
     gsl_matrix *T2_temp2=gsl_matrix_alloc(1,1);
     //int signum=0;
     double T2=0;
     int i=0;
     int j=0;
     int iCounter=0;
     double x=0;
     double temp=0;
     int row1=0;
     int status1=0;
     int status2=0;

     if(SuppliedContrasts==NULL)
       {
         Contrasts=gsl_matrix_calloc(columns-1,columns);
         CS=gsl_matrix_alloc(columns-1,columns);
         CSC=gsl_matrix_alloc(columns-1,columns-1);
         CSC2=gsl_matrix_alloc(columns-1,columns-1);
         Cx=gsl_matrix_alloc(columns-1,1);
         CSCinverse=gsl_matrix_alloc(columns-1,columns-1);
         Cholesky=gsl_matrix_alloc(columns-1,columns-1);
         T2_temp1=gsl_matrix_alloc(1,columns-1);
       }
     else
       {
         row1=SuppliedContrasts->size1;
         CS=gsl_matrix_alloc(row1,columns);
         CSC=gsl_matrix_alloc(row1,row1);
         CSC2=gsl_matrix_alloc(row1,row1);
         Cx=gsl_matrix_alloc(row1,1);
         CSCinverse=gsl_matrix_alloc(row1,row1);
         Cholesky=gsl_matrix_alloc(row1,row1);
         T2_temp1=gsl_matrix_alloc(1,row1);
       }

     //Put data array into data matrix and load mean vector.
     for(i=0; i<columns; i++)
        {
          for(j=0; j<rows; j++)
             {
               x = x + dData[iCounter];
               gsl_matrix_set(DataMatrix, j, i, dData[iCounter]);
               iCounter++;
             }
          gsl_matrix_set(Mean, i, 0, x/(double)rows);
          x=0;
        }

       //Print the matrix to check the format.
       if(ShowAll==1)
         {
           printf("Plate %i Data and Calculations\n", plate);
           printf("data matrix\n");
           for(i=0; i<rows; i++) 
              {
                for(j=0; j<columns; j++)
                   {
                     printf ("%f ", gsl_matrix_get(DataMatrix, i, j));
                   }
                printf("\n");
              }
         }

       //Print mean vector.
       if(ShowAll==1)
         {        
           printf("mean vector\n");
           for(i=0; i<columns; i++)
              {
                printf("%f\n", gsl_matrix_get(Mean, i, 0));
              }
         }

       //Load the contrast matrix.
       if(SuppliedContrasts==NULL)
         {
           build_contrast_matrix(columns, control, Contrasts);
         }  
   
       if(ShowAll==1)
         {
           //Print the contrast matrix.
           if(SuppliedContrasts!=NULL)
             {
               printf("contrast matrix\n");
               for(i=0;i<columns-1;i++)
                  {
                    for(j=0;j<columns;j++)
                       {
                         temp=gsl_matrix_get(SuppliedContrasts, i, j);
                         printf("%f ", temp);
                       }
                    printf("\n");
                  }
             }
           else
             {
               printf("contrast matrix\n");
               for(i=0;i<columns-1;i++)
                  {
                    for(j=0;j<columns;j++)
                       {
                         temp=gsl_matrix_get(Contrasts, i, j);
                         printf("%f ", temp);
                       }
                    printf("\n");
                  }
             }
             
         }

       //Calculate the variance-covariance matrix.
       cov_calculate(Covariance,DataMatrix);
       if(ShowAll==1)
         {
           printf("variance-covariance matrix\n");
           for(i=0; i<columns; i++)
              {
                for(j=0;j<columns;j++)
                   {
                     printf ("%f ", gsl_matrix_get(Covariance, i, j));
                   }
                printf("\n");
              }
          }

       //Get CSC' and Cx.
       if(SuppliedContrasts==NULL)
         {
           gsl_blas_dgemm(CblasNoTrans, CblasNoTrans,1.0, Contrasts, Covariance, 0.0, CS);
         }
       else
         {
           gsl_blas_dgemm(CblasNoTrans, CblasNoTrans,1.0, SuppliedContrasts, Covariance, 0.0, CS);
         }
       //printf("CS multiplied\n");

       if(SuppliedContrasts==NULL)
         {
           gsl_blas_dgemm(CblasNoTrans, CblasTrans,1.0, CS, Contrasts, 0.0, CSC);
         }
       else
         {
           gsl_blas_dgemm(CblasNoTrans, CblasTrans,1.0, CS, SuppliedContrasts, 0.0, CSC);
         }
       //printf("CSC multiplied\n");

       if(SuppliedContrasts==NULL)
         {
           gsl_blas_dgemm(CblasNoTrans, CblasNoTrans,1.0, Contrasts, Mean, 0.0, Cx);
         }
       else
         {
           gsl_blas_dgemm(CblasNoTrans, CblasNoTrans,1.0, SuppliedContrasts, Mean, 0.0, Cx);
         }
       //printf("Cx multiplied\n");

       //Print CSC'.
       if(ShowAll==1)
         {
           printf("CSC' matrix\n");
           for(i=0; i<columns-1; i++)
              {
                for(j=0;j<columns-1;j++)
                   {
                     printf ("%f ", gsl_matrix_get(CSC, i, j));
                   }
                printf("\n");
              }
          }

       //save a copy of CSC to build confidence intervals with.
       gsl_matrix_memcpy(CSC2,CSC);

       //Cholesky decomposition of CSC'
       gsl_matrix_memcpy(Cholesky,CSC);
       //turn off gsl error handling from exiting program on failure.
       gsl_set_error_handler_off();

       status1=gsl_linalg_cholesky_decomp(Cholesky);
       printf("Cholesky decomposition status = %i\n", status1);
       //Invert CSC'
       status2=gsl_linalg_cholesky_invert(Cholesky);
       printf("Cholesky inverse status = %i\n", status2);

       //if status!=0 then exit function
       if(status1!=0||status2!=0)
         {
           printf("The CSC matrix isn't positive definite. Check contrast matrix for linearly dependent rows.\n");
           simple_message_dialog("The CSC matrix isn't positive definite. Check contrast matrix for linearly dependent rows.");
         }
       else
         {
           //Find T2.
           gsl_blas_dgemm(CblasTrans, CblasNoTrans,1.0, Cx, Cholesky, 0.0, T2_temp1);
           gsl_blas_dgemm(CblasNoTrans, CblasNoTrans,1.0, T2_temp1, Cx, 0.0, T2_temp2);
           T2=(double)rows*(gsl_matrix_get(T2_temp2, 0, 0));
           if(ShowAll==1)
             {
               printf("Using Cholesky decomposition T2 = %f\n", T2);
             }
   
           //F for alpha=.05.
           double fdist=gsl_cdf_fdist_Qinv(alpha,columns-1,(rows-columns+1));
           fdist=((double)(rows-1)*(double)(columns-1)/(double)(rows-columns+1))*fdist;
           if(ShowAll==1)
             {
               printf("F-dist %f\n", fdist);
             }

           //call print intervals
           if(SuppliedContrasts==NULL)
             {
               print_t2_confidence_intervals_control(CSC2, Cx, alpha, T2, fdist, rows, columns, plate, control, textview);
             }
           else
             {
               print_t2_confidence_intervals_supplied(CSC2, Cx, SuppliedContrasts, alpha, T2, fdist, row1, rows, plate, textview);
             }
          }

       if(SuppliedContrasts==NULL)
         {
           gsl_matrix_free(Contrasts);
         }
       gsl_matrix_free(DataMatrix);
       gsl_matrix_free(Covariance);
       gsl_matrix_free(CS);
       gsl_matrix_free(CSC);
       gsl_matrix_free(CSC2);
       gsl_matrix_free(CSCinverse);
       gsl_matrix_free(Cholesky);
       gsl_matrix_free(Cx);
       gsl_matrix_free(T2_temp1);
       gsl_matrix_free(T2_temp2);
       gsl_matrix_free(Mean);

    }

void print_t2_confidence_intervals_control(gsl_matrix *CSC2, gsl_matrix *Cx, double alpha, double T2, double fdist, int rows, int columns, int plate, int control, GtkTextView *textview)
    {
       int i=0;
       int j=0;
       double temp=0;
       GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

       //Print intervals.
       //printf("Plate Control Test alpha T2 F LowerConfidence UpperConfidence\n");
       for(i=0; i<columns-1; i++)
           {
             for(j=0;j<columns-1;j++)
                {
                  if(i==j)
                    {
                      temp=gsl_matrix_get(CSC2, i, j);
                    }
                }
             if(i>control||i==control)
                {
                  //printf("%i %i %i %f %f %f %f %f\n", plate, control+1, i+1+1, alpha, T2, fdist, gsl_matrix_get(Cx, i, 0)-(sqrt(fdist))*(sqrt((temp/(double)rows))), gsl_matrix_get(Cx, i, 0)+(sqrt(fdist))*(sqrt((temp/(double)rows))));
                  char *string;
                  asprintf(&string, "%i %i %i %f %f %f %f <uc%i-ut%i< %f\n", plate, control+1, i+1+1, alpha, T2, fdist, gsl_matrix_get(Cx, i, 0)-(sqrt(fdist))*(sqrt((temp/(double)rows))), control+1, i+1+1, gsl_matrix_get(Cx, i, 0)+(sqrt(fdist))*(sqrt((temp/(double)rows)))); 
                  gtk_text_buffer_insert_at_cursor(buffer, string, -1);
                  free(string);
                }
             else
                {
                  //printf("%i %i %i %f %f %f %f %f\n", plate, control+1, i+1, alpha, T2, fdist, gsl_matrix_get(Cx, i, 0)-(sqrt(fdist))*(sqrt((temp/(double)rows))), gsl_matrix_get(Cx, i, 0)+(sqrt(fdist))*(sqrt((temp/(double)rows))));
                  char *string;
                  asprintf(&string, "%i %i %i %f %f %f %f <uc%i-ut%i< %f\n", plate, control+1, i+1, alpha, T2, fdist, gsl_matrix_get(Cx, i, 0)-(sqrt(fdist))*(sqrt((temp/(double)rows))), control+1, i+1, gsl_matrix_get(Cx, i, 0)+(sqrt(fdist))*(sqrt((temp/(double)rows))));
                  gtk_text_buffer_insert_at_cursor(buffer, string, -1);
                  free(string);
                }
           }

       //printf("\n");
   }
void print_t2_confidence_intervals_supplied(gsl_matrix *CSC2, gsl_matrix *Cx, gsl_matrix *SuppliedContrasts, double alpha, double T2, double fdist, int row1, int rows, int plate, GtkTextView *textview)
    {
       int i=0;
       int j=0;
       double temp=0;
       GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
       int Srows=SuppliedContrasts->size1;
       int Scolumns=SuppliedContrasts->size2;

       //Put contrasts into a string
       GString *string1=g_string_new("");
       GString *string2=g_string_new("");
       GString *string3=g_string_new("");
       GPtrArray* sArray = g_ptr_array_new();
       
       for(i=0; i<Srows; i++)
          {
            for(j=0; j<Scolumns; j++)
               {
                 if(gsl_matrix_get(SuppliedContrasts, i, j)>0.5&&string1->len==0)
                   {
                     g_string_append_printf(string1, "(u%i", j+1);
                   }
                 else if(gsl_matrix_get(SuppliedContrasts, i, j)>0.5)
                   {
                     g_string_append_printf(string1, "+u%i", j+1);
                   }
                 else
                   {
                   }
                 if(gsl_matrix_get(SuppliedContrasts, i, j)<-0.5&&string2->len==0)
                   {
                     g_string_append_printf(string2, "(u%i", j+1);
                   }
                 else if(gsl_matrix_get(SuppliedContrasts, i, j)<-0.5)
                   {
                     g_string_append_printf(string2, "+u%i", j+1);
                   }
                 else
                   {
                   }
               }
            g_string_append_printf(string1, ")");
            g_string_append_printf(string2, ")");
            g_string_append(string3, string1->str);
            g_string_append(string3, "-");
            g_string_append(string3, string2->str);
            g_ptr_array_add(sArray, g_strdup(string3->str));
            g_string_truncate(string1,0);
            g_string_truncate(string2,0);
            g_string_truncate(string3,0);      
          } 

       printf("Contrast Matrix Array of Strings\n");
       for(i=0;i<Srows;i++)
          {
            g_print("%s\n", (char*)g_ptr_array_index(sArray, i));
          }

       //Print intervals.
       for(i=0; i<row1; i++)
           {
             for(j=0;j<row1;j++)
                {
                  if(i==j)
                    {
                      temp=gsl_matrix_get(CSC2, i, j);
                    }
                 }
                //printf("%i %i %f %f %f %s %f %f\n", plate, i+1, alpha, T2, fdist, (char*)g_ptr_array_index(sArray, i), gsl_matrix_get(Cx, i, 0)-(sqrt(fdist))*(sqrt((temp/(double)rows))), gsl_matrix_get(Cx, i, 0)+(sqrt(fdist))*(sqrt((temp/(double)rows))));
                char *string;
                asprintf(&string, "%i %i %f %f %f %f <%s< %f\n", plate, i+1, alpha, T2, fdist, gsl_matrix_get(Cx, i, 0)-(sqrt(fdist))*(sqrt((temp/(double)rows))), (char*)g_ptr_array_index(sArray, i), gsl_matrix_get(Cx, i, 0)+(sqrt(fdist))*(sqrt((temp/(double)rows))));
                gtk_text_buffer_insert_at_cursor(buffer, string, -1);
                free(string);
           }
       //printf("\n");

       g_string_free(string1,TRUE);
       g_string_free(string2,TRUE);
       g_string_free(string3,TRUE);
       g_ptr_array_free(sArray, TRUE);
   }   
 void cov_calculate(gsl_matrix *Covariance, gsl_matrix *DataMatrix)
   {
    gsl_vector_view a, b;
    int i;
    int j;

    //Compare combinations of columns for the covariance matrix. Code originally found on internet at gnu.org. It used size1 which needed to be changed to size2 for column by column comparison.
    for (i = 0; i < DataMatrix->size2; i++)  
      {
         for (j = 0; j < DataMatrix->size2; j++)  
          {
            double v;
            a = gsl_matrix_column (DataMatrix, i);
            b = gsl_matrix_column (DataMatrix, j);
            v = gsl_stats_covariance (a.vector.data, a.vector.stride, b.vector.data, b.vector.stride, a.vector.size);
            gsl_matrix_set (Covariance, i, j, v);
          }
       }

   }
void build_contrast_matrix(int columns, int control, gsl_matrix *Contrasts)
    {   
       int i=0;
       int j=0;
       
       for(i=0;i<columns;i++)//need columns by columns to ignore one row
          {
            for(j=0;j<columns;j++)
               {
                 if(i!=control)//ignore the control row
                   {
                      if(i<control)
                        {
                          if(j==control)
                             {
                               gsl_matrix_set(Contrasts, i, j, 1.0);
                             }
                          if(j!=control && i==j)
                             {
                               gsl_matrix_set(Contrasts, i, j, -1.0);
                             }
                         }
                      if(i>control)
                         {
                           if(j==control)
                             {
                               gsl_matrix_set(Contrasts, i-1, j, 1.0);
                             }
                          if(j!=control && i==j)
                             {
                               gsl_matrix_set(Contrasts, i-1, j, -1.0);
                             }
                         }
                   }
               }
          }
    }
int build_contrast_matrix_from_array(char contrasts[], gsl_matrix *Contrasts, int rows, int columns, int numbers)
    {
      //Check that rows equal zero. If so, load the contrast matrix.
      int i=0;
      int j=0;
      double records[numbers];
      int iCounter=0;
      char previous='2';
      int AddRows=0;
      int RowsEqualZero=0;
      
      //Zero the records array;
      for(i=0;i<numbers;i++)
         {
           records[i]=3.0;
         }

      for(i=0;i<=strlen(contrasts);i++)
         {
           printf("%c",contrasts[i]);
         }

      printf("\n");
      //load text into array of doubles.
      for(i=0;i<=strlen(contrasts);i++)
         {
           if(contrasts[i]=='0')
             {
               records[iCounter]=0.0;
               iCounter++;
             }
           else if(contrasts[i]=='1')
             {
               if(previous!='-')
                 {
                   records[iCounter]=1.0;
                   iCounter++;
                 }
             }
           else if(contrasts[i]=='-')
             {
                   
               records[iCounter]=-1.0;
               iCounter++;
             }
           else
             {
               //that should be all the cases
             }  
           previous=contrasts[i];  
         }
    
      //check that the rows equal 0
      iCounter=0;
      for(i=0;i<rows;i++)
         {
           for(j=0;j<columns;j++)
              {
                //printf("%f ", records[iCounter]);
                RowsEqualZero=records[iCounter]+RowsEqualZero;
                iCounter++;
              }
           printf("\n");
           if(RowsEqualZero!=0)
              {
                printf("Row %i doesn't add to zero.\n", i+1);
                simple_message_dialog("A row doesn't add to zero.");
                AddRows=1;
              }
           RowsEqualZero=0;
         }


      //load array of doubles into the contrast matrix
      printf("Load Matrix\n");
      iCounter=0;
      if(AddRows==0)
        {
          for(i=0;i<rows;i++)
             {
               for(j=0;j<columns;j++)
                  {
                    gsl_matrix_set(Contrasts, i, j, records[iCounter]);
                    printf("%f ", gsl_matrix_get(Contrasts, i, j));
                    iCounter++;
                  }
               printf("\n");
             }
        }
      
      
      return AddRows;
    
    }
void z_factor(int iRadioButton, int iControl, GtkTextView *textview)
    {
/*
  
  Z-factor calculation
    Z=1-(3SD of sample+3SD of control)/(mean of sample-mean of control)

*/
      int i=0;
      int j=0;
      double ZFactor=0;
      int iCounter=0;
      int iPlates=0;
      int iGroups=0;
      int iError=0;
      char *sql1;
      char *sql2;
      char *sql3;

      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));

      iControl=iControl-1;

      if(iRadioButton==1)
        {
          asprintf(&sql1, "SELECT avg(T1.data) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Groups ORDER BY Plate,Groups;");
          asprintf(&sql2, "SELECT stddev(T1.data) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Groups ORDER BY Plate,Groups;");
          asprintf(&sql3, "SELECT max(Groups) FROM aux;");
        }
   
      if(iRadioButton==2)
        {
          asprintf(&sql1, "SELECT avg(T1.percent) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Groups ORDER BY Plate,Groups;");
          asprintf(&sql2, "SELECT stddev(T1.percent) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Groups ORDER BY Plate,Groups;");
          asprintf(&sql3, "SELECT max(Groups) FROM aux;");
        }

      if(iRadioButton==3)
        {
          asprintf(&sql1, "SELECT avg(T1.data) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Picks ORDER BY Plate,Picks;");
          asprintf(&sql2, "SELECT stddev(T1.data) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Picks ORDER BY Plate,Picks;");
          asprintf(&sql3, "SELECT max(Picks) FROM aux;");
        }

        if(iRadioButton==4)
        {
          asprintf(&sql1, "SELECT avg(T1.percent) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Picks ORDER BY Plate,Picks;");
          asprintf(&sql2, "SELECT stddev(T1.percent) FROM Data as T1,Aux as T2 WHERE T1. KeyID=T2.KeyID GROUP BY Plate,Picks ORDER BY Plate,Picks;");
          asprintf(&sql3, "SELECT max(Picks) FROM aux;");
        }

      apop_db_open("VelociRaptorData.db");
      gsl_vector *vMean=apop_query_to_vector("%s",sql1);
      gsl_vector *vStdDev=apop_query_to_vector("%s",sql2); 
      gsl_vector *vGroups=apop_query_to_vector("%s",sql3); 
      printf("vMeanSize=%i vStdDevSize=%i vGroups=%i\n", vMean->size, vStdDev->size,(int)gsl_vector_get(vGroups,0) );

      if(vMean==NULL||vStdDev==NULL||vGroups==NULL)
        {
          iError=1;
          printf("No data returned from the database.\n");
          simple_message_dialog("No data returned from the database.");
        }

      if(iError==0)
        {
          gtk_text_buffer_insert_at_cursor(buffer, "Plate ControlMean TestMean ControlSD TestSD Z-factor\n", -1);
          iGroups=(int)gsl_vector_get(vGroups,0);
          //with the aggregate queries this division should work.
          iPlates=vMean->size/iGroups;

          for(i=0;i<iPlates;i++)
             {
              for(j=0;j<iGroups;j++)
                 {
                   //exclude control
                   if(iControl!=iCounter)
                     {
                       ZFactor=1.0-(3.0*gsl_vector_get(vStdDev, iCounter)+3.0*gsl_vector_get(vStdDev, iControl))/fabs(gsl_vector_get(vMean, iCounter)-gsl_vector_get(vMean, iControl));
                       //printf("%i %f %f %f %f %f\n", i+1, gsl_vector_get(vMean, iControl), gsl_vector_get(vMean, iCounter), gsl_vector_get(vStdDev, iControl), gsl_vector_get(vStdDev, iCounter), ZFactor);
                       char *string;
                       asprintf(&string, "%i %f %f %f %f %f\n", i+1, gsl_vector_get(vMean, iControl), gsl_vector_get(vMean, iCounter), gsl_vector_get(vStdDev, iControl), gsl_vector_get(vStdDev, iCounter), ZFactor);
                       gtk_text_buffer_insert_at_cursor(buffer, string, -1);
                       free(string);
                     }
                    iCounter++;
                 }
               iControl=iControl+iGroups;
             }
         }

      apop_db_close(0);
      if(vMean!=NULL)
        {
         gsl_vector_free(vMean);
        }
      if(vStdDev!=NULL)
        {
         gsl_vector_free(vStdDev);
        }
      if(vGroups!=NULL)
        {
         gsl_vector_free(vGroups);
        }
      free(sql1);
      free(sql2);
      free(sql3);
    }
void calculate_contingency_values(double alpha, GtkTextView *textview, int check_box1, int check_box2, int check_box3, int check_box4, int check_box5, int check_box6)
    {
      char *sql1;
      char *sql2;
      char *sql3;
      char *sql4;
      char *sql5;
      char *sql6;
      char *sql7;
      char *sql8;
      int VectorSize=0;
      int i=0;
      double TempValue=0;
    
      printf("Get Contingency Data\n");
      GtkTextBuffer *buffer = gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
      //Get rows
      asprintf(&sql1, "SELECT MAX(RowID) FROM TempContingency GROUP BY Plate;");
      //Get columns
      asprintf(&sql2, "SELECT MAX(ColumnID) FROM TempContingency GROUP BY Plate;");
      //Get total
      asprintf(&sql3, "SELECT SUM(TestValue) FROM TempContingency GROUP BY Plate;");
      //Get X
      asprintf(&sql4, "SELECT SUM(T1TestValue * T2TestValue) FROM TempCombinationsOf2 WHERE T1ColumnID = T2ColumnID;");
      //Get Y
      asprintf(&sql5, "SELECT  SUM(T1TestValue * T2TestValue) FROM TempCombinationsOf2 WHERE T1RowID=T2RowID;");
      //Get P
      asprintf(&sql6, "SELECT SUM(T1TestValue * T2TestValue) FROM TempCombinationsOf2 WHERE T1RowID <> T2RowID AND T1ColumnID <> T2ColumnID AND T1ColumnID < T2ColumnID;");
      //Get Q
      asprintf(&sql7, "SELECT SUM(T1TestValue * T2TestValue) FROM TempCombinationsOf2 WHERE T1RowID <> T2RowID AND T1ColumnID <> T2ColumnID AND T1ColumnID > T2ColumnID;");
      //Get chi2
      asprintf(&sql8, "SELECT (SELECT SUM(TestValue) FROM TempPlateNumber)*(SUM((T4.TestValue*T4.TestValue)/(T3.V1*T3.V2))-1) AS Chi2 FROM (SELECT R, C, V1, V2 FROM(SELECT RowID AS R, SUM(TestValue) AS V1 FROM TempPlateNumber GROUP BY RowID) AS T1, (SELECT ColumnID AS C, SUM(TestValue) AS V2 FROM TempPlateNumber GROUP BY ColumnID) AS T2) AS T3,(SELECT RowID AS R2, ColumnID AS C2, TestValue FROM TempPlateNumber) AS T4 WHERE T4.R2=T3.R  AND T4.C2=T3.C ORDER BY T4.R2;");

      apop_db_open("VelociRaptorData.db");
      
      apop_query("BEGIN;");
      //Join temp table that has rows and columns with the data set.
      apop_query("CREATE VIEW TempContingency AS SELECT TempAux2.TestID, TempAux2.Plate, TempAux2.RowID, TempAux2.ColumnID, Data.data AS TestValue FROM TempAux2,Data WHERE TempAux2.TestID=Data.KeyID;");
      gsl_vector *vRows=apop_query_to_vector("%s",sql1);
      gsl_vector *vColumns=apop_query_to_vector("%s",sql2); 
      gsl_vector *vTotal=apop_query_to_vector("%s",sql3);

      VectorSize=vRows->size; 
      printf("VectorSize=%i\n", VectorSize);
      gsl_vector *vX=gsl_vector_calloc(VectorSize);
      gsl_vector *vY=gsl_vector_calloc(VectorSize);
      gsl_vector *vP=gsl_vector_calloc(VectorSize);
      gsl_vector *vQ=gsl_vector_calloc(VectorSize);
      gsl_vector *vChi2=gsl_vector_calloc(VectorSize);

      for(i=0;i<VectorSize;i++)
         {
           //Iterate one plate at a time. Need combinations of 2 from each plate. 
           apop_query("CREATE VIEW TempPlateNumber AS SELECT * FROM TempContingency WHERE Plate=%i;",i+1);
           apop_query("CREATE VIEW TempCombinationsOf2 AS SELECT T1.Plate AS Plate1, T1.TestID AS T1TestID, T1.RowID AS T1RowID, T1.ColumnID AS T1ColumnID, T1.TestValue AS T1TestValue, T2.TestID AS T2TestID, T2.RowID AS T2RowID, T2.ColumnID AS T2ColumnID, T2.TestValue AS T2TestValue FROM TempPlateNumber T1 INNER JOIN TempPlateNumber T2 ON T1.TestID < T2.TestID;");

           TempValue=apop_query_to_float("%s",sql4);
           gsl_vector_set(vX,i,TempValue);
           TempValue=apop_query_to_float("%s",sql5);
           gsl_vector_set(vY,i,TempValue);
           TempValue=apop_query_to_float("%s",sql6);
           gsl_vector_set(vP,i,TempValue);
           TempValue=apop_query_to_float("%s",sql7);
           gsl_vector_set(vQ,i,TempValue);
           TempValue=apop_query_to_float("%s",sql8);
           gsl_vector_set(vChi2,i,TempValue);

           apop_query("DROP VIEW TempPlateNumber;");
           apop_query("DROP VIEW TempCombinationsOf2;");
         }
      apop_query("DROP VIEW TempContingency;");
      apop_query("COMMIT;");
      //Table made in build_temp_table_for_contingency function. Put after commit to make sure it is dropped if the above transaction gets rolled back.
      printf("Drop TempAux2\n");
      apop_query("DROP TABLE IF EXISTS TempAux2;");
      apop_db_close(0);

      printf("Print Contingency Data to TextView\n");
      //printf("Plate Rows Columns Total X Y P Q Chi2 Chi2Lookup\n");
      gtk_text_buffer_insert_at_cursor(buffer, "Plate Rows Columns Total X Y P Q Chi2 Chi2Lookup", -1);
      if(check_box1==1)
        {
          gtk_text_buffer_insert_at_cursor(buffer, " PearsonC", -1);
        }
      if(check_box2==1)
        {
          gtk_text_buffer_insert_at_cursor(buffer, " PearsonC*", -1);
        }
      if(check_box3==1)
        {
          gtk_text_buffer_insert_at_cursor(buffer, " Tshuprow'sT", -1);
        }
      if(check_box4==1)
        {
          gtk_text_buffer_insert_at_cursor(buffer, " Cramer'sV", -1);
        }
      if(check_box5==1)
        {
          gtk_text_buffer_insert_at_cursor(buffer, " Somer'sd", -1);
        }
      if(check_box6==1)
        {
          gtk_text_buffer_insert_at_cursor(buffer, " Gamma", -1);
        }
      gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
      
      for(i=0;i<VectorSize;i++)
         {
           //printf("%i %i %i %f %f %f %f %f %f %f\n", i+1, (int)gsl_vector_get(vRows,i), (int)gsl_vector_get(vColumns,i), gsl_vector_get(vTotal,i), gsl_vector_get(vX,i), gsl_vector_get(vY,i), gsl_vector_get(vP,i), gsl_vector_get(vQ,i), gsl_vector_get(vChi2,i), gsl_cdf_chisq_Qinv(alpha,((gsl_vector_get(vRows,i)-1)*(gsl_vector_get(vColumns,i)-1))));
           char *string;
           asprintf(&string, "%i %i %i %f %f %f %f %f %f %f", i+1, (int)gsl_vector_get(vRows,i), (int)gsl_vector_get(vColumns,i), gsl_vector_get(vTotal,i), gsl_vector_get(vX,i), gsl_vector_get(vY,i), gsl_vector_get(vP,i), gsl_vector_get(vQ,i), gsl_vector_get(vChi2,i), gsl_cdf_chisq_Qinv(alpha,((gsl_vector_get(vRows,i)-1)*(gsl_vector_get(vColumns,i)-1))));
           gtk_text_buffer_insert_at_cursor(buffer, string, -1);
           free(string);
           //Pearson C
           if(check_box1==1)
             {
               char *string1;
               asprintf(&string1, " %f", sqrt((gsl_vector_get(vChi2,i)/(gsl_vector_get(vChi2,i) + gsl_vector_get(vTotal,i)))));
               gtk_text_buffer_insert_at_cursor(buffer, string1, -1);
               free(string1);
             }
            //Pearson C*
            if(check_box2==1)
             {
               char *string2;
               double temp;
               if(gsl_vector_get(vRows,i)<=gsl_vector_get(vColumns,i))
                 {
                   temp=gsl_vector_get(vRows,i);
                 }
               else
                 {
                   temp=gsl_vector_get(vColumns,i);
                 }
               asprintf(&string2, " %f", sqrt((gsl_vector_get(vChi2,i)/(gsl_vector_get(vChi2,i) + gsl_vector_get(vTotal,i))))/sqrt((temp-1)/temp));
               gtk_text_buffer_insert_at_cursor(buffer, string2, -1);
               free(string2);
             }
            //Tshuprow's T
            if(check_box3==1)
             {
               char *string3;
               asprintf(&string3, " %f", sqrt(gsl_vector_get(vChi2,i)/(gsl_vector_get(vTotal,1)*sqrt((gsl_vector_get(vRows,i)-1)*(gsl_vector_get(vColumns,i)-1)))));
               gtk_text_buffer_insert_at_cursor(buffer, string3, -1);
               free(string3);
             }
            //Cramer's V
            if(check_box4==1)
             {
               char *string4;
               double temp2;
               if(gsl_vector_get(vRows,i)<=gsl_vector_get(vColumns,i))
                 {
                   temp2=gsl_vector_get(vRows,i);
                 }
               else
                 {
                   temp2=gsl_vector_get(vColumns,i);
                 }
               asprintf(&string4, " %f", sqrt(gsl_vector_get(vChi2,i)/(gsl_vector_get(vTotal,1)*(temp2-1))  ));
               gtk_text_buffer_insert_at_cursor(buffer, string4, -1);
               free(string4);
             }
            //Somer's d
            if(check_box5==1)
             {
               char *string5;
               asprintf(&string5, " %f", ((gsl_vector_get(vP,i)-gsl_vector_get(vQ,i))/(gsl_vector_get(vP,i) + gsl_vector_get(vQ,i) + gsl_vector_get(vY,i))));
               gtk_text_buffer_insert_at_cursor(buffer, string5, -1);
               free(string5);
             }
            //Gamma
            if(check_box6==1)
             {
               char *string6;
               asprintf(&string6, " %f", ((gsl_vector_get(vP,i)-gsl_vector_get(vQ,i))/(gsl_vector_get(vP,i) + gsl_vector_get(vQ,i))));
               gtk_text_buffer_insert_at_cursor(buffer, string6, -1);
               free(string6);
             }
            gtk_text_buffer_insert_at_cursor(buffer, "\n", -1);
         }

      if(vRows!=NULL)
        {
         gsl_vector_free(vRows);
        }
      if(vColumns!=NULL)
        {
         gsl_vector_free(vColumns);
        }
      if(vTotal!=NULL)
        {
         gsl_vector_free(vTotal);
        }
      if(vY!=NULL)
        {
         gsl_vector_free(vY);
        }
      if(vX!=NULL)
        {
         gsl_vector_free(vX);
        }
      if(vP!=NULL)
        {
         gsl_vector_free(vP);
        }
      if(vQ!=NULL)
        {
         gsl_vector_free(vQ);
        }
      if(vChi2!=NULL)
        {
         gsl_vector_free(vChi2);
        }
      
      free(sql1);
      free(sql2);
      free(sql3);
      free(sql4);
      free(sql5);
      free(sql6);
      free(sql7);
      free(sql8);
    }
void build_temp_table_for_contingency(int plates,int rows,int columns)
    {
      int i=0;
      int j=0;
      int k=0;
      sqlite3 *handle;
      sqlite3_stmt *stmt1;

      printf("Build TempAux2\n");
      sqlite3_open("VelociRaptorData.db",&handle);
      
      sqlite3_exec(handle, "BEGIN;", NULL, NULL, NULL);
      //Table created here and dropped in function calculate_contingency_values.
      sqlite3_exec(handle, "CREATE TABLE TempAux2(TestID INTEGER PRIMARY KEY NOT NULL, Plate INTEGER, RowID INTEGER, ColumnID INTEGER);", NULL, NULL, NULL);
      sqlite3_prepare(handle, "INSERT INTO TempAux2(Plate,RowID,ColumnID) VALUES(?,?,?);", -1, &stmt1, 0);
      
      for(i=0;i<plates;i++)
         {
           for(j=0;j<rows;j++)
              {
                for(k=0;k<columns;k++)
                   {
                     sqlite3_bind_int(stmt1, 1, i+1);
                     sqlite3_bind_int(stmt1, 2, j+1);
                     sqlite3_bind_int(stmt1, 3, k+1);
                     sqlite3_step(stmt1);
                     sqlite3_reset(stmt1); 
                   }
              }
         }
       sqlite3_exec(handle, "COMMIT;", NULL, NULL, NULL);
       sqlite3_finalize(stmt1);

      sqlite3_close(handle);
      printf("TempAux2 Built\n");
    }

