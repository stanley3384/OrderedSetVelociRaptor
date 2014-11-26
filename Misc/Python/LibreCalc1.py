#!/user/bin/python

"""
Test code for automating LibreOffice Calc from a GTK+. Build a couple of 96-well plates and
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
from gi.repository import Gtk

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="GTK+ and LibreOffice")
        self.rgb_value1=0
        self.set_default_size(300,50)
        self.set_position(Gtk.WindowPosition.CENTER)
        self.button1 = Gtk.Button("Automate Calc")
        self.button1.set_hexpand(True)
        self.button1.set_vexpand(True)
        self.button1.connect("clicked", self.office_connect)
        self.grid = Gtk.Grid()
        self.grid.attach(self.button1, 0, 1, 1, 1)
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
            #Sleep a while to let Writer load. Doesn't always work.
            i = 0
            while(i<5):
                print("Waiting for Calc")
                time.sleep(1)
                i+=1
            #Try to get a connection to libreoffice.
            print("Connection to Office")
            if(pid!=0):               
                os.system('libreoffice "--accept=socket,host=localhost,port=2002;urp;"')                   
                #Get an app instance and send data to Writer
                print("Update Calc")
                localContext = uno.getComponentContext()
                resolver = localContext.ServiceManager.createInstanceWithContext('com.sun.star.bridge.UnoUrlResolver', localContext)
                connection = 'uno:socket,host=localhost,port=2002;urp;StarOffice.ServiceManager'
                manager = resolver.resolve(connection)
                remoteContext = manager.getPropertyValue('DefaultContext')
                desktop = manager.createInstanceWithContext('com.sun.star.frame.Desktop', remoteContext)
                url = 'private:factory/scalc'
                doc = desktop.loadComponentFromURL( url, '_blank', 0, () )
                print("Heatmap")
                #Some plate values.
                rows = 8
                columns = 12
                plates = 2
                row_labels = 0
                column_labels = 0
                test_number = 0.0
                high = 192.0
                low = 0.0
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
                    for row in range(rows):
                        for column in range(columns):
                            cell = sheet.getCellByPosition(column+2, (plate*(rows+1))+row+2)
                            self.heatmap_rgb(low, high, test_number)
                            cell.setPropertyValue("CellBackColor", self.rgb_value1)
                            cell.CharColor=0
                            cell.setValue(test_number)
                            test_number+=1
            else:
                print("Couldn't Get Valid PID")
        except:
            print("Couldn't Resolve Connection")
            print("Press Button Again")
        
win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
