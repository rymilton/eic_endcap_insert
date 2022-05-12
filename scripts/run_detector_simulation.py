#!/usr/bin/env python
from __future__ import absolute_import, unicode_literals
import os
import time
import logging

import argparse
parser = argparse.ArgumentParser(
     prog='run_detector_simulation',
     description='''This runs the simulation  the but that is okay''',
     epilog='''
     This program should be run in the "compact" directory.
         ''')
parser.add_argument("-v","--verbose", help="increase output verbosity", type=int, default=0)
parser.add_argument("--compact", help="compact detector file",default="athena.xml")
parser.add_argument("--vis", help="vis true/false", action="store_true",default=False)
parser.add_argument("--ui", help="ui setting tcsh or qt; default=qt", type=str,default="qt",dest="ui")
parser.add_argument("-b","--batch", help="batch turns off vis/ui", action="store_true",default=False, dest="batch")
parser.add_argument("-n","--n-events", help="number of events", type=int, default=0)
parser.add_argument("-s","--n-skip", help="number of events to skip", type=int, default=0)
parser.add_argument("-m","--macro", help="macro to execute", default="macro/vis.mac")
parser.add_argument("-o","--output", help="output file", default=None)
parser.add_argument("-i","--input", help="input data file", default=None)
parser.add_argument("--target-position", nargs=3, help="nominal location of the target in units of mm", default=[0.0,0.0,-3000.0],metavar=("X","Y","Z"))

args = parser.parse_args()

import DDG4
from DDG4 import OutputLevel as Output
from g4units import keV, GeV, mm, ns, MeV


