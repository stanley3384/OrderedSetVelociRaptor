
/*
    Test putting more than one graph in a drawing area. 

    gcc -Wall four_graphs2.c -o four_graphs2 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
   
    C. Eric Cashon
*/

#include <gtk/gtk.h>
 
static gint graph_rows=1;
static gint graph_columns=1;

static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data);
static void combo1_changed(GtkComboBox *combo6, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Graphs");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

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
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), da);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 1, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 120);
   
    gtk_container_add(GTK_CONTAINER(window), paned1);

    gtk_widget_show_all(window);
    
    gtk_main();

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
    gdouble graph_width=width/graph_columns;
    gdouble graph_height=height/graph_rows;
    gdouble x_tick=graph_width/10.0;
    gdouble y_tick=graph_height/5.0;
    gdouble test_number=500.0;

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);
     
    //Vertical lines.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 1);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            for(k=0;k<10;k++)
              {
                x=j*graph_width+k*x_tick;
                cairo_move_to(cr, x, 0.0);
                cairo_line_to(cr, x, height);
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
            for(k=0;k<5;k++)
              {
                y=i*graph_height+k*y_tick;
                cairo_move_to(cr, 0, y);
                cairo_line_to(cr, width, y);
                cairo_stroke(cr);
              }
          }
      }

    //Data points.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_set_line_width(cr, 2);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            x=graph_width*j;
            y=graph_height*i+graph_height;
            cairo_move_to(cr, x, y);           
            for(k=0;k<4;k++)
              {
                x=j*graph_width+k*x_tick+x_tick;
                y=i*graph_height+graph_height-k*y_tick-y_tick;
                cairo_line_to(cr, x, y);
                cairo_stroke_preserve(cr);
              } 
          }
      }
    
    cairo_stroke(cr);

    //Number of vertical lines for each graph.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for(i=0;i<graph_rows;i++)
      {
        y=i*graph_height+graph_height;
        for(j=0;j<graph_columns;j++)
          {
            for(k=0;k<10;k++)
              {
                x=j*graph_width+k*x_tick;
                cairo_move_to(cr, x+5, y-10);
                gchar *string=g_strdup_printf("%i", k);
                cairo_show_text(cr, string);
                g_free(string);
              }
          }
      }

    //Horizontal line numbers.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for(i=0;i<graph_rows;i++)
      {
        for(j=0;j<graph_columns;j++)
          {
            x=j*graph_width;
            test_number=500;
            for(k=0;k<5;k++)
              {
                y=i*graph_height+graph_height-k*y_tick-y_tick;
                cairo_move_to(cr, x+5, y+20);
                gchar *string=g_strdup_printf("%i", (gint)(test_number+=500));
                cairo_show_text(cr, string);
                g_free(string);
              }
          }
      }

    //Draw graph blocks.
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 10);
    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_stroke(cr);
    cairo_set_line_width(cr, 5);
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
static void combo1_changed(GtkComboBox *combo1, gpointer data)
  {
    gint id=gtk_combo_box_get_active(combo1);
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
    else
      {
        graph_rows=4;
        graph_columns=4;
      }

    gtk_widget_queue_draw(GTK_WIDGET(data));
  }


