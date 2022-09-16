void DrawPi0ggMerge()
{
  auto f = new TFile("/phenix/spin/phnxsp01/zji/data/eic/histos/merging-15_15deg-seppt.root");
  auto h_total = (TH1*)f->Get("h_total");
  auto h_merged = (TH1*)f->Get("h_merged");
  auto g = new TGraphAsymmErrors(h_merged, h_total);
  for(Int_t i=0; i<g->GetN(); i++)
  {
    g->SetPointEXhigh(i, 0.);
    g->SetPointEXlow(i, 0.);
  }

  auto c0 = new TCanvas("c0","c0", 600,600);
  mcd();
  g->SetTitle("Merging prob");
  aset(g, "p_{T} (GeV)","prob");
  style(g, 20, 1);
  g->Draw("APL");
  c0->Print("results/merge-seppt.pdf");
}
