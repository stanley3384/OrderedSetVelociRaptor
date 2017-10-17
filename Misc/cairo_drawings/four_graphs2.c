
/*
    Test putting more than one graph in a drawing area. 

    gcc -Wall four_graphs2.c -o four_graphs2 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
   
    C. Eric Cashon
*/

#include <gtk/gtk.h>

//The grid of rows and columns to draw the graphs in. Start with 1 graph. 
static gint graph_rows=1;
static gint graph_columns=1;
//Test array for number of tick marks on the axis. Max is 4x4 grid or 16 numbers
static gint x_ticks[]={10, 8, 12, 21, 10, 12, 15, 9, 15, 10, 12, 21, 10, 12, 15, 9};
static gint y_ticks[]={5, 4, 6, 7, 5, 4, 9, 7, 11, 4, 6, 5, 5, 4, 10, 7};
//Arrays for random data to test with.
static GArray *data_points=NULL;
//Font scaling on axis. 
static gint x_font_scale=0;
static gint y_font_scale=0;
//Dots=0 or lines=1 combo.
static gint draw_lines=1;
//Scale dots or lines.
static gint scale_dots=0;


static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data);
static void combo1_changed(GtkComboBox *combo, gpointer data);
static void combo2_changed(GtkComboBox *combo, gpointer data);
static void x_spin_changed(GtkSpinButton *spin_button, gpointer data);
static void y_spin_changed(GtkSpinButton *spin_button, gpointer data);
static void scale_dots_changed(GtkSpinButton *spin_button, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 820, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Graphs");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //Get some random numbers to test with.
    gint i=0;
    gint j=0;
    gdouble value=0;
    GRand *rand=g_rand_new();
    data_points=g_array_sized_new(FALSE, FALSE, sizeof(GArray*), 16);
    GArray *temp=NULL;
    for(i=0;i<16;i++)
      {
        temp=g_array_sized_new(FALSE, FALSE, sizeof(gdouble), x_ticks[i]);
        for(j=0;j<x_ticks[i];j++)
          {
            value=g_rand_double(rand);
            g_array_append_val(temp, value);
          }
        g_array_append_val(data_points, temp);
      }
    g_rand_free(rand);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE); 
    gtk_widget_set_vexpand(da, TRUE);  
    g_signal_connect(da, "draw", G_CALLBACK(draw_graphs), NULL);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Graph 1x1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Graph 1x4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Graph 2x2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Graph 3x3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Graph 4x4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "Graph 3x2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 6, "7", "Graph 2x3");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), da);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Draw Points");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Draw Lines");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 1);
    g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), da);

    GtkAdjustment *adjustment1=gtk_adjustment_new(0, -20, 20, 1, 0, 0);
    GtkAdjustment *adjustment2=gtk_adjustment_new(0, -20, 20, 1, 0, 0);
    GtkAdjustment *adjustment3=gtk_adjustment_new(0, -20, 20, 1, 0, 0);

    GtkWidget *x_spin_label=gtk_label_new("Scale x labels");

    GtkWidget *x_spin=gtk_spin_button_new(adjustment1, 1, 0);
    g_signal_connect(x_spin, "value-changed", G_CALLBACK(x_spin_changed), da);

    GtkWidget *y_spin_label=gtk_label_new("Scale y labels");

    GtkWidget *y_spin=gtk_spin_button_new(adjustment2, 1, 0);
    g_signal_connect(y_spin, "value-changed", G_CALLBACK(y_spin_changed), da);

    GtkWidget *dots_spin_label=gtk_label_new("Scale Dots");

    GtkWidget *dots_spin=gtk_spin_button_new(adjustment3, 1, 0);
    g_signal_connect(dots_spin, "value-changed", G_CALLBACK(scale_dots_changed), da);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), x_spin_label, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), x_spin, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), y_spin_label, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), y_spin, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dots_spin_label, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), dots_spin, 0, 7, 1, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 120);
   
    gtk_container_add(GTK_CONTAINER(window), paned1);

    gtk_widget_show_all(window);
    
    gtk_main();

    for(i=0;i<data_points->len;i++)
      {
        g_array_free(g_array_index(data_points, GArray*, i), TRUE);
      }
    g_array_free(data_points, TRUE);

    return 0;
  }
