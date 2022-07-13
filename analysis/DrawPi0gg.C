#include <iostream>
#include <array>
#include <algorithm>

#include <TString.h>
#include <TFile.h>
#include <TTree.h>
#include <TH2.h>
#include <TMultiLayerPerceptron.h>
#include <TCanvas.h>
#include <TStyle.h>

using namespace std;

void DrawPi0gg()
{
  const char *data_dir = "/gpfs/mnt/gpfs02/phenix/spin/spin1/phnxsp01/zji/data/eic";
  TString file_name;
  file_name.Form("%s/histos/training.root", data_dir);
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
  const Int_t nout = 2;
  Int_t ntruth;
  array<Float_t, nin> v_in;
  array<Float_t, nout> v_out;

  auto t_data = (TTree*)f->Get("T");
  t_data->SetBranchAddress("ntruth", &ntruth);
  for(Int_t i=0; i<nin; i++)
    t_data->SetBranchAddress(Form("e%d", i+1), &v_in[i]);
  for(Int_t i=0; i<nout; i++)
    t_data->SetBranchAddress(Form("n%d", i+1), &v_out[i]);

  auto h2_e = (TH2*)f->Get("h2_e");

  auto c0 = new TCanvas("c0", "c0", 600, 600);
  gStyle->SetOptStat(0);
  h2_e->Draw("COLZ");

  string str_node = "e1";
  for(Int_t i=1; i<nin; i++)
    str_node += Form(",e%d", i+1);
  str_node += Form(":%d:n1,n2!", nin+5);

  auto mlp = new TMultiLayerPerceptron(
      str_node.c_str(),
      t_data, "Entry$%10!=0", "Entry$%10==0");

  // Use kStochastic, kBatch doesn't work
  mlp->SetLearningMethod(TMultiLayerPerceptron::kStochastic);
  mlp->Train(10, "text, graph, update=1");

  Long64_t ntrain = t_data->GetEntries();
  Long64_t correct = 0;
  for(Long64_t ien = 0; ien < ntrain; ien++)
  {
    t_data->GetEntry(ien);
    array<Double_t, nin> v_inf;
    copy(v_in.begin(), v_in.end(), v_inf.begin());
    Double_t n1 = mlp->Evaluate(0, v_inf.begin());
    Double_t n2 = mlp->Evaluate(1, v_inf.begin());
    Int_t npred = n1 > n2 ? 1 : 2;
    if(npred == ntruth)
      correct++;
  }
  cout << "correct/event = " << correct << "/" << ntrain << " (" << 100.*correct/ntrain << "%)" << endl;
}
