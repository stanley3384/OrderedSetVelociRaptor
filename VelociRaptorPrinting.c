
/*
 Printing functions for the VelociRaptor application.
     
 Copyright (c) 2013 by C. Eric Cashon. Licensed under the modified GNU GPL v2; see COPYING and COPYING2.
*/

#include "VelociRaptorPrinting.h"

static GtkPrintSettings *settings=NULL;
static void begin_print(GtkPrintOperation*, GtkPrintContext*, GtkTextView*);
static void draw_page(GtkPrintOperation*, GtkPrintContext*, gint, GtkTextView*);
static void end_print(GtkPrintOperation*, GtkPrintContext*, GtkTextView*);
static void printing_layout_set_text_attributes(GtkTextView *textview, GtkTextIter start1, GtkTextIter end1, PangoLayout *layout, GtkPrintContext *context);
static gint printing_text_iter_get_offset_bytes(GtkTextView *textview, const GtkTextIter *iter, const GtkTextIter *start1);

void print_textview(GtkWidget *menu, Widgets *w)
  {
     GtkPrintOperation *operation;
     GError *error=NULL;
     gint result;

     operation=gtk_print_operation_new();
     if (settings!=NULL)
        {
           gtk_print_settings_set_use_color(settings,TRUE);
           gtk_print_operation_set_print_settings (operation, settings);
        }

      g_signal_connect(G_OBJECT(operation), "begin_print", G_CALLBACK(begin_print), GTK_TEXT_VIEW(w->textview));
      g_signal_connect(G_OBJECT(operation), "draw_page", G_CALLBACK(draw_page), GTK_TEXT_VIEW(w->textview));
      g_signal_connect(G_OBJECT(operation), "end_print", G_CALLBACK(end_print), GTK_TEXT_VIEW(w->textview));

      result=gtk_print_operation_run(operation, GTK_PRINT_OPERATION_ACTION_PRINT_DIALOG,
                GTK_WINDOW(w->window), &error);
     
      switch(result)
         {
            case GTK_PRINT_OPERATION_RESULT_ERROR:
                printf("%s\n", error->message);
                g_error_free(error);
            case GTK_PRINT_OPERATION_RESULT_APPLY:
                if (settings)
                   {
                     g_object_unref(settings);
                   }
                settings = g_object_ref(gtk_print_operation_get_print_settings(operation));
            default:
                break;
          }
      
      g_object_unref(operation);
  }
static void begin_print(GtkPrintOperation *operation, GtkPrintContext *context, GtkTextView *textview)
  {
     printf("Begin Printing\n");
     gdouble height;
     gint lines;
     gint lines_per_page;
     gint total_pages;
     gint font_size;
     GtkTextBuffer *buffer=gtk_text_view_get_buffer(textview);
     PangoContext *context1=gtk_widget_get_pango_context(GTK_WIDGET(textview));
     PangoFontDescription *desc=pango_context_get_font_description(context1);

     if(!pango_font_description_get_size_is_absolute(desc))
       {
         font_size=pango_font_description_get_size(desc)/PANGO_SCALE; 
       }
     else
       {
         printf("Problem getting the font size. Print output probably formatted wrong.\n");
         font_size=10;
       }

      /*
       Get the font height and compare to Monospace. Fonts of the same point size have different heights. For example, Century Schoolbook is taller in the layout than Monospace.
     */
     gdouble font_ascent1, font_descent1, font_ascent2, font_descent2, font_height1, font_height2;
     PangoFontDescription * mono_desc=pango_font_description_new();
     pango_font_description_set_family(mono_desc, "Monospace");
     pango_font_description_set_size(mono_desc, font_size*PANGO_SCALE);
     PangoFont *font1=pango_context_load_font(context1, desc);
     PangoFont *font2=pango_context_load_font(context1, mono_desc);
     PangoFontMetrics *metrics1=pango_font_get_metrics(font1, NULL);
     PangoFontMetrics *metrics2=pango_font_get_metrics(font2, NULL);
     font_ascent1=pango_font_metrics_get_ascent(metrics1);
     font_descent1=pango_font_metrics_get_descent(metrics1);
     font_ascent2=pango_font_metrics_get_ascent(metrics2);
     font_descent2=pango_font_metrics_get_descent(metrics2);
     pango_font_description_free(mono_desc);
     font_height1=font_ascent1+font_descent1;
     font_height2=font_ascent2+font_descent2;

     lines=gtk_text_buffer_get_line_count(buffer);
     height=gtk_print_context_get_height(context)-40; 
     //Scale based on Monospace font.
     lines_per_page=floor(((double)height/((double)font_size+1))*(font_height2/font_height1));
     total_pages=((lines-1)/lines_per_page)+1;
     
     gtk_print_operation_set_n_pages(operation, total_pages);
   
  }
