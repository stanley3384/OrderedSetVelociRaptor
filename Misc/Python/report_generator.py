#!/usr/bin/python

"""
   Test some Python and GTK+ printing. Print size defaults to "letter" or 8.5 x 11 sized layout.
   Look at some layout values for positioning fonts and graphics. Work towards making a report
   generator for grids, tables and crosstabs. Change things around as needed.

   Python 2.7 with GTK 3.10 on Ubuntu 14.04.

   C. Eric Cashon
"""

from gi.repository import Gtk, Gdk, Pango, PangoCairo
from gi.repository import GdkPixbuf
from gi.repository import GLib
import cairo
import random
import math
import sys
import re
import sqlite3 as lite
from operator import itemgetter
from itertools import product

g_row_labels = []
g_column_labels = []

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        font_description = Pango.FontDescription("Monospace 12") 
        self.override_font(font_description);
        self.entries_array_text = {}
        self.page_width = 0
        self.page_height = 0
        self.char_width = 0
        self.char_height = 0
        self.text_width = 0
        self.text_height = 0
        self.line_count = 0
        self.lines_per_page = 0
        self.total_lines = 0
        self.pango_markup_string = ""
        self.set_wrap_mode(0)
        self.set_cursor_visible(True)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("       This is the title for the report.\n This is a paragraph.")
        self.font_tag_8 = self.textbuffer.create_tag("font8", font=8)
        self.font_tag_10 = self.textbuffer.create_tag("font10", font=10)
        self.font_tag_12 = self.textbuffer.create_tag("font12", font=12)
        self.font_tag_14 = self.textbuffer.create_tag("font14", font=14)
        self.font_tag_16 = self.textbuffer.create_tag("font16", font=16)
        self.bold_tag = self.textbuffer.create_tag("bold", weight=900)
        self.underline_tag = self.textbuffer.create_tag("underline", underline=Pango.Underline.SINGLE)

    def change_textview_font(self, combo3):
        font = combo3.get_active_text()
        font_description = Pango.FontDescription(font) 
        self.override_font(font_description);
        
    def get_title(self):
        start = self.textbuffer.get_start_iter()
        end = self.textbuffer.get_end_iter()
        string = self.textbuffer.get_text(start, end, True)
        return string

    def print_dialog(self, entries_array):
        self.entries_array_text = entries_array
        operation = Gtk.PrintOperation()
        operation.connect("begin_print", self.begin_print)
        operation.connect("draw_page", self.draw_page)
        result = operation.run(Gtk.PrintOperationAction.PRINT_DIALOG, None)

    def begin_print(self, operation, gtk_context):
        self.plate_counter = 1
        self.plate_counter_sql =1
        self.page_width = gtk_context.get_width()
        self.page_height = gtk_context.get_height()
        pango_context = self.get_pango_context()
        description = pango_context.get_font_description()
        self.pango_layout = gtk_context.create_pango_layout()
        self.pango_layout.set_font_description(description)
        self.pango_layout.set_width(self.page_width*Pango.SCALE);
        self.pango_layout.set_height(self.page_height*Pango.SCALE);
        self.text_width = self.pango_layout.get_width()
        self.text_height = self.pango_layout.get_height()
   
        #Figure out number of pages.
        count_lines = self.textbuffer.get_line_count()
        #Float tables value to force floating point calculation.
        tables = float(self.entries_array_text[7].get_text())
        rows = int(self.entries_array_text[0].get_text())
        shift_below_text = int(self.entries_array_text[3].get_text())
        combo2_index = int(self.entries_array_text[13].get_active_id())
        combo6_index = int(self.entries_array_text[17].get_active_id()) * 2
        label_lines = tables 
        
        #Get rectangle for one monospace char for sizing
        self.pango_layout.set_markup("5")
        rectangle_ink, rectangle_log = self.pango_layout.get_extents()
        #print(" Rectangle1 " + str(rectangle_ink.height) + " Rectangle1 " + str(rectangle_ink.width) + " Rectangle2 " + str(rectangle_log.height) + " Rectangle2 " + str(rectangle_log.width))

        #Get lines of text to be printed on the page.
        #self.get_pango_markup()
        string = "Test Line Count String."
        self.pango_layout.set_markup(string)
        self.line_count = self.pango_layout.get_line_count()
        self.lines_per_page = int(self.text_height/rectangle_log.height)
   
        #Use combo6_index to nudge total_lines if the last page doesn't fit the drawing. 
        self.total_lines = count_lines + ((shift_below_text-1)*tables) + (tables * rows) + label_lines + combo6_index
        print("Total Lines " + str(self.total_lines) + " Lines per Page " + str(self.lines_per_page))
        pages = int(math.ceil((self.total_lines)/self.lines_per_page))
        print("Pages " + str(pages))
        operation.set_n_pages(pages)
       
        #Turn off wrapping
        self.pango_layout.set_width(-1)
        #self.pango_layout.set_wrap(Pango.WrapMode.CHAR)

    def draw_page(self, operation, gtk_context, page_number):
        #Figure out number lines of tables per page.
        count_lines = self.textbuffer.get_line_count()
        #Float tables value to force floating point calculation.
        tables = float(self.entries_array_text[7].get_text())
        rows = int(self.entries_array_text[0].get_text())
        shift_below_text = int(self.entries_array_text[3].get_text())
        combo2_index = int(self.entries_array_text[13].get_active_id())
        combo5_index = int(self.entries_array_text[16].get_active_id())
        #Font Color 
        font_rgb=[0.0, 0.0, 0.0]      
        #Account for first page with title and text.
        tables_first_page = int((self.lines_per_page-count_lines)/(rows + shift_below_text - 1))
        tables_next_page = int(self.lines_per_page/(rows + shift_below_text - 1)) 
        if(page_number==0):
            tables_left_to_print = tables
        else:
            index = page_number - 1        
            tables_left_to_print = int(tables - (index*tables_next_page) - tables_first_page)
        print("Tables Left to Print " + str(tables_left_to_print))
        pages = int(math.ceil((self.total_lines)/self.lines_per_page))
        #print("Total Lines " + str(self.total_lines)) 
        if(page_number==0):
            tables_on_page = tables_first_page
        elif(page_number==pages-1):
            tables_on_page = tables_left_to_print
        else:
            tables_on_page = tables_next_page
        print("Tables on Page " + str(tables_on_page))
        cairo_context = gtk_context.get_cairo_context()
        table_string = ""
        for table in range(tables_on_page):
            table_string = table_string + self.draw_tables(operation, gtk_context, page_number, cairo_context, table, count_lines)
        #Set text and tables.
        cairo_context.set_source_rgb(font_rgb[0], font_rgb[1], font_rgb[2])
        if(page_number == 0):
            self.get_pango_markup()
        else:
            self.markup_string = ""
        self.pango_layout.set_markup(self.markup_string + table_string)
        #Draw page border.
        if(combo5_index!=1):
            if(combo5_index==2):
                cairo_context.set_source_rgb(0.0, 0.0, 0.0)
            elif(combo5_index==3):
                cairo_context.set_source_rgb(1.0, 0.0, 0.0)
            elif(combo5_index==4):
                cairo_context.set_source_rgb(0.0, 1.0, 0.0)
            elif(combo5_index==5):
                cairo_context.set_source_rgb(0.0, 0.0, 1.0)
            else:
                cairo_context.set_source_rgb(1.0, 1.0, 1.0)
            cairo_context.rectangle(0, 0, self.page_width, self.page_height)
            cairo_context.stroke()
        cairo_context.set_source_rgb(0.0, 0.0, 0.0)
        PangoCairo.show_layout(cairo_context, self.pango_layout)

    def draw_tables(self, operation, gtk_context, page_number, cairo_context, table, count_lines):
        #Get variables from UI.
        rows = int(self.entries_array_text[0].get_text())
        columns = int(self.entries_array_text[1].get_text())
        shift_margin = int(self.entries_array_text[2].get_text())
        shift_below_text = int(self.entries_array_text[3].get_text())
        column_width = int(self.entries_array_text[4].get_text())
        shift_number_left = int(self.entries_array_text[5].get_text())
        shift_column_left = int(self.entries_array_text[6].get_text())
        tables = int(self.entries_array_text[7].get_text())
        table_name = self.entries_array_text[8].get_text()
        sql_string = self.entries_array_text[9].get_text()
        round_float = int(self.entries_array_text[10].get_text())
        check1_active = self.entries_array_text[11].get_active()
        combo1_index = int(self.entries_array_text[12].get_active_id())
        combo2_index = int(self.entries_array_text[13].get_active_id())
        combo4_index = int(self.entries_array_text[15].get_active_id())
        table_rectangles_rgb=[0.8, 0.8, 0.8]
        table_grid_rgb=[0.8, 0.8, 0.8]

        #Shift tables for multiple tables.
        if(page_number == 0):
            shift_below_text2 = (count_lines -1) + shift_below_text + (table * (rows + shift_below_text -1))
        else:
            shift_below_text2 = shift_below_text + (table * (rows + shift_below_text -1))
        
        #Get some test data.
        if(combo4_index == 1):
            min_value, max_value, data_values = self.get_test_data(rows, columns, shift_number_left, round_float)
        elif(combo4_index == 2):
            min_value, max_value, data_values = self.get_test_data_db(rows, columns, shift_number_left, round_float)
        elif(combo4_index == 3):
            min_value, max_value, data_values = self.get_db_data_for_crosstab(rows, columns, tables, sql_string, shift_number_left, round_float)
        else:
            min_value, max_value, data_values, column_labels, column_number = self.get_db_data_for_table(rows, columns, tables, sql_string, shift_number_left, round_float)
            columns = column_number

        #Create label values. Check if labels have already been created.
        vertical_labels = []
        horizontal_labels = []
        string_column_shift_left = ""
        string_column_shift_left = "{: <{m}}".format("", m=shift_column_left)
        if(not g_row_labels):
            for x in range(rows):
                vertical_labels.insert(x, " " + str(x+1) + " ")
        else:
            for x in range(rows):
                vertical_labels.insert(x, g_row_labels[x]) 
        #If the table columns are coming from the database field names.
        if(combo4_index==4):
            for col in column_labels:
                horizontal_labels.append(col + string_column_shift_left)
        else:
            if(not g_column_labels):
                for y in range(columns):
                    horizontal_labels.insert(y, "Column" + str(y+1) + string_column_shift_left)
            else:
                for y in range(columns):
                    horizontal_labels.insert(y, g_column_labels[y] + string_column_shift_left)

        #Get max length of grid and label values.
        max_length = 0
        max_vertical_label = 0
        max_horizontal_label = 0
        for x in range(rows):
            if(max_vertical_label < len(str(vertical_labels[x]))):
                max_vertical_label = len(str(vertical_labels[x]))
            for y in range(columns):
                if(max_length < len(str(data_values[x][y]))):
                    max_length = len(str(data_values[x][y]))
        for y in range(columns):
            if(max_horizontal_label < len(str(horizontal_labels[y]))):
                max_horizontal_label = len(str(horizontal_labels[y]))

        #Check if string lengths will fit in boxes. If they don't, truncate them.
        #The vertical label width is set by the max_vertical_label value.
        if(column_width<max_horizontal_label):
            horizontal_labels[:] = (elem[:column_width] for elem in horizontal_labels)
        if(column_width<max_length):
            for x in range(rows):
                for y in range(columns):
                    data_values[x][y] = data_values[x][y][:column_width]
        
        #Get size difference in text due to font tags on page 1.
        if(page_number == 0):
            buffer_string = self.get_title()
            self.pango_layout.set_markup(buffer_string)
            buffer_ink, buffer_log = self.pango_layout.get_extents()
            self.get_pango_markup()
            self.pango_layout.set_markup(self.markup_string)
            markup_ink, markup_log = self.pango_layout.get_extents()
            #print("Markup Difference " + str(markup_log.height - buffer_log.height))
            if(markup_log.height - buffer_log.height > 0):
                markup_difference = markup_log.height - buffer_log.height
            else:
                markup_difference = 0 
        else:
            markup_difference = 0

        #Get rectangle for one monospace char for sizing.
        self.pango_layout.set_markup("5")
        rectangle_ink, rectangle_log = self.pango_layout.get_extents()
        #print(" Rectangle1 " + str(rectangle_ink.height) + " Rectangle1 " + str(rectangle_ink.width) + " Rectangle2 " + str(rectangle_log.height) + " Rectangle2 " + str(rectangle_log.width))
       
        #Shift table down and adjust for title.
        if(check1_active):
            if(combo2_index==2 or combo2_index==3):  
                number_string = "{:\n>{m}}".format("", m=shift_below_text-2)
                number_string = number_string + table_name + str(self.plate_counter) + "\n"
                self.plate_counter+=1
            else:
                number_string = "{:\n>{m}}".format("", m=shift_below_text-1)
                number_string = number_string + table_name + str(self.plate_counter) + "\n"
                self.plate_counter+=1
        else:
            if(combo2_index==2 or combo2_index==3):  
                number_string = "{:\n>{m}}".format("", m=shift_below_text-1)
            else:
                number_string = "{:\n>{m}}".format("", m=shift_below_text)

        #Pad each number and first number in each column.
        column_padding = "{: >{m}}".format("", m=shift_margin)
        pad_horizontal_label = ""
        pad_vertical_label = ""
        if(combo2_index == 3):
            pad_horizontal_label = "{: >{m}}".format("", m=max_vertical_label) 
        #Horizontal label string.
        if(combo2_index==2 or combo2_index == 3):
            horizontal_label_string = column_padding + pad_horizontal_label + "".join( "{k: >{m}}".format(k=k,m=max_length + (column_width-max_length)) for k in horizontal_labels)
            number_string = number_string + horizontal_label_string +"\n"
        #Grid strings 
        for x in range(rows): 
            if(combo2_index==3):
                label_length = max_vertical_label - len(vertical_labels[x])
                pad_vertical_label = "{: >{m}}".format("", m=label_length)  
                row_tuple = vertical_labels[x]+pad_vertical_label +"".join( "{k: >{m}}".format(k=k,m=max_length + (column_width-max_length)) for k in data_values[x])
            else:
                row_tuple = "".join( "{k: >{m}}".format(k=k,m=max_length + (column_width-max_length)) for k in data_values[x])
            #Append row_tuple
            if(x<rows-1):
                number_string = number_string + column_padding + row_tuple + "\n"
            else:
                number_string = number_string + column_padding + row_tuple

        #Draw vertical label rectangle for crosstabs.
        top = self.line_count + shift_below_text2 - 2
        bottom = top + rows
        cairo_context.set_source_rgb(table_rectangles_rgb[0], table_rectangles_rgb[1], table_rectangles_rgb[2])
        if(combo2_index==3):
            cairo_context.rectangle(((shift_margin) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top) + markup_difference)/Pango.SCALE, (rectangle_log.width/Pango.SCALE)*(max_vertical_label+.50), (rectangle_log.height*(rows+1))/Pango.SCALE)
            cairo_context.fill()
            cairo_context.stroke()
            #Draw lines for rectangle.
            #Vertical left.
            #cairo_context.set_source_rgb(0.0, 0.0, 0.0)
            cairo_context.move_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(bottom+1) + markup_difference)/Pango.SCALE)
            cairo_context.stroke()
            #Vertical right is drawn with grid.
            #Short top horizontal.
            cairo_context.move_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top) + markup_difference)/Pango.SCALE)
            cairo_context.stroke()  
            #Short bottom horizontal
            cairo_context.move_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (bottom+1) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(bottom+1) + markup_difference)/Pango.SCALE)
            cairo_context.stroke()  

        #Draw horizontal label rectangle for both crosstab and tabular data.
        top = self.line_count + shift_below_text2 - 2
        cairo_context.set_line_cap(cairo.LINE_CAP_SQUARE)
        if(combo2_index==1 or combo2_index==2):
            max_vertical_label = 0
        if(combo2_index==2 or combo2_index==3):
            cairo_context.rectangle(((shift_margin+max_vertical_label) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top) + markup_difference)/Pango.SCALE, ((rectangle_log.width*columns*column_width)/Pango.SCALE), (rectangle_log.height)/Pango.SCALE)
            cairo_context.fill()
            cairo_context.stroke()
            #Draw lines for rectangle.
            cairo_context.set_line_width(2)
            #Top horizontal
            #cairo_context.set_source_rgb(0.0, 0.0, 0.0)
            cairo_context.move_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label+(column_width*columns)) *rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top) + markup_difference)/Pango.SCALE)
            cairo_context.stroke() 
            #Short top vertical left.
            cairo_context.move_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top+1) + markup_difference)/Pango.SCALE)
            cairo_context.stroke() 
            #Short top vertical right.
            cairo_context.move_to(((shift_margin+max_vertical_label + (column_width*columns))*rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label+(column_width*columns))*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top+1) + markup_difference)/Pango.SCALE)
            cairo_context.stroke() 
        
        #Background color for each cell.
        top = self.line_count + shift_below_text2 -1
        shift_margin = shift_margin + max_vertical_label
        if(combo1_index != 1):
            for x in range(rows):
                for y in range(columns+1):
                    if(combo1_index==2):
                        if(x%2):
                            cairo_context.set_source_rgb(0.5, 0.7, 1.0)
                        else:
                            cairo_context.set_source_rgb(0.7, 1.0, 1.0)
                    else:
                    #Correct for rectangles that don't fill. Should be a better way to do this.
                    #Extend width by 2. This leaves a fragment on the bottom right that needs 
                    #to be corrected after the grid is drawn.
                        if(y != columns):
                            if(combo1_index==3):
                                red, green, blue = self.heatmap_value_rgb(data_values[x][y], max_value, min_value)
                            elif(combo1_index==4):
                                red, green, blue = self.heatmap_value_bry(data_values[x][y], max_value, min_value)
                            else:
                                red, green, blue = self.heatmap_value_iris(data_values[x][y], max_value, min_value)
                            cairo_context.set_source_rgb(red, green, blue) 
                    if(y != columns):
                        cairo_context.rectangle(((shift_margin +(column_width*y)) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top + x) + markup_difference)/Pango.SCALE, (rectangle_log.width/Pango.SCALE)*(column_width+2), (rectangle_log.height)/Pango.SCALE)
                    else:
                        #White out after end of table.
                        if(x!=0):
                            cairo_context.set_source_rgb(1.0, 1.0, 1.0) 
                            cairo_context.rectangle(((shift_margin +(column_width*y)) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top + x - 1) + markup_difference)/Pango.SCALE, (rectangle_log.width/Pango.SCALE)*column_width, (rectangle_log.height*2)/Pango.SCALE)
                    cairo_context.fill()
                    cairo_context.stroke()
        
        #Table grid for test numbers.
        top = self.line_count + shift_below_text2 -1
        bottom = top + rows
        total_chars = column_width * columns
        left_margin = (shift_margin * rectangle_log.width)/Pango.SCALE
        #First draw over fragment left by overlapping color backgrounds.
        cairo_context.set_source_rgb(1.0, 1.0, 1.0)
        cairo_context.set_line_width(2)
        cairo_context.move_to(((rectangle_log.width * total_chars)/Pango.SCALE) + left_margin, (rectangle_log.height  *(top + rows) + markup_difference)/Pango.SCALE)
        cairo_context.line_to(((rectangle_log.width * (total_chars+3))/Pango.SCALE) + left_margin, (rectangle_log.height  *(top + rows) + markup_difference)/Pango.SCALE)
        cairo_context.stroke() 
        cairo_context.move_to(((rectangle_log.width * total_chars)/Pango.SCALE) + left_margin, (rectangle_log.height  *(top) + markup_difference)/Pango.SCALE)
        cairo_context.line_to(((rectangle_log.width * (total_chars+3))/Pango.SCALE) + left_margin, (rectangle_log.height  *(top) + markup_difference)/Pango.SCALE)
        cairo_context.stroke() 
        #Draw grid.
        cairo_context.set_line_cap(cairo.LINE_CAP_SQUARE)
        cairo_context.set_line_width(2)
        cairo_context.set_source_rgb(table_grid_rgb[0], table_grid_rgb[1], table_grid_rgb[2])
        for x in range(rows + 1): 
            cairo_context.move_to(left_margin, (rectangle_log.height * (top + x) + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((rectangle_log.width * total_chars)/Pango.SCALE) + left_margin, (rectangle_log.height  *(top + x) + markup_difference)/Pango.SCALE)
            cairo_context.stroke() 
        for y in range(columns + 1): 
            cairo_context.move_to(((rectangle_log.width * y * column_width)/Pango.SCALE) + left_margin, (rectangle_log.height*top + markup_difference)/Pango.SCALE)
            cairo_context.line_to(((rectangle_log.width * y * column_width)/Pango.SCALE) + left_margin, (rectangle_log.height*bottom + markup_difference)/Pango.SCALE)
            cairo_context.stroke() 

        return number_string
    
    def heatmap_value_rgb(self, data_value, max_value, min_value):
        data_value = float(data_value)
        if(max_value==0 and min_value==0):
            red = 0.5
            green = 0.7
            blue = 1.0
        else:
            percent = ((data_value - min_value)/(max_value - min_value))
            if(percent > 0.75):
                red = 1.0 
                green = 0.0 + (4 * (1 - percent))
                blue = 0.0
            elif(percent <= 0.75 and percent > 0.50):
                red = 1.0 - (4 * (0.75 - percent))
                green = 1.0
                blue = 0.0
            elif(percent <= 0.50 and percent > 0.25):
                red = 0.0 
                green = 1.0
                blue = 0.0 + (4 * (0.5 - percent))
            else:
                red = 0.0
                green = 1.0 - (4 * (0.25 - percent))
                blue = 1.0        
        return red, green, blue
    
    def heatmap_value_bry(self, data_value, max_value, min_value):
        data_value = float(data_value)
        if(max_value==0 and min_value==0):
            red = 0.5
            green = 0.7
            blue = 1.0
        else:
            percent = ((data_value - min_value)/(max_value - min_value))
            if(percent > 0.50):
                red = 1.0 
                green = 1.0 - (2 * (1 - percent))
                blue = 0.0
            else:
                red = 1.0 - (2 * (0.50 - percent))
                green = 0.0
                blue = 0.0 + (2 * (0.50 - percent))       
        return red, green, blue
    
    def heatmap_value_iris(self, data_value, max_value, min_value):
        data_value = float(data_value)
        if(max_value==0 and min_value==0):
            red = 0.5
            green = 0.7
            blue = 1.0
        else:
            percent = ((data_value - min_value)/(max_value - min_value))
            red = 1.0 
            green = 1.0 - (1 - percent)
            blue = 0.0 + (1 - percent)            
        return red, green, blue

    def get_test_data(self, rows, columns, shift_number_left, round_float):
        data_values =  [[0 for x in range(columns)] for x in range(rows)]
        max_value = sys.float_info.min
        min_value = sys.float_info.max
        string_number_shift_left = ""
        string_number_shift_left = "{: <{m}}".format("", m=shift_number_left)
        for x in range(rows):
            for y in range(columns):
                random_number = round((random.random() * 100), round_float)
                data_values[x][y] = str(random_number) + string_number_shift_left
                if(random_number > max_value):
                    max_value = random_number
                if(random_number < min_value):
                    min_value = random_number
        return min_value, max_value, data_values

    def get_test_data_db(self, rows, columns, shift_number_left, round_float):
        data_values =  [[0 for x in range(columns)] for x in range(rows)]
        test_data1 = [[0 for x in range(columns)] for x in range(rows)]
        max_value = sys.float_info.min
        min_value = sys.float_info.max
        string_number_shift_left = ""
        string_number_shift_left = "{: <{m}}".format("", m=shift_number_left)
        for x in range(rows):
            for y in range(columns):
                if(y == 0):
                    test_data1[x][y] = str(x) + string_number_shift_left
                else:
                    random_number = round((random.random() * 100), round_float)
                    test_data1[x][y] = str(random_number) + string_number_shift_left
                    if(random_number > max_value):
                        max_value = random_number
                    if(random_number < min_value):
                       min_value = random_number
        con = lite.connect(":memory:")
        cur = con.cursor()
        cur.execute("SELECT SQLITE_VERSION()")
        data = cur.fetchone()
        print("Python version: " + str(sys.version) + " SQLite version: " + str(data[0]))
        cur.execute("DROP TABLE IF EXISTS PlateData")
        table_string = "CREATE TABLE PlateData( KeyID TEXT"
        for i in range(columns-1):
            table_string = table_string + ", WellData" + str(i) + " TEXT"
        table_string = table_string + ");"
        print(table_string)
        cur.execute(table_string)
        insert_string = "INSERT INTO PlateData VALUES(?"
        for i in range(columns-1):
            insert_string = insert_string + ",?"
        insert_string = insert_string + ");"
        print(insert_string)
        cur.executemany(insert_string, test_data1)
        con.commit()
        select_string = "SELECT KeyID"
        for i in range(columns-1):
            select_string =  select_string + ", WellData" + str(i)
        select_string = select_string + " FROM PlateData;"
        print(select_string)
        cur.execute(select_string)
        data_values = cur.fetchall()
        con.close()
        print("Record Count " + str(len(data_values)) + " Min " + str(min_value) + " Max " + str(max_value))
        return min_value, max_value, data_values

    def get_db_data_for_crosstab(self, rows, columns, tables, sql_string, shift_number_left, round_float):
        data_values =  [[0 for x in range(columns)] for x in range(rows)]
        max_value = sys.float_info.min
        min_value = sys.float_info.max
        string_number_shift_left = ""
        string_number_shift_left = "{: <{m}}".format("", m=shift_number_left)
        top = (self.plate_counter_sql-1) * (rows * columns) + (rows * columns)
        bottom = (self.plate_counter_sql-1) * (rows * columns)
        #Use LIMIT OFFSET instead of WHERE clause. Then you can use WHERE in the UI.
        #select_string = sql_string + " WHERE KeyID <= " + str(top) + " AND KeyID > " + str(bottom) + ";"
        select_string = sql_string + " LIMIT " + str(top-bottom) + " OFFSET " + str(bottom) + ";"
        print(select_string)
        con = lite.connect("VelociRaptorData.db")
        cur = con.cursor()
        cur.execute(select_string)
        data_array = cur.fetchall()
        con.close()
        for x in range(rows):
            for y in range(columns):
                column_type = isinstance(data_array[x*columns+y][0], float)
                if(column_type):
                    temp_value = float(data_array[x*columns+y][0])
                    data_values[x][y] = str(round(data_array[x*columns+y][0],round_float)) + string_number_shift_left
                else:
                    temp_value = data_array[x*columns+y][0]
                    data_values[x][y] = str(data_array[x*columns+y][0]) + string_number_shift_left
                if(temp_value > max_value):
                    max_value = temp_value
                if(temp_value < min_value):
                    min_value = temp_value
        print("Record Count " + str(len(data_array)) + " Min " + str(min_value) + " Max " + str(max_value))
        self.plate_counter_sql+=1
        return min_value, max_value, data_values

    def get_db_data_for_table(self, rows, columns, tables, sql_string, shift_number_left, round_float):
        max_value = 0
        min_value = 0
        string_number_shift_left = ""
        string_number_shift_left = "{: <{m}}".format("", m=shift_number_left)
        top = (self.plate_counter_sql-1) * rows + rows
        bottom = (self.plate_counter_sql-1) * rows
        #Use LIMIT OFFSET instead of WHERE clause. Then you can use WHERE in the UI.
        #select_string = sql_string + " WHERE KeyID <= " + str(top) + " AND KeyID > " + str(bottom) + ";"
        select_string = sql_string + " LIMIT " + str(top-bottom) + " OFFSET " + str(bottom) + ";"
        print(select_string)
        con = lite.connect("VelociRaptorData.db")
        cur = con.cursor()
        cur.execute(select_string)
        column_names = [cn[0] for cn in cur.description]
        column_number = len(column_names)
        columns = column_number
        print("Columns " + str(columns))
        data_array = cur.fetchall()
        con.close()
        data_values =  [[0 for x in range(columns)] for x in range(rows)]
        for x in range(rows):
            for y in range(columns):
                column_type = isinstance(data_array[x][y], float)
                if(column_type):
                    data_values[x][y] = str(round(data_array[x][y],round_float)) + string_number_shift_left
                else:
                    data_values[x][y] = str(data_array[x][y]) + string_number_shift_left
        print("Record Count " + str(len(data_array)) + " Min " + str(min_value) + " Max " + str(max_value))
        self.plate_counter_sql+=1
        return min_value, max_value, data_values, column_names, column_number 

    def set_bold_tag(self, button):
        if(self.textbuffer.get_has_selection()):
            start, end = self.textbuffer.get_selection_bounds()
            self.textbuffer.apply_tag(self.bold_tag, start, end)
        else:
            print("There is no selected text.")

    def set_underline_tag(self, button):
        if(self.textbuffer.get_has_selection()):
            start, end = self.textbuffer.get_selection_bounds()
            self.textbuffer.apply_tag(self.underline_tag, start, end)
        else:
            print("There is no selected text.")

    def set_font_tags(self, combo7):
        combo7_id = int(combo7.get_active_id())
        if(self.textbuffer.get_has_selection()):
            start, end = self.textbuffer.get_selection_bounds()
            #Remove previous font tags.
            self.textbuffer.remove_tag(self.font_tag_8, start, end)
            self.textbuffer.remove_tag(self.font_tag_10, start, end)
            self.textbuffer.remove_tag(self.font_tag_12, start, end)
            self.textbuffer.remove_tag(self.font_tag_14, start, end)
            self.textbuffer.remove_tag(self.font_tag_16, start, end)
            if(combo7_id == 1):
                self.textbuffer.apply_tag(self.font_tag_8, start, end)
            elif(combo7_id == 2):
                self.textbuffer.apply_tag(self.font_tag_10, start, end)
            elif(combo7_id == 3):
                self.textbuffer.apply_tag(self.font_tag_12, start, end)
            elif(combo7_id == 4):
                self.textbuffer.apply_tag(self.font_tag_14, start, end)
            else:
                self.textbuffer.apply_tag(self.font_tag_16, start, end)
              
        else:
            print("There is no selected text.")
        
    def clear_tags(self, button):
        start = self.textbuffer.get_start_iter()
        end = self.textbuffer.get_end_iter()
        self.textbuffer.remove_all_tags(start, end)
        
    
    def get_pango_markup(self):
        tag_table = self.textbuffer.get_tag_table()
        pango_tag_list=[]
        #package pointers together in a list.
        tag_table.foreach(self.get_tags, pango_tag_list)
        self.load_pango_list(pango_tag_list)

    def get_tags(self, tag, pango_tag_list):    
        loop=True
        switch=False
        offset1=0
        offset2=0
        no_tags=False
        tag_start_list=[]
        start = self.textbuffer.get_start_iter()
        if(start.begins_tag(tag)):
            switch = True       
        not_end=start.forward_to_tag_toggle(tag) 
        if(not_end == False):
            no_tags=True      
        while(loop):
            if(not_end):
                offset1=start.get_offset()
                if(offset1 == offset2):
                        break
                if(switch):
                    #print("Tag Found at " + str(offset2) + "-" + str(offset1) + " Tagname " + str(tag.get_property('name')))
                    tag_start_list.append(offset2)
                    pango_tag_list.append(str(tag.get_property('name')))
                    pango_tag_list.append(offset2)
                    pango_tag_list.append(offset1)
                    switch=False
                else:
                    switch=True
                offset2=offset1
                not_end=start.forward_to_tag_toggle(tag)
            else:
                loop=False

    def load_pango_list(self, pango_tag_list): 
        records = int(len(pango_tag_list)/3)
        #print("List Count " + str(records)) 
        start1 = self.textbuffer.get_start_iter()
        end1 = self.textbuffer.get_end_iter()
        text = self.textbuffer.get_text(start1, end1, False)
 
        #Reshape and sort pango_tag_list
        pango_reshape=[]
        pango_sorted=[]
        for i in range(records):
            pango_reshape.append([pango_tag_list[3*i], pango_tag_list[3*i+1], pango_tag_list[3*i+2]])
        #print(pango_reshape)
        pango_sorted = sorted(pango_reshape, key=itemgetter(1))
        #print(pango_sorted) 
        
        #Add the markup. 
        self.markup_string = ""
        open_tags = [False, False, False, False, False, False, False]
        span_open = False
        chars = len(text)+1
        for i in range(chars):
            if any(i in x for x in pango_sorted):
                #print("Found " + str(i))
                for j in range(records):
                    if("underline" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[0]=True
                    if("bold" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[1]=True
                    if("font8" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[2]=True
                    if("font10" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[3]=True
                    if("font12" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[4]=True
                    if("font14" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[5]=True
                    if("font16" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[6]=True
                    if("underline" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i):
                        open_tags[0]=False
                    if("bold" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[1]=False
                    if("font8" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[2]=False 
                    if("font10" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[3]=False 
                    if("font12" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[4]=False 
                    if("font14" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[5]=False 
                    if("font16" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[6]=False 
                if(span_open):
                    self.markup_string+="</span>"
                    span_open = False
                #Check for open tags and build string.
                if(open_tags[0] or open_tags[1] or open_tags[2] or open_tags[3] or open_tags[4] or open_tags[5] or open_tags[6]):
                    self.markup_string+="<span"
                    for k in range(len(open_tags)):  
                        if(open_tags[k] and k == 0):
                            self.markup_string+=" underline='single'"
                        if(open_tags[k] and k == 1):
                            self.markup_string+=" weight='900'"
                        if(open_tags[k] and k == 2):
                            self.markup_string+=" font='8'"
                        if(open_tags[k] and k == 3):
                            self.markup_string+=" font='10'"
                        if(open_tags[k] and k == 4):
                            self.markup_string+=" font='12'"
                        if(open_tags[k] and k == 5):
                            self.markup_string+=" font='14'"
                        if(open_tags[k] and k == 6):
                            self.markup_string+=" font='16'"
                    self.markup_string+=">" 
                    span_open = True        
            if(i < chars-1):
                self.markup_string+=str(text[i])
        #print(self.markup_string)

class LabelsDialog(Gtk.Dialog):
    def __init__(self, parent, rows, columns):
        Gtk.Dialog.__init__(self, "Set Labels", parent, 0, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OK, Gtk.ResponseType.OK))
        content_area = self.get_content_area()
        self.set_border_width(10)
        self.active_row = 0
        self.active_column = 0
        self.rows1 = rows
        self.columns1 = columns
 
        row_label = Gtk.Label("Set Row Labels")
        self.row_combo = Gtk.ComboBoxText.new_with_entry()
        for row in range(rows):
            self.row_combo.append(str(row), "row" + str(row+1))
        self.row_changed = self.row_combo.connect("changed", self.row_combo_changed)
        self.row_combo.set_entry_text_column(0)
        self.row_combo.set_active(0)

        column_label = Gtk.Label("Set Column Labels")
        self.column_combo = Gtk.ComboBoxText.new_with_entry()
        for column in range(columns):
            self.column_combo.append(str(column), "column" + str(column+1))
        self.column_changed = self.column_combo.connect("changed", self.column_combo_changed)
        self.column_combo.set_entry_text_column(0)
        self.column_combo.set_active(0)

        standard_label = Gtk.Label("Standard Formats")
        self.standard_combo = Gtk.ComboBoxText()
        self.standard_combo.append("1", "RowsColumns")
        self.standard_combo.append("2", "Numbers")
        self.standard_combo.append("3", "Microtiter")
        self.standard_combo.set_active_id("1")
        self.standard_combo.connect("changed", self.change_standard_labels)

        focus_button = self.get_widget_for_response(Gtk.ResponseType.OK)
        focus_button.grab_focus()
        focus_button.connect("clicked", self.load_labels) 

        grid = Gtk.Grid()
        grid.set_row_spacing(5)
        grid.attach(row_label, 0, 0, 1, 1)
        grid.attach(self.row_combo, 0, 1, 1, 1)
        grid.attach(column_label, 0, 2, 1, 1)
        grid.attach(self.column_combo, 0, 3, 1, 1)
        grid.attach(standard_label, 0, 4, 1, 1)
        grid.attach(self.standard_combo, 0, 5, 1, 1)
        content_area.add(grid)
        self.show_all()

    def row_combo_changed(self, combo):
        text = combo.get_active_text()
        text_id = combo.get_active()
        if(text_id!=-1):
            self.active_row = text_id
        if(text != None):
            #print(text + " " + str(self.active_row))
            combo.handler_block(self.row_changed)
            combo.insert_text(self.active_row, text)
            combo.remove(self.active_row+1)
            combo.handler_unblock(self.row_changed)

    def column_combo_changed(self, combo):
        text = combo.get_active_text()
        text_id = combo.get_active()
        if(text_id!=-1):
            self.active_column = text_id
        if(text != None):
            #print(text + " " + str(self.active_column))
            combo.handler_block(self.column_changed)
            combo.insert_text(self.active_column, text)
            combo.remove(self.active_column+1)
            combo.handler_unblock(self.column_changed)

    def load_labels(self, button):
        print("Load Labels")
        if(g_row_labels):
            del g_row_labels[:]
        model1 = self.row_combo.get_model()
        iter1 = model1.get_iter_first()
        while(iter1):
	    g_row_labels.append(model1[iter1][0])
	    iter1 = model1.iter_next(iter1)
        print(g_row_labels)
        if(g_column_labels):
            del g_column_labels[:]
        model2 = self.column_combo.get_model()
        iter2 = model2.get_iter_first()
        while(iter2):
	    g_column_labels.append(model2[iter2][0])
	    iter2 = model2.iter_next(iter2)
        print(g_column_labels)

    def change_standard_labels(self, combo):
        print("Change Standard Labels")
        combo_id = int(combo.get_active_id())
        if(combo_id==1):
            self.row_combo.handler_block(self.row_changed)
            self.row_combo.remove_all()
            for row in range(self.rows1):
                self.row_combo.append(str(row), "row" + str(row+1))
            self.row_combo.handler_unblock(self.row_changed)
            self.row_combo.set_active(0)
            self.column_combo.handler_block(self.column_changed)
            self.column_combo.remove_all()
            for column in range(self.columns1):
                self.column_combo.append(str(column), "column" + str(column+1))
            self.column_combo.handler_unblock(self.column_changed)
            self.column_combo.set_active(0)
        if(combo_id==2):
            self.row_combo.handler_block(self.row_changed)
            self.row_combo.remove_all()
            for row in range(self.rows1):
                self.row_combo.append(str(row), str(row+1))
            self.row_combo.handler_unblock(self.row_changed)
            self.row_combo.set_active(0)
            self.column_combo.handler_block(self.column_changed)
            self.column_combo.remove_all()
            for column in range(self.columns1):
                self.column_combo.append(str(column), str(column+1))
            self.column_combo.handler_unblock(self.column_changed)
            self.column_combo.set_active(0)
        if(combo_id==3):
            micro_labels = self.get_letters(self.rows1)
            self.row_combo.handler_block(self.row_changed)
            self.row_combo.remove_all()
            for row in range(self.rows1):
                self.row_combo.append(str(row), micro_labels[row])
            self.row_combo.handler_unblock(self.row_changed)
            self.row_combo.set_active(0)
            self.column_combo.handler_block(self.column_changed)
            self.column_combo.remove_all()
            for column in range(self.columns1):
                self.column_combo.append(str(column), str(column+1))
            self.column_combo.handler_unblock(self.column_changed)
            self.column_combo.set_active(0)
        
    def get_letters(self, count):
        letters = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z']
        counter = 0
        result = []
        for i in range(1, 3):
          string = [letters] * i
          for y in product(*string):
            result.append(''.join(y))
            counter+=1
            if(counter == count):
                return result

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Report Generator")
        self.set_default_size(750,550)
        self.set_border_width(20)
        self.row_value = 0
        self.column_value = 0
        self.menubar1 = Gtk.MenuBar()  
        self.menu1 = Gtk.Menu()
        self.menuitem1 = Gtk.MenuItem("About") 
        self.menuitem1.set_submenu(self.menu1)
        self.menuitem2 = Gtk.MenuItem("Report Generator")
        self.menuitem2.set_name("menuitem2")
        self.menuitem2.connect("activate", self.about_dialog)         
        self.menu1.append(self.menuitem2) 
        self.menubar1.append(self.menuitem1)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.label0 = Gtk.Label()
        self.label0.set_markup("<b>Draw Grids, Crosstabs and Tables</b>")
        self.label0.set_hexpand(True)
        self.label0.set_halign(Gtk.Align.CENTER)
        self.label1 = Gtk.Label("Rows")
        self.entry1 = Gtk.Entry()
        self.entry1.set_halign(Gtk.Align.START)
        self.entry1.set_width_chars(3)
        self.entry1.set_text("10")
        self.entry1.connect("focus-in-event", self.save_current_row_value)
        self.entry1.connect("focus-out-event", self.clear_row_labels)
        self.label2 = Gtk.Label("Columns")
        self.entry2 = Gtk.Entry()
        self.entry2.set_halign(Gtk.Align.START)
        self.entry2.set_width_chars(3)
        self.entry2.set_text("5")
        self.entry2.connect("focus-in-event", self.save_current_column_value)
        self.entry2.connect("focus-out-event", self.clear_column_labels)
        self.entry2.connect("button_press_event", self.change_label_color)
        self.label3 = Gtk.Label("Shift Right")
        self.entry3 = Gtk.Entry()
        self.entry3.set_halign(Gtk.Align.START)
        self.entry3.set_width_chars(3)
        self.entry3.set_text("10")
        self.label4 = Gtk.Label("Shift Down")
        self.entry4 = Gtk.Entry()
        self.entry4.set_halign(Gtk.Align.START)
        self.entry4.set_text("3")
        self.entry4.set_width_chars(3)
        self.label5 = Gtk.Label("Column Width")
        self.entry5 = Gtk.Entry()
        self.entry5.set_text("10")
        self.entry5.set_halign(Gtk.Align.START)
        self.entry5.set_width_chars(3)
        self.label6 = Gtk.Label("Pad Number")
        self.entry6 = Gtk.Entry()
        self.entry6.set_text("1")
        self.entry6.set_halign(Gtk.Align.START)
        self.entry6.set_width_chars(3)
        self.label7 = Gtk.Label("Pad Column")
        self.entry7 = Gtk.Entry()
        self.entry7.set_text("1")
        self.entry7.set_halign(Gtk.Align.START)
        self.entry7.set_width_chars(3)
        self.label8 = Gtk.Label("Tables")
        self.entry8 = Gtk.Entry()
        self.entry8.set_text("5")
        self.entry8.set_halign(Gtk.Align.START)
        self.entry8.set_width_chars(3)
        self.label9 = Gtk.Label("Table Label")
        self.entry9 = Gtk.Entry()
        self.entry9.set_text("Plate ")
        self.entry10 = Gtk.Entry()
        self.entry10.set_text("SELECT Percent FROM Data")
        self.entry10.set_sensitive(False)
        self.label11 = Gtk.Label("Round Floats")
        self.entry11 = Gtk.Entry()
        self.entry11.set_halign(Gtk.Align.START)
        self.entry11.set_text("3")
        self.entry11.set_width_chars(3)
        self.label12 = Gtk.Label("Data Source")
        self.check1 = Gtk.CheckButton("Add Table Label")
        self.button1 = Gtk.Button("  Print Dialog  ")
        self.button1.set_hexpand(False)
        self.button1.set_halign(Gtk.Align.CENTER)
        self.button1.connect("clicked", self.print_dialog)
        self.button2 = Gtk.Button("Bold")
        self.button2.set_hexpand(False)
        self.button2.connect("clicked", self.bold_font)
        self.button3 = Gtk.Button("Underline")
        self.button3.set_hexpand(False)
        self.button3.connect("clicked", self.underline_font)
        self.button4 = Gtk.Button("Clear")
        self.button4.set_hexpand(False)
        self.button4.connect("clicked", self.clear_tags)
        self.button5 = Gtk.Button("Set Labels")
        self.button5.connect("clicked", self.labels_dialog)
        self.combo1 = Gtk.ComboBoxText()
        self.combo1.append("1", "White")
        self.combo1.append("2", "Blue")
        self.combo1.append("3", "RGB")
        self.combo1.append("4", "BRY")
        self.combo1.append("5", "Iris")
        self.combo1.set_active_id("1")
        self.combo2 = Gtk.ComboBoxText()
        self.combo2.append("1", "No Labels")
        self.combo2.append("2", "Tabular")
        self.combo2.append("3", "Crosstab")
        self.combo2.set_active_id("1")
        self.combo3 = Gtk.ComboBoxText()
        self.combo3.append("1", "Monospace 8")
        self.combo3.append("2", "Monospace 10")
        self.combo3.append("3", "Monospace 12")
        self.combo3.append("4", "Monospace 14")
        self.combo3.append("5", "Monospace 16")
        self.combo3.set_active_id("3")
        self.combo3.connect("changed", self.change_font)
        self.combo4 = Gtk.ComboBoxText()
        self.combo4.append("1", "Random")
        self.combo4.append("2", "RandomDB")
        self.combo4.append("3", "CrosstabFromDB")
        self.combo4.append("4", "TableFromDB")
        self.combo4.set_active_id("1")
        self.combo4.connect("changed", self.change_sql_entry)
        self.combo5 = Gtk.ComboBoxText()
        self.combo5.append("1", "No Frame")
        self.combo5.append("2", "Black Frame")
        self.combo5.append("3", "Red Frame")
        self.combo5.append("4", "Green Frame")
        self.combo5.append("5", "Blue Frame")
        self.combo5.set_active_id("2")
        self.combo6 = Gtk.ComboBoxText()
        self.combo6.append("1", "Nudge 1")
        self.combo6.append("2", "Nudge 2")
        self.combo6.append("3", "Nudge 3")
        self.combo6.append("4", "Nudge 4")
        self.combo6.append("5", "Nudge 5")
        self.combo6.set_active_id("2")
        self.combo7 = Gtk.ComboBoxText()
        self.combo7.append("1", "Font 8")
        self.combo7.append("2", "Font 10")
        self.combo7.append("3", "Font 12")
        self.combo7.append("4", "Font 14")
        self.combo7.append("5", "Font 16")
        self.combo7.set_active_id("3")
        self.combo7.connect("changed", self.change_font_size)
        self.grid = Gtk.Grid()
        self.grid.set_row_spacing(10)
        self.grid.set_column_spacing(5)
        self.grid.attach(self.scrolledwindow, 0, 0, 5, 4)
        self.grid.attach(self.button2, 5, 0, 1, 1)
        self.grid.attach(self.button3, 5, 1, 1, 1)
        self.grid.attach(self.combo7, 5, 2, 1, 1)
        self.grid.attach(self.button4, 5, 3, 1, 1)
        self.grid.attach(self.label0, 0, 4, 6, 1)
        self.grid.attach(self.label1, 0, 5, 1, 1)
        self.grid.attach(self.entry1, 1, 5, 1, 1)
        self.grid.attach(self.label2, 0, 6, 1, 1)
        self.grid.attach(self.entry2, 1, 6, 1, 1)
        self.grid.attach(self.label8, 0, 7, 1, 1)
        self.grid.attach(self.entry8, 1, 7, 1, 1)
        self.grid.attach(self.combo3, 0, 8, 1, 1)
        self.grid.attach(self.label3, 2, 5, 1, 1)
        self.grid.attach(self.entry3, 3, 5, 1, 1)
        self.grid.attach(self.label4, 2, 6, 1, 1)
        self.grid.attach(self.entry4, 3, 6, 1, 1)
        self.grid.attach(self.label5, 2, 7, 1, 1)
        self.grid.attach(self.entry5, 3, 7, 1, 1)
        self.grid.attach(self.combo1, 1, 8, 1, 1)
        self.grid.attach(self.label9, 0, 9, 1, 1)
        self.grid.attach(self.entry9, 1, 9, 2, 1)
        self.grid.attach(self.label12, 0, 10, 1, 1)
        self.grid.attach(self.label11, 4, 5, 1, 1)
        self.grid.attach(self.entry11, 5, 5, 1, 1)
        self.grid.attach(self.label6, 4, 6, 1, 1)
        self.grid.attach(self.entry6, 5, 6, 1, 1)
        self.grid.attach(self.label7, 4, 7, 1, 1)
        self.grid.attach(self.entry7, 5, 7, 1, 1)       
        self.grid.attach(self.combo2, 2, 8, 1, 1)
        self.grid.attach(self.button5, 3, 8, 1, 1)
        self.grid.attach(self.combo5, 4, 8, 1, 1)
        self.grid.attach(self.combo6, 5, 8, 1, 1)
        self.grid.attach(self.check1, 3, 9, 2, 1)
        self.grid.attach(self.combo4, 1, 10, 2, 1)
        self.grid.attach(self.entry10, 0, 11, 6, 1)
        self.grid.attach(self.button1, 0, 12, 6, 1)
        self.grid.attach(self.menubar1, 1, 13, 1, 1)
        self.add(self.grid)
        style_provider = Gtk.CssProvider()
        css = "GtkWindow{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(0,255,0,0.5)), color-stop(0.5,rgba(180,180,180,0.5)), color-stop(1.0,rgba(255,0,255,0.5)));}GtkButton{background: rgba(220,220,220,0.5);}"
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

    def print_dialog(self, button1):
        #Check entries.
        return_value = self.validate_entries()
        if(return_value==0):
            entries_array = (self.entry1, self.entry2, self.entry3, self.entry4, self.entry5, self.entry6, self.entry7, self.entry8, self.entry9, self.entry10, self.entry11, self.check1, self.combo1, self.combo2, self.combo3, self.combo4, self.combo5, self.combo6)
            self.TextBox1.print_dialog(entries_array)

    def bold_font(self, button2):
        self.TextBox1.set_bold_tag(button2)

    def underline_font(self, button3):
        self.TextBox1.set_underline_tag(button3)

    def clear_tags(self, button4):
        self.TextBox1.clear_tags(button4)

    def change_font(self, combo3):
        self.TextBox1.change_textview_font(self.combo3)

    def change_font_size(self, combo7):
        self.TextBox1.set_font_tags(combo7)

    def change_label_color(self, label2, event):
        self.label2.set_text("Columns")
        return False

    def change_sql_entry(self, combo4):
        active_id = int(combo4.get_active_id())
        if(active_id==1 or active_id==2):
            self.entry10.set_sensitive(False)
        else:
            self.entry10.set_sensitive(True)
    
    def validate_entry(self, entry):
        try:
            e1 = int(entry.get_text())
        except ValueError:
            e1 = -1
        return e1
    
    def validate_entries(self):
        e1 = self.validate_entry(self.entry1)
        e2 = self.validate_entry(self.entry2)
        e3 = self.validate_entry(self.entry3)
        e4 = self.validate_entry(self.entry4)
        e5 = self.validate_entry(self.entry5)
        e6 = self.validate_entry(self.entry6)
        e7 = self.validate_entry(self.entry7)
        e8 = self.validate_entry(self.entry8)
        e11 = self.validate_entry(self.entry11)
        if(0 >= e1 or e1 > 50):           
            message = "Rows " + self.entry1.get_text() + ", Range 0<rows<=50"
            self.message_dialog(message)
            return 1
        elif(0 >= e2 or e2 > 20):
            message = "Columns " + self.entry2.get_text() + ", Range 0<columns<=20"
            self.message_dialog(message)
            return 1
        elif(0 > e3 or e3 > 30):
            message = "Shift Right " + self.entry3.get_text() + ", Range 0<=Shift Right<=30"
            self.message_dialog(message)
            return 1
        elif(1 > e4 or e4 > 10):
            message = "Shift Down " + self.entry4.get_text() +", Range 1<=Shift Down<=10"
            self.message_dialog(message)
            return 1
        elif(5 > e5 or e5 > 20):
            message = "Column Width " + self.entry5.get_text() + ", Range 5<=Column Width<=20"
            self.message_dialog(message)
            return 1
        elif(0 > e6 or e6 > 5):
            message = "Pad Number " + self.entry6.get_text() + ", Range 0<=Pad Number<=5"
            self.message_dialog(message)
            return 1
        elif(0 > e7 or e7 > 5):
            message = "Pad Column " + self.entry7.get_text() + ", Range 0<=Pad Column<=5"
            self.message_dialog(message)
            return 1
        elif(1 > e8 or e8 > 20):
            message = "Tables " + self.entry8.get_text() + ", Range 1<=Tables<=20"
            self.message_dialog(message)
            return 1
        elif(1 > e11 or e11 > 7):
            message = "Round Floats " + self.entry11.get_text() + ", Range 1<=Tables<=7"
            self.message_dialog(message)
            return 1
        elif(self.entry10.get_sensitive()):
            database_rows = self.check_sql_string(self.entry10.get_text())
            numbers = int(self.entry1.get_text())*int(self.entry2.get_text())*int(self.entry8.get_text())
            print("Numbers " + str(numbers))
            if(database_rows < numbers and database_rows > 0):
                message = "There are not enough rows in the database table\nto print the requested rows, columns and tables.\nDatabase rows " + str(database_rows) + " Requested " + str(numbers)
                self.message_dialog(message)
                return 1
            elif(database_rows == 0):
                message = "The SQL statement isn't valid."
                self.message_dialog(message)
                return 1
            else:
                return 0
        else:
            return 0

    def about_dialog(self, menu):
        about = Gtk.AboutDialog()
        about.set_program_name("Report Generator")
        about.set_version("Test Version 1.0")
        about.set_copyright("(C) 2015 C. Eric Cashon")
        about.set_comments("A report generator for the Ordered Set VelociRaptor program.")
        try:
            about.set_logo(GdkPixbuf.Pixbuf.new_from_file("dino.svg"))
        except GLib.GError:
            print("Couldn't find dino.svg.")
        about.run()
        about.destroy()

    def message_dialog(self, message):
        dialog = Gtk.MessageDialog(self, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, message)
        dialog.run()
        dialog.destroy()

    def check_sql_string(self, sql_string):
        #Build a row count SELECT statement.
        valid_string = True
        ret_val = 0
        sql1 = "SELECT count(*) "
        match1 = re.search("(?i)(FROM).*", sql_string)
        if(match1):
            print(sql1 + match1.group(0))
            select_rows = sql1 + match1.group(0)
        #Count the number of columns in the SELECT statement and change UI columns.
        match2 = re.search("(?i)(?<=\SELECT)(.*?)(?=\FROM)", sql_string)
        if(match2):
            words = match2.group(0).split(",")
            print("Columns in SELECT " + str(len(words)))
            if(int(self.combo4.get_active_id())==4):
                if(int(self.entry2.get_text()) != len(words)):
                    self.entry2.set_text(str(len(words)))
                    self.label2.set_markup("<span foreground='red'>Columns Changed</span>")
        #Check database for necessary rows.
        if(match1):
            con = lite.connect("VelociRaptorData.db")
            cur = con.cursor()
            #Check if sql_string is valid.
            try:
                cur.execute(sql_string)
            except lite.OperationalError: 
                valid_string = False
            #If valid check how many row there are.
            if(valid_string):
                cur.execute(select_rows)           
                records = cur.fetchone()
                ret_val = records[0]
            else:
                ret_val = 0
            con.close()
        print("Database records " + str(ret_val))
        return ret_val

    def labels_dialog(self, button):
        rows = int(self.entry1.get_text())
        columns = int(self.entry2.get_text())
        #set some boundries.
        if(rows > 0 and rows < 100 and columns > 0 and columns < 20):
            dialog = LabelsDialog(self, rows, columns)
            response = dialog.run()        
            dialog.destroy()
        else:
            print("0<rows<100 and 0<columns<20")

    def save_current_row_value(self, event, widget):
        print("Save Row Value")
        self.row_value = int(self.entry1.get_text())

    def clear_row_labels(self, event, widget):
        r_value = int(self.entry1.get_text())
        if(g_row_labels and r_value!=self.row_value):
            print("Clear Row Labels")
            del g_row_labels[:]
            print(g_row_labels)
        return False

    def save_current_column_value(self, event, widget):
        print("Save Column Value")
        self.column_value = int(self.entry2.get_text())

    def clear_column_labels(self, event, widget):
        c_value = int(self.entry2.get_text())
        if(g_column_labels and c_value!=self.column_value):
            print("Clear Column Labels")
            del g_column_labels[:]
            print(g_column_labels)
        return False

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()



