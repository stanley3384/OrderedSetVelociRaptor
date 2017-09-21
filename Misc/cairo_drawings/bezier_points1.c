
/*
    This is testing putting together drag and drop with some linear, smooth and approximate
interpolation using Cairo and GTK. The drag and drop for the list box is from mclasen and
the following. 

    https://blog.gtk.org/2017/06/01/drag-and-drop-in-lists-revisited/

Drag a few points on the drawing for the interpolation. Hopefully changing the point order in the 
list box will change the draw order of the points. Still some things to figure out here. Try out
the animation rotating the drawing around the x, y, z or xyz axis. The drawing will output to svg
in the working directory and open the svg in an image widget. If the drawing is saved while it is 
animated, it will attach a little java script to the svg. If you open the svg in firefox, it will
be animated like in the program.

    For drawing, everything is three or more points. You can draw lines, curves and fill shapes
made of lines and curves. It takes a little while to get used to but it does make some types of
drawing simpler.

    gcc -Wall bezier_points1.c -o bezier_points1 `pkg-config --cflags --libs gtk+-3.0` -lm

    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>
#include<stdlib.h>
#include<string.h>

static void interpolation_combo(GtkComboBox *combo, gpointer data);
static void close_and_fill_check(GtkToggleButton *button, gpointer data);
static void save_top_check(GtkToggleButton *button, gpointer data);
static void rotate_combo(GtkComboBox *combo, gpointer data);
static gboolean animate_drawing(GtkWidget *drawing, GdkFrameClock *frame_clock, gpointer data);
static gboolean start_drawing(GtkWidget *widget, cairo_t *cr, gpointer data);
static void draw_shapes(GtkWidget *widget, cairo_t *cr, GArray *array, gint shape_fill, gint shape_inter, gboolean saved, gint *count_fill);
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
static gboolean key_delete_row(GtkWidget *row, GdkEventKey *event, gpointer data);
static void button_delete_row(GtkWidget *widget, GtkWidget **list_da);
static void change_selected_point(GtkWidget *widget, GtkWidget **list_da);
//Add a new point and row to the list box.
static void add_point(GtkWidget *widget, GtkWidget **list_da);
//Save to svg file.
static void save_svg(GtkWidget *widget, GtkWidget **widgets4);
static void build_gradient_svg(FILE *f);
static void build_drawing_svg(FILE *f, GArray *array, gint shape_fill, gint shape_inter, gint path_id, gint *count_fill_svg, gboolean top_drawing);
static void build_rotation_script_svg(FILE *f);
//Dialog for showing svg.
static void svg_dialog();
//Gradient color stops.
static void add_color_stop(GtkWidget *widget, GtkWidget **widgets2);
static void delete_color_stop(GtkWidget *widget, GtkWidget **widgets2);
static void update_linear_direction(GtkWidget *widget, GtkWidget **widgets2);  
//Save current drawing cordinates.
static void add_points(GtkWidget *widget, GtkWidget **widgets);
static void delete_shape(GtkWidget *widget, GtkWidget **widgets);
static void clear_shapes(GtkWidget *widget, GtkWidget **widgets);
//Get and parse the svg.
static void get_saved_svg(GtkWidget *widget, GtkWidget **widgets5);
static gchar* get_array_type_svg(gchar *p1, gboolean *found, gint *array_type);
static gchar* get_array_start_svg(gchar *p1);
static gchar* parse_array_svg(gchar *p1, GArray *array_temp);
static void build_array_svg(GArray *array_temp, gint array_type);
static GtkTreeStore* get_tree_store();
static GtkTreeStore* get_tree_store_fill();
//For the "Line" tab.
static void update_line_color(GtkWidget *widget, GtkWidget **line_widgets);
static void set_line_width(GtkComboBox *combo, gpointer data);
static void set_line_cap(GtkComboBox *combo, gpointer data);
static void set_layout(GtkWidget *widget, GtkWidget **widgets3);
//The about program dialog and drawing.
static void about_dialog(GtkWidget *widget, gpointer data);
static GdkPixbuf* draw_icon();
//General info message dialog.
static void message_dialog(gchar *msg);
//Clean up all those arrays.
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

//Points, controls and color stops for drawing with.
struct point{
  gdouble x;
  gdouble y;
}point;
struct controls{
  gdouble x1;
  gdouble y1;
  gdouble x2;
  gdouble y2;
}controls;
struct color_stop{
  gdouble p;
  gdouble r;
  gdouble g;
  gdouble b;
  gdouble a;
}color_stop;

/*
  Arrays for saving a drawing in a layer in the program. They are also used for outputing to svg
  and reading in text from the svg.
*/
//A pointer array to save garrays of coordinate points.
static GPtrArray *paths=NULL;
//Saved path draw info. interpolate and fill variables.
static GArray *path_info=NULL;
//The line color of the drawings.
static GArray *line_colors=NULL;
//The linear direction of the gradient.
static GArray *direction=NULL;
//The saved color stops for the gradients.
static GPtrArray *gradients=NULL;
//Saved line widths and line caps.
static GArray *line_widths=NULL;


/*
  Variables for the current or top drawing. These values can get saved to the above arrays. Current
  settings can also be output to svg if the "Save top drawing to SVG" checkbox is checked.
*/
//Coordinate points for the top or current drawing.
static GArray *coords1=NULL;
//Keeps track of where the coordinates are after drag and drop and deletes.
static GArray *array_id=NULL;
//Keeps track of the list box point names.
static GArray *point_id=NULL;
//Color stop array values shown in the treeview.
static GArray *color_stops=NULL;
//The line color to draw with.
static gdouble lca[]={0.0, 0.0, 1.0, 1.0};
//If the curve should be drawn linear, smooth or approximate. Start with smooth.
static gint interpolation=1;
//If the end of the curve connects to the start. If true fill the closed curve and draw a gradient.
static gint fill=0;
//Linear direction for the gradient
static gdouble ld[4];
//Line width setting from combo.
static gint line_width=3;
//Line cap setting from combo.
static gint line_cap=0;

//Drawing background color.
static gdouble b1[]={1.0, 1.0, 1.0, 1.0};
//Current selected row in list.
static gint row_id=0;
//Rotate and animate the drawing.
static gint rotate=0;
static gint begin_id=0;
//For blocking motion signal. Block when not drawing top rectangle
static gint motion_id=0;
static gdouble motion_x=0;
static gdouble motion_y=0;
//Tick id for animation frame clock.
static guint tick_id=0;
//GTK window for dialogs.
static GtkWidget *window;
//It the top drawing is included in the svg.
gboolean save_top=TRUE;
//Save initial drawing area dimensions.
static const gdouble start_width=400.0;
static const gdouble start_height=400.0;
//Initial drawing is 400x400 so need to be able to scale for different sizes in layout and svg.
static gdouble layout_width=400.0;
static gdouble layout_height=400.0;

