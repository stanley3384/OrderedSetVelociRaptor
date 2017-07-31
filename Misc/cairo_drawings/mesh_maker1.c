
/*
    Draw a mesh that fits together like tiles. Be able to dynamically change the curves, colors and
control points. Similar to tensor_product1.c but you can draw the mesh dynamically. 
Draw a t-shirt, fish and butterfly with the tiled mesh pattern.

    gcc -Wall mesh_maker1.c -o mesh_maker1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_grids(GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_grid1(cairo_t *cr, gdouble width, gdouble height);
static void draw_grid2(cairo_t *cr, gdouble width, gdouble height);
static void draw_grid3(cairo_t *cr, gdouble width, gdouble height);
static void draw_shapes(GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_mesh(cairo_t *cr, gdouble width, gdouble height);
static void draw_t_shirt(cairo_t *cr, gdouble width, gdouble height);
static void draw_fish(cairo_t *cr, gdouble width, gdouble height);
static void draw_butterfly(cairo_t *cr, gdouble width, gdouble height);
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data);
static void combo1_changed(GtkComboBox *combo1, gpointer data);
static void combo2_changed(GtkComboBox *combo2, gpointer data);
static void combo3_changed(GtkComboBox *combo3, gpointer data);
static void combo4_changed(GtkComboBox *combo4, gpointer data);
static void check_colors(GtkWidget *widget, GtkWidget **colors);
static gboolean draw_main_window(GtkWidget *widget, cairo_t *cr, gpointer data);
//About dialog and icon.
static void about_dialog(GtkWidget *widget, gpointer data);
static GdkPixbuf* draw_icon();

//For blocking motion signal. Block when not drawing top rectangle
static gint motion_id=0;
//Coordinates for the drawing rectangle.
static gdouble rect[]={0.0, 0.0, 0.0, 0.0};
//Control points for the "box" lines.
static gdouble mesh[]={1.0, 3.0, 3.0, 4.0, 3.0, 3.0, 4.0, 1.0};
//Inside control points P0, P1, P2, P3.
static gdouble mesh_p[]={6.0, 3.0, 7.0, 3.0, 6.0, 4.0, 7.0, 4.0};
//Combo row.
static gint mesh_combo=0;
static gint tile_combo=0;
static gint drawing_combo=0;
static gboolean grid_combo=TRUE;
static gint scale_combo=3;
static gboolean default_drawing=TRUE;
//Drawing background color.
static gdouble b1[]={1.0, 1.0, 1.0, 1.0};
//Bezier curve control point colors.
static gdouble c0[]={1.0, 0.0, 0.0, 1.0};
static gdouble c1[]={1.0, 0.0, 1.0, 1.0};
static gdouble c2[]={0.0, 1.0, 0.0, 1.0};
static gdouble c3[]={0.0, 0.0, 1.0, 1.0};

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    GtkWidget *window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Mesh Maker");
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

    //Draw a 256x256 program icon. A butterfly with 2x2 mesh tiles.
    grid_combo=FALSE;
    rect[2]=0.1*256.0;
    rect[3]=0.3*256.0;
    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);
    grid_combo=TRUE;
    
    GtkWidget *da=gtk_drawing_area_new();
    gtk_widget_set_hexpand(da, TRUE);
    gtk_widget_set_vexpand(da, TRUE);
    //Add some extra events to the top drawing area.
    gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK);
    g_signal_connect(da, "draw", G_CALLBACK(start_drawing), NULL);
    g_signal_connect(da, "button-press-event", G_CALLBACK(start_press), NULL);
    g_signal_connect(da, "button-release-event", G_CALLBACK(stop_press), NULL);
    motion_id=g_signal_connect(da, "motion-notify-event", G_CALLBACK(cursor_motion), NULL);
    g_signal_handler_block(da, motion_id);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_widget_set_vexpand(combo1, FALSE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Drag Point 1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Drag Point 2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Drag Point 3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 3, "4", "Drag Point 4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 4, "5", "Drag Point A");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 5, "6", "Drag Point B");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 6, "7", "Drag Point C");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 7, "8", "Drag Point D");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 0);
    g_signal_connect(combo1, "changed", G_CALLBACK(combo1_changed), NULL);

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_widget_set_vexpand(combo2, FALSE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "Draw Tiled Mesh");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Tiled Mesh 2x2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Tiled Mesh 4x4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Tiled Mesh 8x8");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
    g_signal_connect(combo2, "changed", G_CALLBACK(combo2_changed), da);

    GtkWidget *combo3=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo3, TRUE);
    gtk_widget_set_vexpand(combo3, FALSE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 0, "1", "Draw t-shirt");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 1, "2", "Draw Fish");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo3), 2, "3", "Draw Butterfly");
    g_signal_connect(combo3, "changed", G_CALLBACK(combo3_changed), da);

    GtkWidget *combo4=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo4, TRUE);
    gtk_widget_set_vexpand(combo4, FALSE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 0, "1", "Scale 0.5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 1, "2", "Scale 0.75");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 2, "3", "Scale 0.875");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 3, "4", "Scale 1.0");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 4, "5", "Scale 1.125");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 5, "6", "Scale 1.25");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo4), 6, "7", "Scale 1.5");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo4), 3);
    g_signal_connect(combo4, "changed", G_CALLBACK(combo4_changed), da);

    GtkWidget *label1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label1), "<span font_weight='heavy'> C0 </span>");
    GtkWidget *label2=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label2), "<span font_weight='heavy'> C1 </span>");
    GtkWidget *label3=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label3), "<span font_weight='heavy'> C2 </span>");
    GtkWidget *label4=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label4), "<span font_weight='heavy'> C3 </span>");
    GtkWidget *label5=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label5), "<span font_weight='heavy'>Background </span>");

    GtkWidget *entry1=gtk_entry_new();
    gtk_widget_set_hexpand(entry1, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry1), "rgba(255, 0, 0, 1.0)");

    GtkWidget *entry2=gtk_entry_new();
    gtk_widget_set_hexpand(entry2, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry2), "rgba(255, 0, 255, 1.0)");

    GtkWidget *entry3=gtk_entry_new();
    gtk_widget_set_hexpand(entry3, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry3), "rgba(0, 255, 0, 1.0)");

    GtkWidget *entry4=gtk_entry_new();
    gtk_widget_set_hexpand(entry4, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry4), "rgba(0, 0, 255, 1.0)");

    GtkWidget *entry5=gtk_entry_new();
    gtk_widget_set_hexpand(entry5, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry5), "rgba(255, 255, 255, 1.0)");

    GtkWidget *button1=gtk_button_new_with_label("Update Colors");
    gtk_widget_set_hexpand(button1, FALSE);
    GtkWidget *colors[]={entry1, entry2, entry3, entry4, entry5, window, da};
    g_signal_connect(button1, "clicked", G_CALLBACK(check_colors), colors);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Mesh Maker");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);

    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);

    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);
    
    GtkWidget *grid=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
    gtk_grid_attach(GTK_GRID(grid), combo1, 0, 0, 2, 1);    
    gtk_grid_attach(GTK_GRID(grid), label5, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry5, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry1, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label2, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry2, 1, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label3, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry3, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), label4, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), entry4, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 6, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), combo2, 0, 7, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), combo4, 0, 8, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), combo3, 0, 9, 2, 1);
    gtk_grid_attach(GTK_GRID(grid), menu_bar, 0, 10, 1, 1);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), grid, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 300);

    //Draw background window based on the paned window splitter.
    g_signal_connect(window, "draw", G_CALLBACK(draw_main_window), paned1);

    gtk_container_add(GTK_CONTAINER(window), paned1);

    gtk_widget_show_all(window);

    //Set some initial values for the drawing area.
    gdouble width=(gdouble)gtk_widget_get_allocated_width(da);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(da);
    rect[2]=0.1*width;
    rect[3]=0.3*height;
    gtk_widget_queue_draw(da);

    gtk_main();

    g_object_unref(icon);

    return 0;
  }
static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);

    //Scale the drawing. Exclude "drawing mesh" from scaling.
    if(!default_drawing)
      {
        if(scale_combo==0)
          {
            cairo_translate(cr, 0.25*width, 0.25*height);
            cairo_scale(cr, 0.5, 0.5);
          }
        else if(scale_combo==1)
          {
            cairo_translate(cr, 0.125*width, 0.125*height);
            cairo_scale(cr, 0.75, 0.75);
          }
        else if(scale_combo==2)
          {
            cairo_translate(cr, 0.0625*width, 0.0625*height);
            cairo_scale(cr, 0.875, 0.875);
          }
        else if(scale_combo==4)
          {
            cairo_translate(cr, -0.0625*width, -0.0625*height);
            cairo_scale(cr, 1.125, 1.125);
          }
        else if(scale_combo==5)
          {
            cairo_translate(cr, -0.125*width, -0.125*height);
            cairo_scale(cr, 1.25, 1.25);
          }
        else if(scale_combo==6)
          {
            cairo_translate(cr, -0.25*width, -0.25*height);
            cairo_scale(cr, 1.5, 1.5);
          }
        else
          {
            //Don't scale. scale_combo==3
          }
          
      }

    if(grid_combo) draw_grids(widget, cr, data);
    else draw_shapes(widget, cr, data);
    return TRUE;
  }
static void draw_grids(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);

    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);

    if(tile_combo==0)
      {
        draw_mesh(cr, width, height);
      }
    else if(tile_combo==1)
      {
        draw_grid1(cr, width, height); 
      }
    else if(tile_combo==2)
      {
        draw_grid2(cr, width, height);
      }
    else
      {
        draw_grid3(cr, width, height);
      }
  }
static void draw_grid1(cairo_t *cr, gdouble width, gdouble height)
  {
    gint i=0;
    gint j=0;
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_scale(cr, 0.5, 0.5);
    cairo_translate(cr, 2.0*w1, 2.0*h1);
    for(i=0;i<2;i++)
      {
        for(j=0;j<2;j++)
          {
            draw_mesh(cr, width, height);
            cairo_translate(cr, 6.0*w1, 0.0);
          }
        cairo_translate(cr, -12.0*w1, 6.0*h1);
      }
  }
static void draw_grid2(cairo_t *cr, gdouble width, gdouble height)
  {
    gint i=0;
    gint j=0;
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_scale(cr, 0.25, 0.25);
    cairo_translate(cr, 6.0*w1, 6.0*h1);
    for(i=0;i<4;i++)
      {
        for(j=0;j<4;j++)
          {
            draw_mesh(cr, width, height);
            cairo_translate(cr, 6.0*w1, 0.0);
          }
        cairo_translate(cr, -24.0*w1, 6.0*h1);
      }
  }
static void draw_grid3(cairo_t *cr, gdouble width, gdouble height)
  {
    gint i=0;
    gint j=0;
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_scale(cr, 0.125, 0.125);
    cairo_translate(cr, 14.0*w1, 14.0*h1);
    for(i=0;i<8;i++)
      {
        for(j=0;j<8;j++)
          {
            draw_mesh(cr, width, height);
            cairo_translate(cr, 6.0*w1, 0.0);
          }
        cairo_translate(cr, -48.0*w1, 6.0*h1);
      }
  }
static void draw_shapes(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    cairo_save(cr);
    if(drawing_combo==0) draw_t_shirt(cr, width, height);
    else if(drawing_combo==1) draw_fish(cr, width, height);
    else draw_butterfly(cr, width, height);
    cairo_restore(cr);
    cairo_clip(cr);

    //Scale the mesh a little try to cover the drawing.
    cairo_scale(cr, 1.2, 1.2);
    cairo_translate(cr, -0.8*w1, -0.8*h1);
    if(tile_combo==0)
      {
        draw_mesh(cr, width, height);
      }
    else if(tile_combo==1)
      {
        draw_grid1(cr, width, height); 
      }
    else if(tile_combo==2)
      {
        draw_grid2(cr, width, height);
      }
    else
      {
        draw_grid3(cr, width, height);
      }      
  }
static void draw_mesh(cairo_t *cr, gdouble width, gdouble height)
  {
    gint i=0;
    gint j=0;
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;
    gdouble p1=(rect[2]/width)*10.0;
    gdouble p2=(rect[3]/height)*10.0;

    //Save the mesh points.
    //g_print("x %f y %f x %f y %f\n", (rect[2]/width)*10.0, (rect[3]/height)*10.0, rect[2], rect[3]);
    if(mesh_combo==0)
      {
        mesh[0]=p1;
        mesh[1]=p2;
      }
    else if(mesh_combo==1)
      {
        mesh[2]=p1;
        mesh[3]=p2;
      }
    else if(mesh_combo==2)
      {
        mesh[4]=p1;
        mesh[5]=p2;
      }
    else if(mesh_combo==3)
      {
        mesh[6]=p1;
        mesh[7]=p2;
      }
    //The inside control points.
    else if(mesh_combo==4)
      {
        mesh_p[0]=p1;
        mesh_p[1]=p2;
      }
    else if(mesh_combo==5)
      {
        mesh_p[2]=p1;
        mesh_p[3]=p2;
      }
    else if(mesh_combo==6)
      {
        mesh_p[4]=p1;
        mesh_p[5]=p2;
      }
    else
      {
        mesh_p[6]=p1;
        mesh_p[7]=p2;
      } 
    cairo_set_line_width(cr, 4);
 
    //Four gradient patches.
    for(i=0;i<4;i++)
      {
        cairo_save(cr);
        if(i==1) cairo_translate(cr, 3.0*w1, 0.0);
        if(i==2) cairo_translate(cr, 0.0*w1, 3.0*h1);
        if(i==3) cairo_translate(cr, 3.0*w1, 3.0*h1);
        cairo_pattern_t *pattern1=cairo_pattern_create_mesh();
        cairo_mesh_pattern_begin_patch(pattern1);
        cairo_mesh_pattern_move_to(pattern1, 2.0*w1, 2.0*h1);
        cairo_mesh_pattern_curve_to(pattern1, mesh[4]*w1, mesh[5]*h1, mesh[6]*w1, mesh[7]*h1, 5.0*w1, 2.0*h1);
        cairo_mesh_pattern_curve_to(pattern1, mesh[0]*w1+3.0*w1, mesh[1]*h1, mesh[2]*w1+3.0*w1, mesh[3]*h1, 5.0*w1, 5.0*h1);
        cairo_mesh_pattern_curve_to(pattern1, mesh[6]*w1, mesh[7]*h1+3.0*h1, mesh[4]*w1, mesh[5]*h1+3.0*h1, 2.0*w1, 5.0*h1);
       cairo_mesh_pattern_curve_to(pattern1, mesh[2]*w1, mesh[3]*h1, mesh[0]*w1, mesh[1]*h1, 2.0*w1, 2.0*h1);   
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 0, c0[0], c0[1], c0[2], c0[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 1, c1[0], c1[1], c1[2], c1[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 2, c2[0], c2[1], c2[2], c2[3]);
       cairo_mesh_pattern_set_corner_color_rgba(pattern1, 3, c3[0], c3[1], c3[2], c3[3]);
       /*
           From https://www.adobe.com/products/postscript/pdfs/PLRM.pdf P.286
           Default positions of inside control points.
           P11=S(1/3, 2/3)
           P12=S(2/3, 2/3)
           P21=S(1/3, 1/3)
           P22=S(2/3, 1/3)
           The labels A, B, C and D are a little different but in the same 1/3 and 2/3 positions.
       */
       cairo_mesh_pattern_set_control_point(pattern1, 0, mesh_p[0]*w1-3.0*w1, mesh_p[1]*h1);
       cairo_mesh_pattern_set_control_point(pattern1, 1, mesh_p[2]*w1-3.0*w1, mesh_p[3]*h1);
       cairo_mesh_pattern_set_control_point(pattern1, 2, mesh_p[4]*w1-3.0*w1, mesh_p[5]*h1);
       cairo_mesh_pattern_set_control_point(pattern1, 3, mesh_p[6]*w1-3.0*w1, mesh_p[7]*h1);
       cairo_mesh_pattern_end_patch(pattern1);

       cairo_set_source(cr, pattern1);
       cairo_paint(cr);
       cairo_pattern_destroy(pattern1);
       cairo_restore(cr);
     }

    cairo_set_line_width(cr, 6);    
    //Layout axis for drawing mesh.
    if(tile_combo==0&&grid_combo)
      {
        cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
        cairo_rectangle(cr, 2.0*w1, 2.0*h1, 6.0*w1, 6.0*h1);
        cairo_stroke(cr);
        cairo_move_to(cr, 2.0*w1, 5.0*h1);
        cairo_line_to(cr, 8.0*w1, 5.0*h1);
        cairo_stroke(cr);
        cairo_move_to(cr, 5.0*w1, 2.0*h1);
        cairo_line_to(cr, 5.0*w1, 8.0*h1);
        cairo_stroke(cr);
      }

    //Add layout lines for initial drawing.
     if(tile_combo==0&&grid_combo)
      {
        //6 vertical Bezier curves over layout axis.
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_save(cr);
        for(i=0;i<2;i++)
          {
            for(j=0;j<3;j++)
              {
                cairo_move_to(cr, 2.0*w1, 2.0*h1);
                //cairo_curve_to(cr, 1.0*w1, 3.0*h1, 3.0*w1, 4.0*h1, 2.0*w1, 5.0*h1);
                cairo_curve_to(cr, mesh[0]*w1, mesh[1]*h1, mesh[2]*w1, mesh[3]*h1, 2.0*w1, 5.0*h1);
                cairo_stroke(cr);
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_translate(cr, 3.0*w1, 0.0);
              }
            cairo_translate(cr, -9.0*w1, 3.0*h1);
          }
        cairo_restore(cr);

        //6 horizontal Bezier curves over layout axis.
        cairo_set_source_rgb(cr, 1.0, 1.0, 0.0);
        cairo_save(cr);
        for(i=0;i<2;i++)
          {
            for(j=0;j<3;j++)
              {
                cairo_move_to(cr, 2.0*w1, 2.0*h1);
                //cairo_curve_to(cr, 3.0*w1, 3.0*h1, 4.0*w1, 1.0*h1, 5.0*w1, 2.0*h1);
                cairo_curve_to(cr, mesh[4]*w1, mesh[5]*h1, mesh[6]*w1, mesh[7]*h1, 5.0*w1, 2.0*h1);
                cairo_stroke(cr);
                cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
                cairo_translate(cr, 0.0, 3.0*h1);
              }
           cairo_translate(cr, 3.0*w1, -9.0*h1);
          }
        cairo_restore(cr);
      }

    if(tile_combo==0&&grid_combo)
      {
        cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 12);    
        cairo_set_source_rgb(cr, 0.0 , 0.0 , 0.0);
        cairo_move_to(cr, mesh[0]*width/10.0, mesh[1]*height/10.0);
        cairo_show_text(cr, "1");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh[2]*width/10.0, mesh[3]*height/10.0);
        cairo_show_text(cr, "2");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh[4]*width/10.0, mesh[5]*height/10.0);
        cairo_show_text(cr, "3");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh[6]*width/10.0, mesh[7]*height/10.0);
        cairo_show_text(cr, "4");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh_p[0]*width/10.0, mesh_p[1]*height/10.0);
        cairo_show_text(cr, "A");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh_p[2]*width/10.0, mesh_p[3]*height/10.0);
        cairo_show_text(cr, "B");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh_p[4]*width/10.0, mesh_p[5]*height/10.0);
        cairo_show_text(cr, "C");
        cairo_stroke(cr);
        cairo_move_to(cr, mesh_p[6]*width/10.0, mesh_p[7]*height/10.0);
        cairo_show_text(cr, "D");
        cairo_stroke(cr);
      }
  }
