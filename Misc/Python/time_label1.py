#!/usr/bin/python

#
# Test code for text iters and finding a word in a TextBox.
#
# C. Eric Cashon

from gi.repository import Gtk
from gi.repository import GObject
import time
import random

rand = random.random()

class TimeLabel(Gtk.Label):
    def __init__(self, milliseconds=1000):
        Gtk.Label.__init__(self)
        GObject.timeout_add(milliseconds, self.get_time) 
    
    def get_time(self):
        localtime = time.asctime(time.localtime(time.time()))
        self.set_text(localtime + "\n" + str(random.random()) + "\n" + str(rand))
        return True

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Time Label")
        self.set_default_size(200,100)
        label1 = TimeLabel(100)
        label2 = TimeLabel()
        label3 = TimeLabel(3000)
        grid = Gtk.Grid()
        grid.attach(label1, 0, 0, 1, 1)
        grid.attach(label2, 0, 1, 1, 1)
        grid.attach(label3, 0, 2, 1, 1)
        self.add(grid)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
