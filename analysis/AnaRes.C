#include <iostream>
#include <tuple>
#include <vector>
#include <map>

#include <TMath.h>
#include <TF1.h>
#include <TString.h>
#include <TRandom3.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TH1.h>
#include <TGraphErrors.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLatex.h>
#include <TLegend.h>

using namespace std;

template<class T> inline constexpr T square(const T &x) { return x*x; }

Float_t reco(Float_t E0, TRandom *rnd)
{
  const Float_t threshold = 6.1;
  if(E0 < threshold)
  {
    return 0;
  }
  else
  {
    Float_t a = 0.1;
    Float_t b = 0.0015;
    Float_t sigma =  E0 * TMath::Sqrt(a*a/E0 + b*b);
    return TMath::Abs(rnd->Gaus(E0*0.03, sigma));
  }
}

map<Double_t, Double_t> Beam_MaxEnergy = {{1.0, 100}, {2.0, 100}, {5.0, 200}, {10.0, 400}, {20.0, 700}, {30.0, 1000}, {40.0, 1500}, {50.0, 2000}, {60., 2500}, {70., 3000}, {80., 3500}, {90., 4000}, {100., 4500}};

tuple<Double_t, Double_t> calc_mean_sigma(Double_t energy, Double_t max_energy, Double_t angle, string particle)
{
  const char *data_dir = "/gpfs/mnt/gpfs02/phenix/spin/spin1/phnxsp01/zji/data/eic/insert";
  TString file_name;
  file_name.Form("%s/sim_%s_%gGeV_theta_%gdeg.root", data_dir, particle.c_str(), energy, angle);
  cout << "Opening " << file_name << endl;
  auto data_file = new TFile(file_name);
  if(!data_file->IsOpen())
    return make_tuple(-1., -1.);

  auto h_TotalEdep = new TH1D("h_TotalEdep", "", 1000, 0., max_energy);

  auto rnd = new TRandom3(0);

  const Int_t max_track = 1000;
  Float_t ECalEdep[max_track];
  TTree *Total_tree = (TTree*)data_file->Get("events");
  Total_tree->SetBranchAddress("EcalEndcapPInsertHits.energy", ECalEdep);
  Long64_t num_events = Total_tree->GetEntries();

  for(Long64_t i = 0; i < num_events; i++)
  {
    Total_tree->GetEntry(i);
    Double_t ECal_energy = 0.;
    for(Long64_t j=0; j<Total_tree->GetLeaf("EcalEndcapPInsertHits.energy")->GetLen(); j++)
      ECal_energy += reco(ECalEdep[j]*1e3, rnd);
    Double_t total_energy = ECal_energy;
    h_TotalEdep->Fill(total_energy);
  }

  h_TotalEdep->Draw();
  h_TotalEdep->GetXaxis()->SetTitle("Edep (MeV)");
  h_TotalEdep->GetYaxis()->SetTitle("Number of Events");

  TF1* f_gaus = new TF1("f_gaus", "gaus", 0., max_energy);
  f_gaus->SetParameter(0, 0.1*num_events);
  f_gaus->SetParameter(1, 0.8*max_energy);
  f_gaus->SetParameter(2, 0.1*max_energy);
  h_TotalEdep->Fit(f_gaus, "RQ");

  Double_t mean, sigma;
  if(particle == "e-" || particle == "e+" || particle == "gamma")
  {
    mean = f_gaus->GetParameter(1);
    sigma = f_gaus->GetParameter(2);
  }
  else
  {
    mean = h_TotalEdep->GetMean();
    sigma = h_TotalEdep->GetRMS();
  }

  TString info_text;
  info_text = Form("%s at %0.0f GeV at %g degree", particle.c_str(), energy, angle);
  h_TotalEdep->SetTitle(info_text);

  TLatex info_caption;
  info_caption.SetTextFont(62);
  info_caption.SetTextSize(.04);
  info_caption.SetNDC(kTRUE);
  info_caption.DrawLatex(.15, .85, info_text);

  delete data_file;
  return make_tuple(mean, sigma);
}

void AnaRes()
{
  Int_t ien = 0;
  Int_t nen = Beam_MaxEnergy.size();
  Int_t nrow = ceil(sqrt(nen));

  auto f_out = new TFile("results/resolution.root", "RECREATE");
  auto g_mean_e = new TGraphErrors(nen);
  auto g_mean_pi = new TGraphErrors(nen);
  auto g_res_e = new TGraphErrors(nen);
  auto g_res_pi = new TGraphErrors(nen);
  g_mean_e->SetName("g_mean_e");
  g_mean_pi->SetName("g_mean_pi");
  g_res_e->SetName("g_res_e");
  g_res_pi->SetName("g_res_pi");

  auto c0 = new TCanvas("c0", "c0", 600*nrow, 600*nrow);
  auto c1 = new TCanvas("c1", "c1", 600*nrow, 600*nrow);
  c0->Divide(nrow, nrow);
  c1->Divide(nrow, nrow);
  gStyle->SetOptFit(1111);

  for(const auto &[energy, deposit] : Beam_MaxEnergy)
  {
    c0->cd(ien+1);
    auto [mean_e, sigma_e] = calc_mean_sigma(energy, energy*60, 2.83, "e-");
    c1->cd(ien+1);
    auto [mean_pi, sigma_pi] = calc_mean_sigma(energy, 20, 2.83, "pi-");
    if(mean_e < 0. || mean_pi < 0.) continue;
    g_mean_e->SetPoint(ien, energy, mean_e);
    g_mean_e->SetPointError(ien, 0., sigma_e);
    g_mean_pi->SetPoint(ien, energy, mean_pi);
    g_mean_pi->SetPointError(ien, 0., sigma_pi);
    g_res_e->SetPoint(ien, energy, sigma_e/mean_e);
    g_res_e->SetPointError(ien, 0., square(sigma_e/mean_e));
    g_res_pi->SetPoint(ien, energy, sigma_pi/mean_pi);
    g_res_pi->SetPointError(ien, 0., square(sigma_pi/mean_pi));
    ien++;
  }
  g_mean_e->Set(ien);
  g_mean_pi->Set(ien);
  g_res_e->Set(ien);
  g_res_pi->Set(ien);

  f_out->cd();
  g_mean_e->Write();
  g_mean_pi->Write();
  g_res_e->Write();
  g_res_pi->Write();
  f_out->Close();
}
