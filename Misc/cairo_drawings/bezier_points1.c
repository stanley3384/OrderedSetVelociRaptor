
/*
    This is testing putting together drag and drop with some cairo interpolation and approximation.
The drag and drop for the list box is from mclasen and the following. 

    https://blog.gtk.org/2017/06/01/drag-and-drop-in-lists-revisited/

Drag a few points on the drawing for the interpolation. Hopefully changing the point order in the 
list box will change the draw order of the points. Still some things to figure out here. Try out
the animation rotating the drawing around the x, y, z or xyz axis. The drawing will output to svg
in the working directory and open the svg in an image widget. 

    gcc -Wall bezier_points1.c -o bezier_points1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>
#include<stdlib.h>

static void interpolation_combo(GtkComboBox *combo, gpointer data);
static void close_and_fill_check(GtkToggleButton *button, gpointer data);
static void save_top_check(GtkToggleButton *button, gpointer data);
static void rotate_combo(GtkComboBox *combo, gpointer data);
static gboolean animate_drawing(GtkWidget *drawing, GdkFrameClock *frame_clock, gpointer data);
static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_shapes(GtkWidget *widget, cairo_t *cr, GArray *array, gint shape_fill, gint shape_inter, gboolean saved);
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data);
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data);
static void check_colors(GtkWidget *widget, GtkWidget **colors);
static gboolean draw_main_window(GtkWidget *widget, cairo_t *cr, gpointer data);
//Bezier control points from coordinates.
static GArray* control_points_from_coords2(const GArray *dataPoints);
//Mid points from coordinates to draw an aproximation curve.
static GArray* mid_points_from_coords(const GArray *dataPoints);
//drag and drop.
static void drag_begin(GtkWidget *widget, GdkDragContext *context, gpointer data);
static void drag_end(GtkWidget *widget, GdkDragContext *context, gpointer data);
static void drag_data_get(GtkWidget *widget, GdkDragContext *context, GtkSelectionData *selection_data, guint info, guint time, gpointer data);
static GtkListBoxRow *get_last_row(GtkListBox *list);
static GtkListBoxRow *get_row_before(GtkListBox *list, GtkListBoxRow *row);
static GtkListBoxRow *get_row_after(GtkListBox *list, GtkListBoxRow *row);
static void drag_data_received(GtkWidget *widget, GdkDragContext *context, gint x, gint y, GtkSelectionData *selection_data, guint info, guint32 time, gpointer data);
static gboolean drag_motion(GtkWidget *widget, GdkDragContext *context, int x, int y, guint time);
static void drag_leave(GtkWidget *widget, GdkDragContext *context, guint time);
static GtkWidget *create_row(const gchar *text, GtkWidget *da);
static void on_row_activated(GtkListBox *self, GtkListBoxRow *row, gpointer data);
static void on_selected_children_changed(GtkListBox *self);
static void a11y_selection_changed (AtkObject *obj);
//Delete list box row with Del key.
static gboolean delete_row(GtkWidget *row, GdkEventKey *event, gpointer data);
//Add a new point and row to the list box.
static void add_point(GtkWidget *widget, GtkWidget **list_da);
//Save to svg file.
static void save_svg(GtkWidget *widget, gpointer data);
static void build_drawing_svg(FILE *f, GArray *array, gint width, gint height, gint shape_fill, gint shape_inter, gint path_id);
//Dialog for showing svg.
static void svg_dialog(gint width, gint height);
//Save current drawing cordinates.
static void add_points(GtkWidget *widget, GtkWidget **widgets);
static void delete_shape(GtkWidget *widget, GtkWidget **widgets);
static void clear_shapes(GtkWidget *widget, GtkWidget **widgets);
static GtkTreeStore* get_tree_store();
static void cleanup(GtkWidget *widget, gpointer data);

static GtkTargetEntry entries[] = {
  { "GTK_LIST_BOX_ROW", GTK_TARGET_SAME_APP, 0 }
};

static const char *css =
  ".row:not(:first-child) { "
  "  border-top: 1px solid alpha(gray,0.5); "
  "  border-bottom: 1px solid transparent; "
  "}"
  ".row:first-child { "
  "  border-top: 1px solid transparent; "
  "  border-bottom: 1px solid transparent; "
  "}"
  ".row:last-child { "
  "  border-top: 1px solid alpha(gray,0.5); "
  "  border-bottom: 1px solid alpha(gray,0.5); "
  "}"
  ".row.drag-icon { "
  "  background: white; "
  "  border: 1px solid black; "
  "}"
  ".row.drag-row { "
  "  color: gray; "
  "  background: alpha(gray,0.2); "
  "}"
  ".row.drag-row.drag-hover { "
  "  border-top: 1px solid #4e9a06; "
  "  border-bottom: 1px solid #4e9a06; "
  "}"
  ".row.drag-hover image, "
  ".row.drag-hover label { "
  "  color: #4e9a06; "
  "}"
  ".row.drag-hover-top {"
  "  border-top: 1px solid #4e9a06; "
  "}"
  ".row.drag-hover-bottom {"
  "  border-bottom: 1px solid #4e9a06; "
  "}"
;

//Points and controls for drawing with.
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

//Coordinate points for drawing.
static GArray *coords1=NULL;
//A pointer array to save garrays of points.
static GPtrArray *paths=NULL;
//Saved path draw info. Interpolate and Fill.
static GArray *path_info=NULL;
//List box order array.
static GArray *array_id=NULL;
static gint begin_id=0;
//For blocking motion signal. Block when not drawing top rectangle
static gint motion_id=0;
//Drawing background color.
static gdouble b1[]={1.0, 1.0, 1.0, 1.0};
//Current selected row in list.
static gint row_id=0;
//If the curve should be drawn linear, smooth or approximate. Start with smooth.
static gint interpolation=1;
//If the end of the curve connects to the start. If true fill the closed curve and draw a gradient.
static gint fill=0;
//Rotate and animate the drawing.
static gint rotate=0;
//Tick id for animation frame clock.
static guint tick_id=0;
//GTK window for dialogs.
static GtkWidget *window;
//It the top drawing is included in the svg.
gboolean save_top=TRUE;
//Save initial drawing area dimensions.
static gdouble start_width=0;
static gdouble start_height=0;

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Coordinates with Bezier Points");
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 500);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    g_signal_connect(window, "destroy", G_CALLBACK(cleanup), NULL);
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
    //Add some extra events to the top drawing area.
    gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK);
    g_signal_connect(da, "draw", G_CALLBACK(start_drawing), NULL);
    g_signal_connect(da, "button-press-event", G_CALLBACK(start_press), NULL);
    g_signal_connect(da, "button-release-event", G_CALLBACK(stop_press), NULL);
    motion_id=g_signal_connect(da, "motion-notify-event", G_CALLBACK(cursor_motion), NULL);
    g_signal_handler_block(da, motion_id);

    GtkWidget *label1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label1), "<span font_weight='heavy'>Point Draw Order</span>");

    gint i=0;
    GtkWidget *list = gtk_list_box_new ();
    gtk_widget_set_hexpand(list, TRUE);
    gtk_widget_set_vexpand(list, TRUE);
    gtk_list_box_set_selection_mode (GTK_LIST_BOX (list), GTK_SELECTION_SINGLE);
    GtkWidget *row;
    gchar *text=NULL;
    for (i = 0; i < 12; i++)
    {
      text = g_strdup_printf ("Point %d", i+1);
      row = create_row (text, da);
      gtk_list_box_insert (GTK_LIST_BOX (list), row, -1);
      g_free(text);
    }

    gtk_drag_dest_set (list, GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_DROP, entries, 1, GDK_ACTION_MOVE);
    g_signal_connect (list, "drag-data-received", G_CALLBACK (drag_data_received), NULL);
    g_signal_connect (list, "drag-motion", G_CALLBACK (drag_motion), NULL);
    g_signal_connect (list, "drag-leave", G_CALLBACK (drag_leave), NULL);

    g_signal_connect (list, "row-activated", G_CALLBACK (on_row_activated), da);
    g_signal_connect (list, "selected-rows-changed", G_CALLBACK (on_selected_children_changed), NULL);
    g_signal_connect (gtk_widget_get_accessible (list), "selection-changed", G_CALLBACK (a11y_selection_changed), NULL);

    GtkWidget *sw = gtk_scrolled_window_new (NULL, NULL);
    gtk_widget_set_hexpand (sw, TRUE);
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (sw), GTK_POLICY_NEVER, GTK_POLICY_ALWAYS);
    gtk_container_add (GTK_CONTAINER (sw), list);

    GtkWidget *button1=gtk_button_new_with_label("Add Point");
    gtk_widget_set_hexpand(button1, FALSE);
    GtkWidget *list_da[]={list, da};
    g_signal_connect(button1, "clicked", G_CALLBACK(add_point), list_da);

    GtkWidget *combo1=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo1, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 0, "1", "Linear");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 1, "2", "Smooth");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo1), 2, "3", "Approximate");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo1), 1);
    g_signal_connect(combo1, "changed", G_CALLBACK(interpolation_combo), da);
 
    GtkWidget *check1=gtk_check_button_new_with_label("Close and Fill");
    gtk_widget_set_halign(check1, GTK_ALIGN_CENTER);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check1), FALSE);
    g_signal_connect(check1, "toggled", G_CALLBACK(close_and_fill_check), da); 

    GtkWidget *combo2=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo2, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 0, "1", "No Rotate");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 1, "2", "Rotate X");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 2, "3", "Rotate Y");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 3, "4", "Rotate Z");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo2), 4, "5", "Rotate XYZ");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo2), 0);
    g_signal_connect(combo2, "changed", G_CALLBACK(rotate_combo), da);

    GtkWidget *label2=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label2), "<span font_weight='heavy'>Background </span>");

    GtkWidget *entry1=gtk_entry_new();
    gtk_widget_set_hexpand(entry1, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry1), "rgba(255, 255, 255, 1.0)");

    GtkWidget *button2=gtk_button_new_with_label("Update Background");
    gtk_widget_set_hexpand(button2, FALSE);
    GtkWidget *colors[]={entry1, window, da};
    g_signal_connect(button2, "clicked", G_CALLBACK(check_colors), colors);
    
    GtkWidget *grid1=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid1), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid1), 8);
    gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 2, 1);    
    gtk_grid_attach(GTK_GRID(grid1), sw, 0, 1, 2, 1); 
    gtk_grid_attach(GTK_GRID(grid1), button1, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid1), combo1, 0, 3, 1, 1); 
    gtk_grid_attach(GTK_GRID(grid1), combo2, 1, 3, 1, 1);  
    gtk_grid_attach(GTK_GRID(grid1), check1, 0, 4, 2, 1);     
    gtk_grid_attach(GTK_GRID(grid1), label2, 0, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), entry1, 1, 5, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), button2, 0, 6, 2, 1);

    //Tab 2 widgets.
    GtkWidget *b1=gtk_button_new_with_label("Add Shape");
    gtk_widget_set_hexpand(b1, TRUE);

    GtkWidget *b2=gtk_button_new_with_label("Delete Shape");
    gtk_widget_set_hexpand(b2, TRUE);

    GtkWidget *b3=gtk_button_new_with_label("Clear Shapes");
    gtk_widget_set_hexpand(b3, TRUE);

    GtkWidget *b4=gtk_button_new_with_label("Save Show SVG");
    gtk_widget_set_hexpand(b4, TRUE);
    g_signal_connect(b4, "clicked", G_CALLBACK(save_svg), da);

    GtkWidget *ch1=gtk_check_button_new_with_label("Save Top Drawing to SVG");
    gtk_widget_set_halign(ch1, GTK_ALIGN_CENTER);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ch1), TRUE);
    g_signal_connect(ch1, "toggled", G_CALLBACK(save_top_check), da); 

    paths=g_ptr_array_new();
    path_info=g_array_new(FALSE, FALSE, sizeof(gint));

    GtkTreeStore *store=get_tree_store();
    GtkWidget *tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(G_OBJECT(store));
    GtkCellRenderer *renderer1=gtk_cell_renderer_text_new();
    g_object_set(renderer1, "editable", FALSE, NULL);   
    GtkTreeViewColumn *column1=gtk_tree_view_column_new_with_attributes("Shape Coordinates", renderer1, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column1); 

    GtkWidget *widgets[]={da, check1, tree};
    g_signal_connect(b1, "clicked", G_CALLBACK(add_points), widgets); 
    g_signal_connect(b2, "clicked", G_CALLBACK(delete_shape), widgets); 
    g_signal_connect(b3, "clicked", G_CALLBACK(clear_shapes), widgets);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), tree); 

    GtkWidget *grid2=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid2), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid2), 8);
    gtk_grid_attach(GTK_GRID(grid2), b1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid2), scroll, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid2), b2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), b3, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), ch1, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid2), b4, 0, 4, 2, 1);

    GtkWidget *nb_label1=gtk_label_new("Draw Path");
    GtkWidget *nb_label2=gtk_label_new("Add Shape");
    GtkWidget *notebook=gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(notebook), 15);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid1, nb_label1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid2, nb_label2);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), notebook, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 350);

    //Draw background window based on the paned window splitter.
    g_signal_connect(window, "draw", G_CALLBACK(draw_main_window), paned1);

    gtk_container_add(GTK_CONTAINER(window), paned1);

    GtkCssProvider *provider=gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), 800);

    gtk_widget_show_all(window);

    //Set some initial values for the drawing area.
    start_width=(gdouble)gtk_widget_get_allocated_width(da);
    start_height=(gdouble)gtk_widget_get_allocated_height(da); 
    gdouble w1=start_width*0.4;
    if(start_width>start_height) w1=start_height*0.4;
    struct point p1;
    coords1=g_array_sized_new(FALSE, FALSE, sizeof(struct point), 12);
    for(i=0;i<12;i++)
      {
        p1.x=w1*cos((gdouble)i*G_PI/6.0);
        p1.y=w1*sin((gdouble)i*G_PI/6.0);
        //g_print("x %f y %f\n", p1.x, p1.y);
        g_array_append_val(coords1, p1);
      }
    //Array to track the list.
    array_id=g_array_new (FALSE, FALSE, sizeof(gint));
    for(i=0;i<12;i++) g_array_append_val(array_id, i);
    gtk_widget_queue_draw(da);

    gtk_main();

    return 0;
  }
static void interpolation_combo(GtkComboBox *combo, gpointer data)
  {
    interpolation=gtk_combo_box_get_active(combo);
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void close_and_fill_check(GtkToggleButton *button, gpointer data)
  {
    if(gtk_toggle_button_get_active(button))
      {
        fill=1;
        g_array_append_val(coords1, g_array_index(coords1, struct point, 0));
      }
    else
      {
        g_array_remove_index(coords1, coords1->len-1);
        fill=0;
      }
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void save_top_check(GtkToggleButton *button, gpointer data)
  {
    if(gtk_toggle_button_get_active(button)) save_top=TRUE;
    else save_top=FALSE;
  }
static void rotate_combo(GtkComboBox *combo, gpointer data)
  {
    rotate=gtk_combo_box_get_active(combo);

    if(rotate==0)
      {
        if(tick_id!=0) gtk_widget_remove_tick_callback(GTK_WIDGET(data), tick_id);
        tick_id=0;
        gtk_widget_queue_draw(GTK_WIDGET(data));
      }
    else
      {
        if(tick_id==0)
          {
            tick_id=gtk_widget_add_tick_callback(GTK_WIDGET(data), (GtkTickCallback)animate_drawing, NULL, NULL);
          }
      }
    
  }
static gboolean animate_drawing(GtkWidget *drawing, GdkFrameClock *frame_clock, gpointer data)
  {
    gtk_widget_queue_draw(drawing);
    return G_SOURCE_CONTINUE;
  }
static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    gint i=0;
    static gint j=1;
    gint len=paths->len;
    GArray *array=NULL;
    gint shape_fill=0;
    gint shape_inter=0;
    gboolean saved=TRUE;

    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);

    //Layout axis for drawing.
    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, w1, h1, 8.0*w1, 8.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, w1, 5.0*h1);
    cairo_line_to(cr, 9.0*w1, 5.0*h1);
    cairo_stroke(cr);
    cairo_move_to(cr, 5.0*w1, 1.0*h1);
    cairo_line_to(cr, 5.0*w1, 9.0*h1);
    cairo_stroke(cr);

    //Rotations for animation.
    gdouble angle=0;
    gdouble scale=0;
    gdouble scale_inv=0;
    if(rotate>0)
      {
        if(rotate==1)
          {
            angle=-j*G_PI/256.0-3.0*G_PI/2.0;
            scale=sin(angle);
            scale_inv=1.0/scale;
            cairo_scale(cr, 1.0, scale);
            cairo_translate(cr, width/2.0, scale_inv*height/2.0);
          }
        else if(rotate==2)
          {
            angle=-j*G_PI/256.0;
            scale=cos(angle);
            scale_inv=1.0/scale;
            cairo_scale(cr, scale, 1.0);
            cairo_translate(cr, scale_inv*width/2.0, height/2.0);
          }
        else if(rotate==3)
          {
            angle=j*G_PI/256.0;
            cairo_translate(cr, width/2.0, height/2.0);
            cairo_rotate(cr, angle);
          }
        else
          {
            angle=j*G_PI/256.0;
            scale=cos(angle);
            scale_inv=1.0/scale;
            gdouble scale2=sin(angle);
            gdouble scale_inv2=1.0/scale2;
            cairo_scale(cr, scale, scale2);
            cairo_translate(cr, scale_inv*width/2.0, scale_inv2*height/2.0);
            cairo_rotate(cr, angle);
          }
        j++;
      }
    else
      {
        cairo_translate(cr, width/2.0, height/2.0);
        j=1;
      }

    cairo_scale(cr, width/start_width, height/start_height);

    //Draw saved drawings.
    for(i=0;i<len;i++)
      {
        array=(GArray*)(g_ptr_array_index(paths, i));
        shape_inter=g_array_index(path_info, gint, 2*i);
        shape_fill=g_array_index(path_info, gint, 2*i+1); 
        draw_shapes(widget, cr, array, shape_fill, shape_inter, saved);
        cairo_new_path(cr);
      }

    //Draw top or current drawing.
    array=coords1;
    shape_fill=fill;
    shape_inter=interpolation;
    saved=FALSE;
    draw_shapes(widget, cr, array, shape_fill, shape_inter, saved);
    return FALSE;
  }
static void draw_shapes(GtkWidget *widget, cairo_t *cr, GArray *array, gint shape_fill, gint shape_inter, gboolean saved)
  {
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    GArray *control1=NULL;
    GArray *mid_points=NULL;
    gint i=0;
    gint id=0; 
    gint len=0;
    struct point p1;
    struct point p2;
    struct controls c1;

    if(shape_inter==0)
      {
        //Just draw straight lines. Don't need control points.
      }
    else if(shape_inter==1)
      { 
        control1=control_points_from_coords2(array);
      }
    else 
      {
        mid_points=mid_points_from_coords(array);
        control1=control_points_from_coords2(mid_points);
      }    

    if(shape_inter==0||shape_inter==1)
      {
        p1=g_array_index(array, struct point, 0);
        len=array->len;
      }
    else
      {
        p1=g_array_index(mid_points, struct point, 0);
        len=mid_points->len;
      }

    cairo_set_source_rgba(cr, 0.0, 0.0, 1.0, 1.0);
    cairo_set_line_width(cr, 3.0);
    cairo_move_to(cr, p1.x, p1.y);
    
    if(shape_inter==0)
      {
        for(i=1;i<len;i++)
          {
            p2=g_array_index(array, struct point, i);
            cairo_line_to(cr, p2.x, p2.y);
            cairo_stroke_preserve(cr); 
          }
      }
    else if(shape_inter==1)
      {
        for(i=1;i<len;i++)
          {
            p2=g_array_index(array, struct point, i);
            c1=g_array_index(control1, struct controls, i-1);
            cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, p2.x, p2.y);
            cairo_stroke_preserve(cr); 
          }
      }
    else
      {
        for(i=1;i<len;i++)
          {
            p2=g_array_index(mid_points, struct point, i);
            c1=g_array_index(control1, struct controls, i-1);
            cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, p2.x, p2.y);
            cairo_stroke_preserve(cr); 
          }
      }

    //Fill the pattern.
    if(shape_fill==1) 
      {
        cairo_close_path(cr);
        gdouble x1=0;
        gdouble y1=0;
        cairo_user_to_device(cr, &x1, &y1);
        cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, -height/2.0, 0.0, y1);
        cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 1.0, 0.0, 1.0, 0.8); 
        cairo_pattern_add_color_stop_rgba(pattern1, 0.5, 1.0, 1.0, 0.0, 0.8);
        cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 1.0, 1.0, 0.8);  
        cairo_set_source(cr, pattern1);  
        cairo_fill(cr);
        cairo_pattern_destroy(pattern1);
      }

    if(!saved)
      {
        cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);
        len=array_id->len;
        for(i=0;i<len;i++)
          {
            id=g_array_index(array_id, gint, i);
            p2=g_array_index(array, struct point, i);
            gchar *string=g_strdup_printf("%i", id+1);
            cairo_move_to(cr, p2.x, p2.y);
            if(i==row_id) cairo_set_source_rgba(cr, 1.0, 0.0, 0.0, 1.0);
            else cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
            cairo_show_text(cr, string); 
            g_free(string); 
          }
      }

    if(control1!=NULL) g_array_free(control1, TRUE);
    if(mid_points!=NULL) g_array_free(mid_points, TRUE);

  }
static gboolean start_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_unblock(widget, motion_id);

    return TRUE;
  }
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    g_signal_handler_block(widget, motion_id);
    return TRUE;
  }
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    gdouble width=(gdouble)gtk_widget_get_allocated_width(widget);
    gdouble height=(gdouble)gtk_widget_get_allocated_height(widget);
    struct point p1;

    gdouble w1=start_width*0.5;
    if(start_width>start_height) w1=start_height*0.5;

    p1.x=event->button.x*start_width/width-w1;
    p1.y=event->button.y*start_height/height-w1;
    //g_print("x %f, y %f\n", p1.x, p1.y);

    struct point *p;
    p=&g_array_index(coords1, struct point, row_id);
    *p=p1;

    gtk_widget_queue_draw(widget);
    return TRUE;
  }
static void check_colors(GtkWidget *widget, GtkWidget **colors)
  {
    GdkRGBA rgba;
    
    if(gdk_rgba_parse(&rgba, gtk_entry_get_text(GTK_ENTRY(colors[0]))))
      {
        b1[0]=rgba.red;
        b1[1]=rgba.green;
        b1[2]=rgba.blue;
        b1[3]=rgba.alpha;
      }
    else
      {
        g_print("Color string format error in background\n");
      } 
   
    //Update main window.
    gtk_widget_queue_draw(colors[1]);
    //Update the drawing area.
    gtk_widget_queue_draw(colors[2]);
  }
static gboolean draw_main_window(GtkWidget *widget, cairo_t *cr, gpointer data)
  {
    //Paint background of drawing area.
    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);
    //Paint the background under the grid.
    cairo_set_source_rgba(cr, 0.0, 1.0, 1.0, 0.8);
    gint width=gtk_paned_get_position(GTK_PANED(data));
    gint height=gtk_widget_get_allocated_height(widget);

    cairo_rectangle(cr, 0.0, 0.0, width, height);
    cairo_fill(cr);
    cairo_set_source_rgba(cr, 1.0, 1.0, 0.0, 1.0);
    cairo_rectangle(cr, width, 0.0, 10, height);
    cairo_fill(cr);
    return FALSE;
  }
/*
    This is some exellent work done by Ramsundar Shandilya. Note the following for the original work
    and the rational behind it.
    
    https://medium.com/@ramshandilya/draw-smooth-curves-through-a-set-of-points-in-ios-34f6d73c8f9

    https://github.com/Ramshandilya/Bezier

    The MIT License (MIT)

    Copyright (c) 2015 Ramsundar Shandilya

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in all
    copies or substantial portions of the Software.
  
    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
    SOFTWARE.

    This is a translation of the original Swift code to C. It makes the function easy to use with GTK+,
    cairo and glib.
*/
static GArray* control_points_from_coords2(const GArray *dataPoints)
  {  
    gint i=0;
    GArray *controlPoints=NULL;      
    //Number of Segments
    gint count=0;
    if(dataPoints!=NULL) count=dataPoints->len-1;
    gdouble *fCP=NULL;
    gdouble *sCP=NULL;

    if(count>0)
      {
        fCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
        sCP=(gdouble*)g_malloc(2*count*sizeof(gdouble));
      }
        
    //P0, P1, P2, P3 are the points for each segment, where P0 & P3 are the knots and P1, P2 are the control points.
    if(count<1||dataPoints==NULL)
      {
        //Return NULL.
        controlPoints=NULL;
        g_print("Can't get control points from coordinates. NULL returned.\n");
      }
    else if(count==1)
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

    if(fCP!=NULL) g_free(fCP);
    if(sCP!=NULL) g_free(sCP);

    return controlPoints;
  }
