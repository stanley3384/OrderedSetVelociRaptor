#!/usr/bin/python

"""
Test code for sending text to LibreOffice Writer from a GTK TextView. Change some Writer text 
formats in the script and save to a pdf in the working directory.

For LibreOffice dev files on Ubuntu
  sudo apt-get install libreoffice-dev
  sudo apt-get install python-uno
 
Uses python2.7

C. Eric Cashon
"""

import os
import subprocess
import time
import uno
import unohelper
from com.sun.star.text.ControlCharacter import PARAGRAPH_BREAK
from com.sun.star.awt.FontWeight import BOLD
from com.sun.star.awt.FontUnderline import SINGLE
from com.sun.star.beans import PropertyValue
from gi.repository import Gtk

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="GTK+ and LibreOffice")
        self.rgb_value1=0
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

    def intRGB(self, red, green, blue):
        self.rgb_value1 = (int(red) & 255) << 16 | (int(green) & 255) << 8 | (int(blue) & 255) 

    def office_connect(self, button):
        i = 0
        try:
            #Start Writer.
            print("Start Writer")
            subprocess.Popen(["libreoffice", "--writer", "--invisible"])
            pid = 0
            while(pid==0 and i<5):
                pid = os.popen("pidof soffice.bin").read()
                try:
                    pid = int(pid)
                except ValueError as e:
                    i+=1
                    pid = 0
                    time.sleep(1)
                print("Looking for PID " + str(pid))
            print("Connection to Office")
            if(pid!=0):               
                os.system('libreoffice "--accept=socket,host=localhost,port=2002;urp;"')                   
                #Get an app instance and send data to Writer
                print("Update Writer")
                localContext = uno.getComponentContext()
                resolver = localContext.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver', localContext)
                connection = 'uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager'
                j = 0
                while(j < 4):
                    try:
                        manager = resolver.resolve(connection)
                        j = 5
                    except:
                        print("Trying to Resolve Connection")
                        time.sleep(1)
                        j+=1 
                remoteContext = manager.getPropertyValue('DefaultContext')
                desktop = manager.createInstanceWithContext('com.sun.star.frame.Desktop', remoteContext)
                url = 'private:factory/swriter'
                doc = desktop.loadComponentFromURL( url, '_blank', 0, () )
                text = doc.Text
                cursor = text.createTextCursor()
                #Do some formating.
                text.insertString(cursor, "                                    ", 0)
                self.intRGB(255,0,0)
                cursor.setPropertyValue("CharColor", self.rgb_value1)
                cursor.setPropertyValue("CharWeight", BOLD);
                cursor.setPropertyValue("CharUnderline", SINGLE);
                text.insertString(cursor, "Automation with GTK+, Python and LibreOffice", 0)
                text.insertControlCharacter(cursor, PARAGRAPH_BREAK, 0)
                text.insertControlCharacter(cursor, PARAGRAPH_BREAK, 0)
                self.intRGB(0,255,0)
                cursor.setPropertyValue("CharColor", self.rgb_value1)
                cursor.setPropertyValue("CharWeight", 0);
                cursor.setPropertyValue("CharUnderline", 0);
                start1 = self.TextBox1.textbuffer.get_start_iter()
                end1 = self.TextBox1.textbuffer.get_end_iter()
                text.insertString(cursor, self.TextBox1.textbuffer.get_text(start1, end1, False), 0)
                text.insertControlCharacter(cursor, PARAGRAPH_BREAK, 0)
                self.intRGB(0,0,255)
                cursor.setPropertyValue("CharColor", self.rgb_value1)
                text.insertString(cursor, self.TextBox1.textbuffer.get_text(start1, end1, False), 0)
                text.insertControlCharacter(cursor, PARAGRAPH_BREAK, 0)
                self.intRGB(255,255,0)
                cursor.setPropertyValue("CharColor", self.rgb_value1)
                text.insertString(cursor, self.TextBox1.textbuffer.get_text(start1, end1, False), 0)
                text.insertControlCharacter(cursor, PARAGRAPH_BREAK, 0)
                self.intRGB(255,0,255)
                cursor.setPropertyValue("CharColor", self.rgb_value1)
                text.insertString(cursor, self.TextBox1.textbuffer.get_text(start1, end1, False), 0)
                #Save the document in working directory.
                print("Save Document")
                url = unohelper.systemPathToFileUrl(os.path.abspath("writer1.pdf"))
                args = (PropertyValue('FilterName', 0, "writer_pdf_Export", 0),)
                doc.storeToURL(url, args)
                #Close the document after saving.
                #doc.dispose()
            else:
                print("Couldn't Get Valid PID")
        except:
            print("Couldn't Resolve Connection")
            print("Press Button Again")
        
win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
