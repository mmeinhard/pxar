#include <iostream>
#include <cstdlib> 
#include <bitset>

#include <TApplication.h>
#include <TGButton.h>
#include <TGToolTip.h>
#include <TRandom.h>
#include <TSystem.h>
#include <TCanvas.h>
#include <TGTab.h>
#include <TGLabel.h>
#include <TGButtonGroup.h>
#include <cstdlib>

#include "PixParTab.hh"
#include "log.h"

using namespace std;
using namespace pxar;

ClassImp(PixParTab)

// ----------------------------------------------------------------------
PixParTab::PixParTab(PixGui *p, ConfigParameters *cfg, string tabname) {
  init(p, cfg, tabname); 

  fTabFrame = fGui->getTabs()->AddTab(fTabName.c_str());
  fTabFrame->SetLayoutManager(new TGVerticalLayout(fTabFrame));


  UInt_t w = fGui->getTabs()->GetWidth(); 
  UInt_t h = fGui->getTabs()->GetHeight(); 

  //   fhFrame = new TGHorizontalFrame(fTabFrame);
  fhFrame = new TGCompositeFrame(fTabFrame, w, h, kHorizontalFrame);

  fTabFrame->AddFrame(fhFrame, new TGLayoutHints(kLHintsRight | kLHintsExpandX | kLHintsExpandY));

  Pixel_t colDarkSeaGreen;    
  gClient->GetColorByName("DarkSeaGreen", colDarkSeaGreen);
  TGTabElement *tabel = fGui->getTabs()->GetTabTab(fTabName.c_str());
  tabel->ChangeBackground(colDarkSeaGreen);

  TGTextEntry *te(0); 
  TGLabel *tl(0); 
  TGTextBuffer *tb(0); 
  TGTextButton *tset(0); 

  TGCheckButton *tcb(0); 
  TGHorizontalFrame *hFrame(0); 
  TGVerticalFrame *vFrame(0); 
  TGGroupFrame *g1Frame(0), *g2Frame(0); 

  // -- TB Parameters
  vFrame = new TGVerticalFrame(fhFrame);
  fhFrame->AddFrame(vFrame, new TGLayoutHints(kLHintsLeft, 15, 15, 15, 15));
  g1Frame = new TGGroupFrame(vFrame, "Testboard");
  vector<pair<string, uint8_t> > amap = fConfigParameters->getTbParameters();
  for (unsigned int i = 0; i < amap.size(); ++i) {
    hFrame = new TGHorizontalFrame(g1Frame, 300, 30, kLHintsExpandX); 
    g1Frame->AddFrame(hFrame, new TGLayoutHints(kLHintsRight | kLHintsTop));
    LOG(logDEBUG) << "Creating TGTextEntry for " << amap[i].first; 
    tb = new TGTextBuffer(5); 
    tl = new TGLabel(hFrame, amap[i].first.c_str());
    tl->SetWidth(100);
    hFrame->AddFrame(tl, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 

    te  = new TGTextEntry(hFrame, tb, i); te->SetWidth(100); 
    hFrame->AddFrame(te, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2)); 
    fTbParIds.push_back(amap[i].first); 
    fTbTextEntries.insert(make_pair(amap[i].first, te)); 

    te->SetText(Form("%d", int(amap[i].second)));
    te->Connect("ReturnPressed()", "PixParTab", this, "setTbParameter()");

    tset = new TGTextButton(hFrame, "Set", i);
    tset->SetToolTipText("set the parameter\nor click *return* after changing the numerical value");
    tset->GetToolTip()->SetDelay(2000); // add a bit of delay to ease button hitting
    tset->Connect("Clicked()", "PixParTab", this, "setTbParameter()");
    hFrame->AddFrame(tset, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
  }

  vector<pair<string, double> > dmap = fConfigParameters->getTbPowerSettings();
  for (unsigned int i = 0; i < dmap.size(); ++i) {
    hFrame = new TGHorizontalFrame(g1Frame, 300, 30, kLHintsExpandX); 
    g1Frame->AddFrame(hFrame, new TGLayoutHints(kLHintsRight | kLHintsTop));
    LOG(logDEBUG) << "Creating TGTextEntry for " << dmap[i].first; 
    tb = new TGTextBuffer(5); 
    tl = new TGLabel(hFrame, dmap[i].first.c_str());
    tl->SetWidth(100);
    hFrame->AddFrame(tl, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 

    te  = new TGTextEntry(hFrame, tb, i); te->SetWidth(100); 
    hFrame->AddFrame(te, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2)); 
    fPowerParIds.push_back(dmap[i].first); 
    fPowerTextEntries.insert(make_pair(dmap[i].first, te)); 

    te->SetText(Form("%5.3f", float(dmap[i].second)));
    te->Connect("ReturnPressed()", "PixParTab", this, "setPowerSettings()");

    tset = new TGTextButton(hFrame, "Set", i);
    tset->SetToolTipText("set the parameter\nor click *return* after changing the numerical value");
    tset->GetToolTip()->SetDelay(2000); // add a bit of delay to ease button hitting
    tset->Connect("Clicked()", "PixParTab", this, "setPowerSettings()");
    hFrame->AddFrame(tset, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
  }

  tset = new TGTextButton(g1Frame, "Save Parameters");
  tset->SetToolTipText(Form("Write the testboard parameters to file.\nThe output file will overwrite whatever is in the directory \"%s\"\n(change this in the top right part of the GUI)", fConfigParameters->getDirectory().c_str()));
  tset->Connect("Clicked()", "PixParTab", this, "saveTbParameters()");
  g1Frame->AddFrame(tset, new TGLayoutHints(kLHintsBottom|kLHintsRight, 2, 2, 2, 2)); 

  vFrame->AddFrame(g1Frame);

  // -- TBM Parameters
  TGCompositeFrame *bGroup = new TGCompositeFrame(vFrame, 60, 20, kHorizontalFrame |kSunkenFrame);
  //xx  for (unsigned int i = 0; i < fConfigParameters->getNtbms(); ++i) {
  vector<vector<pair<string, uint8_t> > > cmap;
  for (int i = 0; i < fGui->getApi()->_dut->getNTbms(); ++i) {
    tcb = new TGCheckButton(bGroup, Form("%d", i), i); 
    bGroup->AddFrame(tcb, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2)); 
    fSelectTbm.push_back(tcb); 
    vector<pair<string, uint8_t> > smap = fGui->getApi()->_dut->getTbmDACs(i);
    cmap.push_back(smap); 
  }
  if (fSelectTbm.size() > 0) {
    fSelectedTbm = 0;
    // -- by default enable all present
    for (unsigned itbm = 0; itbm < fSelectTbm.size(); ++itbm) {
      fSelectTbm[itbm]->SetState(kButtonDown);
    }
  }
  vFrame->AddFrame(bGroup, new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 1, 1, 1, 1));  
  updateSelection();

  g2Frame = new TGGroupFrame(vFrame, "DAC of first selected TBM");

  
  //xx  vector<vector<pair<string, uint8_t> > >   cmap = fConfigParameters->getTbmDacs();


  if (cmap.size() > 0) {

    unsigned int firsttbm(0); 
    for (unsigned int i = 0; i < fSelectTbm.size(); ++i) {
      if (kButtonDown == fSelectTbm[i]->GetState()) {
	firsttbm = i; 
	break;
      }
    }

    
    //xx    for (unsigned itbm = 0; itbm < fConfigParameters->getNtbms(); ++itbm) {
    for (int itbm = 0; itbm < fGui->getApi()->_dut->getNTbms(); ++itbm) {
      
      map<string, uint8_t>  parids;
      amap = cmap[itbm];  
      
      for (unsigned int i = 0; i < amap.size(); ++i) {
	if (static_cast<unsigned int>(itbm) == firsttbm) {
	  hFrame = new TGHorizontalFrame(g2Frame, 300, 30, kLHintsExpandX); 
	  g2Frame->AddFrame(hFrame, new TGLayoutHints(kLHintsRight | kLHintsTop));
	  LOG(logDEBUG) << "Creating TGTextEntry for " << amap[i].first; 
	  tb = new TGTextBuffer(5); 
	  tl = new TGLabel(hFrame, amap[i].first.c_str());
	  tl->SetWidth(100);
	  hFrame->AddFrame(tl, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
	  
	  te  = new TGTextEntry(hFrame, tb, i); te->SetWidth(100); 
	  hFrame->AddFrame(te, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2)); 
	  std::bitset<8> bits(amap[i].second);
	    te->SetText(Form("%s", bits.to_string().c_str()));
	  te->Connect("ReturnPressed()", "PixParTab", this, "setTbmParameter()");
	  
	  tset = new TGTextButton(hFrame, "Set", i);
	  tset->SetToolTipText("set the parameter\nor click *return* after changing the numerical value");
	  tset->GetToolTip()->SetDelay(2000); // add a bit of delay to ease button hitting
	  tset->Connect("Clicked()", "PixParTab", this, "setTbmParameter()");
	  hFrame->AddFrame(tset, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
	}
	parids.insert(make_pair(amap[i].first, amap[i].second)); 
	fTbmTextEntries.insert(make_pair(amap[i].first, te)); 
	fTbmTextMap.insert(make_pair(i, amap[i].first)); 
	
      }
      fTbmParIds.push_back(parids);
      vFrame->AddFrame(g2Frame);
      g1Frame->SetWidth(g2Frame->GetDefaultWidth());
    }


    tset = new TGTextButton(g2Frame, "Save Parameters");
    tset->SetToolTipText(Form("Write the TBM parameters of all selected TBMs to file.\nThe output file will overwrite whatever is in the directory \"%s\"\n(change this in the top right part of the GUI)", fConfigParameters->getDirectory().c_str()));
    tset->Connect("Clicked()", "PixParTab", this, "saveTbmParameters()");
    g2Frame->AddFrame(tset, new TGLayoutHints(kLHintsBottom|kLHintsRight, 2, 2, 2, 2)); 
  }

  // -- DAC Parameters
  vFrame = new TGVerticalFrame(fhFrame);
  fhFrame->AddFrame(vFrame, new TGLayoutHints(kLHintsLeft, 15, 15, 15, 15));

  hFrame = new TGHorizontalFrame(vFrame, 300, 30, kLHintsExpandX); 
  vFrame->AddFrame(hFrame); 
  hFrame->AddFrame(tset = new TGTextButton(hFrame, "Select all", B_SELECTALL));
  tset->SetToolTipText("select all ROCs.\nSetting a DAC will affect all selected ROCs.\nTo view the DACs for a specific ROC, select *only* that ROC.");
  tset->Connect("Clicked()", "PixParTab", this, "handleButtons()");
  hFrame->AddFrame(tset = new TGTextButton(hFrame, "Deselect all", B_DESELECTALL));
  tset->SetToolTipText("deselect all ROCs");
  tset->Connect("Clicked()", "PixParTab", this, "handleButtons()");
  
  bGroup = new TGCompositeFrame(vFrame, 60, 20, kHorizontalFrame |kSunkenFrame);
  cmap.clear();  //xx = fConfigParameters->getRocDacs();
  for (int i = 0; i < fGui->getApi()->_dut->getNRocs(); ++i) {
    tcb = new TGCheckButton(bGroup, Form("%d", i), i); 
    tcb->Connect("Clicked()", "PixParTab", this, "selectRoc()");
    bGroup->AddFrame(tcb, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2)); 
    fSelectRoc.push_back(tcb); 
    vector<pair<string, uint8_t> > smap = fGui->getApi()->_dut->getDACs(i);
    cmap.push_back(smap); 
  }
  if (fSelectRoc.size() > 0) {
    fSelectedRoc = 0; 
    // -- by default enable all present
    for (unsigned iroc = 0; iroc < fSelectRoc.size(); ++iroc) {
      fSelectRoc[iroc]->SetState(kButtonDown);
    }
  }
  updateSelection();
  
  vFrame->AddFrame(bGroup, new TGLayoutHints(kLHintsCenterX|kLHintsCenterY, 1, 1, 1, 1));  

  hFrame = new TGHorizontalFrame(vFrame);
  vFrame->AddFrame(hFrame, new TGLayoutHints(kLHintsBottom, 2, 2, 2, 2));
  
  g1Frame = new TGGroupFrame(hFrame, "DACs of first selected ROC");
  hFrame->AddFrame(g1Frame, new TGLayoutHints(kLHintsLeft, 2, 2, 2, 2));
  g2Frame = new TGGroupFrame(hFrame, "DACs of first selected ROC");
  hFrame->AddFrame(g2Frame, new TGLayoutHints(kLHintsRight, 2, 2, 2, 2));
  
  if (cmap.size() > 0) {
    unsigned int firstroc(0); 
    for (unsigned int i = 0; i < fSelectRoc.size(); ++i) {
      if (kButtonDown == fSelectRoc[i]->GetState()) {
	firstroc = i; 
	break;
      }
    }
    
    for (int iroc = 0; iroc < fGui->getApi()->_dut->getNRocs(); ++iroc) {
      //xx for (unsigned iroc = 0; iroc < fConfigParameters->getNrocs(); ++iroc) {

      std::map<std::string, uint8_t>  parids;
      amap = cmap[iroc]; 
      unsigned int idac(0); 
      for (idac = 0; idac < 0.5*amap.size(); ++idac) {
	if (static_cast<unsigned int>(iroc) == firstroc) {
	  hFrame = new TGHorizontalFrame(g1Frame, 300, 30, kLHintsExpandX); 
	  g1Frame->AddFrame(hFrame, new TGLayoutHints(kLHintsRight | kLHintsTop));
	  LOG(logDEBUG) << "Creating TGTextEntry for " << amap[idac].first; 
	  tb = new TGTextBuffer(5); 
	  tl = new TGLabel(hFrame, amap[idac].first.c_str());
	  tl->SetWidth(100);
	  hFrame->AddFrame(tl, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
	  te  = new TGTextEntry(hFrame, tb, idac); te->SetWidth(100); 
	  hFrame->AddFrame(te, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2)); 
	  te->SetText(Form("%d", int(amap[idac].second)));
	  te->Connect("ReturnPressed()", "PixParTab", this, "setRocParameter()");
	  tset = new TGTextButton(hFrame, "Set", idac);
	  tset->SetToolTipText("set the parameter\nor click *return* after changing the numerical value");
	  tset->GetToolTip()->SetDelay(2000); // add a bit of delay to ease button hitting
	  tset->Connect("Clicked()", "PixParTab", this, "setRocParameter()");
	  hFrame->AddFrame(tset, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
	}

	parids.insert(make_pair(amap[idac].first, amap[idac].second)); 
	fRocTextEntries.insert(make_pair(amap[idac].first, te)); 
	fRocTextMap.insert(make_pair(idac, amap[idac].first));
      }
      
      for (idac = 0.5*amap.size()+1; idac < amap.size(); ++idac) {
	if (static_cast<unsigned int>(iroc) == firstroc) {
	  hFrame = new TGHorizontalFrame(g2Frame, 300, 30, kLHintsExpandX); 
	  g2Frame->AddFrame(hFrame, new TGLayoutHints(kLHintsRight | kLHintsTop));
	  LOG(logDEBUG) << "Creating TGTextEntry for " << amap[idac].first; 
	  tb = new TGTextBuffer(5); 
	  tl = new TGLabel(hFrame, amap[idac].first.c_str());
	  tl->SetWidth(100);
	  hFrame->AddFrame(tl, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 
	  
	  te  = new TGTextEntry(hFrame, tb, idac); te->SetWidth(100); 
	  hFrame->AddFrame(te, new TGLayoutHints(kLHintsCenterY | kLHintsCenterX, 2, 2, 2, 2)); 
	  te->SetText(Form("%d", int(amap[idac].second)));
	  te->Connect("ReturnPressed()", "PixParTab", this, "setRocParameter()");
	  
	  tset = new TGTextButton(hFrame, "Set", idac);
	  tset->SetToolTipText("set the parameter\nor click *return* after changing the numerical value");
	  tset->GetToolTip()->SetDelay(2000); // add a bit of delay to ease button hitting
	  tset->Connect("Clicked()", "PixParTab", this, "setRocParameter()");
	  hFrame->AddFrame(tset, new TGLayoutHints(kLHintsCenterY | kLHintsLeft, 2, 2, 2, 2)); 

	}	

	parids.insert(make_pair(amap[idac].first, amap[idac].second)); 
	fRocTextEntries.insert(make_pair(amap[idac].first, te)); 
	fRocTextMap.insert(make_pair(idac, amap[idac].first));
      }
    
      fRocParIds.push_back(parids);
    }

    tset = new TGTextButton(g1Frame, "Save DAC");
    tset->SetToolTipText(Form("Write the DAC parameters of all selected ROCs to file\n(also the DACs of the righthand box will be written).\nThe output file will overwrite whatever is in the directory \"%s\"\n(change this in the top right part of the GUI)", fConfigParameters->getDirectory().c_str()));
    tset->Connect("Clicked()", "PixParTab", this, "saveDacParameters()");
    g1Frame->AddFrame(tset, new TGLayoutHints(kLHintsBottom|kLHintsRight, 2, 2, 2, 2)); 

    tset = new TGTextButton(g1Frame, "Save Trim");
    tset->SetToolTipText(Form("Write the trim parameters of all selected ROCs to file.\nThe output file will overwrite whatever is in the directory \"%s\"\n(change this in the top right part of the GUI)", fConfigParameters->getDirectory().c_str()));
    tset->Connect("Clicked()", "PixParTab", this, "saveTrimParameters()");
    g1Frame->AddFrame(tset, new TGLayoutHints(kLHintsBottom|kLHintsRight, 2, 2, 2, 2)); 
  }

  fTabFrame->Layout();
  fTabFrame->MapSubwindows();
  fTabFrame->Resize(fTabFrame->GetDefaultSize());
  fTabFrame->MapWindow();

}


// ----------------------------------------------------------------------
PixParTab::PixParTab() {
  init(0, 0, "nada");
}

// ----------------------------------------------------------------------
void PixParTab::init(PixGui *p, ConfigParameters *cfg, std::string tabname) {
  LOG(logDEBUG) << "PixParTab::init()";
  fGui = p;
  fConfigParameters = cfg; 
  fTabName = tabname; 
}

// ----------------------------------------------------------------------
// PixParTab destructor
PixParTab::~PixParTab() {
  LOG(logDEBUG) << "PixParTab destructor";
}


// ----------------------------------------------------------------------
void PixParTab::handleButtons(Int_t id) {
  
  if (!fGui->getTabs()) return;
  
  if (id == -1) {
    TGButton *btn = (TGButton*)gTQSender;
    id = btn->WidgetId();
  }
  
  switch (id) {
  case B_SELECTALL: {
    LOG(logDEBUG) << "SELECT ALL";
    for (unsigned int i = 0; i < fSelectRoc.size(); ++i) {
      fSelectRoc[i]->SetState(kButtonDown); 
    }
    updateSelection();
    break;
  }

  case B_DESELECTALL: {
    LOG(logDEBUG) << "DESELECT ALL";
    for (unsigned int i = 0; i < fSelectRoc.size(); ++i) {
      fSelectRoc[i]->SetState(kButtonUp); 
    }
    updateSelection();
    break;
  }
    
  }
}


// ----------------------------------------------------------------------
void PixParTab::setTbParameter() {
  if (!fGui->getTabs()) return;
  LOG(logDEBUG)  << "PixParTab::setTbParameter: ";

  TGButton *btn = (TGButton *) gTQSender;
  int id(-1); 
  id = btn->WidgetId();
  if (-1 == id) {
    LOG(logDEBUG) << "ASLFDKHAPIUDF ";
    return; 
  }

  string svalue = ((TGTextEntry*)(fTbTextEntries[fTbParIds[id]]))->GetText(); 
  uint8_t udac = atoi(svalue.c_str()); 

  cout << "FIXME FIXME: ID = " << id << " -> " << fTbParIds[id] << " set to " << svalue << endl;
  fConfigParameters->setTbParameter(fTbParIds[id], udac); 

  initTestboard(); 

} 

// ----------------------------------------------------------------------
void PixParTab::setPgSettings() {
  if (!fGui->getTabs()) return;
  LOG(logDEBUG)  << "PixParTab::setPgSettings: ";

  TGButton *btn = (TGButton *) gTQSender;
  int id(-1); 
  id = btn->WidgetId();
  if (-1 == id) {
    LOG(logDEBUG) << "ASLFDKHAPIUDF ";
    return; 
  }

  string svalue = ((TGTextEntry*)(fPgTextEntries[fPgParIds[id]]))->GetText(); 
  uint8_t udac = atoi(svalue.c_str()); 

  cout << "FIXME FIXME: ID = " << id << " -> " << fPgParIds[id] << " set to " << int(udac) << " from svalue = " << svalue << endl;

  initTestboard(); 

} 


// ----------------------------------------------------------------------
void PixParTab::setPowerSettings() {
  if (!fGui->getTabs()) return;
  LOG(logDEBUG)  << "PixParTab::setPowerSettings: ";

  TGButton *btn = (TGButton *) gTQSender;
  int id(-1); 
  id = btn->WidgetId();
  if (-1 == id) {
    LOG(logDEBUG) << "ASLFDKHAPIUDF ";
    return; 
  }

  string svalue = ((TGTextEntry*)(fPowerTextEntries[fPowerParIds[id]]))->GetText(); 
  double udac = atof(svalue.c_str()); 
  
  cout << "FIXME FIXME: ID = " << id << " -> " << fPowerParIds[id] << " set to " << svalue << endl;
  fConfigParameters->setTbPowerSettings(fPowerParIds[id], udac); 
  
  initTestboard(); 

  // FIXME UPDATE CONFIGPARAMETERS!

} 


// ----------------------------------------------------------------------
void PixParTab::initTestboard() {

  vector<pair<string, uint8_t> > sig_delays = fConfigParameters->getTbSigDelays();
  vector<pair<string, double> > power_settings = fConfigParameters->getTbPowerSettings();
  vector<pair<uint16_t, uint8_t> > pg_setup = fConfigParameters->getTbPgSettings();;

  LOG(logDEBUG) << "Re-programming TB"; 

  fGui->getApi()->initTestboard(sig_delays, power_settings, pg_setup);

}


// ----------------------------------------------------------------------
void PixParTab::setTbmParameter() {
  if (!fGui->getTabs()) return;
  LOG(logDEBUG)  << "PixParTab::setTbmParameter: ";

  TGButton *btn = (TGButton *) gTQSender;
  int id(-1); 
  id = btn->WidgetId();
  if (-1 == id) {
    LOG(logDEBUG) << "ASLFDKHAPIUDF ";
    return; 
  }

  string sdac = fTbmTextMap[id]; 
  string sval = fTbmTextEntries[sdac]->GetText(); 
  //  uint8_t udac = atoi(sval.c_str()); 
  bitset<8> bits(sval); 
  uint8_t udac = bits.to_ulong();

  int itbm(-1); 
  for (unsigned int i = 0; i < fSelectTbm.size(); ++i) {
    if (kButtonDown == fSelectTbm[i]->GetState()) {
      itbm = i; 
      LOG(logDEBUG) << "TBM " << itbm << " is selected. id = " << id;
      fTbmParIds[itbm][sdac] = udac;
      LOG(logDEBUG)<< "xxx: ID = " << id << " TBM = " << itbm
		  << " -> " << sdac << " set to int(udac) = " << int(udac);
      fGui->getApi()->setTbmReg(sdac, udac, itbm);
      //xx fConfigParameters->setTbmDac(sdac, udac, itbm);       
    }
  }

} 


// ----------------------------------------------------------------------
void PixParTab::selectRoc(int iroc) {

  bool selected(false); 
  if (iroc == -1) {
    TGButton *btn = (TGButton *) gTQSender;
    if (kButtonDown == btn->GetState()) {
      selected = true;
    }
    iroc = btn->WidgetId();
  }

  LOG(logDEBUG) << "selectRoc: iroc = " << iroc << " selected? " << selected;

  if (false == selected) {
    iroc = 0;
    for (unsigned int i = 0; i < fSelectRoc.size(); ++i) {
      if (kButtonDown == fSelectRoc[i]->GetState()) {
	iroc = i; 
	break;
      }
    }
    LOG(logDEBUG) << "choosing first selected ROC (or 0) instead: " << iroc << endl;
  }

  fSelectedRoc = iroc; 

  map<string, uint8_t> amap = fRocParIds[fSelectedRoc]; 
  for (map<string, uint8_t >::iterator mapit = amap.begin(); mapit != amap.end(); ++mapit) {
    fRocTextEntries[(*mapit).first]->SetText(Form("%d", (*mapit).second)); 
  }
  updateSelection();
}


// ----------------------------------------------------------------------
void PixParTab::selectTbm(int id) {

  if (id == -1) {
    TGButton *btn = (TGButton *) gTQSender;
    id = btn->WidgetId();
  }

  LOG(logDEBUG) << "selectTbm: id = " << id;
  fSelectedTbm = id; 
  updateSelection();
}


// ----------------------------------------------------------------------
void PixParTab::setRocParameter() {
  if (!fGui->getTabs()) return;
  LOG(logDEBUG)  << "PixParTab::setRocParameter: ";

  TGButton *btn = (TGButton *) gTQSender;
  int id(-1); 
  id = btn->WidgetId();
  if (-1 == id) {
    LOG(logDEBUG) << "ASLFDKHAPIUDF ";
    return; 
  }

  string sdac = fRocTextMap[id]; 
  string sval = fRocTextEntries[sdac]->GetText(); 
  uint8_t udac = atoi(sval.c_str()); 

  int iroc(-1); 
  for (unsigned int i = 0; i < fSelectRoc.size(); ++i) {
    if (kButtonDown == fSelectRoc[i]->GetState()) {
      iroc = i; 
      LOG(logDEBUG) << "ROC " << iroc << " is selected. id = " << id;
      fRocParIds[iroc][sdac]  = udac; 
      LOG(logDEBUG)<< "xxx: ID = " << id << " roc = " << iroc 
		  << " -> " << sdac << " set to  int(udac) = " << int(udac);
      fGui->getApi()->setDAC(sdac, udac, iroc);
      //xx fConfigParameters->setRocDac(sdac, udac, iroc);       
    }
  }

} 


// ----------------------------------------------------------------------
void PixParTab::saveTbParameters() {
  LOG(logDEBUG) << "save Tb parameters"; 
  fConfigParameters->writeTbParameterFile();
}

// ----------------------------------------------------------------------
void PixParTab::saveTbmParameters() {
  LOG(logDEBUG) << "save Tbm parameters"; 
  //xx  fConfigParameters->writeTbmParameterFiles(getSelectedTbms());
  //  vector<uint8_t> tbms = fGui->getApi()->_dut->getEnabledTbmIDs(); 
  //  for (unsigned int itbm = 0; itbm < tbms.size(); ++itbm) {
  int itbm(0); 
  fConfigParameters->writeTbmParameterFile(0, fGui->getApi()->_dut->getTbmDACs(itbm)); 
  //  }
}

// ----------------------------------------------------------------------
void PixParTab::saveDacParameters() {
  LOG(logDEBUG) << "save DAC parameters"; 
  //xx  fConfigParameters->writeDacParameterFiles(getSelectedRocs());
  vector<uint8_t> rocs = fGui->getApi()->_dut->getEnabledRocIDs(); 
  LOG(logDEBUG) << " rocs.size() = " << rocs.size(); 
  for (unsigned int iroc = 0; iroc < rocs.size(); ++iroc) {
    fConfigParameters->writeDacParameterFile(rocs[iroc], fGui->getApi()->_dut->getDACs(rocs[iroc])); 
  }
}


// ----------------------------------------------------------------------
void PixParTab::saveTrimParameters() {
  LOG(logDEBUG) << "save Trim parameters"; 
  //xx  fConfigParameters->writeTrimFiles(getSelectedRocs());
  vector<uint8_t> rocs = fGui->getApi()->_dut->getEnabledRocIDs(); 
  for (unsigned int iroc = 0; iroc < rocs.size(); ++iroc) {
    fConfigParameters->writeTrimFile(rocs[iroc], fGui->getApi()->_dut->getEnabledPixels(rocs[iroc])); 
  }

}


// ----------------------------------------------------------------------
vector<int> PixParTab::getSelectedTbms() {
  vector<int> result; 
  for (unsigned int i = 0; i < fSelectTbm.size(); ++i) {
    if (kButtonDown == fSelectTbm[i]->GetState()) {
      result.push_back(i); 
    }
  }
  return result;
}

// ----------------------------------------------------------------------
vector<int> PixParTab::getSelectedRocs() {
  vector<int> result; 
  for (unsigned int i = 0; i < fSelectRoc.size(); ++i) {
    if (kButtonDown == fSelectRoc[i]->GetState()) {
      result.push_back(i); 
    }
  }
  return result;
}


// ----------------------------------------------------------------------
void PixParTab::updateSelection() {
  vector<int> selectedRocs = getSelectedRocs(); 
  for (int i = 0; i < fGui->getApi()->_dut->getNRocs(); ++i) {
    fGui->getApi()->_dut->setROCEnable(i, false); 
  }
  for (unsigned i = 0; i < selectedRocs.size(); ++i) {
    fGui->getApi()->_dut->setROCEnable(selectedRocs[i], true); 
  }

  vector<int> selectedTbms = getSelectedTbms(); 
  for (int i = 0; i < fGui->getApi()->_dut->getNTbms(); ++i) {
    fGui->getApi()->_dut->setTBMEnable(i, false); 
  }
  for (unsigned i = 0; i < selectedTbms.size(); ++i) {
    fGui->getApi()->_dut->setTBMEnable(selectedTbms[i], true); 
  }

  //xx fConfigParameters->setSelectedRocs(getSelectedRocs());
  //xx fConfigParameters->setSelectedTbms(getSelectedTbms());
}
