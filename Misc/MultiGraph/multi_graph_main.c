
/*

    This is moving the graphing in four_graph2.c into a widget. The data array points are
initialized to 1 on all the graphs. When the graphs are animated, random values are fed to
the graphs. It will draw up to 16 standardized time series graphs individually or compose the
graphs into one. It can draw points, lines, curves or rectangles.   

    gcc -Wall multi_graph.c multi_graph_main.c -o multi_graph `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "multi_graph.h"

static guint combo3_id=0;
//For the timer and random number generator.
static guint timer_id=0;
static GRand *rand=NULL;

static void combo1_changed(GtkComboBox *combo, gpointer *data);
static void combo2_changed(GtkComboBox *combo, gpointer data);
static void combo3_changed(GtkComboBox *combo, gpointer *data);
static void x_spin_changed(GtkSpinButton *spin_button, gpointer data);
static void y_spin_changed(GtkSpinButton *spin_button, gpointer data);
static void scale_dots_changed(GtkSpinButton *spin_button, gpointer data);
static void button1_clicked(GtkToggleButton *button, GtkWidget *widgets[]);
static gboolean animate_graphs(GtkWidget *widgets[]);
static gboolean click_drawing_area(GtkWidget *widget, GdkEvent *event, gpointer data);
static void swap_button_clicked(GtkWidget *widget, GtkWidget *swap_widgets[]);
static void close_program(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Multi Graph");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 500);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(close_program), NULL);

    rand=g_rand_new();

    GtkWidget *graph1=multi_graph_new();
    gtk_widget_add_events(graph1, GDK_BUTTON_PRESS_MASK);
    /*
      Test some multi graph functions. The set_points also set the x_ticks. 
    */
    multi_graph_set_points(MULTI_GRAPH(graph1), 1, 40);
    multi_graph_set_tick_increment_x(MULTI_GRAPH(graph1), 1, 5);
    multi_graph_set_points(MULTI_GRAPH(graph1), 2, 80);
    multi_graph_set_tick_increment_x(MULTI_GRAPH(graph1), 2, 10);
    //multi_graph_set_y_ticks(MULTI_GRAPH(graph1), 1, 10);
    //multi_graph_set_y_max(MULTI_GRAPH(graph1), 1, 200.0);
    //multi_graph_set_background_color(MULTI_GRAPH(graph1), "rgba(255, 255, 255, 1.0)");
    //multi_graph_set_font_color(MULTI_GRAPH(graph1), "rgba(0, 0, 0, 1.0)");
    //multi_graph_set_tick_color(MULTI_GRAPH(graph1), "rgba(0, 0, 255, 1.0)");
    //multi_graph_set_grid_color(MULTI_GRAPH(graph1), "rgba(200, 200, 200, 1.0)");
    //multi_graph_set_line_color(MULTI_GRAPH(graph1), 3, "rgba(255, 128, 0, 1.0)");
    gtk_widget_set_hexpand(graph1, TRUE);
    gtk_widget_set_vexpand(graph1, TRUE);

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

    gpointer data[]={graph1, combo1, combo3};
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), data);
    g_signal_connect(graph1, "button-press-event", G_CALLBACK(click_drawing_area), combo1);
    g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), graph1);
    combo3_id=g_signal_connect(combo3, "changed", G_CALLBACK(combo3_changed), data);

    //Scale adjustments.
    GtkAdjustment *adjustment1=gtk_adjustment_new(0, -20, 20, 1, 0, 0);
    GtkAdjustment *adjustment2=gtk_adjustment_new(0, -20, 20, 1, 0, 0);
    GtkAdjustment *adjustment3=gtk_adjustment_new(0, -20, 20, 1, 0, 0);

    //Swap graph adjustments.
    GtkAdjustment *adjustment4=gtk_adjustment_new(1, 1, 16, 1, 0, 0);
    GtkAdjustment *adjustment5=gtk_adjustment_new(2, 1, 16, 1, 0, 0);

    //Scale.
    GtkWidget *x_spin_label=gtk_label_new("Scale x labels");

    GtkWidget *x_spin=gtk_spin_button_new(adjustment1, 1, 0);
    g_signal_connect(x_spin, "value-changed", G_CALLBACK(x_spin_changed), graph1);

    GtkWidget *y_spin_label=gtk_label_new("Scale y labels");

    GtkWidget *y_spin=gtk_spin_button_new(adjustment2, 1, 0);
    g_signal_connect(y_spin, "value-changed", G_CALLBACK(y_spin_changed), graph1);

    GtkWidget *dots_spin_label=gtk_label_new("Scale Dots");

    GtkWidget *dots_spin=gtk_spin_button_new(adjustment3, 1, 0);
    g_signal_connect(dots_spin, "value-changed", G_CALLBACK(scale_dots_changed), graph1);

    //Swap graphs.
    GtkWidget *swap1=gtk_spin_button_new(adjustment4, 1, 0);

    GtkWidget *swap2=gtk_spin_button_new(adjustment5, 1, 0);

    GtkWidget *swap_button=gtk_button_new_with_label("Swap Graphs");
    GtkWidget *swap_widgets[]={swap1, swap2, graph1};
    g_signal_connect(swap_button, "clicked", G_CALLBACK(swap_button_clicked), swap_widgets);  

    GtkWidget *button1=gtk_toggle_button_new_with_label("Animate");
    GtkWidget *widgets[]={button1, graph1};
    g_signal_connect(button1, "toggled", G_CALLBACK(button1_clicked), widgets);

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
    gtk_grid_attach(GTK_GRID(grid), swap1, 0, 10, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), swap2, 0, 11, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), swap_button, 0, 12, 1, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), graph1, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 150);
   
    gtk_container_add(GTK_CONTAINER(window), paned1);

    gtk_widget_show_all(window);
    
    gtk_main();

    g_rand_free(rand);

    return 0;
  }
