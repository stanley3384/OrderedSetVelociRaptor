#!/user/bin/python

"""
Test code for automating LibreOffice Calc from GTK+. Build a couple of 96-well plates and
heatmap them.

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
from com.sun.star.beans import PropertyValue
from gi.repository import Gtk, Gdk

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="GTK+ and LibreOffice")
        self.rgb_value1=0
        self.set_default_size(250,50)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.set_border_width(25)

        style_provider = Gtk.CssProvider()
        #Add b for byte string error in python3.2. Works in 2.7 also.
        css = b"""GtkWindow{background: green}
                  GtkLabel{color: black}
                  GtkButton{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,0,0,1)), color-stop(0.5,rgba(0,255,0,1)), color-stop(1.0,rgba(0,0,255,1)));}"""
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)

        self.label1 = Gtk.Label("Send a sequence of numbers\n to Calc and heatmap them.\n Test with small numbers.\n It is a bit slow.")
        self.label2 = Gtk.Label("Plates")
        self.label3 = Gtk.Label("Rows")
        self.label4 = Gtk.Label("Columns")
        self.entry1 = Gtk.Entry()
        self.entry1.set_hexpand(False)
        self.entry1.set_width_chars(2)
        self.entry1.set_text("2")
        self.entry2 = Gtk.Entry()
        self.entry2.set_hexpand(False)
        self.entry2.set_width_chars(2)
        self.entry2.set_text("8")
        self.entry3 = Gtk.Entry()
        self.entry3.set_hexpand(False)
        self.entry3.set_width_chars(2)
        self.entry3.set_text("12")
        self.button1 = Gtk.Button("Automate Calc")
        self.button1.connect("clicked", self.office_connect)

        self.grid = Gtk.Grid()
        self.grid.set_row_spacing(10)
        self.grid.set_column_spacing(80)
        self.grid.attach(self.label1, 0, 1, 2, 1)
        self.grid.attach(self.label2, 0, 2, 1, 1)
        self.grid.attach(self.label3, 0, 3, 1, 1)
        self.grid.attach(self.label4, 0, 4, 1, 1)
        self.grid.attach(self.entry1, 1, 2, 1, 1)
        self.grid.attach(self.entry2, 1, 3, 1, 1)
        self.grid.attach(self.entry3, 1, 4, 1, 1)
        self.grid.attach(self.button1, 0, 5, 2, 1)
        self.add(self.grid)

    def intRGB(self, red, green, blue):
        self.rgb_value1 = (int(red) & 255) << 16 | (int(green) & 255) << 8 | (int(blue) & 255) 

    def heatmap_rgb(self, minimum, maximum, value):
        ratio = 2 * (value-minimum) / (maximum - minimum)
        blue = int(max(0, 255*(1 - ratio)))
        red = int(max(0, 255*(ratio - 1)))
        green = 255 - blue - red
        #print str(value) + " " +str(red) + " " + str(green) + " " + str(blue)
        self.intRGB(red, green, blue)

    def update_calc(self, doc):
        print("Heatmap")
        #Some plate values.
        plates = int(self.entry1.get_text())
        rows = int(self.entry2.get_text())
        columns = int(self.entry3.get_text())
        row_labels = 0
        column_labels = 0
        test_number = 0.0
        #get a sheet to add the numbers to.
        sheet = doc.getSheets().getByIndex(0)
        for plate in range(plates):
            self.intRGB(224,224,224) #Labels as grey.
            #Row labels
            for row in range(rows+1):
                cell = sheet.getCellByPosition(1, (plate*(rows+1))+row+1)
                cell.setPropertyValue("CellBackColor", self.rgb_value1)
                cell.setValue(row)
            #Column labels
            for column in range(columns+1):
                cell = sheet.getCellByPosition(column+1, (plate*(rows+1))+1)
                cell.setPropertyValue("CellBackColor", self.rgb_value1)
                cell.setValue(column)
            #Fill in numbers
            low = float(plate*rows*columns)
            high = low + float(rows * columns - 1)
            print str(low) + " " + str(high)
            for row in range(rows):
                for column in range(columns):
                    cell = sheet.getCellByPosition(column+2, (plate*(rows+1))+row+2)
                    self.heatmap_rgb(low, high, test_number)
                    cell.setPropertyValue("CellBackColor", self.rgb_value1)
                    cell.CharColor=0
                    cell.setValue(test_number)
                    test_number+=1    

    def office_connect(self, button):
        i = 0
        try:
            #Start Writer.
            print("Start Calc")
            subprocess.Popen(["libreoffice", "--calc", "--invisible"])
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
            #Sleep a while to let Calc load. Doesn't always work. Sometimes have to press
            #the button twice.
            i = 0
            while(i<5):
                print("Waiting for Calc")
                time.sleep(1)
                i+=1
            #Try to get a connection to libreoffice.
            print("Connection to Office")
            if(pid!=0):               
                os.system('libreoffice "--accept=socket,host=localhost,port=2002;urp;"')                   
                localContext = uno.getComponentContext()
                resolver = localContext.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver', localContext)
                connection = 'uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager'
                manager = resolver.resolve(connection)
                remoteContext = manager.getPropertyValue('DefaultContext')
                desktop = manager.createInstanceWithContext('com.sun.star.frame.Desktop', remoteContext)
                url = 'private:factory/scalc'
                doc = desktop.loadComponentFromURL( url, '_blank', 0, () )
                print("Update Calc")
                self.update_calc(doc)
            else:
                print("Couldn't Get Valid PID")
        except:
            print("Couldn't Resolve Connection")
            print("Press Button Again")
        
win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
