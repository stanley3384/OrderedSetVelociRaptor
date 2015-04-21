#!/usr/bin/python

'''
    Test code for building some combo box text widgets for providing labels for report_generator.py.
    Python 2.7 with GTK 3.10 on Ubuntu 14.04.

    C. Eric Cashon
'''

from gi.repository import Gtk

row_labels = []
column_labels = []

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="ComboBox Entries")
        self.set_border_width(25)
        self.set_default_size(225,100)
        self.row_value = 0
        self.column_value = 0
        button1 = Gtk.Button("Set Labels")
        button1.connect("clicked", self.labels_dialog)
        button2 = Gtk.Button("Print Labels")
        button2.connect("clicked", self.print_labels)
        self.entry1 = Gtk.Entry()
        self.entry1.set_text("8")
        self.entry1.connect("focus-in-event", self.save_current_row_value)
        self.entry1.connect("focus-out-event", self.clear_row_labels)
        self.entry2 = Gtk.Entry()
        self.entry2.set_text("12")
        self.entry2.connect("focus-in-event", self.save_current_column_value)
        self.entry2.connect("focus-out-event", self.clear_column_labels)

        grid = Gtk.Grid()
        grid.attach(button1, 0, 0, 1, 1)
        grid.attach(button2, 0, 1, 1, 1)
        grid.attach(self.entry1, 0, 2, 1, 1)
        grid.attach(self.entry2, 0, 3, 1, 1)
        self.add(grid)
        self.show_all()

    def labels_dialog(self, button):
        rows = int(self.entry1.get_text())
        columns = int(self.entry2.get_text())
        #set some boundries.
        if(rows > 0 and rows < 20 and columns > 0 and columns < 20):
            dialog = LabelsDialog(self, rows, columns)
            response = dialog.run()        
            dialog.destroy()
        else:
            print("0<rows<20 and 0<columns<20")

    def save_current_row_value(self, event, widget):
        print("Save Row Value")
        self.row_value = int(self.entry1.get_text())

    def clear_row_labels(self, event, widget):
        r_value = int(self.entry1.get_text())
        if(row_labels and r_value!=self.row_value):
            print("Clear Row Labels")
            del row_labels[:]
            print(row_labels)
        return False

    def save_current_column_value(self, event, widget):
        print("Save Column Value")
        self.column_value = int(self.entry2.get_text())

    def clear_column_labels(self, event, widget):
        c_value = int(self.entry2.get_text())
        if(column_labels and c_value!=self.column_value):
            print("Clear Column Labels")
            del column_labels[:]
            print(column_labels)
        return False

    def print_labels(self, widget):
        print("Labels")
        print(row_labels)
        print(column_labels)

class LabelsDialog(Gtk.Dialog):
    def __init__(self, parent, rows, columns):
        Gtk.Dialog.__init__(self, "Set Labels", parent, 0, (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL, Gtk.STOCK_OK, Gtk.ResponseType.OK))
        content_area = self.get_content_area()
        self.set_border_width(10)
        self.active_row = 0
        self.active_column = 0
 
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

        focus_button = self.get_widget_for_response(Gtk.ResponseType.OK)
        focus_button.grab_focus()
        focus_button.connect("clicked", self.load_labels) 

        grid = Gtk.Grid()
        grid.attach(row_label, 0, 0, 1, 1)
        grid.attach(self.row_combo, 0, 1, 1, 1)
        grid.attach(column_label, 0, 2, 1, 1)
        grid.attach(self.column_combo, 0, 3, 1, 1)
        content_area.add(grid)
        self.show_all()

    def row_combo_changed(self, combo):
        text = combo.get_active_text()
        text_id = combo.get_active()
        if(text_id!=-1):
            self.active_row = text_id
        if(text != None):
            print(text + " " + str(self.active_row))
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
            print(text + " " + str(self.active_column))
            combo.handler_block(self.column_changed)
            combo.insert_text(self.active_column, text)
            combo.remove(self.active_column+1)
            combo.handler_unblock(self.column_changed)

    def load_labels(self, button):
        print("Load Labels")
        if(row_labels):
            del row_labels[:]
        model1 = self.row_combo.get_model()
        iter1 = model1.get_iter_first()
        while(iter1):
	    row_labels.append(model1[iter1][0])
	    iter1 = model1.iter_next(iter1)
        print(row_labels)
        if(column_labels):
            del column_labels[:]
        model2 = self.column_combo.get_model()
        iter2 = model2.get_iter_first()
        while(iter2):
	    column_labels.append(model2[iter2][0])
	    iter2 = model2.iter_next(iter2)
        print(column_labels)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()
