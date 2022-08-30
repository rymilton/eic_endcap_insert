#!/bin/bash

# Input simulation parameters
particle="pi-"
beam_energy=50
num_events=1000
theta_low=177.17
theta_high=177.17
phi_low=0
phi_high=360

gps_file="gps.mac"
# Output file names
info_string="${particle}_${beam_energy}GeV_theta_${theta_low}deg_${theta_high}deg"
simfile="insert_sim_${info_string}.edm4hep.root"
recofile="insert_reco_${info_string}.edm4hep.root"

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
   --compactFile endcapP_insert.xml \
   --outputFile ${simfile}  || exit

# Running reconstruction
export JUGGLER_SIM_FILE=${simfile} JUGGLER_REC_FILE=${recofile} JUGGLER_N_EVENTS=${num_events}
gaudirun.py endcapP_insert_reco.py