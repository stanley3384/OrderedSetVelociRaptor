#!/usr/bin/python

'''
   A simple example of formatting some micro-titer plates with Python and HTML. Change things
around as needed. Can even get inventive with CSS or stream data from a database. Open the 
resulting file in a browser.

C. Eric Cashon
'''
import sys
import random
import os

#Add 1 to columns for labels. The first column in a row is a label.
rows = 8
columns = 12 + 1
plates = 10
plate_size = 96

row_labels = ["A","B","C","D","E","F","G","H"]
column_labels = ["1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", ""]

gradient_iris = ["#d507e5", "#d60fde", "#d711dc", "#d716d7", "#d819d5", "#d91ed1", "#d920ce", "#da25ca", "#da28c7", "#db2dc3", "#dc2fc1", "#dd34bc", "#dd37ba", "#de3cb5", "#de3eb3", "#df43ae", "#df46ac", "#e04ba8", "#e14da5", "#e252a1", "#e2559e", "#e35c98", "#e46193", "#e46491", "#e5698c", "#e77085", "#e77383", "#e8787f", "#e97f78", "#ea8275", "#ea8771", "#eb896f", "#ec8e6a", "#ec9168", "#ed9663", "#ed9861", "#ee9d5c", "#efa05a", "#efa556", "#f0a753", "#f1ac4f", "#f1af4c", "#f2b448", "#f2b646", "#f3bb41", "#f4be3f", "#f4c33a", "#f5c538", "#f6ca33", "#f6cd31", "#f7d22d", "#f7d42a", "#f8d926", "#f9dc23", "#f9e11f", "#fae31d", "#fbe818", "#fbeb16", "#fcf011", "#fcf20f", "#fdf50d", "#fdf70a", "#fefa08", "#fefc06", "#fefe04"]

#Get some test data.
test_data=[]
for i in range(0, 960, 1):
    test_data.append(random.uniform(0,100))

#High and low across the entire dataset.
high = max(test_data)
low = min(test_data)

f = open('heatmap1.html','w')

f.write("<!DOCTYPE html>")
f.write("<html lang=\"en\">")
f.write("<head>")
f.write("<meta charset=\"UTF-8\"/>")
f.write("<title>Heatmap</title>")
f.write("</head>")
f.write("<body>")
f.write("<h1>Some 384 Well Plates</h1>")

#Use a counter for simplicity.
counter = 0
for i in range(0, plates, 1):
    #Write the column titles.
    f.write("<p></p>")
    f.write("<table><caption>Plate %s</caption><thead><tr>" % (str(i+1)))
    f.write("<th scope=\"rowgroup\" bgcolor=\"silver\"></th>")
    for j in range(0, columns-1, 1):
        f.write("<th scope=\"col\" bgcolor=\"silver\">%s</th>" % (column_labels[j]))
    f.write("</tr></thead><tbody>")  

    #Write the data and color gradient into the table.
    for j in range(0, rows, 1):
        f.write("<tr>")
        for k in range(0, columns, 1):
            if(k==0):
                #This is a label column. Doesn't count for the test_data counter.
                f.write("<th scope=\"row\" bgcolor=\"silver\">%s</th>" % (row_labels[j]))
            else:
                temp = int((test_data[counter]/(high - low)) *64)
                f.write("<td bgcolor=\"%s\">%s</td>" % (gradient_iris[temp], str(round(test_data[counter], 2))))
                counter+=1
        f.write("</tr>")
    f.write("</tbody></table>")  

f.write("</body>")
f.write("</html>") 

f.close()









