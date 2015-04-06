#!/usr/bin/python

"""
 Test code for getting a line of text from a textview.

 C. Eric Cashon
"""

from gi.repository import Gtk

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        self.set_wrap_mode(2)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("Test for text line 1\n  Test for text line2\nTest for text line3\n\nTest for text line 4\n  Test for text line5")

    def get_lines(self):
        start_line = self.textbuffer.get_start_iter()
        end_line = self.textbuffer.get_start_iter()
        self.forward_display_line_end(end_line)
        print(self.textbuffer.get_text(start_line, end_line, False))
        while(self.forward_display_line(start_line)):
            self.forward_display_line(end_line)
            self.forward_display_line_end(end_line)
            print(self.textbuffer.get_text(start_line, end_line, False))

    def get_cursor_line(self):
        cursor_position = self.textbuffer.get_property("cursor-position")
        cursor_iter1 = self.textbuffer.get_start_iter()
        cursor_iter2 = self.textbuffer.get_start_iter()
        cursor_iter2.set_offset(cursor_position)
        lines = cursor_iter2.get_line()
        cursor_iter2.forward_to_line_end()
        cursor_iter1.forward_lines(lines)
        print(self.textbuffer.get_text(cursor_iter1, cursor_iter2, False))
              
class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Find Lines in Textbox")
        self.set_default_size(300,300)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.button1 = Gtk.Button("Find Lines")
        self.button1.connect("clicked", self.find_lines) 
        self.button2 = Gtk.Button("Move Cursor and Fine Line")
        self.button2.connect("clicked", self.find_cursor_line)         
        self.grid = Gtk.Grid()
        self.grid.attach(self.scrolledwindow, 0, 0, 1, 1)
        self.grid.attach(self.button1, 0, 1, 1, 1)
        self.grid.attach(self.button2, 0, 2, 1, 1)
        self.add(self.grid)
        self.show_all()

    def find_lines(self, button1):
        print("Find Lines")
        self.TextBox1.get_lines()

    def find_cursor_line(self, button2):
        print("Cursor Line")
        self.TextBox1.get_cursor_line()
    
win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()

