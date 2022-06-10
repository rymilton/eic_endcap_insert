import matplotlib.pyplot as plt
import awkward as ak
import uproot as ur
import sys,os
import numpy as np
from scipy.optimize import curve_fit
from matplotlib.ticker import FormatStrFormatter
from matplotlib.ticker import (MultipleLocator, FormatStrFormatter,
                               AutoMinorLocator)
global ene
from matplotlib.legend_handler import HandlerLine2D
#from scipy.stats import norm
import matplotlib.lines as mlines
import matplotlib.colors as mcolors
FilePathReco="/home/bishnu/EIC/Data/hepmc/"
Gev_To_MeV=1000
PathToPlot='/home/bishnu/UCR_EIC/Plots/hepmc/'
FIT_SIGMA=3
### FUNCTION FOR PLOTTING THE XY Distribution 
def XY_plot2D(X,Y,energy_plot, particle):
    print(energy_plot)
    PosRecoX=X
    PosRecoY=Y
    fig,ax = plt.subplots(1,1, figsize=(8, 6),sharex=True,sharey=True)
    hb=ax.hexbin(ak.flatten(PosRecoX),ak.flatten(PosRecoY),gridsize=50, cmap='viridis')

    cb = fig.colorbar(hb, label='hits')
    if particle=='pi-':
        greek_particle='$\pi^{-}$'
    elif particle=='e-':
        greek_particle='$e^-$'
    else: print("You forgot to pick the particle")
    title='{0} GeV {1}'.format(energy_plot,greek_particle)
    plt.xlabel('position x [cm]')
    plt.title(title)

    plt.ylabel('Position y [cm]')
    FigName='XY_distribution_{0}GeV_{1}.png'.format(energy_plot,particle)
    print('before show')
    plt.savefig(f"{PathToPlot}{FigName}")
    plt.show()
    




## FUNCTION FOR PLOTTING ONE DIMENSION HISTOGRAM OF GIVEN VARIABLE
def distribution_1D(variable,title,energy_plot,particle):
        
    fig,ax = plt.subplots(1,1, figsize=(16, 12),sharex=True,sharey=True)
    #print(len(ene))
    if title=='Energy':
        MinRange=0.0
        MaxRange=800.0
        x_title='Energy (MeV)'
    elif title=='z_pos':
        MinRange=0.0
        MaxRange=150.0
        x_title='Z position (cm)'
    elif title =='y_pos':
        MinRange=0.0
        MaxRange=50.0
        x_title='Y position (cm)'
    elif title =='x_pos':
        MinRange=0.0
        MaxRange=50.0
        x_title='X position (cm)'    
    else: print('PLEASE GIVE RIGHT TITLE')
    ax.hist(ak.flatten(variable),bins=100, range=(MinRange,MaxRange),color='r', linewidth='3')
    #ax.hist(energy,bins=100, range=(MinRange,MaxRange),color='b',histtype='step',linewidth='3')#
    if particle=='pi-':
        greek_particle='$\pi^{-}$'
    elif particle=='e-':
        greek_particle='$e^-$'
    title_plot='{0} GeV {1}'.format(energy_plot,greek_particle)
    
    ax.set_title(title_plot)
    ax.set_yscale('log')
    #ax.set_xscale('log')
    ax.set_xlabel(x_title)
    ax.set_ylabel('Hits')
    print(title)
    FigName='OneD_{0}_{1}GeV_{2}.png'.format(title,energy_plot,particle)
    plt.savefig(f"{PathToPlot}{FigName}")
    #ax.legend(['without cut', 'E>0.06 MeV & time<200 ns'],fontsize='25')
    #
    plt.show() 
    

## Particle ID
def ID_Plot(id,particle):
    fig,ax = plt.subplots(1,1, figsize=(8, 6),sharex=True,sharey=True)
    ax.hist(ak.flatten(id),color='r',linewidth='3', bins=51,range=(0,50))
    if particle=='Pion':
        greek_particle='$\pi^{-}$'
    elif particle=='electron':
        greek_particle='$e^-$'
    ax.set_title('MCParticles.PDG ({0})'.format(greek_particle))
    ax.set_xlabel('Particle ID')
    ax.set_yscale('log')
                 
    plt.show()


