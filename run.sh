#!/bin/bash

particle="$2"
energies=(30 40 50 60 80 100)
pcuts=(0.91 0.51 0.46 0.61 0.43 0.41)
j=$(( $1 % 10 ))
k=$(( $1 / 10 ))
beam_energy=${energies[$k]}
pcut=${pcuts[$k]}

working=proc$1
mkdir -p $working $SPIN/data/eic/histos
cp analysis/AnaPi0gg.C $working/AnaPi0gg.C
sed -i "s/str_include/15_15deg-${beam_energy}GeV/g" $working/AnaPi0gg.C
sed -i "s/str_pcut/${pcut}/g" $working/AnaPi0gg.C
root -l -b -q "$working/AnaPi0gg.C($beam_energy, $j, \"$particle\")"
rm -rf $working
