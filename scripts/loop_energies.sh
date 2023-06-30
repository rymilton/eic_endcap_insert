#!/bin/bash

function print_the_help {
  echo "USAGE: ${0} -n <nevents> -part <particle> "
  echo "  OPTIONS: "
  echo "    -n,--nevents     Number of events"
  echo "    -part,--particle particle type"
  exit
}

# Use either run_sim_hepmc or run_sim_gps
simulation_script="run_sim_hepmc.sh"

particle="pi-"
num_events=1000

while [ True ]; do
if [ "$1" = "--help" -o "$1" = "-h" ]; then
   print_the_help
   shift
elif [ "$1" = "-part" -o "$1" = "--particle" ]; then
   particle=$2
   shift 2 # past argument
elif [ "$1" = "-n" -o "$1" = "--nevents" ]; then
   num_events=$2
   shift 2 # past argument
else
   break
fi
done

energies=(1 2 5 10 20 30 40 50 60 70 80 90 100)

for energy in "${energies[@]}"
do
	echo "Starting energy ${energy} GeV"
	time ${SCRIPTS_PATH}/${simulation_script} --particle ${particle} --nevents ${num_events} -p ${energy}
done
