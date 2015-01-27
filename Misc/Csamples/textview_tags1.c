
/*

    Test code for getting a couple of bold text tags from a textview and outputting them
  with the text to a png, svg or pdf file.

  gcc -Wall textview_tags1.c -o textview_tags1 `pkg-config --cflags --libs gtk+-3.0`

  C. Eric Cashon

*/

#include<gtk/gtk.h>
#include<cairo-svg.h>
#include<cairo-pdf.h>
#include<string.h>

static void create_textview_file(GtkWidget *button, GtkWidget *textview);
static void layout_set_text_attributes(GtkTextView *textview, GtkTextIter start, GtkTextIter end, PangoLayout *layout);
static gint printing_text_iter_get_offset_bytes(GtkTextView *textview, const GtkTextIter *iter, const GtkTextIter *start);


int main(int argc, char *argv[])
  {
    GtkWidget *window, *textview, *button, *grid;
    GtkTextIter iter1, iter2;

    gtk_init(&argc, &argv);

    window=gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_resizable(GTK_WINDOW(window), TRUE);
    gtk_window_set_title(GTK_WINDOW(window), "Test Tags");
    gtk_container_set_border_width(GTK_CONTAINER(window), 8);
    gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
    gtk_window_set_default_size(GTK_WINDOW(window), 300, 150);
    g_signal_connect(window, "destroy", G_CALLBACK(gtk_main_quit), NULL);

    textview=gtk_text_view_new();
    gtk_widget_set_hexpand(textview, TRUE);
    gtk_widget_set_vexpand(textview, TRUE);
    gtk_text_view_set_wrap_mode(GTK_TEXT_VIEW(textview), GTK_WRAP_WORD_CHAR);
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_insert_at_cursor(buffer, "Test some text bold tags", -1);
    GtkTextTag *bold_tag=gtk_text_buffer_create_tag(buffer, "bold_font", "weight", 800, NULL);
    gtk_text_buffer_get_start_iter(buffer, &iter1);
    gtk_text_buffer_get_start_iter(buffer, &iter2);
    gtk_text_iter_forward_visible_cursor_positions(&iter1, 5);
    gtk_text_iter_forward_visible_cursor_positions(&iter2, 10);
    gtk_text_buffer_apply_tag(buffer, bold_tag, &iter1, &iter2);
    gtk_text_iter_forward_visible_cursor_positions(&iter1, 10);
    gtk_text_iter_forward_visible_cursor_positions(&iter2, 9);
    gtk_text_buffer_apply_tag(buffer, bold_tag, &iter1, &iter2);
  
    button=gtk_button_new_with_label("Create File");
    g_signal_connect(button, "clicked", G_CALLBACK(create_textview_file), textview);

    grid=gtk_grid_new();
    gtk_grid_attach(GTK_GRID(grid), textview, 0, 2, 1, 1);
    gtk_grid_attach(GTK_GRID(grid), button, 0, 3, 1, 1); 
    gtk_container_add(GTK_CONTAINER(window), grid);
 
    gtk_widget_show_all(window);
    gtk_main();
    return 0;

  }
static void create_textview_file(GtkWidget *button, GtkWidget *textview)
  {
    g_print("Create File\n");
    GtkTextIter start, end;
    GtkTextBuffer *buffer=gtk_text_view_get_buffer(GTK_TEXT_VIEW(textview));
    gtk_text_buffer_get_bounds(buffer, &start, &end);
    guint width=gtk_widget_get_allocated_width(textview);
    guint height=gtk_widget_get_allocated_height(textview);
   
    //Change to the surface that you want. If png also uncomment write to png function below.
    cairo_surface_t *surface=cairo_pdf_surface_create("textview.pdf", width, height);
    //cairo_surface_t *surface=cairo_svg_surface_create("textview.svg", width, height);
    //cairo_surface_t *surface=cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);

    cairo_t *cr=cairo_create(surface);
    cairo_set_source_rgb(cr, 1.0, 1.0, 1.0);
    cairo_paint(cr);
    cairo_move_to(cr, 0, 0);

    PangoContext *context=gtk_widget_get_pango_context(GTK_WIDGET(textview));
    PangoFontDescription *desc=pango_context_get_font_description(context);
    PangoLayout *layout=pango_cairo_create_layout(cr);
    
    cairo_set_source_rgb(cr, 0.0, 0.0, 0.0);
    pango_layout_set_font_description(layout, desc);
    pango_layout_set_width(layout, width*PANGO_SCALE);
    pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
    pango_layout_set_text(layout, gtk_text_buffer_get_text(buffer, &start, &end, FALSE),-1);
    layout_set_text_attributes(GTK_TEXT_VIEW(textview), start, end, layout);
    pango_cairo_update_layout(cr, layout);
    pango_cairo_show_layout(cr, layout);

    //cairo_surface_write_to_png(surface, "textview.png");
    g_print("textview.pdf created\n");

    cairo_destroy(cr);
    cairo_surface_destroy(surface);
    g_object_unref(layout); 
  }
