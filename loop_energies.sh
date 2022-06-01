#!/bin/bash

energies=(1 2 5 10 20 30 40 50 60 70 80 90 100)

for energy in "${energies[@]}"
do
	sed -i "s/beam_energy=.*/beam_energy=${energy}/" "run_sim"
	echo "Starting energy ${energy} GeV"
	time ./run_sim
done