/*
    Approximate by just getting the mid point between two coordinate points. Use the first
and last points in the coordinate set for the start and finish.
*/
static GArray* mid_points_from_coords(const GArray *dataPoints)
  {
    gint i=0;
    GArray *mid_points=NULL;      
    //Number of Segments
    gint count=0;
    if(dataPoints!=NULL) count=dataPoints->len-1;
        
    if(count<1||dataPoints==NULL)
      {
        //Return NULL.
        mid_points=NULL;
        g_print("Can't get mid points from coordinates. NULL returned.\n");
      }
    else 
      {
        mid_points=g_array_new(FALSE, FALSE, sizeof(struct point));
        struct point p1;
        struct point p2;
        struct point p3;
        //Get first coordinate point to start with.
        p1=g_array_index(dataPoints, struct point, 0);
        g_array_append_val(mid_points, p1);
        for(i=0;i<count;i++)
          {
            p1=g_array_index(dataPoints, struct point, i);
            p2=g_array_index(dataPoints, struct point, i+1);
            p3.x=(p1.x+p2.x)/2.0;
            p3.y=(p1.y+p2.y)/2.0;
            g_array_append_val(mid_points, p3);            
          }
        //Add the last point from the coordinates array.
        p1=g_array_index(dataPoints, struct point, count);
        g_array_append_val(mid_points, p1);
      }

    return mid_points;
  }

