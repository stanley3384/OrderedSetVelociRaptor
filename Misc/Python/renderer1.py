
'''
    Automatically scale a treeview font size and height of cell. To scale the height of the cell,
add an extra column of height values and bind that to the column. The other properties can be set
as usual.

    Tested with Ubuntu14.04, GTK3.10 and Python2.7

    C. Eric Cashon

'''

from gi.repository import Gtk

class MainWindow(Gtk.Window):

    def __init__(self):
        Gtk.Window.__init__(self, title="Scale Text")

        self.set_default_size(200, 200)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.connect("size_allocate", self.size_changed)

        #The second value is the starting height of the cell.
        self.liststore = Gtk.ListStore(str, int)
        self.liststore.append(["Apples", 30])
        self.liststore.append(["Oranges", 30])
        self.liststore.append(["Pears", 30])

        self.treeview = Gtk.TreeView(model=self.liststore)
        self.treeview.set_vexpand(True)
        self.treeview.set_hexpand(True)

        self.renderer = Gtk.CellRendererText()
        self.renderer.set_property("font", "Arial 10")
        self.renderer.set_property("xalign", 0.5)
        self.renderer.set_property("background", "#ff00ff")

        #Set the column to get the cell height from.
        self.column = Gtk.TreeViewColumn("Fruit", self.renderer, text = 0, height = 1)
        self.column.set_expand(True)

        self.treeview.append_column(self.column)

        self.add(self.treeview)

    def size_changed(self, widget, data):
        width = self.get_allocated_width()
        height = self.get_allocated_height()        
        if(height > 200):
            scale = height / 20
            print(str(height) + " " + str(scale) + " " + str(scale + 20))
            #Set the height of the cells.
            iter1 = self.liststore.get_iter_first()         
            while(iter1 != None):
                self.liststore.set(iter1, 1, scale+20)
                iter1 = self.liststore.iter_next(iter1)  
            #Set the font and background.
            self.renderer.set_property("font", "Arial " + str(scale))
            self.renderer.set_property("background", "#ffff00")
        else:
            self.renderer.set_property("font", "Arial 10")
            self.renderer.set_property("background", "#ff00ff")

win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()
