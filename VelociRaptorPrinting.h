

/*
 Printing functions for the VelociRaptor application.
     
 Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
*/

#include <gtk/gtk.h>
#include <math.h>
#include <string.h>

typedef struct
  {
    GtkWidget *window, *textview;
  } Widgets;

void print_textview(GtkWidget*, Widgets*);