static void
drag_begin (GtkWidget      *widget,
            GdkDragContext *context,
            gpointer        data)
{
  GtkWidget *row;
  GtkAllocation alloc;
  cairo_surface_t *surface;
  cairo_t *cr;
  int x, y;

  row = gtk_widget_get_ancestor (widget, GTK_TYPE_LIST_BOX_ROW);
  gtk_widget_get_allocation (row, &alloc);
  surface = cairo_image_surface_create (CAIRO_FORMAT_ARGB32, alloc.width, alloc.height);
  cr = cairo_create (surface);

  gtk_style_context_add_class (gtk_widget_get_style_context (row), "drag-icon");
  gtk_widget_draw (row, cr);
  gtk_style_context_remove_class (gtk_widget_get_style_context (row), "drag-icon");

  gtk_widget_translate_coordinates (widget, row, 0, 0, &x, &y);
  cairo_surface_set_device_offset (surface, -x, -y);
  gtk_drag_set_icon_surface (context, surface);

  cairo_destroy (cr);
  cairo_surface_destroy (surface);

  g_object_set_data (G_OBJECT (gtk_widget_get_parent (row)), "drag-row", row);
  gtk_style_context_add_class (gtk_widget_get_style_context (row), "drag-row");
}

static void
drag_end (GtkWidget      *widget,
          GdkDragContext *context,
          gpointer        data)
{
  GtkWidget *row;

  row = gtk_widget_get_ancestor (widget, GTK_TYPE_LIST_BOX_ROW);
  g_object_set_data (G_OBJECT (gtk_widget_get_parent (row)), "drag-row", NULL);
  gtk_style_context_remove_class (gtk_widget_get_style_context (row), "drag-row");
  gtk_style_context_remove_class (gtk_widget_get_style_context (row), "drag-hover");

  g_print("Drag End\n");
  row_id=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
  gint i=0;
  gint value_id=0;
  gint len=0;
  struct point p1;
  g_print("begin %i row %i\n", begin_id, row_id);

  len=coords1->len;
  for(i=0;i<len;i++)
    {
      p1=g_array_index(coords1, struct point, i);
      g_print(" [%i %i %i]", i, (gint)p1.x, (gint)p1.y);
    }
  g_print("\n");

  if(begin_id!=row_id)
    {
      value_id=g_array_index(array_id, gint, begin_id);
      p1=g_array_index(coords1, struct point, begin_id);
      if(begin_id>row_id)
        {
          g_array_insert_val(array_id, row_id, value_id);
          g_array_insert_val(coords1, row_id, p1);
          len=array_id->len-1;
          for(i=len;i>-1;i--)
            {
              if(g_array_index(array_id, gint, i)==value_id)
                {
                  g_array_remove_index(array_id, i);
                  g_array_remove_index(coords1, i);
                  break;
                }
            }
        }
      else
        {
          g_array_insert_val(array_id, row_id+1, value_id);
          g_array_insert_val(coords1, row_id+1, p1);
          len=array_id->len-1;
          for(i=0;i<len;i++)
            {
              if(g_array_index(array_id, gint, i)==value_id)
                {
                  g_array_remove_index(array_id, i);
                  g_array_remove_index(coords1, i);
                  break;
                }
            }
        }
    }

  len=array_id->len;
  for(i=0;i<len;i++) g_print(" %i", g_array_index(array_id, gint, i));
  g_print("\n");

  len=coords1->len;
  for(i=0;i<len;i++)
    {
      p1=g_array_index(coords1, struct point, i);
      g_print(" [%i %i %i]", i, (gint)p1.x, (gint)p1.y);
    }
  g_print("\n");
  
  begin_id=row_id;

  gtk_widget_queue_draw(GTK_WIDGET(data));
}

