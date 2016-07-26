#ifndef PixTest1Pixel_H
#define PixTest1Pixel_H

#include "PixTest.hh"
#include "PHCalibration.hh"

using std::vector;

class DLLEXPORT PixTest1Pixel: public PixTest {
public:
  PixTest1Pixel(PixSetup *, std::string);
  PixTest1Pixel();
  virtual ~PixTest1Pixel();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 


  void doTest();
  
private:

  int fParVcal, fParNtrig; 
  int fParNSteps;

  bool          fPhCalOK;
  PHCalibration fPhCal;

  ClassDef(PixTest1Pixel, 1)

};
#endif