int main(int argc, char *argv[])
  {
    gtk_init (&argc, &argv);

    window=gtk_window_new (GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Coordinates with Bezier Points");
    gtk_window_set_default_size(GTK_WINDOW(window), 850, 450);
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
    else
      {
        gchar *msg=g_strdup("Can't set window transparency.");
        message_dialog(msg);
        g_free(msg);
      } 
    GdkPixbuf *icon=draw_icon();
    gtk_window_set_default_icon(icon);

    GtkWidget *da=gtk_drawing_area_new();
    //Start with a 1000x1000 drawing area with a 400x400 drawing.
    gtk_widget_set_size_request(GTK_WIDGET(da), 1000, 1000);
    //Add some extra events to the top drawing area.
    gtk_widget_add_events(da, GDK_BUTTON_PRESS_MASK|GDK_BUTTON_RELEASE_MASK|GDK_POINTER_MOTION_MASK);
    g_signal_connect(da, "draw", G_CALLBACK(start_drawing), NULL);
    g_signal_connect(da, "button-press-event", G_CALLBACK(start_press), NULL);
    g_signal_connect(da, "button-release-event", G_CALLBACK(stop_press), NULL);
    motion_id=g_signal_connect(da, "motion-notify-event", G_CALLBACK(cursor_motion), NULL);
    g_signal_handler_block(da, motion_id);

    GtkWidget *da_sw=gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(da_sw), GTK_POLICY_ALWAYS, GTK_POLICY_ALWAYS);
    gtk_widget_set_hexpand(da_sw, TRUE);
    gtk_widget_set_vexpand(da_sw, TRUE);
    gtk_container_add(GTK_CONTAINER(da_sw), da);

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
    GtkListBoxRow *select_row=gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), 0);
    gtk_list_box_select_row(GTK_LIST_BOX(list), select_row);

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

    GtkWidget *delete_button=gtk_button_new_with_label("Delete Point");
    gtk_widget_set_hexpand(delete_button, FALSE);
    g_signal_connect(delete_button, "clicked", G_CALLBACK(button_delete_row), list_da);

    GtkWidget *point_button=gtk_button_new_with_label("Change Selected Point");
    gtk_widget_set_hexpand(point_button, FALSE);
    g_signal_connect(point_button, "clicked", G_CALLBACK(change_selected_point), list_da);

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
    
    GtkWidget *grid1=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid1), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid1), 8);
    gtk_grid_attach(GTK_GRID(grid1), label1, 0, 0, 2, 1);    
    gtk_grid_attach(GTK_GRID(grid1), sw, 0, 1, 2, 1); 
    gtk_grid_attach(GTK_GRID(grid1), button1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), delete_button, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), point_button, 0, 3, 1, 1);
    gtk_grid_attach(GTK_GRID(grid1), combo1, 0, 4, 1, 1); 
    gtk_grid_attach(GTK_GRID(grid1), combo2, 1, 4, 1, 1);  
    gtk_grid_attach(GTK_GRID(grid1), check1, 0, 5, 2, 1);     

    //Tab 2 "Line" widgets.
    GtkWidget *line_width1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(line_width1), "<span font_weight='heavy'>Line Width </span>");

    GtkWidget *combo_width=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo_width, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 0, "1", "1");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 1, "2", "2");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 2, "3", "3");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 3, "4", "4");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 4, "5", "5");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 5, "6", "6");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 6, "7", "7");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_width), 7, "8", "8");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_width), 2);
    g_signal_connect(combo_width, "changed", G_CALLBACK(set_line_width), da);

    GtkWidget *line_cap1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(line_cap1), "<span font_weight='heavy'>Line Cap </span>");

    GtkWidget *combo_cap=gtk_combo_box_text_new();
    gtk_widget_set_hexpand(combo_cap, TRUE);
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_cap), 0, "1", "Butt");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_cap), 1, "2", "Round");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo_cap), 2, "3", "Square");
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo_cap), 0);
    g_signal_connect(combo_cap, "changed", G_CALLBACK(set_line_cap), da);

    GtkWidget *line_label1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(line_label1), "<span font_weight='heavy'>Line Color </span>");

    GtkWidget *line_entry1=gtk_entry_new();
    gtk_widget_set_hexpand(line_entry1, TRUE);
    gtk_entry_set_text(GTK_ENTRY(line_entry1), "rgba(0, 0, 255, 1.0)");

    GtkWidget *line_button1=gtk_button_new_with_label("Update Line Color");
    gtk_widget_set_hexpand(line_button1, FALSE);
    GtkWidget *line_widgets[]={line_entry1, window, da};
    g_signal_connect(line_button1, "clicked", G_CALLBACK(update_line_color), line_widgets);

    GtkWidget *background_label=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(background_label), "<span font_weight='heavy'>Background </span>");

    GtkWidget *background_entry=gtk_entry_new();
    gtk_widget_set_hexpand(background_entry, TRUE);
    gtk_entry_set_text(GTK_ENTRY(background_entry), "rgba(255, 255, 255, 1.0)");

    GtkWidget *background_button=gtk_button_new_with_label("Update Background");
    gtk_widget_set_hexpand(background_button, FALSE);
    GtkWidget *colors[]={background_entry, window, da};
    g_signal_connect(background_button, "clicked", G_CALLBACK(check_colors), colors);

    GtkWidget *grid2=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid2), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid2), 8);
    gtk_grid_attach(GTK_GRID(grid2), line_width1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), combo_width, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), line_cap1, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), combo_cap, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), line_label1, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), line_entry1, 1, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), line_button1, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid2), background_label, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), background_entry, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid2), background_button, 0, 5, 2, 1);

    //Tab 3 "Fill" widgets.
    GtkWidget *label_stop=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_stop), "<span font_weight='heavy'>New Color Stop </span>");

    GtkWidget *entry_stop=gtk_entry_new();
    gtk_widget_set_hexpand(entry_stop, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry_stop), "(0.8, 0.5, 0.5, 0.5, 1.0)");

    GtkWidget *b_add=gtk_button_new_with_label("Add Color Stop");
    gtk_widget_set_hexpand(b_add, TRUE); 

    //Initialize default color stops and linear direction.
    struct color_stop st;
    color_stops=g_array_new(FALSE, FALSE, sizeof(struct color_stop));
    st.p=0.0; st.r=1.0; st.g=0.0; st.b=1.0, st.a=0.8;
    g_array_append_val(color_stops, st);
    st.p=0.5; st.r=1.0; st.g=1.0; st.b=0.0, st.a=0.8;
    g_array_append_val(color_stops, st);
    st.p=1.0; st.r=0.0; st.g=1.0; st.b=1.0, st.a=0.8;
    g_array_append_val(color_stops, st);
    ld[0]=0; ld[1]=0; ld[2]=0; ld[3]=100;

    GtkTreeStore *store_fill=get_tree_store_fill();
    GtkWidget *tree_fill=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store_fill));
    g_object_unref(G_OBJECT(store_fill));
    GtkCellRenderer *renderer_fill=gtk_cell_renderer_text_new();
    g_object_set(renderer_fill, "editable", FALSE, NULL);   
    GtkTreeViewColumn *column_fill1=gtk_tree_view_column_new_with_attributes("Color Stops", renderer_fill, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_fill), column_fill1); 

    GtkWidget *scroll_fill=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll_fill, TRUE);
    gtk_widget_set_hexpand(scroll_fill, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll_fill), tree_fill); 

    GtkWidget *b_delete=gtk_button_new_with_label("Delete Color Stop");
    gtk_widget_set_hexpand(b_delete, TRUE);

    GtkWidget *label_dir=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(label_dir), "<span font_weight='heavy'>Linear Direction </span>");

    GtkWidget *entry_dir=gtk_entry_new();
    gtk_widget_set_hexpand(entry_dir, TRUE);
    gtk_entry_set_text(GTK_ENTRY(entry_dir), "(0, 0, 0, 100)");

    GtkWidget *b_linear=gtk_button_new_with_label("Update Linear Direction");
    gtk_widget_set_hexpand(b_linear, TRUE);

    GtkWidget *widgets2[]={entry_stop, tree_fill, da, entry_dir};
    g_signal_connect(b_add, "clicked", G_CALLBACK(add_color_stop), widgets2);
    g_signal_connect(b_delete, "clicked", G_CALLBACK(delete_color_stop), widgets2);
    g_signal_connect(b_linear, "clicked", G_CALLBACK(update_linear_direction), widgets2);

    GtkWidget *grid3=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid3), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid3), 8);
    gtk_grid_attach(GTK_GRID(grid3), label_stop, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid3), entry_stop, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid3), b_add, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid3), scroll_fill, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid3), b_delete, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid3), label_dir, 0, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid3), entry_dir, 1, 4, 1, 1);
    gtk_grid_attach(GTK_GRID(grid3), b_linear, 0, 5, 2, 1);

    //Tab 4 "Add Shape" widgets.
    GtkWidget *shape_button1=gtk_button_new_with_label("Add Shape");
    gtk_widget_set_hexpand(shape_button1, TRUE);

    GtkWidget *shape_button2=gtk_button_new_with_label("Delete Shape");
    gtk_widget_set_hexpand(shape_button2, TRUE);

    GtkWidget *shape_button3=gtk_button_new_with_label("Clear Shapes");
    gtk_widget_set_hexpand(shape_button3, TRUE);

    paths=g_ptr_array_new();
    path_info=g_array_new(FALSE, FALSE, sizeof(gint));
    gradients=g_ptr_array_new();
    direction=g_array_new(FALSE, FALSE, sizeof(gdouble));
    line_colors=g_array_new(FALSE, FALSE, sizeof(gdouble));
    line_widths=g_array_new(FALSE, FALSE, sizeof(gint));

    GtkTreeStore *store=get_tree_store();
    GtkWidget *tree=gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    g_object_unref(G_OBJECT(store));
    GtkCellRenderer *renderer1=gtk_cell_renderer_text_new();
    g_object_set(renderer1, "editable", FALSE, NULL);   
    GtkTreeViewColumn *column1=gtk_tree_view_column_new_with_attributes("Shape Coordinates", renderer1, "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree), column1); 

    GtkWidget *widgets[]={da, check1, tree};
    g_signal_connect(shape_button1, "clicked", G_CALLBACK(add_points), widgets); 
    g_signal_connect(shape_button2, "clicked", G_CALLBACK(delete_shape), widgets); 
    g_signal_connect(shape_button3, "clicked", G_CALLBACK(clear_shapes), widgets);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), tree); 

    GtkWidget *grid4=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid4), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid4), 8);
    gtk_grid_attach(GTK_GRID(grid4), shape_button1, 0, 0, 2, 1);
    gtk_grid_attach(GTK_GRID(grid4), scroll, 0, 1, 2, 1);
    gtk_grid_attach(GTK_GRID(grid4), shape_button2, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid4), shape_button3, 1, 2, 1, 1);
    
    //Tab 5 "Save" widgets.
    GtkWidget *save_label1=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(save_label1), "<span font_weight='heavy'>Layout and SVG Width </span>");

    GtkWidget *save_entry1=gtk_entry_new();
    gtk_widget_set_hexpand(save_entry1, TRUE);
    gtk_entry_set_text(GTK_ENTRY(save_entry1), "400");

    GtkWidget *save_label2=gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(save_label2), "<span font_weight='heavy'>Layout and SVG Height </span>");

    GtkWidget *save_entry2=gtk_entry_new();
    gtk_widget_set_hexpand(save_entry2, TRUE);
    gtk_entry_set_text(GTK_ENTRY(save_entry2), "400");

    GtkWidget *save_button1=gtk_button_new_with_label("Save Layout and SVG Dimensions");
    gtk_widget_set_hexpand(save_button1, TRUE);
    GtkWidget *widgets3[]={save_entry1, save_entry2, da};
    g_signal_connect(save_button1, "clicked", G_CALLBACK(set_layout), widgets3);

    GtkWidget *ch1=gtk_check_button_new_with_label("Save Top Drawing to SVG");
    gtk_widget_set_halign(ch1, GTK_ALIGN_CENTER);
    gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(ch1), TRUE);
    g_signal_connect(ch1, "toggled", G_CALLBACK(save_top_check), da); 

    GtkWidget *save_entry3=gtk_entry_new();
    gtk_widget_set_hexpand(save_entry3, TRUE);
    gtk_entry_set_text(GTK_ENTRY(save_entry3), "bezier_drawing1.svg");

    GtkWidget *save_button2=gtk_button_new_with_label("Save Show SVG");
    gtk_widget_set_hexpand(save_button2, TRUE);
    GtkWidget *widgets4[]={da, save_entry3};
    g_signal_connect(save_button2, "clicked", G_CALLBACK(save_svg), widgets4);

    GtkWidget *save_entry4=gtk_entry_new();
    gtk_widget_set_hexpand(save_entry4, TRUE);
    gtk_entry_set_text(GTK_ENTRY(save_entry4), "bezier_drawing1.svg");

    GtkWidget *save_button3=gtk_button_new_with_label("Get Saved SVG");
    gtk_widget_set_hexpand(save_button3, TRUE);
    GtkWidget *widgets5[]={da, check1, tree, save_entry4};
    g_signal_connect(save_button3, "clicked", G_CALLBACK(get_saved_svg), widgets5);

    GtkWidget *grid5=gtk_grid_new();
    gtk_container_set_border_width(GTK_CONTAINER(grid5), 15);
    gtk_grid_set_row_spacing(GTK_GRID(grid5), 8);
    gtk_grid_attach(GTK_GRID(grid5), save_label1, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_entry1, 1, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_label2, 0, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_entry2, 1, 1, 1, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_button1, 0, 2, 2, 1);
    gtk_grid_attach(GTK_GRID(grid5), ch1, 0, 3, 2, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_entry3, 0, 4, 2, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_button2, 0, 5, 2, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_entry4, 0, 6, 2, 1);
    gtk_grid_attach(GTK_GRID(grid5), save_button3, 0, 7, 2, 1);

    GtkWidget *nb_label1=gtk_label_new("Draw Path");
    GtkWidget *nb_label2=gtk_label_new("Line");
    GtkWidget *nb_label3=gtk_label_new("Gradient");
    GtkWidget *nb_label4=gtk_label_new("Add Shape");
    GtkWidget *nb_label5=gtk_label_new("Save");
    GtkWidget *notebook=gtk_notebook_new();
    gtk_container_set_border_width(GTK_CONTAINER(notebook), 15);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid1, nb_label1);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid2, nb_label2);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid3, nb_label3);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid4, nb_label4);
    gtk_notebook_append_page(GTK_NOTEBOOK(notebook), grid5, nb_label5);

    GtkWidget *paned1=gtk_paned_new(GTK_ORIENTATION_HORIZONTAL);
    gtk_paned_pack1(GTK_PANED(paned1), notebook, FALSE, TRUE);
    gtk_paned_pack2(GTK_PANED(paned1), da_sw, TRUE, TRUE);
    gtk_paned_set_position(GTK_PANED(paned1), 400);
    //Draw background window based on the paned window splitter.
    g_signal_connect(window, "draw", G_CALLBACK(draw_main_window), paned1);

    GtkWidget *menu1=gtk_menu_new();
    GtkWidget *menu1item1=gtk_menu_item_new_with_label("Bezier Points1");
    gtk_menu_shell_append(GTK_MENU_SHELL(menu1), menu1item1);
    GtkWidget *title1=gtk_menu_item_new_with_label("About");
    gtk_menu_item_set_submenu(GTK_MENU_ITEM(title1), menu1);
    GtkWidget *menu_bar=gtk_menu_bar_new();
    gtk_menu_shell_append(GTK_MENU_SHELL(menu_bar), title1);
    g_signal_connect(menu1item1, "activate", G_CALLBACK(about_dialog), window);

    GtkWidget *grid6=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid6), menu_bar, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid6), paned1, 0, 1, 8, 8);

    gtk_container_add(GTK_CONTAINER(window), grid6);

    gint minor_version=gtk_get_minor_version();
    gchar *css_string=NULL;
    //GTK CSS changed in 3.20. The CSS for after 3.18 may need to be modified to have it work.
    if(minor_version>20)
      {
        css_string=g_strdup_printf("%s dialog {background: rgba(0,220,220,0.8);}", css);
      }
    else
      {
        css_string=g_strdup_printf("%s GtkDialog{background: rgba(0,220,220,0.8);}", css);
      }

    GError *css_error=NULL;
    GtkCssProvider *provider=gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    gtk_style_context_add_provider_for_screen(gdk_screen_get_default(), GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    if(css_error!=NULL)
      {
        g_print("CSS loader error %s\n", css_error->message);
        g_error_free(css_error);
      }
    g_object_unref(provider);
    g_free(css_string);

    gtk_widget_show_all(window);

    //Set some initial values for the drawing area.
    gdouble w1=start_width*0.4;
    motion_x=w1*cos(0.0);
    motion_y=w1*sin(0.0);
    struct point p1;
    coords1=g_array_sized_new(FALSE, FALSE, sizeof(struct point), 12);
    for(i=0;i<12;i++)
      {
        p1.x=w1*cos((gdouble)i*G_PI/6.0);
        p1.y=w1*sin((gdouble)i*G_PI/6.0);
        g_array_append_val(coords1, p1);
      }
    //Array to track the list.
    array_id=g_array_new (FALSE, FALSE, sizeof(gint));
    point_id=g_array_new (FALSE, FALSE, sizeof(gint));
    for(i=0;i<12;i++)
      {
        g_array_append_val(array_id, i);
        g_array_append_val(point_id, i);
      }
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
    gint len=0;
    GArray *array=NULL;
    gint shape_fill=0;
    gint shape_inter=0;
    gboolean saved=TRUE;
    //Count the drawings with a fill gradient to match paths with gradients.
    gint count_fill=0;

    gdouble width=layout_width;
    gdouble height=layout_height;
    gdouble w1=width/10.0;
    gdouble h1=height/10.0;

    cairo_set_source_rgba(cr, b1[0], b1[1], b1[2], b1[3]);
    cairo_paint(cr);

    //Layout axis for drawing. Outer box.
    cairo_set_line_width(cr, 2.0);
    cairo_set_source_rgba(cr, 0.0, 0.0, 0.0, 1.0);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);

    //Inner box
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

    //The x, y coordinates of the selected point below the box.
    cairo_move_to(cr, 20, height+30);
    gchar *motion=g_strdup_printf("X: %f Y: %f", motion_x, motion_y);
    cairo_select_font_face(cr, "Arial", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 16);
    cairo_show_text(cr, motion);
    g_free(motion);

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
    gdouble lca0=0;
    gdouble lca1=0;
    gdouble lca2=0;
    gdouble lca3=0;
    //line width.
    gint lw=0;
    //line cap.
    gint line_c=0;
    len=paths->len;
    for(i=0;i<len;i++)
      {
        array=(GArray*)(g_ptr_array_index(paths, i));
        shape_inter=g_array_index(path_info, gint, 2*i);
        shape_fill=g_array_index(path_info, gint, 2*i+1); 
        lca0=g_array_index(line_colors, gdouble, 4*i);
        lca1=g_array_index(line_colors, gdouble, 4*i+1);
        lca2=g_array_index(line_colors, gdouble, 4*i+2);
        lca3=g_array_index(line_colors, gdouble, 4*i+3);       
        cairo_set_source_rgba(cr, lca0, lca1, lca2, lca3);
        lw=g_array_index(line_widths, gint, 2*i);
        line_c=g_array_index(line_widths, gint, 2*i+1);
        cairo_set_line_width(cr, lw);
        cairo_set_line_cap(cr, line_c); 
        draw_shapes(widget, cr, array, shape_fill, shape_inter, saved, &count_fill);
        cairo_new_path(cr);
      }

    //Draw top or current drawing.
    array=coords1;
    shape_fill=fill;
    shape_inter=interpolation;
    saved=FALSE;
    cairo_set_source_rgba(cr, lca[0], lca[1], lca[2], lca[3]);
    cairo_set_line_width(cr, line_width);
    cairo_set_line_cap(cr, line_cap); 
    draw_shapes(widget, cr, array, shape_fill, shape_inter, saved, &count_fill);
    return FALSE;
  }
