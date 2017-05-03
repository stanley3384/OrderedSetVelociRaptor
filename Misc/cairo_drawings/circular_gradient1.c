
/*   
    Test getting some Bezier points with some gradients. Needs to be kept square. The Bezier
curves can be clipped to get a good ring drawing. For a stretchy Bezier circle gradient look
at circular_gradient_clock1.c.

    gcc -Wall circular_gradient1.c -o circular_gradient1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections);
static void combo1_changed(GtkComboBox *combo1, gpointer data);
static void toggle_fade(GtkToggleButton *check1, gpointer data);

gint drawing_combo=0;
gboolean fade=FALSE;

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Bezier Points");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 470);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   //Start the drawing square.
   gtk_widget_set_size_request(da, 400, 400);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), NULL);

   GtkWidget *combo1=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo1, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Draw 4 Sections");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Draw 8 Sections");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Draw 16 Sections");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Clip Ring 4");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Clip Ring 8");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "Clip Ring 16");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
   g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), da);

   GtkWidget *check1=gtk_check_button_new_with_label("Fade Color");
   g_signal_connect(check1, "toggled", G_CALLBACK(toggle_fade), NULL);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), check1, 0, 2, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;

   //Background.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr);

   if(drawing_combo==0) draw_circle(da, cr, -G_PI/2.0, 4);
   else if(drawing_combo==1) draw_circle(da, cr, -G_PI/4.0, 8);
   else if(drawing_combo==2) draw_circle(da, cr, -G_PI/8.0, 16);
   else if(drawing_combo==3)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, 3.8*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/2.0, 4);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, 2.7*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr);
     }
   else if(drawing_combo==4)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, 3.8*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/4.0, 8);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, 2.7*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr);  
     }
   else
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, 3.8*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/8.0, 16);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, 2.7*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr); 
     }
   return FALSE;
 }
static void draw_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble rotation=-next_section;
   gdouble offset=-next_section/2.0;
   gdouble control_points[sections*4];

   /*
     Layout for the drawing is a 10x10 rectangle. Problem with rotating control points
     and sections with a non-square drawing dynamically.
   */
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //For the mesh color fade.
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
   gdouble prev_cos1=2.5*w1;
   gdouble prev_sin1=0.0;
   gdouble prev_cos2=4.0*w1;
   gdouble prev_sin2=0.0;
   cairo_translate(cr, width/2.0, height/2.0);
   for(i=0;i<sections;i++)
     {
       temp_cos1=cos(start-(next_section*(i+1)));
       temp_sin1=sin(start-(next_section*(i+1)));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       //The polar form of the equation for an ellipse to get the radius.
       line_radius1=((2.5*w1)*(2.5*h1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin1*temp_sin1) + ((2.5*h1)*(2.5*h1)*temp_cos1*temp_cos1));
       line_radius2=((4.0*w1)*(4.0*h1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin1*temp_sin1) + ((4.0*h1)*(4.0*h1)*temp_cos1*temp_cos1));

       temp_cos1=temp_cos1*line_radius1;
       temp_sin1=temp_sin1*line_radius1;
       temp_cos2=temp_cos2*line_radius2;
       temp_sin2=temp_sin2*line_radius2;

       /*
         Draw Bezier control points. 
         https://en.wikipedia.org/wiki/Composite_B%C3%A9zier_curve#Approximating_circular_arcs 
         Swapping values needed for rotated points.
       */   
       control_points[4*i+2]=(4.0*line_radius2-line_radius2*cos(G_PI/8.0))/3.0;
       control_points[4*i+3]=((line_radius2-line_radius2*cos(G_PI/8.0))*(3.0*line_radius2-line_radius2*cos(G_PI/8.0)))/(3.0*line_radius2*sin(G_PI/8.0));
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
       cairo_device_to_user_distance(cr, &control_points[4*i], &control_points[4*i+1]);
       cairo_device_to_user_distance(cr, &control_points[4*i+2], &control_points[4*i+3]);
       cairo_restore(cr);               
        
       //Draw the gradients.
       if(fade)
         {
           green1=1.0-(gdouble)i/(gdouble)sections;
           blue1=0.0+(gdouble)i/(gdouble)sections;
           green2=1.0-(gdouble)(i+1)/(gdouble)sections;
           blue2=0.0+(gdouble)(i+1)/(gdouble)sections;
         }        
       cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
       cairo_mesh_pattern_begin_patch(pattern1);
       cairo_mesh_pattern_move_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_curve_to(pattern1, control_points[4*i], control_points[4*i+1],  control_points[4*i+2], control_points[4*i+3], temp_cos2, temp_sin2);
       cairo_mesh_pattern_line_to(pattern1, temp_cos1, temp_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos1, prev_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 0.0, green1, blue1);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 0.0, green2, blue2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, green2, blue2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 0.0, green1, blue1);
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
static void combo1_changed(GtkComboBox *combo1, gpointer data)
 {
   drawing_combo=gtk_combo_box_get_active(combo1);
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void toggle_fade(GtkToggleButton *check1, gpointer data)
 {
   if(gtk_toggle_button_get_active(check1)) fade=TRUE;
   else fade=FALSE;
 }








