
/*

    Test drawing for a speedometer or tachometer gauge with a mesh gradient. This
is using the circular_gradient1.c code with the gauge. The idea is to eventually add gradients
to the adjustable gauge widget.

    gcc -Wall da_speedometer2.c -o da_speedometer2 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18.

    By C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<math.h>

static gdouble needle=0.0;
static gdouble scale_bottom=0.0;
static gdouble scale_top=100.0;

//Testing variables. See circular_gradient1.c to see what they do.
static gint drawing_combo=3;
static gint rotate_combo=G_PI/2.0;
static gboolean fade=TRUE;

static gboolean draw_gage(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_gradient(GtkWidget *da, cairo_t *cr);
static void draw_gradient_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1);
static gboolean time_draw(GtkWidget *da);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_title(GTK_WINDOW(window), "Speedometer Gauge2");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    //Gauge drawing.    
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da1, TRUE);
    gtk_widget_set_vexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_gage), NULL);
   
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);
   
    g_timeout_add(100, (GSourceFunc)time_draw, da1);

    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean draw_gage(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);
    gdouble w1=0.0;

    //Scale.
    if(width<height) w1=width/10.0;
    else w1=height/10.0;
    
    //Paint background.
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    cairo_translate(cr, width/2.0, height/2.0);

    cairo_save(cr);
    draw_gradient(da, cr);
    cairo_restore(cr);

    gdouble diff=scale_top-scale_bottom;

    //Set large tick marks.
    gint i=0;
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    gdouble tenth_scale=diff/10.0;
    gdouble tick_mark=(5.0*G_PI/3.0)/10.0;
    gdouble temp=0;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_text_extents_t tick_extents;
    cairo_set_font_size(cr, 20);
    cairo_move_to(cr, 0, 0);
    for(i=0;i<11;i++)
      {
        temp=(gdouble)i*tick_mark;
        cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*3.5*w1, -sin((4.0*G_PI/3.0)-temp)*3.5*w1);
        cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*3.8*w1, -sin((4.0*G_PI/3.0)-temp)*3.8*w1);
        cairo_stroke(cr);
        //String values at bit tick marks.
        gchar *tick_string=g_strdup_printf("%i", (int)((gdouble)i*tenth_scale));
        cairo_text_extents(cr, tick_string, &tick_extents);
        cairo_move_to(cr, (cos((4.0*G_PI/3.0)-temp)*3.0*w1)-tick_extents.width/2.0, (-sin((4.0*G_PI/3.0)-temp)*3.0*w1)+tick_extents.height/2.0);
        cairo_show_text(cr, tick_string);
        g_free(tick_string);
        //Reset position to the center.
        cairo_move_to(cr, 0, 0);
      }

    //Set small tick marks.
    cairo_set_line_width(cr, 3.0);
    gdouble half_tick=tick_mark/2.0;
    cairo_move_to(cr, 0, 0);
    for(i=0;i<10;i++)
      {
        temp=(gdouble)i*tick_mark+half_tick;
        cairo_move_to(cr, cos((4.0*G_PI/3.0)-temp)*3.65*w1, -sin((4.0*G_PI/3.0)-temp)*3.65*w1);
        cairo_line_to(cr, cos((4.0*G_PI/3.0)-temp)*3.8*w1, -sin((4.0*G_PI/3.0)-temp)*3.8*w1);
        cairo_stroke(cr);
        cairo_move_to(cr, 0, 0);
      }

    //The needle line.
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    gdouble standard_needle=(((needle-scale_bottom)/diff)*(5.0*G_PI/3.0));
    cairo_move_to(cr, 0, 0);
    cairo_line_to(cr, cos((4.0*G_PI/3.0)-standard_needle)*2.4*w1, -sin((4.0*G_PI/3.0)-standard_needle)*2.4*w1);
    cairo_stroke(cr);
    
    //Text for needle value.
    cairo_text_extents_t extents1;
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
    cairo_set_font_size(cr, 30);
    gchar *string1=g_strdup_printf("%3.2f", needle);
    cairo_text_extents(cr, string1, &extents1); 
    cairo_move_to(cr, -extents1.width/2, 3.0*w1+extents1.height/2);  
    cairo_show_text(cr, string1);
    g_free(string1);

    return FALSE;
  }
static void draw_gradient(GtkWidget *da, cairo_t *cr)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=0.0;
   gdouble r1=0; 

   //Scale.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   if(rotate_combo==0) r1=0;
   else if(rotate_combo==1) r1=G_PI/2.0;
   else if(rotate_combo==2) r1=G_PI;
   else r1=3.0*G_PI/2.0;

   if(drawing_combo==0) draw_gradient_circle(da, cr, -G_PI/2.0, 4, r1);
   else if(drawing_combo==1) draw_gradient_circle(da, cr, -G_PI/4.0, 8, r1);
   else if(drawing_combo==2) draw_gradient_circle(da, cr, -G_PI/8.0, 16, r1);
   else if(drawing_combo==3)
     {
       cairo_arc_negative(cr, 0.0, 0.0, 2.2*w1, -5.0*G_PI/3.0, -4.0*G_PI/3.0);
       cairo_line_to(cr, (cos(-4.0*G_PI/3.0)*3.8*w1), sin(-4.0*G_PI/3.0)*3.8*w1);
       cairo_arc(cr, 0.0, 0.0, 3.8*w1, -4.0*G_PI/3.0, -5.0*G_PI/3.0);
       cairo_close_path(cr);
       cairo_clip(cr);
       draw_gradient_circle(da, cr, -G_PI/2.0, 4, r1);
     }
   else if(drawing_combo==4)
     {
       cairo_arc_negative(cr, 0.0, 0.0, 2.2*w1, -5.0*G_PI/3.0, -4.0*G_PI/3.0);
       cairo_line_to(cr, (cos(-4.0*G_PI/3.0)*3.8*w1), sin(-4.0*G_PI/3.0)*3.8*w1);
       cairo_arc(cr, 0.0, 0.0, 3.8*w1, -4.0*G_PI/3.0, -5.0*G_PI/3.0);
       cairo_close_path(cr);
       cairo_clip(cr);
       draw_gradient_circle(da, cr, -G_PI/4.0, 8, r1);
     }
   else
     {
       cairo_arc_negative(cr, 0.0, 0.0, 2.2*w1, -5.0*G_PI/3.0, -4.0*G_PI/3.0);
       cairo_line_to(cr, (cos(-4.0*G_PI/3.0)*3.8*w1), sin(-4.0*G_PI/3.0)*3.8*w1);
       cairo_arc(cr, 0.0, 0.0, 3.8*w1, -4.0*G_PI/3.0, -5.0*G_PI/3.0);
       cairo_close_path(cr);
       cairo_clip(cr);
       draw_gradient_circle(da, cr, -G_PI/8.0, 16, r1);
     }
  
 }
static void draw_gradient_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble rotation=-next_section;
   gdouble offset=-next_section/2.0;
   gdouble control_points[sections*4];
   gdouble w1=0.0;

   //Scale.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   //Rotate drawing.
   cairo_rotate(cr, r1); 
 
   //For the mesh color fade.
   gdouble red1=0.0;
   gdouble red2=0.0;
   gdouble green1=1.0;
   gdouble green2=0.0;
   gdouble blue1=0.0;
   gdouble blue2=1.0;
   
   gdouble start=0.0;
   gdouble line_radius1=0;
   gdouble line_radius2=0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   gdouble prev_cos1=2.0*w1;
   gdouble prev_sin1=0.0;
   gdouble prev_cos2=5.0*w1;
   gdouble prev_sin2=0.0;
   for(i=0;i<sections;i++)
     {
       temp_cos1=cos(start-(next_section*(i+1)));
       temp_sin1=sin(start-(next_section*(i+1)));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       //The polar form of the equation for an ellipse to get the radius. Radius based on width.
       line_radius1=((2.0*w1)*(2.0*w1))/sqrt(((2.0*w1)*(2.0*w1)*temp_sin1*temp_sin1) + ((2.0*w1)*(2.0*w1)*temp_cos1*temp_cos1));
       line_radius2=((5.0*w1)*(5.0*w1))/sqrt(((5.0*w1)*(5.0*w1)*temp_sin1*temp_sin1) + ((5.0*w1)*(5.0*w1)*temp_cos1*temp_cos1));

       temp_cos1=temp_cos1*line_radius1;
       temp_sin1=temp_sin1*line_radius1;
       temp_cos2=temp_cos2*line_radius2;
       temp_sin2=temp_sin2*line_radius2;

       /*
         Draw Bezier control points. 
         https://en.wikipedia.org/wiki/Composite_B%C3%A9zier_curve#Approximating_circular_arcs 
         Swapping values needed for rotated points.
       */   
       control_points[4*i+2]=(4.0*line_radius2-line_radius2*cos(offset))/3.0; 
       control_points[4*i+3]=((line_radius2-line_radius2*cos(offset))*(3.0*line_radius2-line_radius2*cos(offset)))/(3.0*line_radius2*sin(offset));
       control_points[4*i]=control_points[4*i+2];
       control_points[4*i+1]=-control_points[4*i+3];      
       cairo_save(cr);
       cairo_rotate(cr, -(i+1)*rotation+offset);
       if(drawing_combo<3)
         {
           cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
           cairo_move_to(cr, control_points[4*i], control_points[4*i+1]);           
           cairo_show_text(cr, "X");
           cairo_move_to(cr, control_points[4*i+2], control_points[4*i+3]);
           cairo_show_text(cr, "X");
         }
       cairo_rotate(cr, -r1); 
       cairo_device_to_user_distance(cr, &control_points[4*i], &control_points[4*i+1]);
       cairo_device_to_user_distance(cr, &control_points[4*i+2], &control_points[4*i+3]);
       cairo_rotate(cr, r1); 
       cairo_restore(cr);          
        
       //Draw the gradients.
       if(fade)
         {   
           red1=1.0; 
           green1=0.0+(gdouble)(i)/(gdouble)sections;
           blue1=1.0-(gdouble)(i)/(gdouble)sections;
           red2=1.0;
           green2=0.0+(gdouble)(i+1)/(gdouble)sections;
           blue2=1.0-(gdouble)(i+1)/(gdouble)sections;
         }        
       cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
       cairo_mesh_pattern_begin_patch(pattern1);
       cairo_mesh_pattern_move_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_curve_to(pattern1, control_points[4*i], control_points[4*i+1],  control_points[4*i+2], control_points[4*i+3], temp_cos2, temp_sin2);
       cairo_mesh_pattern_line_to(pattern1, temp_cos1, temp_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos1, prev_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, red1, green1, blue1);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, red2, green2, blue2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, red2, green2, blue2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, red1, green1, blue1);
       cairo_mesh_pattern_end_patch(pattern1);
       cairo_set_source(cr, pattern1);
       cairo_paint(cr);
       cairo_pattern_destroy(pattern1); 

       //Polygon
       if(drawing_combo<3)
         {
           cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
           cairo_set_line_width(cr, 4.0); 
           cairo_move_to(cr, prev_cos1, prev_sin1);
           cairo_line_to(cr, temp_cos1, temp_sin1);
           cairo_stroke_preserve(cr);
           cairo_line_to(cr, temp_cos2, temp_sin2);
           cairo_stroke_preserve(cr);
           cairo_line_to(cr, prev_cos2, prev_sin2);
           cairo_close_path(cr);
           cairo_stroke(cr);
         }            
      
       //Save previous values.
       prev_cos1=temp_cos1;
       prev_sin1=temp_sin1;
       prev_cos2=temp_cos2;
       prev_sin2=temp_sin2;
     }
 }

static gboolean time_draw(GtkWidget *da)
  {
    static int i=0;
    //g_print("Redraw Gauge %i\n", i);
    if(i==100) i=0;
    needle=i;
    gtk_widget_queue_draw(da);
    i++;
    return TRUE;
  }







