void sampling_fraction(){

	gStyle->SetOptStat(0);
	gStyle->SetPadBorderMode(0);
    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameLineWidth(2);

	//Plot 1 -- Neg. Muons at 1,10,50 GeV, etastar = 3.5,3.7
	TCanvas *c1 = new TCanvas("c1");
	c1->Divide(3,2);

	c1->cd(1);
	TFile *f1a = new TFile("../output/sampling/etastar_3p5/insert_sim_mu-_1GeV_theta_deg.edm4hep.root");
	TTree *t1a = (TTree*)f1a->Get("events");

	TH1 *h1a = new TH1D("h1a","#mu^{-} at 1 GeV and #eta* = 3.5",100,0.5,2.5);
    h1a->GetXaxis()->SetTitle("Ratio [%]");h1a->GetXaxis()->CenterTitle();
    h1a->SetLineWidth(3);h1a->SetLineColor(kBlue);

	t1a->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h1a");

	c1->cd(2);
	TFile *f1b = new TFile("../output/sampling/etastar_3p5/insert_sim_mu-_10GeV_theta_deg.edm4hep.root");
    TTree *t1b = (TTree*)f1b->Get("events");

	TH1 *h1b = new TH1D("h1b","#mu^{-} at 10 GeV and #eta* = 3.5",100,0.5,2.5);
    h1b->GetXaxis()->SetTitle("Ratio [%]");h1b->GetXaxis()->CenterTitle();
    h1b->SetLineWidth(3);h1b->SetLineColor(kBlue);

    t1b->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h1b");

	c1->cd(3);
	TFile *f1c = new TFile("../output/sampling/etastar_3p5/insert_sim_mu-_50GeV_theta_deg.edm4hep.root");
    TTree *t1c = (TTree*)f1c->Get("events");

	TH1 *h1c = new TH1D("h1c","#mu^{-} at 50 GeV and #eta* = 3.5",100,0.5,2.5);
    h1c->GetXaxis()->SetTitle("Ratio [%]");h1c->GetXaxis()->CenterTitle();
    h1c->SetLineWidth(3);h1c->SetLineColor(kBlue);

    t1c->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h1c");
	
	c1->cd(4);
	TFile *f1d = new TFile("../output/sampling/etastar_3p7/insert_sim_mu-_1GeV_theta_deg.edm4hep.root");
    TTree *t1d = (TTree*)f1d->Get("events");

	TH1 *h1d = new TH1D("h1d","#mu^{-} at 1 GeV and #eta* = 3.7",100,0.5,2.5);
    h1d->GetXaxis()->SetTitle("Ratio [%]");h1d->GetXaxis()->CenterTitle();
    h1d->SetLineWidth(3);h1d->SetLineColor(kBlue);

    t1d->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h1d");

	c1->cd(5);
	TFile *f1e = new TFile("../output/sampling/etastar_3p7/insert_sim_mu-_10GeV_theta_deg.edm4hep.root");
    TTree *t1e = (TTree*)f1e->Get("events");

	TH1 *h1e = new TH1D("h1e","#mu^{-} at 10 GeV and #eta* = 3.7",100,0.5,2.5);
    h1e->GetXaxis()->SetTitle("Ratio [%]");h1e->GetXaxis()->CenterTitle();
    h1e->SetLineWidth(3);h1e->SetLineColor(kBlue);

    t1e->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h1e");

	c1->cd(6);
	TFile *f1f = new TFile("../output/sampling/etastar_3p7/insert_sim_mu-_50GeV_theta_deg.edm4hep.root");
    TTree *t1f = (TTree*)f1f->Get("events");

	TH1 *h1f = new TH1D("h1f","#mu^{-} at 50 GeV and #eta* = 3.7",100,0.5,2.5);
    h1f->GetXaxis()->SetTitle("Ratio [%]");h1f->GetXaxis()->CenterTitle();
    h1f->SetLineWidth(3);h1f->SetLineColor(kBlue);

    t1f->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h1f");

	//Plot 2 -- Neg. Pions at 1,10,50 GeV, etastar = 3.5,3.7
	TCanvas *c2 = new TCanvas("c2");
	c2->Divide(3,2);

	c2->cd(1);
	TFile *f2a = new TFile("../output/sampling/etastar_3p5/insert_sim_pi-_1GeV_theta_deg.edm4hep.root");
	TTree *t2a = (TTree*)f2a->Get("events");

	TH1 *h2a = new TH1D("h2a","#pi^{-} at 1 GeV and #eta* = 3.5",100,0.5,2.5);
    h2a->GetXaxis()->SetTitle("Ratio [%]");h2a->GetXaxis()->CenterTitle();
    h2a->SetLineWidth(3);h2a->SetLineColor(kBlue);

	t2a->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h2a");

	c2->cd(2);
	TFile *f2b = new TFile("../output/sampling/etastar_3p5/insert_sim_pi-_10GeV_theta_deg.edm4hep.root");
	TTree *t2b = (TTree*)f2b->Get("events");

	TH1 *h2b = new TH1D("h2b","#pi^{-} at 10 GeV and #eta* = 3.5",100,0.5,2.5);
    h2b->GetXaxis()->SetTitle("Ratio [%]");h2b->GetXaxis()->CenterTitle();
    h2b->SetLineWidth(3);h2b->SetLineColor(kBlue);

	t2b->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h2b");

	c2->cd(3);
	TFile *f2c = new TFile("../output/sampling/etastar_3p5/insert_sim_pi-_50GeV_theta_deg.edm4hep.root");
	TTree *t2c = (TTree*)f2c->Get("events");

	TH1 *h2c = new TH1D("h2c","#pi^{-} at 50 GeV and #eta* = 3.5",100,0.5,2.5);
    h2c->GetXaxis()->SetTitle("Ratio [%]");h2c->GetXaxis()->CenterTitle();
    h2c->SetLineWidth(3);h2c->SetLineColor(kBlue);

	t2c->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h2c");

	c2->cd(4);
	TFile *f2d = new TFile("../output/sampling/etastar_3p7/insert_sim_pi-_1GeV_theta_deg.edm4hep.root");
	TTree *t2d = (TTree*)f2d->Get("events");

	TH1 *h2d = new TH1D("h2d","#pi^{-} at 1 GeV and #eta* = 3.7",100,0.5,2.5);
    h2d->GetXaxis()->SetTitle("Ratio [%]");h2d->GetXaxis()->CenterTitle();
    h2d->SetLineWidth(3);h2d->SetLineColor(kBlue);

	t2d->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h2d");

	c2->cd(5);
	TFile *f2e = new TFile("../output/sampling/etastar_3p7/insert_sim_pi-_10GeV_theta_deg.edm4hep.root");
	TTree *t2e = (TTree*)f2e->Get("events");

	TH1 *h2e = new TH1D("h2e","#pi^{-} at 10 GeV and #eta* = 3.7",100,0.5,2.5);
    h2e->GetXaxis()->SetTitle("Ratio [%]");h2e->GetXaxis()->CenterTitle();
    h2e->SetLineWidth(3);h2e->SetLineColor(kBlue);

	t2e->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h2e");

	c2->cd(6);
	TFile *f2f = new TFile("../output/sampling/etastar_3p7/insert_sim_pi-_50GeV_theta_deg.edm4hep.root");
	TTree *t2f = (TTree*)f2f->Get("events");

	TH1 *h2f = new TH1D("h2f","#pi^{-} at 50 GeV and #eta* = 3.7",100,0.5,2.5);
    h2f->GetXaxis()->SetTitle("Ratio [%]");h2f->GetXaxis()->CenterTitle();
    h2f->SetLineWidth(3);h2f->SetLineColor(kBlue);

	t2f->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h2f");

	//Plot 3 -- Electron at 1,10,50 GeV, etastar = 3.5,3.7
	TCanvas *c3 = new TCanvas("c3");
	c3->Divide(3,2);

	c3->cd(1);
	TFile *f3a = new TFile("../output/sampling/etastar_3p5/insert_sim_e-_1GeV_theta_deg.edm4hep.root");
	TTree *t3a = (TTree*)f3a->Get("events");

	TH1 *h3a = new TH1D("h3a","e^{-} at 1 GeV and #eta* = 3.5",100,0.5,2.5);
    h3a->GetXaxis()->SetTitle("Ratio [%]");h3a->GetXaxis()->CenterTitle();
    h3a->SetLineWidth(3);h3a->SetLineColor(kBlue);

	t3a->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h3a");

	c3->cd(2);
	TFile *f3b = new TFile("../output/sampling/etastar_3p5/insert_sim_e-_10GeV_theta_deg.edm4hep.root");
	TTree *t3b = (TTree*)f3b->Get("events");

	TH1 *h3b = new TH1D("h3b","e^{-} at 10 GeV and #eta* = 3.5",100,0.5,2.5);
    h3b->GetXaxis()->SetTitle("Ratio [%]");h3b->GetXaxis()->CenterTitle();
    h3b->SetLineWidth(3);h3b->SetLineColor(kBlue);

	t3b->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h3b");

	c3->cd(3);
	TFile *f3c = new TFile("../output/sampling/etastar_3p5/insert_sim_e-_50GeV_theta_deg.edm4hep.root");
	TTree *t3c = (TTree*)f3c->Get("events");

	TH1 *h3c = new TH1D("h3c","e^{-} at 50 GeV and #eta* = 3.5",100,0.5,2.5);
    h3c->GetXaxis()->SetTitle("Ratio [%]");h3c->GetXaxis()->CenterTitle();
    h3c->SetLineWidth(3);h3c->SetLineColor(kBlue);

	t3c->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h3c");

	c3->cd(4);
	TFile *f3d = new TFile("../output/sampling/etastar_3p7/insert_sim_e-_1GeV_theta_deg.edm4hep.root");
	TTree *t3d = (TTree*)f3d->Get("events");

	TH1 *h3d = new TH1D("h3d","e^{-} at 1 GeV and #eta* = 3.7",100,0.5,2.5);
    h3d->GetXaxis()->SetTitle("Ratio [%]");h3d->GetXaxis()->CenterTitle();
    h3d->SetLineWidth(3);h3d->SetLineColor(kBlue);

	t3d->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h3d");

	c3->cd(5);
	TFile *f3e = new TFile("../output/sampling/etastar_3p7/insert_sim_e-_10GeV_theta_deg.edm4hep.root");
	TTree *t3e = (TTree*)f3e->Get("events");

	TH1 *h3e = new TH1D("h3e","e^{-} at 10 GeV and #eta* = 3.7",100,0.5,2.5);
    h3e->GetXaxis()->SetTitle("Ratio [%]");h3e->GetXaxis()->CenterTitle();
    h3e->SetLineWidth(3);h3e->SetLineColor(kBlue);

	t3e->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h3e");

	c3->cd(6);
	TFile *f3f = new TFile("../output/sampling/etastar_3p7/insert_sim_e-_50GeV_theta_deg.edm4hep.root");
	TTree *t3f = (TTree*)f3f->Get("events");

	TH1 *h3f = new TH1D("h3f","e^{-} at 50 GeV and #eta* = 3.7",100,0.5,2.5);
    h3f->GetXaxis()->SetTitle("Ratio [%]");h3f->GetXaxis()->CenterTitle();
    h3f->SetLineWidth(3);h3f->SetLineColor(kBlue);

	t3f->Draw("100.*Sum$(HcalEndcapPInsertScintillatorHits.energy)/(Sum$(HcalEndcapPInsertAbsorberHits.energy)+Sum$(HcalEndcapPInsertScintillatorHits.energy))>>h3f");

	//Print to file
	c1->Print("plots/sampling_fraction.pdf[");
	c1->Print("plots/sampling_fraction.pdf");
	c2->Print("plots/sampling_fraction.pdf");
	c3->Print("plots/sampling_fraction.pdf");
	c3->Print("plots/sampling_fraction.pdf]");

} 
