#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <map>
#include <ROOT/TProcessExecutor.hxx>
#include "TTree.h"
#include "TLeaf.h"
#include "TCanvas.h"
#include "TStyle.h"
#include "TF1.h"
#include "TH1.h"
#include "TString.h"
#include "TFile.h"
#include "TROOT.h"
#include "TLatex.h"
#include "TGraph.h"
#include "TRandom.h"

std::map<Double_t, Double_t> Beam_MaxEnergy = {{1.0, 100}, {2.0, 100}, {3.0, 150}, {5.0, 200}, {10.0, 400}, {20.0, 700}, {30.0, 1000}, {40.0, 1500}, {50.0, 2000}, {60., 2500}, {70., 3000}, {80., 3500}, {90., 4000}, {100., 4500}};

Double_t threshold = 6.1;

Double_t reco(Double_t E0)
{
  Double_t E = E0;
  if(E0 < threshold) return 0;
  else{ 
    Double_t a = 0.1;
    Double_t b = 0.0015;
    Double_t sigma =  E* TMath::Sqrt(a*a/E +b*b);
    Double_t random = gRandom->Gaus(E*0.03,sigma);
    E = random;
    return E;
  }
}

void Tuning(Int_t id = 2, Double_t energy = 10., std::string particle = "e-")
{
  const char *data_dir = "/gpfs/mnt/gpfs02/phenix/spin/spin1/phnxsp01/zji/data/eic";
  TString file_name;
  Double_t max_energy = 0.;
  switch(id)
  {
    case 0:
      // Geant4 with ScFi
      file_name.Form("%s/%s_Geant4/%s_%gGeV.root", data_dir, particle.c_str(), particle.c_str(), energy);
      max_energy = Beam_MaxEnergy[energy];
      break;
    case 1:
      // Geant4 with mixture
      file_name.Form("%s/%s_Mixture/%s_%gGeV.root", data_dir, particle.c_str(), particle.c_str(), energy);
      max_energy = Beam_MaxEnergy[energy];
      break;
    case 2:
      // DD4hep
      file_name.Form("%s/endcap/sim_%s_%gGeV_theta_0_5deg.edm4hep.root", data_dir, particle.c_str(), energy);
      max_energy = Beam_MaxEnergy[energy];
      break;
    case 3:
      // Juggler
      file_name.Form("%s/endcap/rec_%s_%gGeV_theta_0_5deg.edm4hep.root", data_dir, particle.c_str(), energy);
      max_energy = energy*1e3;
      break;
  }
  max_energy *= 1.5;
  std::cout<<"Opening "<<file_name<<std::endl;
  TFile* data_file = new TFile(file_name);

  TH1D* h_TotalEdep = new TH1D("h_TotalEdep", "", 600, max_energy/5, max_energy);
  TTree* Total_tree = NULL;
  switch(id)
  {
    case 0:
      Total_tree = (TTree*) data_file->Get("EdepTotal");
      break;
    case 1:
      Total_tree = (TTree*) data_file->Get("B4");
      break;
    default:
      Total_tree = (TTree*) data_file->Get("events");
  }
  Int_t num_events = (Int_t) Total_tree->GetEntries();
  std::cout<<"Number of events: "<<num_events<<std::endl;

  const Int_t max_track = 1000;
  Double_t ECalEdepD[max_track], HCalEdepD[max_track];
  Float_t ECalEdepF[max_track], HCalEdepF[max_track];
  switch(id)
  {
    case 0:
      Total_tree->SetBranchAddress("ECal_Edep_Total", &ECalEdepD[0]);
      //Total_tree->SetBranchAddress("HCal_Edep_Total", &HCalEdepD[0]);
      break;
    case 1:
      Total_tree->SetBranchAddress("Eabs", &ECalEdepD[0]);
      break;
    case 2:
      Total_tree->SetBranchAddress("EcalEndcapPHits.energy", ECalEdepF);
      //Total_tree->SetBranchAddress("HcalEndcapPHits.energy", HCalEdepF);
      break;
    case 3:
      Total_tree->SetBranchAddress("EcalEndcapPRecHits.energy", ECalEdepF);
      //Total_tree->SetBranchAddress("HcalEndcapPHitsReco.energy", HCalEdepF);
      break;
  }

  for(Int_t i = 0; i < num_events; i++)
  {
    Total_tree->GetEntry(i);
    Double_t ECal_energy = 0.;
    //Double_t HCal_energy = 0.;
    switch(id)
    {
      case 0:
        ECal_energy = ECalEdepD[0];
        //HCal_energy = HCalEdepD[0];
        break;
      case 1:
        ECal_energy = reco(ECalEdepD[0]);
        break;
      case 2:
        for(Int_t j=0; j<Total_tree->GetLeaf("EcalEndcapPHits.energy")->GetLen(); j++)
          ECal_energy += ECalEdepF[j]*1e3;
        //for(Int_t j=0; j<Total_tree->GetLeaf("HcalEndcapPHits.energy")->GetLen(); j++)
        //  HCal_energy += HCalEdepF[j]*1e3;
        break;
      case 3:
        for(Int_t j=0; j<Total_tree->GetLeaf("EcalEndcapPRecHits.energy")->GetLen(); j++)
          ECal_energy += ECalEdepF[j]*1e3;
        //for(Int_t j=0; j<Total_tree->GetLeaf("HcalEndcapPHitsReco.energy")->GetLen(); j++)
        //  HCal_energy += HCalEdepF[j]*1e3;
        break;
    }
    if(ECal_energy < 0.183) ECal_energy = 0.;
    Double_t total_energy = ECal_energy;  // + HCal_energy;
    h_TotalEdep->Fill(total_energy);
  }

  TCanvas* c_Resolution = new TCanvas("c_Resolution", "", 1000, 1000);
  gStyle->SetOptFit(1111);
  h_TotalEdep->Draw();
  h_TotalEdep->GetXaxis()->SetTitle("Edep (MeV)");
  h_TotalEdep->GetYaxis()->SetTitle("Number of Events");

  TF1* f_gaus = new TF1("f_gaus", "gaus", max_energy/5, max_energy);
  f_gaus->SetParameter(0, 0.1*num_events);
  f_gaus->SetParameter(1, 0.8*max_energy);
  f_gaus->SetParameter(2, 0.1*max_energy);
  h_TotalEdep->Fit(f_gaus, "");

  Double_t mean = f_gaus->GetParameter(1);
  Double_t sigma = f_gaus->GetParameter(2);
  Double_t resolution;
  if(mean != 0) resolution = sigma/mean;
  else resolution = 0.;

  std::cout<<"Resolution is "<<resolution<<std::endl;

  TString info_text;
  info_text = Form("%s at %0.0f GeV", particle.c_str(), energy);
  switch(id)
  {
    case 0:
      info_text += " in ScFi";
      break;
    case 1:
      info_text += " in Mixture w/ smearing";
      break;
    case 2:
      info_text += " in DD4hep";
      break;
    case 3:
      info_text += " in Juggler";
      break;
  }
  h_TotalEdep->SetTitle(info_text);

  TString res_text;
  res_text = Form("Resolution = %0.5f", resolution);

  TLatex info_caption;
  info_caption.SetTextFont(62);
  info_caption.SetTextSize(.04);
  info_caption.SetNDC(kTRUE);
  info_caption.DrawLatex(.15, .85, info_text);

  TLatex res_caption;
  res_caption.SetTextFont(62);
  res_caption.SetTextSize(.04);
  res_caption.SetNDC(kTRUE);
  res_caption.DrawLatex(.15, .8, res_text);
}
