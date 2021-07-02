// -------------------------------------------------------------- -*- C++ -*-
// File: ttime.h
// --------------------------------------------------------------------------
//
// IBM Confidential
// OCO Source Materials
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2013
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
//
// ---------------------------------------------------------------------------

#ifndef __CP_ttimeH
#define __CP_ttimeH

#include <ilcp/cpext.h>
//#include <ilcp/ilcsdbtools.h>

int verifyTITT(IloNumToNumSegmentFunction fncij, IloNumToNumSegmentFunction fncik, IloNumToNumSegmentFunction fnckj, IloNum start, IloNum end);
int verifyFIFOTT(IloNumToNumSegmentFunction fnc, IloNumToNumSegmentFunction fncFIFO, IloNum start, IloNum end);
int veriryInverseTT(IloNumToNumSegmentFunction fnc, IloNumToNumSegmentFunction inverse, IloNum start, IloNum end);
  
class IloTDMatrixI {
public:
  IloTDMatrixI(IloEnv env) :_env(env) {}
  virtual ~IloTDMatrixI(){}
  //virtual IloInt getMin(IloInt i, IloInt j, IloInt tmin, IloInt tmax); returns min transition time from i to j between times tmin and tmax
  virtual IloInt getTransitionTimeNext(IloInt i, IloInt j, IloInt departureTime)=0;
  virtual IloInt getTransitionTimeSucc(IloInt i, IloInt j, IloInt departureTime)=0;
  virtual IloInt getInverseTransitionTimeNext(IloInt i, IloInt j, IloInt arrivalTime)=0;
  virtual IloInt getInverseTransitionTimeSucc(IloInt i, IloInt j, IloInt arrivalTime)=0;
  virtual IloInt getCostMax()=0;
  virtual IloInt getMinFrom(IloInt i, IloInt u)=0;
  virtual IloInt getMinTo(IloInt i, IloInt u)=0;
  IloEnv getEnv() const { return _env; }
private:
  IloEnv _env;
};

class IloTD3DMatrixI :public IloTDMatrixI {
public:
  IloTD3DMatrixI(IloEnv env, IloIntArray3 costs, IloInt sizeTS);
  virtual ~IloTD3DMatrixI(){}
  virtual IloInt getTransitionTimeNext(IloInt i, IloInt j, IloInt departureTime);
  virtual IloInt getTransitionTimeSucc(IloInt i, IloInt j, IloInt departureTime);
  virtual IloInt getInverseTransitionTimeNext(IloInt i, IloInt j, IloInt arrivalTime);
  virtual IloInt getInverseTransitionTimeSucc(IloInt i, IloInt j, IloInt arrivalTime);
  virtual IloInt getCostMax(){ return _costMax;}
  virtual IloInt getMinFrom(IloInt i, IloInt u){ return _costsMinFrom[u][i];}
  virtual IloInt getMinTo  (IloInt i, IloInt u){ return _costsMinTo[u][i];}
private:
  IloInt    _nbNodes;
  IloInt    _nbTimeSteps;
  IloInt    _sizeTimeStep;
  IloInt    _costMax;
  IloInt*   _costsMinFrom[2]; // if(unique) _costsMinFrom[0] else _costsMinFrom[1] 
  IloInt*   _costsMinTo  [2];  
  IloInt*** _costs;
};

class IloTDSegmentFunctionMatrix: public IloTDMatrixI {
public:
  IloTDSegmentFunctionMatrix(IloEnv env, IloIntArray3 costs, IloInt sizeTS);
  IloTDSegmentFunctionMatrix(IloEnv env, IloNumToNumSegmentFunction** costs, IloInt nbVertices, IloNum Horizon);
  
  virtual ~IloTDSegmentFunctionMatrix(){}
  virtual IloInt getTransitionTimeNext(IloInt i, IloInt j, IloInt departureTime);
  virtual IloInt getTransitionTimeSucc(IloInt i, IloInt j, IloInt departureTime);
  virtual IloInt getInverseTransitionTimeNext(IloInt i, IloInt j, IloInt arrivalTime);
  virtual IloInt getInverseTransitionTimeSucc(IloInt i, IloInt j, IloInt arrivalTime);
  virtual IloInt getMinFrom(IloInt i, IloInt u){ return _costsMinFrom[u][i];}
  virtual IloInt getMinTo(IloInt i, IloInt u){ return _costsMinTo[u][i];}
  virtual IloInt getCostMax(){ return _costMax;}
  void FIFOTT(IloInt i, IloInt j, IloNumToNumSegmentFunction* res, const IloNumToNumSegmentFunction costs);
  void InverseTT(IloInt i, IloInt j, IloNumToNumSegmentFunction* res, const IloNumToNumSegmentFunction costs);
  void minTT(IloInt k, IloInt i, IloInt j);
  void compareSuccNext();
  void printFIFOTIbench();
  
private:
  IloNum  _start;
  IloNum  _horizon;
  IloInt  _nbNodes;
  IloInt  _nbTimeSteps;
  IloInt  _sizeTimeStep;
  IloInt  _costMax; 
  IloInt* _costsMinFrom[2]; // if(unique) _costsMinFrom[0] else _costsMinFrom[1] 
  IloInt* _costsMinTo  [2];
  IloInt** _min;
  IloInt** _max;
  IloNumToNumSegmentFunction** _costs;
  IloNumToNumSegmentFunction** _nextEarliestTT; //_directMinCosts;
  IloNumToNumSegmentFunction** _nextLatestTT;   //_inverseMinCosts;
  IloNumToNumSegmentFunction** _succEarliestTT;
  IloNumToNumSegmentFunction** _succLatestTT;
};


#endif