static void draw_shapes(GtkWidget *widget, cairo_t *cr, GArray *array, gint shape_fill, gint shape_inter, gboolean saved, gint *count_fill)
  {
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

    //Fill the saved paths with the saved gradient.
    if(shape_fill==1&&saved) 
      {
        cairo_close_path(cr);
        struct color_stop st;
        gdouble x1=g_array_index(direction, gdouble, 4*(*count_fill))/100.0*layout_width-layout_width/2.0;
        gdouble y1=g_array_index(direction, gdouble, 4*(*count_fill)+1)/100.0*layout_height-layout_height/2.0;
        gdouble x2=g_array_index(direction, gdouble, 4*(*count_fill)+2)/100.0*layout_width-layout_width/2.0;
        gdouble y2=g_array_index(direction, gdouble, 4*(*count_fill)+3)/100.0*layout_height-layout_height/2.0;
        GArray *array=((GArray*)(g_ptr_array_index(gradients, (*count_fill))));
        cairo_pattern_t *pattern1=cairo_pattern_create_linear(x1, y1, x2, y2);
        for(i=0;i<array->len;i++)
          {
            st=g_array_index(array, struct color_stop, i);
            cairo_pattern_add_color_stop_rgba(pattern1, st.p, st.r, st.g, st.b, st.a); 
          }  
        cairo_set_source(cr, pattern1);  
        cairo_fill(cr);
        (*count_fill)++;
        cairo_pattern_destroy(pattern1);
      }

    //Fill the top path with a gradient.
    if(shape_fill==1&&color_stops->len>0&&!saved) 
      {
        cairo_close_path(cr);
        struct color_stop st;
        gdouble x1=ld[0]/100.0*layout_width-layout_width/2.0;
        gdouble y1=ld[1]/100.0*layout_height-layout_height/2.0;
        gdouble x2=ld[2]/100.0*layout_width-layout_width/2.0;
        gdouble y2=ld[3]/100.0*layout_height-layout_height/2.0;
        cairo_pattern_t *pattern1=cairo_pattern_create_linear(x1, y1, x2, y2);
        for(i=0;i<color_stops->len;i++)
          {
            st=g_array_index(color_stops, struct color_stop, i);
            cairo_pattern_add_color_stop_rgba(pattern1, st.p, st.r, st.g, st.b, st.a); 
          }  
        cairo_set_source(cr, pattern1);  
        cairo_fill(cr);
        cairo_pattern_destroy(pattern1);
      }

    if(!saved)
      {
        cairo_select_font_face(cr, "Serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
        cairo_set_font_size(cr, 14);
        len=point_id->len;
        for(i=0;i<len;i++)
          {
            id=g_array_index(point_id, gint, i);
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
    //Don't unblock if the drawing is animated and rotating.
    if(rotate==0)
      {
        g_signal_handler_unblock(widget, motion_id); 
        GdkWindow *win=gtk_widget_get_window(widget);
        GdkDisplay *display=gdk_window_get_display(win);
        GdkCursor *cursor=gdk_cursor_new_from_name(display, "move");
        gdk_window_set_cursor(win, cursor);
        g_object_unref(cursor); 
      }

    return TRUE;
  }
static gboolean stop_press(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    if(rotate==0)
      {
        g_signal_handler_block(widget, motion_id);
        GdkWindow *win=gtk_widget_get_window(widget);
        GdkDisplay *display=gdk_window_get_display(win);
        GdkCursor *cursor=gdk_cursor_new_from_name(display, "default");
        gdk_window_set_cursor(win, cursor);
        g_object_unref(cursor);
      } 

    return TRUE;
  }
static gboolean cursor_motion(GtkWidget *widget, GdkEvent *event, gpointer data)
  {
    struct point p1;

    //Update the coordinates in the drawing.
    motion_x=event->button.x-layout_width/2.0;
    motion_y=-(event->button.y-layout_height/2.0);

    gdouble w1=0.5*layout_width-0.5*(layout_width-start_width);
    gdouble h1=0.5*layout_height-0.5*(layout_height-start_height);

    p1.x=event->button.x*start_width/layout_width-w1;
    p1.y=event->button.y*start_height/layout_height-h1;

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
        gchar *msg=g_strdup("Color string format error in background color");
        message_dialog(msg);
        g_free(msg);
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
    cairo_set_source_rgba(cr, 0.0, 0.86, 0.86, 0.8);
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
        g_warning("Can't get control points from coordinates. NULL returned.\n");
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
  GtkWidget *listbox;
  GtkAllocation alloc;
  cairo_surface_t *surface;
  cairo_t *cr;
  int x, y;

  row = gtk_widget_get_ancestor (widget, GTK_TYPE_LIST_BOX_ROW);
  listbox=gtk_widget_get_parent(row);
  //Make sure the row gets selected and begin_id gets updated.
  gtk_list_box_select_row(GTK_LIST_BOX(listbox), GTK_LIST_BOX_ROW(row));
  begin_id=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
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

  row_id=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
  gint i=0;
  gint value_id=0;
  gint value_id2=0;
  gint len=0;
  struct point p1;

  if(begin_id!=row_id)
    {
      value_id=g_array_index(array_id, gint, begin_id);
      value_id2=g_array_index(point_id, gint, begin_id);
      p1=g_array_index(coords1, struct point, begin_id);
      if(begin_id>row_id)
        {
          g_array_insert_val(array_id, row_id, value_id);
          g_array_insert_val(point_id, row_id, value_id2);
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
          len=point_id->len-1;
          for(i=len;i>-1;i--)
            {
              if(g_array_index(point_id, gint, i)==value_id2)
                {
                  g_array_remove_index(point_id, i);
                  break;
                }
            }
        }
      else
        {
          g_array_insert_val(array_id, row_id+1, value_id);
          g_array_insert_val(point_id, row_id+1, value_id2);
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
          len=point_id->len-1;
          for(i=0;i<len;i++)
            {
              if(g_array_index(point_id, gint, i)==value_id2)
                {
                  g_array_remove_index(point_id, i);
                  break;
                }
            }
        }
    }
  
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

  g_signal_connect (row, "key-press-event", G_CALLBACK (key_delete_row), da);

  return row;
}

static void
on_row_activated (GtkListBox *self, GtkListBoxRow *row, gpointer data)
{
  struct point p1;
  row_id=gtk_list_box_row_get_index(row);
  begin_id=row_id;
  p1=g_array_index(coords1, struct point, begin_id);
  motion_x=p1.x*layout_width/start_width;
  motion_y=-(p1.y*layout_height/start_height);
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

static gboolean key_delete_row(GtkWidget *row, GdkEventKey *event, gpointer data)
{
  if(event->keyval==GDK_KEY_Delete)
    {
      if(array_id->len>3)
        {
          gint i=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
          gint j=g_array_index(array_id, gint , i);
          //Adjust indexes into coords.
          gint id=0;
          gint *idp=&g_array_index(array_id, gint , 0);
          for(id=0;id<array_id->len;id++)
            {
              if(g_array_index(array_id, gint , id)>=i) *idp=*idp-1;
              idp++;
            }
          //Remove the point.
          g_array_remove_index(point_id, i);
          g_array_remove_index(array_id, i);
          g_array_remove_index(coords1, j);
          gtk_widget_destroy(row);
          gtk_widget_queue_draw(GTK_WIDGET(data));
        }
      else
        {
          gchar *msg=g_strdup("Couldn't delete row. There needs to be at least 3 points to draw with.");
          message_dialog(msg);
          g_free(msg);
        }
    }
  return TRUE;
}
static void button_delete_row(GtkWidget *widget, GtkWidget **list_da)
{
  if(array_id->len>3)
    {
      GtkListBoxRow *row=NULL;
      row=gtk_list_box_get_selected_row(GTK_LIST_BOX(list_da[0]));
      if(row!=NULL&&gtk_list_box_row_is_selected(row))
        {
          gint i=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
          gint j=g_array_index(array_id, gint , i);
          //Adjust indexes into coords.
          gint id=0;
          gint *idp=&g_array_index(array_id, gint , 0);
          for(id=0;id<array_id->len;id++)
            {
              if(g_array_index(array_id, gint , id)>=i) *idp=*idp-1; 
              idp++;
            }
          
          //Remove the point.
          g_array_remove_index(point_id, i);
          g_array_remove_index(array_id, i);
          g_array_remove_index(coords1, j);
          gtk_widget_destroy(GTK_WIDGET(row));
          gtk_widget_queue_draw(GTK_WIDGET(list_da[1]));
        }
      else
        {
          gchar *msg=g_strdup("No rows are selected in the list box.");
          message_dialog(msg);
          g_free(msg);
        }
    }
  else
    {
      gchar *msg=g_strdup("Couldn't delete row. There needs to be at least 3 points to draw with.");
      message_dialog(msg);
      g_free(msg);
    }
}
static void change_selected_point(GtkWidget *widget, GtkWidget **list_da)
{
  GtkWidget *dialog=gtk_dialog_new_with_buttons ("Change Selected Point", GTK_WINDOW(window), GTK_DIALOG_MODAL|GTK_DIALOG_DESTROY_WITH_PARENT, "Cancel", GTK_RESPONSE_REJECT, "OK", GTK_RESPONSE_ACCEPT, NULL);
  gtk_window_set_default_size(GTK_WINDOW(dialog), 250, 100);

  GtkWidget *x_label=gtk_label_new("X:");
  gtk_widget_set_hexpand(x_label, TRUE);
  gtk_widget_set_halign(x_label, GTK_ALIGN_CENTER);
  GtkWidget *y_label=gtk_label_new("Y:");
  gtk_widget_set_halign(y_label, GTK_ALIGN_CENTER);
  GtkWidget *x_entry=gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(x_entry), "0");
  GtkWidget *y_entry=gtk_entry_new();
  gtk_entry_set_text(GTK_ENTRY(y_entry), "0");

  GtkWidget *grid=gtk_grid_new();
  gtk_container_set_border_width(GTK_CONTAINER(grid), 15);
  gtk_grid_set_row_spacing(GTK_GRID(grid), 8);
  gtk_grid_attach(GTK_GRID(grid), x_label, 0, 0, 1, 1); 
  gtk_grid_attach(GTK_GRID(grid), x_entry, 1, 0, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), y_label, 0, 1, 1, 1);
  gtk_grid_attach(GTK_GRID(grid), y_entry, 1, 1, 1, 1);

  GtkWidget *content_area=gtk_dialog_get_content_area(GTK_DIALOG(dialog));
  gtk_container_add(GTK_CONTAINER(content_area), grid);
  gtk_widget_show_all(dialog);

  gint response=gtk_dialog_run(GTK_DIALOG(dialog));
  if(response==GTK_RESPONSE_ACCEPT)
    {
      GtkListBoxRow *row=NULL;
      row=gtk_list_box_get_selected_row(GTK_LIST_BOX(list_da[0]));
      if(row!=NULL&&gtk_list_box_row_is_selected(row))
        {
          gdouble x_coord=strtod(gtk_entry_get_text(GTK_ENTRY(x_entry)), NULL);
          gdouble y_coord=strtod(gtk_entry_get_text(GTK_ENTRY(y_entry)), NULL);
          //A return of 0 is OK. Set general limits based on the drawing area initail size.
          if(x_coord>=-1000&&x_coord<=1000&&y_coord>=-1000&&y_coord<=1000)
            {
              gint i=gtk_list_box_row_get_index(GTK_LIST_BOX_ROW(row));
              struct point *p1=&g_array_index(coords1, struct point, i);
              (p1->x)=x_coord;
              (p1->y)=-y_coord; 
              motion_x=x_coord;
              motion_y=y_coord;         
              gtk_widget_queue_draw(GTK_WIDGET(list_da[1]));
            }
          else
            {
              gchar *msg=g_strdup("The x and y coordinate range: -1000<=x,y<=1000");
              message_dialog(msg);
              g_free(msg);
            }
        }
      else
        {
          gchar *msg=g_strdup("No rows are selected in the list box.");
          message_dialog(msg);
          g_free(msg);
        }
    }

  gtk_widget_destroy(dialog);
}
static void add_point(GtkWidget *widget, GtkWidget **list_da)
{
  gint i=0;
  gint len=0;
  gint max=0;
  struct point p1;
  gint id=array_id->len;

  //Find the current max point id.
  for(i=0;i<point_id->len;i++)
    {
      if(g_array_index(point_id, gint, i)>max) max=g_array_index(point_id, gint, i);
    }
  max++;
  g_array_append_val(array_id, id);
  g_array_append_val(point_id, max);
  len=coords1->len-1;
  p1=g_array_index(coords1, struct point, len);
  p1.x=p1.x-40.0;
  g_array_append_val(coords1, p1);

  gchar *point=g_strdup_printf("Point %i", max+1);
  GtkWidget *row=create_row(point, list_da[1]);
  gtk_list_box_insert(GTK_LIST_BOX(list_da[0]), row, -1);
  gtk_widget_show_all(row);
  gtk_widget_queue_draw(GTK_WIDGET(list_da[1]));
  g_free(point);
}
static void save_svg(GtkWidget *widget, GtkWidget **widgets4)
{
  gint i=0;
  gint j=0;
  gint path_id=1;
  gint len=0;
  gint array_len=0;
  GArray *array=NULL;
  gint shape_fill=0;
  gint shape_inter=0;
  gdouble lca0=0;
  gdouble lca1=0;
  gdouble lca2=0;
  gdouble lca3=0;
  //line width.
  gint lw=0;
  gint line_c=0;
  gboolean top_drawing=FALSE;
  gboolean file_error=FALSE;
  struct point p1;
  struct color_stop st;
  //Need to count drawings with a fill gradient to match the path with the color stops.
  gint count_fill_svg=0;

  FILE *f=fopen(gtk_entry_get_text(GTK_ENTRY(widgets4[1])), "w");
  if(f!=NULL)
    {
      fprintf(f, "<?xml version=\"1.0\" standalone=\"no\"?>\n");
      fprintf(f, "<!DOCTYPE svg PUBLIC \"-//W3C//DTD SVG 1.1//EN\"\n"); 
      fprintf(f, "\"http://www.w3.org/Graphics/SVG/1.1/DTD/svg11.dtd\">\n");
      fprintf(f, "<svg width=\"%i\" height=\"%i\" viewBox=\"0 0 %i %i\"\n", (gint)layout_width, (gint)layout_height, (gint)layout_width, (gint)layout_height);
      fprintf(f, "xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\">\n");

      //Add xml comment to store data for the bezier_points1 program. 
      fprintf(f, "<!--\n");
      fprintf(f, "bezier_points1 program data.\n");

      //Save the gradient arrays.
      
      //Save the path arrays.
      len=paths->len;
      gdouble x1=0;
      gdouble y1=0;
      gdouble x2=0;
      gdouble y2=0;
      gint lcf=0;
      for(i=0;i<len;i++)
        {
          array=(GArray*)(g_ptr_array_index(paths, i));
          shape_inter=g_array_index(path_info, gint, 2*i);
          shape_fill=g_array_index(path_info, gint, 2*i+1);
          fprintf(f, "Shape%i [", i+1);
          array_len=array->len;
          for(j=0;j<array_len;j++)
            {
              p1=g_array_index(array, struct point, j);
              fprintf(f, "%f %f ", p1.x, p1.y);
            }
          fprintf(f, "]\nInterpolationFill [%i %i]\n", shape_inter, shape_fill);

          lca0=g_array_index(line_colors, gdouble, 4*i);
          lca1=g_array_index(line_colors, gdouble, 4*i+1);
          lca2=g_array_index(line_colors, gdouble, 4*i+2);
          lca3=g_array_index(line_colors, gdouble, 4*i+3);
          fprintf(f, "LineColor [%f %f %f %f]\n", lca0, lca1, lca2, lca3);
          lw=g_array_index(line_widths, gint, 2*i);
          line_c=g_array_index(line_widths, gint, 2*i+1);
          fprintf(f, "WidthCap [%i %i]\n", lw, line_c);

          if(shape_fill==1)
            {
              x1=g_array_index(direction, gdouble, 4*lcf);
              y1=g_array_index(direction, gdouble, 4*lcf+1);
              x2=g_array_index(direction, gdouble, 4*lcf+2);
              y2=g_array_index(direction, gdouble, 4*lcf+3);
              fprintf(f, "Direction [%f %f %f %f]\n", x1, y1, x2, y2);
              array=(GArray*)(g_ptr_array_index(gradients, lcf));
              if(array->len>0)
                {
                  fprintf(f, "Gradient [");
                  for(j=0;j<array->len;j++)
                    {
                      st=g_array_index(array, struct color_stop, j);
                      fprintf(f, "%f %f %f %f %f ", st.p, st.r, st.g, st.b, st.a);
                    }
                  fprintf(f, "]\n");
                }
              lcf++;
            }          
        }

      //Top drawing from the coords array.
      if(save_top)
        {
          array=coords1;
          shape_fill=fill;
          shape_inter=interpolation;
          fprintf(f, "Top [");
          len=array->len;
          for(i=0;i<len;i++)
            {
              p1=g_array_index(array, struct point, i);
              fprintf(f, "%f %f ", p1.x, p1.y);
            }
          fprintf(f, "]\nInterpolationFill [%i %i]\n", shape_inter, shape_fill); 
          fprintf(f, "LineColor [%f %f %f %f]\n", lca[0], lca[1], lca[2], lca[3]);
          fprintf(f, "WidthCap [%i %i]\n", line_width, line_cap);
          if(fill==1)
            {
              fprintf(f, "Direction [%f %f %f %f]\n", ld[0], ld[1], ld[2], ld[3]);
              if(color_stops->len>0)
                {
                  fprintf(f, "Gradient [");
                  for(i=0;i<color_stops->len;i++)
                    {
                      st=g_array_index(color_stops, struct color_stop, i);
                      fprintf(f, "%f %f %f %f %f ", st.p, st.r, st.g, st.b, st.a);  
                    }
                  fprintf(f, "]\n");
                }
            }
        }
      fprintf(f, "-->\n");

      //Fill the background color in.
      fprintf(f, "<rect x=\"0\" y=\"0\" width=\"%i\" height=\"%i\" fill=\"rgb(%i,%i,%i)\" fill-opacity=\"%f\" />\n", (gint)layout_width, (gint)layout_height, (gint)(b1[0]*255.0), (gint)(b1[1]*255.0), (gint)(b1[2]*255.0), b1[3]);

      //Apply the transforms.
      fprintf(f, "<g id=\"scale_drawing\" transform=\"translate(%i, %i) scale(%f, %f) rotate(0)\">\n", (gint)(layout_width/2.0), (gint)(layout_height/2.0), layout_width/start_width, layout_height/start_height);

      //Define a gradient fill to be used.
      build_gradient_svg(f);
      
      //Build the svg from the stored arrays.
      len=paths->len;
      for(i=0;i<len;i++)
        {
          array=(GArray*)(g_ptr_array_index(paths, i));
          shape_inter=g_array_index(path_info, gint, 2*i);
          shape_fill=g_array_index(path_info, gint, 2*i+1);
          build_drawing_svg(f, array, shape_fill, shape_inter, path_id, &count_fill_svg, top_drawing);
          path_id++;
        }

      //Draw top or current drawing from the coords array.
      if(save_top)
        {
          array=coords1;
          shape_fill=fill;
          shape_inter=interpolation;
          top_drawing=TRUE;
          build_drawing_svg(f, array, shape_fill, shape_inter, path_id, &count_fill_svg, top_drawing);
        }

      //If the drawing is animated, add the java script for rotation.
      if(rotate>0) build_rotation_script_svg(f);

      fprintf(f, "</g>\n</svg>\n");
      fclose(f);
    }
  else
    {
      file_error=TRUE;
      gchar *msg=g_strdup("Couldn't open file bezier_drawing1.svg.");
      message_dialog(msg);
      g_free(msg);
    }

  //Show the svg drawing.
  if(!file_error) svg_dialog();
}
static void build_gradient_svg(FILE *f)
  {
    gint i=0;
    gint j=0;
    gint x1=0;
    gint y1=0;
    gint x2=0;
    gint y2=0;
    gint len=gradients->len;
    struct color_stop cs;
    GArray *array=NULL;
 
    //Gradient for the top drawing.
    if(save_top)
      {
        fprintf(f, "<defs>\n");
        fprintf(f, "<linearGradient id=\"grad_top\" x1=\"%i%%\" y1=\"%i%%\" x2=\"%i%%\" y2=\"%i%%\">\n", (gint)ld[0], (gint)ld[1], (gint)ld[2], (gint)ld[3]);
        for(i=0;i<color_stops->len;i++)
          {
            cs=g_array_index(color_stops, struct color_stop, i);
            cs.p=cs.p*100.0;
            cs.r=cs.r*255.0;
            cs.g=cs.g*255.0;
            cs.b=cs.b*255.0;
            fprintf(f, "<stop offset=\"%i%%\" style=\"stop-color:rgb(%i,%i,%i);stop-opacity:%f\" />\n", (gint)cs.p, (gint)cs.r, (gint)cs.g, (gint)cs.b, cs.a);
          }
        fprintf(f, "</linearGradient>\n");
        fprintf(f, "</defs>\n");   
      }

    //The stored gradient array.
    for(i=0;i<len;i++)
      {
        x1=(gint)g_array_index(direction, gdouble, 4*i);
        y1=(gint)g_array_index(direction, gdouble, 4*i+1);
        x2=(gint)g_array_index(direction, gdouble, 4*i+2);
        y2=(gint)g_array_index(direction, gdouble, 4*i+3);
        fprintf(f, "<defs>\n");
        fprintf(f, "<linearGradient id=\"grad%i\" x1=\"%i%%\" y1=\"%i%%\" x2=\"%i%%\" y2=\"%i%%\">\n", i, x1, y1, x2, y2);
        array=((GArray*)(g_ptr_array_index(gradients, i)));
        for(j=0;j<array->len;j++)
          {
            cs=g_array_index(array, struct color_stop, j);
            cs.p=cs.p*100.0;
            cs.r=cs.r*255.0;
            cs.g=cs.g*255.0;
            cs.b=cs.b*255.0;
            fprintf(f, "<stop offset=\"%i%%\" style=\"stop-color:rgb(%i,%i,%i);stop-opacity:%f\" />\n", (gint)cs.p, (gint)cs.r, (gint)cs.g, (gint)cs.b, cs.a);
          }
        fprintf(f, "</linearGradient>\n");
        fprintf(f, "</defs>\n");   
      }

  }
static void build_drawing_svg(FILE *f, GArray *array, gint shape_fill, gint shape_inter, gint path_id, gint *count_fill_svg, gboolean top_drawing)
{
  gint i=0;
  //line color.
  gdouble lca0=0;
  gdouble lca1=0;
  gdouble lca2=0;
  gdouble lca3=0;
  //line width.
  gint lw=0;
  //line cap.
  gint line_c=0;
  gchar *caps[]={"butt", "round", "square"};
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
          fprintf(f, "M%f,%f ", p1.x, p1.y);
          for(i=1;i<len;i++)
            {
              p1=g_array_index(array, struct point, i);
              c1=g_array_index(control1, struct controls, i-1);
              fprintf(f, "C%f,%f %f,%f %f,%f ", c1.x1, c1.y1, c1.x2, c1.y2, p1.x, p1.y); 
            }
        }
      else
        {
          fprintf(f, "<path class=\"Path%i\" d=\"", path_id);
          p1=g_array_index(mid_points, struct point, 0);
          fprintf(f, "M%f,%f ", p1.x, p1.y);
          for(i=1;i<len;i++)
            {
              p1=g_array_index(mid_points, struct point, i);
              c1=g_array_index(control1, struct controls, i-1);
              fprintf(f, "C%f,%f %f,%f %f,%f ", c1.x1, c1.y1, c1.x2, c1.y2, p1.x, p1.y); 
            }
        }

      if(shape_fill==1)
        {
          if(save_top&&top_drawing)
            {
              fprintf(f, "\"\nfill=\"url(#grad_top)\" stroke=\"rgb(%i,%i,%i)\" stroke-opacity=\"%f\" stroke-width=\"%i\" stroke-linecap=\"%s\" />\n", (gint)(lca[0]*255.0), (gint)(lca[1]*255.0), (gint)(lca[2]*255.0), lca[3], line_width, caps[line_cap]);
            }
          else
            {
              lca0=g_array_index(line_colors, gdouble, 4*(path_id-1))*255.0;
              lca1=g_array_index(line_colors, gdouble, 4*(path_id-1)+1)*255.0;
              lca2=g_array_index(line_colors, gdouble, 4*(path_id-1)+2)*255.0;
              lca3=g_array_index(line_colors, gdouble, 4*(path_id-1)+3)*255.0;
              lw=g_array_index(line_widths, gint, 2*(path_id-1));
              line_c=g_array_index(line_widths, gint, 2*(path_id-1)+1);
              fprintf(f, "\"\nfill=\"url(#grad%i)\" stroke=\"rgb(%i,%i,%i)\" stroke-opacity=\"%f\" stroke-width=\"%i\" stroke-linecap=\"%s\" />\n", *count_fill_svg, (gint)lca0, (gint)lca1, (gint)lca2, lca3, lw, caps[line_c]);
              (*count_fill_svg)++;
            }
        }
      else
        {
          if(save_top&&top_drawing)
            {
              fprintf(f, "\"\n fill=\"none\" stroke=\"rgb(%i,%i,%i)\" stroke-opacity=\"%f\" stroke-width=\"%i\" stroke-linecap=\"%s\" />\n", (gint)(lca[0]*255.0), (gint)(lca[1]*255.0), (gint)(lca[2]*255.0), lca[3], line_width, caps[line_cap]);
            }
          else
            {
              lca0=g_array_index(line_colors, gdouble, 4*(path_id-1))*255.0;
              lca1=g_array_index(line_colors, gdouble, 4*(path_id-1)+1)*255.0;
              lca2=g_array_index(line_colors, gdouble, 4*(path_id-1)+2)*255.0;
              lca3=g_array_index(line_colors, gdouble, 4*(path_id-1)+3)*255.0;
              lw=g_array_index(line_widths, gint, 2*(path_id-1));
              line_c=g_array_index(line_widths, gint, 2*(path_id-1)+1);
              fprintf(f, "\"\n fill=\"none\" stroke=\"rgb(%i,%i,%i)\" stroke-opacity=\"%f\" stroke-width=\"%i\" stroke-linecap=\"%s\" />\n", (gint)lca0, (gint)lca1, (gint)lca2, lca3, lw, caps[line_c]);
            }
        }
        
      if(control1!=NULL) g_array_free(control1, TRUE);
      if(mid_points!=NULL) g_array_free(mid_points, TRUE);
    }
  else
    {
      gchar *msg=g_strdup("No coordinates to output svg with.");
      message_dialog(msg);
      g_free(msg);
    }
}
static void build_rotation_script_svg(FILE *f)
  {
    //Add java script to the svg if the drawing is animated.
    if(rotate==1)
      {
        fprintf(f, "<script>\n"
                   "var scale_drawing = document.getElementById(\"scale_drawing\");\n"
                   "var j = 0;\n"
                   "function redraw() {\n"
                   "var angle = -j * Math.PI / 256.0 - 3.0 * Math.PI / 2.0;\n"
                   "var scale = Math.sin(angle) * %f;\n"
                   "var scale_inv = 1.0 / scale * %i;\n"
                   "j += 1.0;\n"
                   "scale_drawing.setAttribute(\"transform\", \"scale(%f, \" + scale + \") translate(%i, \" + scale_inv + \")\");\n"
                   "}\n"
                   "setInterval(\"redraw()\", 16.7);\n"
                   "</script>\n", layout_height/start_height, (gint)(layout_height/2.0), layout_width/start_width, (gint)(layout_width/2.0));
      }
    if(rotate==2)
      {
        fprintf(f, "<script>\n"
                   "var scale_drawing = document.getElementById(\"scale_drawing\");\n"
                   "var j = 0;\n"
                   "function redraw() {\n"
                   "var angle = -j * Math.PI / 256.0;\n"
                   "var scale = Math.cos(angle) * %f;\n"
                   "var scale_inv = 1.0 / scale * %i;\n"
                   "j += 1.0;\n"
                   "scale_drawing.setAttribute(\"transform\", \"scale(\" + scale + \", %f) translate(\" + scale_inv + \", %i)\");\n"
                   "}\n"
                   "setInterval(\"redraw()\", 16.7);\n"
                   "</script>\n", layout_width/start_width, (gint)(layout_width/2.0), layout_height/start_height, (gint)(layout_height/2.0));
      }
    if(rotate==3)
      {
        fprintf(f, "<script>\n"
                   "var scale_drawing = document.getElementById(\"scale_drawing\");\n"
                   "var j = 0;\n"
                   "function redraw() {\n"
                   "var angle = j * Math.PI / 256.0;\n"
                   "angle = angle * 180.0 / Math.PI;\n"
                   "j += 1.0;\n"
                   "scale_drawing.setAttribute(\"transform\", \"scale(%f, %f) translate(%i, %i) rotate(\" + angle + \")\");\n"
                   "}\n"
                   "setInterval(\"redraw()\", 16.7);\n"
                   "</script>\n", layout_width/start_width, layout_height/start_height, (gint)(layout_width/2.0), (gint)(layout_height/2.0));
      }
    if(rotate==4)
      {
        fprintf(f, "<script>\n"
                   "var scale_drawing = document.getElementById(\"scale_drawing\");\n"
                   "var j = 0;\n"
                   "function redraw() {\n"
                   "var angle = j * Math.PI / 256.0;\n"
                   "var scale = Math.cos(angle) * %f;\n"
                   "var scale_inv = 1.0 / scale * %i;\n"
                   "var scale2 = Math.sin(angle) * %f;\n"
                   "var scale_inv2 = 1.0 / scale2 * %i;\n"
                   "angle = angle * 180.0 / Math.PI;\n"
                   "j += 1.0;\n"
                   "scale_drawing.setAttribute(\"transform\", \"scale(\" + scale + \" ,\" + scale2 + \") translate(\" + scale_inv + \" ,\" + scale_inv2 + \") rotate(\" + angle + \")\");\n"
                   "}\n"
                   "setInterval(\"redraw()\", 16.7);\n"
                   "</script>\n", layout_width/start_width, (gint)(layout_width/2.0), layout_height/start_height, (gint)(layout_width/2.0));
       }
  }
