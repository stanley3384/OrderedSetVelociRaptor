#!/user/bin/python

"""
   Test some Python and GTK+ printing. Print size defaults to "letter" or 8.5 x 11 sized layout.
   Look at some layout values for positioning fonts and graphics.
   Python 2.7 with GTK 3.10 on Ubuntu 14.04.

   C. Eric Cashon
"""

from gi.repository import Gtk, Pango, PangoCairo
import math
import sys

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        font_description = Pango.FontDescription("Monospace 12") 
        self.override_font(font_description);
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

    def print_dialog(self):
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
        print("PageWidth " + str(self.page_width) + " PageHeight " + str(self.page_width) + " TextWidth " + str(self.text_width) + " TextHeight " + str(self.text_height))
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

        #Get some test numbers to add to the string.
        number_string = "\n\n\n"
        max_length = 0
        data_values = [[22.4, 22.223, 22.22],[22.27, 26.2267, 22.2456],[22.22, 234.22, 22.22]]
        for x in range(3):
            for y in range(3):
                if(max_length<len(str(data_values[x][y]))):
                    max_length=len(str(data_values[x][y]))
        print max_length
        for x in range(3):
            for y in range(3):
                sys.stdout.write(str(data_values[x][y]) + " ")
            sys.stdout.write("\n")
        for x in range(3):
            #Test with strings 10 chars long. Can join with spaces for testing also.
            row_tuple = "".join( "{k:*>{m}}".format(k=k,m=max_length + 3) for k in data_values[x])
            print row_tuple
            if(x<3-1):
                number_string = number_string + row_tuple + "\n"
            else:
                number_string = number_string + row_tuple

        #Get rectangle for one monospace char for sizing
        self.pango_layout.set_markup("2")
        rectangle_ink, rectangle_log = self.pango_layout.get_extents()
        print(" Rectangle1 " + str(rectangle_ink.height) + " Rectangle1 " + str(rectangle_ink.width) + " Rectangle2 " + str(rectangle_log.height) + " Rectangle2 " + str(rectangle_log.width))

        #Table for test numbers.
        #horizontal lines
        cairo_context.set_source_rgb(1.0, 0.0, 1.0)
        cairo_context.move_to(0, (rectangle_log.height*4)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*30)/Pango.SCALE, (rectangle_log.height*4)/Pango.SCALE)
        cairo_context.stroke() 
        cairo_context.move_to(0, (rectangle_log.height*5)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*30)/Pango.SCALE, (rectangle_log.height*5)/Pango.SCALE)
        cairo_context.stroke()
        cairo_context.move_to(0, (rectangle_log.height*6)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*30)/Pango.SCALE, (rectangle_log.height*6)/Pango.SCALE)
        cairo_context.stroke() 
        cairo_context.move_to(0, (rectangle_log.height*7)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*30)/Pango.SCALE, (rectangle_log.height*7)/Pango.SCALE)
        cairo_context.stroke()
        #vertical lines 
        cairo_context.move_to(0, (rectangle_log.height*4)/Pango.SCALE)
        cairo_context.line_to(0, (rectangle_log.height*7)/Pango.SCALE)
        cairo_context.stroke() 
        cairo_context.move_to((rectangle_log.width*10)/Pango.SCALE, (rectangle_log.height*4)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*10)/Pango.SCALE, (rectangle_log.height*7)/Pango.SCALE)
        cairo_context.stroke() 
        cairo_context.move_to((rectangle_log.width*20)/Pango.SCALE, (rectangle_log.height*4)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*20)/Pango.SCALE, (rectangle_log.height*7)/Pango.SCALE)
        cairo_context.stroke()
        cairo_context.move_to((rectangle_log.width*30)/Pango.SCALE, (rectangle_log.height*4)/Pango.SCALE)
        cairo_context.line_to((rectangle_log.width*30)/Pango.SCALE, (rectangle_log.height*7)/Pango.SCALE)
        cairo_context.stroke()  

        #Text and test numbers
        cairo_context.set_source_rgb(0.0, 0.0, 0.0)
        string = self.get_line(page_number)
        self.pango_layout.set_markup(string + number_string)
        PangoCairo.show_layout(cairo_context, self.pango_layout)

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Print")
        self.set_default_size(300,300)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.button1 = Gtk.Button("Print Dialog")
        self.button1.connect("clicked", self.print_dialog)
        self.grid = Gtk.Grid()
        self.grid.attach(self.scrolledwindow, 0, 0, 1, 1)
        self.grid.attach(self.button1, 0, 1, 1, 1)
        self.add(self.grid)

    def print_dialog(self, button1):
        print("Print Dialog")
        self.TextBox1.print_dialog()

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()



