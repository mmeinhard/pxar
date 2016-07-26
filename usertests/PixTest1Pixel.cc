#include <stdlib.h>     
#include <algorithm>    
#include <iostream>
#include <fstream>

#include <TH1.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include <TStyle.h>

#include "PixTest1Pixel.hh"
#include "PixUtil.hh"
#include "log.h"
#include "helper.h"

using namespace std;
using namespace pxar;

ClassImp(PixTest1Pixel)


/*
-- 1Pixel
vcal                200
ntrig               10
*/


// ----------------------------------------------------------------------
PixTest1Pixel::PixTest1Pixel(PixSetup *a, std::string name) : PixTest(a, name), fParVcal(35), fParNtrig(1) {
  PixTest::init();
  init(); 

  fPhCal.setPHParameters(fPixSetup->getConfigParameters()->getGainPedestalParameters());
  fPhCalOK = fPhCal.initialized();
  //  LOG(logINFO) << "PixTest1Pixel ctor(PixSetup &a, string, TGTab *)";
}


//----------------------------------------------------------
PixTest1Pixel::PixTest1Pixel() : PixTest() {
  //  LOG(logINFO) << "PixTest1Pixel ctor()";
}

// ----------------------------------------------------------------------
bool PixTest1Pixel::setParameter(string parName, string sval) {
  bool found(false);
  string str1, str2; 
  string::size_type s1;
  int pixc, pixr; 
  fApi->setDAC("Vcal", fParVcal);
  fApi->setDAC("CtrlReg", 8);
  std::transform(parName.begin(), parName.end(), parName.begin(), ::tolower);
  for (unsigned int i = 0; i < fParameters.size(); ++i) {
    if (fParameters[i].first == parName) {
      found = true; 
      if (!parName.compare("ntrig")) {
	fParNtrig = atoi(sval.c_str()); 
      }
      if (!parName.compare("vcal")) {
	fParVcal = atoi(sval.c_str()); 
      }
      if (!parName.compare("pix")) {
        s1 = sval.find(","); 
        if (string::npos != s1) {
          str1 = sval.substr(0, s1); 
          pixc = atoi(str1.c_str()); 
          str2 = sval.substr(s1+1); 
          pixr = atoi(str2.c_str()); 
          clearSelectedPixels();
          fPIX.push_back(make_pair(pixc, pixr)); 
          addSelectedPixels(sval); 
        } else {
          clearSelectedPixels();
          fPIX.push_back(make_pair(-1, -1)); 
          addSelectedPixels("-1,-1"); 
        }
      }
      break;
    }
  }
  
  return found; 
}


// ----------------------------------------------------------------------
void PixTest1Pixel::init() {
  setToolTips(); 
  fDirectory = gFile->GetDirectory(fName.c_str()); 
  if (!fDirectory) {
    fDirectory = gFile->mkdir(fName.c_str()); 
  } 
  fDirectory->cd(); 

}



// ----------------------------------------------------------------------
void PixTest1Pixel::setToolTips() {
  fTestTip    = string(Form("send n number of calibrates to one pixel\n")
		       + string("TO BE FINISHED!!"))
    ;
  fSummaryTip = string("summary plot to be implemented")
    ;
}

// ----------------------------------------------------------------------
void PixTest1Pixel::bookHist(string name) {
  fDirectory->cd(); 

  LOG(logDEBUG) << "nothing done with " << name;
}


//----------------------------------------------------------
PixTest1Pixel::~PixTest1Pixel() {
  LOG(logDEBUG) << "PixTest1Pixel dtor";
}


