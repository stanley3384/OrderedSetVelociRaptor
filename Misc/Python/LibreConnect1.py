#!/user/bin/python

"""
Test code for sending text to LibreOffice Writer from a GTK TextView. For it to work, an instance of 
LibreOffice Writer needs to be started. Then the program can connect to the running instance and
send it the text data.

For LibreOffice dev files on Ubuntu
  sudo apt-get install libreoffice-dev
  sudo sudo apt-get install python-uno
 
Uses python2.7

C. Eric Cashon
"""

import os
import subprocess
import time
import uno
from gi.repository import Gtk

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="GTK+ and LibreOffice")
        self.set_default_size(400,200)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.TextBox1 = Gtk.TextView()
        self.TextBox1.set_wrap_mode(2)
        self.TextBox1.textbuffer = self.TextBox1.get_buffer() 
        self.TextBox1.textbuffer.set_text("Copy text from GTK Textview to LibreOffice Writer.")
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.button1 = Gtk.Button("Send Text to Writer")
        self.button1.connect("clicked", self.office_connect)
        self.grid = Gtk.Grid()
        self.grid.attach(self.TextBox1, 0, 2, 2, 1)
        self.grid.attach(self.button1, 0, 3, 2, 1)
        self.add(self.grid)

    def office_connect(self, button):
        try:
            print("Start Writer")
            subprocess.Popen(["libreoffice", "--writer", "--invisible"])
            pid = 0
            while(pid==0):
               pid = os.popen("pidof soffice.bin").read()
               try:
                   pid = int(pid)
               except ValueError as e:
                   pid = 0
               print(pid)
            time.sleep(4) #Added time to load.
            print("Connection to Office");
            os.system('libreoffice "--accept=socket,host=localhost,port=2002;urp;"')
            print("Update Writer")
            localContext = uno.getComponentContext()
            resolver = localContext.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver', localContext)
            connection = 'uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager'
            manager = resolver.resolve(connection)
            remoteContext = manager.getPropertyValue('DefaultContext')
            desktop = manager.createInstanceWithContext('com.sun.star.frame.Desktop', remoteContext)
            url = 'private:factory/swriter'
            doc = desktop.loadComponentFromURL( url, '_blank', 0, () )
            text = doc.Text
            text_range = text.End
            start1 = self.TextBox1.textbuffer.get_start_iter()
            end1 = self.TextBox1.textbuffer.get_end_iter()
            text_range.String = self.TextBox1.textbuffer.get_text(start1, end1, False)
        except:
            print("Couldn't Resolve Connection")
            print("Press Button Again")

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
