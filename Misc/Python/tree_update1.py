
'''

    Test sqlite with a gtk treeview. Load data into a table at startup and retrieve the data with a 
button press. 

    Ubuntu14.04 and GTK3.10

    C. Eric Cashon

'''
import sqlite3 as sqlite
import gi
gi.require_version("Gtk","3.0")
from gi.repository import Gtk, Pango

class MainWindow(Gtk.Window):
    def __init__ (self):
        Gtk.Window.__init__(self, title="Plants")
        self.set_size_request (700, 400)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_border_width(10)
        self.connect("realize", self.create_sqlite_table) 

        self.store = Gtk.ListStore(int, str, str, str, int, str)

        tree = Gtk.TreeView()
        tree.set_hexpand(True)
        tree.set_vexpand(True)
        tree.set_model(model = self.store)

        renderer1 = Gtk.CellRendererText(scale=1.5)
        renderer2 = Gtk.CellRendererText(scale=2)
        renderer3 = Gtk.CellRendererText(underline=True)
        renderer4 = Gtk.CellRendererText(weight=900)
        renderer5 = Gtk.CellRendererText(font="Arial 16", xalign=0.9)

        #Set background color in last liststore column.
        column1 = Gtk.TreeViewColumn ("ID", renderer1, text=0, background=5)
        column2 = Gtk.TreeViewColumn ("Common Name", renderer2, text=1, background=5)
        column3 = Gtk.TreeViewColumn ("Latin Name", renderer3, text=2, background=5)
        column4 = Gtk.TreeViewColumn ("Plant Type", renderer4, text=3, background=5)
        column5 = Gtk.TreeViewColumn ("In Store", renderer5, text=4, background=5)

        tree.append_column(column1)
        tree.append_column(column2)
        tree.append_column(column3)
        tree.append_column(column4)
        tree.append_column(column5)

        scroll = Gtk.ScrolledWindow()
        scroll.add(tree)

        button = Gtk.Button(label = "Load Data")
        button.set_hexpand(True)
        button.connect("clicked", self.add_list)

        grid = Gtk.Grid()
        grid.attach(scroll, 0, 0, 1, 1)
        grid.attach(button, 0, 1, 1, 1)
        self.add(grid)

    def add_list (self, widget): 
        print("Add List")
        con = sqlite.connect('nursery.db')       
        cur = con.cursor()    
        cur.execute("SELECT * FROM plants")
        rows = cur.fetchall()
        for row in rows:
            print(row)
            self.store.append(row)

    def create_sqlite_table(self, widget):
        print("Realize")
        con = sqlite.connect('nursery.db')
        cur = con.cursor()  

        cur.executescript("""
        BEGIN TRANSACTION;
        DROP TABLE IF EXISTS plants;
        CREATE TABLE plants(Id INT, common_name TEXT, latin_name TEXT, general_type TEXT, in_store INT, row_color TEXT);
        INSERT INTO plants VALUES(1, 'Western Red Cedar', 'Thuja plicata', 'Tree', 6, 'cyan');
        INSERT INTO plants VALUES(2, 'Highbush Blueberry', 'Vaccinium corymbosum', 'Shrub', 35, 'yellow');
        INSERT INTO plants VALUES(3, 'Purple Cone Flower', 'Echinacea purpurea', 'Forb', 112, 'lime green');
        COMMIT TRANSACTION;
        """)
        print("Database Loaded")
        
window = MainWindow()
window.connect("delete_event", Gtk.main_quit)
window.show_all()
Gtk.main()

