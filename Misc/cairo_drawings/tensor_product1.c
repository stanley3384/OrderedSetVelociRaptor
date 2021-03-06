
/*
    Draw a tensor-product patch mesh. Start with a drawing area that has a grid to draw
the four meshes to start with. Then "tile" the meshes. Next, clip the tiled mesh region
in a circle and a fish. Scale and translate 4 fish to fit in original coordinates. Tile
again to create a school of fish. That is as far as it goes.
   
    gcc -Wall tensor_product1.c -o tensor_product1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

static void combo_changed(GtkComboBox *combo_box, gpointer data);
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data);
static void mesh_drawing(cairo_t *cr, gint width, gint height, gint drawing_id);
static void mesh_drawing_fish(cairo_t *cr, gint width, gint height);
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data);

int main(int argc, char **argv)
 {
   gtk_init(&argc, &argv);

   GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
   gtk_window_set_title(GTK_WINDOW(window), "Tensor Product Patch Mesh");
   gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
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
   g_signal_connect(window, "draw", G_CALLBACK(draw_background), NULL);

   GtkWidget *da=gtk_drawing_area_new();
   gtk_widget_set_hexpand(da, TRUE);
   gtk_widget_set_vexpand(da, TRUE);

   GtkWidget *combo=gtk_combo_box_text_new();
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "Patch Layout");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "Tile Patches");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "Clip Tiles Circle");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 3, "4", "Clip Tiles Fish");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 4, "5", "Four Fish");
   gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 5, "6", "School of Fish");
   gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 0);
   gtk_widget_set_hexpand(combo, TRUE);
   g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), da);
   g_signal_connect(da, "draw", G_CALLBACK(da_drawing), combo);
   
   GtkWidget *grid=gtk_grid_new();
   gtk_grid_attach(GTK_GRID(grid), da, 0, 0, 1, 1);
   gtk_grid_attach(GTK_GRID(grid), combo, 0, 1, 1, 1);
   
   gtk_container_add(GTK_CONTAINER(window), grid);

   gtk_widget_show_all(window);

   gtk_main();

   return 0;  
 }
static void combo_changed(GtkComboBox *combo_box, gpointer data)
 {
   gtk_widget_queue_draw(GTK_WIDGET(data));
 }
static gboolean da_drawing(GtkWidget *da, cairo_t *cr, gpointer data)
 {
   gint i=0;
   gint j=0;
   gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
   gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;
   gint combo_id=gtk_combo_box_get_active(GTK_COMBO_BOX(data));

   //Transparent background.
   cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 0.3);
   cairo_paint(cr);

   //Draw the combo selection.
   if(combo_id<4)
     {
       mesh_drawing(cr, width, height, combo_id);
     }
   else if(combo_id==4)
     {
       //Draw fish but scale and translate for 4 of them.
       cairo_scale(cr, 0.5, 0.5);
       for(i=0;i<2;i++)
         {
           for(j=0;j<2;j++)
             {
               cairo_save(cr);
               cairo_translate(cr, (pow(9.0, i))*w1, (pow(9.0, j))*h1);
               mesh_drawing(cr, width, height, combo_id);
               cairo_restore(cr);
             }
         }
     }
   else
     {
       //Tile the fish.
       cairo_scale(cr, 0.5, 0.5);
       cairo_scale(cr, 0.25, 0.25);
       cairo_translate(cr, 6.0*w1, 6.0*h1);
       for(i=0;i<8;i++)
         {
           for(j=0;j<8;j++)
             {
               cairo_save(cr);
               mesh_drawing(cr, width, height, combo_id);
               cairo_restore(cr);
               cairo_translate(cr, 8.0*w1, 0.0);
             }
           cairo_translate(cr, -64.0*w1, 8.0*h1);
         }
     }
   return FALSE;
 }
static void mesh_drawing(cairo_t *cr, gint width, gint height, gint drawing_id)
 {
   gint i=0;
   gint j=0;
   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //Clip tiles with a circle.
   cairo_save(cr);
   if(drawing_id==2)
     {
       cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
       cairo_arc(cr, width/2.0, height/2.0, 3.0*h1, 0.0, 2.0*G_PI);
       cairo_clip(cr);
     }

   //Clip tiles with a fish.
   if(drawing_id==3||drawing_id==4||drawing_id==5)
     {
       cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
       mesh_drawing_fish(cr, width, height);
       cairo_clip(cr);
     }

   //Scale and translate the drawings with many tiles.
   if(drawing_id!=0)
     {
       cairo_scale(cr, 0.25, 0.25);
       cairo_translate(cr, 3.0*w1, 3.0*h1);
     }

   //Patch1
   cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern1);
   cairo_mesh_pattern_move_to(pattern1, 1.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 4.0*w1, 2.0*h1, 6.0*w1, 4.0*h1, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern1, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern1, 2.0*w1, 4.0*h1, 0.0*w1, 2.0*h1, 1.0*w1, 1.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 1, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern1, 3, 0.0, 0.0, 1.0);
   //Pull purple into the center with a control point.
   cairo_mesh_pattern_set_control_point(pattern1, 1, 0.0*w1, 9.0*h1);
   cairo_mesh_pattern_end_patch(pattern1);
   
   //Patch2
   cairo_pattern_t *pattern2=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern2);
   cairo_mesh_pattern_move_to(pattern2, 5.0*w1, 1.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 9.0*w1, 1.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 8.0*w1, 2.0*h1, 10.0*w1, 4.0*h1, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern2, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern2, 6.0*w1, 4.0*h1, 4.0*w1, 2.0*h1, 5.0*w1, 1.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 1, 1.0, 0.0, 1.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern2, 3, 0.0, 0.0, 1.0);
   //Pull blue up to the right with a control point.
   cairo_mesh_pattern_set_control_point(pattern2, 3, 9.0*w1, -4.0*h1);
   cairo_mesh_pattern_end_patch(pattern2);

   //Patch3
   cairo_pattern_t *pattern3=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern3);
   cairo_mesh_pattern_move_to(pattern3, 1.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 4.0*w1, 6.0*h1, 6.0*w1, 8.0*h1, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern3, 1.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern3, 2.0*w1, 8.0*h1, 0.0*w1, 6.0*h1, 1.0*w1, 5.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern3, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch(pattern3);

   //Patch4
   cairo_pattern_t *pattern4=cairo_pattern_create_mesh();
   cairo_mesh_pattern_begin_patch(pattern4);
   cairo_mesh_pattern_move_to(pattern4, 5.0*w1, 5.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 9.0*w1, 5.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 8.0*w1, 6.0*h1, 10.0*w1, 8.0*h1, 9.0*w1, 9.0*h1);
   cairo_mesh_pattern_line_to(pattern4, 5.0*w1, 9.0*h1);
   cairo_mesh_pattern_curve_to(pattern4, 6.0*w1, 8.0*h1, 4.0*w1, 6.0*h1, 5.0*w1, 5.0*h1);   
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 0, 1.0, 0.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 1, 1.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 2, 0.0, 1.0, 0.0);
   cairo_mesh_pattern_set_corner_color_rgb(pattern4, 3, 0.0, 0.0, 1.0);
   cairo_mesh_pattern_end_patch(pattern4);

   //Draw the patterns in a grid or just draw the four patterns as is.
   if(drawing_id!=0)
     {
       for(i=0;i<4;i++)
         {
           for(j=0;j<4;j++)
             {
               cairo_set_source(cr, pattern1);
               cairo_paint(cr);
               cairo_set_source(cr, pattern2);
               cairo_paint(cr);
               cairo_set_source(cr, pattern3);
               cairo_paint(cr);
               cairo_set_source(cr, pattern4);
               cairo_paint(cr);
               cairo_translate(cr, 8.0*w1, 0.0);
             }
           cairo_translate(cr, -32.0*w1, 8.0*h1);
         }
     }
   else
     {
       cairo_set_source(cr, pattern1);
       cairo_paint(cr);
       cairo_set_source(cr, pattern2);
       cairo_paint(cr);
       cairo_set_source(cr, pattern3);
       cairo_paint(cr);
       cairo_set_source(cr, pattern4);
       cairo_paint(cr);
     }
   
   cairo_pattern_destroy(pattern1);
   cairo_pattern_destroy(pattern2);
   cairo_pattern_destroy(pattern3);
   cairo_pattern_destroy(pattern4);
   cairo_restore(cr);

   //Some highlights to help with drawing.
   if(drawing_id==0)
     {
       cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
       cairo_set_line_width(cr, 4.0);

       //Yellow curve1.
       cairo_move_to(cr, 1.0*w1, 1.0*h1);
       cairo_curve_to(cr, 0.0*w1, 2.0*h1, 2.0*w1, 4.0*h1, 1.0*w1, 5.0*h1);
       cairo_stroke_preserve(cr);
       cairo_curve_to(cr, 0.0*w1, 6.0*h1, 2.0*w1, 8.0*h1, 1.0*w1, 9.0*h1);
       cairo_stroke(cr);

       //Yellow curve2.
       cairo_move_to(cr, 5.0*w1, 1.0*h1);
       cairo_curve_to(cr, 4.0*w1, 2.0*h1, 6.0*w1, 4.0*h1, 5.0*w1, 5.0*h1);
       cairo_stroke_preserve(cr);
       cairo_curve_to(cr, 4.0*w1, 6.0*h1, 6.0*w1, 8.0*h1, 5.0*w1, 9.0*h1);
       cairo_stroke(cr);

       //Yellow curve3.
       cairo_move_to(cr, 9.0*w1, 1.0*h1);
       cairo_curve_to(cr, 8.0*w1, 2.0*h1, 10.0*w1, 4.0*h1, 9.0*w1, 5.0*h1);
       cairo_stroke_preserve(cr);
       cairo_curve_to(cr, 8.0*w1, 6.0*h1, 10.0*w1, 8.0*h1, 9.0*w1, 9.0*h1);
       cairo_stroke(cr);
     }

   if(drawing_id==0||drawing_id==4)
     {
       //Layout axis for drawing.
       cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
       cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
       cairo_stroke(cr);
       cairo_move_to(cr, 1.0*w1, 5.0*h1);
       cairo_line_to(cr, 9.0*w1, 5.0*h1);
       cairo_stroke(cr);
       cairo_move_to(cr, 5.0*w1, 1.0*h1);
       cairo_line_to(cr, 5.0*w1, 9.0*h1);
       cairo_stroke(cr);
     }

   //A smile and eye on the fish.
   if(drawing_id==3||drawing_id==4||drawing_id==5)
     {
       cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
       cairo_set_line_width(cr, 10.0);
       cairo_move_to(cr, 9.0*w1, 5.0*h1);
       cairo_curve_to(cr, 8.0*w1, 5.0*h1, 7.5*w1, 5.0*h1, 7.0*w1, 4.5*h1);
       cairo_stroke(cr);
       cairo_arc(cr, 7.5*w1, 3.0*h1, 0.25*h1, 0.0, 2.0*G_PI);
       cairo_fill(cr);  
     }

 }
static void mesh_drawing_fish(cairo_t *cr, gint width, gint height)
 {
   //Layout for the drawing is a 10x10 rectangle.
   gdouble w1=width/10.0;
   gdouble h1=height/10.0;

   //The outline for the fish.
   cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
   cairo_set_line_width(cr, 6.0);
   cairo_set_line_join(cr, CAIRO_LINE_JOIN_ROUND);
   cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
   cairo_move_to(cr, 1.0*w1, 3.0*h1);
   cairo_curve_to(cr, 2.0*w1, 3.0*h1, 2.0*w1, 3.0*h1, 3.0*w1, 4.5*h1);
   cairo_stroke_preserve(cr);  
   cairo_curve_to(cr, 4.0*w1, 2.0*h1, 4.0*w1, 2.0*h1, 6.0*w1, 2.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 8.0*w1, 2.0*h1, 8.0*w1, 2.0*h1, 9.0*w1, 5.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 8.0*w1, 8.0*h1, 8.0*w1, 8.0*h1, 6.0*w1, 8.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 4.0*w1, 8.0*h1, 4.0*w1, 8.0*h1, 3.0*w1, 5.5*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 2.0*w1, 7.0*h1, 2.0*w1, 7.0*h1, 1.0*w1, 7.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 2.0*w1, 6.0*h1, 2.0*w1, 6.0*h1, 2.0*w1, 5.0*h1);
   cairo_stroke_preserve(cr);
   cairo_curve_to(cr, 2.0*w1, 4.0*h1, 2.0*w1, 4.0*h1, 1.0*w1, 3.0*h1);
   cairo_stroke_preserve(cr);
   cairo_close_path(cr);
  
 }
static gboolean draw_background(GtkWidget *widget, cairo_t *cr, gpointer data)
 {
   //Draw background window transparent.
   cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
   cairo_paint(cr);
   return FALSE;
 }