static void svg_dialog()
  {
    GtkWidget *dialog=gtk_dialog_new_with_buttons("bezier_drawing1.svg", GTK_WINDOW(window), GTK_DIALOG_MODAL, "OK", GTK_RESPONSE_OK, NULL);
    gtk_window_set_default_size(GTK_WINDOW(dialog), 400, 400);
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
static void add_color_stop(GtkWidget *widget, GtkWidget **widgets2)
  {
    GtkEntryBuffer *buffer=gtk_entry_get_buffer(GTK_ENTRY(widgets2[0]));
    const gchar *text=gtk_entry_buffer_get_text(buffer);
    gsize len=gtk_entry_buffer_get_bytes(buffer);
    gchar *string=g_strndup(text, len);
    gchar *p=string;
    GString *temp=g_string_new(NULL);
    gint counter=0;
    gdouble value=0; 
    gdouble array[5];
    struct color_stop st;

    while(*p!='\0')
      {
        if(g_ascii_isdigit(*p)||*p=='.')
          {
            g_string_append_c(temp, *p);
          }
        else if(!g_ascii_isdigit(*p)&&temp->len>0)
          {
            value=g_ascii_strtod(temp->str, NULL);
            if(value>=0.0&&value<=1.0)
              {
                array[counter]=value;
                counter++;
                if(counter>4) break;
              }
            g_string_truncate(temp, 0);
          }
        p++;
      }

    if(counter!=5)
      {
        gchar *msg=g_strdup("Need five numbers(0.0<=x<=1.0) for the color stop.");
        message_dialog(msg);
        g_free(msg);
      }
    else
      {
        st.p=array[0]; st.r=array[1]; st.g=array[2]; st.b=array[3], st.a=array[4];
        g_array_append_val(color_stops, st);
        GtkTreeStore *store=get_tree_store_fill();
        gtk_tree_view_set_model(GTK_TREE_VIEW(widgets2[1]), GTK_TREE_MODEL(store));
        g_object_unref(G_OBJECT(store));  
        gtk_widget_queue_draw(widgets2[2]);
      }

    g_free(string);
    g_string_free(temp, TRUE);
  }
static void delete_color_stop(GtkWidget *widget, GtkWidget **widgets2)
  {
    GtkTreeIter iter;
    GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets2[1]));
    GtkTreeModel *model=gtk_tree_view_get_model(GTK_TREE_VIEW(widgets2[1]));
    if(gtk_tree_selection_get_selected(selection, NULL, &iter))
      {
        GtkTreePath *path=gtk_tree_model_get_path(model, &iter);
        gchar *string=gtk_tree_path_to_string(path);
        gint row_id=atoi(string);
        if(row_id<=(color_stops->len-1)&&(color_stops->len)>0)
          {
            gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
            g_array_remove_index(color_stops, row_id);
          }
        g_free(string);
      }

    gtk_tree_selection_unselect_all(selection);
    gtk_widget_queue_draw(widgets2[2]);
  }
