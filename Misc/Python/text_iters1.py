#!/user/bin/python

#
# Test code for text iters, text tags and finding a words in a TextBox.
#
# C. Eric Cashon

from gi.repository import Gtk

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        self.set_wrap_mode(2)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("Find a word word word word1 word1 word2 word2 word2")
        #apply some tags.
        self.start1 = self.textbuffer.get_start_iter()
        self.start1.forward_chars(7)
        self.start2 = self.textbuffer.get_start_iter()
        self.start2.forward_chars(11)
        self.tag1 = self.textbuffer.create_tag("green_tag", background="green")
        self.tag2 = self.textbuffer.create_tag("bold_tag", weight=900)
        self.textbuffer.apply_tag(self.tag1, self.start1, self.start2)
        self.start1.forward_chars(10)
        self.start2.forward_chars(10)
        self.textbuffer.apply_tag(self.tag1, self.start1, self.start2)
        self.textbuffer.apply_tag(self.tag2, self.start1, self.start2)
    
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

    def get_tag_table(self):
        tag_table = Gtk.TextTagTable()
        tag_table = self.textbuffer.get_tag_table()
        print("Tag Table Size " + str(tag_table.get_size()))
        tag_table.foreach(self.get_tags, None)
    
    def get_tags(self, tag, data):    
        loop=True
        switch=False
        offset1=0
        offset2=0
        start = self.textbuffer.get_start_iter()
        not_end=start.forward_to_tag_toggle(tag)
        while(loop):
            if(not_end):
                offset1=start.get_offset()
                if(switch):
                    print("Tag Found at " + str(offset2) + "-" + str(offset1) + " Tagname " + str(tag.get_property('name')))
                    switch=False
                else:
                    switch=True
                offset2=offset1
                not_end=start.forward_to_tag_toggle(tag)
            else:
                loop=False

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Text Iters")
        self.set_default_size(400,400)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.button1 = Gtk.Button("Find Words")
        self.button1.connect("clicked", self.MatchWord)
        self.button2 = Gtk.Button("Find Tags")
        self.button2.connect("clicked", self.FindTags)
        self.entry1 = Gtk.Entry()
        self.entry1.set_hexpand(True)
        self.grid = Gtk.Grid()
        self.grid.attach(self.button1, 0, 0, 1, 1)
        self.grid.attach(self.entry1, 1, 0, 1, 1)
        self.grid.attach(self.button2, 0, 1, 2, 1)
        self.grid.attach(self.TextBox1, 0, 2, 2, 1)
        self.add(self.grid)

    def MatchWord(self, button):
        print("Find Words")
        text1 = self.entry1.get_text()
        self.TextBox1.get_word(text1)

    def FindTags(self, button):
        print("Find Tags")
        self.TextBox1.get_tag_table()

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
