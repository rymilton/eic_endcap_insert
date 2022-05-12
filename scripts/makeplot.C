////////////////////////////////////////
// Read reconstruction ROOT output file
// Plot variables
////////////////////////////////////////
int makeplot(const char* input_fname = "sim_output/rec_electron_0GeVto30GeV_100k_output.root")
{
  // Setting figures
  gROOT->SetStyle("Plain");
  gStyle->SetLineWidth(3);
  gStyle->SetOptStat("nem");
  gStyle->SetPadTickX(1);
  gStyle->SetPadTickY(1);
  gStyle->SetPadGridX(1);
  gStyle->SetPadGridY(1);
  gStyle->SetPadLeftMargin(0.14);
  gStyle->SetPadRightMargin(0.14);

  // Input ROOT file
  TFile *f = new TFile(input_fname,"READ");
  TTree *t = (TTree *)f->Get("events");

  // Set Branch status and addressed
  t->SetMakeClass(1);
  t->SetBranchStatus("*", 0);

  Int_t EcalClusters_;
  t->SetBranchStatus("EcalClusters", 1);
  t->SetBranchAddress("EcalClusters", &EcalClusters_);

  Int_t RecoEcalHits_;
  t->SetBranchStatus("RecoEcalHits", 1);
  t->SetBranchAddress("RecoEcalHits", &RecoEcalHits_);

  const Int_t kMaxEcalClusters = 100000;
  Double_t cluster_x_pos[kMaxEcalClusters];
  Double_t cluster_y_pos[kMaxEcalClusters];
  Double_t cluster_z_pos[kMaxEcalClusters];
  Float_t cluster_energy[kMaxEcalClusters];
  t->SetBranchStatus("EcalClusters.position.x",1);
  t->SetBranchStatus("EcalClusters.position.y",1);
  t->SetBranchStatus("EcalClusters.position.z",1);
  t->SetBranchStatus("EcalClusters.energy",1);
  t->SetBranchAddress("EcalClusters.position.x",cluster_x_pos);
  t->SetBranchAddress("EcalClusters.position.y",cluster_y_pos);
  t->SetBranchAddress("EcalClusters.position.z",cluster_z_pos);
  t->SetBranchAddress("EcalClusters.energy",cluster_energy);

  const Int_t kMaxRecoEcalHits = 100000;
  Double_t rec_x_pos[kMaxRecoEcalHits];
  Double_t rec_y_pos[kMaxRecoEcalHits];
  Double_t rec_energy[kMaxRecoEcalHits];
  t->SetBranchStatus("RecoEcalHits.position.x",1);
  t->SetBranchStatus("RecoEcalHits.position.y",1);
  t->SetBranchStatus("RecoEcalHits.energy",1);
  t->SetBranchAddress("RecoEcalHits.position.x",rec_x_pos);
  t->SetBranchAddress("RecoEcalHits.position.y",rec_y_pos);
  t->SetBranchAddress("RecoEcalHits.energy",rec_energy);

  // Setting for Canvas
  TCanvas *c1  = new TCanvas("c1", "c1",  500, 500);
  TCanvas *c2  = new TCanvas("c2", "c2",  500, 500);
  TCanvas *c3  = new TCanvas("c3", "c3",  500, 500);
  TCanvas *c4  = new TCanvas("c4", "c4",  500, 500);
  TCanvas *c5  = new TCanvas("c5", "c5",  500, 500);
  TCanvas *c6  = new TCanvas("c6", "c6",  500, 500);
  TCanvas *c7  = new TCanvas("c7", "c7",  500, 500);
  TCanvas *c8  = new TCanvas("c8", "c8",  500, 500);
  TCanvas *c9  = new TCanvas("c9", "c9",  500, 500);

  // Declare histograms
  TH1D *h1 = new TH1D("h1","Scattering Angle(#theta)",          100,130.0,180.0);
  TH1D *h2 = new TH1D("h2","Pseudo-rapidity(#eta)",             100,-5.0,0.0);
  TH2D *h3 = new TH2D("h3","Cluster E vs Pseudo-rapidity",      100,-0.5,30.5,100,-5.0,0.0);
  TH1D *h4 = new TH1D("h4","Reconstructed energy per event",    100,-0.5,30.5);
  TH1D *h5 = new TH1D("h5","Number of Clusters per event",      5,-0.5,4.5);
  TH1D *h6 = new TH1D("h6","Scattering Angle(#theta) with CUT", 100,130.0,180.0);
  TH1D *h7 = new TH1D("h7","Pseudo-rapidity(#eta) with CUT",    100,-5.0,0.0);
  TH2D *h8 = new TH2D("h8","Cluster Hit Position",              62,-62.0,62.0,62,-62.0,62.0);
  TH2D *h9 = new TH2D("h9","All Hit Position",                  62,-62.0,62.0,62,-62.0,62.0);

  // Declare ellipse for boundary of crystal calorimeter
  TEllipse *ell1 = new TEllipse(0.0, 0.0, 60.0, 60.0);
  ell1->SetFillStyle(4000);
  TEllipse *ell2 = new TEllipse(0.0, 0.0, 12.0, 12.0);
  ell2->SetFillStyle(4000);

  // Total number of entries
  Int_t nentries = t->GetEntries();

  // Variables are used in calculation
  Double_t r;              // Radius [cm]
  Double_t phi;            // Azimuth [degree]
  Double_t theta;          // Inclination [degree]
  Double_t eta;            // Pseudo-rapidity [unitless]
  Float_t  cluster_e;      // Cluster energy [GeV]
  Float_t  total_cluster_e;// Add up clusters per event [GeV]
  Double_t total_thr_e;    // Thrown energy [GeV]

  // Loop over event by event
  for (int ievent = 0; ievent < nentries; ievent++)
  {
	t->GetEntry(ievent);

	Int_t ncluster   = EcalClusters_;
	Int_t nreconhits = RecoEcalHits_;

	total_cluster_e = 0.0;

	h5->Fill(ncluster, 1.0);

	// Loop over cluster by cluster
	for (int icluster=0; icluster < ncluster; icluster++)
	{
		r = TMath::Sqrt((cluster_x_pos[icluster]*cluster_x_pos[icluster]) + 
				(cluster_y_pos[icluster]*cluster_y_pos[icluster]) + 
				(cluster_z_pos[icluster]*cluster_z_pos[icluster]));
		phi = TMath::ATan(cluster_y_pos[icluster]/cluster_x_pos[icluster]) * TMath::RadToDeg();
		theta = TMath::ACos(cluster_z_pos[icluster] / r) * TMath::RadToDeg();
		eta = -1.0 * TMath::Log(TMath::Tan((theta*TMath::DegToRad())/2.0));	
		cluster_e = cluster_energy[icluster] / 1.e+3;
		total_cluster_e += cluster_e;
	}

	// Select events with one cluster
        if(ncluster == 1)
        {       
		h1->Fill(theta, 1.0);
                h2->Fill(eta, 1.0);
                h3->Fill(cluster_e, eta, 1.0);
                h4->Fill(total_cluster_e, 1.0);
                h8->Fill(cluster_x_pos[0],cluster_y_pos[0], 1.0);

		if(total_cluster_e > 0.5)
		{
			h6->Fill(theta, 1.0);
			h7->Fill(eta, 1.0);
		}
	}

	// Loop over hit by hit
        for(int ireconhit=0; ireconhit < nreconhits; ireconhit++)
		h9->Fill(rec_x_pos[ireconhit],rec_y_pos[ireconhit], 1.0);
  }

  // Drawing and Saving figures
  c1->cd();
  h1->SetLineColor(kBlue);
  h1->SetLineWidth(2);
  h1->GetYaxis()->SetRangeUser(0.0,h1->GetMaximum()+10.0);
  h1->GetXaxis()->SetTitle("#theta [degree]");
  h1->GetYaxis()->SetTitle("events");
  h1->GetYaxis()->SetTitleOffset(1.4);
  h1->DrawClone();
  c1->SaveAs("results/electron_theta_hist_0GeVto30GeV.png");
  c1->SaveAs("results/electron_theta_hist_0GeVto30GeV.pdf");

  c2->cd();
  h2->SetLineColor(kBlue);
  h2->SetLineWidth(2);
  h2->GetYaxis()->SetRangeUser(0.0,h2->GetMaximum()+10.0);
  h2->GetXaxis()->SetTitle("#eta");
  h2->GetYaxis()->SetTitle("events");
  h2->GetYaxis()->SetTitleOffset(1.4);
  h2->DrawClone();
  c2->SaveAs("results/electron_eta_hist_0GeVto30GeV.png");
  c2->SaveAs("results/electron_eta_hist_0GeVto30GeV.pdf");

  c3->cd();
  h3->GetXaxis()->SetTitle("Cluster energy [GeV]");
  h3->GetYaxis()->SetTitle("#eta");
  h3->GetYaxis()->SetTitleOffset(1.4);
  h3->DrawClone("COLZ");
  c3->SaveAs("results/eletron_E_vs_eta_hist_0GeVto30GeV.png");
  c3->SaveAs("results/eletron_E_vs_eta_hist_0GeVto30GeV.pdf");

  c4->cd();
  c4->SetLogy(1);
  h4->SetLineColor(kBlue);
  h4->SetLineWidth(2);
  h4->GetXaxis()->SetTitle("reconstructed energy [GeV]");
  h4->GetYaxis()->SetTitle("events");
  h4->GetYaxis()->SetTitleOffset(1.4);
  h4->DrawClone();
  c4->SaveAs("results/electron_Erec_hist_0GeVto30GeV.png");
  c4->SaveAs("results/electron_Erec_hist_0GeVto30GeV.pdf");

  c5->cd();
  c5->SetLogy(1);
  h5->SetLineColor(kBlue);
  h5->SetLineWidth(2);
  h5->GetXaxis()->SetTitle("Number of Clusters");
  h5->GetYaxis()->SetTitle("events");
  h5->GetYaxis()->SetTitleOffset(1.4);
  h5->DrawClone();
  c5->SaveAs("results/electron_ncluster_hist_0GeVto30GeV.png");
  c5->SaveAs("results/electron_ncluster_hist_0GeVto30GeV.pdf");

  c6->cd();
  h6->SetLineColor(kBlue);
  h6->SetLineWidth(2);
  h6->GetYaxis()->SetRangeUser(0.0,h1->GetMaximum()+10.0);
  h6->GetXaxis()->SetTitle("#theta [degree]");
  h6->GetYaxis()->SetTitle("events");
  h6->GetYaxis()->SetTitleOffset(1.4);
  h6->DrawClone();
  c6->SaveAs("results/electron_theta_hist_CUT_0GeVto30GeV.png");
  c6->SaveAs("results/electron_theta_hist_CUT_0GeVto30GeV.pdf");

  c7->cd();
  h7->SetLineColor(kBlue);
  h7->SetLineWidth(2);
  h7->GetYaxis()->SetRangeUser(0.0,h2->GetMaximum()+10.0);
  h7->GetXaxis()->SetTitle("#eta");
  h7->GetYaxis()->SetTitle("events");
  h7->GetYaxis()->SetTitleOffset(1.4);
  h7->DrawClone();
  c7->SaveAs("results/electron_eta_hist_CUT_0GeVto30GeV.png");
  c7->SaveAs("results/electron_eta_hist_CUT_0GeVto30GeV.pdf");

  c8->cd();
  h8->GetXaxis()->SetTitle("Hit position X [cm]");
  h8->GetYaxis()->SetTitle("Hit position Y [cm]");
  h8->GetYaxis()->SetTitleOffset(1.4);
  h8->DrawClone("COLZ");
  ell1->Draw("same");
  ell2->Draw("same");
  c8->SaveAs("results/electron_hit_pos_cluster_0GeVto30GeV.png");
  c8->SaveAs("results/electron_hit_pos_cluster_0GeVto30GeV.pdf");

  c9->cd();
  h9->GetXaxis()->SetTitle("Hit position X [cm]");
  h9->GetYaxis()->SetTitle("Hit position Y [cm]");
  h9->GetYaxis()->SetTitleOffset(1.4);
  h9->DrawClone("COLZ");
  ell1->Draw("same");
  ell2->Draw("same");
  c9->SaveAs("results/electron_hit_pos_all_0GeVto30GeV.png");
  c9->SaveAs("results/electron_hit_pos_all_0GeVto30GeV.pdf");

  return 0;
}