static void draw_t_shirt(cairo_t *cr, gdouble width, gdouble height)
  {
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //Draw the shirt.
    cairo_move_to(cr, 4.25*w1, 2.0*h1);
    cairo_curve_to(cr, 4.5*w1, 2.5*h1, 5.5*w1, 2.5*h1, 5.75*w1, 2.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.0*w1, 2.0*h1, 7.0*w1, 2.0*h1, 8.0*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.25*w1, 4.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 6.75*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 6.75*w1, 8.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 3.25*w1, 8.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 3.25*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 2.75*w1, 4.5*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 2.0*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.0*w1, 2.0*h1, 4.0*w1, 2.0*h1, 4.0*w1, 2.0*h1);
    cairo_close_path(cr);        
  }
static void draw_fish(cairo_t *cr, gdouble width, gdouble height)
  {
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //The fish.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 2);  
    cairo_move_to(cr, 8.0*w1, 5.0*h1);
    cairo_curve_to(cr, 7.5*w1, 5.65*h1, 7.5*w1, 5.65*h1, 6.5*w1, 6.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.5*w1, 6.25*h1, 6.5*w1, 6.25*h1, 6.25*w1, 6.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.25*w1, 6.25*h1, 6.25*w1, 6.25*h1, 6.1*w1, 6.1*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.0*w1, 6.25*h1, 5.0*w1, 6.25*h1, 4.0*w1, 6.1*h1);
    cairo_stroke_preserve(cr);    
    cairo_curve_to(cr, 3.65*w1, 6.75*h1, 3.65*w1, 6.75*h1, 3.25*w1, 7.0*h1);
    cairo_stroke_preserve(cr);    
    cairo_curve_to(cr, 3.5*w1, 6.65*h1, 3.5*w1, 6.65*h1, 3.6*w1, 6.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.25*w1, 5.75*h1, 3.25*w1, 5.75*h1, 2.5*w1, 5.25*h1);
    cairo_stroke_preserve(cr);    
    cairo_curve_to(cr, 2.25*w1, 6.0*h1, 2.25*w1, 6.0*h1, 2.0*w1, 6.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.2*w1, 5.0*h1, 2.2*w1, 5.0*h1, 2.0*w1, 3.5*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.25*w1, 4.0*h1, 2.25*w1, 4.0*h1, 2.5*w1, 4.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.25*w1, 4.25*h1, 3.25*w1, 4.25*h1, 3.6*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.5*w1, 3.25*h1, 3.5*w1, 3.25*h1, 3.25*w1, 3.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.65*w1, 3.25*h1, 3.65*w1, 3.25*h1, 4.0*w1, 3.9*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 4.5*w1, 3.75*h1, 4.5*w1, 3.75*h1, 5.0*w1, 3.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.25*w1, 3.5*h1, 5.75*w1, 3.5*h1, 6.0*w1, 2.75*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.25*w1, 3.0*h1, 6.25*w1, 3.0*h1, 6.5*w1, 3.65*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.5*w1, 4.0*h1, 7.5*w1, 4.0*h1, 8.0*w1, 4.7*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 7.0*w1, 4.9*h1);
    cairo_stroke_preserve(cr);
    cairo_line_to(cr, 8.0*w1, 5.0*h1);
    cairo_close_path(cr);
  }