static void
drag_data_get (GtkWidget        *widget,
               GdkDragContext   *context,
               GtkSelectionData *selection_data,
               guint             info,
               guint             time,
               gpointer          data)
{
  gtk_selection_data_set (selection_data,
                          gdk_atom_intern_static_string ("GTK_LIST_BOX_ROW"),
                          32,
                          (const guchar *)&widget,
                          sizeof (gpointer));
}

static GtkListBoxRow *
get_last_row (GtkListBox *list)
{
  int i;
  GtkListBoxRow *row;

  row = NULL;
  for (i = 0; ; i++)
    {
      GtkListBoxRow *tmp;
      tmp = gtk_list_box_get_row_at_index (list, i);
      if (tmp == NULL)
        return row;
      row = tmp;
    }
  return row;
}

static GtkListBoxRow *
get_row_before (GtkListBox    *list,
                GtkListBoxRow *row)
{
  int pos = gtk_list_box_row_get_index (row);
  return gtk_list_box_get_row_at_index (list, pos - 1);
}

static GtkListBoxRow *
get_row_after (GtkListBox    *list,
               GtkListBoxRow *row)
{
  int pos = gtk_list_box_row_get_index (row);
  return gtk_list_box_get_row_at_index (list, pos + 1);
}

static void
drag_data_received (GtkWidget        *widget,
                    GdkDragContext   *context,
                    gint              x,
                    gint              y,
                    GtkSelectionData *selection_data,
                    guint             info,
                    guint32           time,
                    gpointer          data)
{
  GtkWidget *row_before;
  GtkWidget *row_after;
  GtkWidget *row;
  GtkWidget *source;
  int pos;

  row_before = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "row-before"));
  row_after = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "row-after"));

  g_object_set_data (G_OBJECT (widget), "row-before", NULL);
  g_object_set_data (G_OBJECT (widget), "row-after", NULL);

  if (row_before)
    gtk_style_context_remove_class (gtk_widget_get_style_context (row_before), "drag-hover-bottom");
  if (row_after)
    gtk_style_context_remove_class (gtk_widget_get_style_context (row_after), "drag-hover-top");

  row = (gpointer)* (gpointer*)gtk_selection_data_get_data (selection_data);
  source = gtk_widget_get_ancestor (row, GTK_TYPE_LIST_BOX_ROW);

  if (source == row_after)
    return;

  g_object_ref (source);
  gtk_container_remove (GTK_CONTAINER (gtk_widget_get_parent (source)), source);

  if (row_after)
    pos = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW (row_after));
  else
    pos = gtk_list_box_row_get_index (GTK_LIST_BOX_ROW (row_before)) + 1;

  gtk_list_box_insert (GTK_LIST_BOX (widget), source, pos);
  g_object_unref (source);
}

