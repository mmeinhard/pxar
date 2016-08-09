#ifndef PIXTESTNOISETHRESHOLD_H
#define PIXTESTNOISETHRESHOLD_H

#include "PixTest.hh"

class DLLEXPORT PixTestNoiseThreshold: public PixTest {
public:
  PixTestNoiseThreshold(PixSetup *, std::string);
  PixTestNoiseThreshold();
  virtual ~PixTestNoiseThreshold();
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
  int         fParNtrig, fParDacLo, fParDacHi, fParDacsPerStep, fParNtrigPerStep, fAdjustVcal, fDumpAll, fDumpProblematic, fDumpOutputFile, fParVthrCompLow,fParVthrCompHigh,fParVthrCompStep;

  ClassDef(PixTestNoiseThreshold, 1)

};
#endif
