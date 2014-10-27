//&>/dev/null;x="${0%.*}";[ ! "$x" -ot "$0" ]||(rm -f "$x";g++ -o "$x" "$0" -I`root-config --incdir` `root-config --libs`);exit

// Build: g++ lhe_reader_non_decayed.c -o lhe_reader_non_decayed -I`root-config --incdir` `root-config --libs`

#include <cmath>
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <fstream>
#include <TH1F.h>
#include <TFile.h>
#include <TLorentzVector.h>
using namespace std;

// Pour une description du format leshouches
// hep-ph/0609017
//
// pour chaque evenement
// une ligne générale : NbPart idprocess poids scale alpha_em alpha_s
// pour chaque particule : id status mere1 mere2 couleur1 couleur2 px py pz E m lifetime spin  


int main(int argc, char **argv) {

  if (argc != 2) {
    cout << "Missing argument. Expected LHE filename without '.lhe'"<< endl;
    exit(-5);
  }


  string basename=argv[1];

  string lhefname = basename+".lhe";
  string rootfname = basename+".root";

  string tt;
  int event=0;
  int npart,idprocess;
  double weight,scale,alpha_em,alpha_s;

  TH1::SetDefaultSumw2(true);


// --- insert here histogram definition
  TH1F* hMtt = new TH1F("mtt","tt invariant mass",500,200,1000) ;
  TH1F* hPt_ttbar = new TH1F("pt_ttbar","Pt ttbar",500,0,2000) ;
  TH1F* hBoost_ttbar = new TH1F("boost_ttbar","Boost ttbar",500,0,1);
  TH1F* hPt_top = new TH1F("top_pt","top pt",500,0,2000) ;
  TH1F* hEta_top = new TH1F("top_eta","top eta",100,-5,5) ;
  TH1F* hPhi_top = new TH1F("top_phi","top phi",500,-3.15,3.15) ;
  TH1F* hPt_tbar = new TH1F("tbar_pt","tbar pt",500,0,2000) ;
  TH1F* hEta_tbar = new TH1F("tbar_eta","tbar eta",100,-5,5) ;
  TH1F* hPhi_tbar = new TH1F("tbar_phi","tbar phi",500,-3.15,3.15) ;
  TH1F* hNextrajets = new TH1F("njets","n extra jets",4,0,4);
  TH1F* hextrajet_flavour = new TH1F("jet_flavour","extra jets flavour",30,-5,26);
  TH1F* hPt_extrajet = new TH1F("extrajet_pt","extra jets pt",500,0,2000) ;
  TH1F* hEta_extrajet = new TH1F("extrajet_eta","extra jets eta",100,-5,5) ;
  TH1F* hPhi_extrajet = new TH1F("extrajet_phi","extra jets phi",500,-3.15,3.15) ;
  TH1F* hPt_Zp = new TH1F("zprime_pt","Z' pt",500,0,2000) ; 
  //TH1F* hEta_Zp =new TH1F("zprime_eta","Z' eta",100,-5,5) ; 
  //TH1F* hPhi_Zp =new TH1F("zprime_phi","Z' phi",500,-3.15,3.15) ; 
  TH1F* hPt_lmax = new TH1F("lmax_pt","lmax pt",500,0,2000) ;
  TH1F* hEta_lmax = new TH1F("lmax_eta","lmax eta",100,-5,5) ;
  TH1F* hPhi_lmax = new TH1F("lmax_phi","lmax phi",500,-3.15,3.15) ;
  TH1F* hPt_lmin = new TH1F("lmin_pt","lmin pt",500,0,2000) ;
  TH1F* hEta_lmin = new TH1F("lmin_eta","lmin eta",100,-5,5) ;
  TH1F* hPhi_lmin = new TH1F("lmin_phi","lmin phi",500,-3.15,3.15) ;
  TH1F* hPt_met = new TH1F("met_pt","met pt",500,0,2000) ;
  TH1F* hEta_met = new TH1F("met_eta","met eta",100,-5,5) ;
  TH1F* hPhi_met = new TH1F("met_phi","met phi",500,-3.15,3.15) ;
  TH1F* hPt_bmax = new TH1F("bmax_pt","bmax pt",500,0,2000) ;
  TH1F* hEta_bmax = new TH1F("bmax_eta","bmax eta",100,-5,5) ;
  TH1F* hPhi_bmax = new TH1F("bmax_phi","bmax phi",500,-3.15,3.15) ;
  TH1F* hPt_bmin = new TH1F("bmin_pt","bmin pt",500,0,2000) ;
  TH1F* hEta_bmin = new TH1F("bmin_eta","bmin eta",100,-5,5) ;
  TH1F* hPhi_bmin = new TH1F("bmin_phi","bmin phi",500,-3.15,3.15) ;
  TH1F** hPt_q = new TH1F*[4];
  TH1F** hEta_q = new TH1F*[4];
  TH1F** hPhi_q = new TH1F*[4];
  for (int iquark=0 ; iquark<4 ; iquark++) {
    ostringstream oss;
    oss << "q" << iquark ;
    string ptstr = oss.str()+"_pt"; 
    hPt_q[iquark] = new TH1F(ptstr.c_str(),ptstr.c_str(),500,0,2000) ;
    string etastr=oss.str()+"_eta";
    hEta_q[iquark] = new TH1F(etastr.c_str(),etastr.c_str(),100,-5,5) ;
    string phistr=oss.str()+"_phi";
    hPhi_q[iquark] = new TH1F(phistr.c_str(),phistr.c_str(),500,-3.15,3.15) ;
  }
  TH1F* hCoM_eta_top = new TH1F("top_eta_CoM","top_eta_CoM",100, -5,5);
  TH1F* hCoM_pt_top = new TH1F("top_pt_CoM","top_pt_CoM",500, 0, 2000);
  TH1F* hCoM_theta_top = new TH1F("top_theta_CoM","top_theta_CoM",100, -M_PI,M_PI);
// --- end histogram definition
 
  int nlept=0, nsemi=0, nhadr=0;


  ifstream ff(lhefname.c_str(),ios::in); //ouverture du fichier .lhe
  //ifstream ff("test.lhe",ios::in); //ouverture du fichier .lhe
  //ifstream ff("/home/cms/perries/madgraph-newphysics/s1/madevent/Events/zp4000_unweighted_events.lhe",ios::in); //ouverture du fichier .lhe
  //ifstream ff("/home/cms/perries/madgraph-newphysics/QCD/madevent/Events/qcd_unweighted_events.lhe",ios::in); //ouverture du fichier .lhe
  int negativeWeight = 0;
  long long line = 0;
  while(!ff.eof()) {
    std::stringstream buffer;
    ff>>tt;
    buffer << tt << std::endl;
    line++;
    if(tt=="<event>") {
      ff>>npart>>idprocess>>weight>>scale>>alpha_em>>alpha_s; //event definition
      buffer << npart << " " << idprocess << " " << weight << " " << scale << " " << alpha_em << " " << alpha_s << std::endl;
      line++;
      event++;
      if (weight < 0) {
        negativeWeight++;
        weight = -1;
      } else {
        weight = 1;
      }

      /*weight = 1.;*/

      if (event%10000==0) cout << "reading event "<< event << endl;
      int lmin=-1, lmax=-1, bmin=-1, met1=-1, met2=-1, bmax=-1;
      int q[4]={-1,-1,-1,-1};
      int top=-1,topbar=-1,zprime=-1;
      int *Id      = new int[npart+1]; 
      int *Status  = new int[npart+1];
      int *Mother1 = new int[npart+1]; 
      int *Mother2 = new int[npart+1]; 
      int *Color1  = new int[npart+1];
      int *Color2  = new int[npart+1];
      double *px = new double[npart+1];
      double *py = new double[npart+1];
      double *pz = new double[npart+1];
      double *E = new double[npart+1];
      double *m = new double[npart+1];
      double *lifetime = new double[npart+1];
      double *spin = new double[npart+1];
      TLorentzVector **v = new TLorentzVector*[npart+1];
      // in lhe first line is number 1, so fill unused array [0] with a crazy value;
      Id[0]= -99999; 
      Status[0]= -99999;
      Mother1[0]= -99999; 
      Mother2[0]= -99999; 
      Color1[0]= -99999;
      Color2[0]= -99999;
      px[0]= -99999;
      py[0]= -99999;
      pz[0]= -99999;
      E[0]= -99999;
      m[0]= -99999;
      lifetime[0]= -99999;
      spin[0]= -99999;
      for (int i=1 ; i<npart+1 ; i++) { //start at one
        ff >> Id[i] >> Status[i] >> Mother1[i] >> Mother2[i] >> Color1[i] >> Color2[i]  
           >> px[i] >> py[i] >> pz[i] >> E[i] >> m[i] >> lifetime[i] >> spin[i] ;
        buffer << Id[i] << " " << Status[i] << " " << std::endl;
        line++;
        v[i] = new TLorentzVector(px[i], py[i], pz[i], E[i]);
        if (Status[i]==-1) continue; // status -1 = initial quark ==> skip
        if (Id[i]==6)  top=i;
        if (Id[i]==-6) topbar=i;
        if (Id[i]>6000000) zprime=i;
        //cout << Id[i] << "  " << Mother1[i] << "  " << Id[Mother1[i]] << endl;
        if (abs(Id[Mother1[i]])==24 || abs(Id[Mother1[i]])==6) { // mother = W
          int id = abs(Id[i]);
          if ( id==11 || id==13 || id==15 ) { // charged leptons
            if (lmax == -1) lmax=i;
            else if (v[i]->Pt() > v[lmax]->Pt()) {
              lmin=lmax; lmax=i;
            } 
            else 
              lmin=i;
          }
          if ( id==12 || id==14 || id==16 ) { // neutrinos = MET 
            if (met1 == -1) met1=i;
            else if (met2==-1) met2=i;
            else cout << "ERROR : more than 2 neutrinos" << endl;
          } 
          if (id<5) { // light quarks
            //cout << q[0] << " " << q[1] << " " << q[2] << " " << q[3] << " " << endl;
            if (q[0] == -1) q[0]=i;
            else if (q[1]==-1) q[1]=i;
            else if (q[2]==-1) q[2]=i;
            else if (q[3]==-1) q[3]=i;
            else cout << "ERROR : more than 4 light quarks" << endl;
          }
        }
      //  else { // mother is not a W
          if ( Status[i]==1 && Mother1[i]==1 && Mother2[i]==2 && (abs(Id[i])<6 || Id[i]==21) ) {
            // extra jet
            hextrajet_flavour->Fill(Id[i]+0.5); // in order to center the bin (a quark is in the next bin)
            hPt_extrajet->Fill(v[i]->Pt());
            hEta_extrajet->Fill(v[i]->Eta());
            hPhi_extrajet->Fill(v[i]->Phi());
          }
          else if (abs(Id[i])==5) { // bjets
            if (bmax == -1) bmax=i;
            else if (v[i]->Pt() > v[bmax]->Pt()) {
              bmin=bmax; bmax=i;
            } 
            else 
              bmin=i;
          }
      //  }
      }
      // sort the quarks
      int nquarks=0;
      if (q[1] != -1) nquarks = 2;
      if (q[3] != -1) nquarks = 4;
      for (int j=0; j<nquarks; j++) {
        double ptmax=-1.; int imax=-1;
        for (int k=j; k<nquarks; k++) {
          if (v[q[k]]->Pt()>ptmax) { 
            ptmax = v[q[k]]->Pt();
            imax = k;
          }
        }
        int tmp=q[j];
        q[j]=q[imax];
        q[imax]=tmp;
      }       
      
      if (top == -1 || topbar == -1) {
        std::cout << "Warning: no tt~ in this event (line " << line << ")" << std::endl;
        std::cout << buffer.str() << std::endl;
        continue;
      }

// --- insert here the code to fill the histograms
      // top kinematics
      hPt_top->Fill( v[top]->Pt(), weight);
      hEta_top->Fill( v[top]->Eta(), weight);
      hPhi_top->Fill( v[top]->Phi(), weight);
      // tbar kinematics
      hPt_tbar->Fill( v[topbar]->Pt(), weight);
      hEta_tbar->Fill( v[topbar]->Eta(), weight);
      hPhi_tbar->Fill( v[topbar]->Phi(), weight );
      // Z' kinematics
      if (zprime != -1) 
        hPt_Zp->Fill( v[zprime]->Pt(), weight );
      //hEta_Zp->Fill( v[zprime]->Eta() );
      //hPhi_Zp->Fill( v[zprime]->Phi() );
      // mtt
      double mtt = sqrt( m[top]*m[top] + m[topbar]*m[topbar] 
              + 2*(E[top]*E[topbar] - px[top]*px[topbar] - py[top]*py[topbar] - pz[top]*pz[topbar]) );

      /*if (mtt > 500)*/
        /*weight = -1;*/
      /*else*/
        /*weight = 1;*/

      hMtt->Fill(mtt, weight);
      // boost ttbar system
      hBoost_ttbar->Fill((*(v[top])+*(v[topbar])).Beta(), weight);

      // go back to CoM
      TLorentzVector ttbar_CoM =  *v[top] + *v[topbar];
      //cout << ttbar_CoM.E()<< endl;
      TVector3 b(0,0,-1*ttbar_CoM.Pz()/ttbar_CoM.E());
      ttbar_CoM.Boost(b);
      //cout << ttbar_CoM.Pz() << endl;
      TLorentzVector vtop_com = *v[top];
      vtop_com.Boost(b);
      hCoM_eta_top->Fill(vtop_com.Eta(), weight);
      hCoM_pt_top->Fill(vtop_com.Pt(), weight);
      hCoM_theta_top->Fill(vtop_com.Theta(), weight);


      // ttbar system
      hPt_ttbar->Fill((*v[top]+*v[topbar]).Pt(), weight);
      // n extra jets
      /*hNextrajets->Fill( (npart - 13)+0.5 ) ; //for decayed ttbar*/
      //hNextrajets->Fill( (npart - 5)+0.5 )  ; // for undecayed ttbar

      // lmax kinematics
      if (lmax != -1) {
        hPt_lmax->Fill( v[lmax]->Pt(), weight );
        hEta_lmax->Fill( v[lmax]->Eta(), weight );
        hPhi_lmax->Fill( v[lmax]->Phi(), weight );
      }     
      // lmin kinematics
      if (lmin != -1) {
        hPt_lmin->Fill( v[lmin]->Pt() );
        hEta_lmin->Fill( v[lmin]->Eta() );
        hPhi_lmin->Fill( v[lmin]->Phi() );
      }
      // met kinematics
      if (met1 !=-1) {
        if (met2 !=-1 ) {
          hPt_met->Fill( (*v[met1]+*v[met2]).Pt() );
          hEta_met->Fill( (*v[met1]+*v[met2]).Eta() );
          hPhi_met->Fill( (*v[met1]+*v[met2]).Phi() );
        }
        else {
          hPt_met->Fill( v[met1]->Pt() );
          hEta_met->Fill( v[met1]->Eta() );
          hPhi_met->Fill( v[met1]->Phi() );
        }
      }
      // bmax kinematics
      if (bmax != -1) {
        hPt_bmax->Fill( v[bmax]->Pt() );
        hEta_bmax->Fill( v[bmax]->Eta() );
        hPhi_bmax->Fill( v[bmax]->Phi() );
      }     
      // bmin kinematics
      if (bmin != -1) {
        hPt_bmin->Fill( v[bmin]->Pt() );
        hEta_bmin->Fill( v[bmin]->Eta() );
        hPhi_bmin->Fill( v[bmin]->Phi() );
      }
      // light quarks kinematics
      for (int nq=0; nq<nquarks ; nq++) {
        hPt_q[nq]->Fill( v[q[nq]]->Pt() );
        hEta_q[nq]->Fill( v[q[nq]]->Eta() );
        hPhi_q[nq]->Fill( v[q[nq]]->Phi() );
      }
        
      if (lmin !=-1) nlept++;
      else if (nquarks==4) nhadr++;
      else nsemi++;
// --- end filling the histograms
      ff>>tt;
      line++;
      //if (event==100)  break;
      delete Id;
      delete Status;
      delete Mother1;
      delete Mother2;
      delete Color1;
      delete Color2;
      delete px;
      delete py;
      delete pz;
      delete E;
      delete m;
      delete lifetime;
      delete spin;
      for (int k=1 ; k<npart+1 ; delete v[k++]);
    }
  } // end while !eof
  cout << " Total number of events --> " << event << endl;
  TFile *rootfile = new TFile(rootfname.c_str(),"recreate");
  hMtt->Write();
  hPt_ttbar->Write();
  hPt_top->Write();
  hEta_top->Write();
  hPhi_top->Write();
  hPt_tbar->Write();
  hEta_tbar->Write();
  hPhi_tbar->Write();
  hPt_Zp->Write();
  //hEta_Zp->Write();
  //hPhi_Zp->Write();
  hNextrajets->Write();
  hPt_lmax->Write();
  hEta_lmax->Write();
  hPhi_lmax->Write();
  hPt_lmin->Write();
  hEta_lmin->Write();
  hPhi_lmin->Write();
  hPt_bmax->Write();
  hEta_bmax->Write();
  hPhi_bmax->Write();
  hPt_bmin->Write();
  hEta_bmin->Write();
  hPhi_bmin->Write();
  hPt_met->Write();
  hEta_met->Write();
  hPhi_met->Write();
  for (int i=0;i<4;i++) { 
    hPt_q[i]->Write();
    hEta_q[i]->Write();
    hPhi_q[i]->Write();
  }
  hextrajet_flavour->Write();
  hPt_extrajet->Write();
  hEta_extrajet->Write();
  hPhi_extrajet->Write();
  hBoost_ttbar->Write();
  hCoM_eta_top->Write();
  hCoM_theta_top->Write();
  hCoM_pt_top->Write();
  rootfile->Close();

  cout << "Events with negative weight: " << negativeWeight << endl;
  cout << "lept decay = " << nlept*1.0/event << endl;
  cout << "hadr decay = " << nhadr*1.0/event << endl;
  cout << "semi decay = " << nsemi*1.0/event << endl;
  exit(0);
}
