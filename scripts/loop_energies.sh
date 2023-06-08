#!/bin/bash

# Use eithr run_sim_hepmc or run_sim_gps
simulation="run_sim_hepmc"
energies=(1 2 5 10 20 30 40 50 60 70 80 90 100)

for energy in "${energies[@]}"
do
	sed -i "s/beam_energy=.*/beam_energy=${energy}/" ${simulation}
	echo "Starting energy ${energy} GeV"
	time ./${simulation}
done