// ----------------------------------------------------------------------
void PixTest1Pixel::doTest() {

  TStopwatch t;

  fDirectory->cd();
  PixTest::update(); 
  bigBanner(Form("PixTest1Pixel::doTest()"));

  fProblem = false;
  gStyle->SetPalette(1);
  bool verbose(false);
  cacheDacs(verbose);
  fDirectory->cd();
  PixTest::update(); 

  vector<pair<string, uint8_t> > a;
  
  uint8_t wbc = 100;
  uint8_t delay = 5;
  //fApi->setDAC("Vcal", fParVcal);
  //fApi->setDAC("CtrlReg",8);
  fApi->flushTestboard();
  fPg_setup.clear();
  //a.push_back(make_pair("resetroc",25));    // PG_RESR b001000 
  a.push_back(make_pair("calibrate",wbc+delay)); // PG_CAL  b000100
  a.push_back(make_pair("trigger",16));    // PG_TRG  b000010
  a.push_back(make_pair("token",0));     // PG_TOK  b000001
  for (unsigned i = 0; i < a.size(); ++i) {
    fPg_setup.push_back(a[i]);
  }

  fApi->setPatternGenerator(fPg_setup);
 
  fApi->_dut->maskAllPixels(false);
  fApi->_dut->testAllPixels(false);

  fApi->_dut->testPixel(fPIX[0].first,fPIX[0].second,true);

  fApi->daqStart();
  fApi->daqTrigger(fParNtrig, 500);
  fApi->daqStop();

  vector<pxar::Event> daqdat;
  try { daqdat = fApi->daqGetEventBuffer(); }
  catch(pxar::DataNoEvent &) {
    LOG(logERROR) << "no data";
  }
  int EventId=0;

  TH1D *h;
  h = bookTH1D("PH distribution", "PH distribution", 256, 0, 256.0);
  TH1D *h_noise;
  h_noise = bookTH1D("PH distribution (noise)", "PH distribution (noise)", 256, 0, 256.0);
  TH1D *h_charge;
  h_charge = bookTH1D("PH distribution (charge)", "PH distribution (charge)", 200, 0, 400.0);
  TH1D *h_time;
  h_time = bookTH1D("nhits", "nhits", fParNtrig, 0, fParNtrig);

  for (std::vector<pxar::Event>::iterator it = daqdat.begin(); it != daqdat.end(); ++it) {
    std::stringstream ss("");
    ss << "Event " << EventId << ": ";
      bool pixelSeen = false;
      for (int i=0;i<it->pixels.size();i++) {
        /*
        if (it->pixels[i].row() != 20 || it->pixels[i].column() != 11) {
          ss << "[";
        }
        ss << (int)it->pixels[i].column() << "," << (int)it->pixels[i].row() << ": " << it->pixels[i].value();
        if (it->pixels[i].row() != 20 || it->pixels[i].column() != 11) {
          ss << "]";
        }*/

        if (it->pixels[i].row() == fPIX[0].second && it->pixels[0].column() == fPIX[i].first) {
          h->Fill(it->pixels[i].value());
          double q;
          if (fPhCalOK) {
            q = fPhCal.vcal(it->pixels[i].roc(), 
                it->pixels[i].column(), 
                it->pixels[i].row(), 
                it->pixels[i].value());

            h_charge->Fill(q);
          }
          pixelSeen = true;
        } else {
          h_noise->Fill(it->pixels[i].value());
        }
        h_time->Fill(EventId);
        //ss << " ";
      }

      if (!pixelSeen) {
          h->Fill(0);
      }
    //LOG(logINFO) << ss.str();
    EventId++;
  }

  fHistList.push_back(h_noise);
  fHistList.push_back(h_charge);
  fHistList.push_back(h_time);
  fHistList.push_back(h);
  h->Draw();
  fDisplayedHist = find(fHistList.begin(), fHistList.end(), h);
  

  if (fProblem) {
    LOG(logINFO) << "PixTest1Pixel::doTest() aborted because of problem ";
    return;
  }

  int seconds = t.RealTime(); 
  LOG(logINFO) << "PixTest1Pixel::doTest() done, duration: " << seconds << " seconds";
  LOG(logINFO) << "number of triggers sent: " << fParNtrig; 
  //LOG(logINFO) << "number of hits detected:  " << nhits; 
  //LOG(logINFO) << "hits missed: ";

  PixTest::update(); 
}