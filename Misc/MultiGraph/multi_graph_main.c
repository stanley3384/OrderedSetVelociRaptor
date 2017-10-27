
/*

    This is the start of moving four_graph2.c into a multi graph widget. It works but has a ways to go.

    gcc -Wall multi_graph.c multi_graph_main.c -o multi_graph `pkg-config gtk+-3.0 --cflags --libs`

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon

*/

#include<gtk/gtk.h>
#include "multi_graph.h"

static void combo1_changed(GtkComboBox *combo, gpointer *data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Multi Graph");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 500);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *graph1=multi_graph_new();
    //multi_graph_set_background_color(MULTI_GRAPH(graph1), "rgba(0, 0, 255, 1.0)");
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
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), graph1);
  
    GtkWidget *grid=gtk_grid_new(); 
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);   

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), graph1, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 150);
   
    gtk_container_add(GTK_CONTAINER(window), paned1);
  
    gtk_widget_show_all(window);

    gtk_main();
    return 0;
  }
static void combo1_changed(GtkComboBox *combo, gpointer *data)
  {
    gint id=gtk_combo_box_get_active(combo);
    if(id==0)
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 1);
        multi_graph_set_columns(MULTI_GRAPH(data), 1);
      }
    else if(id==1)
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 1);
        multi_graph_set_columns(MULTI_GRAPH(data), 4);
      }
    else if(id==2)
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 2);
        multi_graph_set_columns(MULTI_GRAPH(data), 2);
      }
    else if(id==3)
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 3);
        multi_graph_set_columns(MULTI_GRAPH(data), 3);
      }
    else if(id==4)
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 4);
        multi_graph_set_columns(MULTI_GRAPH(data), 4);
      }
    else if(id==5)
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 3);
        multi_graph_set_columns(MULTI_GRAPH(data), 2);
      }
    else
      {
        multi_graph_set_rows(MULTI_GRAPH(data), 2);
        multi_graph_set_columns(MULTI_GRAPH(data), 3);
      }

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }

