#ifndef PIXTESTXSCURVES_H
#define PIXTESTXSCURVES_H

#include "PixTest.hh"

using std::vector;

class DLLEXPORT PixTestXSCurves: public PixTest {
public:
  PixTestXSCurves(PixSetup *, std::string);
  PixTestXSCurves();
  virtual ~PixTestXSCurves();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 


  void doTest();
  
private:

  int fParVcal, fParNtrig; 
  int fParNSteps;


  ClassDef(PixTestXSCurves, 1)

};
#endif