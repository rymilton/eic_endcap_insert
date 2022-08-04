#include <iostream>
#include <array>
#include <algorithm>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TGraph.h>
#include <TMultiLayerPerceptron.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>

using namespace std;

void DrawPi0gg(Double_t energy = 80.)
{
  const Int_t nd = 5;
  const Int_t nb = 1;
  const Int_t nin = (2*nb+1)*(2*nb+1);
  Int_t ntruth;
  array<Float_t, nin> v_in;
  Float_t center_x, center_y, pout;

  auto rnd = new TRandom3(0);

  auto f_data = new TFile("results/training-data.root", "RECREATE");
  auto t_data = new TTree("T", "Training data");
  t_data->Branch("ntruth", &ntruth, "ntruth/I");
  for(Int_t i=0; i<nin; i++)
    t_data->Branch(Form("e%d", i+1), &v_in[i], Form("e%d/F", i+1));
  t_data->Branch("center_x", &center_x, "center_x/F");
  t_data->Branch("center_y", &center_y, "center_y/F");
  t_data->Branch("p", &pout, "p/F");

  const char *data_dir = "/gpfs/mnt/gpfs02/phenix/spin/spin1/phnxsp01/zji/data/eic";
  const char *particle[2] = {"gamma", "pi0"};
  TTree *t_input[2];
  Long64_t nen[2] = {};
  for(Int_t it=0; it<2; it++)
  {
    auto f_input = new TFile(Form("%s/histos/training-%s_%gGeV_theta_15_20deg.root", data_dir, particle[it], energy));
    t_input[it] = (TTree*)f_input->Get("T");
    nen[it] = t_input[it]->GetEntries();
    for(Int_t i=0; i<nin; i++)
      t_input[it]->SetBranchAddress(Form("e%d",i+1), &v_in[i]);
    t_input[it]->SetBranchAddress("center_x", &center_x);
    t_input[it]->SetBranchAddress("center_y", &center_y);
  }

  Long64_t ien[2] = {};
  while(ien[0]+ien[1] < nen[0]+nen[1])
  {
    v_in.fill(0.);
    ntruth = rnd->Integer(2) + 1;
    if(ien[0] >= nen[0]) ntruth = 2;
    else if(ien[1] >= nen[1]) ntruth = 1;
    t_input[ntruth-1]->GetEntry(ien[ntruth-1]++);
    pout = 2 - ntruth;
    t_data->Fill();
  }

  string str_node = "center_x,center_y,e1";
  for(Int_t i=1; i<nin; i++)
    str_node += Form(",e%d", i+1);
  str_node += Form(":%d:p!", nin+6);

  auto mlp = new TMultiLayerPerceptron(
      str_node.c_str(),
      t_data, "Entry$%2!=0", "Entry$%2==0");

  mlp->SetLearningMethod(TMultiLayerPerceptron::kStochastic);
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

  for(Long64_t ien=0; ien<t_data->GetEntries(); ien+=2)
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

  auto c0 = new TCanvas("c0", "c0", 600, 600);
  auto leg0 = new TLegend(0.2, 0.2);
  const char *leg0_text[3] = {"BG eff", "Sig eff", "S/#sqrt{S+B}"};
  for(Int_t ig=0; ig<3; ig++)
  {
    g_eff[ig]->SetTitle("Efficiency for photon");
    g_eff[ig]->GetXaxis()->SetTitle("Cut value");
    g_eff[ig]->GetYaxis()->SetTitle("Efficiency");
    g_eff[ig]->GetYaxis()->SetRangeUser(0., 1.);
    g_eff[ig]->SetLineStyle(20+ig);
    g_eff[ig]->SetLineColor(1+ig);
    g_eff[ig]->SetLineWidth(2);
    g_eff[ig]->Draw(ig==0?"AL":"L");
    leg0->AddEntry(Form("g_eff_%d",ig), leg0_text[ig], "L");
  }
  leg0->Draw();
  c0->Print("results/Pi0gg-15_20deg-Stochastic-80GeV-xy.pdf");
}
