
/*   
    Remove the Bezier curves from circular_gradient1.c and just use the trapezoids to draw the ring.
A little easier and it should work fine if you have enough trapezoids to approximate a ring. Testing 
some gradient drawings to use in the adjustable gauge widget.

    gcc -Wall circular_gradient2.c -o circular_gradient2 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1);
static void draw_gradient(cairo_t *cr, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3, gdouble x4, gdouble y4, gdouble color_start1[], gdouble color_mid1[], gdouble color_stop1[], gint gradient_id);
static void combo1_changed(GtkComboBox *combo1, gpointer data);
static void combo2_changed(GtkComboBox *combo2, gpointer data);
static void combo3_changed(GtkComboBox *combo3, gpointer data);
static void combo4_changed(GtkComboBox *combo4, gpointer data);
static void combo5_changed(GtkComboBox *combo5, gpointer data);
static void combo6_changed(GtkComboBox *combo6, gpointer data);
static void toggle_fade(GtkToggleButton *check1, gpointer data);
static void time_drawing(GtkToggleButton *check2, gpointer data);
static void check_colors(GtkWidget *widget, GtkWidget **colors);
static void cycle_mid_color(GtkWidget *widget, GtkWidget *widgets[]);
static gboolean animate_color(GtkWidget *widgets[]);

//Test colors to start and stop the gradients. Set by UI entries.
static gdouble color_start[]={0.0, 1.0, 0.0, 1.0};
static gdouble color_mid[]={1.0, 1.0, 0.0, 1.0};
static gdouble color_stop[]={0.0, 0.0, 1.0, 1.0};
//Variables set by the UI combos and check boxes.
static gdouble mid_color_pos=100.0;
static gint drawing_combo=0;
static gint rotate_combo=0;
static gint skip_combo=0;
static gboolean fade=FALSE;
static gboolean time_it=FALSE;
static gdouble inside_radius=2;
static gdouble outside_radius=4;
//During timer save mid_color_pos.
static gdouble mid_color_saved=0;

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Trapezoid Ring");
   gtk_window_set_default_size(GTK_WINDOW(window), 800, 500);
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
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Draw 24 Sections");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Draw 32 Sections");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "Clip Ring 4");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 6, "7", "Clip Ring 8");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 7, "8", "Clip Ring 16");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 8, "9", "Clip Ring 24");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 9, "10", "Clip Ring 32");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
   g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), da);

   GtkWidget *check1=gtk_check_button_new_with_label("Fade Color");
   gtk_widget_set_halign(check1, GTK_ALIGN_CENTER);
   g_signal_connect(check1, "toggled", G_CALLBACK(toggle_fade), da);

   GtkWidget *check2=gtk_check_button_new_with_label("Time Drawing");
   gtk_widget_set_halign(check2, GTK_ALIGN_CENTER);
   g_signal_connect(check2, "toggled", G_CALLBACK(time_drawing), NULL);

   GtkWidget *combo2=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo2, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Rotate 0");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Rotate pi/2");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Rotate 2*pi/3");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Rotate 3*pi/4");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 4, "5", "Rotate pi");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 5, "6", "Rotate 3*pi/2");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
   g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), da);

   GtkWidget *combo3=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo3, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "No Mid Color");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "Mid Color Stop 90%");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "Mid Color Stop 75%");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 3, "4", "Mid Color Stop 50%");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 4, "5", "Mid Color Stop 25%");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo3), 0);
   g_signal_connect(combo3, "changed", G_CALLBACK(combo3_changed), da);

   GtkWidget *combo4=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo4, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 0, "1", "Skip 0");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 1, "2", "Skip 2");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 2, "3", "Skip 4");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo4), 0);
   g_signal_connect(combo4, "changed", G_CALLBACK(combo4_changed), da);

   GtkWidget *combo5=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo5, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 0, "1", "Inside Radius 1.0");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 1, "2", "Inside Radius 2.0");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo5), 2, "3", "Inside Radius 3.0");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo5), 1);
   g_signal_connect(combo5, "changed", G_CALLBACK(combo5_changed), da);

   GtkWidget *combo6=gtk_combo_box_text_new();
   gtk_widget_set_hexpand(combo6, TRUE);
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 0, "1", "Outside Radius 4.0");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 1, "2", "Outside Radius 4.5");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo6), 2, "3", "Outside Radius 5.0");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo6), 0);
   g_signal_connect(combo6, "changed", G_CALLBACK(combo6_changed), da);

   GtkWidget *label1=gtk_label_new("Start Color");
   gtk_widget_set_hexpand(label1, TRUE);

   GtkWidget *label2=gtk_label_new("Mid Color");
   gtk_widget_set_hexpand(label2, TRUE);

   GtkWidget *label3=gtk_label_new("End Color");
   gtk_widget_set_hexpand(label3, TRUE);

   GtkWidget *entry1=gtk_entry_new();
   gtk_widget_set_hexpand(entry1, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry1), "rgba(0, 255, 0, 1.0)");

   GtkWidget *entry2=gtk_entry_new();
   gtk_widget_set_hexpand(entry2, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry2), "rgba(255, 255, 0, 1.0)");

   GtkWidget *entry3=gtk_entry_new();
   gtk_widget_set_hexpand(entry3, TRUE);
   gtk_entry_set_text(GTK_ENTRY(entry3), "rgba(0, 0, 255, 1.0)");

   GtkWidget *button1=gtk_button_new_with_label("Update Colors");
   gtk_widget_set_halign(button1, GTK_ALIGN_CENTER);
   gtk_widget_set_hexpand(button1, FALSE);
   GtkWidget *colors[]={entry1, entry2, entry3, da};
   g_signal_connect(button1, "clicked", G_CALLBACK(check_colors), colors);

   GtkWidget *button2=gtk_button_new_with_label("Cycle Mid Color");
   gtk_widget_set_halign(button2, GTK_ALIGN_CENTER);
   gtk_widget_set_hexpand(button2, FALSE);
   GtkWidget *widgets[]={button2, da};
   g_signal_connect(button2, "clicked", G_CALLBACK(cycle_mid_color), widgets);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
   gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), check1, 0, 1, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo2, 0, 2, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo3, 0, 3, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo4, 0, 4, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo5, 0, 5, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo6, 0, 6, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), check2, 0, 7, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry1, 1, 8, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label2, 0, 9, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry2, 1, 9, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label3, 0, 10, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry3, 1, 10, 1, 1);   
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 11, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), button2, 0, 12, 2, 1);

   GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
   gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
   gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
   gtk_paned_set_position(GTK_PANED(paned1), 300);
   
   gtk_container_add(GTK_CONTAINER(window), paned1);

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
   GTimer *timer=NULL;
 
   if(time_it==TRUE) timer=g_timer_new();

   //Scale.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   //Background.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr); 

   if(rotate_combo==0) r1=0;
   else if(rotate_combo==1) r1=G_PI/2.0;
   else if(rotate_combo==2) r1=2.0*G_PI/3.0;
   else if(rotate_combo==3) r1=3.0*G_PI/4.0;
   else if(rotate_combo==4) r1=G_PI;
   else r1=3.0*G_PI/2.0;

   if(drawing_combo==0) draw_circle(da, cr, -G_PI/2.0, 4, r1);
   else if(drawing_combo==1) draw_circle(da, cr, -G_PI/4.0, 8, r1);
   else if(drawing_combo==2) draw_circle(da, cr, -G_PI/8.0, 16, r1);

   else if(drawing_combo==3) draw_circle(da, cr, -G_PI/12.0, 24, r1);

   else if(drawing_combo==4) draw_circle(da, cr, -G_PI/16.0, 32, r1);
   else if(drawing_combo==5)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, (outside_radius-0.2)*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/2.0, 4, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, (inside_radius+0.2)*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr);
     }
   else if(drawing_combo==6)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, (outside_radius-0.2)*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/4.0, 8, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, (inside_radius+0.2)*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr);  
     }
   else if(drawing_combo==7)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, (outside_radius-0.2)*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/8.0, 16, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, (inside_radius+0.2)*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr); 
     }
   else if(drawing_combo==8)
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, (outside_radius-0.2)*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/12.0, 24, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, (inside_radius+0.2)*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr); 
     }
   else
     {
       cairo_save(cr);
       cairo_arc(cr, width/2.0, height/2.0, (outside_radius-0.2)*w1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
       draw_circle(da, cr, -G_PI/16.0, 32, r1);
       cairo_restore(cr);
       cairo_arc(cr, width/2.0, height/2.0, (inside_radius+0.2)*w1, 0.0, 2.0*G_PI);
       cairo_fill(cr); 
     }

   if(time_it==TRUE) 
     {
       g_print("Draw Time %f\n", g_timer_elapsed(timer, NULL));
       g_timer_destroy(timer);
     }

   return FALSE;
 }
static void draw_circle(GtkWidget *da, cairo_t *cr, gdouble next_section, gint sections, gdouble r1)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=0;

   //Scale. Keep square.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   //Draw in the center.
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_rotate(cr, r1); 
   
   //Draw the trapezoids and paint them with a gradient.
   gdouble start=0.0;
   gdouble line_radius1=0;
   gdouble line_radius2=0;
   gdouble temp_cos1=0;
   gdouble temp_sin1=0;
   gdouble temp_cos2=0;
   gdouble temp_sin2=0;
   gdouble prev_cos1=inside_radius*w1;
   gdouble prev_sin1=0.0;
   gdouble prev_cos2=outside_radius*w1;
   gdouble prev_sin2=0.0;
   const gdouble ir=inside_radius*w1;
   const gdouble or=outside_radius*w1;
   //Colors and slopes for gradients.
   gdouble color_start1[3];
   gdouble color_mid1[3];
   gdouble color_stop1[3];
   gdouble diff0=0;
   gdouble diff1=0;
   gdouble diff2=0;
   gdouble diff3=0;
   gdouble diff4=0;
   gdouble diff5=0;
   //Adjust stop and sections for incomplete rings.
   sections=sections-(2*skip_combo);
   //For when a trapezoid needs to be split when the gradient color is inside the trapezoid.
   gdouble split_trap=(mid_color_pos/100.0)*sections;
   gdouble split_trap_int, split_trap_frac;  
   split_trap_frac=modf(split_trap, &split_trap_int);
   gdouble trap_cos1=0;
   gdouble trap_sin1=0;
   gdouble trap_cos2=0;
   gdouble trap_sin2=0;

   for(i=0;i<sections;i++)
     {
       temp_cos1=cos(start-(next_section*(i+1)));
       temp_sin1=sin(start-(next_section*(i+1)));
       temp_cos2=temp_cos1;
       temp_sin2=temp_sin1;

       //The polar form of the equation for an ellipse to get the radius. Radius based on width.
       line_radius1=((ir)*(ir))/sqrt(((ir)*(ir)*temp_sin1*temp_sin1)+((ir)*(ir)*temp_cos1*temp_cos1));
       line_radius2=((or)*(or))/sqrt(((or)*(or)*temp_sin1*temp_sin1) + ((or)*(or)*temp_cos1*temp_cos1));

       temp_cos1=temp_cos1*line_radius1;
       temp_sin1=temp_sin1*line_radius1;
       temp_cos2=temp_cos2*line_radius2;
       temp_sin2=temp_sin2*line_radius2;
        
       //Set up colors for the gradients.
       color_start1[0]=color_start[0];
       color_start1[1]=color_start[1];
       color_start1[2]=color_start[2];
       color_mid1[0]=color_mid[0];
       color_mid1[1]=color_mid[1];
       color_mid1[2]=color_mid[2];
       color_stop1[0]=color_stop[0];
       color_stop1[1]=color_stop[1];
       color_stop1[2]=color_stop[2];
       if(fade)
         {  
           //100 means there is no mid color in the drawing. Just draw start to end. 
           if(mid_color_pos==100)
             {
               diff0=color_stop[0]-color_start[0];
               diff1=color_stop[1]-color_start[1];
               diff2=color_stop[2]-color_start[2];
               color_start1[0]=color_start[0]+(diff0*(gdouble)(i)/(gdouble)sections);
               color_start1[1]=color_start[1]+(diff1*(gdouble)(i)/(gdouble)sections);
               color_start1[2]=color_start[2]+(diff2*(gdouble)(i)/(gdouble)sections);
               color_stop1[0]=color_start[0]+(diff0*(gdouble)(i+1)/(gdouble)sections);
               color_stop1[1]=color_start[1]+(diff1*(gdouble)(i+1)/(gdouble)sections);
               color_stop1[2]=color_start[2]+(diff2*(gdouble)(i+1)/(gdouble)sections);
             }
           else if(i<split_trap_int)
             { 
               diff0=color_mid[0]-color_start[0];
               diff1=color_mid[1]-color_start[1];
               diff2=color_mid[2]-color_start[2];
               color_start1[0]=color_start[0]+(diff0*(gdouble)(i)/split_trap);
               color_start1[1]=color_start[1]+(diff1*(gdouble)(i)/split_trap);
               color_start1[2]=color_start[2]+(diff2*(gdouble)(i)/split_trap);
               color_mid1[0]=color_start[0]+(diff0*(gdouble)(i+1)/split_trap);
               color_mid1[1]=color_start[1]+(diff1*(gdouble)(i+1)/split_trap);
               color_mid1[2]=color_start[2]+(diff2*(gdouble)(i+1)/split_trap);
             }
           else if(split_trap_int==i)
             { 
               diff0=color_mid[0]-color_start[0];
               diff1=color_mid[1]-color_start[1];
               diff2=color_mid[2]-color_start[2];
               diff3=color_stop[0]-color_mid[0];
               diff4=color_stop[1]-color_mid[1];
               diff5=color_stop[2]-color_mid[2];
               color_start1[0]=color_start[0]+(diff0*(gdouble)(i)/(gdouble)split_trap);
               color_start1[1]=color_start[1]+(diff1*(gdouble)(i)/(gdouble)split_trap);
               color_start1[2]=color_start[2]+(diff2*(gdouble)(i)/(gdouble)split_trap);

               color_mid1[0]=color_start[0]+(diff0*((gdouble)i+split_trap_frac)/split_trap);
               color_mid1[1]=color_start[1]+(diff1*((gdouble)i+split_trap_frac)/split_trap);
               color_mid1[2]=color_start[2]+(diff2*((gdouble)i+split_trap_frac)/split_trap);

               color_stop1[0]=color_mid[0]+(diff3*(1.0-split_trap_frac)/((gdouble)sections-split_trap));
               color_stop1[1]=color_mid[1]+(diff4*(1.0-split_trap_frac)/((gdouble)sections-split_trap));
               color_stop1[2]=color_mid[2]+(diff5*(1.0-split_trap_frac)/((gdouble)sections-split_trap));
             }
           else
             { 
               diff0=color_stop[0]-color_mid[0];
               diff1=color_stop[1]-color_mid[1];
               diff2=color_stop[2]-color_mid[2];
               color_mid1[0]=color_mid[0]+(diff0*((gdouble)i-split_trap)/((gdouble)sections-split_trap));
               color_mid1[1]=color_mid[1]+(diff1*((gdouble)i-split_trap)/((gdouble)sections-split_trap));
               color_mid1[2]=color_mid[2]+(diff2*((gdouble)i-split_trap)/((gdouble)sections-split_trap));
               color_stop1[0]=color_mid[0]+(diff0*((gdouble)i-split_trap+1.0)/((gdouble)sections-split_trap));
               color_stop1[1]=color_mid[1]+(diff1*((gdouble)i-split_trap+1.0)/((gdouble)sections-split_trap));
               color_stop1[2]=color_mid[2]+(diff2*((gdouble)i-split_trap+1.0)/((gdouble)sections-split_trap));
             }
         }

       if(mid_color_pos==100)
         {
           draw_gradient(cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 0);
         } 
       else if(i<split_trap_int)
         {  
           draw_gradient(cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 1);
         }
       else if(mid_color_pos<100&&(gint)split_trap_int==i)
         {
           trap_cos1=prev_cos1+split_trap_frac*(temp_cos1-prev_cos1);
           trap_sin1=prev_sin1+split_trap_frac*(temp_sin1-prev_sin1);
           trap_cos2=prev_cos2+split_trap_frac*(temp_cos2-prev_cos2);
           trap_sin2=prev_sin2+split_trap_frac*(temp_sin2-prev_sin2);
           draw_gradient(cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, trap_cos1, trap_sin1, trap_cos2, trap_sin2, color_start1, color_mid1, color_stop1, 1);
           draw_gradient(cr, trap_cos1, trap_sin1, trap_cos2, trap_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 2);
         } 
       else
         {
           draw_gradient(cr, prev_cos1, prev_sin1, prev_cos2, prev_sin2, temp_cos1, temp_sin1, temp_cos2, temp_sin2, color_start1, color_mid1, color_stop1, 2);
         }   

       //The trapezoid that gets split.
       if(mid_color_pos<100&&(gint)split_trap_int==i)
         {
           cairo_set_source_rgb(cr, 0.0, 1.0, 1.0);
           cairo_set_line_width(cr, 3.0); 
           cairo_move_to(cr, trap_cos1, trap_sin1);
           cairo_line_to(cr, trap_cos2, trap_sin2);
           cairo_stroke(cr);
         }        

       //Trapezoid polygon
       if(drawing_combo<5)
         {
           cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
           cairo_set_line_width(cr, 3.0); 
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
static void draw_gradient(cairo_t *cr, gdouble x1, gdouble y1, gdouble x2, gdouble y2, gdouble x3, gdouble y3, gdouble x4, gdouble y4, gdouble color_start1[], gdouble color_mid1[], gdouble color_stop1[], gint gradient_id)
 {
   //Draw the gradients.    
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, x2, y2);
   cairo_mesh_pattern_line_to(pattern1, x4, y4);
   cairo_mesh_pattern_line_to(pattern1, x3, y3);
   cairo_mesh_pattern_line_to(pattern1, x1, y1);
   cairo_mesh_pattern_line_to(pattern1, x2, y2);

   //Draw the gradient across the whole arc.
   if(gradient_id==0)
     {
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_start1[0], color_start1[1], color_start1[2], color_start[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2], color_stop[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2], color_stop[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_start1[0], color_start1[1], color_start1[2], color_start[3]);
     }
   //Draw the gradient to the mid point.
   else if(gradient_id==1)
     {
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_start1[0], color_start1[1], color_start1[2], color_start[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_mid1[0], color_mid1[1], color_mid1[2], color_mid[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_mid1[0], color_mid1[1], color_mid1[2], color_mid[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_start1[0], color_start1[1], color_start1[2], color_start[3]);
     }
   //Draw the gradient from the mid point to the end.
   else
     {
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_mid1[0], color_mid1[1], color_mid1[2], color_mid[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2], color_stop[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2], color_stop[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_mid1[0], color_mid1[1], color_mid1[2], color_mid[3]);
     }
   cairo_mesh_pattern_end_patch(pattern1);
   cairo_set_source(cr, pattern1);
   cairo_paint(cr);
   cairo_pattern_destroy(pattern1); 
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
   if(row==0) mid_color_pos=100.0;
   else if(row==1) mid_color_pos=90.0;
   else if(row==2) mid_color_pos=75.0;
   else if(row==3) mid_color_pos=50.0;
   else mid_color_pos=25.0;
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void combo4_changed(GtkComboBox *combo4, gpointer data)
 {
   skip_combo=gtk_combo_box_get_active(combo4);
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void combo5_changed(GtkComboBox *combo5, gpointer data)
 {
   inside_radius=gtk_combo_box_get_active(combo5)+1;
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void combo6_changed(GtkComboBox *combo6, gpointer data)
 {
   outside_radius=gtk_combo_box_get_active(combo6)+4;
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void toggle_fade(GtkToggleButton *check1, gpointer data)
 {
   if(gtk_toggle_button_get_active(check1)) fade=TRUE;
   else fade=FALSE;
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static void time_drawing(GtkToggleButton *check2, gpointer data)
 {
   if(gtk_toggle_button_get_active(check2)) time_it=TRUE;
   else time_it=FALSE;
 }
static void check_colors(GtkWidget *widget, GtkWidget **colors)
  {
    gint i=0;
    GdkRGBA rgba;

    for(i=0;i<3;i++)
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
                  color_mid[0]=rgba.red;
                  color_mid[1]=rgba.green;
                  color_mid[2]=rgba.blue;
                  color_mid[3]=rgba.alpha;
                  break;
               case 2:
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

    gtk_widget_queue_draw(colors[3]);
  }
static void cycle_mid_color(GtkWidget *widget, GtkWidget *widgets[])
  {
    gtk_widget_set_sensitive(widget, FALSE);
    mid_color_saved=mid_color_pos;
    mid_color_pos=0;
    g_timeout_add(100, (GSourceFunc)animate_color, widgets);
  }
static gboolean animate_color(GtkWidget *widgets[])
 {
   if(mid_color_pos<100)
     {
       mid_color_pos++;
       gtk_widget_queue_draw(widgets[1]);
       return TRUE;
     }
   else
     {
       mid_color_pos=mid_color_saved;
       gtk_widget_queue_draw(widgets[1]);
       gtk_widget_set_sensitive(widgets[0], TRUE);
       return FALSE;
     }
 }

