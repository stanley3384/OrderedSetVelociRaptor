#!/usr/bin/perl

# Send the data from the output of siman_tsp.c and siman_run.sh to Gnuplot.
# Change things around to see how things work or... don't work. Experiment.
# Test a little perl code.

@cities = ("-105.95 35.68 Santa Fe", "-112.07 33.54 Phoenix", "-106.62 35.12 Albuquerque", "-103.2 34.41 Clovis", "-107.87 37.29 Durango", "-96.77 32.79 Dallas", "-105.92 35.77 Tesuque", "-107.84 35.15 Grants", "-106.28 35.89 Los Alamos", "-106.76 32.34 Las Cruces", "-108.58 37.35 Cortez", "-108.74 35.52 Gallup", "-105.95 35.68 Santa Fe");

open PROC, "| /usr/bin/gnuplot -persist" || die "Could not start gnuplot: $!";
print PROC "set xlabel 'Longitude'; set ylabel 'Latitude';";
#linecolor 1 red, 2 green, 3 blue, 4 magenta, 5 lightblue, 6 yellow, 7 black, 8 orange, 9 grey
$plot = "plot '-' using 1:2 with linespoints pointtype 6 linecolor 4";

for ($pattern=1; $pattern <=4500 ; $pattern=$pattern+50)
    {
     $cmd="awk -v ref=$pattern 'match(\$1, ref){print \$5,\$6,\$7,\$8,\$9,\$10,\$11,\$12,\$13,\$14,\$15,\$16;exit}' tsp.output";
     #print "$cmd\n";
     $string=`$cmd`;
     @indexes=split(' ', $string);
     #print "index @indexes\n";
     for ($i=0;$i<@indexes;$i++)
         {
           $temp_cities[$i]=$cities[$indexes[$i]];
         }
     $temp_cities[$i]=$cities[0];
     $temp_cities[$i+1]="e";
     $temp_cities[$i+2]="pause .05"; 
     #print "$plot\n";
     print PROC "$plot\n";
     for ($j=0;$j<@temp_cities;$j++)
         {
           #print "$temp_cities[$j]\n";
           print PROC "$temp_cities[$j]\n";
         }
     }

close PROC;

