/* Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.*/

void simple_message_dialog(const char *str);
void entry_field_validation(const gchar *pPlateSizeText1, const gchar *pPlateStatsText1, GtkWidget *entry);
void control_changed_validation(const gchar *pPlateSizeText, GtkWidget *entry);
int critical_value_changed_validation(GtkWidget *entry);
int groups_database_validation(GtkWidget *entry);
int picks_database_validation(GtkWidget *entry);
int contrast_matrix_validation(char contrasts[], int *rows, int *columns);



