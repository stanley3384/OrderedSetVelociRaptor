#!/user/bin/python

"""
   Test some Python and GTK+ printing. Print size defaults to "letter" or 8.5 x 11 sized layout.
   Look at some layout values for positioning fonts and graphics. Work towards making a report
   generator.

   Python 2.7 with GTK 3.10 on Ubuntu 14.04.

   C. Eric Cashon
"""

from gi.repository import Gtk, Pango, PangoCairo
import random
import math
import sys

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
        self.set_wrap_mode(0)
        self.set_cursor_visible(True)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("This is the title for the report.")

    def change_textview_font(self, combo3):
        font = combo3.get_active_text()
        font_description = Pango.FontDescription(font) 
        self.override_font(font_description);
        
    def get_line(self, line_number):
        start_line = self.textbuffer.get_iter_at_line(line_number)
        end_line = self.textbuffer.get_iter_at_line(line_number)
        self.forward_display_line_end(end_line)
        #print(self.textbuffer.get_text(start_line, end_line, False))
        string = self.textbuffer.get_text(start_line, end_line, False)
        return string

    def print_dialog(self, entries_array):
        self.entries_array_text = entries_array
        operation = Gtk.PrintOperation()
        #Figure out number of pages.
        count_lines = self.textbuffer.get_line_count()
        #Float tables value to force floating point calculation.
        tables = float(self.entries_array_text[7].get_text())
        rows = int(self.entries_array_text[0].get_text())
        shift_below_text = int(self.entries_array_text[3].get_text())
        font_size_id = int(self.entries_array_text[12].get_active_id())-1       
        #Lines per page for different font sizes.
        font_lines = [78, 63, 52, 45, 39]
        lines_per_page = font_lines[font_size_id]
        total_lines = count_lines + ((shift_below_text-1)*tables) + (tables * rows)
        tables_per_page = int((lines_per_page-count_lines)/(rows + shift_below_text - 1)) 
        if(tables_per_page == 1):
            total_lines = tables*lines_per_page
        pages = math.ceil(tables/tables_per_page)
        operation.set_n_pages(pages)
        operation.connect("begin_print", self.begin_print)
        operation.connect("draw_page", self.draw_page)
        result = operation.run(Gtk.PrintOperationAction.PRINT_DIALOG, None)

    def begin_print(self, operation, gtk_context):
        self.plate_counter = 1
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
        #print("PageWidth " + str(self.page_width) + " PageHeight " + str(self.page_height) + " TextWidth " + str(self.text_width) + " TextHeight " + str(self.text_height))
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
        total_lines = count_lines + ((shift_below_text-1)*tables) + (tables * rows)
        font_size_id = int(self.entries_array_text[12].get_active_id())-1       
        #Lines per page for different font sizes.
        font_lines = [78, 63, 52, 45, 39]
        lines_per_page = font_lines[font_size_id]
        tables_per_page = int((lines_per_page-count_lines)/(rows + shift_below_text - 1)) 
        if(tables_per_page == 1):
            total_lines = tables*lines_per_page
        pages = math.ceil(tables/tables_per_page)
        print("Total Lines " + str(total_lines)) 
        print("Tables per Page " + str(tables_per_page))
        if(page_number==pages-1 and page_number > 0):
            if(tables_per_page!=1):
                if(int(tables)%int(tables_per_page)==0):
                    tables_on_page = tables_per_page
                else:
                    tables_on_page = int(tables)%(int(tables_per_page))
            else:
                tables_on_page = 1
        else:
            if(tables < tables_per_page):
               tables_on_page = int(tables)
            else:
                tables_on_page = tables_per_page
        #print("Tables Lines " + str(tables_on_page) + " " + str(total_lines))
        cairo_context = gtk_context.get_cairo_context()
        #Color some different rectangles on different pages.
        if(page_number%2):
            cairo_context.set_source_rgb(1.0, 0.0, 1.0)
        else:
            cairo_context.set_source_rgb(1.0, 1.0, 0.0)
        cairo_context.rectangle(0, 0, self.page_width, self.page_height)
        cairo_context.stroke()
        table_string = ""
        for table in range(tables_on_page):
            table_string = table_string + self.draw_tables(operation, gtk_context, page_number, cairo_context, table)
        #Set text and tables
        cairo_context.set_source_rgb(0.0, 0.0, 0.0)
        if(page_number == 0):
            string = self.get_line(0)
        else:
            string = ""
        self.pango_layout.set_markup(string + table_string)
        PangoCairo.show_layout(cairo_context, self.pango_layout)

    def draw_tables(self, operation, gtk_context, page_number, cairo_context, table):
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
        check1_active = self.entries_array_text[9].get_active()
        combo1_index = int(self.entries_array_text[10].get_active_id())
        combo2_index = int(self.entries_array_text[11].get_active_id())

        #Shift tables for multiple tables.
        shift_below_text2 = shift_below_text + (table * (rows + shift_below_text -1))
        
        #Get some test numbers to add to the string.
        data_values =  [[0 for x in range(columns)] for x in range(rows)]
        max_value = 0
        min_value = 100 
        string_number_shift_left = ""
        string_number_shift_left = "{: <{m}}".format("", m=shift_number_left)
        for x in range(rows):
            for y in range(columns):
                random_number = round((random.random() * 100), 3)
                data_values[x][y] = str(random_number) + string_number_shift_left
                if(random_number > max_value):
                    max_value = random_number
                if(random_number < min_value):
                    min_value = random_number

        #Create label values.
        vertical_labels = []
        horizontal_labels = []
        string_column_shift_left = ""
        string_column_shift_left = "{: <{m}}".format("", m=shift_column_left)
        for x in range(rows):
            vertical_labels.insert(x, "Row" + str(x))
        for y in range(columns):
            horizontal_labels.insert(y, "Column" + str(y) + string_column_shift_left)

        #Get max length and print to screen.
        max_length = 0
        max_vertical_label = 0
        for x in range(rows):
            if(max_vertical_label < len(str(vertical_labels[x]))):
                max_vertical_label = len(str(vertical_labels[x]))
            for y in range(columns):
                if(max_length < len(str(data_values[x][y]))):
                    max_length = len(str(data_values[x][y]))
                #sys.stdout.write(str(data_values[x][y]) + " ")
            #sys.stdout.write("\n")

        #Get rectangle for one monospace char for sizing
        self.pango_layout.set_markup("2")
        rectangle_ink, rectangle_log = self.pango_layout.get_extents()
        #print(" Rectangle1 " + str(rectangle_ink.height) + " Rectangle1 " + str(rectangle_ink.width) + " Rectangle2 " + str(rectangle_log.height) + " Rectangle2 " + str(rectangle_log.width))

        #Get lines of text to be printed on the page.
        string = "Test Line Count String."
        self.pango_layout.set_markup(string)
        line_count = self.pango_layout.get_line_count()
        lines_per_page = int(self.text_height / rectangle_log.height)
        print("Line Count " + str(line_count) + " Lines Per Page " + str(lines_per_page))
        
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
        top = line_count + shift_below_text2 - 2
        bottom = top + rows
        cairo_context.set_source_rgb(0.8, 0.8, 0.8)
        if(combo2_index==3):
            cairo_context.rectangle(((shift_margin) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top))/Pango.SCALE, (rectangle_log.width/Pango.SCALE)*max_vertical_label, (rectangle_log.height*(rows+1))/Pango.SCALE)
            cairo_context.fill()
            cairo_context.stroke()
            #Draw lines for rectangle.
            #Vertical left.
            #cairo_context.set_source_rgb(0.0, 0.0, 0.0)
            cairo_context.move_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top))/Pango.SCALE)
            cairo_context.line_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(bottom+1))/Pango.SCALE)
            cairo_context.stroke() 
            #Short top horizontal.
            cairo_context.move_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top))/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top))/Pango.SCALE)
            cairo_context.stroke()  
            #Short bottom horizontal
            cairo_context.move_to(((shift_margin)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (bottom+1))/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(bottom+1))/Pango.SCALE)
            cairo_context.stroke()  

        #Draw horizonal label rectangle for both crosstab and tabular data.
        top = line_count + shift_below_text2 - 2
        cairo_context.set_source_rgb(0.8, 0.8, 0.8)
        if(combo2_index==1 or combo2_index==2):
            max_vertical_label = 0
        if(combo2_index==2 or combo2_index==3):
            cairo_context.rectangle(((shift_margin+max_vertical_label) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top))/Pango.SCALE, ((rectangle_log.width*columns*column_width)/Pango.SCALE), (rectangle_log.height)/Pango.SCALE)
            cairo_context.fill()
            cairo_context.stroke()
            #Draw lines for rectangle.
            #Top horizontal
            #cairo_context.set_source_rgb(0.0, 0.0, 0.0)
            cairo_context.move_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top))/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label+(column_width*columns)) *rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top))/Pango.SCALE)
            cairo_context.stroke() 
            #Short top vertical left.
            cairo_context.move_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top))/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label)*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top+1))/Pango.SCALE)
            cairo_context.stroke() 
           #Short top vertical right.
            cairo_context.move_to(((shift_margin+max_vertical_label+(column_width*columns))*rectangle_log.width)/Pango.SCALE, (rectangle_log.height* (top))/Pango.SCALE)
            cairo_context.line_to(((shift_margin+max_vertical_label+(column_width*columns))*rectangle_log.width)/Pango.SCALE, (rectangle_log.height*(top+1))/Pango.SCALE)
            cairo_context.stroke() 

        #Background color for each cell.
        top = line_count + shift_below_text2 -1
        shift_margin = shift_margin + max_vertical_label
        if(combo1_index != 1):
            for x in range(rows):
                for y in range(columns):
                    if(combo1_index==2):
                        if(x%2):
                            cairo_context.set_source_rgb(0.5, 0.7, 1.0)
                        else:
                            cairo_context.set_source_rgb(0.7, 1.0, 1.0)
                    else:
                        red, green, blue = self.heatmap_value(data_values[x][y], max_value, min_value)
                        cairo_context.set_source_rgb(red, green, blue) 
                    cairo_context.rectangle(((shift_margin +(column_width*y)) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top + x))/Pango.SCALE, (rectangle_log.width/Pango.SCALE)*column_width, rectangle_log.height/Pango.SCALE)
                    cairo_context.fill()
                    cairo_context.stroke()

        #Table grid for test numbers.
        cairo_context.set_source_rgb(0.8, 0.8, 0.8)
        top = line_count + shift_below_text2 -1
        bottom = top + rows
        total_chars = column_width * columns
        left_margin = (shift_margin * rectangle_log.width)/Pango.SCALE
        for x in range(rows + 1): 
            cairo_context.move_to(left_margin, (rectangle_log.height * (top + x))/Pango.SCALE)
            cairo_context.line_to(((rectangle_log.width * total_chars)/Pango.SCALE) + left_margin, (rectangle_log.height  *(top + x))/Pango.SCALE)
            cairo_context.stroke() 
        for y in range(columns + 1): 
            cairo_context.move_to(((rectangle_log.width * y * column_width)/Pango.SCALE) + left_margin, (rectangle_log.height*top)/Pango.SCALE)
            cairo_context.line_to(((rectangle_log.width * y * column_width)/Pango.SCALE) + left_margin, (rectangle_log.height*bottom)/Pango.SCALE)
            cairo_context.stroke() 

        return number_string

    def heatmap_value(self, data_value, max_value, min_value):
        data_value = float(data_value.rstrip("*"))
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

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Report Generator")
        self.set_default_size(400,500)
        self.set_border_width(10)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.label0 = Gtk.Label("Draw Test Table")
        self.label0.set_halign(Gtk.Align.START)
        self.label1 = Gtk.Label("Rows")
        self.entry1 = Gtk.Entry()
        self.entry1.set_width_chars(3)
        self.entry1.set_text("10")
        self.label2 = Gtk.Label("Columns")
        self.entry2 = Gtk.Entry()
        self.entry2.set_width_chars(3)
        self.entry2.set_text("5")
        self.label3 = Gtk.Label("Shift Right")
        self.entry3 = Gtk.Entry()
        self.entry3.set_width_chars(3)
        self.entry3.set_text("10")
        self.label4 = Gtk.Label("Shift Down")
        self.entry4 = Gtk.Entry()
        self.entry4.set_text("3")
        self.entry4.set_width_chars(3)
        self.label5 = Gtk.Label("Column Width")
        self.entry5 = Gtk.Entry()
        self.entry5.set_text("10")
        self.entry5.set_width_chars(3)
        self.label6 = Gtk.Label("Pad Number")
        self.entry6 = Gtk.Entry()
        self.entry6.set_text("1")
        self.entry6.set_width_chars(3)
        self.label7 = Gtk.Label("Pad Column")
        self.entry7 = Gtk.Entry()
        self.entry7.set_text("1")
        self.entry7.set_width_chars(3)
        self.label8 = Gtk.Label("Tables")
        self.entry8 = Gtk.Entry()
        self.entry8.set_text("5")
        self.entry8.set_width_chars(3)
        self.label9 = Gtk.Label("Table Label")
        self.entry9 = Gtk.Entry()
        self.entry9.set_text("Plate ")
        self.check1 = Gtk.CheckButton("Add Table Label")
        self.button1 = Gtk.Button("Print Dialog")
        self.button1.set_hexpand(True)
        self.button1.set_halign(Gtk.Align.START)
        self.button1.connect("clicked", self.print_dialog)
        self.combo1 = Gtk.ComboBoxText()
        self.combo1.append("1", "White")
        self.combo1.append("2", "Blue")
        self.combo1.append("3", "RGB")
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
        self.grid = Gtk.Grid()
        self.grid.set_row_spacing(10)
        self.grid.attach(self.scrolledwindow, 0, 0, 4, 4)
        self.grid.attach(self.label0, 1, 4, 2, 1)
        self.grid.attach(self.label1, 0, 5, 1, 1)
        self.grid.attach(self.entry1, 1, 5, 1, 1)
        self.grid.attach(self.label2, 0, 6, 1, 1)
        self.grid.attach(self.entry2, 1, 6, 1, 1)
        self.grid.attach(self.combo1, 0, 7, 2, 1)
        self.grid.attach(self.combo2, 0, 8, 2, 1)
        self.grid.attach(self.label3, 2, 5, 1, 1)
        self.grid.attach(self.entry3, 3, 5, 1, 1)
        self.grid.attach(self.label4, 2, 6, 1, 1)
        self.grid.attach(self.entry4, 3, 6, 1, 1)
        self.grid.attach(self.label5, 2, 7, 1, 1)
        self.grid.attach(self.entry5, 3, 7, 1, 1)
        self.grid.attach(self.label6, 2, 8, 1, 1)
        self.grid.attach(self.entry6, 3, 8, 1, 1)
        self.grid.attach(self.label7, 2, 9, 1, 1)
        self.grid.attach(self.entry7, 3, 9, 1, 1)
        self.grid.attach(self.label8, 0, 9, 1, 1)
        self.grid.attach(self.entry8, 1, 9, 1, 1)
        self.grid.attach(self.label9, 0, 10, 1, 1)
        self.grid.attach(self.entry9, 1, 10, 3, 1)
        self.grid.attach(self.check1, 2, 11, 1, 1)
        self.grid.attach(self.combo3, 0, 12, 1, 1)
        self.grid.attach(self.button1, 1, 13, 2, 1)
        self.add(self.grid)

    def print_dialog(self, button1):
        #Check entries.
        return_value = self.validate_entries()
        if(return_value==0):
            entries_array = (self.entry1, self.entry2, self.entry3, self.entry4, self.entry5, self.entry6, self.entry7, self.entry8, self.entry9, self.check1, self.combo1, self.combo2, self.combo3)
            self.TextBox1.print_dialog(entries_array)

    def change_font(self, combo3):
        self.TextBox1.change_textview_font(self.combo3)

    def validate_entries(self):
        if(0 >= int(self.entry1.get_text()) or int(self.entry1.get_text()) > 50):
            print("Rows " + self.entry1.get_text() + " Range 0<rows<=50")
            return 1
        elif(0 >= int(self.entry2.get_text()) or int(self.entry2.get_text()) > 10):
            print("Columns " + self.entry2.get_text() + " Range 0<columns<=10")
            return 1
        elif(0 > int(self.entry3.get_text()) or int(self.entry3.get_text()) > 30):
            print("Shift Right " + self.entry3.get_text() + " Range 0<=Shift Right<=30")
            return 1
        elif(1 > int(self.entry4.get_text()) or int(self.entry4.get_text()) > 40):
            print("Shift Down " + self.entry4.get_text() +" Range 1<=Shift Down<=40")
            return 1
        elif(5 > int(self.entry5.get_text()) or int(self.entry5.get_text()) > 20):
            print("Column Width " + self.entry5.get_text() + " Range 5<=Column Width<=20")
            return 1
        elif(0 > int(self.entry6.get_text()) or int(self.entry6.get_text()) > 5):
            print("Pad Number " + self.entry6.get_text() + " Range 0<=Pad Number<=5")
            return 1
        elif(0 > int(self.entry7.get_text()) or int(self.entry7.get_text()) > 5):
            print("Pad Column " + self.entry7.get_text() + " Range 0<=Pad Column<=5")
            return 1
        elif(1 > int(self.entry8.get_text()) or int(self.entry8.get_text()) > 20):
            print("Tables " + self.entry8.get_text() + " Range 1<=Tables<=20")
            return 1
        else:
            return 0

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()