### DEFINATION OF GAUSSIAN AND LINEAR FUNCTION FOR FITTING    
#def gaussian(x, amp, mean, sigma):
#    return amp * np.exp( -(x - mean)**2 / (2*sigma**2) )

def gaussian(x, amp, mean, sigma):
    return amp * np.exp( -0.5*((x - mean)/sigma)**2) /sigma

def linear_fit(xl,slope,intercept):
    return (slope*xl)+intercept



## FUNCTION TO FIT THE ENERGY DISTRIBUION WITH GAUSSIAN AND DETERMINE THE MEAN,SIGMA
## FITTING RANGE WITHIN +-3 SIGMA AND FOR LEAKAGE EVENTS BELOW 3SIGMA ARE COUNTED
def get_resolution(good_energy,energy_plot, particle,Sigma_For_leakage):
    #fig = plt.figure( figsize=(6, 4))
    fig,ax = plt.subplots(1,1, figsize=(16, 12),sharex=True,sharey=True)
    ene_good=good_energy
    
    ene_total = ak.sum(ene_good,axis=-1)

    ene_average = ak.mean(ene_good,axis=-1)
    #ene_nhits = ak.num(ene_good)
    #HCAL_total=HCAL_total[0:90]

    mean_guess=np.mean(ene_total)
    sigma_guess=np.std(ene_total)
    nbins=325
    count, bins,_= plt.hist(np.array(ene_total),bins=nbins,alpha=0.5,range=(0,1300),label='HCAL',linewidth='3',color='b')
    binscenters = np.array([0.5 * (bins[i] + bins[i+1]) for i in range(len(bins)-1)])
    #print (count)

    ## CHOOSE THE DATA POINTS WITHIN GIVEN SIGMAS FOR FITTING
    mask=(binscenters>(mean_guess-FIT_SIGMA*sigma_guess)) & (binscenters<(mean_guess+FIT_SIGMA*sigma_guess))
    error_counts=np.sqrt(count)
    error_counts=np.where(error_counts==0,1,error_counts)
    
    # PARAMETER BOUNDS ARE NOT USED FOR NOW
    param_bounds=([-np.inf,-np.inf,-np.inf], [np.inf,np.inf,np.inf])
    popt, pcov = curve_fit(gaussian, binscenters[mask], count[mask],p0=[np.max(count),mean_guess,sigma_guess],bounds=param_bounds)
        
    ax.plot(binscenters[mask], gaussian(binscenters[mask], *popt), color='red', linewidth=2.5, label=r'F')
    ax.set_title("Energy = {0} GeV ".format(energy_plot))
    ax.xaxis.set_major_locator(MultipleLocator(200))
    ax.set_xlabel("Event energy (MeV)")
    FigName='Fit_SimEnergy_{0}_{1}.png'.format(energy_plot,particle)
    
    ### GET MEAN SIGMA AND ERRORS FROM FIT
    mean=popt[1]
    std=popt[2]

    ### CHECK THE GOODNESS OF THE FIT
    chisq=np.sum(((count[mask]-gaussian(binscenters[mask],popt[0],mean,std))/error_counts[mask])**2)
    ss_res=np.sum((count-gaussian(binscenters,popt[0],mean,std))**2)
    ss_tot=np.sum((count-np.mean(count))**2)
    R2=1-(ss_res/ss_tot)
    
                  
                  

    ##### PRINT CHI SQUARE ON THE FITTED PLOT ##############
    print('R2',R2)
    ndf=np.count_nonzero(count)-2
    #ndf=len(count[mask])-2
    #print(count)
    #plt.figtext(0.15,0.8,"$\chi^2/ndf$={0:.2f}/{1}".format(chisq,ndf),fontweight='bold',fontsize=40)
    

    
    #errors=np.sqrt(np.diag(pcov))
    mean_error=np.sqrt(pcov[1, 1])
    std_error=np.sqrt(pcov[2, 2])
    #print(mean_guess,'   sigma   ', sigma_guess,' actual mean ', mean,  'actual std',std)  
    ### GET THE BIN WITH VALUE WHERE IT LIES mEAN -3SIGMA
    two_sig_threshold=mean-(Sigma_For_leakage*std)
    bin_2sig=np.digitize(two_sig_threshold,bins)
    
    ### GET FRACTION OF lEAKAGE
    num_leak=np.sum(count[0:bin_2sig])
    deno_leak=np.sum(count)
    leak_per=(num_leak/deno_leak)*100
    if leak_per==0:
        leak_per_error=0
    else:    
        leak_per_error=np.sqrt((np.sqrt(num_leak)/num_leak)**2 + (np.sqrt(deno_leak)/deno_leak)**2)*leak_per


    ####### PRINT THE VERTICLE LINE WITH 2 SIGMA ################    
    #ax.vlines([two_sig_threshold,  two_sig_threshold],0, np.max(count),linestyles='dashed', colors='m', linewidth=5)
   
    #print(two_sig_threshold,  ' leak_per  ',  leak_per, 'val Num       ',val_num/val_deno*100 )
    #print('3sigma x val =', two_sig_threshold, 'bin ' , bin_2sig,'  ',num_leak,'  ',deno_leak)
    #print(count)
    #ax.tick_params('both', length=20, width=2, which='major')
    #ax.tick_params('both', length=10, width=1, which='minor')
    plt.savefig(f"{PathToPlot}{FigName}")
    plt.show()
    return mean, std, mean_error, std_error, leak_per,leak_per_error



