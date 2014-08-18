
/*

     Test Code. A 3d stacked heatmapped microtiter platemap. Version 2. Add menu and dialogs
to get uniform random data and database data dynamically. Provide a couple of heatmap settings
also.

Test the environment and FPS on atom netbook with Mesa driver. Ubuntu 12.04. OK, it works.
    vblank_mode=0 glxgears
    glxinfo

About Xlib windows and displays.
    http://www.sbin.org/doc/Xlib/chapt_03.html

Compile with
    gcc -Wall -O2 `pkg-config --cflags gtk+-3.0` plate_stack_viewGL2.c -o plate_stack_viewGL2 -lGL -lGLU -lX11 -lm -lgsl -lgslcblas -lsqlite3 `pkg-config --libs gtk+-3.0 gdk-x11-3.0`

C. Eric Cashon
*/

#include<X11/Xlib.h>
#include<GL/glx.h>
#include<GL/gl.h>
#include<GL/glu.h>
#include<gtk/gtk.h>
#include<gdk/gdkx.h>
#include<gsl/gsl_matrix.h>
#include<gsl/gsl_rng.h>
#include<gsl/gsl_randist.h>
#include<stdio.h>
#include<stdlib.h>
#include<stdbool.h>
#include<sqlite3.h>

static GtkWidget *window=NULL;
static GtkWidget *da=NULL;
static GdkWindow *DrawingWindow=NULL;
static Window X_window;
static Display *X_display;
static GLXContext X_context;
static XVisualInfo *X_visual;
static XWindowAttributes X_attributes;
static GLint attributes[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
static float ang=0.0;
static guint timer_id;
static float scaleGL=2.5;
static float rotation[]={1.0 , 0.0, 0.0};
static bool rotate_drawing=true;
static gsl_matrix *test_data_points= NULL;
static double high=0;
static double low=0; 
static int rows=0;
static int columns=0;
static int plates=0;
static bool setting_rgb=true;
static bool setting_above=false;
static double setting_percent=0.10;

static void data_db_dialog(GtkWidget *menu, gpointer p);
static void data_test_dialog(GtkWidget *menu, gpointer p);
static void about_dialog(GtkWidget *menu, gpointer p);
static void setting_above_dialog(GtkWidget *menu, gpointer p);
static void close_program(void);
static void get_data_points(void);
static void get_db_data(int iRadioButton);
static void heatmap_rgb(double temp1, double high, double low, float rgb[]);
static void heatmap_above(double temp1, double high, double low, float rgb[]);
static void set_heatmap(GtkWidget *menu, gpointer data);
static void drawGL(GtkWidget *da, gpointer data);
static void configureGL(GtkWidget *da, gpointer data);
static gboolean rotate(gpointer data);
static void stop_rotation(GtkWidget *da, gpointer data);
static void scale_drawing(GtkRange *range,  gpointer data);
static void rotation_axis(GtkWidget *axis, gpointer data);


int main(int argc, char **argv)
 {
   GtkWidget *data_menu, *data_db, *data_test, *data_item, *rotate_menu, *rotate_x, *rotate_y, *rotate_z, *menu_bar, *rotate_item, *settings_menu, *settings_item, *settings_rgb, *settings_above, *help_menu, *help_about, *help_item;
   int x1=0;
   int y1=1;
   int z1=2;
   int set_rgb=0;
   int set_above=1;
   gtk_init(&argc, &argv);

   window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Platemap Viewer 3d");
   gtk_window_set_default_size(GTK_WINDOW(window), 500, 550);

   data_menu=gtk_menu_new();
   data_db=gtk_menu_item_new_with_label("Database");
   data_test=gtk_menu_item_new_with_label("Test");
   gtk_menu_shell_append(GTK_MENU_SHELL(data_menu), data_db);
   gtk_menu_shell_append(GTK_MENU_SHELL(data_menu), data_test);
   g_signal_connect(data_db, "activate", G_CALLBACK(data_db_dialog), NULL);
   g_signal_connect(data_test, "activate", G_CALLBACK(data_test_dialog), NULL);

   rotate_menu=gtk_menu_new();
   rotate_x=gtk_menu_item_new_with_label("Rotate x-axis");
   rotate_y=gtk_menu_item_new_with_label("Rotate y-axis");
   rotate_z=gtk_menu_item_new_with_label("Rotate z-axis");
   gtk_menu_shell_append(GTK_MENU_SHELL(rotate_menu), rotate_x);
   gtk_menu_shell_append(GTK_MENU_SHELL(rotate_menu), rotate_y);
   gtk_menu_shell_append(GTK_MENU_SHELL(rotate_menu), rotate_z);
   g_signal_connect(rotate_x, "activate", G_CALLBACK(rotation_axis), &x1);
   g_signal_connect(rotate_y, "activate", G_CALLBACK(rotation_axis), &y1);
   g_signal_connect(rotate_z, "activate", G_CALLBACK(rotation_axis), &z1);

   settings_menu=gtk_menu_new();
   settings_rgb=gtk_menu_item_new_with_label("Heatmap RGB");
   settings_above=gtk_menu_item_new_with_label("Heatmap Above");
   gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), settings_rgb);
   gtk_menu_shell_append(GTK_MENU_SHELL(settings_menu), settings_above);
   g_signal_connect(settings_rgb, "activate", G_CALLBACK(set_heatmap), &set_rgb);
   g_signal_connect(settings_above, "activate", G_CALLBACK(setting_above_dialog), &set_above);

   help_menu=gtk_menu_new();
   help_about=gtk_menu_item_new_with_label("About");
   gtk_menu_shell_append(GTK_MENU_SHELL(help_menu), help_about);
   g_signal_connect(help_about, "activate", G_CALLBACK(about_dialog), NULL);

   menu_bar=gtk_menu_bar_new();
   gtk_widget_show(menu_bar);
   data_item=gtk_menu_item_new_with_label("Data");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(data_item), data_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), data_item);
   rotate_item=gtk_menu_item_new_with_label("Rotate");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(rotate_item), rotate_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), rotate_item);
   settings_item=gtk_menu_item_new_with_label("Settings");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(settings_item), settings_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), settings_item);
   help_item=gtk_menu_item_new_with_label("Help");
   gtk_menu_item_set_submenu(GTK_MENU_ITEM(help_item), help_menu);
   gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), help_item);

   GtkWidget *label1=gtk_label_new("Microtiter Platemap Stack");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *scale1=gtk_scale_new_with_range(GTK_ORIENTATION_HORIZONTAL,1,20,1);
   gtk_widget_set_hexpand(scale1, TRUE);
   gtk_range_set_increments(GTK_RANGE(scale1),1,1);
   g_signal_connect(GTK_RANGE(scale1), "value_changed", G_CALLBACK(scale_drawing), NULL);

   da=gtk_drawing_area_new();
   gtk_widget_set_double_buffered(da, FALSE);
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK);
   g_signal_connect(da, "button-press-event", G_CALLBACK(stop_rotation), NULL);

   GtkWidget *grid1=gtk_grid_new();
   gtk_container_add(GTK_CONTAINER(window), grid1);

   gtk_grid_attach(GTK_GRID(grid1), menu_bar, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), label1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), scale1, 0, 2, 1, 1);
   gtk_grid_attach(GTK_GRID(grid1), da, 0, 3, 1, 1);
  
   g_signal_connect_swapped(window, "destroy", G_CALLBACK(close_program), NULL);

   gtk_widget_show(window);

   g_signal_connect(da, "configure-event", G_CALLBACK(configureGL), NULL);
   g_signal_connect(da, "draw", G_CALLBACK(drawGL), NULL);

   gtk_widget_show_all(window);

   //Start with some random data points.
   rows=8;
   columns=12;
   plates=7;
   get_data_points();

   timer_id=g_timeout_add(1000/10, rotate, da);

   gtk_main();
   return 0;
  }
