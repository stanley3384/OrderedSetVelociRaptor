
/*
    Test putting more than one graph in a drawing area. Draw dots, lines, curves or rectangles.
with or without animation. If you click on a graph in the grid layout it will swap it to the 1x1 or 
first graph position. The data sets can also be composed into one graph or decomposed into
many graphs. Keep in mind that the test data scales are different for each data set.

    gcc -Wall four_graphs2.c -o four_graphs2 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
   
    C. Eric Cashon
*/

#include<gtk/gtk.h>

struct point{
  gdouble x;
  gdouble y;
};
struct controls{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
};

//Line colors for the different graphs.
static gdouble lc[16][4]=
{
  {1.0, 1.0, 0.0, 1.0},
  {0.0, 1.0, 0.0, 1.0},
  {0.0, 1.0,  1.0, 1.0},
  {1.0,  0.0,  1.0, 1.0},
  {1.0, 0.0, 0.0, 1.0},
  {0.5, 0.5, 0.0, 1.0},
  {0.0, 0.5,  0.0, 1.0},
  {0.0,  0.5,  0.5, 1.0},
  {0.5, 0.0, 0.5, 1.0},
  {0.5, 0.0, 0.0, 1.0},
  {0.25, 1.0,  0.0, 1.0},
  {1.0,  0.25,  1.0, 1.0},
  {1.0, 0.0, 0.25, 1.0},
  {0.5, 0.5, 0.25, 1.0},
  {0.0, 0.5,  0.25, 1.0},
  {0.25,  0.5,  0.5, 1.0}
};

//The grid of rows and columns to draw the graphs in. Start with 1 graph. 
static gint graph_rows=1;
static gint graph_columns=1;
//Test array for number of tick marks on the axis. Max is 4x4 grid or 16 numbers
static gint x_ticks[]={10, 8, 12, 21, 10, 12, 15, 9, 15, 10, 12, 21, 10, 12, 15, 9};
static gint y_ticks[]={10, 4, 6, 7, 5, 4, 9, 7, 11, 4, 6, 5, 5, 4, 10, 7};
//Test numbers for the x and y axis scale or increase at each tick. All starting at 0.
static gdouble test_increment_x=5.0; 
static gdouble y_max[]={500.0, 1000.0, 100.0, 700.0, 10000.0, 750.0, 25.0, 125.0, 8000.0, 380.0, 450.0, 3000.0, 10.0, 40.0, 770.0, 8900.0};
//Arrays for random data to test with.
static GArray *data_points=NULL;
//Font scaling on axis. 
static gint x_font_scale=0;
static gint y_font_scale=0;
//Points=0, lines=1 or smooth=2 combo.
static gint draw_lines=0;
//Scale dots and lines.
static gint scale_dots=0;
//For the timer and random number generator.
static int timer_id=0;
static GRand *rand=NULL;
//The number of data sets to compose in the drawing.
static gint compose=0;
//For blocking a combo signal
static gint combo3_id=0;
//Output draw times.
static gboolean draw_time=FALSE;

