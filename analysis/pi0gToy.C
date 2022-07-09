#include <iostream>
#include <string>
#include <array>
#include <algorithm>

#include <TMath.h>
#include <TRandom3.h>
#include <TTree.h>
#include <TH2.h>
#include <TMultiLayerPerceptron.h>
#include <TCanvas.h>
#include <TStyle.h>

using namespace std;

void pi0gToy(const Int_t nev = 10000)
{
  const Int_t nd = 5;
  const Int_t nb = 1;
  const Int_t nin = (2*nb+1)*(2*nb+1);
  const Int_t nout = 2;
  Int_t ntruth;
  array<Double_t, (2*nd+1)*(2*nd+1)> v_e;
  array<Double_t, nin> v_in;
  array<Double_t, nout> v_out;

  auto t_data = new TTree("T", "Training data");
  t_data->Branch("ntruth", &ntruth, "ntruth/I");
  for(Int_t i=0; i<nin; i++)
    t_data->Branch(Form("e%d", i+1), &v_in[i]);
  for(Int_t i=0; i<nout; i++)
    t_data->Branch(Form("n%d", i+1), &v_out[i]);

  auto h2_e = new TH2F("h2_e", "Energy;x;y", 2*nd+1,-nd-0.5,nd+0.5, 2*nd+1,-nd-0.5,nd+0.5);

  auto rnd = new TRandom3(0);

  for(Int_t iev=0; iev<nev; iev++)
  {
    v_e.fill(0.);
    v_in.fill(0.);
    v_out.fill(0.);

    ntruth = rnd->Integer(nout) + 1;
    v_out[ntruth-1] = 1.;

    Double_t truth_x[nout], truth_y[nout], truth_peak[nout];
    for(Int_t ic=0; ic<ntruth; ic++)
    {
      truth_x[ic] = rnd->Gaus(0, 0.8);
      truth_y[ic] = rnd->Gaus(0, 0.8);
      truth_peak[ic] = max(rnd->Gaus(200., 30.), 10.);
    }

    for(Int_t i=-nd; i<=nd; i++)
      for(Int_t j=-nd; j<=nd; j++)
        for(Int_t ic=0; ic<ntruth; ic++)
          v_e[(i+nd)*(2*nd+1)+(j+nd)] += TMath::Abs(
              truth_peak[ic] *
              TMath::Gaus(i - truth_x[ic], 0, 0.6, kTRUE) *
              TMath::Gaus(j - truth_y[ic], 0, 0.6, kTRUE) *
              (1 + rnd->Gaus(0, 0.08)) );

    Double_t sum_in = 0.;
    Int_t npeak = distance(v_e.begin(), max_element(v_e.begin(), v_e.end()));
    Int_t ipeak = npeak / (2*nd+1);
    Int_t jpeak = npeak % (2*nd+1);
    for(Int_t i=-nb; i<=nb; i++)
      if(ipeak+i >= 0 && ipeak+i <= 2*nd)
        for(Int_t j=-nb; j<=nb; j++)
          if(jpeak+j >= 0 && jpeak+j <= 2*nd)
          {
            Double_t et = v_e[(ipeak+i)*(2*nd+1)+(jpeak+j)];
            v_in[(i+nb)*(2*nb+1)+(j+nb)] = et;
            sum_in += et;
          }

    if(sum_in > 0.)
      for(Int_t i=-nb; i<=nb; i++)
        for(Int_t j=-nb; j<=nb; j++)
          v_in[(i+nb)*(2*nb+1)+(j+nb)] /= sum_in;

    for(Int_t i=-nd; i<=nd; i++)
      for(Int_t j=-nd; j<=nd; j++)
        h2_e->Fill(i, j, v_e[(i+nd)*(2*nd+1)+(j+nd)]);

    t_data->Fill();
  } // iev

  auto c0 = new TCanvas("c0", "c0", 600, 600);
  gStyle->SetOptStat(0);
  h2_e->Draw("COLZ");

  string str_node = "e1";
  for(Int_t i=1; i<nin; i++)
    str_node += Form(",e%d", i+1);
  str_node += Form(":%d:n1,n2!", nin+5);

  auto mlp = new TMultiLayerPerceptron(
      str_node.c_str(),
      t_data, "Entry$%5!=0", "Entry$%5==0");

  // Use kStochastic, kBatch doesn't work
  mlp->SetLearningMethod(TMultiLayerPerceptron::kStochastic);
  mlp->Train(10, "text, graph, update=1");

  Long64_t correct = 0;
  for(Long64_t ien=0; ien<t_data->GetEntries(); ien++)
  {
    t_data->GetEntry(ien);
    Double_t n1 = mlp->Evaluate(0, v_in.begin());
    Double_t n2 = mlp->Evaluate(1, v_in.begin());
    Int_t npred = n1 > n2 ? 1 : 2;
    if(npred == ntruth)
      correct++;
  }
  cout << "correct/event = " << correct << "/" << nev << " (" << 100.*correct/nev << "%)" << endl;
}