static void data_db_dialog(GtkWidget *menu, gpointer p)
  {
     GtkWidget *dialog, *grid1, *entry1, *entry2, *entry3, *label1, *label2, *label3, *label4, *radio1, *radio2, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("VelociRaptor Data", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);

     radio1=gtk_radio_button_new_with_label(NULL, "Data");
     radio2=gtk_radio_button_new_with_label_from_widget(GTK_RADIO_BUTTON(radio1), "Percent");
     
     label1=gtk_label_new("Database Column"); 
     label2=gtk_label_new("      Plates");
     label3=gtk_label_new("      Rows");
     label4=gtk_label_new("      Columns"); 
       
     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
     gtk_entry_set_text(GTK_ENTRY(entry1), "7");
  
     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 3);
     gtk_entry_set_text(GTK_ENTRY(entry2), "8");

     entry3=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry3), 3);
     gtk_entry_set_text(GTK_ENTRY(entry3), "12");    
     
     grid1=gtk_grid_new();
     gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 3, 1);
     gtk_grid_attach(GTK_GRID(grid1), radio1, 0, 1, 3, 1);
     gtk_grid_attach(GTK_GRID(grid1), radio2, 0, 2, 3, 1);
     gtk_grid_attach(GTK_GRID(grid1), entry1, 2, 3, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), entry2, 2, 4, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), entry3, 2, 5, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), label2, 0, 3, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), label3, 0, 4, 1, 1); 
     gtk_grid_attach(GTK_GRID(grid1), label4, 0, 5, 1, 1);         
 
     gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
     gtk_grid_set_column_spacing(GTK_GRID(grid1), 30);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
     gtk_container_add(GTK_CONTAINER(content_area), grid1); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 10);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int iRadioButton=0;
        int check1=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        int check2=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        int check3=atoi(gtk_entry_get_text(GTK_ENTRY(entry3)));

        if(check1<1||check2<1||check3<1)
          {
             printf("Rows, columns and plates need to be 1 or greater.\n");
          }
        else
          {
            //The globals
            plates=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
            rows=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
            columns=atoi(gtk_entry_get_text(GTK_ENTRY(entry3)));

            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio1)))
              {
                iRadioButton=1;
                get_db_data(iRadioButton);
              }
            if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(radio2)))
              {
                iRadioButton=2;
                get_db_data(iRadioButton);
              }

          }
       }

     gtk_widget_destroy(dialog);
  
  }
