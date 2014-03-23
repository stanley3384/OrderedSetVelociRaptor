#!/usr/bin/env python

# Send the data from the output of siman_tsp.c and siman_run.sh to Gnuplot.
# Change things around to see how things work or... don't work. Experiment.
# Test a little python code. 

import os
import subprocess

cities = ["-105.95 35.68 Santa Fe", "-112.07 33.54 Phoenix", "-106.62 35.12 Albuquerque", "-103.2 34.41 Clovis", "-107.87 37.29 Durango", "-96.77 32.79 Dallas", "-105.92 35.77 Tesuque", "-107.84 35.15 Grants", "-106.28 35.89 Los Alamos", "-106.76 32.34 Las Cruces", "-108.58 37.35 Cortez", "-108.74 35.52 Gallup", "-105.95 35.68 Santa Fe"]

gp = os.popen( '/usr/bin/gnuplot', 'w' )

gp.write( "set xlabel 'Longitude'; set ylabel 'Latitude';" )
plot = "plot '-' using 1:2 with linespoints pointtype 6 linecolor 4"

temp_cities=[]
for pattern in range(1, 4500, 50):
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
     temp_cities.append("pause .05")
     gp.write(plot + "\n")
     for j in temp_cities:
         {
           gp.write( j + "\n" )
         }
     del temp_cities[:]

gp.close()

