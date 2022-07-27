#include <iostream>
#include <array>
#include <algorithm>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TGraph.h>
#include <TMultiLayerPerceptron.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>

using namespace std;

void DrawPi0gg(Double_t energy = 60.)
{
  const char *data_dir = "/gpfs/mnt/gpfs02/phenix/spin/spin1/phnxsp01/zji/data/eic";
  TString file_name;
  file_name.Form("%s/histos/training-%gGeV_theta_15_20deg.root", data_dir, energy);
  auto f = new TFile(file_name);
  if(!f || !f->IsOpen())
  {
    cerr << "Error opening file " << file_name << endl;
    return;
  }
  else
  {
    cout << "Opening " << file_name << endl;
  }

  const Int_t nd = 5;
  const Int_t nb = 1;
  const Int_t nin = (2*nb+1)*(2*nb+1);
  Int_t ntruth;
  array<Float_t, nin> v_in;
  Float_t pout;

  auto rnd = new TRandom3(0);

  //auto t_data = (TTree*)f->Get("T");
  //t_data->SetBranchAddress("ntruth", &ntruth);
  //for(Int_t i=0; i<nin; i++)
  //  t_data->SetBranchAddress(Form("e%d", i+1), &v_in[i]);
  //t_data->SetBranchAddress("p", &pout);

  auto t_data = new TTree("T", "Training data");
  t_data->Branch("ntruth", &ntruth, "ntruth/I");
  for(Int_t i=0; i<nin; i++)
    t_data->Branch(Form("e%d", i+1), &v_in[i], Form("e%d/F", i+1));
  t_data->Branch("p", &pout, "p/F");

  //auto f_g = new TFile(Form("%s/histos/g.root", data_dir));
  //auto t_g = (TTree*)f_g->Get("treeML");
  auto f_g = new TFile(Form("%s/histos/training-gamma_60GeV_theta_15_20deg-0.root", data_dir));
  auto t_g = (TTree*)f_g->Get("T");
  for(Int_t i=0; i<nin; i++)
    t_g->SetBranchAddress(Form("e%d",i+1), &v_in[i]);

  //auto f_pi0 = new TFile(Form("%s/histos/pi0.root", data_dir));
  //auto t_pi0 = (TTree*)f_pi0->Get("treeML");
  auto f_pi0 = new TFile(Form("%s/histos/training-pi0_60GeV_theta_15_20deg-0.root", data_dir));
  auto t_pi0 = (TTree*)f_pi0->Get("T");
  for(Int_t i=0; i<nin; i++)
    t_pi0->SetBranchAddress(Form("e%d",i+1), &v_in[i]);

  Int_t ig = 0;
  Int_t ipi = 0;
  for(Long64_t ien = 0; ien < 2000; ien++)
  {
    v_in.fill(0.);
    ntruth = rnd->Integer(2) + 1;
    if(ntruth == 1 && ig < 1000)
      t_g->GetEntry(ig++);
    else
      t_pi0->GetEntry(ipi++);
    pout = 2 - ntruth;
    t_data->Fill();
  }

  auto h2_e = (TH2*)f->Get("h2_e");

  auto c0 = new TCanvas("c0", "c0", 600, 600);
  gStyle->SetOptStat(0);
  h2_e->Draw("COLZ");

  string str_node = "e1";
  for(Int_t i=1; i<nin; i++)
    str_node += Form(",e%d", i+1);
  str_node += Form(":%d:p!", nin+6);

  auto mlp = new TMultiLayerPerceptron(
      str_node.c_str(),
      t_data, "Entry$%2==0", "Entry$%2!=0");

  mlp->SetLearningMethod(TMultiLayerPerceptron::kBFGS);
  mlp->Train(600, "text, graph, update=100");

  const Int_t nbins = 100;
  TH1 *h_p[2];
  TGraph *g_eff[2];
  for(Int_t ig=0; ig<3; ig++)
  {
    h_p[ig] = new TH1F(Form("h_p_%d",ig), "p", nbins,0.,1.);
    g_eff[ig] = new TGraph(nbins);
    g_eff[ig]->SetName(Form("g_eff_%d",ig));
  }

  for(Long64_t ien=1; ien<t_data->GetEntries(); ien+=2)
  {
    t_data->GetEntry(ien);
    array<Double_t, nin> v_inf;
    copy(v_in.begin(), v_in.end(), v_inf.begin());
    Float_t pinf = mlp->Evaluate(0, v_inf.begin());
    h_p[ntruth-1]->Fill(pinf);
  }

  Float_t n_total[2];
  for(Int_t ig=0; ig<2; ig++)
    n_total[ig] = h_p[ig]->Integral(1, nbins);

  for(Int_t ib=0; ib<nbins; ib++)
  {
    Float_t p = h_p[0]->GetXaxis()->GetBinCenter(1+ib);
    Float_t n_eff[2];
    for(Int_t ig=0; ig<2; ig++)
    {
      n_eff[ig] = h_p[ig]->Integral(1+ib, nbins);
      g_eff[ig]->SetPoint(ib, p, n_eff[ig]/n_total[ig]);
    }
    Float_t sb = n_eff[0] / sqrt(n_eff[0] + n_eff[1]) / sqrt(n_total[0]);
    g_eff[2]->SetPoint(ib, p, sb);
  }

  auto c1 = new TCanvas("c1", "c1", 600, 600);
  auto leg0 = new TLegend(0.2, 0.2);
  const char *leg0_text[3] = {"BG eff", "Sig eff", "S/#sqrt{S+B}"};
  for(Int_t ig=0; ig<3; ig++)
  {
    g_eff[ig]->SetTitle("Efficiency for #pi^{0}");
    g_eff[ig]->GetXaxis()->SetTitle("Cut value");
    g_eff[ig]->GetYaxis()->SetTitle("Efficiency");
    g_eff[ig]->SetLineStyle(20+ig);
    g_eff[ig]->SetLineColor(1+ig);
    g_eff[ig]->SetLineWidth(2);
    g_eff[ig]->Draw(ig==0?"AL":"L");
    leg0->AddEntry(Form("g_eff_%d",ig), leg0_text[ig], "L");
  }
  leg0->Draw();
  c1->Print("results/Pi0gg.pdf");
}
