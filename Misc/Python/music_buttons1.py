#!/usr/bin/python

'''
    Some test music buttons using GTK2 and Python2.7.

    C. Eric Cashon
'''

import gtk
import cairo

class MainWindow(gtk.Window):
    def __init__(self):
        gtk.Window.__init__(self)
        self.set_title("Music Buttons")
        self.set_default_size(400, 50)
        self.set_position(gtk.WIN_POS_CENTER)

        self.toggle1 = gtk.ToggleButton(label="Change Scale", use_underline=False)
        self.toggle1.set_app_paintable(True)
        self.toggle1.connect("expose-event", self.draw_play_toggle)

        self.button1 = gtk.Button(label="Play", use_underline=False)
        self.button1.set_app_paintable(True)
        self.button1.connect("expose-event", self.draw_stop_toggle)
        self.button1.connect("button-press-event", self.stop_press)
        self.button1.connect("button-release-event", self.stop_release)

        self.button2 = gtk.Button(label=None, use_underline=False)
        self.button2.set_app_paintable(True)
        self.button2.connect("expose-event", self.draw_forward)
        self.button2.connect("button-press-event", self.forward_press)
        self.button2.connect("button-release-event", self.forward_release)

        self.button3 = gtk.Button(label=None, use_underline=False)
        self.button3.set_app_paintable(True)
        self.button3.connect("expose-event", self.draw_backward)
        self.button3.connect("button-press-event", self.backward_press)
        self.button3.connect("button-release-event", self.backward_release)

        #Put your widgets in a table.
        table = gtk.Table()
        table.attach_defaults(self.button3, 0, 2, 0, 1)
        table.attach_defaults(self.button2, 2, 4, 0, 1)
        table.attach_defaults(self.toggle1, 4, 5, 0, 1)
        table.attach_defaults(self.button1, 5, 6, 0, 1)

        #Add the table to the window.
        self.add(table)
    
    def draw_play_toggle(self, event, widget):
        #Draw on the toggle button window.
        width = self.toggle1.allocation.width
        height = self.toggle1.allocation.height
        win = self.toggle1.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 150.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        if(self.toggle1.get_active()):
            #The pause drawing.
            cr.set_source_rgb(0.25, 0.25, 0.25)
            cr.paint()
            cr.set_source_rgb(1.0, 1.0, 1.0)
            cr.set_line_width(6)
            cr.move_to(center_x - 6, center_y - 15)
            cr.line_to(center_x - 6, center_y + 15)
            cr.stroke()
            cr.move_to(center_x + 6, center_y - 15)
            cr.line_to(center_x + 6, center_y + 15)
            cr.stroke()
        else:
            #The play drawing
            cr.set_source_rgb(0.25, 0.25, 0.25)
            cr.paint()
            cr.set_source_rgb(0.0, 1.0, 1.0)
            cr.set_line_width(6)
            cr.move_to(center_x - 10, center_y - 15)
            cr.line_to(center_x - 10, center_y + 15)
            cr.line_to(center_x + 20, center_y)
            cr.line_to(center_x - 10 , center_y - 15)
            cr.fill()
            cr.stroke()

        return True

    def draw_stop_toggle(self, event, widget):
        #Draw on the toggle button window.
        width = self.button1.allocation.width
        height = self.button1.allocation.height
        win = self.button1.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 120.0
        scale_y = height / 40.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(0.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x - 10, center_y - 10)
        cr.rectangle(center_x - 10, center_y - 10, 40, 20)
        cr.fill()
        cr.stroke()

        return True

    def stop_press(self, event, widget):
        #Draw on the toggle button window.
        width = self.button1.allocation.width
        height = self.button1.allocation.height
        win = self.button1.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 120.0
        scale_y = height / 40.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(1.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x - 10, center_y - 10)
        cr.rectangle(center_x - 10, center_y - 10, 40, 20)
        cr.fill()
        cr.stroke()

        return True

    def stop_release(self, event, widget):
        #Draw on the toggle button window.
        width = self.button1.allocation.width
        height = self.button1.allocation.height
        win = self.button1.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 120.0
        scale_y = height / 40.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(0.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x - 10, center_y - 10)
        cr.rectangle(center_x - 10, center_y - 10, 40, 20)
        cr.fill()
        cr.stroke()

        return True

    def draw_forward(self, event, widget):
        #Draw on the toggle button window.
        width = self.button2.allocation.width
        height = self.button2.allocation.height
        win = self.button2.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 100.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(0.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x - 30, center_y - 15)
        cr.line_to(center_x - 30, center_y + 15)
        cr.line_to(center_x + 0, center_y)
        cr.line_to(center_x - 30 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 5, center_y - 15)
        cr.line_to(center_x + 5, center_y + 15)
        cr.line_to(center_x + 35, center_y)
        cr.line_to(center_x + 5 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 40, center_y - 15)
        cr.line_to(center_x + 40, center_y + 15)
        cr.stroke()

        return True

    def forward_press(self, event, widget):
        #Draw on the toggle button window.
        width = self.button2.allocation.width
        height = self.button2.allocation.height
        win = self.button2.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 100.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(1.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x - 30, center_y - 15)
        cr.line_to(center_x - 30, center_y + 15)
        cr.line_to(center_x + 0, center_y)
        cr.line_to(center_x - 30 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 5, center_y - 15)
        cr.line_to(center_x + 5, center_y + 15)
        cr.line_to(center_x + 35, center_y)
        cr.line_to(center_x + 5 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 40, center_y - 15)
        cr.line_to(center_x + 40, center_y + 15)
        cr.stroke()

        return True

    def forward_release(self, event, widget):
        #Draw on the toggle button window.
        width = self.button2.allocation.width
        height = self.button2.allocation.height
        win = self.button2.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 100.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(0.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x - 30, center_y - 15)
        cr.line_to(center_x - 30, center_y + 15)
        cr.line_to(center_x + 0, center_y)
        cr.line_to(center_x - 30 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 5, center_y - 15)
        cr.line_to(center_x + 5, center_y + 15)
        cr.line_to(center_x + 35, center_y)
        cr.line_to(center_x + 5 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 40, center_y - 15)
        cr.line_to(center_x + 40, center_y + 15)
        cr.stroke()

        return True

    def draw_backward(self, event, widget):
        #Draw on the toggle button window.
        width = self.button3.allocation.width
        height = self.button3.allocation.height
        win = self.button3.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 100.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(0.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x, center_y - 15)
        cr.line_to(center_x, center_y + 15)
        cr.line_to(center_x - 30, center_y)
        cr.line_to(center_x, center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 35, center_y - 15)
        cr.line_to(center_x + 35, center_y + 15)
        cr.line_to(center_x + 5, center_y)
        cr.line_to(center_x + 35 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x - 40, center_y - 15)
        cr.line_to(center_x - 40, center_y + 15)
        cr.stroke()

        return True

    def backward_press(self, event, widget):
        #Draw on the toggle button window.
        width = self.button3.allocation.width
        height = self.button3.allocation.height
        win = self.button3.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 100.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(1.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x, center_y - 15)
        cr.line_to(center_x, center_y + 15)
        cr.line_to(center_x - 30, center_y)
        cr.line_to(center_x, center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 35, center_y - 15)
        cr.line_to(center_x + 35, center_y + 15)
        cr.line_to(center_x + 5, center_y)
        cr.line_to(center_x + 35 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x - 40, center_y - 15)
        cr.line_to(center_x - 40, center_y + 15)
        cr.stroke()

        return True

    def backward_release(self, event, widget):
        #Draw on the toggle button window.
        width = self.button3.allocation.width
        height = self.button3.allocation.height
        win = self.button3.get_event_window()
        cr = win.cairo_create()

        #Scale the drawing for window resize. Use cr.scale().
        scale_x = width / 100.0
        scale_y = height / 50.0
        center_x = (width / 2.0) / scale_x 
        center_y = (height / 2.0) / scale_y  
        cr.scale(scale_x, scale_y)
      
        cr.set_source_rgb(0.25, 0.25, 0.25)
        cr.paint()
        cr.set_source_rgb(0.0, 1.0, 1.0)
        cr.set_line_width(6)
        cr.move_to(center_x, center_y - 15)
        cr.line_to(center_x, center_y + 15)
        cr.line_to(center_x - 30, center_y)
        cr.line_to(center_x, center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x + 35, center_y - 15)
        cr.line_to(center_x + 35, center_y + 15)
        cr.line_to(center_x + 5, center_y)
        cr.line_to(center_x + 35 , center_y - 15)
        cr.fill()
        cr.stroke()

        cr.move_to(center_x - 40, center_y - 15)
        cr.line_to(center_x - 40, center_y + 15)
        cr.stroke()

        return True
       
win = MainWindow()
win.connect("delete-event", gtk.main_quit)
win.show_all()
gtk.main()