static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data);
static void combo1_changed(GtkComboBox *combo, gpointer *data);
static void combo2_changed(GtkComboBox *combo, gpointer data);
static void combo3_changed(GtkComboBox *combo, gpointer *data);
static void x_spin_changed(GtkSpinButton *spin_button, gpointer data);
static void y_spin_changed(GtkSpinButton *spin_button, gpointer data);
static void scale_dots_changed(GtkSpinButton *spin_button, gpointer data);
static void button1_clicked(GtkToggleButton *button, GtkWidget *widgets[]);
static void button2_clicked(GtkToggleButton *button, gpointer data);
static gboolean animate_graphs(GtkWidget *widgets[]);
static gboolean click_drawing_area(GtkWidget *widget, GdkEvent *event, gpointer data);
//Bezier control points from coordinates for smoothing.
static GArray* control_points_from_coords2(const GArray *dataPoints);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Graphs");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    //Get some random numbers to test with.
    gint i=0;
    gint j=0;
    struct point pt;
    rand=g_rand_new();
    data_points=g_array_sized_new(FALSE, FALSE, sizeof(GArray*), 16);
    GArray *temp=NULL;
    for(i=0;i<16;i++)
      {
        //Test array size based on x tick marks.
        temp=g_array_sized_new(FALSE, FALSE, sizeof(struct point), x_ticks[i]);
        for(j=0;j<x_ticks[i];j++)
          {
            pt.x=(gdouble)j;
            pt.y=g_rand_double(rand);
            g_array_append_val(temp, pt);
          }
        g_array_append_val(data_points, temp);
      }

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK);
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

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Draw Points");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Draw Lines");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Draw Smooth");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Draw Rectangles");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);

    //Just compose 8. The graph gets very busy with many data sets. 
    GtkWidget *combo3=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "Compose 1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "Compose 2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "Compose 3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 3, "4", "Compose 4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 4, "5", "Compose 5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 5, "6", "Compose 6");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 6, "7", "Compose 7");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 7, "8", "Compose 8");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);

    gpointer data[]={da, combo1, combo3};
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), data);
    g_signal_connect(da, "button-press-event", G_CALLBACK(click_drawing_area), combo1);
    g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), da);
    combo3_id=g_signal_connect(combo3, "changed", G_CALLBACK(combo3_changed), data);

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

    GtkWidget *button1=gtk_toggle_button_new_with_label("Animate");
    GtkWidget *widgets[]={button1, da};
    g_signal_connect(button1, "toggled", G_CALLBACK(button1_clicked), widgets);

    GtkWidget *button2=gtk_toggle_button_new_with_label("Draw Time");
    g_signal_connect(button2, "toggled", G_CALLBACK(button2_clicked), NULL);

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
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 8, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 0, 9, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 10, 1, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 150);
   
    gtk_container_add(GTK_CONTAINER(window), paned1);

    gtk_widget_show_all(window);
    
    gtk_main();

    for(i=0;i<data_points->len;i++)
      {
        g_array_free(g_array_index(data_points, GArray*, i), TRUE);
      }
    g_array_free(data_points, TRUE);
    g_rand_free(rand);

    return 0;
  }
