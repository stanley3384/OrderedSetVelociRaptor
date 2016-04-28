
/* Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.*/

//Interop with Fortran for Dunnett's critical values.
extern void mvdistdun( int *N, double COVRNC1[], int *NU, int *M, double LOWER[], double CONSTR[], double UPPER[], int INFIN[], double DELTA[], int *MAXPTS, double *ABSEPS, double *RELEPS, double *ERROR, double *VALUE, int *NEVALS, int *INFORM, double *ALPHA, double *TALPHA);

void basic_statistics_sql(GtkTextView *textview, int iRadioButton);
void anderson_darling_test(GtkTextView *textview, int iRadioButton);
void levenes_variance_test(GtkTextView *textview, int iRadioButton, double alpha);
void one_way_anova_sql(GtkTextView *textview, int iRadioButton, int check_box, double alpha);
void anova_format(GtkTextView *textview, int iPlates, int iBetweenDf,int iWithinDf,int iTotalDf, double one, double two, double three, double alpha);
void anova_format_tabular(GtkTextView *textview, int iPlates, int iBetweenDf,int iWithinDf,int iTotalDf, double one, double two, double three, double alpha);
void database_to_box_graph_sql(int iRadioButton, int iCombo, int lower_bound, int upper_bound);
void database_to_error_graph_sql(int iRadioButton1, int iRadioButton2, int iCombo, int lower_bound, int upper_bound);
void database_to_scatter_graph_sql(int iRadioButton, int iCombo, int lower_bound, int upper_bound);
void plot_matrix_now(gsl_matrix *data, int graph, int iCombo, int lower_bound, int upper_bound);
void comparison_with_control_sql(int iRadioButton, int iControlValue, double alpha, int iRadioCritVal, GtkTextView *textview, GtkWidget *progress, int *pBreakLoop);
void multiple_comparison_with_controls(char *sql1, char *sql2, char *sql3, char *sql4, char *sql5, char *sql6, int iControlValue, double alpha, int iRadioCritVal, GtkTextView *textview, GtkWidget*,int *pBreakLoop);
double dunn_sidak_critical_value(double alpha, int Comparisons, int DF);
double bonferroni_critical_value(double alpha, int Comparisons, int DF);
double dunnetts_critical_value(double alpha1,int CovarianceArray[],int iCovSize,int iMeanDistance,int iDF);
void GenerateRandomValues(double dDataArray[], int iNumberOfPlates,int iPlateSize,int iSetSizeForStatistics, const gchar *pPlatePosControlText, const gchar *pPlateNegControlText, int seed_value, double param, double pFuncDistribution());
void CalculatePercentControl(double dDataArray[], double dPercentArray[], int iPlateSize, int iNumberOfPlates, int iSetSizeForStatistics, const gchar *pPlatePosControlText, const gchar *pPlateNegControlText);
void PlateMapInt(double dDataArray[], int iNumberOfPlates, int iPlateSize, int iRows, int iColumns);
void PlateMapDouble(double dDataArray[], int iNumberOfPlates, int iPlateSize, int iRows, int iColumns);
void PlateMapDoubleTruncate(double dDataArray[], int iNumberOfPlates, int iPlateSize, int iRows, int iColumns, int digits);
void send_text_to_database(const gchar *pTableName, GtkWidget *textview);
void build_combo_table_sql(int ComboSet, int ComboSubSet, const gchar *TableName);
void build_permutation_table_sql(int ComboSet, const gchar *TableName);
void copy_treeview_to_database_sql(GtkWidget*, GtkWidget*, const gchar *pWindowTitle);
void format_text_platemap(double dEntry1, double dEntry2, double dEntry3, double dEntry4, GtkTextView *textview);
void format_text_platemap_heatmap_high_low(GtkTextView *textview, double *high, double *low);
void format_text_platemap_heatmap(GtkTextView *textview, int high, int low);
void format_text_platemap_heatmap_iris(GtkTextView *textview, int high, int low);
void format_text_platemap_heatmap_iris2(GtkTextView *textview, int high, int low);
void format_text_platemap_heatmap_sun(GtkTextView *textview, int high, int low);
void heatmap_to_html_sql(int iRadioButton, int rows, int columns, int precision, int font_size, int gradient);
void heatmap_to_html_gradient(double test_data[], int rows, int columns, int plate_size, int plates, int precision, int font_size, int gradient);
void make_heatmap_html(double test_data[], int rows, int columns, int plate_size, int plates, int precision, int font_size, char **gradient);