static void update_linear_direction(GtkWidget *widget, GtkWidget **widgets2)
  {
    GtkEntryBuffer *buffer=gtk_entry_get_buffer(GTK_ENTRY(widgets2[3]));
    const gchar *text=gtk_entry_buffer_get_text(buffer);
    gsize len=gtk_entry_buffer_get_bytes(buffer);
    gchar *string=g_strndup(text, len);
    gchar *p=string;
    GString *temp=g_string_new(NULL);
    gint counter=0;
    gdouble value=0; 
    gdouble array[4];

    while(*p!='\0')
      {
        if(g_ascii_isdigit(*p)||*p=='.')
          {
            g_string_append_c(temp, *p);
          }
        else if(!g_ascii_isdigit(*p)&&temp->len>0)
          {
            value=g_ascii_strtod(temp->str, NULL);
            if(value>=0.0&&value<=100.0)
              {
                array[counter]=value;
                counter++;
                if(counter>3) break;
              }
            g_string_truncate(temp, 0);
          }
        p++;
      }

    if(counter!=4)
      {
        gchar *msg=g_strdup("Need four numbers(0.0<=x<=100.0) for the linear direction.");
        message_dialog(msg);
        g_free(msg);
      }
    else
      {
        ld[0]=array[0]; ld[1]=array[1]; ld[2]=array[2]; ld[3]=array[3];
        gtk_widget_queue_draw(widgets2[2]);
      }

    g_free(string);
    g_string_free(temp, TRUE);
  }
