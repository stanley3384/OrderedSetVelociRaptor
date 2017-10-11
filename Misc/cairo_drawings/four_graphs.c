
/*
    Test some graph drawing out with C and Perl. The Perl script is at the end of the C
code. 

    gcc -Wall four_graphs.c -o four_graphs `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18
   
    C. Eric Cashon
*/

#include <gtk/gtk.h>

//Number of graphs to draw. Either 1 or 4. Changed by the two buttons. 
static gint graphs=1;

static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data);
static void single_button_clicked(GtkWidget *widget, gpointer data);
static void multiple_button_clicked(GtkWidget *widget, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Four Graphs");
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE); 
    gtk_widget_set_vexpand(da, TRUE);  
    g_signal_connect(da, "draw", G_CALLBACK(draw_graphs), NULL);

    GtkWidget *single_button=gtk_button_new_with_label("graphs 1");
    g_signal_connect(single_button, "clicked", G_CALLBACK(single_button_clicked), da);

    GtkWidget *multiple_button=gtk_button_new_with_label("graphs 4");
    g_signal_connect(multiple_button, "clicked", G_CALLBACK(multiple_button_clicked), da);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), single_button, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), multiple_button, 2, 5, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
  }
static gboolean draw_graphs(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gint i=0;
    gint j=0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    //Some drawing variables.
    gdouble x=0.0;
    gdouble y=0.0;
    gdouble graph_width=width/graphs;
    gdouble vertical_width=0.0;
    gdouble horizontal_width=0.0;
    gint v_lines[]={15, 10, 12, 21};
    gint h_lines[]={4, 5, 6, 7};
    gdouble max_number=0.0;
    gdouble data_y=0.0;
    gdouble scale=0.0;

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_paint(cr);
     
    //Vertical lines.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 1);
    for(i=0;i<graphs;i++)
      {
        vertical_width=graph_width/v_lines[i];
        for(j=0; j<v_lines[i]; j++)
          {
            x=i*graph_width+j*vertical_width;
            cairo_move_to(cr, x, 0.0);
            cairo_line_to(cr, x, height);
            cairo_stroke(cr);
          } 
      }

    //Horizontal lines.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    for(i=0;i<graphs;i++)
      {
        horizontal_width=height/h_lines[i];
        for(j=0;j<h_lines[i];j++)
          {
            y=j*horizontal_width;
            cairo_move_to(cr, graph_width*i, y);
            cairo_line_to(cr, graph_width*i+graph_width, y);
            cairo_stroke(cr);
          }
      }

    //Data points.
    cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
    cairo_set_line_width(cr, 2);
    for(i=0;i<graphs;i++)
      {
        vertical_width=graph_width/v_lines[i];
        cairo_move_to(cr, graph_width*i, height);
        x=graph_width*i;
        y=height;
        if(i==0||i==2)
          { 
            data_y=0.02;
            scale=0.8;
          }
        else
          {
            data_y=0.2;
            scale=0.5;
          }
        for(j=0;j<v_lines[i];j++)
          {
            data_y=data_y*1.5;
            x=x+vertical_width;
            data_y=data_y*scale;
            y=y-(height*data_y);
            cairo_line_to(cr, x, y);
            cairo_stroke_preserve(cr);
          } 
      }
    cairo_stroke(cr);

    //Number of vertical lines for each graph.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for(i=0;i<graphs;i++)
      {
        vertical_width=graph_width/v_lines[i];
        for(j=0;j<v_lines[i];j++)
          {
            x=i*graph_width+j*vertical_width;
            cairo_move_to(cr, x+5, height-10);
            gchar *string=g_strdup_printf("%i", j);
            cairo_show_text(cr, string);
            g_free(string);
          }
      }

    //Horizontal line numbers.
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    for(i=0;i<graphs;i++)
      {
        horizontal_width=height/h_lines[i];
        max_number=h_lines[i]*500;
        for(j=0;j<h_lines[i];j++)
          {
            y=j*horizontal_width;
            cairo_move_to(cr, graph_width*i+5, y+20);
            gchar *string=g_strdup_printf("%i", (gint)(max_number-j*500));
            cairo_show_text(cr, string);
            g_free(string);
          }
      }

    //Draw graph blocks.
    cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 10);
    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_stroke(cr);
    cairo_set_line_width(cr, 5);
    for(i=1; i<graphs; i++)
      {
        x=i*graph_width;
        cairo_move_to(cr, x, 0.0);
        cairo_line_to(cr, x, height);
        cairo_stroke(cr);
      }
 
    return FALSE;
  }