static void draw_butterfly(cairo_t *cr, gdouble width, gdouble height)
  {
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    //The antenna.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 4);  
    cairo_move_to(cr, 5.0*w1, 3.75*h1);
    cairo_curve_to(cr, 5.5*w1, 2.5*h1, 5.5*w1, 2.5*h1, 6.0*w1, 2.5*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*w1, 3.75*h1);
    cairo_curve_to(cr, 4.5*w1, 2.5*h1, 4.5*w1, 2.5*h1, 4.0*w1, 2.5*h1);
    cairo_stroke(cr);

    //The butterfly.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 2);  
    cairo_move_to(cr, 5.25*w1, 6.0*h1);
    cairo_curve_to(cr, 6.25*w1, 6.0*h1, 6.0*w1, 7.5*h1, 6.25*w1, 7.25*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 6.25*w1, 5.5*h1, 6.25*w1, 5.5*h1, 6.75*w1, 5.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.75*w1, 4.5*h1, 7.75*w1, 4.5*h1, 8.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 7.0*w1, 3.25*h1, 6.0*w1, 3.75*h1, 5.25*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 5.2*w1, 3.25*h1, 4.8*w1, 3.25*h1, 4.75*w1, 4.0*h1);
    cairo_stroke_preserve(cr);

    cairo_curve_to(cr, 4.0*w1, 3.75*h1, 3.0*w1, 3.25*h1, 2.0*w1, 4.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 2.25*w1, 4.5*h1, 2.25*w1, 4.5*h1, 3.25*w1, 5.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 3.75*w1, 5.5*h1, 3.75*w1, 5.5*h1, 3.75*w1, 7.25*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 4.0*w1, 7.5*h1, 3.75*w1, 6.0*h1, 4.75*w1, 6.0*h1);
    cairo_stroke_preserve(cr);
    cairo_curve_to(cr, 4.8*w1, 5.5*h1, 5.2*w1, 5.5*h1, 5.25*w1, 6.0*h1);
    cairo_close_path(cr);
  }
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_unblock(widget, motion_id);

    rect[0]=event->button.x;
    rect[1]=event->button.y;

    return TRUE;
  }
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_block(widget, motion_id);
    return TRUE;
  }
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    rect[2]=event->button.x;
    rect[3]=event->button.y;
    gtk_widget_queue_draw(widget);
    return TRUE;
  }