static gboolean
drag_motion (GtkWidget      *widget,
             GdkDragContext *context,
             int             x,
             int             y,
             guint           time)
{
  GtkAllocation alloc;
  GtkWidget *row;
  int hover_row_y;
  int hover_row_height;
  GtkWidget *drag_row;
  GtkWidget *row_before;
  GtkWidget *row_after;

  row = GTK_WIDGET (gtk_list_box_get_row_at_y (GTK_LIST_BOX (widget), y));

  drag_row = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "drag-row"));
  row_before = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "row-before"));
  row_after = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "row-after"));

  gtk_style_context_remove_class (gtk_widget_get_style_context (drag_row), "drag-hover");
  if (row_before)
    gtk_style_context_remove_class (gtk_widget_get_style_context (row_before), "drag-hover-bottom");
  if (row_after)
    gtk_style_context_remove_class (gtk_widget_get_style_context (row_after), "drag-hover-top");

  if (row)
    {
      gtk_widget_get_allocation (row, &alloc);
      hover_row_y = alloc.y;
      hover_row_height = alloc.height;

      if (y < hover_row_y + hover_row_height/2)
        {
          row_after = row;
          row_before = GTK_WIDGET (get_row_before (GTK_LIST_BOX (widget), GTK_LIST_BOX_ROW (row)));
        }
      else
        {
          row_before = row;
          row_after = GTK_WIDGET (get_row_after (GTK_LIST_BOX (widget), GTK_LIST_BOX_ROW (row)));
        }
    }
  else
    {
      row_before = GTK_WIDGET (get_last_row (GTK_LIST_BOX (widget)));
      row_after = NULL;
    }

  g_object_set_data (G_OBJECT (widget), "row-before", row_before);
  g_object_set_data (G_OBJECT (widget), "row-after", row_after);

  if (drag_row == row_before || drag_row == row_after)
    {
      gtk_style_context_add_class (gtk_widget_get_style_context (drag_row), "drag-hover");
      return FALSE;
    }

  if (row_before)
    gtk_style_context_add_class (gtk_widget_get_style_context (row_before), "drag-hover-bottom");
  if (row_after)
    gtk_style_context_add_class (gtk_widget_get_style_context (row_after), "drag-hover-top");

  return TRUE;
}

