#!/usr/bin/gnuplot

#
# A basic platemap heatmap with gnuplot. Set up for 3x96 well plates. Line things up for 
# a comparison with the VelociRaptor application. The heatmap1.dat file is just the 
# 2d grid data. It can be created by right clicking on test plate data and saving to the
# clipboard with truncate. Copy and save that to a file. Change numbers around and test
# it out.
# run with; gnuplot heatmap_plate.gp -persist
#

set terminal wxt size 900,700 enhanced font 'Verdana,10' persist

unset key
set tic scale 1
set palette rgbformula 7,5,15
set format cb "%4.1f"

set cbrange [-100:1100]
set cblabel "Score"

unset xtics
set x2range [-0.5:11.5]
set x2tics 1 out nomirror

unset ytics
set y2range [7.5:-0.5]
set y2tics 1 out nomirror

# grid lines
set mx2tics 2
set my2tics 2
set grid front mx2tics my2tics lw 1.5 lt -1 lc rgb 'white'

set multiplot layout 3,1 scale 0.9,1.0
do for [i=0:2]{
    set title sprintf('Plate %i',i+1) offset 0,-1
    plot 'heatmap1.dat' index i matrix axes x2y2 with image
    print "Plate ", i+1
  }
unset multiplot
