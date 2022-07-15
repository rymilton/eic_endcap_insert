void DrawRes()
{
  auto f = new TFile("results/resolution.root");
  auto g_mean_e = (TGraphErrors*)f->Get("g_mean_e");
  auto g_mean_pi= (TGraphErrors*)f->Get("g_mean_pi");
  auto g_res_e  = (TGraphErrors*)f->Get("g_res_e");
  auto g_res_pi = (TGraphErrors*)f->Get("g_res_pi");

  mc(0, 2,2);

  mcd(0, 1);
  g_mean_e->SetTitle("Electron Energy");
  aset(g_mean_e, "Energy (GeV)","Mean Energy (MeV)");
  style(g_mean_e, 20, 1);
  g_mean_e->Draw("AP");

  mcd(0, 2);
  g_mean_pi->SetTitle("Pion Energy");
  aset(g_mean_pi, "Energy (GeV)","Mean Energy (MeV)");
  style(g_mean_pi, 20, 1);
  g_mean_pi->Draw("AP");

  mcd(0, 3);
  g_res_e->SetTitle("Electron Energy Resolution");
  aset(g_res_e, "Energy (GeV)","#sigma/E");
  style(g_res_e, 20, 1);
  g_res_e->Draw("AP");

  mcd(0, 4);
  g_res_pi->SetTitle("Pion Energy Resolution");
  aset(g_res_pi, "Energy (GeV)","#sigma/E");
  style(g_res_pi, 20, 1);
  g_res_pi->Draw("AP");

  gROOT->ProcessLine("c0->Print(\"results/resolution.pdf\");");
}
