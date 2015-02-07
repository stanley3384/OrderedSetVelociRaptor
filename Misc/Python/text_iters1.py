#!/user/bin/python

"""
 Test code for text iters, text tags and finding words and formatting words in a TextBox.
 Worked on the Pango part but Pango for Python doesn't have Pango.attr_background_new(0, 65535, 0);
 along with other functions for attibutes. Use Pango markup instead for formatting text.

 C. Eric Cashon
"""

from gi.repository import Gtk, Pango
from operator import itemgetter

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        self.set_wrap_mode(2)
        self.set_cursor_visible(True)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("Find a word word word word1 word1 word2 word2 word2 word word1 word2")
        self.tag1 = self.textbuffer.create_tag("green_tag", background="green")
        self.tag2 = self.textbuffer.create_tag("bold_tag", weight=900)
    
    def get_word(self, text1, combo): ##text1 is word to match
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
                    if("combo1" == combo.get_name()):
                        if(int(combo.get_active_id()) == 1):
                            self.textbuffer.apply_tag(self.tag1, match1[0], match1[1])
                        if(int(combo.get_active_id()) == 2):
                            self.textbuffer.apply_tag(self.tag2, match1[0], match1[1])
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

    def get_tag_table(self, button, combo, label):
        tag_table = Gtk.TextTagTable()
        tag_table = self.textbuffer.get_tag_table()
        pango_tag_list=[]
        #package pointers together in a list.
        button_combo_list = [button, combo, label, pango_tag_list]
        tag_table.foreach(self.get_tag_filter, button_combo_list)
        #Update label. Two pango_tag_list pointers sent to load_pango_list. Shouldn't be a problem.
        if("button6" == button.get_name()):
            self.load_pango_list(pango_tag_list, button_combo_list)

    def get_tag_filter(self, tag, button_combo_list):
        if(button_combo_list[1]):
            tag_id = int(button_combo_list[1].get_active_id())
        else:
            tag_id = 0
        tag_name = tag.get_property('name')
        if(tag_id==0):
            print("Get All Tags")
            self.get_tags(tag, button_combo_list)
        elif(tag_id==1):
            if("green_tag" == tag_name):
                print("Get Green Tags")
                self.get_tags(tag, button_combo_list)
        else:
            if("bold_tag" == tag_name):
                print("Get Bold Tags")
                self.get_tags(tag, button_combo_list)

    def get_tags(self, tag, button_combo_list):    
        loop=True
        switch=False
        offset1=0
        offset2=0
        no_tags=False
        tag_start_list=[]
        start = self.textbuffer.get_start_iter()
        if(start.begins_tag(tag)):
            switch = True       
        not_end=start.forward_to_tag_toggle(tag) 
        if(not_end == False):
            no_tags=True      
        while(loop):
            if(not_end):
                offset1=start.get_offset()
                if(offset1 == offset2):
                        break
                if(switch):
                    print("Tag Found at " + str(offset2) + "-" + str(offset1) + " Tagname " + str(tag.get_property('name')))
                    tag_start_list.append(offset2)
                    button_combo_list[3].append(str(tag.get_property('name')))
                    button_combo_list[3].append(offset2)
                    button_combo_list[3].append(offset1)
                    switch=False
                else:
                    switch=True
                offset2=offset1
                not_end=start.forward_to_tag_toggle(tag)
            else:
                loop=False

        if(button_combo_list[0]):
            if(no_tags == False):
                if("button3"== button_combo_list[0].get_name()):
                    tag_start_list.sort(None, None, True)
                    self.move_backward_to_tag(tag_start_list, button_combo_list)
                if("button4" == button_combo_list[0].get_name()):
                    tag_start_list.sort()
                    self.move_forward_to_tag(tag_start_list, button_combo_list)
            else:
                print("No Tags")
        
    def move_forward_to_tag(self, tag_start_list, button_combo_list):
        iter1 = None 
        if("combo3" == (button_combo_list[1]).get_name()):
            self.grab_focus()
            cursor = self.textbuffer.get_property("cursor-position")
            cursor_changed=False
            for value in tag_start_list:
                if(value > cursor):
                    iter2 = self.textbuffer.get_iter_at_offset(value)
                    self.textbuffer.place_cursor(iter2)
                    cursor_changed=True
                    break               
            if(cursor_changed == False):
                iter1 = self.textbuffer.get_iter_at_offset(tag_start_list[0])
                if(iter1):
                    self.textbuffer.place_cursor(iter1)               
            print("Cursor Position " + str(self.textbuffer.get_property("cursor-position")))

    def move_backward_to_tag(self, tag_start_list, button_combo_list): 
        iter1 = None
        if("combo3" == (button_combo_list[1]).get_name()):
            self.grab_focus()
            cursor = self.textbuffer.get_property("cursor-position")
            cursor_changed=False
            for value in tag_start_list:
                if(value < cursor):
                    iter2 = self.textbuffer.get_iter_at_offset(value)
                    self.textbuffer.place_cursor(iter2)
                    cursor_changed=True
                    break               
            if(cursor_changed == False):
                iter1 = self.textbuffer.get_iter_at_offset(tag_start_list[0])
                if(iter1):
                    self.textbuffer.place_cursor(iter1)               
            print("Cursor Position " + str(self.textbuffer.get_property("cursor-position")))

    def remove_tags(self, widget):
        start = self.textbuffer.get_start_iter()
        end = self.textbuffer.get_end_iter()
        self.textbuffer.remove_all_tags(start, end)

    def load_pango_list(self, pango_tag_list, button_combo_list): 
        records = len(pango_tag_list)/3
        print("List Count " + str(records)) 
        start1 = self.textbuffer.get_start_iter()
        end1 = self.textbuffer.get_end_iter()
        text = self.textbuffer.get_text(start1, end1, False)
 
        #Reshape and sort pango_tag_list
        pango_reshape=[]
        pango_sorted=[]
        for i in range(records):
            pango_reshape.append([pango_tag_list[3*i], pango_tag_list[3*i+1], pango_tag_list[3*i+2]])
        #print(pango_reshape)
        pango_sorted = sorted(pango_reshape, key=itemgetter(1))
        print(pango_sorted) 
        
        #Add the markup. 
        s = ""
        open_tags = [False, False]
        span_open = False
        chars = len(text)+1
        for i in range(chars):
            if any(i in x for x in pango_sorted):
                #print("Found " + str(i))
                for j in range(records):
                    if("green_tag" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[0]=True
                    if("bold_tag" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[1]=True
                    if("green_tag" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i):
                        open_tags[0]=False
                    if("bold_tag" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[1]=False 
                if(span_open):
                    s+="</span>"
                    span_open = False
                #Check for open tags and build string.
                if(open_tags[0] or open_tags[1]):
                    s+="<span"
                    for k in range(len(open_tags)):  
                        if(open_tags[k] and k == 0):
                            s+=" background='green'"
                        if(open_tags[k] and k == 1):
                            s+=" weight='900'"
                    s+=">" 
                    span_open = True        
            if(i < chars-1):
                s+=str(text[i])
       
        print(s)
        button_combo_list[2].set_markup(s)
       
        """
        #Trouble getting Pango functions for attributes. Use markup instead
        attr = Pango.AttrList() 
        button_combo_list[2].set_text(text) 
        for i in range(records):
            print(str(pango_tag_list[3*i]))
            if("green_tag" == str(pango_tag_list[3*i])):
                print(str(dir(Pango)))
                attr_green = Pango.attr_background_new(0, 65535, 0);
                attr_green.start_index = pango_tag_list[3*i+1]
                attr_green.end_index = pango_tag_list[3*i+2]
                #gtk2 method
                #attr.insert(Pango.AttrBackground(0, 65535, 0, pango_tag_list[3*i+1], pango_tag_list[3*i+2]))
            if("bold_tag" == str(pango_tag_list[3*i])):
                #gtk2 method
                attr.insert(Pango.AttrWeight(900, pango_tag_list[3*i+1], pango_tag_list[3*i+2]))
        button_combo_list[2].set_attributes(attr)
        """

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Text and Tag Iters")
        self.set_default_size(400,450)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.button1 = Gtk.Button("Find Words")
        self.button1.connect("clicked", self.MatchWord)
        self.button2 = Gtk.Button("Find Tags")
        self.button2.connect("clicked", self.FindTags)
        self.button3 = Gtk.Button("Cursor Back")
        self.button3.set_name("button3")
        self.button3.connect("clicked", self.CursorBack)
        self.button4 = Gtk.Button("Cursor Forward")
        self.button4.set_name("button4")
        self.button4.connect("clicked", self.CursorForward)
        self.button5 = Gtk.Button("Remove Tags")
        self.button5.connect("clicked", self.RemoveTags) 
        self.button6 = Gtk.Button("Pango Tags")
        self.button6.set_name("button6")
        self.button6.connect("clicked", self.PangoTags)
        self.label1 = Gtk.Label("Pango text and tags.")
        self.label1.set_hexpand(True)
        self.label1.set_property("justify", 0)
        self.label1.set_line_wrap(True)              
        self.entry1 = Gtk.Entry()
        self.entry1.set_hexpand(True)
        self.combo1 = Gtk.ComboBoxText()
        self.combo1.append("1", "Green Tags")
        self.combo1.append("2", "Bold Tags")
        self.combo1.set_active_id("1")
        self.combo1.set_name("combo1")
        self.combo2 = Gtk.ComboBoxText()
        self.combo2.append("0", "All Tags")
        self.combo2.append("1", "Green Tags")
        self.combo2.append("2", "Bold Tags")
        self.combo2.set_active_id("0")
        self.combo2.set_name("combo2")
        self.combo3 = Gtk.ComboBoxText()
        self.combo3.append("1", "Green Tags")
        self.combo3.append("2", "Bold Tags")
        self.combo3.set_active_id("1")
        self.combo3.set_name("combo3")
        self.grid = Gtk.Grid()
        self.grid.attach(self.button1, 0, 0, 1, 1)
        self.grid.attach(self.entry1, 1, 0, 1, 1)
        self.grid.attach(self.combo1, 2, 0, 1, 1)
        self.grid.attach(self.button2, 0, 1, 1, 1)
        self.grid.attach(self.combo2, 1, 1, 1, 1)
        self.grid.attach(self.button3, 0, 2, 1, 1)
        self.grid.attach(self.button4, 1, 2, 1, 1)
        self.grid.attach(self.combo3, 2, 2, 1, 1)
        self.grid.attach(self.button5, 0, 3, 1, 1)
        self.grid.attach(self.scrolledwindow, 0, 4, 3, 1)
        self.grid.attach(self.button6, 0, 5, 1, 1)
        self.grid.attach(self.label1, 0, 6, 3, 1)
        self.add(self.grid)

    def MatchWord(self, button1):
        print("Find Words")
        text1 = self.entry1.get_text()
        self.TextBox1.get_word(text1, self.combo1)

    def FindTags(self, button2):
        print("Find Tags")
        self.TextBox1.get_tag_table(None, self.combo2, None)

    def CursorBack(self, button3):
        print("Move Back")
        self.TextBox1.get_tag_table(button3, self.combo3, None)

    def CursorForward(self, button4):
        print("Move Forward")
        self.TextBox1.get_tag_table(button4, self.combo3, None)

    def RemoveTags(self, button5):
        print("Remove Tags")
        self.TextBox1.remove_tags(button5)

    def PangoTags(self, button6):
        print("Pango Tags")
        self.TextBox1.get_tag_table(button6, None, self.label1)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
