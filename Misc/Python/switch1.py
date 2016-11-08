
'''
    Test a delayed GTK switch. There is a da_circuit_breaker.c switch in the cairo folder also.
   
    GTK3.18

    C. Eric Cashon

'''

import gi
gi.require_version("Gtk","3.0")
from gi.repository import Gtk, GObject
import random

class MainWindow(Gtk.Window):
    def __init__ (self):
        Gtk.Window.__init__(self, title="Delayed Switch")
        self.set_size_request (200, 100)
        self.set_position(Gtk.WindowPosition.CENTER)

        self.timeout_id=0

        self.switch1 = Gtk.Switch()
        self.switch1.set_hexpand(True)
        self.switch1.set_vexpand(True)
        self.switch1.connect("state_set", self.switch_state)
        
        grid = Gtk.Grid()
        grid.attach(self.switch1, 0, 0, 1, 1)
        self.add(grid)

    def switch_state(self, switch, state):
        if(state):
            self.switch1.set_sensitive(False)
            GObject.timeout_add(1000, self.start_process)
            return True 
        else:
            print("Timout ID " + str(self.timeout_id))
            if(self.timeout_id > 0):
                print("Remove service check timer.")
                GObject.source_remove(self.timeout_id)
                self.timout_id = 0
            return False

    def start_process(self):
        num = random.random()
        if(num < 0.2):
            self.switch1.set_sensitive(True)
            self.switch1.set_state(True)
            print("Service started! Number " + str(num) + " Return False")
            #Keep check that the service is running.
            self.timeout_id = GObject.timeout_add(1000, self.service_killed)
            return False
        else:
            print("Number " + str(num) + " Return True")
            return True

    def service_killed(self):
        print("Check if service is running.")
        num = random.random()
        if(num < 0.15 and self.switch1.get_state()):
            print("Service Killed " + str(num))
            self.switch1.set_state(False)
            self.switch1.set_active(False)
            return False
        else:
            return True
        
window = MainWindow()
window.connect("delete_event", Gtk.main_quit)
window.show_all()
Gtk.main()


