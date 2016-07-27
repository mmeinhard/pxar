#include <stdlib.h>     
#include <algorithm>    
#include <iostream>
#include <fstream>

#include <TH1.h>
#include <TGraphErrors.h>
#include <TRandom.h>
#include <TStopwatch.h>
#include <TStyle.h>

#include "PixTestXSCurves.hh"
#include "PixUtil.hh"
#include "log.h"
#include "helper.h"

using namespace std;
using namespace pxar;

ClassImp(PixTestXSCurves)


/*
-- XSCurves
vcal                200
ntrig               10
*/


// ----------------------------------------------------------------------
PixTestXSCurves::PixTestXSCurves(PixSetup *a, std::string name) : PixTest(a, name), fParNtrig(1),fParNSteps(20) {
  PixTest::init();
  init(); 

 
  //  LOG(logINFO) << "PixTestXSCurves ctor(PixSetup &a, string, TGTab *)";
}


//----------------------------------------------------------
PixTestXSCurves::PixTestXSCurves() : PixTest() {
  //  LOG(logINFO) << "PixTestXSCurves ctor()";
}

// ----------------------------------------------------------------------
bool PixTestXSCurves::setParameter(string parName, string sval) {
  bool found(false);
  string str1, str2; 
  string::size_type s1;
  int pixc, pixr; 
  fApi->setDAC("CtrlReg", 8);
  std::transform(parName.begin(), parName.end(), parName.begin(), ::tolower);
  for (unsigned int i = 0; i < fParameters.size(); ++i) {
    if (fParameters[i].first == parName) {
      found = true; 
      if (!parName.compare("ntrig")) {
	fParNtrig = atoi(sval.c_str()); 
      }
      if (!parName.compare("nsteps")) {
	fParNSteps = atoi(sval.c_str()); 
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
void PixTestXSCurves::init() {
  setToolTips(); 
  fDirectory = gFile->GetDirectory(fName.c_str()); 
  if (!fDirectory) {
    fDirectory = gFile->mkdir(fName.c_str()); 
  } 
  fDirectory->cd(); 

}



// ----------------------------------------------------------------------
void PixTestXSCurves::setToolTips() {
  fTestTip    = string(Form("send n number of calibrates to one pixel\n")
		       + string("TO BE FINISHED!!"))
    ;
  fSummaryTip = string("summary plot to be implemented")
    ;
}

// ----------------------------------------------------------------------
void PixTestXSCurves::bookHist(string name) {
  fDirectory->cd(); 

  LOG(logDEBUG) << "nothing done with " << name;
}


//----------------------------------------------------------
PixTestXSCurves::~PixTestXSCurves() {
  LOG(logDEBUG) << "PixTestXSCurves dtor";
}


// ----------------------------------------------------------------------
void PixTestXSCurves::doTest() {

  TStopwatch t;

  fDirectory->cd();
  PixTest::update(); 
  bigBanner(Form("PixTestXSCurves::doTest()"));

  fProblem = false;
  gStyle->SetPalette(1);
  bool verbose(false);
  cacheDacs(verbose);
  fDirectory->cd();
  PixTest::update(); 

  vector<pair<string, uint8_t> > a;
  
  uint8_t wbc = 100;
  uint8_t delay = 5;

  fApi->flushTestboard();
  fPg_setup.clear();
  a.push_back(make_pair("resetroc",25));    // PG_RESR b001000 
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

	for (int dac = 0; dac<200; dac+=(200/fParNSteps)) {
			fApi->setDAC("Vcal", dac);
			fApi->daqTrigger(fParNtrig, 10000);
	}

	fApi->_dut->testPixel(fPIX[0].first,fPIX[0].second,false);
	fApi->daqStop();

  vector<pxar::Event> daqdat;
	try { 
		daqdat = fApi->daqGetEventBuffer(); 
	} catch(pxar::DataNoEvent &) {
		LOG(logERROR) << "no data";
	}

	int EventId=0;

	TH1D *h_countsvcal;
	h_countsvcal = bookTH1D("counts", "counts", fParNSteps, 0, 200);
	TH1D *h_countsvolts;
	h_countsvolts = bookTH1D("counts volts", "counts volts", 3072, 0, 0.3072);
	TH2D *h_noisemap;
	h_noisemap = bookTH2D("background hits", "background hits", 52,0,52,80,0,80);



	for (std::vector<pxar::Event>::iterator it = daqdat.begin(); it != daqdat.end(); ++it) {
		int vcal = (EventId/fParNtrig)%fParNSteps;
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

			if (it->pixels[i].row() == fPIX[0].second && it->pixels[i].column() == fPIX[0].first) {
				h_countsvcal->Fill(vcal*200./fParNSteps,1);
				pixelSeen = true;
				
			} else {
				h_noisemap->Fill(it->pixels[i].column(), it->pixels[i].row(), 1);
			}
			//ss << " ";
		}

	//LOG(logINFO) << ss.str();
	EventId++;
	}

	


	double VcalVoltageCalibrationLowRange[200] = 
	{0.0000, 0.0015, 0.0028, 0.0042, 0.0053, 0.0053, 0.0080, 0.0094, 0.0089, 0.0103, 
		0.0116, 0.0130, 0.0141, 0.0156, 0.0169, 0.0183, 0.0205, 0.0219, 0.0232, 0.0246, 
		0.0256, 0.0269, 0.0283, 0.0296, 0.0292, 0.0305, 0.0319, 0.0332, 0.0343, 0.0357, 
		0.0371, 0.0384, 0.0404, 0.0417, 0.0430, 0.0444, 0.0454, 0.0467, 0.0481, 0.0494, 
		0.0489, 0.0503, 0.0516, 0.0530, 0.0541, 0.0555, 0.0568, 0.0582, 0.0604, 0.0617, 
		0.0630, 0.0643, 0.0653, 0.0667, 0.0680, 0.0693, 0.0688, 0.0702, 0.0715, 0.0728, 
		0.0739, 0.0753, 0.0766, 0.0780, 0.0805, 0.0818, 0.0831, 0.0844, 0.0854, 0.0867, 
		0.0880, 0.0894, 0.0889, 0.0902, 0.0915, 0.0929, 0.0940, 0.0953, 0.0967, 0.0967, 
		0.1002, 0.1015, 0.1028, 0.1041, 0.1051, 0.1064, 0.1077, 0.1090, 0.1085, 0.1099, 
		0.1112, 0.1125, 0.1136, 0.1149, 0.1163, 0.1176, 0.1196, 0.1209, 0.1221, 0.1235, 
		0.1244, 0.1257, 0.1270, 0.1283, 0.1278, 0.1291, 0.1304, 0.1318, 0.1329, 0.1342, 
		0.1355, 0.1369, 0.1391, 0.1404, 0.1417, 0.1430, 0.1439, 0.1452, 0.1465, 0.1478, 
		0.1472, 0.1486, 0.1499, 0.1512, 0.1523, 0.1536, 0.1550, 0.1563, 0.1569, 0.1582, 
		0.1594, 0.1607, 0.1616, 0.1629, 0.1642, 0.1655, 0.1649, 0.1662, 0.1675, 0.1689, 
		0.1700, 0.1713, 0.1726, 0.1740, 0.1762, 0.1775, 0.1787, 0.1800, 0.1809, 0.1822, 
		0.1834, 0.1848, 0.1841, 0.1854, 0.1867, 0.1881, 0.1891, 0.1905, 0.1918, 0.1933, 
		0.1952, 0.1965, 0.1977, 0.1991, 0.2001, 0.2011, 0.2021, 0.2031, 0.2031, 0.2041, 
		0.2051, 0.2071, 0.2081, 0.2091, 0.2111, 0.2121, 0.2141, 0.2161, 0.2171, 0.2181, 
		0.2191, 0.2201, 0.2211, 0.2231, 0.2221, 0.2231, 0.2241, 0.2261, 0.2271, 0.2281, 
		0.2301, 0.2311, 0.2341, 0.2341, 0.2361, 0.2371, 0.2381, 0.2391, 0.2411, 0.2421,};
  //double VcalVoltageCalibrationHighRange[] = {0.0000, 0.0093, 0.0183, 0.0274, 0.0344, 0.0436, 0.0525, 0.0616, 0.0586, 0.0678, 0.0767, 0.0859, 0.0933, 0.1025, 0.1114, 0.1207, 0.1355, 0.1444, 0.1531, 0.1620, 0.1689, 0.1689, 0.1865, 0.1955, 0.1924, 0.2017, 0.2097, 0.2197, 0.2267, 0.2357, 0.2447, 0.2537, 0.2667, 0.2757, 0.2837, 0.2927, 0.2997, 0.3087, 0.3167, 0.3257, 0.3227, 0.3317, 0.3407, 0.3497, 0.3567, 0.3657, 0.3747, 0.3837, 0.3977, 0.4067, 0.4157, 0.4237, 0.4307, 0.4397, 0.4487, 0.4567, 0.4537, 0.4627, 0.4717, 0.4807, 0.4877, 0.4967, 0.5047, 0.5137, 0.5307, 0.5397, 0.5477, 0.5567, 0.5627, 0.5717, 0.5797, 0.5887, 0.5857, 0.5947, 0.6027, 0.6117, 0.6187, 0.6277, 0.6367, 0.6457, 0.6597, 0.6687, 0.6767, 0.6857, 0.6917, 0.7007, 0.7087, 0.7177, 0.7147, 0.7227, 0.7317, 0.7397, 0.7477, 0.7557, 0.7647, 0.7737, 0.7867, 0.7947, 0.8027, 0.8117, 0.8177, 0.8267, 0.8347, 0.8427, 0.8397, 0.8487, 0.8567, 0.8657, 0.8727, 0.8817, 0.8897, 0.8897, 0.9137, 0.9217, 0.9297, 0.9377, 0.9447, 0.9527, 0.9607, 0.9697, 0.9657, 0.9747, 0.9827, 0.9917, 0.9987, 1.0067, 1.0157, 1.0247, 1.0277, 1.0367, 1.0447, 1.0527, 1.0587, 1.0667, 1.0747, 1.0837, 1.0797, 1.0877, 1.0967, 1.1047, 1.1117, 1.1207, 1.1297, 1.1377, 1.1527, 1.1607, 1.1687, 1.1767, 1.1817, 1.1907, 1.1977, 1.2067, 1.2027, 1.2107, 1.2187, 1.2277, 1.2347, 1.2427, 1.2517, 1.2607, 1.2727, 1.2807, 1.2887, 1.2967, 1.3017, 1.3097, 1.3177, 1.3257, 1.3207, 1.3297, 1.3377, 1.3457, 1.3527, 1.3607, 1.3697, 1.3787, 1.3917, 1.3997, 1.4067, 1.4147, 1.4187, 1.4267, 1.4337, 1.4417, 1.4367, 1.4447, 1.4527, 1.4597, 1.4657, 1.4737, 1.4817, 1.4897, 1.5027, 1.5097, 1.5147, 1.5207, 1.5237, 1.5297, 1.5347, 1.5407, 1.5367, 1.5427, 1.5477, 1.5537, 1.5567, 1.5627, 1.5677, 1.5727, 1.5797, 1.5837, 1.5867, 1.5897, 1.5917, 1.5947, 1.5977, 1.6017, 1.5987, 1.6027, 1.6057, 1.6087, 1.6107, 1.6147, 1.6177, 1.6177, 1.6247, 1.6267, 1.6287, 1.6307, 1.6317, 1.6337, 1.6357, 1.6377, 1.6367, 1.6387, 1.6407, 1.6427, 1.6447, 1.6467, 1.6487, 1.6517, 1.6547, 1.6567, 1.6577, 1.6587, 1.6597, 1.6607, 1.6627, 1.6637, 1.6627, 1.6647, 1.6657, 1.6677, 1.6687, 1.6707, 1.6717, 1.6737};

	double values[200] = {0};
 	for (int k=0; k<200;k++) {
		h_countsvolts->Fill(VcalVoltageCalibrationLowRange[k],h_countsvcal->GetBinContent(k));
 		values[k] = h_countsvcal->GetBinContent(k);
	}

	TGraphErrors *volts = new TGraphErrors(200,VcalVoltageCalibrationLowRange,values);
  

  TF1 *scurveFit = new TF1("Fit","[0]*TMath::Erf([2] * (x-[1])) + [3]",0,.3);
  scurveFit->SetParameters(fParNtrig/2.,0.14,1/0.02,fParNtrig/2.);
  volts->Fit(scurveFit,"Q", "", 0.0, 0.3);
  //LOG(logINFO) << "Par0: " << scurveFit->GetParameter(0);

  float slope = VcalVoltageCalibrationLowRange[199]/200;

  double thr = scurveFit->GetParameter(1) * 50. / slope;  // conversion Vcal voltage -> Vcal DACs -> electrons
  double sig = 1. / (sqrt(2.) * scurveFit->GetParameter(2)) * 50. / slope;  // conversion Vcal voltage -> Vcal DACs -> electrons	

  LOG(logINFO) << "Threshold: " << thr << "e = " << thr/50 << "Vcal, Noise: " << sig << "e";

  fHistList.push_back(h_noisemap);
  fHistList.push_back(h_countsvcal);
  fHistList.push_back(h_countsvolts);
  fHistOptions.insert(make_pair(h_noisemap, "colz"));
  h_countsvcal->Draw();
  fDisplayedHist = find(fHistList.begin(), fHistList.end(), h_noisemap);
  

  if (fProblem) {
    LOG(logINFO) << "PixTestXSCurves::doTest() aborted because of problem ";
    return;
  }

  int seconds = t.RealTime(); 
  LOG(logINFO) << "PixTestXSCurves::doTest() done, duration: " << seconds << " seconds";
  LOG(logINFO) << "number of triggers sent: " << fParNtrig; 
  //LOG(logINFO) << "number of hits detected:  " << nhits; 
  //LOG(logINFO) << "hits missed: ";

  PixTest::update(); 
}