static void
drag_leave (GtkWidget      *widget,
            GdkDragContext *context,
            guint           time)
{
  GtkWidget *drag_row;
  GtkWidget *row_before;
  GtkWidget *row_after;

  drag_row = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "drag-row"));
  row_before = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "row-before"));
  row_after = GTK_WIDGET (g_object_get_data (G_OBJECT (widget), "row-after"));

  gtk_style_context_remove_class (gtk_widget_get_style_context (drag_row), "drag-hover");
  if (row_before)
    gtk_style_context_remove_class (gtk_widget_get_style_context (row_before), "drag-hover-bottom");
  if (row_after)
    gtk_style_context_remove_class (gtk_widget_get_style_context (row_after), "drag-hover-top");
}

static GtkWidget *
create_row (const gchar *text, GtkWidget *da)
{
  GtkWidget *row, *ebox, *box, *label, *image;

  row = gtk_list_box_row_new ();
  ebox = gtk_event_box_new ();
  image = gtk_image_new_from_icon_name ("open-menu-symbolic", 1);
  gtk_container_add (GTK_CONTAINER (ebox), image);
  box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 10);
  g_object_set (box, "margin-start", 10, "margin-end", 10, NULL);
  label = gtk_label_new (text);
  gtk_container_add (GTK_CONTAINER (row), box);
  gtk_box_pack_start (GTK_BOX (box), label, TRUE, FALSE, 0);
  gtk_container_add (GTK_CONTAINER (box), ebox);

  gtk_style_context_add_class (gtk_widget_get_style_context (row), "row");
  gtk_drag_source_set (ebox, GDK_BUTTON1_MASK, entries, 1, GDK_ACTION_MOVE);
  g_signal_connect (ebox, "drag-begin", G_CALLBACK (drag_begin), NULL);
  g_signal_connect (ebox, "drag-end", G_CALLBACK (drag_end), da);
  g_signal_connect (ebox, "drag-data-get", G_CALLBACK (drag_data_get), NULL);

  g_signal_connect (row, "key-press-event", G_CALLBACK (delete_row), da);

  return row;
}

static void
on_row_activated (GtkListBox *self, GtkListBoxRow *row, gpointer data)
{
  row_id=gtk_list_box_row_get_index(row);
  begin_id=row_id;
  gtk_widget_queue_draw(GTK_WIDGET(data));
}

static void
on_selected_children_changed (GtkListBox *self)
{
  //g_message ("Selection changed");
}

static void
a11y_selection_changed (AtkObject *obj)
{
  //g_message ("Accessible selection changed");
}

