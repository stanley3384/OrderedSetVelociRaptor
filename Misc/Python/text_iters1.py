#!/user/bin/python

"""
 Test code for text iters, text tags, finding words and formatting words in a textbox, label and printing.
 Worked on the Pango part but Pango for Python doesn't have Pango.attr_background_new(0, 65535, 0);
 along with other functions for attibutes. Use Pango markup instead for formatting text.

 C. Eric Cashon
"""

from gi.repository import Gtk, Gdk, Pango, PangoCairo
from operator import itemgetter

class TextBox(Gtk.TextView):
    def __init__(self):
        Gtk.TextView.__init__(self)
        self.set_wrap_mode(2)
        self.set_cursor_visible(True)
        self.textbuffer = self.get_buffer() 
        self.textbuffer.set_text("Find a word word word word1 word1 word2 word2 word2 word word1 word2")
        self.tag1 = self.textbuffer.create_tag("background='green'", background="green")
        self.tag2 = self.textbuffer.create_tag("weight='900'", weight=900)
    
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
        if(button):
            if("button6" == button.get_name() or "button7" == button.get_name() or "button8" == button.get_name() or "menuitem3" == button.get_name()):
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
            if("background='green'" == tag_name):
                print("Get Green Tags")
                self.get_tags(tag, button_combo_list)
        else:
            if("weight='900'" == tag_name):
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
        records = int(len(pango_tag_list)/3)
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
        self.markup_string = ""
        open_tags = [False, False]
        span_open = False
        chars = len(text)+1
        for i in range(chars):
            if any(i in x for x in pango_sorted):
                #print("Found " + str(i))
                for j in range(records):
                    if("background='green'" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[0]=True
                    if("weight='900'" == str(pango_sorted[j][0]) and pango_sorted[j][1] == i):
                        open_tags[1]=True
                    if("background='green'" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i):
                        open_tags[0]=False
                    if("weight='900'" == str(pango_sorted[j][0]) and pango_sorted[j][2] == i): 
                        open_tags[1]=False 
                if(span_open):
                    self.markup_string+="</span>"
                    span_open = False
                #Check for open tags and build string.
                if(open_tags[0] or open_tags[1]):
                    self.markup_string+="<span"
                    for k in range(len(open_tags)):  
                        if(open_tags[k] and k == 0):
                            self.markup_string+=" background='green'"
                        if(open_tags[k] and k == 1):
                            self.markup_string+=" weight='900'"
                    self.markup_string+=">" 
                    span_open = True        
            if(i < chars-1):
                self.markup_string+=str(text[i])
       
        print(self.markup_string)

        #Set markup to label
        button_combo_list[2].set_markup(self.markup_string)
        #Print markup
        if("button7"== button_combo_list[0].get_name()):
            self.print_dialog()
        if("button8"== button_combo_list[0].get_name()):
            self.cycle_names_textbox()
        if("menuitem3"== button_combo_list[0].get_name()):
            print("Write Markup to PangoMarkup.txt")
            self.write_markup_to_file()
            
       
        """
        #Trouble getting Pango functions for attributes. Use markup instead
        attr = Pango.AttrList() 
        button_combo_list[2].set_text(text) 
        for i in range(records):
            print(str(pango_tag_list[3*i]))
            if("background='green'" == str(pango_tag_list[3*i])):
                print(str(dir(Pango)))
                attr_green = Pango.attr_background_new(0, 65535, 0);
                attr_green.start_index = pango_tag_list[3*i+1]
                attr_green.end_index = pango_tag_list[3*i+2]
                #gtk2 method
                #attr.insert(Pango.AttrBackground(0, 65535, 0, pango_tag_list[3*i+1], pango_tag_list[3*i+2]))
            if("weight='900'" == str(pango_tag_list[3*i])):
                #gtk2 method
                attr.insert(Pango.AttrWeight(900, pango_tag_list[3*i+1], pango_tag_list[3*i+2]))
        button_combo_list[2].set_attributes(attr)
        """

    #Simple print.
    def print_dialog(self):
        print("Print")
        operation = Gtk.PrintOperation()
        #Number of pages must be set. Should calculate the number of pages.
        operation.set_n_pages(1)
        operation.connect("begin_print", self.begin_print)
        operation.connect("draw_page", self.draw_page)
        result = operation.run(Gtk.PrintOperationAction.PRINT_DIALOG, None)

    def begin_print(self, operation, gtk_context):
        page_width=gtk_context.get_width();
        pango_context = self.get_pango_context()
        description = pango_context.get_font_description()
        self.pango_layout = gtk_context.create_pango_layout()
        self.pango_layout.set_font_description(description)
        self.pango_layout.set_width(page_width*Pango.SCALE);
        self.pango_layout.set_wrap(Pango.WrapMode.WORD_CHAR)
        self.pango_layout.set_markup(self.markup_string)

    def draw_page(self, operation, gtk_context, page_number):
        cairo_context = gtk_context.get_cairo_context()
        PangoCairo.show_layout(cairo_context, self.pango_layout)

    def read_markup_from_file(self):
        try:
            self.markup_string = open("PangoMarkup.txt", 'r').read()
            self.cycle_names_textbox()
        except:
            print("Could not find PangoMarkup.txt")

    def write_markup_to_file(self):
        try:
            open("PangoMarkup.txt", 'w').write(self.markup_string)
        except:
            print("Could not save to PangoMarkup.txt")

    #Test parsing Pango markup. Fragile code.
    def cycle_names_textbox(self):
        self.textbuffer.set_text(self.markup_string, -1)
        start_iter = self.textbuffer.get_start_iter()
        span_iter = self.textbuffer.get_start_iter()
        test_string = ""
        index = 0
        count = True
        move_ahead_six = 0
        tag_locations = []
        tag_names = []
        new_string = ""
        span_string = ""

        #Parse markup string. Just check for span tags.
        while(not start_iter.is_end()):
            if("<" == start_iter.get_char()):
                span_iter.assign(start_iter)
                span_iter.forward_chars(5)
                test_string = self.textbuffer.get_text(start_iter, span_iter, False)
                if(test_string == "<span" or test_string == "</spa"):
                    count = False
                    tag_locations.append(index)
                    move_ahead_six = 0
            if(count==False):
                move_ahead_six+=1
                if(move_ahead_six>6):
                    if(">" != start_iter.get_char()):
                        span_string+=start_iter.get_char()
            if(count==True):
                index+=1
                new_string+=start_iter.get_char()
            if(">" == start_iter.get_char() and count == False):
                count = True
                if(span_string != ""):
                    tag_names.append(span_string)
                span_string = ""
            start_iter.forward_char()
        
        print tag_locations
        print tag_names
        print new_string

        #Get a list of unique tags.
        unique_tags = []
        for tuples in tag_names:
           tags = tuples.split()
           for tag in tags:
               if tag not in unique_tags:
                   unique_tags.append(tag)

        print unique_tags
        '''
        #Create tags from unique_tags.
        for tag in unique_tags:
            if(tag == "background='green'"):
                self.textbuffer.create_tag(tag, background='green')
            if(tag == "weight='900'"):
                self.textbuffer.create_tag(tag, weight=900)
        print("Tags Created")
        '''
        #Update textview with parsed string.
        self.textbuffer.set_text(new_string, -1)

        #Apply tags to buffer.
        offset1 = self.textbuffer.get_start_iter()
        offset2 = self.textbuffer.get_start_iter()
        for i in range(len(tag_locations)/2):
            start = tag_locations[2*i]
            end = tag_locations[2*i+1]
            print(str(i) + " " + str(start) + " " + str(end))
            offset1.set_offset(start)
            offset2.set_offset(end)
            names = tag_names[i].split()
            for value in names: 
                self.textbuffer.apply_tag_by_name(value, offset1, offset2)
                
        print("Tags Applied")

class MainWindow(Gtk.Window):
    def __init__(self):
        Gtk.Window.__init__(self, title="Text and Tag Iters")
        self.set_default_size(400,450)
        self.menubar1 = Gtk.MenuBar()  
        self.menu1 = Gtk.Menu()
        self.menuitem1 = Gtk.MenuItem("File") 
        self.menuitem1.set_submenu(self.menu1)
        self.menuitem2 = Gtk.MenuItem("Read File")
        self.menuitem2.set_name("menuitem2")
        self.menuitem2.connect("activate", self.open_file)      
        self.menuitem3 = Gtk.MenuItem("Write File")
        self.menuitem3.set_name("menuitem3")
        self.menuitem3.connect("activate", self.save_file)   
        self.menu1.append(self.menuitem2)
        self.menu1.append(self.menuitem3) 
        self.menubar1.append(self.menuitem1)
        self.TextBox1 = TextBox()
        self.TextBox1.set_hexpand(True)
        self.TextBox1.set_vexpand(True)
        self.scrolledwindow = Gtk.ScrolledWindow()
        self.scrolledwindow.set_hexpand(True)
        self.scrolledwindow.set_vexpand(True)
        self.scrolledwindow.add(self.TextBox1)
        self.button1 = Gtk.Button("Find Words")
        self.button1.connect("clicked", self.match_word)
        self.button2 = Gtk.Button("Find Tags")
        self.button2.connect("clicked", self.find_tags)
        self.button3 = Gtk.Button("Cursor Back")
        self.button3.set_name("button3")
        self.button3.connect("clicked", self.cursor_back)
        self.button4 = Gtk.Button("Cursor Forward")
        self.button4.set_name("button4")
        self.button4.connect("clicked", self.cursor_forward)
        self.button5 = Gtk.Button("Remove Tags")
        self.button5.connect("clicked", self.remove_all_tags) 
        self.button6 = Gtk.Button("Pango Tags")
        self.button6.set_name("button6")
        self.button6.connect("clicked", self.pango_tags)
        self.button7 = Gtk.Button("Print")
        self.button7.set_name("button7")
        self.button7.connect("clicked", self.print_dialog)      
        self.button8 = Gtk.Button("Cycle Markup")
        self.button8.set_name("button8")
        self.button8.connect("clicked", self.cycle_names)
        self.label1 = Gtk.Label("Pango text and tags.")
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
        self.grid.attach(self.menubar1, 0, 0, 1, 1)
        self.grid.attach(self.button1, 0, 1, 1, 1)
        self.grid.attach(self.entry1, 1, 1, 1, 1)
        self.grid.attach(self.combo1, 2, 1, 1, 1)
        self.grid.attach(self.button2, 0, 2, 1, 1)
        self.grid.attach(self.combo2, 1, 2, 1, 1)
        self.grid.attach(self.button3, 0, 3, 1, 1)
        self.grid.attach(self.button4, 1, 3, 1, 1)
        self.grid.attach(self.combo3, 2, 3, 1, 1)
        self.grid.attach(self.button5, 0, 4, 1, 1)
        self.grid.attach(self.scrolledwindow, 0, 5, 3, 1)
        self.grid.attach(self.button6, 0, 6, 1, 1)
        self.grid.attach(self.button7, 1, 6, 1, 1)
        self.grid.attach(self.button8, 2, 6, 1, 1)
        self.grid.attach(self.label1, 0, 7, 3, 1)
        self.add(self.grid)
        style_provider = Gtk.CssProvider()
        css = "GtkWindow{background: cyan;}GtkButton{background: yellow;}"
        style_provider.load_from_data(css)
        Gtk.StyleContext.add_provider_for_screen(Gdk.Screen.get_default(), style_provider,Gtk.STYLE_PROVIDER_PRIORITY_APPLICATION)
        self.show_all()

    def match_word(self, button1):
        print("Find Words")
        text1 = self.entry1.get_text()
        self.TextBox1.get_word(text1, self.combo1)

    def find_tags(self, button2):
        print("Find Tags")
        self.TextBox1.get_tag_table(None, self.combo2, None)

    def cursor_back(self, button3):
        print("Move Back")
        self.TextBox1.get_tag_table(button3, self.combo3, None)

    def cursor_forward(self, button4):
        print("Move Forward")
        self.TextBox1.get_tag_table(button4, self.combo3, None)

    def remove_all_tags(self, button5):
        print("Remove Tags")
        self.TextBox1.remove_tags(button5)

    def pango_tags(self, button6):
        print("Pango Tags")
        self.TextBox1.get_tag_table(button6, None, self.label1)

    def print_dialog(self, button7):
        print("Print Dialog")
        self.TextBox1.get_tag_table(button7, None, self.label1)

    def cycle_names(self, button8):
        print("Cycle Names")
        self.TextBox1.get_tag_table(button8, None, self.label1)

    def open_file(self, menuitem2):
        print("Open File")
        self.TextBox1.read_markup_from_file()

    def save_file(self, menuitem3):
        print("Save File")
        self.TextBox1.get_tag_table(menuitem3, None, self.label1)

win = MainWindow()
win.connect("delete-event", Gtk.main_quit) 
win.show_all()
Gtk.main()
