#include <stdlib.h>     
#include <algorithm>    
#include <iostream>
#include <fstream>

#include <TH1.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include <TStyle.h>

#include "PixTestXPixelAlive2.hh"
#include "PixUtil.hh"
#include "log.h"
#include "helper.h"


using namespace std;
using namespace pxar;

ClassImp(PixTestXPixelAlive2)


/*
-- XPixelAlive2
vcal                200
ntrig               10
*/


// ----------------------------------------------------------------------
PixTestXPixelAlive2::PixTestXPixelAlive2(PixSetup *a, std::string name) : PixTest(a, name), fParVcal(35), fParNtrig(1), fParReset(-1),fParMask(-1),fParDelayTBM(false) {
	PixTest::init();
	init(); 

	fPhCal.setPHParameters(fPixSetup->getConfigParameters()->getGainPedestalParameters());
	fPhCalOK = fPhCal.initialized();
	//  LOG(logINFO) << "PixTestXPixelAlive2 ctor(PixSetup &a, string, TGTab *)";
}


//----------------------------------------------------------
PixTestXPixelAlive2::PixTestXPixelAlive2() : PixTest() {
	//  LOG(logINFO) << "PixTestXPixelAlive2 ctor()";
}

// ----------------------------------------------------------------------
bool PixTestXPixelAlive2::setParameter(string parName, string sval) {
	bool found(false);
	string str1, str2; 
	std::transform(parName.begin(), parName.end(), parName.begin(), ::tolower);
	for (unsigned int i = 0; i < fParameters.size(); ++i) {
		if (fParameters[i].first == parName) {
			found = true; 
			if (!parName.compare("resetroc")) {
				PixUtil::replaceAll(sval, "checkbox(", ""); 
				PixUtil::replaceAll(sval, ")", ""); 
				fParReset = atoi(sval.c_str()); 
				setToolTips();
			}
			if (!parName.compare("maskborder")) {
				PixUtil::replaceAll(sval, "checkbox(", ""); 
				PixUtil::replaceAll(sval, ")", ""); 
				fParMask = atoi(sval.c_str()); 
				setToolTips();
			}
			if (!parName.compare("delaytbm")) {
				PixUtil::replaceAll(sval, "checkbox(", "");
				PixUtil::replaceAll(sval, ")", "");
				fParDelayTBM = !(atoi(sval.c_str())==0);
				setToolTips();
      			}
			if (!parName.compare("ntrig")) {
	fParNtrig = atoi(sval.c_str()); 
			}
			if (!parName.compare("vcal")) {
	fParVcal = atoi(sval.c_str()); 
			}
			break;
		}
	}
	
	return found; 
}


// ----------------------------------------------------------------------
void PixTestXPixelAlive2::init() {
	setToolTips(); 
	fDirectory = gFile->GetDirectory(fName.c_str()); 
	if (!fDirectory) {
		fDirectory = gFile->mkdir(fName.c_str()); 
	} 
	fDirectory->cd(); 

}



// ----------------------------------------------------------------------
void PixTestXPixelAlive2::setToolTips() {
	fTestTip    = string(Form("send n number of calibrates to one pixel\n")
					 + string("TO BE FINISHED!!"))
		;
	fSummaryTip = string("summary plot to be implemented")
		;
}
// ----------------------------------------------------------------------
void PixTestXPixelAlive2::bookHist(string name) {
  fDirectory->cd();

}


//----------------------------------------------------------
PixTestXPixelAlive2::~PixTestXPixelAlive2() {
	LOG(logDEBUG) << "PixTestXPixelAlive2 dtor";
}


