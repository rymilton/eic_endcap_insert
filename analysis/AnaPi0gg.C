#include <iostream>
#include <array>
#include <algorithm>

#include <TMath.h>
#include <TString.h>
#include <TRandom3.h>
#include <TFile.h>
#include <TTree.h>
#include <TLeaf.h>
#include <TH2.h>

using namespace std;

template<class T> inline constexpr T square(const T &x) { return x*x; }
template<class T> inline constexpr T point_dis(const T &x1, const T &y1, const T &x2, const T &y2) { return sqrt(square(x1-x2)+square(y1-y2)); }

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

void AnaPi0gg(Double_t energy, Int_t proc, string particle = "pi0")
{
  const char *data_dir = "/gpfs/mnt/gpfs02/phenix/spin/spin1/phnxsp01/zji/data/eic";
  TString file_name;
  file_name.Form("%s/endcap/sim_%s_%gGeV_theta_15_20deg-%d.root", data_dir, particle.c_str(), energy, proc);
  auto data_file = new TFile(file_name);
  if(!data_file || !data_file->IsOpen())
  {
    cerr << "Error opening file " << file_name << endl;
    return;
  }
  else
  {
    cout << "Opening " << file_name << endl;
  }

  const Float_t tsize_x = 24.925;
  const Float_t tsize_y = 24.65;
  const Float_t tsize = (tsize_x + tsize_y) / 2.;

  const Int_t nd = 5;
  const Int_t nb = 1;
  const Int_t nin = (2*nb+1)*(2*nb+1);
  Int_t ntruth;
  array<Float_t, nin> v_in;
  Float_t pout;

  file_name.Form("%s/histos/training-%s_%gGeV_theta_15_20deg-%d.root", data_dir, particle.c_str(), energy, proc);
  auto f_out = new TFile(file_name, "RECREATE");
  auto t_data = new TTree("T", "Training data");
  t_data->Branch("ntruth", &ntruth, "ntruth/I");
  for(Int_t i=0; i<nin; i++)
    t_data->Branch(Form("e%d", i+1), &v_in[i], Form("e%d/F", i+1));
  t_data->Branch("p", &pout, "p/F");

  auto h2_e = new TH2F("h2_e", "Energy;x;y", 2*nd+1,-nd-0.5,nd+0.5, 2*nd+1,-nd-0.5,nd+0.5);

  auto rnd = new TRandom3(0);

  const Int_t ntrack = 1000;
  Int_t pid[ntrack];
  Double_t mc_x[ntrack], mc_y[ntrack];
  Float_t hit_x[ntrack], hit_y[ntrack], hit_e[ntrack];
  TTree *events = (TTree*)data_file->Get("events");
  events->SetBranchAddress("MCParticles.PDG", pid);
  events->SetBranchAddress("MCParticles.endpoint.x", mc_x);
  events->SetBranchAddress("MCParticles.endpoint.y", mc_y);
  events->SetBranchAddress("EcalEndcapPHits.position.x", hit_x);
  events->SetBranchAddress("EcalEndcapPHits.position.y", hit_y);
  events->SetBranchAddress("EcalEndcapPHits.energy", hit_e);

  for(Long64_t iev = 0; iev < events->GetEntries(); iev++)
  {
    events->GetEntry(iev);

    ntruth = 0;
    Int_t imcg[ntrack];
    for(Int_t imc = 2; imc < events->GetLeaf("MCParticles.PDG")->GetLen(); imc++)
      if(pid[imc] == 22)
        imcg[ntruth++] = imc;

    if(ntruth <= 0 || ntruth > 2)
      continue;

    if(ntruth == 2 && point_dis(mc_x[imcg[0]], mc_y[imcg[0]], mc_x[imcg[1]], mc_y[imcg[1]]) > 1.5*tsize)
      continue;

    v_in.fill(0.);
    pout = ntruth - 1;

    Float_t sum_in = 0.;
    Int_t nhit = events->GetLeaf("EcalEndcapPHits.energy")->GetLen();
    Int_t ip = distance(hit_e, max_element(hit_e, hit_e+nhit));
    for(Int_t ihit = 0; ihit < nhit; ihit++)
    {
      Int_t ix = round((hit_x[ihit] - hit_x[ip]) / tsize_x);
      Int_t iy = round((hit_y[ihit] - hit_y[ip]) / tsize_y);
      Float_t ereco = reco(hit_e[ihit]*1e3, rnd);
      h2_e->Fill(ix, iy, ereco);
      if(abs(ix) <= nb && abs(iy) <= nb)
      {
        v_in[(ix+nb)*(2*nb+1)+(iy+nb)] = ereco;
        sum_in += ereco;
      }
    } // ihit

    if(sum_in > 0.)
      for(Int_t i=-nb; i<=nb; i++)
        for(Int_t j=-nb; j<=nb; j++)
          v_in[(i+nb)*(2*nb+1)+(j+nb)] /= sum_in;

    t_data->Fill();
  } // iev

  f_out->Write();
  f_out->Close();
}
