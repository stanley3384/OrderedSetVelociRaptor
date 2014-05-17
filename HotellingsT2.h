
/* Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.  */

void hotellings_T2(int iRadioButton, double alpha, int ShowAll, int control, GtkTextView *textview, GtkWidget *progress, gsl_matrix *SuppliedContrasts, int MatrixColumns);
void build_contrast_matrix(int columns, int control, gsl_matrix *Contrasts);
void cov_calculate(gsl_matrix *Covariance, gsl_matrix *DataMatrix);
void tsquared_multiple_comparison(double alpha, int ShowAll, int plate, int rows, int columns, int control, double dData[], GtkTextView *textview, gsl_matrix *SuppliedContrasts);
void print_t2_confidence_intervals_control(gsl_matrix *CSC2,gsl_matrix *Cx,double alpha, double T2, double fdist, int rows, int columns, int plate, int control, GtkTextView *textview);
void print_t2_confidence_intervals_supplied(gsl_matrix *CSC2, gsl_matrix *Cx,gsl_matrix *SuppliedContrasts, double alpha, double T2, double fdist, int row1, int rows, int plate, GtkTextView *textview);
int build_contrast_matrix_from_array(char contrasts[], gsl_matrix *Contrasts, int rows, int columns, int numbers);
void z_factor(int iRadioButton, int iControl, GtkTextView *textview);
void build_temp_table_for_contingency(int plates,int rows,int columns);
void calculate_contingency_values(double alpha, GtkTextView *textview, int check_box1, int check_box2, int check_box3, int check_box4, int check_box5, int check_box6);
