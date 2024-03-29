import numpy as np, subprocess
import sys
if (len(sys.argv)!=3):
    print("syntax:  integrated_interaction_lengths.py input.xml eta")
    exit()
xml_file=sys.argv[1]
eta = float(sys.argv[2])

useTilt=False
if useTilt:
    tilt=-.025
else:
    tilt = 0



for theta in [2*np.arctan(np.exp(-eta))]:
    phis = np.linspace(-np.pi, np.pi, 49)
    vals = []
    for phi in phis:
        Z=500
        uxp = np.sin(theta)*np.cos(phi)
        uyp = np.sin(theta)*np.sin(phi)
        uzp = np.cos(theta)
        ux = uxp*np.cos(tilt)+uzp*np.sin(tilt)
        uy = uyp
        uz = uzp*np.cos(tilt)-uxp*np.sin(tilt)
        command=f"print_materials {xml_file} 0 0 0 {Z*ux/uz} {Z*uy/uz} {Z}"
        #print(command)
        #command += " | grep 'Integrated interaction lengths' | awk '{print $5;}'"
        result = subprocess.run(command.split(), stdout=subprocess.PIPE, text=True, stderr=subprocess.DEVNULL)
        for line in result.stdout.split("\n"):
            if "Integrated interaction lengths" in line:
                val=float((line.split()[4]))
                break;

        vals.append(val)
    print("nils[",eta,"] = ",vals)
        #val = float(result.stdout)
        #print(theta, phi, val)
