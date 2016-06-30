
/*

    Draw progress bars with a cairo linear pattern on a GTK drawing area. Draw custom horizontal
and vertical progress bars along with the standard GTK progress bar. Eventually build a LED type
of progress bar widget.

    gcc -Wall -Werror da_progress1.c -o da_progress1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu14.04 and GTK3.10.

    By C. Eric Cashon

*/

#include <gtk/gtk.h>

//The progress of the bars. Test a count of 10 in the program. Increment in the timer function.
static gint progress_step=0;
//The number of steps to draw at start. Can be changed with the combo box.
#define start_steps 20
static gint steps=start_steps;
//For the pattern size.
static gint total_steps=20*start_steps;
//gradient_end_end value < 20. The end color for the gradient.
static gdouble gradient_end=19.0;

static gboolean time_draw(GtkWidget *widgets[]);
static void start_drawing(GtkWidget *button, GtkWidget *widgets[]);
static gboolean draw_custom_progress_horizontal(GtkWidget *da, cairo_t *cr, gpointer data);
static gboolean draw_custom_progress_vertical(GtkWidget *da, cairo_t *cr, gpointer data);
static void combo_changed(GtkComboBox *combo, GtkWidget *widgets[]);

int main(int argc, char **argv)
  {      
    gtk_init(&argc, &argv);   

    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 500);
    gtk_window_set_title(GTK_WINDOW(window), "Progress Bars");
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    
    //Custom horizontal progress bar.    
    GtkWidget *da1=gtk_drawing_area_new();
    gtk_widget_set_size_request(da1, 300, 30);
    gtk_widget_set_hexpand(da1, TRUE);
    g_signal_connect(da1, "draw", G_CALLBACK(draw_custom_progress_horizontal), NULL);

    //Custom vertical progress bars.   
    GtkWidget *da2=gtk_drawing_area_new();
    gtk_widget_set_name(da2, "da2");
    gtk_widget_set_size_request(da2, 30, 300);
    gtk_widget_set_vexpand(da2, TRUE);
    gtk_widget_set_hexpand(da2, FALSE);
    g_signal_connect(da2, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da3=gtk_drawing_area_new();
    gtk_widget_set_name(da3, "da3");
    gtk_widget_set_size_request(da3, 30, 300);
    gtk_widget_set_vexpand(da3, TRUE);
    gtk_widget_set_hexpand(da3, FALSE);
    g_signal_connect(da3, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da4=gtk_drawing_area_new();
    gtk_widget_set_name(da4, "da4");
    gtk_widget_set_size_request(da4, 30, 300);
    gtk_widget_set_vexpand(da4, TRUE);
    gtk_widget_set_hexpand(da4, FALSE);
    g_signal_connect(da4, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da5=gtk_drawing_area_new();
    gtk_widget_set_name(da5, "da5");
    gtk_widget_set_size_request(da5, 30, 300);
    gtk_widget_set_vexpand(da5, TRUE);
    gtk_widget_set_hexpand(da5, FALSE);
    g_signal_connect(da5, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    GtkWidget *da6=gtk_drawing_area_new();
    gtk_widget_set_name(da6, "da6");
    gtk_widget_set_size_request(da6, 30, 300);
    gtk_widget_set_vexpand(da6, TRUE);
    gtk_widget_set_hexpand(da6, FALSE);
    g_signal_connect(da6, "draw", G_CALLBACK(draw_custom_progress_vertical), NULL);

    //Standard progress bar.
    GtkWidget *progress=gtk_progress_bar_new();

    GtkWidget *combo=gtk_combo_box_text_new();
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 0, "1", "10 steps");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 1, "2", "20 steps");
    gtk_combo_box_text_insert(GTK_COMBO_BOX_TEXT(combo), 2, "3", "30 steps");
    gtk_widget_set_hexpand(combo, TRUE);  
    gtk_combo_box_set_active(GTK_COMBO_BOX(combo), 1);
   
    GtkWidget *button=gtk_button_new_with_label("Test Progress Bars");
    gtk_widget_set_hexpand(button, TRUE);
    GtkWidget *widgets[]={button, progress, da1, da2, da3, da4, da5, da6, combo};
    g_signal_connect(button, "clicked", G_CALLBACK(start_drawing), widgets);

    g_signal_connect(combo, "changed", G_CALLBACK(combo_changed), widgets);
      
    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), da1, 0, 0, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), da2, 0, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da3, 2, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da4, 4, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da5, 6, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), da6, 8, 1, 1, 10);
    gtk_grid_attach(GTK_GRID(grid), progress, 0, 11, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), combo, 0, 12, 10, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 13, 10, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    //Set up CSS colors for the background window and the button.
    gchar *css_string=NULL;
    gint minor_version=gtk_get_minor_version();
    g_print("Minor Version %i\n", minor_version);    
    if(minor_version>=18) css_string=g_strdup("button, combobox{background: #0088FF; font: Arial 16; color: cyan} window{background: #000000;}");
    else css_string=g_strdup("GtkButton, GtkComboBox{background: #0088FF; font: Arial 16; color: cyan} GtkWindow{background: #000000;}");
    GError *css_error=NULL;
    GtkCssProvider *provider = gtk_css_provider_new();
    GdkDisplay *display = gdk_display_get_default();
    GdkScreen *screen = gdk_display_get_default_screen(display);
    gtk_style_context_add_provider_for_screen(screen, GTK_STYLE_PROVIDER(provider), GTK_STYLE_PROVIDER_PRIORITY_APPLICATION);
    gtk_css_provider_load_from_data(provider, css_string, -1, &css_error);
    if(css_error!=NULL)
      {
        g_print("CSS error %s\n", css_error->message);
        g_error_free(css_error);
      }
    g_object_unref(provider);
    if(css_string!=NULL) g_free(css_string);
    
    gtk_widget_show_all(window);                  
    gtk_main();

    return 0;
  }
