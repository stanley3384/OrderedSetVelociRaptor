#!/user/bin/python

#
# Test code for capturing a signal from a TextTag
#
# C. Eric Cashon

from gi.repository import Gtk, Gdk

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        self.counter=0
        self.set_wrap_mode(2)
        self.set_has_tooltip(True)
        self.connect("motion-notify-event", self.SetToolTip)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("Some Text Some phrase.")
        self.start1 = self.textbuffer.get_start_iter()
        #Move start iter forward.
        self.start1.forward_chars(10)
        self.end1 = self.textbuffer.get_end_iter()
        self.tag1 = self.textbuffer.create_tag("yellow_tag", background="yellow")
        self.tag1.connect("event", self.TagEvent)
        self.textbuffer.apply_tag(self.tag1, self.start1, self.end1)

    def TagEvent(self, tag, widget, event, iter):
        print ("Tag Signal " + str(self.counter) + "  Tag Name " + str(tag.get_property("name"))  + "  Start Iter " + str(iter.get_offset()))
        if event.type == Gdk.EventType.MOTION_NOTIFY:
            print ("   Motion Notify Tag")
            self.set_tooltip_text("Different Language")
        if event.type == Gdk.EventType.BUTTON_PRESS:
            print ("   Clicked")
            dialog = Gtk.MessageDialog(None, 0, Gtk.MessageType.INFO, Gtk.ButtonsType.OK, "Different Language")
            dialog.run()
            print("Dialog Closed")
            dialog.destroy()
        self.counter+=1
        return True

    def SetToolTip(self, event, data):
        print ("Motion Notify TextBox")
        self.set_tooltip_text("TextBox")
        return False
   
class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Text Iters and Tag")
        self.set_default_size(300,200)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.grid = Gtk.Grid()
        self.grid.attach(self.TextBox1, 0, 0, 1, 1)
        self.add(self.grid)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
