#!/bin/bash

# Input simulation parameters
particle="pi-"
beam_energy=50
num_events=1000
theta_min=2.83 # in degrees
theta_max=2.83 # in degrees
phi_min=0. # in degrees
phi_max=360. # in degrees 
physics_list="FTFP_BERT_HP"

# Output file names
info_string="${particle}_${beam_energy}GeV_theta_${theta}deg"
hepmcfile="gen_${info_string}.hepmc"
simfile="insert_sim_${info_string}.edm4hep.root"
recofile="insert_reco_${info_string}.edm4hep.root"

# Generating hepmc file
root -l -b -q "./hepmc_generation/gen_particles.cxx+(\
${num_events},\
\"${hepmcfile}\",\
\"${particle}\",\
${theta_min},\
${theta_max},\
${phi_min},\
${phi_max},\
${beam_energy})"

# Running simulation
npsim \
   --compactFile endcapP_insert.xml \
   --numberOfEvents ${num_events} \
   --physicsList ${physics_list} \
   --inputFiles ${hepmcfile} \
   --outputFile ${simfile}  || exit
   
# Deleting hepmcfile  
rm ${hepmcfile}

# Running reconstruction
export JUGGLER_SIM_FILE=${simfile} JUGGLER_REC_FILE=${recofile} JUGGLER_N_EVENTS=${num_events}
gaudirun.py endcapP_insert_reco.py