static gboolean time_draw(GtkWidget *widgets[])
  {
    g_print("Timer %i\n", progress_step);
    if(progress_step<10)
      {
        progress_step++;
        gtk_widget_queue_draw(widgets[2]);
        gtk_widget_queue_draw(widgets[3]);
        gtk_widget_queue_draw(widgets[4]);
        gtk_widget_queue_draw(widgets[5]);
        gtk_widget_queue_draw(widgets[6]);
        gtk_widget_queue_draw(widgets[7]);
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets[1]), (gdouble)progress_step/10.0);
        return TRUE;
      }
    else  
      {
        gtk_widget_set_sensitive(widgets[0], TRUE);
        gtk_widget_set_sensitive(widgets[8], TRUE);
        return FALSE;
      }   
  }
static void start_drawing(GtkWidget *button, GtkWidget *widgets[])
  {
    g_print("Click\n");
    progress_step=0;
    gtk_widget_set_sensitive(button, FALSE);
    gtk_widget_set_sensitive(widgets[8], FALSE);
    g_timeout_add(300, (GSourceFunc)time_draw, widgets);
  }
static gboolean draw_custom_progress_horizontal(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da);
    gint height=gtk_widget_get_allocated_height(da);
    gint i=0;
    
    //The background gradient.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
    for(i=0;i<=total_steps;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i/(gdouble)total_steps), 0.0, 1.0, 1.0); 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i+gradient_end)/(gdouble)total_steps, 0.0, 0.0, 1.0); 
      }
    cairo_set_source(cr, pattern1);
     
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    //The foreground gradient.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, width, 0.0);
    for(i=0;i<=total_steps;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i/(gdouble)total_steps), 1.0, 1.0, 0.0); 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i+gradient_end)/(gdouble)total_steps, 1.0, 0.0, 0.0); 
      }
    cairo_set_source(cr, pattern2);
     
    cairo_rectangle(cr, 0, 0, ((gdouble)progress_step/(gdouble)steps)*(gdouble)width, height);
    cairo_fill(cr);

    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_stroke(cr);

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);
    return FALSE;
  }
