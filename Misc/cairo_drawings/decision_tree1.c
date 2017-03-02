/*

    A decision tree to take a look at some simple probablilities. 

    gcc -Wall decision_tree1.c -o decision_tree1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static GRand *rand1;
static GArray *bins;
static gdouble ball_position[]={0.0, 2.5};
static gint bin_count[]={0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

static gboolean redraw(gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Decision Tree");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   rand1=g_rand_new();
   bins=g_array_new(FALSE, TRUE, sizeof(gdouble));

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   g_timeout_add(100, (GSourceFunc)redraw, da);

   gtk_widget_show_all(window);

   gtk_main();

   g_rand_free(rand1);
   g_array_free(bins, TRUE);

   return 0;  
 }
static gboolean redraw(gpointer data)
  {
    gdouble move=0;
    static gint x=0;

    x++;

    if(x%2==0)
      {
        move=g_rand_double(rand1);
        //Change probabilites around. Starting with 50/50 here.
        if(move>0.50)
          {
            ball_position[0]=ball_position[0]+0.5;
            ball_position[1]++;
          }
        else
          {
            ball_position[0]=ball_position[0]-0.5;
            ball_position[1]++;
          }
      }
    else
      {
        ball_position[1]++;
      }

    //Reset when ball is at bottom. Should eventually reset arrays also.
    if(ball_position[1]>21.0)
      {
        g_array_append_val(bins, ball_position[0]);
        //Add the ball to the bin.
        if(ball_position[0]<-3.6) bin_count[0]++;
        else if(ball_position[0]>-3.6&&ball_position[0]<-2.6) bin_count[1]++;
        else if(ball_position[0]>-2.6&&ball_position[0]<-1.6) bin_count[2]++;
        else if(ball_position[0]>-1.6&&ball_position[0]<-0.6) bin_count[3]++;
        else if(ball_position[0]>-0.6&&ball_position[0]<0.1) bin_count[4]++;
        else if(ball_position[0]>0.1&&ball_position[0]<0.6) bin_count[5]++;
        else if(ball_position[0]>0.6&&ball_position[0]<1.6) bin_count[6]++;
        else if(ball_position[0]>1.6&&ball_position[0]<2.6) bin_count[7]++;
        else if(ball_position[0]>2.6&&ball_position[0]<3.6) bin_count[8]++;
        else if(ball_position[0]>3.6&&ball_position[0]<4.6) bin_count[9]++;
        else g_print("Outside of bins!\n");
        ball_position[0]=0.0;
        ball_position[1]=2.5;
        x=0;
      }

    gtk_widget_queue_draw(GTK_WIDGET(data)); 
    return TRUE;
  }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/24.0;
   gdouble h1=height/24.0;

   //Background white.
   cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   //Layout axis for drawing.
   cairo_set_line_width(cr, 1.0);
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
   cairo_rectangle(cr, w1, h1, 22.0*w1, 22.0*h1);
   cairo_stroke(cr);
   cairo_move_to(cr, 1.0*w1, 12.0*h1);
   cairo_line_to(cr, 23.0*w1, 12.0*h1);
   cairo_stroke(cr);
   cairo_move_to(cr, 12.0*w1, 1.0*h1);
   cairo_line_to(cr, 12.0*w1, 23.0*h1);
   cairo_stroke(cr);

   //Draw the gates and bins
   gint i=2;
   gint j=0;
   gint ticks=2;
   //x position of first tick(line).
   gdouble offset=11.5;
   gdouble tick_row=0;
   cairo_set_line_width(cr, 2.0);
   cairo_set_source_rgba(cr, 0.0, 1.0, 0.0, 1.0);
   for(i=2;i<21;i+=2)
     {
       ticks=i-tick_row;
       for(j=0;j<ticks;j++)
         {
           cairo_move_to(cr, (gdouble)(offset+j)*w1, (gdouble)i*h1);
           if(i==20)
             {
               cairo_set_source_rgba(cr, 1.0, 0.0, 1.0, 1.0);
               cairo_line_to(cr, (gdouble)(offset+j)*w1, height);
             }
           else cairo_line_to(cr, (gdouble)(offset+j)*w1, (gdouble)(i+1)*h1);
           cairo_stroke(cr);
         }
       offset=offset-0.5;
       tick_row++;
     }

   //Draw the ball.
   cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
   cairo_arc(cr, (12.0+ball_position[0])*w1, ball_position[1]*h1, 0.5*w1, 0.0, 2.0*G_PI);
   cairo_fill(cr);

   //Draw bin progress bars.
   cairo_set_source_rgb(cr, 1.0, 0.0, 1.0);
   gdouble start_x=7.0*w1;
   for(i=0;i<10;i++)
     {
       cairo_rectangle(cr, start_x, height-(0.25*bin_count[i]*h1), 1.0*w1, height);
       cairo_fill(cr);
       start_x=start_x+(1.0*w1); 
     }

   return FALSE;
}







