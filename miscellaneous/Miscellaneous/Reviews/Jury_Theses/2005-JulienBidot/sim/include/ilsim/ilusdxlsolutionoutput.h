// --------------------------------------------------------------------------
// Copyright (C) 1999-2004 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////////
//
// SDXL exporter
//
/////////////////////////////////////////////////////////////////////////

#ifndef __SIM_ilusdxlsolutionoutputsimH
#define __SIM_ilusdxlsolutionoutputsimH

#include<ilsim/ilusdxloutput.h>
#include<ilsim/iluprocessplan.h>



class IloSDXLSchedSolOutput : public  IloSDXLOutput {
 protected:
  void writeProcPlans(IloSchedulerSolution solution,
					  IloArray<IloActivityArray> actArrays,
					  IloNum ratio);
  void writeProcPlan(IluProcPlan* procPlan, IloSchedulerSolution solution, IloNum ratio);
 public:
  IloSDXLSchedSolOutput(IloEnv env);
  void writeSchedSol(IloSchedulerSolution solution,
					 ILCSTD(ofstream)& s,
					 IloArray<IloActivityArray> actArrays,
					 IloNum ratio,
					 const char *title=0);
};

#endif