#### Plot for the Resolution, Mean Sigma and Lakage

def plot_resolution(energies,particle,means, stds, mean_errors, std_errors, resolutions, resolution_errors, leaks_per,leaks_per_error,Sigma_For_leakage):
    fig,ax = plt.subplots(1,1, figsize=(16, 12),sharex=True,sharey=True)
    resolutions=np.multiply(resolutions,100)
    print(resolutions)
    resolution_errors=np.multiply(resolution_errors,100)
    ax.errorbar(energies,resolutions, resolution_errors,color="red",marker='o',markersize=20,label='Reconstructed')
    #ax.plot(energies,resolutions, color="blue",marker='*',label="Generated")
    ax.set_xlabel('Energy (GeV)')
    ax.set_ylabel('$\sigma$/E ')
    ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
    ax.set_ylim(0,30)

    ax.xaxis.set_major_locator(MultipleLocator(10))
    ax.yaxis.set_major_locator(MultipleLocator(5))   
    ax.xaxis.grid(True)
    ax.yaxis.grid(True)
    #ax.set_title("Resolution vs Energy")
    FigName="Resolution_Energy_{0}.png".format(particle)
    plt.savefig(f"{PathToPlot}{FigName}")

    plt.show()
    #print(resolutions)
    #print(resolution_errors)
  

    ############ FOR MEAN ################
    if particle=='pi-':
        lowLimit=0
        upperLimit=101
    else:
        lowLimit=0
        upperLimit=101
        
    fig,ax = plt.subplots(1,1, figsize=(16, 12),sharex=True,sharey=True)
    ax.errorbar(energies,means, mean_errors, color="red",marker='o',linestyle='None',markersize=20,label='Reconstructed')
    energies=np.asarray(energies)
    means=np.asarray(means)
    mask=(energies>lowLimit) &(energies<upperLimit)
    poptLin,_popcovLin=curve_fit(linear_fit,energies[mask],means[mask],p0=[0,10],bounds=(0,101))
    slope=poptLin[0]
    intercept=poptLin[1]
   
    ax.plot(energies[mask],linear_fit(energies[mask],*poptLin),label='Fit',color='b',linewidth='4')
    print(type(energies),'slope    ',type(slope))
    ax.set_xlabel('Energy (GeV)')
    ax.set_ylabel('Mean from fit (MeV)')
    #ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
    ax.set_ylim(0,1200)
    #ax.set_xlim(0,70)

    ax.xaxis.set_major_locator(MultipleLocator(10))
    ax.yaxis.set_major_locator(MultipleLocator(200))   
    ax.xaxis.grid(True)
    ax.yaxis.grid(True)
    #ax.set_title("Fitted Mean vs  Energy")


    chisqLin=np.sum(((means-linear_fit(energies,slope,intercept))/mean_errors)**2)
    plt.figtext(0.15,0.8,"m= {0:.2f} and c={1:.2f}".format(slope,intercept),fontweight='bold',fontsize=40)
    #residuals=means-linear_fit(energies,*poptLin)
    print(chisqLin)
    FigName="MeanE_Energy_{0}.png".format(particle)
    
    plt.savefig(f"{PathToPlot}{FigName}")

    plt.show()

    print(means,energies)

    ############ FOR SIGMA ################
    fig,ax = plt.subplots(1,1, figsize=(16, 12),sharex=True,sharey=True)
    ax.errorbar(energies,stds, std_errors, color="red",marker='o',markersize=20,label='Reconstructed')
    #ax.plot(energies,resolutions, color="blue",marker='*',label="Generated")
    ax.set_xlabel('Energy (GeV)')
    ax.set_ylabel('Sigma (GeV)')
    ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
    #ax.set_ylim(0,10)

    ax.xaxis.set_major_locator(MultipleLocator(10))
    ax.yaxis.set_major_locator(MultipleLocator(20))   
    ax.xaxis.grid(True)
    ax.yaxis.grid(True)
    #ax.set_title("Fitted sigma vs Energy")
    FigName="Sigma_Energy_{0}.png".format(particle)
    plt.savefig(f"{PathToPlot}{FigName}")
    plt.show()
    
    
    ############### Leakage below 2 Sigma ############
    if particle=='pi-':
        ylimit_leakage=20;
    else:
        ylimit_leakage=5;
        
    fig,ax = plt.subplots(1,1, figsize=(16, 12),sharex=True,sharey=True)
    ax.errorbar(energies,leaks_per, leaks_per_error,color="red",marker='o',markersize=20,label='Reconstructed')
    #ax.plot(energies,resolutions, color="blue",marker='*',label="Generated")
    ax.set_ylabel('Leakage [%]')
    ax.set_xlabel('Energy (GeV) ')
    ax.yaxis.set_major_formatter(FormatStrFormatter('%.2f'))
      
    ax.set_ylim(-0.25,ylimit_leakage)
   
    ax.xaxis.set_major_locator(MultipleLocator(10))
    #ax.yaxis.set_major_locator(MultipleLocator(5))   
    ax.xaxis.grid(True)
    ax.yaxis.grid(True)
    #ax.set_title("Fraction of events below {0}$\sigma$".format(Sigma_For_leakage))
    FigName="Leakage_Energy_{0}.png".format(particle)
    plt.savefig(f"{PathToPlot}{FigName}")
    plt.show()


