#!/user/bin/python

"""
   Test some Python and GTK+ printing. Print size defaults to "letter" or 8.5 x 11 sized layout.
   Look at some layout values for positioning fonts and graphics.
   Python 2.7 with GTK 3.10 on Ubuntu 14.04.

   C. Eric Cashon
"""

from gi.repository import Gtk, Pango, PangoCairo
import random
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
        self.textbuffer.set_text("Print this on a page!Fill the line to the end and wrap on a char. cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\nPrint line 2 on a page\nPrint line 3 on a page\nPrint line 4 on a page")

    def get_line(self, line_number):
        start_line = self.textbuffer.get_iter_at_line(line_number)
        end_line = self.textbuffer.get_iter_at_line(line_number)
        self.forward_display_line_end(end_line)
        print(self.textbuffer.get_text(start_line, end_line, False))
        string = self.textbuffer.get_text(start_line, end_line, False)
        return string

    def print_dialog(self, entries_array):
        self.entries_array_text = entries_array
        operation = Gtk.PrintOperation()
        #Figure out number of pages from number of lines.
        count_lines = self.textbuffer.get_line_count()
        operation.set_n_pages(count_lines)
        operation.connect("begin_print", self.begin_print)
        operation.connect("draw_page", self.draw_page)
        result = operation.run(Gtk.PrintOperationAction.PRINT_DIALOG, None)

    def begin_print(self, operation, gtk_context):
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
        print("PageWidth " + str(self.page_width) + " PageHeight " + str(self.page_height) + " TextWidth " + str(self.text_width) + " TextHeight " + str(self.text_height))
        self.pango_layout.set_wrap(Pango.WrapMode.CHAR)

    def draw_page(self, operation, gtk_context, page_number):
        cairo_context = gtk_context.get_cairo_context()
        #Color some different rectangles on different pages.
        if(page_number%2):
            cairo_context.set_source_rgb(1.0, 0.0, 1.0)
        else:
            cairo_context.set_source_rgb(1.0, 1.0, 0.0)
        cairo_context.rectangle(0, 0, self.page_width, self.page_height)
        cairo_context.stroke()

        #Get variables from UI.
        rows = int(self.entries_array_text[0].get_text())
        columns = int(self.entries_array_text[1].get_text())
        shift_margin = int(self.entries_array_text[2].get_text())
        shift_below_text = int(self.entries_array_text[3].get_text())
        column_width = int(self.entries_array_text[4].get_text())
        
        #Get some test numbers to add to the string.
        data_values =  [[0 for x in range(columns)] for x in range(rows)] 
        for x in range(rows):
            for y in range(columns):
                data_values[x][y] = str(round((random.random() * 100), 3))

        #Get max length and print to screen.
        max_length = 0
        for x in range(rows):
            for y in range(columns):
                if(max_length<len(str(data_values[x][y]))):
                    max_length=len(str(data_values[x][y]))
                #sys.stdout.write(str(data_values[x][y]) + " ")
            #sys.stdout.write("\n")
        print("Max Length " +str(max_length))

        #Get rectangle for one monospace char for sizing
        self.pango_layout.set_markup("2")
        rectangle_ink, rectangle_log = self.pango_layout.get_extents()
        print(" Rectangle1 " + str(rectangle_ink.height) + " Rectangle1 " + str(rectangle_ink.width) + " Rectangle2 " + str(rectangle_log.height) + " Rectangle2 " + str(rectangle_log.width))

        #Get lines of text to be printed on the page.
        string = self.get_line(page_number)
        self.pango_layout.set_markup(string)
        line_count = self.pango_layout.get_line_count()
        lines_per_page = int(self.text_height / rectangle_log.height)
        print("Line Count " + str(line_count) + " Lines Per Page " + str(lines_per_page))

        #pad each number and first number in each column.
        number_string = "{:\n>{m}}".format("", m=shift_below_text) 
        column_padding = "{:*>{m}}".format("", m=shift_margin) 
        for x in range(rows):
            #Test with strings 10 chars long. Can join with spaces for testing also. 
            row_tuple = "".join( "{k:*>{m}}".format(k=k,m=max_length + (column_width-max_length)) for k in data_values[x])
            #print row_tuple
            if(x<rows-1):
                number_string = number_string + column_padding + row_tuple + "\n"
            else:
                number_string = number_string + column_padding + row_tuple

        #Background color for each cell.
        top = line_count + shift_below_text -1
        left_margin = (shift_margin * rectangle_log.width)/Pango.SCALE
        for x in range(rows):
            for y in range(columns):
                if(x%2):
                    cairo_context.set_source_rgb(0.5, 0.7, 1.0)
                else:
                    cairo_context.set_source_rgb(0.7, 1.0, 1.0)
                cairo_context.rectangle(((shift_margin +(column_width*y)) * rectangle_log.width)/Pango.SCALE, (rectangle_log.height * (top + x))/Pango.SCALE, (rectangle_log.width/Pango.SCALE)*column_width, rectangle_log.height/Pango.SCALE)
                cairo_context.fill()
                cairo_context.stroke()

        #Table grid for test numbers.
        cairo_context.set_source_rgb(1.0, 0.0, 1.0)
        top = line_count + shift_below_text -1
        bottom = top + rows
        left_margin = (shift_margin * rectangle_log.width)/Pango.SCALE
        column_width = column_width
        total_chars = column_width * columns
        for x in range(rows + 1): 
            cairo_context.move_to(left_margin, (rectangle_log.height * (top + x))/Pango.SCALE)
            cairo_context.line_to(((rectangle_log.width * total_chars)/Pango.SCALE) + left_margin, (rectangle_log.height  *(top + x))/Pango.SCALE)
            cairo_context.stroke() 
        for x in range(columns + 1): 
            cairo_context.move_to(((rectangle_log.width * x * column_width)/Pango.SCALE) + left_margin, (rectangle_log.height*top)/Pango.SCALE)
            cairo_context.line_to(((rectangle_log.width * x * column_width)/Pango.SCALE) + left_margin, (rectangle_log.height*bottom)/Pango.SCALE)
            cairo_context.stroke() 

        #Text and test numbers
        cairo_context.set_source_rgb(0.0, 0.0, 0.0)
        string = self.get_line(page_number)
        self.pango_layout.set_markup(string + number_string)
        PangoCairo.show_layout(cairo_context, self.pango_layout)

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Print")
        self.set_default_size(350,350)
        self.set_border_width(10)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.label0 = Gtk.Label("Draw Test Table")
        self.label1 = Gtk.Label("Rows")
        self.entry1 = Gtk.Entry()
        self.entry1.set_width_chars(3)
        self.entry1.set_text("20")
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
        self.entry4.set_text("2")
        self.entry4.set_width_chars(3)
        self.label5 = Gtk.Label("Column Width")
        self.entry5 = Gtk.Entry()
        self.entry5.set_text("10")
        self.entry5.set_width_chars(3)
        self.button1 = Gtk.Button("Print Dialog")
        self.button1.connect("clicked", self.print_dialog)
        self.grid = Gtk.Grid()
        self.grid.set_row_spacing(10)
        self.grid.attach(self.scrolledwindow, 0, 0, 4, 4)
        self.grid.attach(self.label0, 1, 4, 2, 1)
        self.grid.attach(self.label1, 0, 5, 1, 1)
        self.grid.attach(self.entry1, 1, 5, 1, 1)
        self.grid.attach(self.label2, 0, 6, 1, 1)
        self.grid.attach(self.entry2, 1, 6, 1, 1)
        self.grid.attach(self.label3, 2, 5, 1, 1)
        self.grid.attach(self.entry3, 3, 5, 1, 1)
        self.grid.attach(self.label4, 2, 6, 1, 1)
        self.grid.attach(self.entry4, 3, 6, 1, 1)
        self.grid.attach(self.label5, 2, 7, 1, 1)
        self.grid.attach(self.entry5, 3, 7, 1, 1)
        self.grid.attach(self.button1, 1, 8, 2, 1)
        self.add(self.grid)

    def print_dialog(self, button1):
        print("Print Dialog")
        #Check entries.
        return_value = self.validate_entries()
        if(return_value==0):
            entries_array = (self.entry1, self.entry2, self.entry3, self.entry4, self.entry5)
            self.TextBox1.print_dialog(entries_array)

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
            print("Column Width " + self.entry1.get_text() + " Range 5<=Column Width<=20")
            return 1
        else:
            return 0

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()



