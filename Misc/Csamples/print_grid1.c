/*
    Print a few monospace fonts in a grid to get an idea of how to layout text for printing.  

    gcc -Wall print_grid1.c -o print_grid1 `pkg-config --cflags --libs gtk+-3.0` -lm
    Tested on Ubuntu16.04 and GTK3.18

    C. Eric Cashon
*/

#include<gtk/gtk.h>
#include<math.h>

//Some print variables to pass along to the print callbacks.
struct print_stuff
  {
    GtkWidget *window;
    GtkWidget *textview;
    PangoLayout *pango_layout;
    gdouble font_width;
    gdouble font_height;
    gint lines_per_page;
  };

static void print_dialog(GtkWidget *widget, struct print_stuff *ps);
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, struct print_stuff *ps);
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_number, struct print_stuff *ps);
static void end_print(GtkPrintOperation *operation, GtkPrintContext *context, struct print_stuff *ps);

int main(int argc, char *argv[])
  {
    gtk_init(&argc, &argv);
   
    GtkWidget *window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 700);
    gtk_window_set_title(GTK_WINDOW(window), "Print Dialog");
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_container_set_border_width(GTK_CONTAINER(window), 10);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    GtkWidget *textview1=gtk_text_view_new();
    gtk_widget_set_hexpand(textview1, TRUE);
    gtk_widget_set_vexpand(textview1, TRUE);
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview1));
    gtk_text_buffer_set_text(buffer, "1 alma\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28                             1\n1 alma2\n2\n3\n4\n5\n6\n7\n8\n9\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28                             1", -1);

    struct print_stuff ps;
    ps.window=window;
    ps.textview=textview1;
    ps.pango_layout=NULL; 
    ps.font_width=0; 
    ps.font_height=0; 
    ps.lines_per_page=0;

    GtkWidget *scroll=gtk_scrolled_window_new(NULL, NULL);
    gtk_container_add(GTK_CONTAINER(scroll), textview1);

    GtkWidget *button1=gtk_toggle_button_new_with_label("Print Dialog");
    g_signal_connect(button1, "clicked", G_CALLBACK(print_dialog), &ps);

    GtkWidget *grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), scroll, 0, 0, 4, 4);
    gtk_grid_attach(GTK_GRID(grid), button1, 0, 5, 4, 1);
   
    gtk_container_add(GTK_CONTAINER(window), grid);

    gtk_widget_show_all(window);
    
    gtk_main();

    return 0;
  }
static void print_dialog(GtkWidget *widget, struct print_stuff *ps)
  {
    GError *error=NULL;
    GtkPrintOperation *operation=gtk_print_operation_new();
    GtkPageSetup *setup=gtk_page_setup_new();
    //Try the A4 paper size. 
    GtkPaperSize *paper_size=gtk_paper_size_new("iso_a4");
    gtk_page_setup_set_paper_size(setup, paper_size);
    gtk_print_operation_set_default_page_setup(operation, setup);
    g_signal_connect(operation, "begin_print", G_CALLBACK(begin_print), ps);
    g_signal_connect(operation, "draw_page", G_CALLBACK(draw_page), ps);
    g_signal_connect(operation, "end_print", G_CALLBACK(end_print), ps);
    GtkPrintOperationResult res=gtk_print_operation_run(operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG, GTK_WINDOW(ps->window), &error);
    
    if(res==GTK_PRINT_OPERATION_RESULT_APPLY)
      {
        g_print("Print Success\n");
      }
    if(res==GTK_PRINT_OPERATION_RESULT_ERROR)
      {
        g_print("%s", error->message);
        g_error_free(error);
      }

    gtk_paper_size_free(paper_size);
    g_object_unref(setup);
    g_object_unref(operation);
  }
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, struct print_stuff *ps)
  {
    gdouble page_width=gtk_print_context_get_width(context);
    gdouble page_height=gtk_print_context_get_height(context);
    //Set a monospace font. Easier for figuring out layouts.
    PangoFontDescription *new_font=pango_font_description_from_string("Monospace 24");
    ps->pango_layout=gtk_print_context_create_pango_layout(context);
    pango_layout_set_font_description(ps->pango_layout, new_font);
    pango_layout_set_width(ps->pango_layout, page_width*PANGO_SCALE);
    pango_layout_set_height(ps->pango_layout, page_height*PANGO_SCALE);   

    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps->textview));
    gint lines=gtk_text_buffer_get_line_count(buffer);

    //Get font width and height for a monospace font.
    pango_layout_set_markup(ps->pango_layout, "5", -1);
    PangoRectangle rectangle_ink;
    PangoRectangle rectangle_log;
    pango_layout_get_extents(ps->pango_layout, &rectangle_ink, &rectangle_log);
    ps->font_width=(gdouble)rectangle_log.width/(gdouble)PANGO_SCALE;
    ps->font_height=(gdouble)rectangle_log.height/(gdouble)PANGO_SCALE;

    //Calculate lines per page. 28 lines of monspace 24 font fit on a A4 one page. 
    ps->lines_per_page=(gint)(page_height/(ps->font_height));
    gtk_print_operation_set_n_pages(operation, ceil((gdouble)lines/(gdouble)ps->lines_per_page));

    pango_font_description_free(new_font);  
  }
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_number, struct print_stuff *ps)
  {
    gint i=0;
    gint j=0;
    gdouble page_width=gtk_print_context_get_width(context);
    gdouble page_height=gtk_print_context_get_height(context);
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(ps->textview));    
    cairo_t *cr=gtk_print_context_get_cairo_context(context);

    //Draw rectangles around monospaced text.
    cairo_set_source_rgb(cr, 0.0, 1.0, 0.0);
    cairo_set_line_width(cr, 1);
    for(i=0;i<28;i++)
      { 
        for(j=0;j<32;j++)
          {
            cairo_rectangle(cr, (gdouble)j*(ps->font_width), (gdouble)i*(ps->font_height), ps->font_width, ps->font_height);
            cairo_stroke(cr);
          }
      }

    //Page border rectangle.
    cairo_set_source_rgb(cr, 0.0, 0.0, 1.0);
    cairo_set_line_width(cr, 2);
    cairo_rectangle(cr, 0, 0, page_width, page_height);
    cairo_stroke(cr);
   
    //Get the lines of text to put on the page.
    cairo_set_source_rgb(cr, 0, 0, 0);
    gint line_offset=page_number*(ps->lines_per_page);
    GtkTextIter start, end;
    gtk_text_buffer_get_iter_at_line(buffer, &start, line_offset);
    gtk_text_buffer_get_iter_at_line(buffer, &end, line_offset+(ps->lines_per_page)-1);
    gtk_text_iter_forward_to_line_end(&end);
    gchar *string=gtk_text_buffer_get_text(buffer, &start, &end, FALSE);

    //Set the layout.
    pango_layout_set_markup(ps->pango_layout, string, -1);
    pango_cairo_show_layout(cr, ps->pango_layout);
    g_free(string);
  }
static void end_print(GtkPrintOperation *operation, GtkPrintContext *context, struct print_stuff *ps)
  {
    g_object_unref(ps->pango_layout);
  }