static void data_test_dialog(GtkWidget *menu, gpointer p)
  {
     GtkWidget *dialog, *grid1, *entry1, *entry2, *entry3, *label1, *label2, *label3, *label4, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Test Data", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
     
     label1=gtk_label_new("Uniform Random");
     label2=gtk_label_new("      Plates");  
     label3=gtk_label_new("      Rows");
     label4=gtk_label_new("      Columns"); 
         
     entry1=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry1), 3);
     gtk_entry_set_text(GTK_ENTRY(entry1), "7");

     entry2=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry2), 3);
     gtk_entry_set_text(GTK_ENTRY(entry2), "8"); 

     entry3=gtk_entry_new();
     gtk_entry_set_width_chars(GTK_ENTRY(entry3), 3);
     gtk_entry_set_text(GTK_ENTRY(entry3), "12");   
     
     grid1=gtk_grid_new();
     gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 3, 1);
     gtk_grid_attach(GTK_GRID(grid1), entry1, 2, 1, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), entry2, 2, 2, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), entry3, 2, 3, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), label2, 0, 1, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), label3, 0, 2, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), label4, 0, 3, 1, 1);          
 
     gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
     gtk_grid_set_column_spacing(GTK_GRID(grid1), 30);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
     gtk_container_add(GTK_CONTAINER(content_area), grid1); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 10);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        int check1=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
        int check2=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
        int check3=atoi(gtk_entry_get_text(GTK_ENTRY(entry3)));

        if(check1<1||check2<1||check3<1)
          {
             printf("Rows, columns and plates need to be 1 or greater.\n");
          }
        else
          {
             //Set the globals.
             plates=atoi(gtk_entry_get_text(GTK_ENTRY(entry1)));
             rows=atoi(gtk_entry_get_text(GTK_ENTRY(entry2)));
             columns=atoi(gtk_entry_get_text(GTK_ENTRY(entry3)));

             printf("Plates %i Rows %i Columns %i\n", plates, rows, columns);
             get_data_points();
          }

       }
     gtk_widget_destroy(dialog);
  
  }
