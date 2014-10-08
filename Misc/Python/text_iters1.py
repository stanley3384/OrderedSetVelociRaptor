#!/user/bin/python

#
# Test code for text iters and finding a word in a TextBox.
#
# C. Eric Cashon

from gi.repository import Gtk

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        self.set_wrap_mode(2)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("Find a word word word word1 word1 word2 word2 word2")
    
    def get_word(self, text1): ##text1 is word to match
        offset = 0
        prev = 0
        move = 0
        index = 1
        find_match = True
        start1 = self.textbuffer.get_start_iter()
        end1 = self.textbuffer.get_end_iter()
        print("Chars " + str(end1.get_offset()))
        if(text1):
            while find_match:
                match1 = start1.forward_search(text1, 0, None) 
                if(match1):
                    print(str(index) + " " + self.textbuffer.get_slice(match1[0], match1[1], include_hidden_chars=True) + " " + str(match1[0].get_offset()) +"-" + str(match1[1].get_offset()))            
                    offset = match1[1].get_offset()
                    move = offset - prev
                    prev = offset 
                    ##Move start1 iter forward.
                    start1.forward_chars(move)
                    index = index + 1
                else:
                    find_match=False
                    if(index==1):
                        print("No match found.")
        else:
            print("Empty entry.")

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Text Iters")
        self.set_default_size(400,400)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.button1 = Gtk.Button("Find Words")
        self.button1.connect("clicked", self.MatchWord)
        self.entry1 = Gtk.Entry()
        self.entry1.set_hexpand(True)
        self.grid = Gtk.Grid()
        self.grid.attach(self.button1, 0, 0, 1, 1)
        self.grid.attach(self.entry1, 1, 0, 1, 1)
        self.grid.attach(self.TextBox1, 0, 2, 2, 1)
        self.add(self.grid)

    def MatchWord(self, button):
        text1 = self.entry1.get_text()
        self.TextBox1.get_word(text1)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
