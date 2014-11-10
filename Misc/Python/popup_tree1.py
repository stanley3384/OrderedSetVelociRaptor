#!/user/bin/python

'''
   Test code for using a pop-up menu to change values in a treeview. Works with Python2.7 and GTK 3.4
on Ubuntu 12.04. Also works with Python3.2. The treeview cells can be edited, deleted and rows can be
appended or removed. Change some colors around also.

C. Eric Cashon

'''

from gi.repository import Gtk, Gdk

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Treeview Popup")
        self.set_border_width(10)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_default_size(250, 100)

        style_provider = Gtk.CssProvider()
        #Add b for byte string error in python3.2. Works in 2.7 also.
        css = b"""GtkWindow{background: blue;}
                 GtkLabel#css_label1{color: white}
                 GtkLabel#css_label2{color: white}
                 GtkTreeView{color: black}
                 GtkTreeView:selected{color: black; background: green; border-width: 1px; border-color: black;}
                 GtkMenu#css_popup1{color: black; background: green;}
                 column-header .button{color: white; background: purple;}"""
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        label1 = Gtk.Label("Treeview1")
        label1.set_name("css_label1")
        label2 = Gtk.Label("Treeview2")
        label2.set_name("css_label2")
        treeview1 = TreeView()
        treeview2 = TreeView()

        box = Gtk.Box()
        box.set_orientation(Gtk.Orientation.VERTICAL)
        box.pack_start(label1, False, False, 0)
        box.pack_start(treeview1, False, False, 0)
        box.pack_start(label2, False, False, 0)
        box.pack_start(treeview2, False, False, 0)
        self.add(box)

class TreeView(Gtk.TreeView):
    def __init__(self):
        Gtk.TreeView.__init__(self)
        self.x1=0
        self.y1=0
        self.set_headers_visible(True)

        self.liststore = Gtk.ListStore(str, str)
        self.liststore.append(["Top", "Red"])
        self.liststore.append(["Middle", "Green"])
        self.liststore.append(["Bottom", "Blue"])

        self.set_model(model=self.liststore)

        self.menu = Gtk.Menu()
        self.menu.set_name("css_popup1")
        self.rename_item = Gtk.MenuItem("Edit Cell")
        self.rename_item.connect("activate", self.edit_cell)
        self.delete_item = Gtk.MenuItem("Delete From Cell")
        self.delete_item.connect("activate", self.delete_from_cell)
        self.add_row_item = Gtk.MenuItem("Append Row")
        self.add_row_item.connect("activate", self.append_row)
        self.remove_row_item = Gtk.MenuItem("Remove Row")
        self.remove_row_item.connect("activate", self.remove_row)
        self.menu.append(self.rename_item)
        self.menu.append(self.delete_item)
        self.menu.append(self.add_row_item)
        self.menu.append(self.remove_row_item)
        self.menu.attach_to_widget(self, None)
        self.connect("button-press-event", self.show_menu)

        self.renderer_text1 = Gtk.CellRendererText()
        self.renderer_text1.set_property("background", "red")
        self.column_text1 = Gtk.TreeViewColumn("Title1", self.renderer_text1, text=0)
        self.renderer_text1.set_property("editable", True)    
        self.append_column(self.column_text1)
        self.renderer_text1.connect("edited", self.text_edited1)

        self.renderer_text2 = Gtk.CellRendererText()
        self.renderer_text2.set_property("editable", True)        
        self.column_text2 = Gtk.TreeViewColumn("Title2", self.renderer_text2, text=1)
        self.append_column(self.column_text2)
        self.renderer_text2.connect("edited", self.text_edited2)

    def text_edited1(self, widget, path, text):
        self.liststore[path][0] = text

    def text_edited2(self, widget, path, text):
        self.liststore[path][1] = text

    def show_menu(self, widget, event):
        if event.button == 1:
            self.x1 = int(event.x)
            self.y1 = int(event.y)
            self.menu.show_all()
            self.menu.popup(None, None, None, None, 0, Gtk.get_current_event_time())
        return False

    def edit_cell(self, *args):
        print("Edit Cell")
        path_info = self.get_path_at_pos(self.x1, self.y1)
        if path_info is not None:
            path, col, cellx, celly = path_info
            self.grab_focus()
            self.set_cursor(path, col, True)
            
    def delete_from_cell(self, *args):
        print("Delete From Cell")
        path_info = self.get_path_at_pos(self.x1, self.y1)
        if path_info is not None:
            path, col, cellx, celly = path_info
            self.grab_focus()
            self.set_cursor(path, col, True)
            if(col==self.column_text1):
                self.text_edited1(self, path, "")
            if(col==self.column_text2):
                self.text_edited2(self, path, "")

    def append_row(self, *args):
        print("Append Row")
        self.liststore.append(None)

    def remove_row(self, *args):
        print("Remove Row")
        path_info = self.get_path_at_pos(self.x1, self.y1)
        if path_info is not None:
            path, col, cellx, celly = path_info
            self.grab_focus()
            self.set_cursor(path, col, True)
            treemodel = self.get_model()
            iter1=treemodel.get_iter(path)
            self.liststore.remove(iter1)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()