static void single_button_clicked(GtkWidget *widget, gpointer data)
  {
    graphs=1;
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void multiple_button_clicked(GtkWidget *widget, gpointer data)
  {
    graphs=4;
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }

/*
#!/usr/bin/perl
use strict;
use diagnostics;
use warnings;
use Gtk3 '-init';
use Glib qw(TRUE FALSE);

my $window = Gtk3::Window->new();
$window->signal_connect('delete_event' => sub { Gtk3->main_quit; });
$window->set_default_size(800, 400);
$window->set_border_width(10);
$window->set_title("Graphs");

#Number of graphs. Either 1 or 4. Changed by the two buttons. 
my $graphs = 1;

my $da = Gtk3::DrawingArea->new();
$da->set_vexpand(TRUE);
$da->set_hexpand(TRUE);
$da->signal_connect('draw' => \&draw_graph);

my $single_button = Gtk3::Button->new( 'graph 1 date' );
$single_button->signal_connect('clicked' => \&single_button_clicked, $da);

my $multiple_button = Gtk3::Button->new( 'graph 4 dates' );
$multiple_button->signal_connect('clicked' => \&multiple_button_clicked, $da);

my $grid1 = Gtk3::Grid->new();
$grid1->set_row_spacing(10);
$grid1->attach($da, 0, 0, 4, 4);
$grid1->attach($single_button, 1, 5, 1, 1);
$grid1->attach($multiple_button, 2, 5, 1, 1);

$window->add($grid1);

$window->show_all();
Gtk3->main;

sub draw_graph 
{
  my ($widget, $cr, $data) = @_;
  my $width = $widget->get_allocated_width();
  my $height = $widget->get_allocated_height();
  #Some drawing variables.
  my $x = 0;
  my $y = 0;
  my $graph_width = $width / $graphs;
  my $vertical_width = 0;
  my $horizontal_width = 0;
  my @v_lines = (15, 10, 12, 21);
  my @h_lines = (4, 5, 6, 7);
  my $max_number = 0;
  my $data_y = 0;
  my $scale = 0;
  
  #Paint background.
  $cr->set_source_rgb(0, 0, 0);
  $cr->paint;

  #Vertical lines.
  $cr->set_source_rgb(0, 0, 1);
  $cr->set_line_width(1);
  for (my $i=0; $i < $graphs; $i++)
    {
      $vertical_width = $graph_width / $v_lines[$i];
      for (my $j = 0; $j < $v_lines[$i]; $j++)
        {
          $x = $i * $graph_width + $j * $vertical_width;
          $cr->move_to($x, 0);
          $cr->line_to($x, $height);
          $cr->stroke();
        } 
    }

  #Horizontal lines.
  $cr->set_source_rgb(0, 0, 1);
  for (my $i = 0; $i < $graphs; $i++)
    {
      $horizontal_width = $height / $h_lines[$i];
      for (my $j = 0; $j < $h_lines[$i]; $j++)
        {
          $y = $j * $horizontal_width;
          $cr->move_to($graph_width * $i, $y);
          $cr->line_to($graph_width * $i + $graph_width, $y);
          $cr->stroke();
        }
    }

  #Data points.
  $cr->set_source_rgb(1, 1, 0);
  $cr->set_line_width(2);
  for (my $i = 0; $i < $graphs; $i++)
    {
      $vertical_width = $graph_width / $v_lines[$i];
      $cr->move_to($graph_width * $i, $height);
      $x = $graph_width * $i;
      $y = $height;
      if ($i == 0 or $i == 2)
        { 
          $data_y = 0.02;
          $scale = 0.8;
        }
      else
        {
          $data_y = 0.2;
          $scale = 0.5;
        }
      for (my $j = 0; $j < $v_lines[$i]; $j++)
        {
          $data_y = $data_y * 1.5;
          $x = $x + $vertical_width;
          $data_y = $data_y * $scale;
          $y = $y - ($height * $data_y);
          $cr->line_to($x, $y);
          $cr->stroke_preserve();
        } 
    }
  $cr->stroke();

  #Number of vertical lines for each graph.
  $cr->set_source_rgb(1, 1, 1);
  for (my $i=0; $i < $graphs; $i++)
    {
      $vertical_width = $graph_width / $v_lines[$i];
      for (my $j = 0; $j < $v_lines[$i]; $j++)
        {
          $x = $i * $graph_width + $j * $vertical_width;
          $cr->move_to($x + 5, $height - 10);
          $cr->show_text ($j);
        }
    }

  #Horizontal line numbers.
  $cr->set_source_rgb(1, 1, 1);
  for (my $i=0; $i < $graphs; $i++)
    {
      $horizontal_width = $height / $h_lines[$i];
      $max_number = $h_lines[$i] * 500;
      for (my $j = 0; $j < $h_lines[$i]; $j++)
        {
          $y = $j * $horizontal_width;
          $cr->move_to($graph_width * $i + 5, $y + 20);
          $cr->show_text ($max_number - $j * 500);
        }
    }


  #Draw graph blocks.
  $cr->set_source_rgb(0, 1, 1);
  $cr->set_line_width(10);
  $cr->rectangle(0, 0, $width, $height);
  $cr->stroke();
  $cr->set_line_width(5);
  for (my $i=1; $i < $graphs; $i++)
    {
      $x = $i * $graph_width;
      $cr->move_to($x, 0);
      $cr->line_to($x, $height);
      $cr->stroke();
    }

  return FALSE;
}
sub single_button_clicked 
{
  my ($widget, $da) = @_; 
  $graphs = 1;
  $da->queue_draw();
}
sub multiple_button_clicked 
{
  my ($widget, $da) = @_; 
  $graphs = 4;
  $da->queue_draw(); 
}

*/


