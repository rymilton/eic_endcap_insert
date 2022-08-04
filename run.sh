#!/bin/bash

j=$(( $1 % 100 ))
k=$(( $1 / 100 ))
particle=$2
energies=(80)
beam_energy=${energies[$k]}

mkdir -p $SPIN/data/eic/histos
root -l -b -q "analysis/AnaPi0gg.C+($beam_energy, $j, \"$particle\")"
