#ifndef PixTestXPixelAlive2_H
#define PixTestXPixelAlive2_H

#include "PixTest.hh"
#include "PHCalibration.hh"

using std::vector;

class DLLEXPORT PixTestXPixelAlive2: public PixTest {
public:
  PixTestXPixelAlive2(PixSetup *, std::string);
  PixTestXPixelAlive2();
  virtual ~PixTestXPixelAlive2();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 


  void doTest();
  
private:

  int fParVcal, fParNtrig, fParReset;
  int fParNSteps;

  bool          fPhCalOK;
  PHCalibration fPhCal;

  ClassDef(PixTestXPixelAlive2, 1)

};
#endif
