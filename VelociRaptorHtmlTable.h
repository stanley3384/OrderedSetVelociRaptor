
/*

For tabular HTML output from SQLite.

Copyright (c) 2014 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
cecashon@aol.com

*/

int check_sql_for_select(char *sql);
void parse_sql_field_names(char *html_file_name, char *database_name, char *sql, int precision, int font_size, char *bg_color, char *field_bg_color, char *font_color);