// ----------------------------------------------------------------------
void PixTestXPixelAlive2::doTest() {

	ConfigParameters* config = fPixSetup->getConfigParameters();

	TStopwatch t;

	vector<uint8_t> rocIds = fApi->_dut->getEnabledRocIDs();
        unsigned nrocs = rocIds.size();

	fDirectory->cd();
	PixTest::update(); 
	bigBanner(Form("PixTestXPixelAlive2::doTest()"));

	fProblem = false;
	gStyle->SetPalette(1);
	bool verbose(false);
	cacheDacs(verbose);
	fDirectory->cd();
	PixTest::update(); 

	  // -- cache triggerdelay
  vector<pair<string, uint8_t> > oldDelays = fPixSetup->getConfigParameters()->getTbSigDelays();
  bool foundIt(false);
  for (unsigned int i = 0; i < oldDelays.size(); ++i) {
    if (oldDelays[i].first == "triggerdelay") {
      foundIt = true;
    }
    LOG(logDEBUG) << " old set: " << oldDelays[i].first << ": " << (int)oldDelays[i].second;
  }

  vector<pair<string, uint8_t> > delays = fPixSetup->getConfigParameters()->getTbSigDelays();
  if (!foundIt) {
    delays.push_back(make_pair("triggerdelay", 20));
    oldDelays.push_back(make_pair("triggerdelay", 0));
  } else {
    for (unsigned int i = 0; i < delays.size(); ++i) {
      if (delays[i].first == "triggerdelay") {
	delays[i].second = 20;
      }
    }
  }

  //fApi->setTbmReg("delays",0x40);

  for (unsigned int i = 0; i < delays.size(); ++i) {
    LOG(logDEBUG) << " setting: " << delays[i].first << ": " << (int)delays[i].second;
  }
  fApi->setTestboardDelays(delays);

	vector<pair<string, uint8_t> > a;
	
	/*uint8_t wbc = 100;
	uint8_t delay = 5;
	fApi->setDAC("Vcal", fParVcal);
	fPg_setup.clear();
	if (fParReset == 1) {
		a.push_back(make_pair("resetroc",50));    // PG_RESR b001000 
	}
	a.push_back(make_pair("calibrate",wbc+delay)); // PG_CAL  b000100
	LOG(logINFO) << config->getNtbms() ;
	if (config->getNtbms() < 1) {
	a.push_back(make_pair("trigger",16));    // PG_TRG  b000010
	a.push_back(make_pair("token",0));     // PG_TOK  b000001
	}
	else {
	a.push_back(std::make_pair("trigger;sync",0));     // PG_TRG PG_SYNC
	}
	for (unsigned i = 0; i < a.size(); ++i) {
		fPg_setup.push_back(a[i]);
	}



	fApi->setPatternGenerator(fPg_setup);*/
if (fParReset == 0) {
  fPg_setup.clear();
  vector<pair<string, uint8_t> > pgtmp = fPixSetup->getConfigParameters()->getTbPgSettings();
  for (unsigned i = 0; i < pgtmp.size(); ++i) {
    if (string::npos != pgtmp[i].first.find("resetroc")) continue;
    if (string::npos != pgtmp[i].first.find("resettbm")) continue;
    fPg_setup.push_back(pgtmp[i]);
  }
  if (0) for (unsigned int i = 0; i < fPg_setup.size(); ++i) cout << fPg_setup[i].first << ": " << (int)fPg_setup[i].second << endl;

  fApi->setPatternGenerator(fPg_setup);
}
	fApi->_dut->maskAllPixels(false);

	fApi->_dut->testAllPixels(false);
	if (fParMask == 1) {
		for (int i=0;i<52;i++) {
			fApi->_dut->maskPixel(i,0,true);
			fApi->_dut->maskPixel(i,79,true);
		}
		for (int i=0;i<80;i++) {
			fApi->_dut->maskPixel(0,i,true);
			fApi->_dut->maskPixel(51,i,true);
		}
	}
	
	
	vector <TH2D*> h_alive;
	vector <TH2D*> h_hitmap;

	for (int i=0; i<nrocs; i++) {
	h_alive.push_back(bookTH2D(Form("highRate_C%d", getIdFromIdx(i)), Form("highRate_C%d", getIdFromIdx(i)), 52,0,52,80,0,80));
	h_hitmap.push_back(bookTH2D(Form("highRate_xraymap_C%d", getIdFromIdx(i)), Form("highRate_xraymap_C%d", getIdFromIdx(i)), 52,0,52,80,0,80));

}

	//fApi->flushTestboard();

	fApi->daqSingleSignal("resetroc");
	fApi->daqStart();

	for (int co = 0; co<52; co++) {
		LOG(logINFO) << "col: " << co;
		for (int ro = 0; ro<80; ro++) {
			fApi->_dut->testPixel(co,ro,true);
			fApi->SetCalibrateBits(true);

			fApi->daqTrigger(fParNtrig, 1000);

			fApi->_dut->testPixel(co,ro,false);
			fApi->SetCalibrateBits(false);
		}
	}


	fApi->daqStop();

	vector<pxar::Event> daqdat;
	try { 
		daqdat = fApi->daqGetEventBuffer(); 
	} catch(pxar::DataNoEvent &) {
		LOG(logERROR) << "no data";
	}

	int EventId=0;

	for (std::vector<pxar::Event>::iterator it = daqdat.begin(); it != daqdat.end(); ++it) {
		int ro = (EventId/fParNtrig)%80;
		int co = (EventId/fParNtrig)/80;
	std::stringstream ss("");
	ss << "Event " << EventId << ": ";
		for (unsigned int i=0;i<it->pixels.size();i++) {
			/*
			if (it->pixels[i].row() != 20 || it->pixels[i].column() != 11) {
				ss << "[";
			}
			ss << (int)it->pixels[i].column() << "," << (int)it->pixels[i].row() << ": " << it->pixels[i].value();
			if (it->pixels[i].row() != 20 || it->pixels[i].column() != 11) {
				ss << "]";
			}*/
			int rocid = it->pixels[i].roc();
			int rocidx = getIdxFromId(rocid);
			if (it->pixels[i].row() == ro && it->pixels[i].column() == co) {
				h_alive[rocidx]->Fill(co,ro,1);
				
			} else {
				h_hitmap[rocidx]->Fill(it->pixels[i].column(), it->pixels[i].row(), 1);
			}
		}

	EventId++;
	}

    vector<int> deadPixel(nrocs, 0);
    vector<int> probPixel(nrocs, 0);
    vector<int> xHits(nrocs,0);
    vector<int> fidHits(nrocs,0);
    vector<int> allHits(nrocs,0);
    vector<int> fidPixels(nrocs,0);
	for (int r=0; r<nrocs; r++) {
	for (int ix = 0; ix < h_alive[r]->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < h_alive[r]->GetNbinsY(); ++iy) {
      allHits[r] += static_cast<int>(h_alive[r]->GetBinContent(ix+1, iy+1));
  		if ((ix > 0) && (ix < 51) && (iy < 79) && (h_alive[r]->GetBinContent(ix+1, iy+1) > 0)) {
    		fidHits[r] += static_cast<int>(h_alive[r]->GetBinContent(ix+1, iy+1));
    		++fidPixels[r];
  		}
  		// -- count dead pixels
 		  if (h_alive[r]->GetBinContent(ix+1, iy+1) < fParNtrig) {
    		++probPixel[r];
    		if (h_alive[r]->GetBinContent(ix+1, iy+1) < 1) {
      		++deadPixel[r];
    		}
  		}
  		// -- Count X-ray hits detected
  		if (h_hitmap[r]->GetBinContent(ix+1,iy+1)>0){
    		xHits[r] += static_cast<int> (h_hitmap[r]->GetBinContent(ix+1,iy+1));
  		}
    }
  }
}




	

	for (int r=0; r<nrocs; r++) {
	fHistList.push_back(h_hitmap[r]);
	fHistList.push_back(h_alive[r]);
	fHistOptions.insert(make_pair(h_hitmap[r], "colz"));
	fHistOptions.insert(make_pair(h_alive[r], "colz"));
	h_alive[r]->Draw("COLZ");
	}
	fDisplayedHist = find(fHistList.begin(), fHistList.end(), h_alive[nrocs-1]);
	

	if (fProblem) {
		LOG(logINFO) << "PixTestXPixelAlive2::doTest() aborted because of problem ";
		return;
	}

	double sensorArea = 0.015 * 0.010 * 54 * 81; // in cm^2, accounting for larger edge pixels (J. Hoss 2014/10/21)
	if (fParMask == 1) {
		sensorArea = 0.015 * 0.010 * 50 * 78;
	}
	string deadPixelString, probPixelString, xHitsString, numTrigsString,
	fidCalHitsString, allCalHitsString,
	fidCalEfficiencyString, allCalEfficiencyString,
	xRayRateString;
	for (unsigned int i = 0; i < probPixel.size(); ++i) {
		float fidefficiency = fidHits[i]/static_cast<double>(fidPixels[i]*fParNtrig);
		probPixelString += Form(" %4d", probPixel[i]);
		deadPixelString += Form(" %4d", deadPixel[i]);
		xHitsString     += Form(" %4d", xHits[i]);
		allCalHitsString += Form(" %4d", allHits[i]);
		fidCalHitsString += Form(" %4d", fidHits[i]);
		int numTrigs = fParNtrig * 4160;
		numTrigsString += Form(" %4d", numTrigs );
		fidCalEfficiencyString += Form(" %.1f", fidHits[i]/static_cast<double>(fidPixels[i]*fParNtrig)*100);
		allCalEfficiencyString += Form(" %.1f", allHits[i]/static_cast<double>(numTrigs)*100);
		xRayRateString += Form(" %.1f", xHits[i]/static_cast<double>(numTrigs)/25./sensorArea*1000./fidefficiency);
	}

	int seconds = t.RealTime(); 
	LOG(logINFO) << "number of dead pixels (per ROC): " << deadPixelString;
	LOG(logINFO) << "number of red-efficiency pixels: " << probPixelString;
	LOG(logINFO) << "number of X-ray hits detected:   " << xHitsString;
	LOG(logINFO) << "number of triggers sent (total per ROC): " << numTrigsString;
	LOG(logINFO) << "number of Vcal hits detected: " << allCalHitsString;
	LOG(logINFO) << "Vcal hit fiducial efficiency (%): " << fidCalEfficiencyString;
	LOG(logINFO) << "Vcal hit overall efficiency (%): " << allCalEfficiencyString;
	LOG(logINFO) << "X-ray hit rate [MHz/cm2]: " <<  xRayRateString;
	LOG(logINFO) << "XPixelAlive2::doTest() done, duration: " << seconds << " seconds";

	PixTest::update(); 
}
