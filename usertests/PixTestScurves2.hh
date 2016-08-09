#ifndef PIXTESTSCURVES2_H
#define PIXTESTSCURVES2_H

#include "PixTest.hh"

class DLLEXPORT PixTestScurves2: public PixTest {
public:
  PixTestScurves2(PixSetup *, std::string);
  PixTestScurves2();
  virtual ~PixTestScurves2();
  virtual bool setParameter(std::string parName, std::string sval); 
  void init(); 
  void setToolTips();
  void bookHist(std::string); 

  void runCommand(std::string command); 
  void thrMap(); 
  void fitS(); 
  void scurves(); 
  void adjustVcal();

  void doTest(); 
  void fullTest(); 

private:

  std::string fParDac;
  int         fParNtrig, fParDacLo, fParDacHi, fParDacsPerStep, fParNtrigPerStep, fAdjustVcal, fDumpAll, fDumpProblematic, fDumpOutputFile, fParVwllPrLow,fParVwllPrHigh,fParVwllPrStep,fParVwllShLow,fParVwllShHigh,fParVwllShStep;

  ClassDef(PixTestScurves2, 1)

};
#endif