static void setting_above_dialog(GtkWidget *menu, gpointer p)
  {
     GtkWidget *dialog, *grid1, *combo1, *label1, *label2, *content_area, *action_area;
    int result;

     dialog=gtk_dialog_new_with_buttons("Heatmap Above", NULL, GTK_DIALOG_MODAL, GTK_STOCK_OK, GTK_RESPONSE_OK, GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL, NULL);
     gtk_dialog_set_default_response(GTK_DIALOG(dialog), GTK_RESPONSE_OK);
     gtk_container_set_border_width(GTK_CONTAINER(dialog), 10);
     
     label1=gtk_label_new("Set Percent Above");
     label2=gtk_label_new("      Percent");  
         
     combo1=gtk_combo_box_text_new();     
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "0", "1");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "1", "5");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "2", "10");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "3", "20");
     gtk_combo_box_text_append(GTK_COMBO_BOX_TEXT(combo1), "4", "30");
     gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 2);
     
     grid1=gtk_grid_new();
     gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 3, 1);
     gtk_grid_attach(GTK_GRID(grid1), combo1, 2, 1, 1, 1);
     gtk_grid_attach(GTK_GRID(grid1), label2, 0, 1, 1, 1);        
 
     gtk_grid_set_row_spacing(GTK_GRID(grid1), 10);
     gtk_grid_set_column_spacing(GTK_GRID(grid1), 30);

     content_area = gtk_dialog_get_content_area(GTK_DIALOG(dialog));
     action_area=gtk_dialog_get_action_area(GTK_DIALOG(dialog));
     gtk_container_set_border_width(GTK_CONTAINER(content_area), 10);
     gtk_container_add(GTK_CONTAINER(content_area), grid1); 
     gtk_container_set_border_width(GTK_CONTAINER(action_area), 10);

     gtk_widget_show_all(dialog);
     result=gtk_dialog_run(GTK_DIALOG(dialog));

     if(result==GTK_RESPONSE_OK)
       {
        setting_percent=(100.0-atof(gtk_combo_box_text_get_active_text(GTK_COMBO_BOX_TEXT(combo1))))/100.0;
        int set=1;
        set_heatmap(dialog, &set);
       }

     gtk_widget_destroy(dialog);
  
  }
static void about_dialog(GtkWidget *menu, gpointer p)
  {
    GtkWidget *dialog;
    const gchar *authors[]={"C. Eric Cashon", NULL};

    dialog=gtk_about_dialog_new();
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Platemap Viewer 3d");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "With GTK+ and OpenGL");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2014 C. Eric Cashon");
    gtk_about_dialog_set_authors(GTK_ABOUT_DIALOG(dialog), authors);

    //gtk_widget_set_size_request(dialog, 400,370);
    //content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
    //gtk_widget_set_size_request(content_area, 400,300);
    //gtk_container_set_border_width(GTK_CONTAINER(content_area), 100);
    
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static void close_program()
 {
   //timer can trigger warnings when closing program.
   g_source_remove(timer_id);
   //free matrix on exit.
   gsl_matrix_free(test_data_points);
   printf("Quit Program\n");
   gtk_main_quit();
 }
static void get_data_points()
 {
   int i=0;
   int j=0;
   int k=0;
   const gsl_rng_type *T;
   gsl_rng *r;
   gsl_rng_env_setup();
   T = gsl_rng_mt19937;
   r = gsl_rng_alloc(T);

   //Global variable. Free on exit.
   if(test_data_points==NULL)
     {
       test_data_points=gsl_matrix_calloc(plates*rows, columns);
     }
   else
     {
       gsl_matrix_free(test_data_points);
       test_data_points=gsl_matrix_calloc(plates*rows, columns);      
     }
   //printf("rows %i columns %i\n", test_data_points->size1, test_data_points->size2);

   //Load test data.
   for(i=0;i<plates;i++)
      {
        for(j=0;j<rows;j++)
           {
              for(k=0;k<columns;k++)
                 {
                   //printf("plate %i row %i column %i plate_counter %i counter %i matrix_row %i matrix_column %i\n", i, j, k, (columns*j)+k, (rows*columns*i)+(columns*j)+k, i*rows+j, k);                  
                   gsl_matrix_set(test_data_points, i*rows+j, k, 10.0*gsl_rng_uniform(r));
                   //printf("%f ", gsl_matrix_get(test_data_points, i*rows+j, k));
                 }
             // printf("\n");
           }
      }
   
   //Get min and max.
   low=gsl_matrix_min(test_data_points);
   high=gsl_matrix_max(test_data_points);

 }
