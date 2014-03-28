#!/usr/bin/env python

#
# Here is some fun with Gnuplot. Get the shortest route between cites and 
# graph them. There is an extra file needed to make this work. It can
# be found at
# http://www.gnuplotting.org/mercator-projection/ 
# the file needed is world_110m.txt
# A very good article there with some great work.
# Also comment out one cities array for the other in siman_tsp.c
# Of course, change things around to see how they work. Might
# even want to comment out exhaustive_search(); in siman_tsp.c
# Might have to give this one a few seconds.
#

import os
import subprocess

cities = ["64.08 -21.56 Reykjavik", "47.37 -122.20 Seattle",  "-26.12 28.03 Johannesburg", "9.02 38.44 Addis Ababa", " 21.19, -157.50 Honolulu", "32.79 -96.77 Dallas", "-16.30 -68.09 La Paz", "53.34, 10.00 Hamburg", "-36.50 174.44 Auckland", "-12.28 130.50 Darwin", "59.57 30.18 Saint Petersburg", "1.17, 103.50 Singapore"]

print "Compile Code"
os.system('gcc siman_tsp.c -o siman_tsp -lgsl -lgslcblas -lm')
print "Run Simulated Annealing"
os.system('./siman_tsp > tsp.output')
print "Create EquiRectangular Projection"
#Tried to pipe this but gnuplot generates errors. This seems to work.
os.system('gnuplot mercator_projection.gp')

print "Open Pipe for Simulation"
gp = os.popen( '/usr/bin/gnuplot', 'w' )

#Set up graph
gp.write("reset\n")
gp.write("set terminal wxt size 720,360 enhanced font 'Verdana,10' persist\n")
gp.write("unset key\nunset border\nset lmargin screen 0\nset bmargin screen 0\nset rmargin screen 1\nset tmargin screen 1\nset format ''\n")
gp.write("set xrange [0:719]\nset yrange [0:359]\n")
gp.write("set multiplot\nplot \"equirectangular_projection.png\" binary filetype=png w rgbimage\n")
#linecolor 1 red, 2 green, 3 blue, 4 magenta, 5 lightblue, 6 yellow, 7 black, 8 orange, 9 grey
plot = "plot '-' using (($2+180)*2):(($1+90)*2) with linespoints pointtype 6 linecolor 4"
plot2 = "plot \"equirectangular_projection.png\" binary filetype=png w rgbimage\nplot '-' using (($2+180)*2):(($1+90)*2) with linespoints pointtype 6 linecolor 3"

temp_cities=[]
#Get a couple of slices from the simulated annealing output. Change things around for different slices.
for pattern in range(0, 2500, 50):
     cmd = "awk -v ref=" + str(pattern) + " 'match($1, ref){print $5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16;exit}' tsp.output"
     #print cmd
     ret_string = subprocess.check_output(cmd, stderr=subprocess.STDOUT, shell=True)
     #print ret_string
     ret_string = ret_string.rstrip("\n")
     #print ret_string
     indexes=ret_string.split(' ')
     #print indexes
     for i in indexes:
         {
           temp_cities.append(cities[int(i)]) 
         }
     temp_cities.append(cities[0])
     temp_cities.append("e")
     temp_cities.append("pause .00")

     if pattern == 2450:
         print "Data in Pipe"
         gp.write(plot2 + "\n")
     else:
         gp.write(plot + "\n")

     for j in temp_cities:
         {
           gp.write( j + "\n" )
         }
     del temp_cities[:]

gp.close()
print "Best Route"


