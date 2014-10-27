#!/bin/bash

# Send the data from the output of siman_tsp.c and siman_run.sh to Gnuplot.
# Change things around to see how things work or... don't work. Experiment.
# Just pipe the data to Gnuplot on this one.

#IFS=$'\n'
cities=(
        "-105.95 35.68 Santa Fe"
        "-112.07 33.54 Phoenix" 
        "-106.62 35.12 Albuquerque"
        "-103.2 34.41 Clovis"
        "-107.87 37.29 Durango"
        "-96.77 32.79 Dallas"
        "-105.92 35.77 Tesuque"
        "-107.84 35.15 Grants"
        "-106.28 35.89 Los Alamos"
        "-106.76 32.34 Las Cruces"
        "-108.58 37.35 Cortez"
        "-108.74 35.52 Gallup"
        "-105.95 35.68 Santa Fe"
        )

echo "Run Simulation"
#change increment values and test.
{
for ((pattern=1; pattern <=4500 ; pattern=pattern+50));
  do
     indexes=($(awk -v ref=$pattern 'match($1, ref){print $5,$6,$7,$8,$9,$10,$11,$12,$13,$14,$15,$16;exit}' tsp.output))
     #echo $pattern ${indexes[*]}
     for ((i=0;i<${#indexes[@]};i++));
         do
           #echo "###initial_city_coord: ${cities[i]}" 
           temp_cities[i]="###initial_city_coord: ${cities[${indexes[i]}]}"
         done
     temp_cities[14]="###initial_city_coord: ${cities[0]}"
     temp_cities[15]="e e"
     temp_cities[16]="e pause .2" 
     #\x27 for ' . Had a little trouble getting '-' 
     printf "%s\n" "${temp_cities[@]}" | awk 'NR == 1 { print "plot \x27-\x27", "using 1:2 with linespoints pointtype 6 linecolor 4" }{print $2, $3}'
     #linecolor 1 red, 2 green, 3 blue, 4 magenta, 5 lightblue, 6 yellow, 7 black, 8 orange, 9 grey
     unset temp_cities
     unset indexes
  done
} | gnuplot -p
exit 0



