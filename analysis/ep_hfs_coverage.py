#!/usr/bin/env python3
########################
import sys
import pandas as pd
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.backends.backend_pdf import PdfPages
import matplotlib.patches as patches
import matplotlib as mpl

#mpl.rcParams['text.usetex'] = True
#mpl.rcParams['text.latex.preamble'] = [r'\usepackage{amsmath}']

#Output pdf files
pp1 = PdfPages('ep_hfs_coverage.pdf')

e_elec = [18,5]
e_prot = [275,41]
alfa = [0.4,0.3]
colors = ['blue','red','green']

#EIC coverage
xran = np.logspace(-5,0,100)
for inum in range(len(e_elec)):

    s_cm = 4.*e_elec[inum]*e_prot[inum]

    fac = s_cm*0.95
    y1eic = fac*xran
    
    fac = s_cm*0.01
    y2eic = fac*xran

    #theta = 177.17 #eta~-3.7
    theta = [2.83,5.70] #eta~3.7,3.0
    cost_arr = np.cos(np.radians(theta))

    #scattered electron
    #Q2eic = 2*e_elec[inum]*e_elec[inum]*(1.+cost)
    #Q2eic = Q2eic / ( 1. + ( 2*e_elec[inum]*e_elec[inum]*(1.+cost)/(xran*s_cm) ) - ( 2*e_elec[inum]*e_prot[inum]*(1.+cost)/s_cm ) )

    #hfs
    for cost in cost_arr:
        Q2eic = 2.*(e_prot[inum]*xran)*(e_prot[inum]*xran)*(1.-cost)
        Q2eic = Q2eic / ( 1. - (2.*e_elec[inum]*e_prot[inum]*(1.-cost)/s_cm) + (2*e_prot[inum]*e_prot[inum]*xran*(1.-cost)/s_cm) )
        
        plt.plot(xran,Q2eic,'--',color=colors[inum])

    limeic1 = y1eic
    limeic2 = y2eic
    #limeic2 = np.maximum(y2eic, Q2eic)
    plt.plot(xran,limeic1,color=colors[inum])
    plt.plot(xran,limeic2,color=colors[inum])
    plt.fill_between(xran,limeic1,limeic2,alpha=alfa[inum],facecolor=colors[inum])

#Make plot
plt.subplots_adjust(left=0.125, bottom=0.125, right=0.875, top=0.875)
plt.xlabel('x')
plt.ylabel(r'$Q^{2} \thinspace [GeV^{2}]$')
plt.xscale('log')
plt.yscale('log')
axes = plt.gca()
axes.xaxis.label.set_size(22)
axes.yaxis.label.set_size(22)
axes.tick_params(axis='both',labelsize=14)
axes.spines["left"].set_linewidth(2)
axes.spines["right"].set_linewidth(2)
axes.spines["bottom"].set_linewidth(2)
axes.spines["top"].set_linewidth(2)
plt.xlim([1e-5,1])
plt.ylim([1e-2,1e4])

#Add text
plt.text(2.5e-5,0.85,'18 x 275 GeV',fontsize=16,color='blue',rotation=26)
plt.text(2.5e-5,0.035,'5 x 41 GeV',fontsize=16,color='red',rotation=24)
plt.text(1.1e-2,0.225,r'$\eta_{hfs} = 3.0$',fontsize=12,color='blue',rotation=40)
plt.text(6e-2,0.15,r'$\eta_{hfs} = 3.0$',fontsize=12,color='red',rotation=40)
plt.text(1.25e-2,2e-2,r'$\eta_{hfs} = 3.7$',fontsize=12,color='blue',rotation=40)
plt.text(9e-2,2e-2,r'$\eta_{hfs} = 3.7$',fontsize=12,color='red',rotation=40)
plt.text(0.5,1.2e4,'y = 0.95',fontsize=12,color='blue',rotation=26)
plt.text(1.1,200,'y = 0.01',fontsize=12,color='blue',rotation=26)
#plt.text(1.1,20,r'y = $10^{-3}$',fontsize=12,color='blue',rotation=26)
plt.text(1.1,775,'y = 0.95',fontsize=12,color='red',rotation=24)
plt.text(1.1,8,'y = 0.01',fontsize=12,color='red',rotation=24)
#plt.text(1.05,0.875,r'y = $10^{-3}$',fontsize=12,color='red',rotation=24)

#Figure Output
fig = plt.gcf()
fig.set_size_inches(18.5/2, 10.5/2)
fig.savefig(pp1, format='pdf')

plt.show()

#Close output pdfs
pp1.close()

# end of script