def run():
    if not args.vis:
        os.environ['G4UI_USE_TCSH'] = "0"
    else:
    #if args.vis:
        os.environ['G4UI_USE_TCSH'] = "1"

    kernel = DDG4.Kernel()
    description = kernel.detectorDescription()
    kernel.loadGeometry(str("file:" + args.compact))
    DDG4.importConstants(description)

    geant4 = DDG4.Geant4(kernel)
    if args.vis > 0:
        geant4.printDetectors()

    n_events = args.n_events
    if args.vis:
        if args.batch:
            geant4.setupUI(typ=args.ui, vis=True,ui=False, macro=args.macro)
        else:
            geant4.setupUI(typ=args.ui, vis=True,ui=True, macro=args.macro)
    else:
        if args.batch:
            geant4.setupUI(typ=args.ui, vis=False,ui=False)
        else:
            geant4.setupUI(typ=args.ui, vis=True,ui=True, macro=args.macro)
        if n_events <= 0:
            n_events = 1
    kernel.NumEvents = n_events
     
    geant4.setupTrackingField(stepper='ClassicalRK4', equation='Mag_UsualEqRhs')

    rndm = DDG4.Action(kernel, 'Geant4Random/Random')
    if os.getenv('SEED') is None:
        rndm.Seed = 1234
    else :
       rndm.Seed = os.getenv('SEED')
    rndm.initialize()
    rndm.showStatus()

    run1 = DDG4.RunAction(kernel, 'Geant4TestRunAction/RunInit')
    run1.Property_int = 12345
    run1.Property_double = -5e15 * keV
    run1.Property_string = 'Startrun: Hello_2'
    run1.enableUI()
    kernel.registerGlobalAction(run1)
    kernel.runAction().adopt(run1)

    prt = DDG4.EventAction(kernel, 'Geant4ParticlePrint/ParticlePrint')
    #prt.OutputLevel = Output.INFO
    #prt.OutputType = 3  # Print both: table and tree
    kernel.eventAction().adopt(prt)

    outputfile = args.output
    if outputfile is None:
        outputfile = 'data/athena_' + time.strftime('%Y-%m-%d_%H-%M')
    #rootoutput = geant4.setupROOTOutput('RootOutput', outputfile)
    #rootoutput.HandleMCTruth = True

    podio = DDG4.EventAction(kernel, 'Geant4Output2Podio/RootOutput', True)
    podio.HandleMCTruth = False
    podio.Control = True
    podio.Output = outputfile
    podio.enableUI()
    kernel.eventAction().adopt(podio)

    #--------------------------------

    gen = DDG4.GeneratorAction(kernel, "Geant4GeneratorActionInit/GenerationInit")
    gen.OutputLevel = 1
    gen.enableUI()
    kernel.generatorAction().adopt(gen)

    #gen = DDG4.GeneratorAction(kernel, "Geant4InputAction/hepmc3")
    #gen.Parameters["Flow1"] = "flow1"
    #gen.Parameters["Flow2"] = "flow2"

    inputfile = args.input
    if inputfile is None:
        inputfile = "data/proton_dvcs_eic.hepmc"
    gen = DDG4.GeneratorAction(kernel, "Geant4InputAction/hepmc3")
    gen.Input = "HEPMC3FileReader|" + inputfile 
    gen.OutputLevel = 0  # generator_output_level
    gen.Mask = 0
    gen.Sync = args.n_skip
    gen.enableUI()
    kernel.generatorAction().adopt(gen)

    gen = DDG4.GeneratorAction(kernel, "EICInteractionVertexSmear/BeamDivergence")
    gen.Mask = 0
    gen.OutputLevel = 5  # generator_output_level
    #gen.Sigma = (4 * mm, 1 * mm, 1 * mm, 0 * ns)
    kernel.generatorAction().adopt(gen)

    gen = DDG4.GeneratorAction(kernel, "EICInteractionVertexBoost/CrossingAngle")
    gen.Mask = 0
    gen.OutputLevel = 5  # generator_output_level
    #gen.Sigma = (4 * mm, 1 * mm, 1 * mm, 0 * ns)
    kernel.generatorAction().adopt(gen)

    #gen = DDG4.GeneratorAction(kernel, "Geant4InteractionVertexSmear/SmearVert")
    #gen.Offset = (0 * mm, 0 * mm, 0 * mm, 0 * ns)
    #gen.Sigma = (3 * mm, 3 * mm, 200 * mm, 0 * ns)
    #kernel.generatorAction().adopt(gen)

    #gen = DDG4.GeneratorAction(kernel, "Geant4GeneratorWrapper/GPS")
    #gen.Uses = 'G4GeneralParticleSource'
    ##gen.OutputLevel = Output.WARNING
    #gen.enableUI()
    #kernel.generatorAction().adopt(gen)

    gen = DDG4.GeneratorAction(kernel, "Geant4InteractionMerger/InteractionMerger")
    #gen.OutputLevel = 0  # generator_output_level
    gen.enableUI()
    kernel.generatorAction().adopt(gen)

    gen = DDG4.GeneratorAction(kernel, "Geant4PrimaryHandler/PrimaryHandler")
    #gen.OutputLevel = 0  # generator_output_level
    gen.enableUI()
    kernel.generatorAction().adopt(gen)

    part = DDG4.GeneratorAction(kernel, "Geant4ParticleHandler/ParticleHandler")
    # part.SaveProcesses = ['conv','Decay']
    #part.SaveProcesses = ['Decay']
    part.MinimalKineticEnergy = 100000 * GeV
    #part.OutputLevel = 0  # generator_output_level
    part.enableUI()
    kernel.generatorAction().adopt(part)


    #gen = DDG4.GeneratorAction(kernel, "Geant4IsotropeGenerator/IsotropE-")
    #gen.Particle = 'e-'
    #gen.Energy = 25 * GeV
    #gen.Multiplicity = 3
    #gen.Distribution = 'uniform'
    #kernel.generatorAction().adopt(gen)

    #gen = DDG4.GeneratorAction(kernel, "Geant4InteractionMerger/InteractionMerger")
    #gen.OutputLevel = 4  # generator_output_level
    #gen.enableUI()
    #kernel.generatorAction().adopt(gen)

    #gen = DDG4.GeneratorAction(kernel, "Geant4PrimaryHandler/PrimaryHandler")
    #gen.OutputLevel = 4  # generator_output_level
    #gen.enableUI()
    #kernel.generatorAction().adopt(gen)

    #part = DDG4.GeneratorAction(kernel, "Geant4ParticleHandler/ParticleHandler")
    #kernel.generatorAction().adopt(part)
    ## part.SaveProcesses = ['conv','Decay']
    #part.SaveProcesses = ['Decay']
    #part.MinimalKineticEnergy = 100 * MeV
    #part.OutputLevel = 5  # generator_output_level
    #part.enableUI()

    #user = DDG4.Action(kernel, "Geant4TCUserParticleHandler/UserParticleHandler")
    #user.TrackingVolume_Zmax = DDG4.EcalEndcap_zmin
    #user.TrackingVolume_Rmax = DDG4.EcalBarrel_rmin
    #user.enableUI()
    #part.adopt(user)

    #f1 = DDG4.Filter(kernel, 'GeantinoRejectFilter/GeantinoRejector')

    f2 = DDG4.Filter(kernel, 'ParticleRejectFilter/OpticalPhotonRejector')
    f2.particle = 'opticalphoton'

    f3 = DDG4.Filter(kernel, 'ParticleSelectFilter/OpticalPhotonSelector')
    f3.particle = 'opticalphoton'

    #f4 = DDG4.Filter(kernel, 'EnergyDepositMinimumCut')
    #f4.Cut = 10 * MeV

    #f4.enableUI()
    #kernel.registerGlobalFilter(f1)
    kernel.registerGlobalFilter(f2)
    kernel.registerGlobalFilter(f3)
    #kernel.registerGlobalFilter(f4)

    #seq, act = geant4.setupDetector('ForwardRICH','PhotoMultiplierSDAction')
    #act.adopt(f3)
    #seq, act = geant4.setupDetector('HeavyGasCherenkov','PhotoMultiplierSDAction')
    #act.adopt(f3)

    #seq, act = geant4.setupTracker('SiTrackerBarrel_Layer1')
    #seq, act = geant4.setupTracker('SiTrackerEndcapP_Layer1')
    #seq, act = geant4.setupTracker('SiTrackerEndcapN_Layer1')
    #seq, act = geant4.setupTracker('SiTrackerBarrel_Layer2')
    #seq, act = geant4.setupTracker('SiTrackerEndcapP_Layer2')
    #seq, act = geant4.setupTracker('SiTrackerEndcapN_Layer2')
    #seq, act = geant4.setupTracker('SiTrackerBarrel_Layer3')
    #seq, act = geant4.setupTracker('SiTrackerEndcapP_Layer3')
    #seq, act = geant4.setupTracker('SiTrackerEndcapN_Layer3')
    #seq, act = geant4.setupTracker('SiTrackerBarrel_Layer4')
    #seq, act = geant4.setupTracker('SiTrackerEndcapP_Layer4')
    #seq, act = geant4.setupTracker('SiTrackerEndcapN_Layer4')
    #seq, act = geant4.setupTracker('SiTrackerBarrel_Layer5')
    #seq, act = geant4.setupTracker('SiTrackerEndcapP_Layer5')
    #seq, act = geant4.setupTracker('SiTrackerEndcapN_Layer5')
    #seq, act = geant4.setupTracker('SiVertexBarrel')
    #seq, act = geant4.setupTracker('SiTrackerForward')
    #seq, act = geant4.setupCalorimeter('EcalBarrel')
    #seq, act = geant4.setupCalorimeter('EcalEndcap')
    #seq, act = geant4.setupCalorimeter('HcalBarrel')
    #seq, act = geant4.setupTracker('RomanPot1')
    #seq, act = geant4.setupTracker('RomanPot2')
    #seq, act = geant4.setupTracker('RomanPot3')
    #seq, act = geant4.setupTracker('RomanPot4')
    #seq, act = geant4.setupTracker('RomanPot44')
    #seq, act = geant4.setupTracker('RomanPot45')
    #seq, act = geant4.setupTracker('RomanPot46')
    #seq, act = geant4.setupTracker('RomanPot47')
    #seq, act = geant4.setupTracker('RomanPot48')
    #seq, act = geant4.setupTracker('RomanPot49')
    #seq, act = geant4.setupTracker('RomanPot50')
    #seq, act = geant4.setupTracker('RomanPot51')

    phys = geant4.setupPhysics('QGSP_BERT')
    geant4.addPhysics(str('Geant4PhysicsList/Myphysics'))

    ph = DDG4.PhysicsList(kernel, 'Geant4OpticalPhotonPhysics/OpticalPhotonPhys')
    ph.VerboseLevel = 0
    ph.enableUI()
    phys.adopt(ph)

    ph = DDG4.PhysicsList(kernel, 'Geant4CerenkovPhysics/CerenkovPhys')
    ph.MaxNumPhotonsPerStep = 10
    ph.MaxBetaChangePerStep = 10.0
    ph.TrackSecondariesFirst = True
    ph.VerboseLevel = 0
    ph.enableUI()
    phys.adopt(ph)

    ## Add special particle types from specialized physics constructor
    #part = geant4.addPhysics('Geant4ExtraParticles/ExtraParticles')
    #part.pdgfile = 'checkout/DDG4/examples/particle.tbl'

    # Add global range cut
    rg = geant4.addPhysics('Geant4DefaultRangeCut/GlobalRangeCut')
    rg.RangeCut = 0.7 * mm

    phys.dump()

    #ui_action = dd4hep.sim.createAction(kernel, "Geant4UIManager/UI")
    #ui_action.HaveVIS = True
    #ui_action.HaveUI = True
    #ui_action.SessionType = qt
    #ui_action.SetupUI = macro
    #kernel.registerGlobalAction(ui_action)

    kernel.configure()
    kernel.initialize()

    # DDG4.setPrintLevel(Output.DEBUG)
    kernel.run()
    kernel.terminate()

if __name__ == "__main__":
    run()