static gboolean delete_row(GtkWidget *row, GdkEventKey *event, gpointer data)
{
  if(event->keyval==GDK_KEY_Delete)
    {
      g_print("Delete Row\n");
      gint i=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
      g_array_remove_index(array_id, i);
      g_array_remove_index(coords1, i);
      gtk_widget_destroy(row);
      gtk_widget_queue_draw(GTK_WIDGET(data));
    }
  return TRUE;
}
static void add_point(GtkWidget *widget, GtkWidget **list_da)
{
  g_print("Add Point\n");
  //Keep track of added list box rows. Start program with 12 rows in the listbox.
  static gint add_row=12;
  gint len=0;
  gint i=add_row++;
  struct point p1;

  g_array_append_val(array_id, i);
  len=coords1->len-1;
  p1=g_array_index(coords1, struct point, len);
  p1.x=p1.x-40.0;
  g_array_append_val(coords1, p1);

  gchar *point=g_strdup_printf("Point %i", i+1);
  GtkWidget *row=create_row(point, list_da[1]);
  gtk_list_box_insert(GTK_LIST_BOX(list_da[0]), row, -1);
  gtk_widget_show_all(row);
  gtk_widget_queue_draw(GTK_WIDGET(list_da[1]));
  g_free(point);
}
static void save_svg(GtkWidget *widget, gpointer data)
{
  gint i=0;
  gint path_id=1;
  gint width=gtk_widget_get_allocated_width(GTK_WIDGET(data));
  gint height=gtk_widget_get_allocated_height(GTK_WIDGET(data));
  gint len=0;
  GArray *array=NULL;
  gint shape_fill=0;
  gint shape_inter=0;
  gboolean file_error=FALSE;

  FILE *f=fopen("bezier_drawing1.svg", "w");
  if(f!=NULL)
    {
      fprintf(f, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
      fprintf(f, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"); 
      fprintf(f, "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
      fprintf(f, "<svg width=\"%i\" height=\"%i\" viewBox=\"0 0 %i %i\"\n", width, height, width, height);
      fprintf(f, "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n");
      fprintf(f, "<rect x=\"0\" y=\"0\" width=\"%i\" height=\"%i\" fill=\"rgb(%i,%i,%i)\" fill-opacity=\"%f\" />\n", width, height, (gint)(b1[0]*255.0), (gint)(b1[1]*255.0), (gint)(b1[2]*255.0), b1[3]);

      //Define a fill to be used.
      fprintf(f, "<defs>\n");
      fprintf(f, "<linearGradient id=\"grad2\" x1=\"0%%\" y1=\"0%%\" x2=\"0%%\" y2=\"100%%\">\n");
      fprintf(f, "<stop offset=\"0%%\" style=\"stop-color:rgb(255,0,255);stop-opacity:0.8\" />\n");
      fprintf(f, "<stop offset=\"50%%\" style=\"stop-color:rgb(255,255,0);stop-opacity:0.8\" />\n");
      fprintf(f, "<stop offset=\"100%%\" style=\"stop-color:rgb(0,255,255);stop-opacity:0.8\" />\n");
      fprintf(f, "</linearGradient>\n");
      fprintf(f, "</defs>\n");       

      //Build the svg from the stored arrays.
      len=paths->len;
      for(i=0;i<len;i++)
        {
          array=(GArray*)(g_ptr_array_index(paths, i));
          shape_inter=g_array_index(path_info, gint, 2*i);
          shape_fill=g_array_index(path_info, gint, 2*i+1);
          build_drawing_svg(f, array, width, height, shape_fill, shape_inter, path_id);
          path_id++;
        }

      //Draw top or current drawing from the coords array.
      if(save_top)
        {
          array=coords1;
          shape_fill=fill;
          shape_inter=interpolation;
          build_drawing_svg(f, array, width, height, shape_fill, shape_inter, path_id);
        }

      fprintf(f, "</svg>\n");
      fclose(f);
    }
  else
    {
      file_error=TRUE;
      g_print("Couldn't open file bezier_drawing1.svg\n");
    }

  //Show the svg drawing.
  if(!file_error) svg_dialog(width, height);
}
static void build_drawing_svg(FILE *f, GArray *array, gint width, gint height, gint shape_fill, gint shape_inter, gint path_id)
{
  gint i=0;
  gint len=array->len;
  struct point p1;
  struct controls c1;

  if(len>0)
    {
      GArray *control1=NULL;
      GArray *mid_points=NULL;
      if(shape_inter==0)
        {
          //Just draw lines. Don't need control points.
        }
      else if(shape_inter==1)
        { 
          control1=control_points_from_coords2(array);
        }
      else
        {
          mid_points=mid_points_from_coords(array);
          len=mid_points->len;
          control1=control_points_from_coords2(mid_points);
        }

      if(shape_inter==0)
        {
          fprintf(f, "<polyline points=\"");
          p1=g_array_index(array, struct point, 0);
          for(i=0;i<len;i++)
            {
              p1=g_array_index(array, struct point, i);
              fprintf(f, "%i,%i ", (gint)p1.x, (gint)p1.y); 
            }
        }
      else if(shape_inter==1)
        {
          fprintf(f, "<path class=\"Path%i\" d=\"", path_id);
          p1=g_array_index(array, struct point, 0);
          fprintf(f, "M%i,%i ", (gint)p1.x, (gint)p1.y);
          for(i=1;i<len;i++)
            {
              p1=g_array_index(array, struct point, i);
              c1=g_array_index(control1, struct controls, i-1);
              fprintf(f, "C%i,%i %i,%i %i,%i ", (gint)c1.x1, (gint)c1.y1, (gint)c1.x2, (gint)c1.y2, (gint)p1.x, (gint)p1.y); 
            }
        }
      else
        {
          fprintf(f, "<path class=\"Path%i\" d=\"", path_id);
          p1=g_array_index(mid_points, struct point, 0);
          fprintf(f, "M%i,%i ", (gint)p1.x, (gint)p1.y);
          for(i=1;i<len;i++)
            {
              p1=g_array_index(mid_points, struct point, i);
              c1=g_array_index(control1, struct controls, i-1);
              fprintf(f, "C%i,%i %i,%i %i,%i ", (gint)c1.x1, (gint)c1.y1, (gint)c1.x2, (gint)c1.y2, (gint)p1.x, (gint)p1.y); 
            }
        }

      if(shape_fill==1) fprintf(f, "\"\nfill=\"url(#grad2)\" stroke=\"blue\" stroke-width=\"3\" transform=\"translate(%i,%i)\" />\n", width/2, height/2);
      else fprintf(f, "\"\n fill=\"none\" stroke=\"blue\" stroke-width=\"3\" transform=\"translate(%i,%i)\" />\n", width/2, height/2);
        
      if(control1!=NULL) g_array_free(control1, TRUE);
      if(mid_points!=NULL) g_array_free(mid_points, TRUE);
    }
  else
    {
      g_print("No coordinates to output svg with.\n");
    }
}
static void svg_dialog(gint width, gint height)
{
  GtkWidget *dialog=gtk_dialog_new_with_buttons("bezier_drawing1.svg", GTK_WINDOW(window), GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, NULL);
  gtk_window_set_default_size(GTK_WINDOW(dialog), width, height);
  GtkWidget *content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));

  GError *error=NULL;
  GdkPixbuf *svg=gdk_pixbuf_new_from_file("bezier_drawing1.svg", &error);
  if(error!=NULL) g_print("%s\n", error->message);
     
  //If error open image anyway. It will just show a broken image.
  GtkWidget *image=gtk_image_new_from_pixbuf(svg);
  gtk_widget_set_hexpand(image, TRUE);
  gtk_widget_set_vexpand(image, TRUE);

  GtkWidget *sw=gtk_scrolled_window_new(NULL, NULL);
  gtk_container_add(GTK_CONTAINER(sw), image);

  gtk_container_add(GTK_CONTAINER(content_area), sw);
  gtk_widget_show_all(dialog);

  gtk_dialog_run(GTK_DIALOG(dialog));
            
  gtk_widget_destroy(dialog); 
  if(error==NULL) g_object_unref(svg);
  else g_error_free(error);
                  
}
static void add_points(GtkWidget *widget, GtkWidget **widgets)
{
  g_print("Save Drawing\n");
  gint i=0;
  gint len=coords1->len;
  GArray *c1=g_array_sized_new(FALSE, FALSE, sizeof(struct point), len);
  for(i=0;i<len;i++)
    {
      g_array_append_val(c1, g_array_index(coords1, struct point, i));
    }
  g_ptr_array_add(paths, (GArray*)c1);
  g_array_append_val(path_info, interpolation);
  g_array_append_val(path_info, fill);

  //Reset coords to draw again.
  len=coords1->len; 
  gdouble w1=start_width*0.4;
  if(start_width>start_height) w1=start_height*0.4;
  struct point p1;
  for(i=0;i<len;i++) g_array_remove_index_fast(coords1, 0);

  for(i=0;i<len;i++)
    {
      p1.x=w1*cos((gdouble)i*G_PI/((gdouble)len/2.0));
      p1.y=w1*sin((gdouble)i*G_PI/((gdouble)len/2.0));
      //g_print("x %f y %f\n", p1.x, p1.y);
      g_array_append_val(coords1, p1);
    }

  gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(widgets[1]), FALSE);
  fill=0;

  GtkTreeStore *store=get_tree_store();
  gtk_tree_view_set_model(GTK_TREE_VIEW(widgets[2]), GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));
  
  gtk_widget_queue_draw(widgets[0]);
}
static void delete_shape(GtkWidget *widget, GtkWidget **widgets)
{
  g_print("Delete Shape\n");
  GtkTreeIter iter;
  GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets[2]));
  GtkTreeModel *model=gtk_tree_view_get_model(GTK_TREE_VIEW(widgets[2]));
  if(gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
      GtkTreePath *path=gtk_tree_model_get_path(model, &iter);
      if(gtk_tree_path_get_depth(path)==1)
        {
          gchar *string=gtk_tree_path_to_string(path);
          gint row_id=atoi(string);
          if(row_id<=(paths->len-1)&&(paths->len)>0)
            {
              gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
              g_array_free((GArray*)(g_ptr_array_index(paths, row_id)), TRUE);
              g_ptr_array_remove_index(paths, row_id);
            }
          g_free(string);
        }
      else
        {
          g_print("Can only delete top node shape.\n");
        }
    }

  gtk_tree_selection_unselect_all(selection);
  gtk_widget_queue_draw(widgets[0]);
}
static void clear_shapes(GtkWidget *widget, GtkWidget **widgets)
{
  gint i=0;
  gint len=paths->len;
  g_print("Free %i Paths\n", len);
  for(i=0;i<len;i++)
    {
      g_array_free((GArray*)(g_ptr_array_index(paths, i)), TRUE);
    }
  for(i=0;i<len;i++) g_ptr_array_remove_index_fast(paths, 0);

  len=path_info->len;
  for(i=0;i<len;i++) g_array_remove_index_fast(path_info, 0);

  GtkTreeStore *store=get_tree_store();
  gtk_tree_view_set_model(GTK_TREE_VIEW(widgets[2]), GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));

  gtk_widget_queue_draw(GTK_WIDGET(widgets[0]));
}
static GtkTreeStore* get_tree_store()
  {
    gint i=0;
    gint j=0;
    GtkTreeStore *store=gtk_tree_store_new(1, G_TYPE_STRING);
    gint len=0;
    GArray *array=NULL;
    if((paths->len)>0)
      {
        GtkTreeIter iter1;
        GtkTreeIter iter2;
        struct point p1;
        gtk_tree_store_append(store, &iter1, NULL);
        for(i=0;i<paths->len;i++)
          {
            gchar *string1=g_strdup_printf("Shape%i", i+1);
            gtk_tree_store_set(store, &iter1, 0, string1, -1);
            g_free(string1);
            len=((GArray*)(g_ptr_array_index(paths, i)))->len;
            array=((GArray*)(g_ptr_array_index(paths, i)));
            for(j=0;j<len;j++)
              {
                gtk_tree_store_append(store, &iter2, &iter1);  
                p1=g_array_index(array, struct point, j); 
                gchar *string2=g_strdup_printf("%i. x: %i, y: %i", j+1, (gint)p1.x, (gint)p1.y);
                gtk_tree_store_set(store, &iter2, 0, string2, -1);
                g_free(string2);
              }
            gtk_tree_store_append(store, &iter1, NULL);
          }
       }

    return store;
  }
static void cleanup(GtkWidget *widget, gpointer data)
{
  gint i=0;
  g_array_free(array_id, TRUE);
  g_array_free(coords1, TRUE);
  gint len=paths->len;
  for(i=0;i<len;i++)
    {
      g_array_free((GArray*)(g_ptr_array_index(paths, i)), TRUE);
    }
  g_ptr_array_free(paths, TRUE);
  g_array_free(path_info, TRUE);
  gtk_main_quit();
}






