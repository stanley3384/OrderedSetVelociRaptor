
/*   
    Test getting some Bezier points and drawing some gradients. The Bezier curves can be
clipped to get a good ring drawing.

    gcc -Wall circular_gradient1.c -o circular_gradient1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1);
static void combo1_changed(GtkComboBox *combo1, gpointer data);
static void combo2_changed(GtkComboBox *combo2, gpointer data);
static void combo3_changed(GtkComboBox *combo3, gpointer data);
static void toggle_fade(GtkToggleButton *check1, gpointer data);
static void check_colors(GtkWidget *widget, GtkWidget **colors);

//Test a colors to start and stop the gradient.
static gdouble color_start[]={0.0, 1.0, 0.0, 1.0};
static gdouble color_stop[]={0.0, 0.0, 1.0, 1.0};
static gdouble cutoff1=100.0;
static gint drawing_combo=0;
static gint rotate_combo=0;
static gboolean fade=FALSE;

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Bezier Points");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 600);
   gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
   g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);
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
   gtk_widget_set_halign(check1, GTK_ALIGN_CENTER);
   g_signal_connect(check1, "toggled", G_CALLBACK(toggle_fade), da);

   GtkWidget *combo2=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo2, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Rotate 0");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Rotate pi/2");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Rotate 3*pi/4");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Rotate pi");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 4, "5", "Rotate 3*pi/2");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
   g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), da);

   GtkWidget *combo3=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo2, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "Gradient Stop 100%");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "Gradient Stop 75%");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "Gradient Stop 50%");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 3, "4", "Gradient Stop 25%");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);
   g_signal_connect(combo3, "changed", G_CALLBACK(combo3_changed), da);

   GtkWidget *label1=gtk_label_new("Start Color");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *label2=gtk_label_new("End Color");
   gtk_widget_set_hexpand(label2, TRUE);

   GtkWidget *entry1=gtk_entry_new();
   gtk_widget_set_hexpand(entry1, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry1), "rgba(0, 255, 0, 1.0)");

   GtkWidget *entry2=gtk_entry_new();
   gtk_widget_set_hexpand(entry2, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry2), "rgba(0, 0, 255, 1.0)");

   GtkWidget *button1=gtk_button_new_with_label("Update Colors");
   gtk_widget_set_halign(button1, GTK_ALIGN_CENTER);
   gtk_widget_set_hexpand(button1, FALSE);
   GtkWidget *colors[]={entry1, entry2, da};
   g_signal_connect(button1, "clicked", G_CALLBACK(check_colors), colors);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 1, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), check1, 0, 2, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo2, 0, 3, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo3, 0, 4, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 5, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry1, 1, 5, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label2, 0, 6, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry2, 1, 6, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 7, 2, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble r1=0;
   gdouble w1=0;

   //Scale.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   //Background.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr); 

   if(rotate_combo==0) r1=0;
   else if(rotate_combo==1) r1=G_PI/2.0;
   else if(rotate_combo==2) r1=3.0*G_PI/4.0;
   else if(rotate_combo==3) r1=G_PI;
   else r1=3.0*G_PI/2.0;

   if(drawing_combo==0) draw_circle(da, cr, -G_PI/2.0, 4, r1);
   else if(drawing_combo==1) draw_circle(da, cr, -G_PI/4.0, 8, r1);
   else if(drawing_combo==2) draw_circle(da, cr, -G_PI/8.0, 16, r1);
   else if(drawing_combo==3)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, 3.8*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/2.0, 4, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, 2.7*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr);
     }
   else if(drawing_combo==4)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, 3.8*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/4.0, 8, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, 2.7*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr);  
     }
   else
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, 3.8*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/8.0, 16, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, 2.7*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr); 
     }
   return FALSE;
 }
static void draw_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble rotation=-next_section;
   gdouble offset=-next_section/2.0;
   gdouble control_points[sections*4];
   gdouble w1=0;

   //Scale.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   //Draw in the center.
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_rotate(cr, r1); 
   
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
   for(i=0;i<sections;i++)
     {
       temp_cos1=cos(start-(next_section*(i+1)));
       temp_sin1=sin(start-(next_section*(i+1)));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;
       //The polar form of the equation for an ellipse to get the radius. Radius based on width.
       line_radius1=((2.5*w1)*(2.5*w1))/sqrt(((2.5*w1)*(2.5*w1)*temp_sin1*temp_sin1) + ((2.5*w1)*(2.5*w1)*temp_cos1*temp_cos1));
       line_radius2=((4.0*w1)*(4.0*w1))/sqrt(((4.0*w1)*(4.0*w1)*temp_sin1*temp_sin1) + ((4.0*w1)*(4.0*w1)*temp_cos1*temp_cos1));

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
        
       //Set up colors for the gradients.
       gdouble color_start1[3];
       gdouble color_stop1[3];
       gdouble diff0=color_stop[0]-color_start[0];
       gdouble diff1=color_stop[1]-color_start[1];
       gdouble diff2=color_stop[2]-color_start[2];
       color_start1[0]=color_start[0];
       color_start1[1]=color_start[1];
       color_start1[2]=color_start[2];
       color_stop1[0]=color_stop[0];
       color_stop1[1]=color_stop[1];
       color_stop1[2]=color_stop[2];
       if(fade)
         {   
           gint stop=(gint)(cutoff1*(gdouble)sections/100.0);
           if(i<stop)
             { 
               color_start1[0]=color_start[0]+(diff0*(gdouble)(i)/(gdouble)stop);
               color_start1[1]=color_start[1]+(diff1*(gdouble)(i)/(gdouble)stop);
               color_start1[2]=color_start[2]+(diff2*(gdouble)(i)/(gdouble)stop);
               color_stop1[0]=color_start[0]+(diff0*(gdouble)(i+1)/(gdouble)stop);
               color_stop1[1]=color_start[1]+(diff1*(gdouble)(i+1)/(gdouble)stop);
               color_stop1[2]=color_start[2]+(diff2*(gdouble)(i+1)/(gdouble)stop);
             }
           else
             {
               color_start1[0]=color_stop[0];
               color_start1[1]=color_stop[1];
               color_start1[2]=color_stop[2];
               color_stop1[0]=color_stop[0];
               color_stop1[1]=color_stop[1];
               color_stop1[2]=color_stop[2];
             }
         }

       //Draw the gradients.        
       cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
       cairo_mesh_pattern_begin_patch(pattern1);
       cairo_mesh_pattern_move_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_curve_to(pattern1, control_points[4*i], control_points[4*i+1],  control_points[4*i+2], control_points[4*i+3], temp_cos2, temp_sin2);
       cairo_mesh_pattern_line_to(pattern1, temp_cos1, temp_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos1, prev_sin1);
       cairo_mesh_pattern_line_to(pattern1, prev_cos2, prev_sin2);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, color_start1[0], color_start1[1], color_start1[2]);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2]);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2]);
       cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, color_start1[0], color_start1[1], color_start1[2]);
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
static void combo2_changed(GtkComboBox *combo2, gpointer data)
 {
   rotate_combo=gtk_combo_box_get_active(combo2);
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void combo3_changed(GtkComboBox *combo3, gpointer data)
 {
   gint row=gtk_combo_box_get_active(combo3);
   if(row==0) cutoff1=100.0;
   else if(row==1) cutoff1=75.0;
   else if(row==2) cutoff1=50.0;
   else cutoff1=25.0;
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void toggle_fade(GtkToggleButton *check1, gpointer data)
 {
   if(gtk_toggle_button_get_active(check1)) fade=TRUE;
   else fade=FALSE;
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void check_colors(GtkWidget *widget, GtkWidget **colors)
  {
    gint i=0;
    GdkRGBA rgba;

    for(i=0;i<2;i++)
      {
        if(gdk_rgba_parse(&rgba, gtk_entry_get_text(GTK_ENTRY(colors[i]))))
          {
            switch(i)
              {
                case 0:
                  color_start[0]=rgba.red;
                  color_start[1]=rgba.green;
                  color_start[2]=rgba.blue;
                  color_start[3]=rgba.alpha;
                  break;
                case 1:
                  color_stop[0]=rgba.red;
                  color_stop[1]=rgba.green;
                  color_stop[2]=rgba.blue;
                  color_stop[3]=rgba.alpha;
                  break;
             }
          }
        else
          {
            g_print("Color string format error in Entry %i\n", i);
          } 
      }

    gtk_widget_queue_draw(colors[2]);
  }

