#include <iostream>
#include <string>
#include <array>
#include <algorithm>

#include <TMath.h>
#include <TRandom3.h>
#include <TTree.h>
#include <TH1.h>
#include <TH2.h>
#include <TGraph.h>
#include <TMultiLayerPerceptron.h>
#include <TCanvas.h>
#include <TStyle.h>
#include <TLegend.h>

using namespace std;

template<class T> inline constexpr T square(const T &x) { return x*x; }
template<class T> inline constexpr T point_dis(const T &x1, const T &y1, const T &x2, const T &y2) { return sqrt(square(x1-x2)+square(y1-y2)); }

void ToyPi0gg(const Int_t nev = 10000)
{
  const Int_t nd = 5;
  const Int_t nb = 1;
  const Int_t nin = (2*nb+1)*(2*nb+1);
  Int_t ntruth;
  array<Float_t, (2*nd+1)*(2*nd+1)> v_e;
  array<Float_t, nin> v_in;
  Float_t pout;

  auto t_data = new TTree("T", "Training data");
  t_data->Branch("ntruth", &ntruth, "ntruth/I");
  for(Int_t i=0; i<nin; i++)
    t_data->Branch(Form("e%d", i+1), &v_in[i], Form("e%d/F", i+1));
  t_data->Branch("p", &pout, "p/F");

  auto h2_e = new TH2F("h2_e", "Energy;x;y", 2*nd+1,-nd-0.5,nd+0.5, 2*nd+1,-nd-0.5,nd+0.5);

  auto rnd = new TRandom3(0);

  for(Int_t iev=0; iev<nev; iev++)
  {
    v_e.fill(0.);
    v_in.fill(0.);

    ntruth = rnd->Integer(2) + 1;
    pout = ntruth - 1;

    Float_t truth_x[2], truth_y[2], truth_peak[2];
    for(Int_t ig=0; ig<ntruth; ig++)
    {
      truth_x[ig] = rnd->Gaus(0, 0.4);
      truth_y[ig] = rnd->Gaus(0, 0.4);
      truth_peak[ig] = max(rnd->Gaus(200., 30.), 10.);
    }

    if(ntruth == 2 && point_dis(truth_x[0], truth_y[0], truth_x[1], truth_y[1]) > 1.5)
      continue;

    for(Int_t i=-nd; i<=nd; i++)
      for(Int_t j=-nd; j<=nd; j++)
        for(Int_t ig=0; ig<ntruth; ig++)
          v_e[(i+nd)*(2*nd+1)+(j+nd)] += TMath::Abs(
              truth_peak[ig] *
              TMath::Gaus(i - truth_x[ig], 0, 0.5, kTRUE) *
              TMath::Gaus(j - truth_y[ig], 0, 0.5, kTRUE) *
              (1 + rnd->Gaus(0, 0.08)) );

    for(Int_t i=-nd; i<=nd; i++)
      for(Int_t j=-nd; j<=nd; j++)
        h2_e->Fill(i, j, v_e[(i+nd)*(2*nd+1)+(j+nd)]);

    Float_t sum_in = 0.;
    Int_t kp = distance(v_e.begin(), max_element(v_e.begin(), v_e.end()));
    Int_t ip = kp / (2*nd+1) - nd;
    Int_t jp = kp % (2*nd+1) - nd;
    for(Int_t i=-nb; i<=nb; i++)
      if(abs(ip+i) <= nd)
        for(Int_t j=-nb; j<=nb; j++)
          if(abs(jp+j) <= nd)
          {
            Float_t et = v_e[(ip+i+nd)*(2*nd+1)+(jp+j+nd)];
            v_in[(i+nb)*(2*nb+1)+(j+nb)] = et;
            sum_in += et;
          }

    if(sum_in > 0.)
      for(Int_t i=-nb; i<=nb; i++)
        for(Int_t j=-nb; j<=nb; j++)
          v_in[(i+nb)*(2*nb+1)+(j+nb)] /= sum_in;

    t_data->Fill();
  } // iev

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

  // Use kStochastic, kBatch doesn't work
  mlp->SetLearningMethod(TMultiLayerPerceptron::kStochastic);
  mlp->Train(10, "text, graph, update=1");

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
    Float_t sb = n_eff[1] / sqrt(n_eff[0] + n_eff[1]) / sqrt(n_total[1]);
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
  c1->Print("results/ToyPi0gg.pdf");
}
