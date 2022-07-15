#!/bin/bash

j=$(( $1 % 10 ))
k=$(( $1 / 10 ))
particle=$2
energies=(40 45 50 55 60)
beam_energy=${energies[$k]}

mkdir -p $SPIN/data/eic/histos
root -l -b -q "analysis/AnaPi0gg.C+($beam_energy, $j, \"$particle\")"