static void get_db_data(int iRadioButton)
 {
   int i=0;
   int j=0;
   int k=0;
   int counter=0;
   int iRecordCount=0;
   int plate_size=0;
   int n_plates=0;
   sqlite3 *cnn=NULL;
   sqlite3_stmt *stmt1=NULL;
   sqlite3_stmt *stmt2=NULL;
   sqlite3_stmt *stmt3=NULL;
   sqlite3_stmt *stmt4=NULL;
   sqlite3_stmt *stmt5=NULL;
   char sql1[]="SELECT count(data) FROM data;";
   char sql2[]="SELECT max(wells) FROM aux;";
   char sql3[]="SELECT max(plate) FROM aux;";
   char sql4[]="SELECT Data FROM data ORDER BY KeyID;";
   char sql5[]="SELECT Percent FROM data ORDER BY KeyID;";

   //Global variable. Free on exit.
   if(test_data_points==NULL)
     {
       test_data_points=gsl_matrix_calloc(plates*rows, columns);
     }
   else
     {
       printf("Reallocate Matrix\n");
       gsl_matrix_free(test_data_points);
       test_data_points=gsl_matrix_calloc(plates*rows, columns);      
     }

   sqlite3_open("VelociRaptorData.db",&cnn);
    //Get record count.
    sqlite3_prepare_v2(cnn,sql1,-1,&stmt1,0);
    sqlite3_step(stmt1);
    iRecordCount=sqlite3_column_int(stmt1, 0);
    sqlite3_finalize(stmt1);
    //Get plate_number
    sqlite3_prepare_v2(cnn,sql2,-1,&stmt2,0);
    sqlite3_step(stmt2);
    plate_size=sqlite3_column_int(stmt2, 0);
    sqlite3_finalize(stmt2);
    //Get plates.
    sqlite3_prepare_v2(cnn,sql3,-1,&stmt3,0);
    sqlite3_step(stmt3);
    n_plates=sqlite3_column_int(stmt3, 0);
    sqlite3_finalize(stmt3);

    //Do some basic error checking.
    printf("rows %i columns %i plates %i database_records %i database_plate_size %i database_plates %i\n", rows, columns, plates, iRecordCount, plate_size, n_plates);

    if(iRecordCount==0)
      {
        printf("No records returned from database.\n");
      }
    else if(plate_size==0)
      { 
        printf("Couldn't get plate size from aux table.\n");
      }
    else if(rows*columns!=plate_size)
      {
        printf("Rows times columns doesn't equal the plate size.\n");
      }
    else if(plate_size*plates!=iRecordCount)
      {
        printf("Plate size times plates doesn't equal the record count.\n");
      }
    else
      {
        if(iRadioButton==1)
           {
             sqlite3_prepare_v2(cnn,sql4,-1,&stmt4,0);
             //Load data from database.
             for(i=0;i<plates;i++)
                {
                  for(j=0;j<rows;j++)
                     {
                       for(k=0;k<columns;k++)
                          {                  
                            sqlite3_step(stmt4);                  
                            gsl_matrix_set(test_data_points, i*rows+j, k, sqlite3_column_double(stmt4, 0));
                            counter++;
                          }
                     }
                }
              sqlite3_finalize(stmt4);
            }
        if(iRadioButton==2)
           {
             sqlite3_prepare_v2(cnn,sql5,-1,&stmt5,0);
             //Load data from database.
             for(i=0;i<plates;i++)
                {
                  for(j=0;j<rows;j++)
                     {
                       for(k=0;k<columns;k++)
                          {                  
                            sqlite3_step(stmt5);                  
                            gsl_matrix_set(test_data_points, i*rows+j, k, sqlite3_column_double(stmt5, 0));
                            counter++;
                          }
                     }
                }
              sqlite3_finalize(stmt5);
            }
       }

   sqlite3_close(cnn); 
 
   //Get min and max.
   printf("Get High and Low\n");
   low=gsl_matrix_min(test_data_points);
   high=gsl_matrix_max(test_data_points);
   printf("records returned from database %i high %f low %f\n", counter, high, low);

 }
static void heatmap_rgb(double temp1, double high, double low, float rgb[])
 {
    //temp2 for checking values.
    double temp2=temp1;
    //Scale temp1 for rgb.
    temp1=(temp2-low)/((high-low));
   
    //Get colors for rgb.
    //red to green.
    if(temp1>0.50)
      {
        temp1=(temp1-0.50)*2.0;
        if(temp1>0.50)
          {
            rgb[0]=1.0;
            rgb[1]=0.0;
            rgb[2]=0.0;
            rgb[1]=rgb[1]+2*(1.0-temp1);
          }
        else
          {
            rgb[0]=1.0;
            rgb[1]=1.0;
            rgb[2]=0.0;
            rgb[0]=rgb[0]-(1.0-temp1);
          }
      }
    //green to blue.
    else
      {
        temp1=temp1*2.0;
        if(temp1>0.50)
          {
            rgb[0]=0.0;
            rgb[1]=1.0;
            rgb[2]=0.0;
            rgb[2]=rgb[2]+2*(1.0-temp1);
          }
        else
         {
            rgb[0]=0.0;
            rgb[1]=1.0;
            rgb[2]=1.0;
            rgb[1]=rgb[1]-(1.0-temp1);
         }
      }

    //printf("Scaled %f high %f low %f red %f green %f blue %f\n", temp2, high, low, rgb[0], rgb[1], rgb[2]);
 }
