
/*   
    Use a smoothing function to get the Bezier control points for the gradient circle. The code
is clearer than the trapezoid version, circlar_gradient2.c. Drawing times are about the
same. The clipped trapezoid ring border looks smoother.

    gcc -Wall circular_gradient3.c -o circular_gradient3 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

struct point{
  gdouble x;
  gdouble y;
}points;

struct controls{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
}controls;

static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void draw_circle(GtkWidget *da, cairo_t *cr, gint sections, gdouble r1);
static void combo1_changed(GtkComboBox *combo1, gpointer data);
static void combo2_changed(GtkComboBox *combo2, gpointer data);
static void combo3_changed(GtkComboBox *combo3, gpointer data);
static void toggle_fade(GtkToggleButton *check1, gpointer data);
static void check_colors(GtkWidget *widget, GtkWidget **colors);
static void time_drawing(GtkToggleButton *check2, gpointer data);
static GArray* control_points_from_coords2(const GArray *dataPoints);

//Test a colors to start and stop the gradient.
static gdouble color_start[]={0.0, 1.0, 0.0, 1.0};
static gdouble color_stop[]={0.0, 0.0, 1.0, 1.0};
static gdouble cutoff1=100.0;
static gint drawing_combo=0;
static gint rotate_combo=0;
static gboolean fade=FALSE;
static gboolean time_it=FALSE;

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Smooth Points");
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

   GtkWidget *check2=gtk_check_button_new_with_label("Time Drawing");
   gtk_widget_set_halign(check2, GTK_ALIGN_CENTER);
   g_signal_connect(check2, "toggled", G_CALLBACK(time_drawing), NULL);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
   gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
   gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), check1, 0, 1, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo2, 0, 2, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), combo3, 0, 3, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), label1, 0, 4, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry1, 1, 4, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), label2, 0, 5, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), entry2, 1, 5, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), check2, 0, 6, 2, 1);
   gtk_grid_attach(GTK_GRID(grid), button1, 0, 7, 2, 1);

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
   gdouble r1=0;
   GTimer *timer=NULL;
 
   if(time_it==TRUE) timer=g_timer_new();

   //Background.
   cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 1.0);
   cairo_paint(cr); 

   if(rotate_combo==0) r1=0;
   else if(rotate_combo==1) r1=G_PI/2.0;
   else if(rotate_combo==2) r1=3.0*G_PI/4.0;
   else if(rotate_combo==3) r1=G_PI;
   else r1=3.0*G_PI/2.0;

   if(drawing_combo==0) draw_circle(da, cr, 4, r1);
   else if(drawing_combo==1) draw_circle(da, cr, 8, r1);
   else if(drawing_combo==2) draw_circle(da, cr, 16, r1);
   else draw_circle(da, cr, 24, r1);
  
   if(time_it==TRUE) 
     {
       g_print("Draw Time %f\n", g_timer_elapsed(timer, NULL));
       g_timer_destroy(timer);
     }

   return FALSE;
 }
static void draw_circle(GtkWidget *da, cairo_t *cr, gint sections, gdouble r1)
 {
   gint i=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=0;

   //Scale.
   if(width<height) w1=width/10.0;
   else w1=height/10.0;

   //Arrays for circle points.
   GArray *coords1=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *coords2=g_array_new(FALSE, FALSE, sizeof(struct point));
   GArray *cps1=NULL;
   GArray *cps2=NULL;

   //Get outside circle points.
   struct point circle;
   for(i=0;i<sections;i++)
     {
       circle.x=cos(2.0*G_PI*i/sections)*4.0*w1;
       circle.y=sin(2.0*G_PI*i/sections)*4.0*w1;
       g_array_append_val(coords1, circle);
     }
   //close the circle.
   circle.x=4.0*w1;
   circle.y=0.0;
   g_array_append_val(coords1, circle);

   //Get inside circle points.
   for(i=0;i<sections;i++)
     {
       circle.x=cos(2.0*G_PI*i/sections)*2.0*w1;
       circle.y=sin(2.0*G_PI*i/sections)*2.0*w1;
       g_array_append_val(coords2, circle);
     }
   //close the circle.
   circle.x=2.0*w1;
   circle.y=0.0;
   g_array_append_val(coords2, circle);

   //Get the Bezier points.
   cps1=control_points_from_coords2(coords1);
   cps2=control_points_from_coords2(coords2);

   //Draw in the center.
   cairo_translate(cr, width/2.0, height/2.0);
   cairo_rotate(cr, r1); 
     
   struct point outside1;
   struct point outside2;
   struct point inside1;
   struct point inside2;
   struct controls outside_c; 
   struct controls inside_c;  
   for(i=0;i<sections;i++)
     {
       outside1=g_array_index(coords1, struct point, i);
       outside2=g_array_index(coords1, struct point, i+1);
       inside1=g_array_index(coords2, struct point, i);
       inside2=g_array_index(coords2, struct point, i+1);
       outside_c=g_array_index(cps1, struct controls, i);
       inside_c=g_array_index(cps2, struct controls, i);       
        
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
       cairo_mesh_pattern_move_to(pattern1, outside1.x, outside1.y);
       cairo_mesh_pattern_curve_to(pattern1, outside_c.x1, outside_c.y1, outside_c.x2, outside_c.y2, outside2.x, outside2.y);
       cairo_mesh_pattern_line_to(pattern1, inside2.x, inside2.y);
       cairo_mesh_pattern_curve_to(pattern1, inside_c.x2, inside_c.y2, inside_c.x1, inside_c.y1, inside1.x, inside1.y);
       cairo_mesh_pattern_line_to(pattern1, outside1.x, outside1.y);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, color_start1[0], color_start1[1], color_start1[2], color_start[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, color_stop1[0], color_stop1[1], color_stop1[2], color_stop[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, color_stop1[0], color_stop1[1], color_stop1[2], color_stop[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, color_start1[0], color_start1[1], color_start1[2], color_start[3]);
       cairo_mesh_pattern_end_patch(pattern1);
       cairo_set_source(cr, pattern1);
       cairo_paint(cr);
       cairo_pattern_destroy(pattern1);                
     }

   if(coords1!=NULL) g_array_free(coords1, TRUE);
   if(cps1!=NULL) g_array_free(cps1, TRUE);
   if(coords2!=NULL) g_array_free(coords2, TRUE);
   if(cps2!=NULL) g_array_free(cps2, TRUE);
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
static void time_drawing(GtkToggleButton *check2, gpointer data)
  {
    if(gtk_toggle_button_get_active(check2)) time_it=TRUE;
    else time_it=FALSE;
  }
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=dataPoints->len-1;
    gdouble *fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
    gdouble *sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count==1)
      {
        struct point P0=g_array_index(dataPoints, struct point, 0);
        struct point P3=g_array_index(dataPoints, struct point, 1);

        //Calculate First Control Point
        //3P1 = 2P0 + P3
        struct point P1;
        P1.x=(2.0*P0.x+P3.x)/3.0;
        P1.y=(2.0*P0.y+P3.y)/3.0;

        *(fCP)=P1.x;
        *(fCP+1)=P1.y;

        //Calculate second Control Point
        //P2 = 2P1 - P0
        struct point P2;
        P2.x=(2.0*P1.x-P0.x);
        P2.y=(2.0*P1.y-P0.x);

        *(sCP)=P2.x;
        *(sCP+1)=P2.y;      
      }
    else
      {
        gdouble *rhs=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        gdouble *a=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *b=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble *c=(gdouble*)g_malloc(count*sizeof(gdouble));
        gdouble rhsValueX=0;
        gdouble rhsValueY=0;
        struct point P0;
        struct point P3;        
        gdouble m=0;
        gdouble b1=0;
        gdouble r2x=0;
        gdouble r2y=0;
        gdouble P1_x=0;
        gdouble P1_y=0;
   
        for(i=0;i<count;i++)
          {
            P0=g_array_index(dataPoints, struct point, i);
            P3=g_array_index(dataPoints, struct point, i+1);

            if(i==0)
              {
                *(a)=0.0;
                *(b)=2.0;
                *(c)=1.0;

                //rhs for first segment
                rhsValueX=P0.x+2.0*P3.x;
                rhsValueY=P0.y+2.0*P3.y;
              }
            else if(i==count-1)
              {
                *(a+i)=2.0;
                *(b+i)=7.0;
                *(c+i)=0.0;

                //rhs for last segment
                rhsValueX=8.0*P0.x+P3.x;
                rhsValueY=8.0*P0.y+P3.y;
              }
            else
              {
                *(a+i)=1.0;
                *(b+i)=4.0;
                *(c+i)=1.0;

                rhsValueX=4.0*P0.x+2.0*P3.x;
                rhsValueY=4.0*P0.y+2.0*P3.y;
              }
            *(rhs+i*2)=rhsValueX;
            *(rhs+i*2+1)=rhsValueY;
          }

        //Solve Ax=B. Use Tridiagonal matrix algorithm a.k.a Thomas Algorithm
        for(i=1;i<count;i++)
          {
            m=(*(a+i))/(*(b+i-1));

            b1=(*(b+i))-m*(*(c+i-1));
            *(b+i)=b1;

            r2x=(*(rhs+i*2))-m*(*(rhs+(i-1)*2));
            r2y=(*(rhs+i*2+1))-m*(*(rhs+(i-1)*2+1));

            *(rhs+i*2)=r2x;
            *(rhs+i*2+1)=r2y;
          }

        //Get First Control Points
        
        //Last control Point
        gdouble lastControlPointX=(*(rhs+2*count-2))/(*(b+count-1));
        gdouble lastControlPointY=(*(rhs+2*count-1))/(*(b+count-1));

        *(fCP+2*count-2)=lastControlPointX;
        *(fCP+2*count-1)=lastControlPointY;

        gdouble controlPointX=0;
        gdouble controlPointY=0;

        for(i=count-2;i>=0;--i)
          {
            controlPointX=(*(rhs+i*2)-(*(c+i))*(*(fCP+(i+1)*2)))/(*(b+i));
            controlPointY=(*(rhs+i*2+1)-(*(c+i))*(*(fCP+(i+1)*2+1)))/(*(b+i));

             *(fCP+i*2)=controlPointX;
             *(fCP+i*2+1)=controlPointY; 
          }

        //Compute second Control Points from first.
        for(i=0;i<count;i++)
          {
            if(i==count-1)
              {
                P3=g_array_index(dataPoints, struct point, i+1);
                P1_x=(*(fCP+i*2));
                P1_y=(*(fCP+i*2+1));

                controlPointX=(P3.x+P1_x)/2.0;
                controlPointY=(P3.y+P1_y)/2.0;

                *(sCP+count*2-2)=controlPointX;
                *(sCP+count*2-1)=controlPointY;
              }
            else
              {
                P3=g_array_index(dataPoints, struct point, i+1);                
                P1_x=(*(fCP+(i+1)*2));
                P1_y=(*(fCP+(i+1)*2+1));

                controlPointX=2.0*P3.x-P1_x;
                controlPointY=2.0*P3.y-P1_y;

                *(sCP+i*2)=controlPointX;
                *(sCP+i*2+1)=controlPointY;
              }

          }

        controlPoints=g_array_new(FALSE, FALSE, sizeof(struct controls));
        struct controls cp;
        for(i=0;i<count;i++)
          {
            cp.x1=(*(fCP+i*2));
            cp.y1=(*(fCP+i*2+1));
            cp.x2=(*(sCP+i*2));
            cp.y2=(*(sCP+i*2+1));
            g_array_append_val(controlPoints, cp);
          }

        g_free(rhs);
        g_free(a);
        g_free(b);
        g_free(c);
     }

    g_free(fCP);
    g_free(sCP);
    return controlPoints;
  }




