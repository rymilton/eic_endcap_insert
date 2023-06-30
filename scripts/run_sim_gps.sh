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
theta_low=177.17
theta_high=177.17
phi_low=0
phi_high=360

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

gps_file="${SCRIPTS_PATH}/gps.mac"
# Output file names
info_string="${particle}_${beam_energy}GeV_theta_${theta_low}deg_${theta_high}deg"
simfile="${DETECTOR}_sim_${info_string}.edm4hep.root"
recofile="${DETECTOR}_reco_${info_string}.edm4hep.root"

# Putting parameters into gps file
sed -i "s/\/gps\/particle .*/\/gps\/particle ${particle}/" $gps_file
sed -i "s/\/gps\/ene\/mono .*/\/gps\/ene\/mono ${beam_energy} GeV/" $gps_file
sed -i "s/\/gps\/ang\/mintheta .*/\/gps\/ang\/mintheta ${theta_low} degree/" $gps_file
sed -i "s/\/gps\/ang\/maxtheta .*/\/gps\/ang\/maxtheta ${theta_high} degree/" $gps_file
sed -i "s/\/gps\/ang\/minphi .*/\/gps\/ang\/minphi ${phi_low} degree/" $gps_file
sed -i "s/\/gps\/ang\/maxphi .*/\/gps\/ang\/maxphi ${phi_high} degree/" $gps_file
sed -i "s/\/run\/beamOn .*/\/run\/beamOn $num_events/" $gps_file

# Running simulation
npsim  --runType run  --enableG4GPS \
   --macroFile ${gps_file} \
   --compactFile ${DETECTOR_PATH}/${DETECTOR}.xml \
   --outputFile ${simfile}  || exit

# Running reconstruction
export JUGGLER_SIM_FILE=${simfile} JUGGLER_REC_FILE=${recofile} JUGGLER_N_EVENTS=${num_events}
gaudirun.py ${SCRIPTS_PATH}/${DETECTOR}_reco.py
