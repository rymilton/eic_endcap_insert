# eic_endcap_insert
Treat this directory as you would the ATHENA repository (https://eicweb.phy.anl.gov/EIC/detectors/athena.git). Make sure to link the ip6 repository to this one.

The added/edited components are the definitions, materials, ECal, HCal, and insert. The pECal is currently commented out since it needs to be adjusted for the beampipe.

Once the ip6 directory is linked, change "CrossingAngle" in ./ip6/ip6_defs.xml to 0 radians to avoid overlaps with the insert/HCal. These will be rotated back once development is finished. 
