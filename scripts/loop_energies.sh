#!/bin/bash

# Use either run_sim_hepmc or run_sim_gps
simulation_script="run_sim_hepmc.sh"
energies=(1 2 5 10 20 30 40 50 60 70 80 90 100)

for energy in "${energies[@]}"
do
	sed -i "s/beam_energy=.*/beam_energy=${energy}/" ${SCRIPTS_PATH}/${simulation_script}
	echo "Starting energy ${energy} GeV"
	time source ${SCRIPTS_PATH}/${simulation_script}
done