def read_rootfile(ienergy,theta,particle,Time_Threshold,Energy_Threshold):
    FileName="insert_reco_{0}_{1}GeV_theta_{2}deg.edm4hep.root".format(particle,ienergy,theta)
    events=ur.open(f"{FilePathReco}{FileName}:events")
    events.keys()
    arrays = events.arrays()
    #ene = Gev_To_MeV*(ak.flatten(arrays['HcalEndcapPInsertHitsReco.energy'][:])) # in Mev
    ene = Gev_To_MeV*(arrays['HcalEndcapPInsertHitsReco.energy'][:]) # in Mev
    
    time = (arrays['HcalEndcapPInsertHitsReco.time'][:]) # in ns
    #time = (ak.flatten(arrays['HcalEndcapPInsertHitsReco.time'][:])) # in ns
    PosRecoX = (arrays['HcalEndcapPInsertHitsReco.position.x'])/10.0
    PosRecoY = (arrays['HcalEndcapPInsertHitsReco.position.y'])/10.0
    PosRecoZ= (arrays['HcalEndcapPInsertHitsReco.position.z'])/10.0

    
    cut_primary = arrays["MCParticles.generatorStatus"]==1
    px = arrays['MCParticles.momentum.x'][cut_primary]
    py = arrays['MCParticles.momentum.y'][cut_primary]
    pz = arrays['MCParticles.momentum.z'][cut_primary]
    mass = arrays["MCParticles.mass"][cut_primary]
    ID=arrays['MCParticles.PDG']#[cut_primary]
    
    #Calculate generated primary particle properties
    mom = np.sqrt(px**2+py**2+pz**2)
    energy_gen = np.sqrt(mom**2+mass**2)
    #fig = plt.figure( figsize=(8, 6))
    phi = np.arctan2(py,px)


    mask=(ene>Energy_Threshold)  & (time<Time_Threshold) & (ene<1e10)
    ene_good=ene[mask]
     
    return ene,time,PosRecoX,PosRecoY,PosRecoZ,mass,mom,energy_gen,phi,ene_good
