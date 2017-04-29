
/*

    Test some different colors out. Add some entries to gouraud_mesh2.c to change the colors
dynamically. Draw a couple different gems.

    gcc -Wall gem_maker1.c -o gem_maker1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean time_redraw(gpointer da);
static gboolean start_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_gem(GtkWidget *da, cairo_t *cr, gpointer data);
static void check_colors(GtkWidget *widget, GtkWidget **colors);
static gboolean draw_main_window(GtkWidget *widget, cairo_t *cr, gpointer data);

GtkWidget *button1=NULL;
//Count of drawn triangle sectors.
static gint counter=0;
//Drawing background color.
static gdouble b1[]={1.0, 1.0, 1.0, 1.0};
//Triangle control point colors.
static gdouble t1_old[]={0.75, 0.0, 0.75, 1.0, 0.0, 0.0, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0};
static gdouble t1_new[]={0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0};
static gdouble t2_old[]={0.75, 0.0, 0.75, 1.0, 0.75, 0.0, 0.75, 1.0, 0.0, 0.0, 0.0, 1.0};
static gdouble t2_new[]={0.0, 1.0, 1.0, 1.0, 0.0, 1.0, 1.0, 1.0, 0.0, 0.0, 1.0, 1.0};

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    GtkWidget *window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Gem Maker");
    gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    gtk_widget_set_app_paintable(window, TRUE);
    //Try to set transparency of main window.
    if(gtk_widget_is_composited(window))
      {
        GdkScreen *screen=gtk_widget_get_screen(window);  
        GdkVisual *visual=gdk_screen_get_rgba_visual(screen);
        gtk_widget_set_visual(window, visual);
      }
    else g_print("Can't set window transparency.\n");
    
    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(start_drawing), NULL);    

    GtkWidget *label1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label1), "<span font_weight='heavy'>Background </span>");
    GtkWidget *label2=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label2), "<span font_weight='heavy'> T1_1 </span>");
    GtkWidget *label3=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label3), "<span font_weight='heavy'> T1_2 </span>");
    GtkWidget *label4=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label4), "<span font_weight='heavy'> T1_3 </span>");
    GtkWidget *label5=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label5), "<span font_weight='heavy'> T2_1 </span>");
    GtkWidget *label6=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label6), "<span font_weight='heavy'> T2_2 </span>");
    GtkWidget *label7=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label7), "<span font_weight='heavy'> T2_3 </span>");

    GtkWidget *entry1=gtk_entry_new();
    gtk_widget_set_hexpand(entry1, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry1), "rgba(255, 255, 255, 1.0)");

    GtkWidget *entry2=gtk_entry_new();
    gtk_widget_set_hexpand(entry2, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry2), "rgba(0, 255, 255, 1.0)");

    GtkWidget *entry3=gtk_entry_new();
    gtk_widget_set_hexpand(entry3, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry3), "rgba(0, 0, 255, 1.0)");

    GtkWidget *entry4=gtk_entry_new();
    gtk_widget_set_hexpand(entry4, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry4), "rgba(0, 0, 255, 1.0)");

    GtkWidget *entry5=gtk_entry_new();
    gtk_widget_set_hexpand(entry5, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry5), "rgba(0, 255, 255, 1.0)");

    GtkWidget *entry6=gtk_entry_new();
    gtk_widget_set_hexpand(entry6, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry6), "rgba(0, 255, 255, 1.0)");

    GtkWidget *entry7=gtk_entry_new();
    gtk_widget_set_hexpand(entry7, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry7), "rgba(0, 0, 255, 1.0)");

    button1=gtk_button_new_with_label("Update Colors");
    gtk_widget_set_hexpand(button1, FALSE);
    GtkWidget *colors[]={entry1, entry2, entry3, entry4, entry5, entry6, entry7, window, da};
    g_signal_connect(button1, "clicked", G_CALLBACK(check_colors), colors);
    gtk_widget_set_sensitive(button1, FALSE);
    
    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);    
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label5, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry5, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry6, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry7, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 7, 2, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 300);

    //Draw background window based on the paned window splitter.
    g_signal_connect(window, "draw", G_CALLBACK(draw_main_window), paned1);

    gtk_container_add(GTK_CONTAINER(window), paned1);

    g_timeout_add(400, (GSourceFunc)time_redraw, da);

    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
static gboolean time_redraw(gpointer da)
 {
   if(counter>10)
     {
       gtk_widget_set_sensitive(button1, TRUE);
       return FALSE;
     }
   else
     {
       gtk_widget_queue_draw(GTK_WIDGET(da));
       counter++;
       return TRUE;
     }
 }
static gboolean start_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    //Need scaling here.
    draw_gem(da, cr, data);
    return TRUE;
  }
static void draw_gem(GtkWidget *da, cairo_t *cr, gpointer data)
  { 
    gint i=0;
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    gdouble advance=counter%12+1;

    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Set background.
    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);

    //A circle to clip the mesh in.
    cairo_arc(cr, width/2.0, height/2.0, 3.5*h1, 0.0, 2.0*G_PI);
    cairo_clip(cr);

    //Draw 12 gouraud shaded triangles in a circle.
    gdouble hour_start=-G_PI/2.0;
    gdouble next_hour=-G_PI/6.0;
    gdouble hour_radius=4.0*h1;
    gdouble temp_cos=0;
    gdouble temp_sin=0;
    gdouble prev_cos=0;
    gdouble prev_sin=0;
    cairo_move_to(cr, 0.0, 0.0);
    cairo_translate(cr, width/2.0, height/2.0);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    //Outside part of circle triangles. 
    for(i=0;i<13;i++)
      {
        temp_cos=cos(hour_start-(next_hour*i));
        temp_sin=sin(hour_start-(next_hour*i));
        //The polar form of the equation for an ellipse to get the radius.
        hour_radius=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin*temp_sin) + ((4.0*h1)*(4.0*h1)*temp_cos*temp_cos));
        temp_cos=temp_cos*hour_radius;
        temp_sin=temp_sin*hour_radius;
       
        if(i>0)
          {
            cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
            cairo_mesh_pattern_begin_patch(pattern1);
            cairo_mesh_pattern_move_to(pattern1, prev_cos, prev_sin);
            cairo_mesh_pattern_line_to(pattern1, temp_cos, temp_sin);
            cairo_mesh_pattern_line_to(pattern1, 0.0, 0.0);
            if(i<=advance)
              {
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, t1_new[0], t1_new[1], t1_new[2], t1_new[3]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, t1_new[4], t1_new[5], t1_new[6], t1_new[7]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, t1_new[8], t1_new[9], t1_new[10], t1_new[11]);
              }
            else //background colors
              {
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, t1_old[0], t1_old[1], t1_old[2], t1_old[3]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, t1_old[4], t1_old[5], t1_old[6], t1_old[7]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, t1_old[8], t1_old[9], t1_old[10], t1_old[11]);
              }
            cairo_mesh_pattern_end_patch(pattern1);
            cairo_set_source(cr, pattern1);
            cairo_paint(cr);   
            cairo_pattern_destroy(pattern1);
          }
        prev_cos=temp_cos;
        prev_sin=temp_sin;
      }

    //Inside part of circle triangles.
    for(i=0;i<13;i++)
      {
        temp_cos=cos(hour_start-(next_hour*i));
        temp_sin=sin(hour_start-(next_hour*i));
        //The polar form of the equation for an ellipse to get the radius.
        hour_radius=((2.0*w1)*(2.0*h1))/sqrt(((2.0*w1)*(2.0*w1)*temp_sin*temp_sin) + ((2.0*h1)*(2.0*h1)*temp_cos*temp_cos));
        temp_cos=temp_cos*hour_radius;
        temp_sin=temp_sin*hour_radius;
       
        if(i>0)
          {
            cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
            cairo_mesh_pattern_begin_patch(pattern1);
            cairo_mesh_pattern_move_to(pattern1, prev_cos, prev_sin);
            cairo_mesh_pattern_line_to(pattern1, temp_cos, temp_sin);
            cairo_mesh_pattern_line_to(pattern1, 0.0, 0.0);
            if(i<=advance)
              {
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, t2_new[0], t2_new[1], t2_new[2], t2_new[3]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, t2_new[4], t2_new[5], t2_new[6], t2_new[7]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, t2_new[8], t2_new[9], t2_new[10], t2_new[11]);
              }
            else //background colors
              {
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, t2_old[0], t2_old[1], t2_old[2], t2_old[3]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, t2_old[4], t2_old[5], t2_old[6], t2_old[7]);
                cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, t2_old[8], t2_old[9], t2_old[10], t2_old[11]);
              }
            cairo_mesh_pattern_end_patch(pattern1);
            cairo_set_source(cr, pattern1);
            cairo_paint(cr);   
            cairo_pattern_destroy(pattern1);
          }
        prev_cos=temp_cos;
        prev_sin=temp_sin;
      }    
  }
static void check_colors(GtkWidget *widget, GtkWidget **colors)
  {
    gint i=0;
    GdkRGBA rgba;
    counter=0;

    for(i=0;i<7;i++)
      {
        if(gdk_rgba_parse(&rgba, gtk_entry_get_text(GTK_ENTRY(colors[i]))))
          {
            switch(i)
              {
                case 0:
                  b1[0]=rgba.red;
                  b1[1]=rgba.green;
                  b1[2]=rgba.blue;
                  b1[3]=rgba.alpha;
                  break;
                case 1:
                  t1_old[0]=t1_new[0];
                  t1_old[1]=t1_new[1];
                  t1_old[2]=t1_new[2];
                  t1_old[3]=t1_new[3];
                  t1_new[0]=rgba.red;
                  t1_new[1]=rgba.green;
                  t1_new[2]=rgba.blue;
                  t1_new[3]=rgba.alpha;
                  break;
                case 2:
                  t1_old[4]=t1_new[4];
                  t1_old[5]=t1_new[5];
                  t1_old[6]=t1_new[6];
                  t1_old[7]=t1_new[7];
                  t1_new[4]=rgba.red;
                  t1_new[5]=rgba.green;
                  t1_new[6]=rgba.blue;
                  t1_new[7]=rgba.alpha;
                  break;
                case 3:
                  t1_old[8]=t1_new[8];
                  t1_old[9]=t1_new[9];
                  t1_old[10]=t1_new[10];
                  t1_old[11]=t1_new[11];
                  t1_new[8]=rgba.red;
                  t1_new[9]=rgba.green;
                  t1_new[10]=rgba.blue;
                  t1_new[11]=rgba.alpha;
                  break;
                case 4:
                  t2_old[0]=t2_new[0];
                  t2_old[1]=t2_new[1];
                  t2_old[2]=t2_new[2];
                  t2_old[3]=t2_new[3];
                  t2_new[0]=rgba.red;
                  t2_new[1]=rgba.green;
                  t2_new[2]=rgba.blue;
                  t2_new[3]=rgba.alpha;
                  break;
                case 5:
                  t2_old[4]=t2_new[4];
                  t2_old[5]=t2_new[5];
                  t2_old[6]=t2_new[6];
                  t2_old[7]=t2_new[7];
                  t2_new[4]=rgba.red;
                  t2_new[5]=rgba.green;
                  t2_new[6]=rgba.blue;
                  t2_new[7]=rgba.alpha;
                  break;
                case 6:
                  t2_old[8]=t2_new[8];
                  t2_old[9]=t2_new[9];
                  t2_old[10]=t2_new[10];
                  t2_old[11]=t2_new[11];
                  t2_new[8]=rgba.red;
                  t2_new[9]=rgba.green;
                  t2_new[10]=rgba.blue;
                  t2_new[11]=rgba.alpha;
                  break;
             }
          }
        else
          {
            g_print("Color string format error in Entry %i\n", i);
          } 
      }

    //Update main window.
    gtk_widget_queue_draw(colors[7]);
    //Update the drawing area.
    gtk_widget_set_sensitive(button1, FALSE);
    g_timeout_add(400, (GSourceFunc)time_redraw, colors[8]);
  }
static gboolean draw_main_window(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    //Paint background of drawing area.
    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);
    //Paint the background under the grid.
    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.5);
    gint width=gtk_paned_get_position(GTK_PANED(data));
    gint height=gtk_widget_get_allocated_height(widget);

    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_rectangle(cr, width, 0.0, 10, height);
    cairo_fill(cr);
    return FALSE;
  }

