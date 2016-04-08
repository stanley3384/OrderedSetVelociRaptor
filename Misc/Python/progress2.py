
'''
    Try some of the concepts from the progress2.c program in Python. The loop counters might need 
to be adjusted for testing with faster processors. 

    Tested on Ubuntu14.04 and Python2.7
    Change "import thread" to "import _thread" to use Python3

    C. Eric Cashon

'''
from gi.repository import Gtk, GLib
import thread

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Show Progress")
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_default_size(400, 200)

        #The counter variable is shared between threads.
        self.mutex = GLib.Mutex()
        GLib.Mutex.init(self.mutex)
        self.counter = 0.0  

        self.progress = Gtk.ProgressBar()
        self.progress.set_hexpand(True)
        self.progress.set_vexpand(True)
        self.progress.set_show_text(True)

        self.radio1 = Gtk.RadioButton.new_with_label_from_widget(None, "Lazy Load")
        self.radio2 = Gtk.RadioButton.new_from_widget(self.radio1)
        self.radio2.set_label("Thread")

        self.label1 = Gtk.Label("Compare Methods")
        self.label1.set_hexpand(True)

        self.button = Gtk.Button("Show Progress")
        self.button.connect("clicked", self.show_progress)  

        self.grid = Gtk.Grid()
        self.grid.attach(self.radio1, 0, 0, 1, 1)
        self.grid.attach(self.radio2, 0, 1, 1, 1)
        self.grid.attach(self.label1, 0, 2, 1, 1)
        self.grid.attach(self.button, 0, 3, 1, 1)
        self.grid.attach(self.progress, 0, 4, 1, 1)
        self.add(self.grid)

    def show_progress(self, widget):
        self.button.set_sensitive(False)

        if(self.radio1.get_active()):
            #Lazy load.
            GLib.idle_add(self.lazy_load_items, None, priority=GLib.PRIORITY_DEFAULT_IDLE)
        else:
            #Thread load.
            self.counter = .001
            thread.start_new_thread(self.thread_load_items, ("new_thread", ))
            #Need a timer to check the progress of the thread.
            self.timout_id = GLib.timeout_add(200, self.check_progress, None)
 
    #Single thread so no need for a mutex.
    def lazy_load_items(self, widget):
        print("Lazy Load " + str(self.counter))
        for i in range(1000):
            for j in range(1000):
                if(i == 999 and j == 999):
                    self.counter += 1.0
        
        if(self.counter <= 10):
            self.progress.set_fraction(self.counter / 10.0)
            return True
        else:
            self.button.set_sensitive(True)
            self.counter = -0.001
            self.progress.set_fraction(self.counter)
            return False

    #Use a mutex to protect against shared access.
    def thread_load_items(self, args):
        print("Thread Load")
        for i in range(10000):
            for j in range(1000):
                if(i % 999 == 0 and j == 999):
                    print("Check" + str(i))
                    self.mutex.lock()
                    self.counter += 1.0
                    self.mutex.unlock()

        self.mutex.lock()
        self.counter = -0.001
        self.mutex.unlock()

    #Check the progress of the working thread from the main thread.
    def check_progress(self, widget):
        print("Check Progress")
        temp = 0
        self.mutex.lock()
        temp = self.counter
        self.mutex.unlock()
        self.progress.set_fraction(temp / 10.0)
        if(self.counter > 0):
            return True
        else:
            self.button.set_sensitive(True)
            return False
                     
win = MainWindow()
win.connect("delete-event", Gtk.main_quit)
win.show_all()
Gtk.main()

