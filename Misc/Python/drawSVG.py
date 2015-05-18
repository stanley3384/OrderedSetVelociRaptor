#!/user/bin/python

'''
   Test code for looking at Python2.7, GTK+ Cairo, CSS and SVG along with some animation.
From various items from the internet. Just some fun with the technology. The 
script needs the test1.svg to open.
   To get it to work with Python3.2 it needed

   sudo apt-get install python3-gi-cairo

for the drawing area to work correctly.

'''

from gi.repository import Gtk, GdkPixbuf, Gdk, GObject, cairo
import time
import random
import math

# Initialize circle coordinates and velocities.
class InitializeCircles:
    def __init__(self):
        self.num = 100
        self.size_x = 800
        self.size_y = 600
        self.x = []
        self.y = []
        self.xv = []
        self.yv = []
        for i in range(self.num):
            self.x.append(random.randint(0, self.size_x))
            self.y.append(random.randint(0, self.size_y))
            self.xv.append(random.randint(-5, 15))
            self.yv.append(random.randint(-5, 15))

class DialogTime(Gtk.MessageDialog):
    def __init__(self, parent):
        Gtk.MessageDialog.__init__(self, parent, 0, Gtk.MessageType.INFO,
            Gtk.ButtonsType.OK, "DateTime")
        time1 = time.strftime("%a  %b %d, %Y  %-I:%M:%S %p",time.localtime(time.time()))
        self.format_secondary_text(time1)
        self.set_default_size(150, 100)
        self.show_all()

class DialogMessage(Gtk.MessageDialog):
    def __init__(self, parent, message = "Default Message"):
        Gtk.MessageDialog.__init__(self, parent, 0, Gtk.MessageType.INFO,
            Gtk.ButtonsType.OK, "Info Message")
        self.format_secondary_text(message)
        self.set_default_size(150, 100)
        self.show_all()

class DialogFileChooser(Gtk.FileChooserDialog):
    def __init__(self, parent):
        Gtk.FileChooserDialog.__init__(self, "My Dialog", parent, 0,
            (Gtk.STOCK_CANCEL, Gtk.ResponseType.CANCEL,
             Gtk.STOCK_OK, Gtk.ResponseType.OK))
        self.set_default_size(150, 100)
        self.show_all()

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Test SVG")
        self.set_default_size(300,350);
        self.set_position(Gtk.WindowPosition.CENTER)
        self.connect_after('destroy', self.destroy)
         
        self.circles = InitializeCircles()

        button1 = Gtk.Button ("Open a picture...")
        button1.set_hexpand(True)
        button1.set_name("css_button1")
        button1.connect_after('clicked', self.on_button1_clicked)

        button2 = Gtk.Button ("What time is it?")
        button2.set_hexpand(True)
        button2.set_name("css_button2")
        button2.connect_after('clicked', self.on_button2_clicked)

        self.drawing_area = Gtk.DrawingArea()
        self.drawing_area.set_hexpand(True)
        self.drawing_area.set_vexpand(True)
        self.drawing_area.set_size_request(300, 300)
        self.drawing_area.connect("draw", self.on_draw)
        self.drawing_area.connect("draw", self.expose)

        self.grid = Gtk.Grid()
        self.grid.attach(button1, 0, 0, 1, 1)
        self.grid.attach(button2, 0, 1, 1, 1)
        self.grid.attach(self.drawing_area, 0, 2, 1, 1)
        self.add(self.grid)

        #Add the CSS linear gradients.
        style_provider = Gtk.CssProvider()
        css = b"""GtkButton#css_button1{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,255,0,1)), color-stop(1.0,rgba(0,255,255,1)));}
                 GtkButton#css_button2{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(0,255,255,1)), color-stop(1.0,rgba(255,255,0,1)));}
                 GtkMessageDialog{background-image: -gtk-gradient (linear, left center, right center, color-stop(0.0,rgba(255,255,0,1)), color-stop(1.0,rgba(255,0,255,1)));}"""
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider, Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)
                
        self.show_all()
        try:
            self.pb = GdkPixbuf.Pixbuf.new_from_file_at_scale("test1.svg", 300, 300, False)
        except:
            dialog = DialogMessage(self, "Couldn't find test1.svg.")
            dialog.run()
            dialog.destroy()
        GObject.idle_add(self.timeout)

    #Text to draw over SVG.
    def on_draw(self, wid, cr):
        width = self.drawing_area.get_allocated_width()
        height = self.drawing_area.get_allocated_height()
        pixbuf = self.pb.scale_simple(width, height, 2)
        Gdk.cairo_set_source_pixbuf(cr, pixbuf, 5, 5)
        cr.paint()
        cr.set_font_size(25)
        '''
        Use constants from Cairo documentation for
        select_font_face("Serif", enum cairo_font_slant_t, enum cairo_font_weight_t)
        http://cairographics.org/manual/cairo-text.html
        ''' 
        cr.select_font_face("Serif", 0, 1)
        cr.set_source_rgb(0.9 , 0.9 , 0.0)
        cr.move_to(50 , 250)
        cr.rotate(-3.14/6)
        cr.show_text("Cairo Text Over SVG")
        cr.stroke() 

    # Draw the circles and update their positions.
    def expose(self, wid, cr):
        cr.set_line_width(4)
        for i in range(self.circles.num):
            cr.set_source_rgb(0, 1, 0)
            cr.translate(-7, 0)
            cr.arc(self.circles.x[i], self.circles.y[i], 8, 0, 2 * math.pi)
            cr.stroke_preserve()
            cr.set_source_rgb(1, 1, 1)
            cr.fill()
            self.circles.x[i] += self.circles.xv[i]
            self.circles.y[i] += self.circles.yv[i]
            if self.circles.x[i] > self.circles.size_x or self.circles.x[i] < 0:
                self.circles.xv[i] = -self.circles.xv[i]
            if self.circles.y[i] > self.circles.size_y or self.circles.y[i] < 0:
                self.circles.yv[i] = -self.circles.yv[i]

    def timeout(self):
        self.drawing_area.queue_draw()
        return True

    def on_button1_clicked(self, widget):
        dialog = DialogFileChooser(self)
        response = dialog.run()

        if response == Gtk.ResponseType.OK:
            try:
                self.pb = GdkPixbuf.Pixbuf.new_from_file(dialog.get_filename())
            except:
                dialog2 = DialogMessage(self, "The file isn't a recognized graphics file.")
                dialog2.run()
                dialog2.destroy()
        elif response == Gtk.ResponseType.CANCEL:
            print("The Cancel button was clicked")
        dialog.destroy()

    def on_button2_clicked(self, widget):
        dialog = DialogTime(self)
        dialog.run()
        dialog.destroy()

    def destroy(window, self):
        Gtk.main_quit()

def main():    
    app = MainWindow()
    Gtk.main()
        
if __name__ == "__main__":    
    main()

