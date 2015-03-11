#!/user/bin/python

"""
   Test some Python and GTK+ printing. Print size defaults to "letter" or 8.5 x 11 sized layout.
   Python 2.7 with PyGTK and GTK2 on Ubuntu 14.04. For comparison with print1.py.

   C. Eric Cashon
"""

import gtk
import pango
import pangocairo
import math

class TextBox(gtk.TextView):
    def __init__(self):
        gtk.TextView.__init__(self)
        self.page_width = 0
        self.page_height = 0
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
        operation = gtk.PrintOperation()
        #Figure out number of pages from number of lines.
        count_lines = self.textbuffer.get_line_count()
        operation.set_n_pages(count_lines)
        operation.connect("begin_print", self.begin_print)
        operation.connect("draw_page", self.draw_page)
        result = operation.run(gtk.PRINT_OPERATION_ACTION_PRINT_DIALOG, None)

    def begin_print(self, operation, gtk_context):
        self.page_width = gtk_context.get_width()
        self.page_height = gtk_context.get_height()
        pango_context = self.get_pango_context()
        description = pango_context.get_font_description()
        self.pango_layout = gtk_context.create_pango_layout()
        self.pango_layout.set_font_description(description)
        self.pango_layout.set_width(int(self.page_width*pango.SCALE));
        self.pango_layout.set_wrap(pango.WRAP_CHAR)

    def draw_page(self, operation, gtk_context, page_number):
        cairo_context = gtk_context.get_cairo_context()
        #Color some different rectangles on different pages.
        if(page_number%2):
            cairo_context.set_source_rgb(1.0, 0.0, 1.0)
        else:
            cairo_context.set_source_rgb(1.0, 1.0, 0.0)
        cairo_context.rectangle(0, 0, self.page_width, self.page_height)
        cairo_context.stroke()
        #Smile.
        cairo_context.set_source_rgb(0.0, 0.5, 0.5)
        cairo_context.arc(self.page_width/2, 100, 50, 0, 2*math.pi);
        cairo_context.stroke()
        cairo_context.arc(self.page_width/2+20, 80, 3, 0, 2*math.pi);
        cairo_context.fill()
        cairo_context.stroke()
        cairo_context.arc(self.page_width/2-20, 80, 3, 0, 2*math.pi);
        cairo_context.fill()
        cairo_context.stroke()
        cairo_context.scale(1, 0.7)
        cairo_context.arc(self.page_width/2, 150, 30, 0, math.pi);
        cairo_context.stroke()
        #Text
        cairo_context.set_source_rgb(0.0, 0.0, 0.0)
        string = self.get_line(page_number)
        self.pango_layout.set_markup(string)
        cairo_context.show_layout(self.pango_layout)

class MainWindow(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)
        self.set_title("Print")
        self.set_default_size(300,300)
        self.TextBox1 = TextBox()
        self.scrolledwindow = gtk.ScrolledWindow()
        self.scrolledwindow.add(self.TextBox1)
        self.button1 = gtk.Button("Print Dialog")
        self.button1.connect("clicked", self.print_dialog)
        self.table = gtk.Table(5, 1, True)
        self.table.attach(self.scrolledwindow, 0, 1, 0, 6)
        self.table.attach(self.button1, 0, 1, 6, 7)
        self.add(self.table)

    def print_dialog(self, button1):
        print("Print Dialog")
        self.TextBox1.print_dialog()

win = MainWindow()
win.connect("delete-event", gtk.main_quit) 
win.show_all()
gtk.main()



