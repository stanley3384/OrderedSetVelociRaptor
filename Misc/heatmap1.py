#!/usr/bin/python

'''
   A simple example of formatting some microtiter plates with Python and HTML. Change things
around as needed. Can even get inventive with CSS or stream data from a database. Open the 
resulting file in a HTML5 browser.
   Python version 2.7.3

C. Eric Cashon
'''
import sys
import random
import os
import sqlite3 as lite

#Add 1 to columns for labels. The first column in a row is a label.
rows = 8
columns = 12 + 1
plates = 10
plate_size = 96

row_labels = ["A","B","C","D","E","F","G","H"]
column_labels = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12"]

gradient_iris = ["#d507e5", "#d60fde", "#d711dc", "#d716d7", "#d819d5", "#d91ed1", "#d920ce", "#da25ca", "#da28c7", "#db2dc3", "#dc2fc1", "#dd34bc", "#dd37ba", "#de3cb5", "#de3eb3", "#df43ae", "#df46ac", "#e04ba8", "#e14da5", "#e252a1", "#e2559e", "#e35c98", "#e46193", "#e46491", "#e5698c", "#e77085", "#e77383", "#e8787f", "#e97f78", "#ea8275", "#ea8771", "#eb896f", "#ec8e6a", "#ec9168", "#ed9663", "#ed9861", "#ee9d5c", "#efa05a", "#efa556", "#f0a753", "#f1ac4f", "#f1af4c", "#f2b448", "#f2b646", "#f3bb41", "#f4be3f", "#f4c33a", "#f5c538", "#f6ca33", "#f6cd31", "#f7d22d", "#f7d42a", "#f8d926", "#f9dc23", "#f9e11f", "#fae31d", "#fbe818", "#fbeb16", "#fcf011", "#fcf20f", "#fdf50d", "#fdf70a", "#fefa08", "#fefc06", "#fefe04"]

#Get some test data.
test_data=[]
for i in range(0, 960, 1):
    test_data.append(random.uniform(0,100))

'''
#Test a database connection.
test_data2=[]
for i in range(0, 960, 1):
    test_data2.append( (i, random.uniform(0,100)) )
con = lite.connect('test2.db')    
cur = con.cursor()    
cur.execute('SELECT SQLITE_VERSION()')   
data = cur.fetchone()    
print "Python version: %s SQLite version: %s" % (sys.version, str(data[0])) 
cur.execute("DROP TABLE IF EXISTS PlateData")
cur.execute("CREATE TABLE PlateData( KeyID INTEGER PRIMARY KEY, WellData REAL)")
cur.executemany("INSERT INTO PlateData VALUES(?,?)", test_data2)
#con.commit()
cur.execute("SELECT KeyID, WellData FROM PlateData")
records = cur.fetchall()
print "Database Record Count %d" % (len(records))
'''

#High and low for each plate.
high=[]
low=[]
for i in range(0, plates, 1):
    index1 = plate_size*i
    index2 = plate_size*i+(plate_size)
    high.append(max(test_data[index1:index2]))   
    low.append(min(test_data[index1:index2]))

f = open('heatmap1.html','w')

f.write("<!DOCTYPE html>\n")
f.write("<html lang=\"en\">\n")
f.write("<head>\n")
f.write("<meta charset=\"UTF-8\"/>\n")
f.write("<title>Heatmap</title>\n")
f.write("</head>\n")
f.write("<body>\n")
f.write("<h1 align=\"center\">Heatmap 96 Well Plates</h1>\n")

#Use a counter for simplicity.
counter = 0
for i in range(0, plates, 1):

    #Write the score table 
    f.write("<table align=\"center\"><caption>Score %s</caption><tr>\n" % (str(i+1)))
    for j in range(0, 8, 1):
        diff = high[i] - low[i]
        temp1 = (low[i] + j*(diff/7))
        temp2 = int((temp1/diff) *64)
        if(temp2>64):
            temp2=64
        if(temp2<0):
            temp2=0
        f.write("<td bgcolor=\"%s\">%s</td>\n" % (gradient_iris[temp2], str(round(temp1, 2))))
    f.write("</tr></table>")

    #The heatmap table
    f.write("<table align=\"center\" bgcolor=\"silver\"  style=\"margin-bottom:40pt\"><caption>Plate %s</caption><thead><tr>\n" % (str(i+1)))
    f.write("<th scope=\"rowgroup\" bgcolor=\"silver\"></th>\n")
    for j in range(0, columns-1, 1):
        f.write("<th scope=\"col\" bgcolor=\"silver\">%s</th>\n" % (column_labels[j]))
    f.write("</tr></thead><tbody>\n")  

    #Write the data and color gradient into the table.
    for j in range(0, rows, 1):
        f.write("<tr>\n")
        for k in range(0, columns, 1):
            if(k==0):
                #This is a label column. Doesn't count for the test_data counter.
                f.write("<th scope=\"row\" bgcolor=\"silver\">%s</th>\n" % (row_labels[j]))
            else:
                temp3 = int((test_data[counter]/(high[i] - low[i])) *64)
                if(temp3>64):
                    temp3=64
                if(temp3<0):
                    temp3=0
                f.write("<td bgcolor=\"%s\">%s</td>\n" % (gradient_iris[temp3], str(round(test_data[counter], 2))))
                counter+=1
        f.write("</tr>\n")
    f.write("</tbody></table>\n")

f.write("</body>\n")
f.write("</html>\n") 

f.close()