static void draw_page(GtkPrintOperation *operation, GtkPrintContext *context, gint page_nr, GtkTextView *textview)
  {
     printf("Draw Page %i\n", page_nr+1);
     cairo_t *cr;
     PangoLayout *layout;
     gint font_size;
     gint lines, height, lines_per_page, total_pages, page_width;
     gint indent=gtk_text_view_get_indent(textview);
     PangoContext *context1=gtk_widget_get_pango_context(GTK_WIDGET(textview));
     PangoFontDescription *desc=pango_context_get_font_description(context1);

     if(!pango_font_description_get_size_is_absolute(desc))
       {
         font_size=pango_font_description_get_size(desc)/PANGO_SCALE; 
       }
     else
       {
         printf("Problem getting the font size. Print output probably formatted wrong.\n");
         font_size=10;
       }
     printf("Global Font Size %i\n", font_size);
     
     /*
       Get the font height and compare to Monospace. Fonts of the same point size have different heights. For example, Century Schoolbook is taller in the layout than Monospace.
     */
     gdouble font_ascent1, font_descent1, font_ascent2, font_descent2, font_height1, font_height2;
     PangoFontDescription * mono_desc=pango_font_description_new();
     pango_font_description_set_family(mono_desc, "Monospace");
     pango_font_description_set_size(mono_desc, font_size*PANGO_SCALE);
     PangoFont *font1=pango_context_load_font(context1, desc);
     PangoFont *font2=pango_context_load_font(context1, mono_desc);
     PangoFontMetrics *metrics1=pango_font_get_metrics(font1, NULL);
     PangoFontMetrics *metrics2=pango_font_get_metrics(font2, NULL);
     font_ascent1=pango_font_metrics_get_ascent(metrics1);
     font_descent1=pango_font_metrics_get_descent(metrics1);
     font_ascent2=pango_font_metrics_get_ascent(metrics2);
     font_descent2=pango_font_metrics_get_descent(metrics2);
     pango_font_description_free(mono_desc);
     font_height1=font_ascent1+font_descent1;
     font_height2=font_ascent2+font_descent2;
     printf("Font Ascent %i Font Descent %i Font Height %i\n", (int)font_ascent1, (int)font_descent1, (int)font_height1);
     printf("Monospace Font Ascent %i Font Descent %i Font Height %i\n", (int)font_ascent2, (int)font_descent2, (int)font_height2);

     GtkTextIter start1, start2, end1, end2, newline;
     GtkTextBuffer *buffer=gtk_text_view_get_buffer(textview);

     lines=gtk_text_buffer_get_line_count(buffer);
     height=gtk_print_context_get_height(context)-40; 
     //Scale based on Monospace font.
     lines_per_page=floor(((double)height/((double)font_size+1))*(font_height2/font_height1));
     total_pages=((lines-1)/lines_per_page)+1;

     //Problem getting color on first number on the top of a page. Add a newline to solve it.
     if(lines<=lines_per_page)
        {
          gtk_text_buffer_get_bounds(buffer, &start1, &end1);
          GtkTextMark *newstart=gtk_text_buffer_create_mark(buffer, "startmark", &start1, TRUE);
          GtkTextMark *newend=gtk_text_buffer_create_mark(buffer, "endmark", &end1, TRUE);
          gtk_text_buffer_insert(buffer, &start1, "\n", 1);
          gtk_text_buffer_get_iter_at_mark(buffer, &start2, newstart);
          gtk_text_buffer_get_iter_at_mark(buffer, &end2, newend);
          printf("One Page\n");
        }
     else if(lines>lines_per_page&&page_nr<total_pages-1)
        {
          gtk_text_buffer_get_iter_at_line(buffer, &start1, (page_nr*lines_per_page));
          gtk_text_buffer_get_iter_at_line(buffer, &end1, ((page_nr*lines_per_page)+lines_per_page));
          GtkTextMark *newstart=gtk_text_buffer_create_mark(buffer, "startmark", &start1, TRUE);
          GtkTextMark *newend=gtk_text_buffer_create_mark(buffer, "endmark", &end1, TRUE);
          gtk_text_buffer_insert(buffer, &start1, "\n", 1);
          gtk_text_buffer_get_iter_at_mark(buffer, &start2, newstart);
          gtk_text_buffer_get_iter_at_mark(buffer, &end2, newend);
          printf("Full Page\n");
        }
     else
        {
          gtk_text_buffer_get_iter_at_line(buffer, &start1, (page_nr*lines_per_page));
          GtkTextMark *newstart=gtk_text_buffer_create_mark(buffer, "startmark", &start1, TRUE);
          gtk_text_buffer_insert(buffer, &start1, "\n", 1);
          gtk_text_buffer_get_iter_at_mark(buffer, &start2, newstart);
          gtk_text_buffer_get_end_iter(buffer, &end2);
          printf("Last Page\n");
        }

     cr=gtk_print_context_get_cairo_context(context);
     cairo_move_to(cr, indent, 0);
     page_width=gtk_print_context_get_width(context);

     layout=gtk_print_context_create_pango_layout(context);

     pango_layout_set_font_description(layout, desc);
     pango_layout_set_text(layout, gtk_text_buffer_get_text(buffer, &start2, &end2, FALSE),-1);
     
     printing_layout_set_text_attributes(textview, start2, end2, layout, context);
     //Remove the added newline.
     newline=start2;
     gtk_text_iter_forward_cursor_position(&start2);
     gtk_text_buffer_delete(buffer, &newline, &start2);

     pango_layout_set_width(layout, page_width*PANGO_SCALE);
     pango_layout_set_wrap(layout, PANGO_WRAP_WORD_CHAR);
     pango_layout_set_alignment(layout, PANGO_ALIGN_LEFT);

     pango_cairo_show_layout(cr, layout);

     g_object_unref(layout);

  }
