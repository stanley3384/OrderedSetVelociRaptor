
'''
    A python script to display a login box with GTK+ before the main application starts. Similar
to the C code in The Encryption folder. 

    Tested with Ubuntu14.04, GTK3.10 and Python2.7

C. Eric Cashon
'''

from gi.repository import Gtk

start_program = True

class DialogWindow(Gtk.Dialog):
    def __init__(self, main_window):
        Gtk.Dialog.__init__(self, "Login", main_window, 0, ("Cancel", Gtk.ResponseType.CANCEL, "OK", Gtk.ResponseType.OK))
        self.set_default_size(200, 100);

        global start_program

        self.label1 = Gtk.Label("User Name")
        self.label1.set_hexpand(True)
        self.label2 = Gtk.Label("Password")
        self.label2.set_hexpand(True)

        self.entry1 = Gtk.Entry()
        self.entry1.set_hexpand(True)
        self.entry1.set_text("Admin")
        self.entry2 = Gtk.Entry()
        self.entry2.set_hexpand(True)
        self.entry2.set_text("password")

        box = self.get_content_area()
        box.add(self.label1)
        box.add(self.entry1)
        box.add(self.label2)
        box.add(self.entry2)

        self.show_all()
    
        response = self.run()
        if response == Gtk.ResponseType.OK:
            print("OK")
            #Check password and user name.
            if("password" == self.entry2.get_text()):
                main_window.label1.set_text(self.entry1.get_text())
                main_window.label2.set_text(self.entry2.get_text())
            else:
                start_program = False
        else:
            print("Cancel")
            start_program = False            

        self.destroy()
           
class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Show Login Entry Values")
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_default_size(300, 200) 
        self.connect("realize", self.login_dialog) 

        self.label1 = Gtk.Label("Fred")
        self.label1.set_hexpand(True)
        self.label1.set_vexpand(True)
        self.label2 = Gtk.Label("password")
        self.label2.set_hexpand(True)
        self.label2.set_vexpand(True)
        self.button1 = Gtk.Button("Test Click")
        self.button1.connect('clicked', self.button_clicked)

        self.grid = Gtk.Grid()
        self.grid.attach(self.label1, 0, 0, 1, 1)
        self.grid.attach(self.label2, 0, 1, 1, 1)
        self.grid.attach(self.button1, 0, 2, 1, 1)
        self.add(self.grid)

    def button_clicked(self, widget):
        print("Clicked")

    def login_dialog(self, widget):
        dialog = DialogWindow(self)
                     
win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
print("Start " + str(start_program))
if(start_program):
    Gtk.main()