/*
The following is an abbreviated version of the following just for an example with bold tags.
https://github.com/mishamehra/claws-mail/blob/1cab7c9949355b126509a4a756677241f27555c4/src/printing.c
printing_layout_set_text_attributes function
*/
static void layout_set_text_attributes(GtkTextView *textview, GtkTextIter start, GtkTextIter end, PangoLayout *layout)
 {
        PangoAttrList *attr_list=NULL;
        PangoAttribute *attr=NULL;
        GSList *open_attrs=NULL;
        GSList *attr_walk=NULL;
        GtkTextIter iter=start;

        //Make sure to get the first tag.
        gtk_text_iter_backward_to_tag_toggle(&iter, NULL);

        attr_list = pango_attr_list_new();
        
        do{
            gboolean weight_set;
            GSList *tags=NULL;
            GSList *tag_walk=NULL;
            GtkTextTag *tag=NULL;
            PangoWeight weight;

           if(gtk_text_iter_ends_tag(&iter, NULL)) 
             {
               PangoAttrInt *attr_int;
               tags = gtk_text_iter_get_toggled_tags(&iter, FALSE);
               for(tag_walk = tags; tag_walk != NULL; tag_walk = tag_walk->next)
                  {
                    gboolean found;
                    tag = GTK_TEXT_TAG(tag_walk->data);
                    g_object_get(G_OBJECT(tag), "weight-set", &weight_set, NULL);                  
                    if(weight_set)
                      {
                        found = FALSE;
                        for(attr_walk = open_attrs; attr_walk != NULL; attr_walk = attr_walk->next)
                           {
                             attr = (PangoAttribute*)attr_walk->data;
                             if(attr->klass->type == PANGO_ATTR_WEIGHT)
                               {
                                 attr_int = (PangoAttrInt*)attr;
                                 g_object_get(G_OBJECT(tag), "weight", &weight, NULL);
                                 if(attr_int->value == weight)
                                   {
                                     attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start);
                                     pango_attr_list_insert(attr_list, attr);
                                     found = TRUE;
                                     open_attrs = g_slist_delete_link(open_attrs, attr_walk);
                                     break;
                                   }
                               }
                           }
                         if(!found)
                           {
                             //g_print("Error generating weight list.\n");
                           }
                        } 
                     
                     }
                  g_slist_free(tags);
              }

            if(gtk_text_iter_begins_tag(&iter, NULL))
              {
                tags = gtk_text_iter_get_toggled_tags(&iter, TRUE);
                /* Sometimes, an iter has several weights. Use only the first in this case */
                gboolean weight_set_for_this_iter;
                weight_set_for_this_iter = FALSE;
                for(tag_walk = tags; tag_walk != NULL; tag_walk = tag_walk->next)
                  {
                    tag=GTK_TEXT_TAG(tag_walk->data);
                    g_object_get(G_OBJECT(tag), "weight-set", &weight_set, NULL);
                    if(weight_set && !weight_set_for_this_iter)
                      {
                        weight_set_for_this_iter = TRUE;
                        g_object_get(G_OBJECT(tag), "weight", &weight, NULL);
                        attr = pango_attr_weight_new(weight);
                        attr->start_index = printing_text_iter_get_offset_bytes(textview, &iter, &start);
                        open_attrs = g_slist_prepend(open_attrs, attr);
                      }
                   }
                 g_slist_free(tags);
              }
         }while(gtk_text_iter_compare(&iter, &end)<0 && gtk_text_iter_forward_to_tag_toggle(&iter, NULL));

        /* close all open attributes */
        for (attr_walk = open_attrs; attr_walk != NULL; attr_walk = attr_walk->next) {
                attr = (PangoAttribute*) attr_walk->data;
                attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start);
                pango_attr_list_insert(attr_list, attr);
        }     

        g_slist_free(open_attrs);
        g_slist_free(attr_walk);

        pango_layout_set_attributes(layout, attr_list);
          
        pango_attr_list_unref(attr_list);
  }
static gint printing_text_iter_get_offset_bytes(GtkTextView *textview, const GtkTextIter *iter, const GtkTextIter *start)
  {
        gint off_bytes;
        gchar *text;

        text = gtk_text_iter_get_text(start, iter);
        off_bytes = strlen(text);
        g_free(text);
        return off_bytes;
  }