static gboolean draw_custom_progress_vertical(GtkWidget *da, cairo_t *cr, gpointer data)
  {
    gint width=gtk_widget_get_allocated_width(da)*10;
    gint height=gtk_widget_get_allocated_height(da);
    gint i=0;
    
    //Test some offset counters and colors. Eventually move this code to calling functions.
    gint step_stop=0;
    gdouble background_rgb1[]={0.0, 1.0, 1.0};
    gdouble background_rgb2[]={0.0, 0.0, 1.0};
    gdouble forground_rgb1[]={1.0, 1.0, 0.0};
    gdouble forground_rgb2[]={1.0, 0.0, 0.0};
    if(g_strcmp0(gtk_widget_get_name(da), "da2")==0)
      {
        background_rgb2[1]=1.0;
        background_rgb2[1]=0.6;
      }
    if(g_strcmp0(gtk_widget_get_name(da), "da3")==0)
      {
        forground_rgb1[0]=0.6;
        forground_rgb1[1]=0.0;
      }
    if(g_strcmp0(gtk_widget_get_name(da), "da5")==0)
      {
        forground_rgb1[0]=0.0;
        forground_rgb2[0]=0.0;
        forground_rgb2[1]=0.6;
      }   
    if(g_strcmp0(gtk_widget_get_name(da), "da2")==0&&progress_step>1) step_stop=2;
    else if(g_strcmp0(gtk_widget_get_name(da), "da3")==0&&progress_step>3) step_stop=4;
    else if(g_strcmp0(gtk_widget_get_name(da), "da4")==0&&progress_step>5) step_stop=6;
    else if(g_strcmp0(gtk_widget_get_name(da), "da5")==0&&progress_step>7) step_stop=8;
    else step_stop=progress_step;
    //

    //Transforms for drawing.
    cairo_save(cr);
    cairo_rotate(cr, G_PI/2.0);
    cairo_translate(cr, 0.0, -height);

    //The background pattern.
    cairo_pattern_t *pattern1=cairo_pattern_create_linear(0.0, 0.0, height, 0.0);
    for(i=0;i<=total_steps;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i/(gdouble)total_steps), background_rgb1[0], background_rgb1[1], background_rgb1[2]); 
        cairo_pattern_add_color_stop_rgb(pattern1, (gdouble)(i+gradient_end)/(gdouble)total_steps, background_rgb2[0], background_rgb2[1], background_rgb2[2]); 
      }
    cairo_set_source(cr, pattern1);
     
    cairo_rectangle(cr, 0, 0, width, height);
    cairo_fill(cr);

    //The foreground pattern.
    cairo_pattern_t *pattern2=cairo_pattern_create_linear(0.0, 0.0, height, 0.0);
    for(i=0;i<=total_steps;i+=20)
      { 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i/(gdouble)total_steps), forground_rgb1[0], forground_rgb1[1], forground_rgb1[2]); 
        cairo_pattern_add_color_stop_rgb(pattern2, (gdouble)(i+gradient_end)/(gdouble)total_steps, forground_rgb2[0], forground_rgb2[1], forground_rgb2[2]); 
      }
    cairo_set_source(cr, pattern2);
     
    cairo_rectangle(cr, (1.0-(step_stop/(gdouble)steps))*height, 0, (step_stop/(gdouble)steps)*height, height);
    cairo_fill(cr);

    cairo_restore(cr);
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 6);
    cairo_rectangle(cr, 0, 0, width/10, height);
    cairo_stroke(cr);

    cairo_pattern_destroy(pattern1);
    cairo_pattern_destroy(pattern2);
    return FALSE;
  }
static void combo_changed(GtkComboBox *combo, GtkWidget *widgets[])
  {
    gint combo_id=gtk_combo_box_get_active(combo);
    g_print("ComboId Changed %i\n", combo_id);
    if(combo_id==0)
      {
        steps=10;
        total_steps=20*steps;
      }
    else if(combo_id==1)
      {
        steps=20;
        total_steps=20*steps;
      }
    else if(combo_id==2)
      {
        steps=30;
        total_steps=20*steps;
      }
    else
      {
        steps=10;
        total_steps=10*steps;
      }
    gtk_widget_queue_draw(widgets[2]);
    gtk_widget_queue_draw(widgets[3]);
    gtk_widget_queue_draw(widgets[4]);
    gtk_widget_queue_draw(widgets[5]);
    gtk_widget_queue_draw(widgets[6]);
    gtk_widget_queue_draw(widgets[7]);
  }