static void heatmap_above(double temp1, double high, double low, float rgb[])
 {
    temp1=(temp1-low)/((high-low));
    if(temp1>setting_percent)
      {
        rgb[0]=1.0;
        rgb[1]=0.0;
        rgb[2]=0.0;
      }
    else
      {
        rgb[0]=0.0;
        rgb[1]=0.0;
        rgb[2]=1.0;
      }    
 }
static void set_heatmap(GtkWidget *menu, gpointer data)
 {
    if(*(int*)data==0) 
     {
       setting_rgb=true;
       setting_above=false;
     }
    else
     {
       setting_rgb=false;
       setting_above=true;
     }
 }
static void drawGL(GtkWidget *da, gpointer data)
 {
    int i=0;
    int j=0;
    int k=0;
    double temp1=0.0;
    float rgb[]={0.0, 0.0, 0.0};
    glClearColor(0.0, 0.0, 0.0, 0.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glPushMatrix();
	
    //Rotate around x-axis
    glRotatef(ang, rotation[0], rotation[1], rotation[2]);

    //Scale
    glScalef(scaleGL, scaleGL, scaleGL);
  
    glShadeModel(GL_FLAT);
    
    //Axis lines
    glLineWidth(4.0);
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(rows, 0.0, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, columns, 0.0);
    glEnd();
	
    glBegin(GL_LINES);
    glColor3f(1.0, 0.0, 1.0);
    glVertex3f(0.0, 0.0, 0.0);
    glVertex3f(0.0, 0.0, plates);
    glEnd();

    glPointSize(8.0);
    glBegin(GL_POINTS);    
    for(i=0; i<plates; i++)
       {
         for(j=0; j<rows; j++)
            {
              for(k=0;k<columns;k++)
                 {
                   temp1=gsl_matrix_get(test_data_points, i*rows+j, k);
                   if(setting_rgb==true) heatmap_rgb(temp1, high, low, rgb);
                   if(setting_above==true) heatmap_above(temp1, high, low, rgb);
                   glColor4f(rgb[0], rgb[1], rgb[2], 1.0);
                   glVertex3f(j/3.0,k,i);
                 }
            }
       }
    glEnd();

    glPopMatrix();
    glXSwapBuffers(X_display, X_window);

 }
static void configureGL(GtkWidget *da, gpointer data)
 {
   printf("Congigure GL\n");
   DrawingWindow=gtk_widget_get_window(GTK_WIDGET(da));

   if(DrawingWindow==NULL)
     {
       printf("Couldn't get GdkWindow\n");
     }
   else
     {
       X_window=gdk_x11_window_get_xid(GDK_WINDOW(DrawingWindow));
       X_display=gdk_x11_get_default_xdisplay();
       X_visual=glXChooseVisual(X_display, 0, attributes);
       X_context=glXCreateContext(X_display, X_visual, NULL, GL_TRUE);
     }

   XGetWindowAttributes(X_display, X_window, &X_attributes);
   glXMakeCurrent(X_display, X_window, X_context);
   XMapWindow(X_display, X_window);
   printf("Viewport %i %i\n", (int)X_attributes.width, (int)X_attributes.height);
   glViewport(0, 0, X_attributes.width, X_attributes.height);
   glOrtho(-10,10,-10,10,-100,100);
   glScalef(2.5, 2.5, 2.5);

 }
static gboolean rotate(gpointer data)
 {
   if(rotate_drawing==true)
     {
       ang++;
       gtk_widget_queue_draw_area(GTK_WIDGET(da), 0, 0, 500, 550); 
     } 
   return true;
 }
static void stop_rotation(GtkWidget *da, gpointer data)
 {
   if(rotate_drawing==true) rotate_drawing=false;
   else rotate_drawing=true;
 }
static void scale_drawing(GtkRange *range,  gpointer data)
 {  
   scaleGL=2.5-gtk_range_get_value(range)/5.0;     
 }
static void rotation_axis(GtkWidget *axis, gpointer data)
 {
   if(*(int*)data==0)
     {
       rotation[0]=1.0;
       rotation[1]=0.0;
       rotation[2]=0.0;
     }
   else if(*(int*)data==1)
     {
       rotation[0]=0.0;
       rotation[1]=1.0;
       rotation[2]=0.0;
     }
   else
     {
       rotation[0]=0.0;
       rotation[1]=0.0;
       rotation[2]=1.0;
     }
 }