static void combo1_changed(GtkComboBox *combo1, gpointer data)
  {
    mesh_combo=gtk_combo_box_get_active(combo1);
  }
static void combo2_changed(GtkComboBox *combo2, gpointer data)
  {
    tile_combo=gtk_combo_box_get_active(combo2);
    if(tile_combo==0) default_drawing=TRUE;
    else default_drawing=FALSE;
    grid_combo=TRUE;
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void combo3_changed(GtkComboBox *combo3, gpointer data)
  {
    drawing_combo=gtk_combo_box_get_active(combo3);
    default_drawing=FALSE;
    grid_combo=FALSE;
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void combo4_changed(GtkComboBox *combo4, gpointer data)
  {
    scale_combo=gtk_combo_box_get_active(combo4);
    if(tile_combo!=0) default_drawing=FALSE;
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void check_colors(GtkWidget *widget, GtkWidget **colors)
  {
    gint i=0;
    GdkRGBA rgba;

    for(i=0;i<5;i++)
      {
        if(gdk_rgba_parse(&rgba, gtk_entry_get_text(GTK_ENTRY(colors[i]))))
          {
            switch(i)
              {
                case 0:
                  c0[0]=rgba.red;
                  c0[1]=rgba.green;
                  c0[2]=rgba.blue;
                  c0[3]=rgba.alpha;
                  break;
                case 1:
                  c1[0]=rgba.red;
                  c1[1]=rgba.green;
                  c1[2]=rgba.blue;
                  c1[3]=rgba.alpha;
                  break;
                case 2:
                  c2[0]=rgba.red;
                  c2[1]=rgba.green;
                  c2[2]=rgba.blue;
                  c2[3]=rgba.alpha;
                  break;
                case 3:
                  c3[0]=rgba.red;
                  c3[1]=rgba.green;
                  c3[2]=rgba.blue;
                  c3[3]=rgba.alpha;
                  break;
                case 4:
                  b1[0]=rgba.red;
                  b1[1]=rgba.green;
                  b1[2]=rgba.blue;
                  b1[3]=rgba.alpha;
             }
          }
        else
          {
            g_print("Color string format error in c%i\n", i);
          } 
      }
    //Update main window.
    gtk_widget_queue_draw(colors[5]);
    //Update the drawing area.
    gtk_widget_queue_draw(colors[6]);
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
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(data));
   
    gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Mesh Maker");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "Tile a mesh and clip the pattern inside a drawing.");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2017 C. Eric Cashon");
   
    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
//The butterfly program icon.
static GdkPixbuf* draw_icon()
  {
    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface);
    
    //Paint the background.
    cairo_set_source_rgb(cr, 0.5, 0.8, 1.0);
    cairo_paint(cr);

    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 0.0);
    /*
        Scale drawing 256*1.5=384. 128 extra pixels, half on each side of drawing.
        Move the drawing back by 64 and scale.
    */
    cairo_translate(cr, -64.0, -64.0);
    cairo_scale(cr, 1.5, 1.5);
    cairo_save(cr);
    draw_butterfly(cr, 256.0, 256.0);
    cairo_restore(cr);
    cairo_clip(cr);

    draw_grid2(cr, 256.0, 256.0);

    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface, 0, 0, 256, 256);

    cairo_destroy(cr);
    cairo_surface_destroy(surface); 
    return icon;
  }










