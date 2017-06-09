
/*
    Test drawing a gear with a few variables. Draw cartesian coordinates and circular coordinates
as part of the layout to help setup the gear drawing.

    gcc -Wall gears2.c -o gears2 `pkg-config gtk+-3.0 --cflags --libs` -lm

    Tested on Ubuntu16.04, GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<stdlib.h>
#include<math.h>

struct gear_vars{
  gdouble inside_radius;
  gdouble outside_radius;
  gdouble spindle_radius;
  gint teeth;
  gdouble bevel;
  gboolean fill;
  gdouble fill_color[4];
  gdouble rotation;
  gboolean draw_bezier;
}gear_vars;

//ID for the frame clock.
static guint tick_id=0;
//Show circular coordinates with the rectangular coordinates.
static gboolean show_circular_coords=FALSE;

static void update_gear(GtkWidget *button1, gpointer entries[]);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, struct gear_vars *g1);
static void draw_gears(GtkWidget *da, cairo_t *cr, struct gear_vars *g1);
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1, struct gear_vars *g1);
static void gear(cairo_t *cr, gdouble w1, struct gear_vars *g1);
static void spin_gear(GtkToggleButton *button, gpointer g2[]);
static gboolean animate_gear(GtkWidget *da, GdkFrameClock *frame_clock, gpointer g2[]);
static void circular_coordinates(GtkToggleButton *button, gpointer data);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Draw Gear");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 450);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    struct gear_vars g1;
    g1.inside_radius=3.0;
    g1.outside_radius=4.0;
    g1.spindle_radius=2.0;
    g1.teeth=12;
    g1.bevel=G_PI/36.0;
    g1.fill=FALSE;
    g1.fill_color[0]=0.0;
    g1.fill_color[1]=0.0;
    g1.fill_color[2]=1.0;
    g1.fill_color[3]=1.0;
    g1.rotation=0; 
    g1.draw_bezier=FALSE;   

    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    g_signal_connect(da, "draw", G_CALLBACK(da_drawing), &g1);

    GtkWidget *label1=gtk_label_new("Inside Tooth Radius ");
    gtk_widget_set_hexpand(label1, TRUE);

    GtkWidget *entry1=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry1), 4);
    gtk_widget_set_hexpand(entry1, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry1), "3.0"); 

    GtkWidget *label2=gtk_label_new("Outside Tooth Radius ");
    gtk_widget_set_hexpand(label2, TRUE);

    GtkWidget *entry2=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry2), 4);
    gtk_widget_set_hexpand(entry2, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry2), "4.0"); 

    GtkWidget *label3=gtk_label_new("Spindle Radius ");
    gtk_widget_set_hexpand(label3, TRUE);

    GtkWidget *entry3=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry3), 4);
    gtk_widget_set_hexpand(entry3, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry3), "2.0"); 

    GtkWidget *label4=gtk_label_new("Gear Teeth ");
    gtk_widget_set_hexpand(label4, TRUE);

    GtkWidget *entry4=gtk_entry_new();
    gtk_entry_set_width_chars(GTK_ENTRY(entry4), 4);
    gtk_widget_set_hexpand(entry4, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry4), "12"); 

    GtkWidget *label5=gtk_label_new("Gear Tooth Bevel ");
    gtk_widget_set_hexpand(label4, TRUE);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_widget_set_vexpand(combo1, FALSE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Pi/16.0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Pi/24.0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Pi/30.0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Pi/36.0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Pi/40.0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "Pi/46.0");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 3);

    GtkWidget *check1=gtk_check_button_new_with_label("Color Fill");
    gtk_widget_set_halign(check1, GTK_ALIGN_CENTER); 

    GtkWidget *label6=gtk_label_new("Fill Color ");
    gtk_widget_set_hexpand(label6, TRUE);

    GtkWidget *entry5=gtk_entry_new();
    gtk_widget_set_hexpand(entry5, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry5), "rgba(0, 0, 255, 1.0)");

    GtkWidget *check2=gtk_check_button_new_with_label("Show Circular Cordinates pi/30");
    gtk_widget_set_halign(check2, GTK_ALIGN_CENTER);
    g_signal_connect(check2, "toggled", G_CALLBACK(circular_coordinates), NULL);  

    GtkWidget *label7=gtk_label_new("Rotate Angle PI* ");
    gtk_widget_set_hexpand(label7, TRUE);

    GtkWidget *entry6=gtk_entry_new();
    gtk_widget_set_hexpand(entry6, FALSE);
    gtk_entry_set_text(GTK_ENTRY(entry6), "0");

    GtkWidget *check3=gtk_check_button_new_with_label("Draw Bezier");
    gtk_widget_set_halign(check3, GTK_ALIGN_CENTER);

    GtkWidget *button1=gtk_button_new_with_label("Update Gear");
    gtk_widget_set_halign(button1, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(button1, FALSE);
    gpointer entries[]={entry1, entry2, entry3, entry4, combo1, check1, entry5, entry6, check3, &g1, da};
    g_signal_connect(button1, "clicked", G_CALLBACK(update_gear), entries); 

    GtkWidget *button2=gtk_toggle_button_new_with_label("Spin Gear"); 
    gtk_widget_set_halign(button2, GTK_ALIGN_CENTER);
    gtk_widget_set_hexpand(button2, FALSE);
    gpointer g2[]={&g1, da};
    g_signal_connect(button2, "toggled", G_CALLBACK(spin_gear), g2); 
  
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
    gtk_grid_attach(GTK_GRID(grid), combo1, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check1, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), label6, 0, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry5, 1, 6, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label7, 0, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry6, 1, 7, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), check3, 0, 8, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), button2, 0, 10, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), check2, 0, 11, 2, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 400);
  
    gtk_container_add(GTK_CONTAINER(window), paned1);
    gtk_widget_show_all(window);

    gtk_main();

    return 0;
  }
static void update_gear(GtkWidget *button1, gpointer entries[])
  {
    GdkRGBA rgba;
    gboolean numbers_ok=TRUE;
    gdouble inside_radius=atof(gtk_entry_get_text(GTK_ENTRY(entries[0])));
    gdouble outside_radius=atof(gtk_entry_get_text(GTK_ENTRY(entries[1])));
    gdouble spindle_radius=atof(gtk_entry_get_text(GTK_ENTRY(entries[2])));
    gint teeth=atoi(gtk_entry_get_text(GTK_ENTRY(entries[3])));
    gint bevel=gtk_combo_box_get_active(GTK_COMBO_BOX(entries[4]));
    gboolean color_fill=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(entries[5]));
    gdouble rotation=atof(gtk_entry_get_text(GTK_ENTRY(entries[7])));
    gboolean bezier=gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(entries[8]));

    //gboolean from check boxes.
    ((struct gear_vars*)entries[9])->fill=color_fill;
    ((struct gear_vars*)entries[9])->draw_bezier=bezier;
    
    //Check and update numbers as a group.
    if(inside_radius>=outside_radius||inside_radius<0||inside_radius>=5.0)
      {
        g_print("Inside Radius out of range.\n");
        numbers_ok=FALSE;
      }    
    if(outside_radius<=inside_radius||outside_radius>5.0)
      {
        g_print("Outside Radius out of range.\n");
        numbers_ok=FALSE;
      }    
    if(spindle_radius>=inside_radius)
      {
        g_print("Spindle Radius out of range.\n");
        numbers_ok=FALSE;
      }  
    if(teeth<6||teeth>24)
      {
        g_print("Teeth out of range.");
        numbers_ok=FALSE;
      }
    if(numbers_ok)
      {
        ((struct gear_vars*)entries[9])->inside_radius=inside_radius;
        ((struct gear_vars*)entries[9])->outside_radius=outside_radius;
        ((struct gear_vars*)entries[9])->spindle_radius=spindle_radius;
        ((struct gear_vars*)entries[9])->teeth=teeth;
        ((struct gear_vars*)entries[9])->rotation=-rotation*G_PI;
      }

    //Bevel settings.
    switch(bevel)
      {
        case 0:
          ((struct gear_vars*)entries[9])->bevel=G_PI/16.0;
          break;
        case 1:
          ((struct gear_vars*)entries[9])->bevel=G_PI/24.0;
          break;
        case 2:
          ((struct gear_vars*)entries[9])->bevel=G_PI/30.0;
          break;
        case 3:
          ((struct gear_vars*)entries[9])->bevel=G_PI/36.0;
          break;
        case 4:
          ((struct gear_vars*)entries[9])->bevel=G_PI/40.0;
          break;
        case 5:
          ((struct gear_vars*)entries[9])->bevel=G_PI/46.0;
          break;
        default: g_print("Bevel out of range.\n");
      }
     
    //Color setting.             
    if(gdk_rgba_parse(&rgba, gtk_entry_get_text(GTK_ENTRY(entries[6]))))
      {
        ((struct gear_vars*)entries[9])->fill_color[0]=rgba.red;
        ((struct gear_vars*)entries[9])->fill_color[1]=rgba.green;
        ((struct gear_vars*)entries[9])->fill_color[2]=rgba.blue;
        ((struct gear_vars*)entries[9])->fill_color[3]=rgba.alpha;
      }
    else g_print("Color out of range.\n");

    gtk_widget_queue_draw(GTK_WIDGET(entries[10]));   
  }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, struct gear_vars *g1)
  {
    //GTimer *timer=g_timer_new();
    draw_gears(da, cr, g1);
    //g_print("%f\n", g_timer_elapsed(timer, NULL));
    //g_timer_destroy(timer); 
    return FALSE;
  }
static void draw_gears(GtkWidget *da, cairo_t *cr, struct gear_vars *g1)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    //Layout for the drawing is a 10x10 rectangle.
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Scale.
    if(w1>h1)
      {
        w1=h1;
      }
  
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);

    //Draw the gear or gears.
    gears1(cr, width, height, w1, g1); 

    //Cartesian coordinates for drawing.
    gint i=0;
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, width/10.0, height/10.0, 8.0*width/10.0, 8.0*height/10.0);
    cairo_stroke(cr);
    cairo_move_to(cr, 1.0*width/10.0, 5.0*height/10.0);
    cairo_line_to(cr, 9.0*width/10.0, 5.0*height/10.0);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*width/10.0, 1.0*height/10.0);
    cairo_line_to(cr, 5.0*width/10.0, 9.0*height/10.0);
    cairo_stroke(cr); 

    //Circular coordinates with marks at each second or pi/30.
    if(show_circular_coords)
      {
        cairo_translate(cr, width/2.0, height/2.0);
        cairo_arc(cr, 0.0, 0.0, 4.0*w1, 0.0, 2.0*M_PI);
        cairo_stroke(cr);
        gdouble start=-G_PI/2.0;
        gdouble next_second=-G_PI/30.0;
        gdouble temp_cos1=0;
        gdouble temp_sin1=0;
        gdouble temp_cos2=0;
        gdouble temp_sin2=0;
        for(i=0;i<60;i++)
          {
            temp_cos1=cos(start-(next_second*i));
            temp_sin1=sin(start-(next_second*i));
            temp_cos2=temp_cos1;
            temp_sin2=temp_sin1;
            temp_cos1=temp_cos1*3.6*w1;
            temp_sin1=temp_sin1*3.6*w1;
            temp_cos2=temp_cos2*4.0*w1;
            temp_sin2=temp_sin2*4.0*w1;
            cairo_move_to(cr, temp_cos1, temp_sin1);
            cairo_line_to(cr, temp_cos2, temp_sin2);
            cairo_stroke(cr);
         }
    }
  }
static void gears1(cairo_t *cr, gdouble width, gdouble height, gdouble w1, struct gear_vars *g1)
  {
    cairo_save(cr);
    cairo_set_source_rgb(cr, g1->fill_color[0], g1->fill_color[1], g1->fill_color[2]);
    cairo_set_line_width(cr, 5.0);
    cairo_translate(cr, width/2.0, height/2.0);
    cairo_rotate(cr, g1->rotation);

    //Draw the gear. 
    gear(cr, w1, g1);

    //Draw the inside circle of the gear.
    cairo_arc(cr, 0.0, 0.0, g1->spindle_radius*w1, 0, 2*G_PI);
    if(g1->fill)
      {
        cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
        cairo_fill(cr);
      }
    else cairo_stroke(cr);
   
    //The timing mark at 0 radians on the unit circle.
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    cairo_move_to(cr, g1->inside_radius*w1, 0.0);
    cairo_line_to(cr, g1->outside_radius*w1, 0.0);
    cairo_stroke(cr);  
    cairo_restore(cr);
  }
static void gear(cairo_t *cr, gdouble w1, struct gear_vars *g1)
  {
    gint i=0;
    gdouble step=-G_PI*2.0/(g1->teeth);

    //Outside points.
    gdouble outside_start1=-G_PI/2.0-(g1->bevel);
    gdouble outside_cos1=0;
    gdouble outside_sin1=0;
    gdouble outside_start2=-G_PI/2.0+(g1->bevel);
    gdouble outside_cos2=0;
    gdouble outside_sin2=0;
    //Inside points.
    gdouble inside_start1=-G_PI/2.0+G_PI/(g1->teeth)-(g1->bevel);
    gdouble inside_cos1=0;
    gdouble inside_sin1=0;
    gdouble inside_start2=-G_PI/2.0+G_PI/(g1->teeth)+(g1->bevel);
    gdouble inside_cos2=0;
    gdouble inside_sin2=0;
    //Draw the gear teeth.
    outside_cos1=cos(outside_start1-(step*i))*(g1->outside_radius)*w1;
    outside_sin1=sin(outside_start1-(step*i))*(g1->outside_radius)*w1;                
    cairo_move_to(cr, outside_cos1, outside_sin1);
    if(!g1->draw_bezier)
      {
        for(i=0;i<(g1->teeth);i++)
          {
            outside_cos2=cos(outside_start2-(step*i))*(g1->outside_radius)*w1;
            outside_sin2=sin(outside_start2-(step*i))*(g1->outside_radius)*w1;                
            cairo_line_to(cr, outside_cos2, outside_sin2);
            cairo_stroke_preserve(cr);   
         
            inside_cos1=cos(inside_start1-(step*i))*(g1->inside_radius)*w1;
            inside_sin1=sin(inside_start1-(step*i))*(g1->inside_radius)*w1;
            cairo_line_to(cr, inside_cos1, inside_sin1);
            cairo_stroke_preserve(cr); 

            inside_cos2=cos(inside_start2-(step*i))*(g1->inside_radius)*w1;
            inside_sin2=sin(inside_start2-(step*i))*(g1->inside_radius)*w1;
            cairo_line_to(cr, inside_cos2, inside_sin2);
            cairo_stroke_preserve(cr); 

            outside_cos1=cos(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;
            outside_sin1=sin(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;   
            cairo_line_to(cr, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);                           
         }
      }
    else
      {
        for(i=0;i<(g1->teeth);i++)
          {              
            outside_cos2=cos(outside_start2-(step*i))*(g1->outside_radius)*w1;
            outside_sin2=sin(outside_start2-(step*i))*(g1->outside_radius)*w1;                
            cairo_line_to(cr, outside_cos2, outside_sin2);
            cairo_stroke_preserve(cr);   
         
            inside_cos1=cos(inside_start1-(step*i))*(g1->inside_radius)*w1;
            inside_sin1=sin(inside_start1-(step*i))*(g1->inside_radius)*w1;

            inside_cos2=cos(inside_start2-(step*i))*(g1->inside_radius)*w1;
            inside_sin2=sin(inside_start2-(step*i))*(g1->inside_radius)*w1;

            outside_cos1=cos(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;
            outside_sin1=sin(outside_start1-(step*(i+1)))*(g1->outside_radius)*w1;   
            cairo_curve_to(cr, inside_cos1, inside_sin1, inside_cos2, inside_sin2, outside_cos1, outside_sin1);
            cairo_stroke_preserve(cr);                   
         }
     }
    cairo_close_path(cr);
    if(g1->fill) cairo_fill(cr); 
    else cairo_stroke(cr);
  }
static void spin_gear(GtkToggleButton *button, gpointer g2[])
  {
    if(gtk_toggle_button_get_active(button))
      {
        tick_id=gtk_widget_add_tick_callback(GTK_WIDGET(g2[1]), (GtkTickCallback)animate_gear, g2, NULL);
      }
    else
      {
        gtk_widget_remove_tick_callback(GTK_WIDGET(g2[1]), tick_id);
        tick_id=0;
        gtk_widget_queue_draw(GTK_WIDGET(g2[1]));
      }
  }
static gboolean animate_gear(GtkWidget *da, GdkFrameClock *frame_clock, gpointer g2[])
  {
    //Check frame rate.
    gint64 frame=gdk_frame_clock_get_frame_counter(frame_clock);
    if(frame%60==0&&frame>0)
      {
        gint64 current_time=gdk_frame_clock_get_frame_time(frame_clock);
        gint64 start = gdk_frame_clock_get_history_start(frame_clock);
        gint64 history_len=frame-start;
        GdkFrameTimings *previous_timings=gdk_frame_clock_get_timings(frame_clock, frame-history_len);
        gint64 previous_frame_time=gdk_frame_timings_get_frame_time(previous_timings);
        g_print("Frame %lld, %f fps\n", frame, (gdouble)(history_len)*G_USEC_PER_SEC/(gdouble)(current_time-previous_frame_time));
      }

    ((struct gear_vars*)g2[0])->rotation=((struct gear_vars*)g2[0])->rotation+G_PI/128;
    gtk_widget_queue_draw(GTK_WIDGET(g2[1]));

    return G_SOURCE_CONTINUE;
  }
static void circular_coordinates(GtkToggleButton *button, gpointer data)
  {
    if(gtk_toggle_button_get_active(button)) show_circular_coords=TRUE;
    else show_circular_coords=FALSE;
  }

