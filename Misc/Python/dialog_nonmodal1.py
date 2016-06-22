#!/usr/bin/python3

import gi
gi.require_version('Gtk', '3.0')
from gi.repository import Gtk, Gdk

'''
    A non-modal dialog in GTK+. There is also an example of this in the Csamples folder that is named
similarly.

    C. Eric Cashon

'''

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self)
        self.set_title("Non Modal Dialogs")
        self.set_default_size(300, 100);
        self.set_position(Gtk.WindowPosition.CENTER)

        self.i = 1 

        button = Gtk.Button("Show Dialog")
        button.connect('clicked', self.show_dialog) 

        self.add(button)

    def show_dialog(self, widget):
        dialog = DialogWindow(self)

class DialogWindow(Gtk.Dialog):
    def __init__(self, main_window):
        Gtk.Dialog.__init__(self, "Dialog", main_window, Gtk.DialogFlags.DESTROY_WITH_PARENT, ("Cancel", Gtk.ResponseType.CANCEL))
        self.set_default_size(200, 100);

        string = "Dialog " + str(main_window.i)
        label = Gtk.Label(string)

        box = self.get_content_area()
        box.pack_start(label, True, True, 0)

        self.connect("response", lambda *x: self.destroy())

        #Move the dialog windows.
        width = Gdk.Screen.width();
        height = Gdk.Screen.height();
        sections_x = width // 100;
        sections_y = height // 100;
        s1 = (main_window.i - 1) % (sections_x * sections_y);
        move_x = (s1 % sections_x) * 100;
        move_y = (s1 // sections_x) * 100;    
        print("Move " +  str(width) + " " + str(height) + " " + str(sections_x) + " " + str(sections_y) + " " + str(move_x) + " " + str(move_y));
        self.move(move_x, move_y); 
        main_window.i += 1

        self.show_all()

win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()