static void add_points(GtkWidget *widget, GtkWidget **widgets)
{
  gint i=0;
  gint len=coords1->len;
  GArray *c1=g_array_sized_new(FALSE, FALSE, sizeof(struct point), len);

  //Save the current coordinates.
  for(i=0;i<len;i++)
    {
      g_array_append_val(c1, g_array_index(coords1, struct point, i));
    }
  g_ptr_array_add(paths, (GArray*)c1);
  g_array_append_val(path_info, interpolation);
  g_array_append_val(path_info, fill);
  g_array_append_val(line_colors, lca[0]);
  g_array_append_val(line_colors, lca[1]);
  g_array_append_val(line_colors, lca[2]);
  g_array_append_val(line_colors, lca[3]);
  g_array_append_val(line_widths, line_width);
  g_array_append_val(line_widths, line_cap);

  //Save the color stops and direction for the gradient.
  if(fill==1)
    {
      len=color_stops->len;
      GArray *c2=g_array_sized_new(FALSE, FALSE, sizeof(struct color_stop), len);
      for(i=0;i<len;i++)
        {
          g_array_append_val(c2, g_array_index(color_stops, struct color_stop, i));
        }
      g_ptr_array_add(gradients, (GArray*)c2);      
      g_array_append_val(direction, ld[0]);
      g_array_append_val(direction, ld[1]);
      g_array_append_val(direction, ld[2]);
      g_array_append_val(direction, ld[3]);       
    }

  /*
    Reset coords to draw again. Reset based on number of current coordinates. This can
    look a little strange in the UI.
  */
  len=coords1->len; 
  gdouble w1=start_width*0.4;
  if(start_width>start_height) w1=start_height*0.4;
  struct point p1;
  g_array_remove_range(coords1, 0, len);

  for(i=0;i<len;i++)
    {
      p1.x=w1*cos((gdouble)i*G_PI/((gdouble)len/2.0));
      p1.y=w1*sin((gdouble)i*G_PI/((gdouble)len/2.0));
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
  gint i=0;
  GtkTreeIter iter;
  GtkTreeSelection *selection=gtk_tree_view_get_selection(GTK_TREE_VIEW(widgets[2]));
  GtkTreeModel *model=gtk_tree_view_get_model(GTK_TREE_VIEW(widgets[2]));
  if(gtk_tree_selection_get_selected(selection, NULL, &iter))
    {
      GtkTreePath *path=gtk_tree_model_get_path(model, &iter);
      gint count=0;
      if(gtk_tree_path_get_depth(path)==1)
        {
          gchar *string=gtk_tree_path_to_string(path);
          gint row_id=atoi(string);
          if(row_id<=(paths->len-1)&&(paths->len)>0)
            {
              gtk_tree_store_remove(GTK_TREE_STORE(model), &iter);
              //Need to delete direction and gradient entries based on the number of fills up to the row_id.
              if(g_array_index(path_info, gint, 2*row_id+1)==1)
                {
                  for(i=1;i<path_info->len;i+=2)
                    {
                      if(g_array_index(path_info, gint, i)==1) count++;
                    }
                  count--;
                  g_array_remove_index(direction, 4*count);
                  g_array_remove_index(direction, 4*count);
                  g_array_remove_index(direction, 4*count);
                  g_array_remove_index(direction, 4*count);
                  g_array_free((GArray*)(g_ptr_array_index(gradients, count)), TRUE);
                  g_ptr_array_remove_index(gradients, count);
                }
              g_array_free((GArray*)(g_ptr_array_index(paths, row_id)), TRUE);
              g_ptr_array_remove_index(paths, row_id);
              g_array_remove_index(path_info, 2*row_id);
              g_array_remove_index(path_info, 2*row_id);
              g_array_remove_index(line_colors, 4*row_id);
              g_array_remove_index(line_colors, 4*row_id);
              g_array_remove_index(line_colors, 4*row_id);
              g_array_remove_index(line_colors, 4*row_id);
              g_array_remove_index(line_widths, 2*row_id);
              g_array_remove_index(line_widths, 2*row_id);               
            }
          g_free(string);
        }
      else
        {
          gchar *msg=g_strdup("Can only delete top node shape.");
          message_dialog(msg);
          g_free(msg);
        }
    }

  gtk_tree_selection_unselect_all(selection);
  gtk_widget_queue_draw(widgets[0]);
}
static void clear_shapes(GtkWidget *widget, GtkWidget **widgets)
{
  gint i=0;
  gint len=paths->len;
 
  for(i=0;i<len;i++)
    {
      g_array_free((GArray*)(g_ptr_array_index(paths, i)), TRUE);
    }
  g_ptr_array_remove_range(paths, 0, len);

  len=path_info->len;
  g_array_remove_range(path_info, 0, len);

  len=line_colors->len;
  g_array_remove_range(line_colors, 0, len);

  len=line_widths->len;
  g_array_remove_range(line_widths, 0, len);

  len=direction->len;
  g_array_remove_range(direction, 0, len);

  len=gradients->len;
  for(i=0;i<len;i++)
    {
      g_array_free((GArray*)(g_ptr_array_index(gradients, i)), TRUE);
    }
  g_ptr_array_remove_range(gradients, 0, len);

  GtkTreeStore *store=get_tree_store();
  gtk_tree_view_set_model(GTK_TREE_VIEW(widgets[2]), GTK_TREE_MODEL(store));
  g_object_unref(G_OBJECT(store));

  gtk_widget_queue_draw(GTK_WIDGET(widgets[0]));
}
static void get_saved_svg(GtkWidget *widget, GtkWidget **widgets5)
  {
    //Test getting the previously saved shape.
    GFile *text_file=g_file_new_for_path(gtk_entry_get_text(GTK_ENTRY(widgets5[3])));
    GFileInputStream *file_stream=NULL;
    GFileInfo *file_info=NULL;
    gint file_size=-1;
    gchar *text_buffer=NULL;
    GError *error=NULL;

    file_stream=g_file_read(text_file, NULL, &error);
    if(error==NULL)
      {
        file_info=g_file_input_stream_query_info(G_FILE_INPUT_STREAM(file_stream), G_FILE_ATTRIBUTE_STANDARD_SIZE, NULL, NULL);     
        file_size=g_file_info_get_size(file_info);
        g_object_unref(file_info);
        text_buffer=(char *) malloc(sizeof(gchar) * file_size+1);
        memset(text_buffer, '\0', file_size+1);
        g_input_stream_read(G_INPUT_STREAM(file_stream), text_buffer, file_size, NULL, NULL);
       
        //Parse
        gint array_type=0;
        gchar *p1=text_buffer;
        gboolean xml_comment=FALSE;
        
        //Look for start of comment tag. Could check more of the comment tag "bezier...".
        while(*p1!='\0')
          {
            if(*p1=='-'&&*(p1-1)=='!'&&*(p1-2)=='<'&&*(p1+3)=='b'&&*(p1+5)=='z')
              {
                xml_comment=TRUE;
                break;
              }
            p1++;
          }

        if(xml_comment) 
          {
            gint len=0;
            gboolean found_tag=TRUE;
            GArray *array_temp=g_array_new(FALSE, FALSE, sizeof(gdouble));
            while(found_tag)
              {
                p1=get_array_type_svg(p1, &found_tag, &array_type);
                if(found_tag)
                  {
                    p1=get_array_start_svg(p1);
                    //parse the svg text.
                    p1=parse_array_svg(p1, array_temp);
                    //build the program array.
                    build_array_svg(array_temp, array_type);
                    //Clear temp array.
                    len=array_temp->len;
                    g_array_remove_range(array_temp, 0, len);
                  }
 
                 if(p1=='\0') break;
              }
            g_array_free(array_temp, TRUE);
          }
        else
          {
            gchar *msg=g_strdup("Couldn't find program data in svg file.");
            message_dialog(msg);
            g_free(msg);
          }
                      
        g_object_unref(file_stream);
        g_free(text_buffer);

        //Update the treeview.
        GtkTreeStore *store=get_tree_store();
        gtk_tree_view_set_model(GTK_TREE_VIEW(widgets5[2]), GTK_TREE_MODEL(store));
        g_object_unref(G_OBJECT(store));
        gtk_widget_queue_draw(GTK_WIDGET(widgets5[0]));
      }
    else
      {
        message_dialog(error->message);
        g_error_free(error);
      }
   
    g_object_unref(text_file);
  }
static gchar* get_array_type_svg(gchar *p1, gboolean *found, gint *array_type)
  {
    while(*p1!='\0')
      {
        if(*p1=='\n'&&(*(p1+1)=='S'||*(p1+1)=='T'))
          {
            *array_type=0;
            *found=TRUE;
            break;
          }
        if(*p1=='\n'&&*(p1+1)=='I')
          {
            *array_type=1;
            *found=TRUE;
            break;
          }
        if(*p1=='\n'&&*(p1+1)=='D')
          {
            *array_type=2;
            *found=TRUE;
            break;
          }
        if(*p1=='\n'&&*(p1+1)=='G')
          {
            *array_type=3;
            *found=TRUE;
            break;
          }
        if(*p1=='\n'&&*(p1+1)=='L')
          {
            *array_type=4;
            *found=TRUE;
            break;
          }
        if(*p1=='\n'&&*(p1+1)=='W')
          {
            *array_type=5;
            *found=TRUE;
            break;
          }
        if(*p1=='>'&&*(p1-1)=='-')
          {
            *found=FALSE;
            break;
          }
        p1++;
      }
    return p1;
  }
static gchar* get_array_start_svg(gchar *p1)
  {
    while(*p1!='\0')
      {
        if(*p1=='[')
          {
            break;
          }
        p1++;
      }
     
    return p1;             
  }
static gchar* parse_array_svg(gchar *p1, GArray *array_temp)
  {
    GString *string=g_string_new(NULL);
    gdouble temp=0;

    while(*p1!='\0')
      {
        if(g_ascii_isdigit(*p1)||*p1=='-'||*p1=='.')
          {
            g_string_append_c(string, *p1);
          }
        else if(*p1==' '||*p1==']')
          {
            if(string->len>0)
              {                
                temp=g_ascii_strtod(string->str, NULL);
                g_array_append_val(array_temp, temp);
                g_string_truncate(string, 0);
              }
          }
        if(*p1==']') break;
        p1++;
      }

     g_string_free(string, TRUE);
     return p1;
  }
static void build_array_svg(GArray *array_temp, gint array_type)
  {
    gint i=0;
    gint len=array_temp->len;
    if(array_type==0)
      {
        struct point pt;
        GArray *array=g_array_new(FALSE, FALSE, sizeof(struct point));
        for(i=0;i<len;i+=2)
          {
            pt.x=g_array_index(array_temp, gdouble, i);
            pt.y=g_array_index(array_temp, gdouble, i+1);
            g_array_append_val(array, pt);
          }
        g_ptr_array_add(paths, (GArray*)array);
      }
    else if(array_type==1)
      {
        for(i=0;i<len;i++)
          {
            gint value=(gint)g_array_index(array_temp, gdouble, i);
            g_array_append_val(path_info, value);
          }
      }
    else if(array_type==2)
      {
        for(i=0;i<len;i++)
          {
            gdouble dr=g_array_index(array_temp, gdouble, i);
            g_array_append_val(direction, dr);
          }
      }
    else if(array_type==3)
      {
        struct color_stop cs;
        GArray *array=g_array_new(FALSE, FALSE, sizeof(struct color_stop));
        for(i=0;i<len;i+=5)
          {
            cs.p=g_array_index(array_temp, gdouble, i);
            cs.r=g_array_index(array_temp, gdouble, i+1);
            cs.g=g_array_index(array_temp, gdouble, i+2);
            cs.b=g_array_index(array_temp, gdouble, i+3);
            cs.a=g_array_index(array_temp, gdouble, i+4);
            g_array_append_val(array, cs);
          }
        g_ptr_array_add(gradients, (GArray*)array);
      } 
    else if(array_type==4)
      {
        for(i=0;i<len;i++)
          {
            gdouble value=g_array_index(array_temp, gdouble, i);
            g_array_append_val(line_colors, value);
          }
      } 
    else
      {
        for(i=0;i<len;i++)
          {
            gint value=(gint)g_array_index(array_temp, gdouble, i);
            g_array_append_val(line_widths, value);
          }
      }            
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
static GtkTreeStore* get_tree_store_fill()
  {
    gint i=0;
    GtkTreeStore *store=gtk_tree_store_new(1, G_TYPE_STRING);
    if((color_stops->len)>0)
      {
        GtkTreeIter iter1;
        struct color_stop st;
        gtk_tree_store_append(store, &iter1, NULL);
        for(i=0;i<color_stops->len;i++)
          {
            st=g_array_index(color_stops, struct color_stop, i);
            gchar *string1=g_strdup_printf("%i. p:%.2f. r: %.2f, g: %.2f, b: %.2f, a: %.2f", i+1, st.p, st.r, st.g, st.b, st.a);
            gtk_tree_store_set(store, &iter1, 0, string1, -1);
            gtk_tree_store_append(store, &iter1, NULL);
            g_free(string1);
          }
       }

    return store;
  }
static void update_line_color(GtkWidget *widget, GtkWidget **line_widgets)
  {
    GdkRGBA rgba;
    
    if(gdk_rgba_parse(&rgba, gtk_entry_get_text(GTK_ENTRY(line_widgets[0]))))
      {
        lca[0]=rgba.red;
        lca[1]=rgba.green;
        lca[2]=rgba.blue;
        lca[3]=rgba.alpha;
      }
    else
      {
        gchar *msg=g_strdup("Color string format error in line color.");
        message_dialog(msg);
        g_free(msg);
      } 
   
    //Update main window.
    gtk_widget_queue_draw(line_widgets[1]);
    //Update the drawing area.
    gtk_widget_queue_draw(line_widgets[2]);
  }
static void set_line_width(GtkComboBox *combo, gpointer data)
  {
    line_width=gtk_combo_box_get_active(combo)+1;
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void set_line_cap(GtkComboBox *combo, gpointer data)
  {
    line_cap=gtk_combo_box_get_active(combo);
    gtk_widget_queue_draw(GTK_WIDGET(data));
  }
static void set_layout(GtkWidget *widget, GtkWidget **widgets3)
  {
    gdouble width=strtod(gtk_entry_get_text(GTK_ENTRY(widgets3[0])), NULL);
    gdouble height=strtod(gtk_entry_get_text(GTK_ENTRY(widgets3[1])), NULL);

    if(width>=20&&height>=20&&width<=600&&height<=600)
      {
        motion_x=motion_x*width/layout_width;
        motion_y=motion_y*height/layout_height;
        layout_width=width;
        layout_height=height;
        gtk_widget_queue_draw(GTK_WIDGET(widgets3[2]));
      }
    else
      {
        gchar *msg=g_strdup("The layout width and height range 30<=x<=500.");
        message_dialog(msg);
        g_free(msg);
      }
  }
static void about_dialog(GtkWidget *widget, gpointer data)
  {
    GtkWidget *dialog=gtk_about_dialog_new();
    gtk_window_set_transient_for(GTK_WINDOW(dialog), GTK_WINDOW(data));
    //gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(dialog), NULL);
    gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(dialog), "Bezier Points1");
    gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(dialog), "Test Version 1.0");
    gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(dialog), "A non linear approach to drawing.");
    gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(dialog), "(C) 2017 C. Eric Cashon");

    gtk_widget_show_all(dialog);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static GdkPixbuf* draw_icon()
  {  
    gint i=0;
    gint len=0;
    struct point p1;
    struct controls c1;
    GArray *cp1=NULL;
    GArray *cp2=NULL;
    GArray *cp3=NULL;
    GArray *cp4=NULL;
    GArray *head=g_array_new(FALSE, FALSE, sizeof(struct point));
    GArray *eye1=g_array_new(FALSE, FALSE, sizeof(struct point));
    GArray *eye2=g_array_new(FALSE, FALSE, sizeof(struct point));
    GArray *smile=g_array_new(FALSE, FALSE, sizeof(struct point));
    gdouble shape1[]={197.600000, 0.000000, 171.855286, 165.421722, 92.251190, 80.340851, -3.041626, 157.230347, -91.292328, 80.802856, -173.921051, 166.943161, -197.600000, 0.000000, -171.126620, -98.800000, -98.800000, -171.126620, -0.000000, -197.600000, 98.800000, -171.126620, 171.126620, -98.800000, 197.600000, 0.000000};
    gdouble shape2[]={-80.005219, -116.974167, -60.891556, -99.481995, -47.832367, -53.821945, -90.579529, -53.343048, -119.620056, -73.651749, -106.794327, -112.442886, -80.005219, -116.974167};
    gdouble shape3[]={118.749344, -89.965118, 123.728241, -64.674652, 96.609802, -49.756699, 54.696991, -48.287888, 67.510803, -92.461472, 101.569748, -109.263504, 118.749344, -89.965118};
    gdouble shape4[]={125.234634, 6.535904, 70.436905, 64.270874, -4.464111, 81.433762, -78.761032, 52.571838, -128.618881, 2.941559};

    //Create a surface to draw a 256x256 icon. 
    cairo_surface_t *surface_icon=cairo_image_surface_create (CAIRO_FORMAT_ARGB32, 256, 256);
    cairo_t *cr=cairo_create(surface_icon);

    //Paint the background.
    cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);
    cairo_paint(cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_translate(cr, 256.0/2.0, 256.0/2.0);
    cairo_scale(cr, 256.0/500.0, 256.0/500.0);
    cairo_set_line_width(cr, 6.0);
    
    //Load arrays.
    for(i=0;i<26;i+=2)
      {
        p1.x=shape1[i];
        p1.y=shape1[i+1];
        g_array_append_val(head, p1);
      }
    for(i=0;i<14;i+=2)
      {
        p1.x=shape2[i];
        p1.y=shape2[i+1];
        g_array_append_val(eye1, p1);
      }
    for(i=0;i<14;i+=2)
      {
        p1.x=shape3[i];
        p1.y=shape3[i+1];
        g_array_append_val(eye2, p1);
      }

    for(i=0;i<10;i+=2)
      {
        p1.x=shape4[i];
        p1.y=shape4[i+1];
        g_array_append_val(smile, p1);
      }

    //Get the Bezier control points.
    cp1=control_points_from_coords2(head);
    cp2=control_points_from_coords2(eye1);
    cp3=control_points_from_coords2(eye2);
    cp4=control_points_from_coords2(smile);

    //Draw arrays and patterns.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0, -128, 0, 128);
    cairo_pattern_add_color_stop_rgba(pattern1, 0.0, 1.0, 0.0, 1.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 0.5, 1.0, 1.0, 0.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern1, 1.0, 0.0, 1.0, 1.0, 1.0); 
    len=head->len;
    p1=g_array_index(head, struct point, 0);
    cairo_move_to(cr, p1.x, p1.y);
    for(i=1;i<len;i++)
      {
        p1=g_array_index(head, struct point, i);
        c1=g_array_index(cp1, struct controls, i-1);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, p1.x, p1.y);
        cairo_stroke_preserve(cr); 
      }
    cairo_set_source(cr, pattern1);  
    cairo_fill(cr);
    cairo_pattern_destroy(pattern1);
    cairo_new_path(cr);

    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0, -128, 0, 128);
    cairo_pattern_add_color_stop_rgba(pattern2, 0.08, 0.0, 0.0, 1.0, 1.0); 
    cairo_pattern_add_color_stop_rgba(pattern2, 0.3, 0.0, 1.0, 1.0, 1.0);  
    len=eye1->len;
    p1=g_array_index(eye1, struct point, 0);
    cairo_move_to(cr, p1.x, p1.y);
    for(i=1;i<len;i++)
      {
        p1=g_array_index(eye1, struct point, i);
        c1=g_array_index(cp2, struct controls, i-1);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, p1.x, p1.y);
        cairo_stroke_preserve(cr); 
      }
    cairo_set_source(cr, pattern2);  
    cairo_fill(cr);
    cairo_new_path(cr);

    len=eye2->len;
    p1=g_array_index(eye2, struct point, 0);
    cairo_move_to(cr, p1.x, p1.y);
    for(i=1;i<len;i++)
      {
        p1=g_array_index(eye2, struct point, i);
        c1=g_array_index(cp3, struct controls, i-1);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, p1.x, p1.y);
        cairo_stroke_preserve(cr); 
      }
    cairo_fill(cr);
    cairo_pattern_destroy(pattern2);
    cairo_new_path(cr);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 7.0);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    len=smile->len;
    p1=g_array_index(smile, struct point, 0);
    cairo_move_to(cr, p1.x, p1.y);
    for(i=1;i<len;i++)
      {
        p1=g_array_index(smile, struct point, i);
        c1=g_array_index(cp4, struct controls, i-1);
        cairo_curve_to(cr, c1.x1, c1.y1, c1.x2, c1.y2, p1.x, p1.y);
        cairo_stroke_preserve(cr); 
      }
    cairo_new_path(cr);
       
    GdkPixbuf *icon=gdk_pixbuf_get_from_surface(surface_icon, 0, 0, 256, 256);

    g_array_free(head, TRUE);
    g_array_free(eye1, TRUE);
    g_array_free(eye2, TRUE);
    g_array_free(smile, TRUE);
    g_array_free(cp1, TRUE);
    g_array_free(cp2, TRUE);
    g_array_free(cp3, TRUE);
    g_array_free(cp4, TRUE);
    cairo_destroy(cr);
    cairo_surface_destroy(surface_icon); 
    return icon;
  }
static void message_dialog(gchar *msg)
  {
    GtkWidget *dialog=gtk_message_dialog_new(GTK_WINDOW(window), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, "%s", msg);
    gtk_dialog_run(GTK_DIALOG(dialog));
    gtk_widget_destroy(dialog);
  }
static void cleanup(GtkWidget *widget, gpointer data)
  {
    gint i=0;
    g_array_free(array_id, TRUE);
    g_array_free(point_id, TRUE);
    g_array_free(coords1, TRUE);
    gint len=paths->len;
    for(i=0;i<len;i++)
      {
        g_array_free((GArray*)(g_ptr_array_index(paths, i)), TRUE);
      }
    g_ptr_array_free(paths, TRUE);
    g_array_free(path_info, TRUE);

    len=gradients->len;
    for(i=0;i<len;i++)
      {
        g_array_free((GArray*)(g_ptr_array_index(gradients, i)), TRUE);
      }
    g_ptr_array_free(gradients, TRUE);
    g_array_free(color_stops, TRUE);
    g_array_free(direction, TRUE);
    g_array_free(line_colors, TRUE);
    g_array_free(line_widths, TRUE);

    gtk_main_quit();
}






