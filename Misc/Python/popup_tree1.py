#!/user/bin/python

'''
   Test code for using a pop-up menu to change values in a treeview.

C. Eric Cashon

'''

from gi.repository import Gtk, Gdk

class CellRendererTextWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Treeview Popup")
        self.set_border_width(10)
        self.set_default_size(250, 100)
        self.x1=0
        self.y1=0
        self.liststore = Gtk.ListStore(str, str)
        self.liststore.append(["Top", "Red"])
        self.liststore.append(["Middle", "Green"])
        self.liststore.append(["Bottom", "Blue"])

        self.treeview = Gtk.TreeView(model=self.liststore)
        self.treeview.set_headers_visible(True)

        self.menu = Gtk.Menu()
        self.menu.set_name("css_popup1")
        self.rename_item = Gtk.MenuItem("Rename")
        self.rename_item.connect("activate", self.rename)
        self.delete_item = Gtk.MenuItem("Delete")
        self.delete_item.connect("activate", self.delete)
        self.menu.append(self.rename_item)
        self.menu.append(self.delete_item)
        self.menu.attach_to_widget(self.treeview, None)
        self.treeview.connect("button-press-event", self.show_menu)

        self.renderer_text1 = Gtk.CellRendererText()
        self.column_text1 = Gtk.TreeViewColumn("Text1", self.renderer_text1, text=0)
        self.renderer_text1.set_property("editable", True)    
        self.treeview.append_column(self.column_text1)
        self.renderer_text1.connect("edited", self.text_edited1)

        self.renderer_text2 = Gtk.CellRendererText()
        self.renderer_text2.set_property("editable", True)        
        self.column_text2 = Gtk.TreeViewColumn("Text2", self.renderer_text2, text=1)
        self.treeview.append_column(self.column_text2)
        self.renderer_text2.connect("edited", self.text_edited2)

        style_provider = Gtk.CssProvider()
        css = """GtkWindow{background: blue;}
                 GtkTreeView{background: yellow;}
                 GtkTreeView:selected{color: black; background: green; border-width: 1px; border-color: black;}
                 GtkMenu#css_popup1{color: black; background: green;}
                 column-header .button{color: white; background: red;}"""
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        box = Gtk.Box()
        box.set_orientation(Gtk.Orientation.VERTICAL)
        box.pack_start(self.treeview, False, False, 0)
        self.add(box)

    def text_edited1(self, widget, path, text):
        print("Text1 Edited")
        self.liststore[path][0] = text

    def text_edited2(self, widget, path, text):
        print("Text2 Edited")
        self.liststore[path][1] = text

    def show_menu(self, treeview, event):
        print("Show Menu")
        if event.button == 1:
            self.x1 = int(event.x)
            self.y1 = int(event.y)
            self.menu.show_all()
            self.menu.popup(None, None, None, None, 0, Gtk.get_current_event_time())
        return False

    def rename(self, *args):
        print("Rename")
        path_info = self.treeview.get_path_at_pos(self.x1, self.y1)
        if path_info is not None:
            path, col, cellx, celly = path_info
            self.treeview.grab_focus()
            self.treeview.set_cursor(path, col, True)
            
    def delete(self, *args):
        print("Delete")
        path_info = self.treeview.get_path_at_pos(self.x1, self.y1)
        if path_info is not None:
            path, col, cellx, celly = path_info
            self.treeview.grab_focus()
            self.treeview.set_cursor(path, col, True)
            if(col==self.column_text1):
                self.text_edited1(self.treeview, path, "")
            if(col==self.column_text2):
                self.text_edited2(self.treeview, path, "")

win = CellRendererTextWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()
