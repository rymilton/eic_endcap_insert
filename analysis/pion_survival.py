import numpy as np, subprocess

import sys
if (len(sys.argv)==1):
    print("syntax:  integrated_interaction_lengths.py input.xml")
    exit()
xml_file=sys.argv[1]

tilt=.025
avgs=[]
thetas=[]
for thetad in np.linspace(0, 10, 41):
    theta=thetad/180*np.pi
    thesum=0
    N=0
    for phi in np.linspace(0, np.pi, 13):
        Z=500
        uxp = np.sin(theta)*np.cos(phi)
        uyp = np.sin(theta)*np.sin(phi)
        uzp = np.cos(theta)
        ux = uxp*np.cos(tilt)-uzp*np.sin(tilt)
        uy = uyp
        uz = uzp*np.cos(tilt)+uxp*np.sin(tilt)
        command=f"print_materials {xml_file} 0 0 0 {Z*ux/uz} {Z*uy/uz} {Z}"
        print(command)
        #command += " | grep 'Integrated interaction lengths' | awk '{print $5;}'"
        result = subprocess.run(command.split(), stdout=subprocess.PIPE, text=True, stderr=subprocess.DEVNULL)
        for line in result.stdout.split("\n"):
            if "Integrated interaction lengths" in line:
                val=float((line.split()[4]))
                break;
        #if we were doing 0 to pi and -0 to -pi, don't double count 0 and +- pi.
        weight = 1 if not (phi==0 or phi==np.pi) else 0.5
        thesum+=np.exp(-val)*weight
        N+=weight
    avgs.append(thesum/N);
    thetas.append(theta)
    print(theta, avgs[-1])
print("theta:", thetas)
print("avg pion survival probability", avgs)    
        #val = float(result.stdout)
        #print(theta, phi, val)
