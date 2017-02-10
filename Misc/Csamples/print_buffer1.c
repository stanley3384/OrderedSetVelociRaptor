
/*
    Test print output from a text buffer.

    gcc -Wall print_buffer1.c -o print_buffer1 `pkg-config --cflags --libs gtk+-3.0`

    Tested on Ubuntu16.04 with GTK3.18.

    C. Eric Cashon
*/

#include<gtk/gtk.h>

static GtkWidget *window;

static void do_print(GtkWidget *widget, gpointer *data);
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, gpointer *data);
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_number, gpointer *data);
  
int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Print the Buffer");
    gtk_window_set_default_size(GTK_WINDOW(window), 400, 400);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 20);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *textview=gtk_text_view_new();
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD);
    gtk_widget_set_size_request(textview, 400, 300);
    //Should use CSS. Test different font sizes.
    PangoFontDescription *font=pango_font_description_from_string("Monospace 30");
    G_GNUC_BEGIN_IGNORE_DEPRECATIONS 
    gtk_widget_override_font(textview, font);
    G_GNUC_END_IGNORE_DEPRECATIONS
    pango_font_description_free(font);

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_widget_set_vexpand(scroll, TRUE);
    gtk_widget_set_hexpand(scroll, TRUE);
    gtk_container_add(GTK_CONTAINER(scroll), textview);

    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_set_text(buffer, "1\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n", -1); 

    gpointer pointers[]={textview, buffer};

    GtkWidget *button1=gtk_button_new_with_label("Print");
    g_signal_connect(button1, "clicked", G_CALLBACK(do_print), pointers);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 1, 1, 1);

    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
  }
static void do_print(GtkWidget *widget, gpointer *data)
{
  GtkPrintOperation *print;
  GtkPrintOperationResult res;

  print = gtk_print_operation_new();

  g_signal_connect (print, "begin_print", G_CALLBACK (begin_print), data);
  g_signal_connect (print, "draw_page", G_CALLBACK (draw_page), data);

  res = gtk_print_operation_run (print, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                                 GTK_WINDOW (window), NULL);

  if (res == GTK_PRINT_OPERATION_RESULT_APPLY)
    {
      g_print("Start Printing\n");
    }

  g_object_unref (print);
}
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, gpointer *data)
{
  gdouble height=gtk_print_context_get_height(context);
  
  PangoContext *pango_context=gtk_widget_get_pango_context(GTK_WIDGET(data[0]));
  PangoFontDescription *desc=pango_context_get_font_description(pango_context);
  PangoFontMetrics *metrics=pango_context_get_metrics(pango_context, desc, NULL);
  gdouble char_height=(gdouble)pango_font_metrics_get_ascent(metrics)/PANGO_SCALE;
  pango_font_metrics_unref(metrics);
  
  gint lines=gtk_text_buffer_get_line_count(GTK_TEXT_BUFFER(data[1]));
  gint lines_per_page=(gint)(height/char_height);
  gint n_pages=(lines_per_page+lines)/lines_per_page;

  g_print("Printing %i Pages\n", n_pages);

  gtk_print_operation_set_n_pages(operation, n_pages);
}
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, gpointer *data)
{
  cairo_t *cr=gtk_print_context_get_cairo_context(context);
  gdouble width=gtk_print_context_get_width(context);
  gdouble height=gtk_print_context_get_height(context);

  cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
  cairo_set_line_width(cr, 5);
  cairo_rectangle(cr, 2.0, 2.0, width-4.0, height-4.0);
  cairo_stroke(cr);
  cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);

  PangoLayout *layout = gtk_print_context_create_pango_layout (context);  
  PangoContext *pango_context=gtk_widget_get_pango_context(GTK_WIDGET(data[0]));
  PangoFontDescription *desc=pango_context_get_font_description(pango_context);
  PangoFontMetrics *metrics=pango_context_get_metrics(pango_context, desc, NULL);
  gdouble char_height=(gdouble)pango_font_metrics_get_ascent(metrics)/PANGO_SCALE;  
  pango_layout_set_font_description (layout, desc);
  gint lines_per_page=(gint)(height/char_height);

  //Print the text on the pages. Get offset text from the buffer.
  gint offset1=page_nr*(gint)lines_per_page;
  gint offset2=page_nr*(gint)lines_per_page+(gint)lines_per_page;
  GtkTextIter start;
  GtkTextIter end;
  gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(data[1]), &start, offset1);
  gtk_text_buffer_get_iter_at_line(GTK_TEXT_BUFFER(data[1]), &end, offset2);
  gchar *string=gtk_text_buffer_get_text(GTK_TEXT_BUFFER(data[1]), &start, &end, FALSE);
  pango_layout_set_text(layout, string, -1);
  g_free(string);

  pango_layout_set_width(layout, width * PANGO_SCALE);
  pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);

  cairo_move_to(cr, 10, 30);
  pango_cairo_show_layout(cr, layout);

  g_object_unref(layout);
  pango_font_metrics_unref(metrics);
}
