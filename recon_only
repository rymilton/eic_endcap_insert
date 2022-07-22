#!/bin/bash

# Input simulation parameters
particle="e-"
beam_energy=100
num_events=1000

# Output file names
info_string="${particle}_${beam_energy}GeV_theta_25.16deg"
simfile="insert_sim_${info_string}.root"
recofile="insert_reco_${info_string}.edm4hep.root"


# Run reconstruction
export JUGGLER_SIM_FILE=${simfile} JUGGLER_REC_FILE=${recofile} JUGGLER_N_EVENTS=${num_events}
gaudirun.py endcapP_insert_reco.py