static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    GTimer *timer=NULL;
    if(draw_time==TRUE) timer=g_timer_new();
    gint i=0;
    gint j=0;
    gint k=0;
    gint h=0;
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
    GArray *rnd_data=NULL;

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);

    //Draw grid lines before points, lines and curve and after for histogram rectangles.
    if(draw_lines!=3)
      {
        //Vertical lines.
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
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
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
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
      }
         
    //Draw points, lines, curves and rectangles.
    struct point pt;
    //Draw points. 
    if(draw_lines==0)
      {
        cairo_set_line_width(cr, 8*ratio_x+scale_dots);
        cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the points in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                y=graph_height*i+graph_height;
                cairo_move_to(cr, x, y);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/x_ticks[temp_tick];
                y_tick=graph_height/y_ticks[temp_tick];
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[id][3]);
                    rnd_data=g_array_index(data_points, GArray*, id);           
                    for(k=0;k<rnd_data->len;k++)
                      {
                        pt=g_array_index(rnd_data, struct point, k);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);
                        cairo_line_to(cr, x, y);
                        cairo_stroke(cr);
                      }
                  }
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(data_points, GArray*, h);
                        x_tick=graph_width/x_ticks[h];           
                        for(k=0;k<rnd_data->len;k++)
                          {
                            pt=g_array_index(rnd_data, struct point, k);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            cairo_move_to(cr, x, y);
                            cairo_line_to(cr, x, y);
                            cairo_stroke(cr);
                          }
                      }
                  } 
                cairo_restore(cr);
              }
          }    
      } 
    //Draw lines between points.  
    else if(draw_lines==1)
      {
        cairo_set_line_width(cr, 4*ratio_x+scale_dots);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the lines in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/x_ticks[temp_tick];
                y_tick=graph_height/y_ticks[temp_tick];
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                    rnd_data=g_array_index(data_points, GArray*, id);
                    pt=g_array_index(rnd_data, struct point, 0);
                    x=j*graph_width+pt.x*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*pt.y);
                    cairo_move_to(cr, x, y);           
                    for(k=1;k<rnd_data->len;k++)
                      {
                        pt=g_array_index(rnd_data, struct point, k);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_line_to(cr, x, y);
                        cairo_stroke(cr);
                        cairo_move_to(cr, x, y);
                      }
                  } 
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(data_points, GArray*, h);
                        x_tick=graph_width/x_ticks[h];
                        pt=g_array_index(rnd_data, struct point, 0);
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);           
                        for(k=1;k<rnd_data->len;k++)
                          {
                            pt=g_array_index(rnd_data, struct point, k);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            cairo_line_to(cr, x, y);
                            cairo_stroke(cr);
                            cairo_move_to(cr, x, y);
                          } 
                      }
                  }
                cairo_restore(cr);
              }
          }    
      }
    //Draw curves between points.
    else if(draw_lines==2)
      {
        struct controls c1;
        gdouble ct1=0;
        gdouble ct2=0;
        gdouble ct3=0;
        gdouble ct4=0;
        cairo_set_line_width(cr, 4*ratio_x+scale_dots);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the curve in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/x_ticks[temp_tick];
                y_tick=graph_height/y_ticks[temp_tick];
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                    rnd_data=g_array_index(data_points, GArray*, id);
                    GArray *bezier_pts=control_points_from_coords2(rnd_data);
                    pt=g_array_index(rnd_data, struct point, 0);
                    x=j*graph_width+pt.x*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*pt.y);
                    cairo_move_to(cr, x, y);       
                    for(k=1;k<rnd_data->len;k++)
                      {
                        pt=g_array_index(rnd_data, struct point, k);
                        c1=g_array_index(bezier_pts, struct controls, k-1);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        ct1=j*graph_width+c1.x1*x_tick+x_tick;
                        ct2=i*graph_height+graph_height-(graph_height*c1.y1);
                        ct3=j*graph_width+c1.x2*x_tick+x_tick;
                        ct4=i*graph_height+graph_height-(graph_height*c1.y2);
                        cairo_curve_to(cr, ct1, ct2, ct3, ct4, x, y);
                        cairo_stroke(cr);
                        cairo_move_to(cr, x, y);
                      } 
                    g_array_free(bezier_pts, TRUE);
                  }
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(data_points, GArray*, h);
                        x_tick=graph_width/x_ticks[h];
                        GArray *bezier_pts=control_points_from_coords2(rnd_data);
                        pt=g_array_index(rnd_data, struct point, 0);
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);       
                        for(k=1;k<rnd_data->len;k++)
                          {
                            pt=g_array_index(rnd_data, struct point, k);
                            c1=g_array_index(bezier_pts, struct controls, k-1);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            ct1=j*graph_width+c1.x1*x_tick+x_tick;
                            ct2=i*graph_height+graph_height-(graph_height*c1.y1);
                            ct3=j*graph_width+c1.x2*x_tick+x_tick;
                            ct4=i*graph_height+graph_height-(graph_height*c1.y2);
                            cairo_curve_to(cr, ct1, ct2, ct3, ct4, x, y);
                            cairo_stroke(cr);
                            cairo_move_to(cr, x, y);
                          } 
                        g_array_free(bezier_pts, TRUE);
                      }
                  }
                cairo_restore(cr);
              }
          }    
      }
    //Draw histogram rectangles.
    else
      {
        cairo_set_line_width(cr, 4*ratio_x+scale_dots);
        cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                //Clip rectangles to keep the lines in bounds.
                cairo_save(cr);
                x=graph_width*j;
                y=graph_height*i;
                cairo_rectangle(cr, x, y, graph_width, graph_height);
                cairo_clip(cr);
                temp_tick=i*graph_columns+j; 
                x_tick=graph_width/x_ticks[temp_tick];
                y_tick=graph_height/y_ticks[temp_tick];
                if(compose==0)
                  {
                    gint id=i*graph_columns+j;
                    cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                    rnd_data=g_array_index(data_points, GArray*, id);
                    pt=g_array_index(rnd_data, struct point, 0);
                    x=j*graph_width+pt.x*x_tick+x_tick;
                    y=i*graph_height+graph_height-(graph_height*pt.y);
                    cairo_move_to(cr, x, y);           
                    for(k=1;k<rnd_data->len;k++)
                      {
                        cairo_rectangle(cr, x, y, x_tick, graph_height);
                        cairo_fill(cr);
                        cairo_set_source_rgba(cr, 0.5, 0.8, 1.0, 1.0);
                        cairo_rectangle(cr, x, y, x_tick, graph_height);
                        cairo_stroke(cr);
                        cairo_set_source_rgba(cr, lc[id][0], lc[id][1], lc[id][2], lc[h][3]);
                        cairo_fill(cr);
                        pt=g_array_index(rnd_data, struct point, k);
                        //k=pt.x for testing.
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);
                      }
                  } 
                else
                  {
                    for(h=0;h<compose+1;h++)
                      {
                        cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                        rnd_data=g_array_index(data_points, GArray*, h);
                        x_tick=graph_width/x_ticks[h];
                        pt=g_array_index(rnd_data, struct point, 0);
                        x=j*graph_width+pt.x*x_tick+x_tick;
                        y=i*graph_height+graph_height-(graph_height*pt.y);
                        cairo_move_to(cr, x, y);           
                        for(k=1;k<rnd_data->len;k++)
                          {
                            cairo_rectangle(cr, x, y, x_tick, graph_height);
                            cairo_fill(cr);
                            cairo_set_source_rgba(cr, 0.5, 0.8, 1.0, 1.0);
                            cairo_rectangle(cr, x, y, x_tick, graph_height);
                            cairo_stroke(cr);
                            cairo_set_source_rgba(cr, lc[h][0], lc[h][1], lc[h][2], lc[h][3]);
                            cairo_fill(cr);
                            pt=g_array_index(rnd_data, struct point, k);
                            //k=pt.x for testing.
                            x=j*graph_width+pt.x*x_tick+x_tick;
                            y=i*graph_height+graph_height-(graph_height*pt.y);
                            cairo_move_to(cr, x, y);
                          } 
                      }
                  }
                cairo_restore(cr);
              }
          }    
      }

    //Draw histogram lines after drawing the data.
    if(draw_lines==3)
      {
        //Vertical lines.
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
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
        cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
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
      }
   
    //The x-axis numbers.
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 18*ratio_x+x_font_scale);
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            cairo_save(cr);
            x=graph_width*j;
            y=graph_height*i;
            cairo_rectangle(cr, x, y, graph_width, graph_height);
            cairo_clip(cr);
            y=i*graph_height+graph_height;
            temp_tick=i*graph_columns+j; 
            x_tick=graph_width/x_ticks[temp_tick];
            for(k=0;k<x_ticks[temp_tick];k++)
              {
                x=j*graph_width+k*x_tick;
                gchar *string=g_strdup_printf("%i", (gint)(test_increment_x*k));
                cairo_move_to(cr, x+8.0*ratio_x, y-10.0*ratio_x);
                cairo_show_text(cr, string);
                g_free(string);
              }
            cairo_restore(cr);
          }
      }

    //The y-axis numbers.
    gint len=0;
    gdouble y_value=0;
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_set_font_size(cr, 20*ratio_y+y_font_scale);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            cairo_save(cr);
            x=graph_width*j;
            y=graph_height*i;
            cairo_rectangle(cr, x, y, graph_width, graph_height);
            cairo_clip(cr);
            x=j*graph_width;
            temp_tick=i*graph_columns+j; 
            y_tick=graph_height/y_ticks[temp_tick];
            y_value=y_max[i*graph_columns+j]/y_ticks[temp_tick];
            len=y_ticks[temp_tick]+1;
            for(k=0;k<len;k++)
              {
                y=i*graph_height+graph_height-k*y_tick;
                gchar *string=g_strdup_printf("%.2f", (y_value*k));
                cairo_move_to(cr, x+8.0*ratio_y, y+25.0*ratio_y);
                cairo_show_text(cr, string);
                g_free(string);
              }
            cairo_restore(cr);
          }
      }

    //Draw graph blocks.
    cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 1.0);
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
 
    if(draw_time==TRUE)
      {
        g_print("Draw Time %f\n", g_timer_elapsed(timer, NULL));
        g_timer_destroy(timer);
      }
    return FALSE;
  }