static void end_print(GtkPrintOperation *operation, GtkPrintContext *context, GtkTextView *textview)
  {
     printf("End Printing\n");
  }
/*
 A truncated version of the following print code. An attempt to get the textview heatmapped platemaps
 to print. The original code is much more complete and a better example of doing this.

http://www.claws-mail.org
Claws Mail -- a GTK+ based, lightweight, and fast e-mail client
 Copyright (C) 2007-2012 Holger Berndt <hb@claws-mail.org>,
 Colin Leroy <colin@colino.net>, and the Claws Mail team
*/
static void printing_layout_set_text_attributes(GtkTextView *textview, GtkTextIter start1, GtkTextIter end1, PangoLayout *layout, GtkPrintContext *context)
{
        printf("Get Attributes\n");
        PangoAttrList *attr_list=NULL;
        PangoAttribute *attr=NULL;
        GSList *open_attrs=NULL;
        GSList *attr_walk=NULL;
        GtkTextIter iter=start1;

        //Check iterators.
        //printf("PageLines %i to %i\n", gtk_text_iter_get_line(&iter), gtk_text_iter_get_line(&end1));
        //Trouble getting the first tag for the layout.
        gtk_text_iter_backward_to_tag_toggle(&iter, NULL);

        attr_list = pango_attr_list_new();
        
        do{
            gboolean fg_set, bg_set, weight_set, size_set;
            GSList *tags=NULL;
            GSList *tag_walk=NULL;
            GtkTextTag *tag=NULL;
            GdkColor *color = NULL;
            gint weight;
            gint size;

           if(gtk_text_iter_ends_tag(&iter, NULL)) 
             {
               PangoAttrInt *attr_int;
               tags = gtk_text_iter_get_toggled_tags(&iter, FALSE);
               for(tag_walk = tags; tag_walk != NULL; tag_walk = tag_walk->next)
                  {
                    gboolean found;
                    tag = GTK_TEXT_TAG(tag_walk->data);
                    g_object_get(G_OBJECT(tag), "background-set", &bg_set, "foreground-set", &fg_set, "weight-set", &weight_set, "size", &size_set, NULL);
                    if(fg_set)
                      {
                        found = FALSE;
                        for(attr_walk = open_attrs; attr_walk != NULL; attr_walk = attr_walk->next) 
                           {
                             attr = (PangoAttribute*)attr_walk->data;
                             if(attr->klass->type == PANGO_ATTR_FOREGROUND)
                               {
                                 g_object_get(G_OBJECT(tag), "foreground_gdk", &color, NULL);
                                 if(color)
                                   {
                                     attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                                     pango_attr_list_insert(attr_list, attr);
                                     found = TRUE;
                                     open_attrs = g_slist_delete_link(open_attrs, attr_walk);
                                     break;
                                   }
                                 if(color)
                                   {
                                     gdk_color_free(color);
                                   }
                                }
                             }
                         if(!found)
                            {
                              //printf("Error generating foreground attribute list.\n");
                            }
                      }
                    if(bg_set)
                      {
                        found=FALSE;
                        for(attr_walk=open_attrs; attr_walk!=NULL; attr_walk=attr_walk->next)
                           {
                             attr=(PangoAttribute*)attr_walk->data;
                             if(attr->klass->type==PANGO_ATTR_BACKGROUND)
                               {
                                 g_object_get(G_OBJECT(tag), "background-gdk", &color, NULL);
                                 if(color)
                                   {
                                     attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                                     pango_attr_list_insert(attr_list, attr);
                                     found = TRUE;
                                     open_attrs = g_slist_delete_link(open_attrs, attr_walk);
                                     break;
                                   }
                                  if(color)
                                    {
                                      gdk_color_free(color);
                                    }
                                }
                           }
                         if(!found)
                           {
                             //printf("Error generating background attribute list.\n");
                           }
                       }
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
                                     attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                                     pango_attr_list_insert(attr_list, attr);
                                     found = TRUE;
                                     open_attrs = g_slist_delete_link(open_attrs, attr_walk);
                                     break;
                                   }
                               }
                           }
                         if(!found)
                           {
                             //printf("Error generating weight list.\n");
                           }
                        } 
                     if(size_set)
                      {
                        found = FALSE;
                        for(attr_walk = open_attrs; attr_walk != NULL; attr_walk = attr_walk->next)
                           {
                             attr = (PangoAttribute*)attr_walk->data;
                             if(attr->klass->type == PANGO_ATTR_SIZE)
                               {
                                 attr_int = (PangoAttrInt*)attr;
                                 g_object_get(G_OBJECT(tag), "size", &size, NULL);
                                 if(attr_int->value == size)
                                   {
                                     attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                                     pango_attr_list_insert(attr_list, attr);
                                     found = TRUE;
                                     open_attrs = g_slist_delete_link(open_attrs, attr_walk);
                                     break;
                                   }
                               }
                           }
                         if(!found)
                           {
                             //printf("Error generating size list.\n");
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
                    g_object_get(G_OBJECT(tag), "background-set", &bg_set, "foreground-set", &fg_set, "weight-set", &weight_set, "size", &size_set, NULL);
                    if(fg_set)
                      {
                        g_object_get(G_OBJECT(tag), "foreground-gdk", &color, NULL);
                        attr = pango_attr_foreground_new(color->red,color->green,color->blue);
                        attr->start_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                        open_attrs = g_slist_prepend(open_attrs, attr);
                      }
                    if(bg_set)
                      {
                        g_object_get(G_OBJECT(tag), "background-gdk", &color, NULL);
                        attr = pango_attr_background_new(color->red,color->green,color->blue);
                        attr->start_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                        open_attrs = g_slist_prepend(open_attrs, attr);
                      }
                    if(weight_set && !weight_set_for_this_iter)
                      {
                        weight_set_for_this_iter = TRUE;
                        g_object_get(G_OBJECT(tag), "weight", &weight, NULL);
                        attr = pango_attr_weight_new(weight);
                        attr->start_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                        open_attrs = g_slist_prepend(open_attrs, attr);
                      }
                     if(size_set)
                      {
                        g_object_get(G_OBJECT(tag), "size", &size, NULL);
                        attr = pango_attr_size_new(size);
                        attr->start_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                        open_attrs = g_slist_prepend(open_attrs, attr);
                      }
                   }
                 g_slist_free(tags);
              }
         }while(gtk_text_iter_compare(&iter, &end1)<0 && gtk_text_iter_forward_to_tag_toggle(&iter, NULL));

        //printf("End PageLine %i\n", gtk_text_iter_get_line(&iter));

        /* close all open attributes */
        for (attr_walk = open_attrs; attr_walk != NULL; attr_walk = attr_walk->next) {
                attr = (PangoAttribute*) attr_walk->data;
                attr->end_index = printing_text_iter_get_offset_bytes(textview, &iter, &start1);
                pango_attr_list_insert(attr_list, attr);
        }     

        g_slist_free(open_attrs);
        g_slist_free(attr_walk);

        pango_layout_set_attributes(layout, attr_list);
          
        pango_attr_list_unref(attr_list);
}

static gint printing_text_iter_get_offset_bytes(GtkTextView *textview, const GtkTextIter *iter, const GtkTextIter *start1)
{
        gint off_bytes;
        gchar *text;

        text = gtk_text_iter_get_text(start1, iter);
        off_bytes = strlen(text);
        g_free(text);
        return off_bytes;
}

