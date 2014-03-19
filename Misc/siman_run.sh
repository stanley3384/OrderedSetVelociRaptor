#!/bin/bash

# Put a few things together in a script. Compile siman_tsp.c, output to graphs and open the
# graphs with Document Viewer. Set up for Ubuntu. Might have to apt-get the graphing.
# A great GSL example.

function progress(){
echo -n "Simulated Annealing"
while true
do
     echo -n "."
     sleep 1
done
}
rm -f siman_tsp, 12-cities.eps, a-initial-route.eps, final-route.eps, final.pdf
echo "Compile code"
gcc siman_tsp.c -o siman_tsp -lgsl -lgslcblas -lm
echo "Run with exaustive search"
echo "./siman_tsp > tsp.output"
progress &
# Save PID
DotCounter=$!
./siman_tsp > tsp.output
# Stop the counter
kill $DotCounter
wait $! 2>/dev/null
echo "done"
echo "Create energy graph"
grep -v "^#" tsp.output | awk '{print $1, $18}' | graph -y 3300 6500 -W0 -X generation -Y distance -L "TSP-12 southwest cities(energy)" | plot -Tps > 12-cities.eps
echo "Create initial graph"
grep initial_city_coord tsp.output | awk '{print $2, $3}' | graph -X "longitude (- means west)" -Y "latitude" -L "TSP-initial-order" -f 0.03 -S 1 0.1 | plot -Tps > a-initial-route.eps
echo "Create final graph"
grep final_city_coord tsp.output | awk '{print $2, $3}' | graph -X "longitude (- means west)" -Y "latitude" -L "TSP-final-order" -f 0.03 -S 1 0.1 | plot -Tps > final-route.eps
echo "Put the .eps files into a pdf"
convert *.eps -quality 100 final.pdf
echo "Open Document Viewer"
evince final.pdf
exit 0
