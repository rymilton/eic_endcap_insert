'''
    An example option file to digitize/reconstruct/clustering calorimeter hits
'''
from Gaudi.Configuration import *
import json
import os
import ROOT

from Configurables import ApplicationMgr, EICDataSvc, PodioInput, PodioOutput, GeoSvc
from GaudiKernel.SystemOfUnits import MeV, GeV, mm, cm, mrad

detector_name = "${DETECTOR}"
detector_path = "${DETECTOR_PATH}"
compact_path = os.path.join(detector_path, detector_name)

# get sampling fractions from system environment variable, 1.0 by default
ci_zdc_hcal_sf = "1."

# input and output
input_sims = [f.strip() for f in str.split(os.environ["JUGGLER_SIM_FILE"], ",") if f.strip()]
output_rec = str(os.environ["JUGGLER_REC_FILE"])
n_events = int(os.environ["JUGGLER_N_EVENTS"])

# geometry service
geo_service = GeoSvc("GeoSvc", detectors=["{}.xml".format(compact_path)], OutputLevel=INFO)
# data service
podioevent = EICDataSvc("EventDataSvc", inputs=input_sims)


# juggler components
from Configurables import Jug__Digi__CalorimeterHitDigi as CalHitDigi
from Configurables import Jug__Reco__CalorimeterHitReco as CalHitReco

from Configurables import Jug__Fast__InclusiveKinematicsTruth as InclusiveKinematicsTruth

# branches needed from simulation root file
sim_coll = [
    "MCParticles",
    "ZDCHcalHits",
    "ZDCHcalHitsContributions"
]

# input and output
podin = PodioInput("PodioReader", collections=sim_coll)
podout = PodioOutput("out", filename=output_rec)

# ZDC Hcal
ci_zdc_hcal_daq = dict(
         dynamicRangeADC=200.*MeV,
         capacityADC=32768,
         pedestalMean=400,
         pedestalSigma=10)
ci_zdc_hcal_digi = CalHitDigi("ci_zdc_hcal_digi",
         inputHitCollection="ZDCHcalHits",
         outputHitCollection="ZDCHcalHitsDigi",
         **ci_zdc_hcal_daq)
ci_zdc_hcal_reco = CalHitReco("ci_hcal_reco",
        inputHitCollection=ci_zdc_hcal_digi.outputHitCollection,
        outputHitCollection="ZDCHcalHitsReco",
        thresholdFactor=0.0,
        samplingFraction=ci_zdc_hcal_sf,
        **ci_zdc_hcal_daq)

# Truth level kinematics
truth_incl_kin = InclusiveKinematicsTruth("truth_incl_kin",
        inputMCParticles = "MCParticles",
        outputInclusiveKinematics = "InclusiveKinematicsTruth"
)

# Output
podout.outputCommands = ['drop *',
        'keep MCParticles',
        'keep *Digi',
        'keep *Reco*',
	'keep Inclusive*']

ApplicationMgr(
    TopAlg = [podin,
            ci_zdc_hcal_digi, ci_zdc_hcal_reco,
	    truth_incl_kin, podout],
    EvtSel = 'NONE',
    EvtMax = n_events,
    ExtSvc = [podioevent],
    OutputLevel=DEBUG
)