static void combo1_changed(GtkComboBox *combo, gpointer *data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0)
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 1);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 1);
      }
    else if(id==1)
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 1);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 4);
      }
    else if(id==2)
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 2);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 2);
      }
    else if(id==3)
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 3);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 3);
      }
    else if(id==4)
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 4);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 4);
      }
    else if(id==5)
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 3);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 2);
      }
    else
      {
        multi_graph_set_rows(MULTI_GRAPH(data[0]), 2);
        multi_graph_set_columns(MULTI_GRAPH(data[0]), 3);
      }

    multi_graph_set_compose(MULTI_GRAPH(data[0]), 0);
    g_signal_handler_block(GTK_COMBO_BOX(data[2]), combo3_id);
    gtk_combo_box_set_active(GTK_COMBO_BOX(data[2]), 0);
    g_signal_handler_unblock(GTK_COMBO_BOX(data[2]), combo3_id);

    gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  }
static void combo2_changed(GtkComboBox *combo, gpointer data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0) multi_graph_set_draw_lines(MULTI_GRAPH(data), 0);
    else if(id==1) multi_graph_set_draw_lines(MULTI_GRAPH(data), 1);
    else if(id==2) multi_graph_set_draw_lines(MULTI_GRAPH(data), 2);
    else multi_graph_set_draw_lines(MULTI_GRAPH(data), 3);

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void combo3_changed(GtkComboBox *combo, gpointer *data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0) multi_graph_set_compose(MULTI_GRAPH(data[0]), 0);
    else if(id==1) multi_graph_set_compose(MULTI_GRAPH(data[0]), 1);
    else if(id==2) multi_graph_set_compose(MULTI_GRAPH(data[0]), 2);
    else if(id==3) multi_graph_set_compose(MULTI_GRAPH(data[0]), 3);
    else if(id==4) multi_graph_set_compose(MULTI_GRAPH(data[0]), 4);
    else if(id==5) multi_graph_set_compose(MULTI_GRAPH(data[0]), 5);
    else if(id==6) multi_graph_set_compose(MULTI_GRAPH(data[0]), 6);
    else multi_graph_set_compose(MULTI_GRAPH(data[0]), 7);
    
    multi_graph_set_rows(MULTI_GRAPH(data[0]), 1);
    multi_graph_set_columns(MULTI_GRAPH(data[0]), 1);
    gtk_combo_box_set_active(GTK_COMBO_BOX(data[1]), 0);
      
    gtk_widget_queue_draw(GTK_WIDGET(data[0]));
  }
static void x_spin_changed(GtkSpinButton *spin_button, gpointer data)
  {
    multi_graph_set_x_font_scale(MULTI_GRAPH(data), gtk_spin_button_get_value_as_int(spin_button));
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void y_spin_changed(GtkSpinButton *spin_button, gpointer data)
  {
    multi_graph_set_y_font_scale(MULTI_GRAPH(data), gtk_spin_button_get_value_as_int(spin_button));
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void scale_dots_changed(GtkSpinButton *spin_button, gpointer data)
  {
    multi_graph_set_scale_dots(MULTI_GRAPH(data), gtk_spin_button_get_value_as_int(spin_button));
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void button1_clicked(GtkToggleButton *button, GtkWidget *widgets[])
  {
    if(gtk_toggle_button_get_active(button)&&timer_id==0)
      {
        timer_id=g_timeout_add(500, (GSourceFunc)animate_graphs, widgets);
      }
  }
static gboolean animate_graphs(GtkWidget *widgets[])
 {
   if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(widgets[0])))
     {
       gint i=0; 
       /*      
         Remove last point and add beginning point to each data set. The time series data feed
         for the graphs. Update all the graphs for testing.
       */
       for(i=0;i<16;i++)
         {
           /*
             Feed a standardized y number between 0 and 1. It is multiplied by the y_max value
             for graphing.
           */
           multi_graph_feed_point(MULTI_GRAPH(widgets[1]), i, 0.0, g_rand_double(rand));          
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
    gint graph_rows=multi_graph_get_graph_rows(MULTI_GRAPH(widget));
    gint graph_columns=multi_graph_get_graph_columns(MULTI_GRAPH(widget));
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
                    gint index=i*graph_columns+j;
                    multi_graph_swap_graphs(MULTI_GRAPH(widget), 0, index);
                    gtk_combo_box_set_active(GTK_COMBO_BOX(data), 0);
                    multi_graph_set_rows(MULTI_GRAPH(widget), 1);
                    multi_graph_set_columns(MULTI_GRAPH(widget), 1);
                    gtk_widget_queue_draw(widget);
                    return TRUE;
                  }
              }
          }
      }
    
    return TRUE;
  }
static void swap_button_clicked(GtkWidget *widget, GtkWidget *swap_widgets[])
  {
    gint id1=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(swap_widgets[0]))-1;
    gint id2=gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(swap_widgets[1]))-1;
    multi_graph_swap_graphs(MULTI_GRAPH(swap_widgets[2]), id1, id2);
    gtk_widget_queue_draw(swap_widgets[2]);
  }
static void close_program(GtkWidget *widget, gpointer data)
  {
    //Check if timer is still active when closing the program.
    if(timer_id!=0) g_source_remove(timer_id);
    gtk_main_quit();
  }