static void combo1_changed(GtkComboBox *combo, gpointer *data)
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

    compose=0;
    g_signal_handler_block(GTK_COMBO_BOX(data[2]), combo3_id);
    gtk_combo_box_set_active(GTK_COMBO_BOX(data[2]), 0);
    g_signal_handler_unblock(GTK_COMBO_BOX(data[2]), combo3_id);

    gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  }
static void combo2_changed(GtkComboBox *combo, gpointer data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0) draw_lines=0;
    else if(id==1) draw_lines=1;
    else if(id==2) draw_lines=2;
    else draw_lines=3;

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void combo3_changed(GtkComboBox *combo, gpointer *data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0) compose=0;
    else if(id==1) compose=1;
    else if(id==2) compose=2;
    else if(id==3) compose=3;
    else if(id==4) compose=4;
    else if(id==5) compose=5;
    else if(id==6) compose=6;
    else compose=7;
    
    graph_rows=1;
    graph_columns=1;
    gtk_combo_box_set_active(GTK_COMBO_BOX(data[1]), 0);
      
    gtk_widget_queue_draw(GTK_WIDGET(data[0]));
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
static void button1_clicked(GtkToggleButton *button, GtkWidget *widgets[])
  {
    if(gtk_toggle_button_get_active(button)&&timer_id==0)
      {
        timer_id=g_timeout_add(500, (GSourceFunc)animate_graphs, widgets);
      }
  }