static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gint i=0;
    gint j=0;
    gint k=0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    //Some drawing variables.
    gdouble x=0.0;
    gdouble y=0.0;
    gint temp_tick=0;
    gdouble graph_width=width/graph_columns;
    gdouble graph_height=height/graph_rows;
    gdouble ratio_x=graph_width/700.0;
    gdouble ratio_y=graph_height/500.0;
    //Initialize to first tick marks in the arrays.
    gdouble x_tick=graph_width/x_ticks[0];
    gdouble y_tick=graph_height/y_ticks[0];
    //Test number for y axis. 
    gdouble test_number=500.0;
    GArray *rnd_data=NULL;

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);
     
    //Vertical lines.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 1);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            temp_tick=i*graph_columns+j; 
            x_tick=graph_width/x_ticks[temp_tick];
            for(k=0;k<x_ticks[temp_tick];k++)
              {
                x=j*graph_width+k*x_tick;
                y=i*graph_height+graph_height;
                cairo_move_to(cr, x, y);
                cairo_line_to(cr, x, y-graph_height);
                cairo_stroke(cr);
              } 
          }
      }

    //Horizontal lines.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            temp_tick=i*graph_columns+j; 
            y_tick=graph_height/y_ticks[temp_tick];
            for(k=0;k<y_ticks[temp_tick];k++)
              {
                x=j*graph_width;
                y=i*graph_height+k*y_tick;
                cairo_move_to(cr, x, y);
                cairo_line_to(cr, x+graph_width, y);
                cairo_stroke(cr);
              }
          }
      }

    //Test data in yellow.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    
    if(draw_lines==1)
      {
        cairo_set_line_width(cr, 2*ratio_x+scale_dots);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                x=graph_width*j;
                y=graph_height*i+graph_height;
                cairo_move_to(cr, x, y);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/x_ticks[temp_tick];
                y_tick=graph_height/y_ticks[temp_tick];
                rnd_data=g_array_index(data_points, GArray*, i*graph_columns+j);
                //Draw the random points.           
                for(k=0;k<rnd_data->len;k++)
                  {
                    x=j*graph_width+k*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*g_array_index(rnd_data, gdouble, k));
                    cairo_line_to(cr, x, y);
                    cairo_stroke_preserve(cr);
                  } 
              }
          }    
        cairo_stroke(cr);
      }

    if(draw_lines==0)
      {
        cairo_set_line_width(cr, 8*ratio_x+scale_dots);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                x=graph_width*j;
                y=graph_height*i+graph_height;
                cairo_move_to(cr, x, y);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/x_ticks[temp_tick];
                y_tick=graph_height/y_ticks[temp_tick];
                rnd_data=g_array_index(data_points, GArray*, i*graph_columns+j);
                //Draw the random points.           
                for(k=0;k<rnd_data->len;k++)
                  {
                    x=j*graph_width+k*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*g_array_index(rnd_data, gdouble, k));
                    cairo_move_to(cr, x, y);
                    cairo_line_to(cr, x, y);
                    cairo_stroke(cr);
                  } 
              }
          }    
      }
 

    //Number of vertical lines for each graph.
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 18*ratio_x+x_font_scale);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for(i=0;i<graph_rows;i++)
      {
        y=i*graph_height+graph_height;
        for(j=0;j<graph_columns;j++)
          {
            temp_tick=i*graph_columns+j; 
            x_tick=graph_width/x_ticks[temp_tick];
            for(k=0;k<x_ticks[temp_tick];k++)
              {
                x=j*graph_width+k*x_tick;
                gchar *string=g_strdup_printf("%i", k);
                cairo_move_to(cr, x+5.0*ratio_x, y-10.0*ratio_x);
                cairo_show_text(cr, string);
                g_free(string);
              }
          }
      }

    //Horizontal line numbers.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_set_font_size(cr, 20*ratio_y+y_font_scale);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            x=j*graph_width;
            test_number=0;
            temp_tick=i*graph_columns+j; 
            y_tick=graph_height/y_ticks[temp_tick];
            for(k=0;k<y_ticks[temp_tick];k++)
              {
                y=i*graph_height+graph_height-k*y_tick-y_tick;
                gchar *string=g_strdup_printf("%i", (gint)(test_number+=500));
                cairo_move_to(cr, x+5.0*ratio_y, y+25.0*ratio_y);
                cairo_show_text(cr, string);
                g_free(string);
              }
          }
      }

    //Draw graph blocks.
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_BUTT);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_stroke(cr);
    for(i=0;i<graph_columns;i++)
      {
        x=i*graph_width;
        cairo_move_to(cr, x, 0);
        cairo_line_to(cr, x, height);
        cairo_stroke(cr);
      }
    for(i=0;i<graph_rows;i++)
      {
        y=i*graph_height;
        cairo_move_to(cr, 0, y);
        cairo_line_to(cr, width, y);
        cairo_stroke(cr);
      }
 
    return FALSE;
  }
static void combo1_changed(GtkComboBox *combo, gpointer data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0)
      {
        graph_rows=1;
        graph_columns=1;
      }
    else if(id==1)
      {
        graph_rows=1;
        graph_columns=4;
      }
    else if(id==2)
      {
        graph_rows=2;
        graph_columns=2;
      }
    else if(id==3)
      {
        graph_rows=3;
        graph_columns=3;
      }
    else if(id==4)
      {
        graph_rows=4;
        graph_columns=4;
      }
    else if(id==5)
      {
        graph_rows=3;
        graph_columns=2;
      }
    else
      {
        graph_rows=2;
        graph_columns=3;
      }

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void combo2_changed(GtkComboBox *combo, gpointer data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0) draw_lines=0;
    else draw_lines=1;

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void x_spin_changed(GtkSpinButton *spin_button, gpointer data)
  {
    x_font_scale=(gint)gtk_spin_button_get_value(spin_button);
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void y_spin_changed(GtkSpinButton *spin_button, gpointer data)
  {
    y_font_scale=(gint)gtk_spin_button_get_value(spin_button);
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void scale_dots_changed(GtkSpinButton *spin_button, gpointer data)
  {
    scale_dots=(gint)gtk_spin_button_get_value(spin_button);
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }


