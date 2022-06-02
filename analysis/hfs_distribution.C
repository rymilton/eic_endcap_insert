R__LOAD_LIBRARY(libeicsmear);

void hfs_distribution(){

    //Event Class
    erhic::EventPythia *event(NULL);

    //Particle Class
    erhic::ParticleMC *particle(NULL);

    //Load ROOT Files
    TChain *tree = new TChain("EICTree");
    for(int i=0;i<15;i++)
        tree->Add(Form("/eic/data/baraks/pythiaeRHIC/outfiles/yellow_report/18_275/ep_18_275_newtune_%d.root",i));

    int nEntries = tree->GetEntries();
    cout<<"-------------------------------"<<endl;
    cout<<"Total Number of Events = "<<nEntries<<endl<<endl;

    //Access event Branch
    tree->SetBranchAddress("event",&event);

    //Variables
    int status(0);
    int nParticles(0);
    int orig(0);
    int id(0);
    double Q2_event(0),y_event;

    //Set Style
    gStyle->SetOptStat(0);
    gStyle->SetPadBorderMode(0);
    gStyle->SetFrameBorderMode(0);
    gStyle->SetFrameLineWidth(2);
    gStyle->SetLabelSize(0.035,"X");
    gStyle->SetLabelSize(0.035,"Y");
    //gStyle->SetLabelOffset(0.01,"X");
    //gStyle->SetLabelOffset(0.01,"Y");
    gStyle->SetTitleXSize(0.04);
    gStyle->SetTitleXOffset(0.9);
    gStyle->SetTitleYSize(0.04);
    gStyle->SetTitleYOffset(0.9);

    //Define Histograms
    TH1 *h1a = new TH1D("h1a","",200,-10,10); //All events (Q2>0.5)
    h1a->GetXaxis()->SetTitle("#eta_{gen}");h1a->GetXaxis()->CenterTitle();
    h1a->SetLineWidth(3);h1a->SetLineColor(kBlue);
    
    TH1 *h1b = new TH1D("h1b","",200,-10,10); //Q2>10
    h1b->GetXaxis()->SetTitle("#eta_{gen}");h1b->GetXaxis()->CenterTitle();
    h1b->SetLineWidth(3);h1b->SetLineColor(kBlue);

    //y vs. E-pz
    //use constant log binning for y
    double y_min = 1E-4;
    double y_max = 1.1;
	const int nbins = 200;
	double log_bw = (log10(y_max) - log10(y_min))/((double)nbins);
    double log_div;
    double y_bins[nbins+1];
    for(int i=0;i<nbins+1;i++){
		log_div = log10(y_min) + (i*log_bw);
		y_bins[i] = pow(10,log_div);
	}

    //use constant log binning for E-pz
    double epz_min = 1E-4;
    double epz_max = 50;
	log_bw = (log10(epz_max) - log10(epz_min))/((double)nbins);
    double epz_bins[nbins+1];
    for(int i=0;i<nbins+1;i++){
		log_div = log10(epz_min) + (i*log_bw);
		epz_bins[i] = pow(10,log_div);
	}

    //For perfect detector acceptance
    double Ehfs(0),pzhfs(0),pxhfs(0),pyhfs(0);
    TH1* h2  = new TH2D("h2","Perfect Detector Acceptance",nbins,y_bins,nbins,epz_bins);
    h2->GetXaxis()->SetTitle("True y");h2->GetXaxis()->CenterTitle();
    h2->GetYaxis()->SetTitle("Total #Sigma (E-P_{z})_{h} [GeV]");h2->GetYaxis()->CenterTitle();

    //For different Eta cuts.
    const int n_cuts = 6;
    double eta_cut[n_cuts] = {10,5,4,3.5,3,2.5};
    double Ehfsa[n_cuts],pzhfsa[n_cuts],pxhfsa[n_cuts],pyhfsa[n_cuts];
    TH1 *h2a[n_cuts]; //E-pz vs. y_true

    for(int icut=0;icut<n_cuts;icut++){
        h2a[icut]  = new TH2D(Form("h2a[%d]",icut),Form("Detector Acceptance: -4.0 < #eta_{gen} <%.1f",eta_cut[icut]),
                            nbins,y_bins,nbins,epz_bins);
        h2a[icut]->GetXaxis()->SetTitle("True y");h2a[icut]->GetXaxis()->CenterTitle();
        h2a[icut]->GetYaxis()->SetTitle("Accepted #Sigma (E-P_{z})_{h} [GeV]");h2a[icut]->GetYaxis()->CenterTitle();

        Ehfsa[icut]=0;
        pzhfsa[icut]=0;
        pxhfsa[icut]=0;
        pyhfsa[icut]=0;
    }


    //Loop Over Events
    for(int iEvent=0;iEvent<nEntries;iEvent++){
     
        tree->GetEntry(iEvent);
        if(iEvent%10000==0) cout<<"Events Analysed = "<<iEvent<<"!"<<endl;

        //Reset variables
        Ehfs=0;pzhfs=0;pxhfs=0;pyhfs=0;

        for(int icut=0;icut<n_cuts;icut++){
            Ehfsa[icut]=0;
            pzhfsa[icut]=0;
            pxhfsa[icut]=0;
            pyhfsa[icut]=0;
        }

        //Kinematics
        Q2_event = (double) event->GetQ2();
        y_event = (double) event->GetY();

        nParticles = event->GetNTracks();

        // Loop over all particles in event 
        for(int iParticle=0;iParticle<nParticles;iParticle++){

            particle = event->GetTrack(iParticle);
      
            id = (int) particle->Id();  //Particle Id Number
            status = (int) particle->GetStatus();  //Particle status
            orig = (int) particle->GetParentIndex(); //Particle origin (i.e. parent)

            //Skip scattered electron
            if(status==1 && id==11 && orig==3){
                continue;
            }

            //Get all other final-state particles
            if(status==1){
                //Fill Eta distributions
                h1a->Fill(particle->GetEta());
                if(Q2_event>10)h1b->Fill(particle->GetEta());

                Ehfs+=particle->GetE();
                pzhfs+=particle->GetPz();
                pxhfs+=particle->GetPx();
                pyhfs+=particle->GetPy();

                for(int icut=0;icut<n_cuts;icut++){
                    if(particle->GetEta()>-4.0 && particle->GetEta()<eta_cut[icut]){
                        Ehfsa[icut]+=particle->GetE();
                        pzhfsa[icut]+=particle->GetPz();
                        pxhfsa[icut]+=particle->GetPx();
                        pyhfsa[icut]+=particle->GetPy();
                    }
                }
            }
        }//End Particle Loop

        //Fill sum histograms
        h2->Fill(y_event,Ehfs-pzhfs);

        for(int icut=0;icut<n_cuts;icut++)
            h2a[icut]->Fill(y_event,Ehfsa[icut]-pzhfsa[icut]);

    }//End Event Loop

    //Make plots
    //First set of plots
    TCanvas *c1a = new TCanvas("c1a");
    h1a->Draw();

    //Latex
    TLatex *tex1a = new TLatex(-9,3500e3,"Pythia6: 18x275GeV");
    tex1a->SetTextFont(42);
    tex1a->SetTextSize(0.0325);
    tex1a->SetLineWidth(2);
    tex1a->Draw();
    tex1a = new TLatex(-9,3250e3,"Q^{2} > 0.5 GeV^{2}");
    tex1a->SetTextFont(42);
    tex1a->SetTextSize(0.0325);
    tex1a->SetLineWidth(2);
    tex1a->Draw();
    tex1a = new TLatex(-9,2950e3,"Colinear frame");
    tex1a->SetTextFont(42);
    tex1a->SetTextSize(0.0325);
    tex1a->SetLineWidth(2);
    tex1a->Draw();
    tex1a = new TLatex(-9,2500e3,"All final-state particles");
    tex1a->SetTextFont(42);
    tex1a->SetTextSize(0.0325);
    tex1a->SetLineWidth(2);
    tex1a->Draw();
    tex1a = new TLatex(-9,2300e3,"other than scattered electron");
    tex1a->SetTextFont(42);
    tex1a->SetTextSize(0.0325);
    tex1a->SetLineWidth(2);
    tex1a->Draw();

    TCanvas *c1b = new TCanvas("c1b");
    h1b->Draw();

    //Latex
    TLatex *tex1b = new TLatex(-9,0.057*3500e3,"Pythia6: 18x275GeV");
    tex1b->SetTextFont(42);
    tex1b->SetTextSize(0.0325);
    tex1b->SetLineWidth(2);
    tex1b->Draw();
    tex1b = new TLatex(-9,0.057*3250e3,"Q^{2} > 10 GeV^{2}");
    tex1b->SetTextFont(42);
    tex1b->SetTextSize(0.0325);
    tex1b->SetLineWidth(2);
    tex1b->Draw();
    tex1b = new TLatex(-9,0.057*2950e3,"Colinear frame");
    tex1b->SetTextFont(42);
    tex1b->SetTextSize(0.0325);
    tex1b->SetLineWidth(2);
    tex1b->Draw();
    tex1b = new TLatex(-9,0.057*2500e3,"All final-state particles");
    tex1b->SetTextFont(42);
    tex1b->SetTextSize(0.0325);
    tex1b->SetLineWidth(2);
    tex1b->Draw();
    tex1b = new TLatex(-9,0.057*2300e3,"other than scattered electron");
    tex1b->SetTextFont(42);
    tex1b->SetTextSize(0.0325);
    tex1b->SetLineWidth(2);
    tex1b->Draw();

    c1a->Print("hfs_distribution.pdf[");
    c1a->Print("hfs_distribution.pdf");
    c1b->Print("hfs_distribution.pdf");

    //Second set of plots
    TCanvas *c2 = new TCanvas("c2");
    c2->SetLogx();c2->SetLogy();c2->SetLogz();
    h2->Draw("colz");

    //Latex
    TLatex *tex2a = new TLatex(2e-4,10,"Pythia6: 18x275GeV");
    tex2a->SetTextFont(42);tex2a->SetTextSize(0.0325);tex2a->SetLineWidth(2);
    tex2a->Draw();
    TLatex *tex2b = new TLatex(2e-4,5,"Q^{2} > 0.5 GeV^{2}");
    tex2b->SetTextFont(42);tex2b->SetTextSize(0.0325);tex2b->SetLineWidth(2);
    tex2b->Draw();
    TLatex *tex2c = new TLatex(2e-4,3.,"Colinear frame");
    tex2c->SetTextFont(42);tex2c->SetTextSize(0.0325);tex2c->SetLineWidth(2);
    tex2c->Draw();
    TLatex *tex2d = new TLatex(2e-4,1,"Sums all final-state particles");
    tex2d->SetTextFont(42);tex2d->SetTextSize(0.0325);tex2d->SetLineWidth(2);
    tex2d->Draw();
    TLatex *tex2e = new TLatex(2e-4,0.7,"other than scattered electron");
    tex2e->SetTextFont(42);tex2e->SetTextSize(0.0325);tex2e->SetLineWidth(2);
    tex2e->Draw();

    c2->Print("hfs_distribution.pdf");

    TCanvas *c2a[n_cuts];

    for(int icut=0;icut<n_cuts;icut++){
        c2a[icut] = new TCanvas(Form("c2a[%d]",icut));
        c2a[icut]->SetLogx();c2a[icut]->SetLogy();c2a[icut]->SetLogz();
        h2a[icut]->Draw("colz");

        tex2a->Draw();tex2b->Draw();tex2c->Draw();tex2d->Draw();tex2e->Draw();

        c2a[icut]->Print("hfs_distribution.pdf");
    }
    c2a[n_cuts-1]->Print("hfs_distribution.pdf]");

} //End of script