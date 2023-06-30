#!/bin/bash

function print_the_help {
  echo "USAGE: ${0} -n <nevents> -part <particle> -p <momentum> -thmin <theta_min> -thmax <theta_max>"
  echo "  OPTIONS: "
  echo "    -n,--nevents     Number of events"
  echo "    -part,--particle particle type"
  echo "    -p, --momentum   particle momentum (GeV)"
  echo "    -thmin, --theta_min   Minimum theta (degrees)"
  echo "    -thmax, --theta_max   Maximum theta (degrees)" 
  echo "    theta (polar angle) will generate uniformly between theta_min and theta_max"
  exit
}

# Input simulation parameters
particle="pi-"
beam_energy=10
num_events=1000
theta_min=2.83
theta_max=2.83

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
elif [ "$1" = "-p" -o "$1" = "--momentum" ]; then
   beam_energy=$2
   shift 2 # past argument
elif [ "$1" = "-thmin" -o "$1" = "--theta_min" ]; then
   theta_min=$2
   shift 2 # past argument
elif [ "$1" = "-thmax" -o "$1" = "--theta_max" ]; then
   theta_max=$2
   shift 2 # past argument
else
   break
fi
done

# Output file names
info_string="${particle}_${beam_energy}GeV_theta_${theta_min}deg_thru_${theta_max}deg"
simfile="insert_sim_${info_string}.edm4hep.root"
recofile="insert_reco_${info_string}.edm4hep.root"

# Run reconstruction
export JUGGLER_SIM_FILE=${simfile} JUGGLER_REC_FILE=${recofile} JUGGLER_N_EVENTS=${num_events}
gaudirun.py ${SCRIPTS_PATH}/${DETECTOR}_reco.py