static void button2_clicked(GtkToggleButton *button, gpointer data)
  {
    if(gtk_toggle_button_get_active(button)) draw_time=TRUE;
    else draw_time=FALSE;
  }
static gboolean animate_graphs(GtkWidget *widgets[])
 {
   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[0])))
     {
       gint i=0;
       gint j=0;
       struct point pt;
       GArray *temp=NULL;
       struct point *x_point=NULL;
       gint len=0;
       //Remove last point and add beginning point to each data set.
       for(i=0;i<data_points->len;i++)
         {
           temp=g_array_index(data_points, GArray*, i);
           len=temp->len-1;
           g_array_remove_index_fast(temp, len);
           pt.x=0.0;
           pt.y=g_rand_double(rand);
           g_array_prepend_val(temp, pt);
           for(j=1;j<temp->len;j++)
             {
               x_point=&g_array_index(temp, struct point, j);
               x_point->x=(gdouble)j;
             }
          }
       gtk_widget_queue_draw(widgets[1]);
       return TRUE;
     }
   else
     {
       timer_id=0;
       gtk_widget_queue_draw(widgets[1]);
       return FALSE;
     }
 }
static gboolean click_drawing_area(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gint i=0;
    gint j=0;
    gint width=gtk_widget_get_allocated_width(widget);
    gint height=gtk_widget_get_allocated_height(widget);
    gint graph_width=width/graph_columns;
    gint graph_height=height/graph_rows;
    gint top_x=0;
    gint top_y=0;
    gint bottom_x=0;
    gint bottom_y=0;

    if(graph_rows!=1&&graph_columns!=1)
      {
        for(i=0;i<graph_rows;i++)
          {
            for(j=0;j<graph_columns;j++)
              {
                top_x=j*graph_width;
                top_y=i*graph_height;
                bottom_x=top_x+graph_width;
                bottom_y=top_y+graph_height;
                if(event->button.x>top_x&&event->button.y>top_y&&event->button.x<bottom_x&&event->button.y<bottom_y)
                  {
                    //Swap array values and show the graph by itself.
                    GArray **temp1=NULL;
                    GArray **temp2=NULL;
                    GArray *temp=NULL;
                    gdouble d_temp=0;
                    gint i_temp=0;
                    gint index=i*graph_columns+j;
                    temp1=&g_array_index(data_points, GArray*, 0);
                    temp2=&g_array_index(data_points, GArray*, index);
                    temp=*temp1;
                    *temp1=*temp2;
                    *temp2=temp;
                    //Swap x tick values.
                    i_temp=x_ticks[0];
                    x_ticks[0]=x_ticks[index];
                    x_ticks[index]=i_temp;
                    //Swap y tick values.
                    i_temp=y_ticks[0];
                    y_ticks[0]=y_ticks[index];
                    y_ticks[index]=i_temp;
                    //Swap y label values.
                    d_temp=y_max[0];
                    y_max[0]=y_max[index];
                    y_max[index]=d_temp;
                    //Swap line colors.
                    d_temp=lc[0][0];
                    lc[0][0]=lc[index][0];
                    lc[index][0]=d_temp;
                    d_temp=lc[0][1];
                    lc[0][1]=lc[index][1];
                    lc[index][1]=d_temp;
                    d_temp=lc[0][2];
                    lc[0][2]=lc[index][2];
                    lc[index][2]=d_temp;
                    d_temp=lc[0][3];
                    lc[0][3]=lc[index][3];
                    lc[index][3]=d_temp;
                    //Show single graph window.
                    gtk_combo_box_set_active(GTK_COMBO_BOX(data), 0);
                    graph_rows=1;
                    graph_columns=1;
                    gtk_widget_queue_draw(widget);
                    return TRUE;
                  }
              }
          }
      }
    
    return TRUE;
  }
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=0;
    if(dataPoints!=NULL) count=dataPoints->len-1;
    gdouble *fCP=NULL;
    gdouble *sCP=NULL;

    if(count>0)
      {
        fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
      }
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count<1||dataPoints==NULL)
      {
        //Return NULL.
        controlPoints=NULL;
        g_warning("Can't get control points from coordinates. NULL returned.\n");
      }
    else if(count==1)
      {
        struct point P0=g_array_index(dataPoints, struct point, 0);
        struct point P3=g_array_index(dataPoints, struct point, 1);

        //Calculate First Control Point
        //3P1 = 2P0 + P3
        struct point P1;
        P1.x=(2.0*P0.x+P3.x)/3.0;
        P1.y=(2.0*P0.y+P3.y)/3.0;

        *(fCP)=P1.x;
        *(fCP+1)=P1.y;

        //Calculate second Control Point
        //P2 = 2P1 - P0
        struct point P2;
        P2.x=(2.0*P1.x-P0.x);
        P2.y=(2.0*P1.y-P0.x);

        *(sCP)=P2.x;
        *(sCP+1)=P2.y;      
      }
    else
      {
        gdouble *rhs=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        gdouble *a=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *b=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *c=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble rhsValueX=0;
        gdouble rhsValueY=0;
        struct point P0;
        struct point P3;        
        gdouble m=0;
        gdouble b1=0;
        gdouble r2x=0;
        gdouble r2y=0;
        gdouble P1_x=0;
        gdouble P1_y=0;
   
        for(i=0;i<count;i++)
          {
            P0=g_array_index(dataPoints, struct point, i);
            P3=g_array_index(dataPoints, struct point, i+1);

            if(i==0)
              {
                *(a)=0.0;
                *(b)=2.0;
                *(c)=1.0;

                //rhs for first segment
                rhsValueX=P0.x+2.0*P3.x;
                rhsValueY=P0.y+2.0*P3.y;
              }
            else if(i==count-1)
              {
                *(a+i)=2.0;
                *(b+i)=7.0;
                *(c+i)=0.0;

                //rhs for last segment
                rhsValueX=8.0*P0.x+P3.x;
                rhsValueY=8.0*P0.y+P3.y;
              }
            else
              {
                *(a+i)=1.0;
                *(b+i)=4.0;
                *(c+i)=1.0;

                rhsValueX=4.0*P0.x+2.0*P3.x;
                rhsValueY=4.0*P0.y+2.0*P3.y;
              }
            *(rhs+i*2)=rhsValueX;
            *(rhs+i*2+1)=rhsValueY;
          }

        //Solve Ax=B. Use Tridiagonal matrix algorithm a.k.a Thomas Algorithm
        for(i=1;i<count;i++)
          {
            m=(*(a+i))/(*(b+i-1));

            b1=(*(b+i))-m*(*(c+i-1));
            *(b+i)=b1;

            r2x=(*(rhs+i*2))-m*(*(rhs+(i-1)*2));
            r2y=(*(rhs+i*2+1))-m*(*(rhs+(i-1)*2+1));

            *(rhs+i*2)=r2x;
            *(rhs+i*2+1)=r2y;
          }

        //Get First Control Points
        
        //Last control Point
        gdouble lastControlPointX=(*(rhs+2*count-2))/(*(b+count-1));
        gdouble lastControlPointY=(*(rhs+2*count-1))/(*(b+count-1));

        *(fCP+2*count-2)=lastControlPointX;
        *(fCP+2*count-1)=lastControlPointY;

        gdouble controlPointX=0;
        gdouble controlPointY=0;

        for(i=count-2;i>=0;--i)
          {
            controlPointX=(*(rhs+i*2)-(*(c+i))*(*(fCP+(i+1)*2)))/(*(b+i));
            controlPointY=(*(rhs+i*2+1)-(*(c+i))*(*(fCP+(i+1)*2+1)))/(*(b+i));

             *(fCP+i*2)=controlPointX;
             *(fCP+i*2+1)=controlPointY; 
          }

        //Compute second Control Points from first.
        for(i=0;i<count;i++)
          {
            if(i==count-1)
              {
                P3=g_array_index(dataPoints, struct point, i+1);
                P1_x=(*(fCP+i*2));
                P1_y=(*(fCP+i*2+1));

                controlPointX=(P3.x+P1_x)/2.0;
                controlPointY=(P3.y+P1_y)/2.0;

                *(sCP+count*2-2)=controlPointX;
                *(sCP+count*2-1)=controlPointY;
              }
            else
              {
                P3=g_array_index(dataPoints, struct point, i+1);                
                P1_x=(*(fCP+(i+1)*2));
                P1_y=(*(fCP+(i+1)*2+1));

                controlPointX=2.0*P3.x-P1_x;
                controlPointY=2.0*P3.y-P1_y;

                *(sCP+i*2)=controlPointX;
                *(sCP+i*2+1)=controlPointY;
              }

          }

        controlPoints=g_array_new(FALSE, FALSE, sizeof(struct controls));
        struct controls cp;
        for(i=0;i<count;i++)
          {
            cp.x1=(*(fCP+i*2));
            cp.y1=(*(fCP+i*2+1));
            cp.x2=(*(sCP+i*2));
            cp.y2=(*(sCP+i*2+1));
            g_array_append_val(controlPoints, cp);
          }

        g_free(rhs);
        g_free(a);
        g_free(b);
        g_free(c);
     }

    if(fCP!=NULL) g_free(fCP);
    if(sCP!=NULL) g_free(sCP);

    return controlPoints;
  }


