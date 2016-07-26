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
PixTestXPixelAlive2::PixTestXPixelAlive2(PixSetup *a, std::string name) : PixTest(a, name), fParVcal(35), fParNtrig(1), fParReset(-1) {
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

	LOG(logDEBUG) << "nothing done with " << name;
}


//----------------------------------------------------------
PixTestXPixelAlive2::~PixTestXPixelAlive2() {
	LOG(logDEBUG) << "PixTestXPixelAlive2 dtor";
}


// ----------------------------------------------------------------------
void PixTestXPixelAlive2::doTest() {

	TStopwatch t;

	fDirectory->cd();
	PixTest::update(); 
	bigBanner(Form("PixTestXPixelAlive2::doTest()"));

	fProblem = false;
	gStyle->SetPalette(1);
	bool verbose(false);
	cacheDacs(verbose);
	fDirectory->cd();
	PixTest::update(); 

	vector<pair<string, uint8_t> > a;
	
	uint8_t wbc = 100;
	uint8_t delay = 5;
	fApi->setDAC("Vcal", fParVcal);
	fPg_setup.clear();
	if (fParReset == 1) {
		a.push_back(make_pair("resetroc",50));    // PG_RESR b001000 
	}
	a.push_back(make_pair("calibrate",wbc+delay)); // PG_CAL  b000100
	a.push_back(make_pair("trigger",16));    // PG_TRG  b000010
	a.push_back(make_pair("token",0));     // PG_TOK  b000001
	for (unsigned i = 0; i < a.size(); ++i) {
		fPg_setup.push_back(a[i]);
	}

	fApi->setPatternGenerator(fPg_setup);
 
	fApi->_dut->maskAllPixels(false);
	fApi->_dut->testAllPixels(false);

	for (int i=0;i<52;i++) {
		fApi->_dut->maskPixel(i,0,true);
		fApi->_dut->maskPixel(i,79,true);
	}
	for (int i=0;i<80;i++) {
		fApi->_dut->maskPixel(0,i,true);
		fApi->_dut->maskPixel(51,i,true);
	}
	TH1D *h;
	h = bookTH1D("PH distribution", "PH distribution", 256, 0, 256.0);
	TH1D *h_noise;
	h_noise = bookTH1D("PH distribution (noise)", "PH distribution (noise)", 256, 0, 256.0);
	TH1D *h_charge;
	h_charge = bookTH1D("PH distribution (charge)", "PH distribution (charge)", 200, 0, 400.0);
	TH1D *h_time;
	h_time = bookTH1D("nhits", "nhits", fParNtrig, 0, fParNtrig);
	TH1D *h_timecal;
	h_timecal = bookTH1D("nhits cal", "nhits cal", fParNtrig, 0, fParNtrig);
	TH2D *h_alive;
	h_alive = bookTH2D("XPixelAlive", "XPixelAlive", 52,0,52,80,0,80);
	TH2D *h_noisemap;
	h_noisemap = bookTH2D("background hits", "background hits", 52,0,52,80,0,80);

	vector<int> deadPixel(1, 0);
  vector<int> probPixel(1, 0);
  vector<int> xHits(1,0);
  vector<int> fidHits(1,0);
  vector<int> allHits(1,0);
  vector<int> fidPixels(1,0);

	fApi->daqSingleSignal("resetroc");
	fApi->daqStart();

	for (int co = 0; co<52; co++) {
		LOG(logINFO) << "col: " << co;
		for (int ro = 0; ro<80; ro++) {
			fApi->_dut->testPixel(co,ro,true);
			fApi->SetCalibrateBits(true);

			fApi->daqTrigger(fParNtrig, 10000);

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
		bool pixelSeen = false;
		for (unsigned int i=0;i<it->pixels.size();i++) {
			/*
			if (it->pixels[i].row() != 20 || it->pixels[i].column() != 11) {
				ss << "[";
			}
			ss << (int)it->pixels[i].column() << "," << (int)it->pixels[i].row() << ": " << it->pixels[i].value();
			if (it->pixels[i].row() != 20 || it->pixels[i].column() != 11) {
				ss << "]";
			}*/

			if (it->pixels[i].row() == ro && it->pixels[i].column() == co) {
				h->Fill(it->pixels[i].value());
				h_alive->Fill(co,ro,1);
				h_timecal->Fill(EventId%fParNtrig);
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
				h_noisemap->Fill(it->pixels[i].column(), it->pixels[i].row(), 1);
			}
			h_time->Fill(EventId%fParNtrig);
			//ss << " ";
		}

		if (!pixelSeen) {
				h->Fill(0);
		}
	//LOG(logINFO) << ss.str();
	EventId++;
	}
	
	for (int ix = 0; ix < h_alive->GetNbinsX(); ++ix) {
    for (int iy = 0; iy < h_alive->GetNbinsY(); ++iy) {
      allHits[0] += static_cast<int>(h_alive->GetBinContent(ix+1, iy+1));
  		if ((ix > 0) && (ix < 51) && (iy < 79) && (h_alive->GetBinContent(ix+1, iy+1) > 0)) {
    		fidHits[0] += static_cast<int>(h_alive->GetBinContent(ix+1, iy+1));
    		++fidPixels[0];
  		}
  		// -- count dead pixels
 		  if (h_alive->GetBinContent(ix+1, iy+1) < fParNtrig) {
    		++probPixel[0];
    		if (h_alive->GetBinContent(ix+1, iy+1) < 1) {
      		++deadPixel[0];
    		}
  		}
  		// -- Count X-ray hits detected
  		if (h_noisemap->GetBinContent(ix+1,iy+1)>0){
    		xHits[0] += static_cast<int> (h_noisemap->GetBinContent(ix+1,iy+1));
  		}
    }
  }




	

	fHistList.push_back(h_noise);
	fHistList.push_back(h_charge);
	fHistList.push_back(h_time);
	fHistList.push_back(h_timecal);
	fHistList.push_back(h);
	fHistList.push_back(h_noisemap);
	fHistList.push_back(h_alive);
	fHistOptions.insert(make_pair(h_noisemap, "colz"));
	fHistOptions.insert(make_pair(h_alive, "colz"));
	h_alive->Draw("COLZ");
	fDisplayedHist = find(fHistList.begin(), fHistList.end(), h);
	

	if (fProblem) {
		LOG(logINFO) << "PixTestXPixelAlive2::doTest() aborted because of problem ";
		return;
	}

	double sensorArea = 0.015 * 0.010 * 54 * 81; // in cm^2, accounting for larger edge pixels (J. Hoss 2014/10/21)
	string deadPixelString, probPixelString, xHitsString, numTrigsString,
	fidCalHitsString, allCalHitsString,
	fidCalEfficiencyString, allCalEfficiencyString,
	xRayRateString;
	for (unsigned int i = 0; i < probPixel.size(); ++i) {
		probPixelString += Form(" %4d", probPixel[i]);
		deadPixelString += Form(" %4d", deadPixel[i]);
		xHitsString     += Form(" %4d", xHits[i]);
		allCalHitsString += Form(" %4d", allHits[i]);
		fidCalHitsString += Form(" %4d", fidHits[i]);
		int numTrigs = fParNtrig * 4160;
		numTrigsString += Form(" %4d", numTrigs );
		fidCalEfficiencyString += Form(" %.1f", fidHits[i]/static_cast<double>(fidPixels[i]*fParNtrig)*100);
		allCalEfficiencyString += Form(" %.1f", allHits[i]/static_cast<double>(numTrigs)*100);
		xRayRateString += Form(" %.1f", xHits[i]/static_cast<double>(numTrigs)/25./sensorArea*1000.);
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