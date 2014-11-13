#!/user/bin/python

'''
   Test code for using a pop-up menu to change values in a treeview. Works with Python2.7 and GTK 3.4
on Ubuntu 12.04. Also works with Python3.2. The treeview cells can be edited, deleted and rows can be
appended or removed. Change some colors around also and instantiate several treeviews.

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
                 GtkLabel#css_label3{color: white}
                 GtkLabel#css_label4{color: white}
                 GtkTreeView{color: black; -GtkTreeView-grid-line-width: 2}
                 GtkTreeView:selected{color: black; background: green; border-width: 1px; border-color: black;}
                 GtkMenu#css_popup1{color: black; background: green;}
                 column-header .button{color: white; background: purple;}"""
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        label1 = Gtk.Label("Treeview1")
        label1.set_name("css_label1")
        label2 = Gtk.Label("Treeview2")
        label2.set_name("css_label2")
        label3 = Gtk.Label("Treeview3")
        label3.set_name("css_label3")
        label4 = Gtk.Label("Treeview4")
        label4.set_name("css_label4")
        #Test out a few treeviews.
        self.treeview1 = TreeView()
        self.treeview2 = TreeView("orange")
        self.treeview3 = TreeView("yellow")
        self.treeview4 = TreeView("purple")

        #Reset selected in treeviews.
        self.connect("set-focus", self.reset_selected_treeview)

        grid = Gtk.Grid()
        grid.set_column_spacing(10)
        grid.attach(label1, 0, 0, 1, 1)
        grid.attach(self.treeview1, 0, 1, 1, 1)
        grid.attach(label2, 0, 2, 1, 1)
        grid.attach(self.treeview2, 0, 3, 1, 1)
        grid.attach(label3, 1, 0, 1, 1)
        grid.attach(self.treeview3, 1, 1, 1, 1)
        grid.attach(label4, 1, 2, 1, 1)
        grid.attach(self.treeview4, 1, 3, 1, 1)
        self.add(grid)

    #Move the selection of the treeviews.
    def reset_selected_treeview(self, *args):
        print("Focus Changed")
        if(self.treeview1.active==False):
            self.treeview1.unselect_row()
        if(self.treeview2.active==False):
            self.treeview2.unselect_row()
        if(self.treeview3.active==False):
            self.treeview3.unselect_row()
        if(self.treeview4.active==False):
            self.treeview4.unselect_row()        
        self.treeview1.active=False
        self.treeview2.active=False
        self.treeview3.active=False
        self.treeview4.active=False

class TreeView(Gtk.TreeView):
    def __init__(self, first_column_color = "red"):
        Gtk.TreeView.__init__(self)
        self.x1=0
        self.y1=0
        self.active=False
        self.set_headers_visible(True)
        #Gtk.TREE_VIEW_GRID_LINES_BOTH enumeration
        self.set_grid_lines(3)

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
        self.insert_row_item = Gtk.MenuItem("Insert Row")
        self.insert_row_item.connect("activate", self.insert_row)
        self.remove_row_item = Gtk.MenuItem("Remove Row")
        self.remove_row_item.connect("activate", self.remove_row)
        self.menu.append(self.rename_item)
        self.menu.append(self.delete_item)
        self.menu.append(self.add_row_item)
        self.menu.append(self.insert_row_item)
        self.menu.append(self.remove_row_item)
        self.menu.attach_to_widget(self, None)
        self.connect("button-press-event", self.show_menu)

        self.renderer_text1 = Gtk.CellRendererText()
        self.renderer_text1.set_property("background", first_column_color)
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
        print("Show Menu")
        self.active=True
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

    def insert_row(self, *args):
        print("Remove Row")
        path_info = self.get_path_at_pos(self.x1, self.y1)
        if path_info is not None:
            path, col, cellx, celly = path_info
            treemodel = self.get_model()
            iter1=treemodel.get_iter(path)
            self.liststore.insert_after(iter1, None)

    def unselect_row(self):
        print("Unselect Row")
        tree_select = self.get_selection()
        tree_select.unselect_all()

win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()
