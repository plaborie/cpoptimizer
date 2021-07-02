// -------------------------------------------------------------- -*- C++ -*-
// File: tdtsp_tdnooverlap.cpp
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


#include <ilcp/cp.h>

IloInt TIME_LIMIT = 900;
IloBool DEBUG= false;
ILOSTLBEGIN
IloBool REAL_INDEX = true;


class TDTSPInstance {
public:
	// TDTSPInstance(const char* nameInstance);
	TDTSPInstance(const char* nameBench, const char* nameInstance);
  void solve();
  void print(const char* nameBench, const char* nameInstance);

  
private:
  IloInt     _nbVertices;
	IloInt     _nbTimeWindows;
	IloInt     _lengthTimeWindow;
  IloInt**    _windows;
	IloInt***  _cost;    // nbVertices*nbVertices*nbTimeWindows
  IloInt**   _minCost; // nbVertices*nbVertices
  IloInt     _maxCost;
	IloInt*    _duration;
	IloInt*    _locations;
	IloInt     _horizon;
  IloInt*     _visits;
};


TDTSPInstance::TDTSPInstance(const char* nameBench, const char* nameInstance) 
  :_nbVertices       (0)
	,_nbTimeWindows    (0)
	,_lengthTimeWindow (0)
	,_windows          (0)
	,_cost             (0)
	,_minCost          (0)
	,_maxCost          (0)
	,_duration         (0)
	,_locations        (0)
	,_horizon          (0)
  ,_visits           (0)
{
  ifstream input(nameInstance);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << nameInstance << "'!" << endl;
  }
  ifstream input_bench(nameBench);
  if (!input_bench.is_open()) {
    std::cout << "Can't open file '" << nameBench << "'!" << endl;
  }
  
  
  IloInt nbWindows=0;
  IloInt nbPrec=0;
  IloInt indexMax=0;
  IloInt i,j,k;
  input >> _nbVertices;
  input >> nbPrec;
  _visits= new IloInt[_nbVertices];
  _duration= new IloInt[_nbVertices];
  _windows= new IloInt*[_nbVertices];
  for(i=0; i<_nbVertices; i++){
    input >> _visits[i];
    if(_visits[i] > indexMax) indexMax= _visits[i];
    input >> _duration[i];
    input >> nbWindows;
    if(nbWindows!=0){
      _windows[i]= new IloInt[2*nbWindows];
      for(j=0; j<2*nbWindows; j++){
        input >> _windows[i][j];
      }
    } 
  }
  
  
  
  // for(IloInt i=0; i<nbPrec; i++){
  // }
  IloInt benchSize;
  input_bench >> benchSize;
  input_bench >> _nbTimeWindows;
  input_bench >> _lengthTimeWindow;
    
  if(indexMax > benchSize) std::cerr << "Not enough addresses in bench file" << std::endl; 
  
  IloInt*** cost    = new IloInt**[benchSize];
  for(i=0; i<benchSize; i++){
    cost   [i] = new IloInt*[benchSize];
    for(j=0; j<benchSize; j++) {
      cost[i][j] = new IloInt[_nbTimeWindows];
      for (k=0; k<_nbTimeWindows; ++k)
        input_bench >> cost[i][j][k];
    }
  }
  input.close();
  input_bench.close();
  
  IloInt mincost = IloIntervalMax;
  IloInt maxcost = 0;
  IloInt v;
  _cost    = new IloInt**[_nbVertices];
  _minCost = new IloInt* [_nbVertices];
  for(i=0; i<_nbVertices; i++){
    _cost   [i] = new IloInt*[_nbVertices];
    _minCost[i] = new IloInt [_nbVertices];
    for(j=0; j<_nbVertices; j++) {
      _cost[i][j] = new IloInt[_nbTimeWindows];
      for (k=0; k<_nbTimeWindows; ++k){
        v = cost[_visits[i]][_visits[j]][k];
        _cost[i][j][k]= v;
        if (v > maxcost && i!=j) {
          maxcost = v;
        }
        if (v < mincost && i!=j) {
          mincost = v;
        }
      }
      _minCost[i][j]=((i==j)?0:mincost);
      mincost= IloIntervalMax;
    }
  }
  _maxCost= maxcost;
  _horizon = 10*_nbVertices*_maxCost;
  _locations = new IloInt[_nbVertices+1];
  for(i=0; i < _nbVertices; i++) {
    _locations[i]=i;
  }
  _locations[_nbVertices]= 0;
  
  // for(i=0; i<_nbVertices; i++){
    // for(j=0; j<_nbVertices; j++) {
      // for (k=0; k<_nbTimeWindows; ++k){
        // std::cout << _cost[i][j][k] << " ";
      // }
      // std::cout << std::endl;
    // }
  // }
}


void TDTSPInstance::solve() {
  IloEnv env;
  try {
    IloModel model(env);
    IloIntArray locationsArray(env, _nbVertices+1);
    for(IloInt i=0; i < _nbVertices+1; i++)
      locationsArray[i]=_locations[i];

    IloIntArray3 costMatrix(env, _nbVertices);
    IloIntArray2 costMatrix2(env, _nbVertices);
    IloIntervalVarArray stops(env,_nbVertices+1);
    IloIntVarArray travelTime(env, _nbVertices);
    IloTransitionDistance minCostMatrix(env, _nbVertices);
    IloIntExprArray timeWindow(env, _nbVertices);
    for (IloInt i=0; i<_nbVertices; ++i) {
      for (IloInt j=0; j<_nbVertices; ++j) {
        if (i!=j) {
          minCostMatrix.setValue(i, j, _minCost[i][j]);
        }
      }
    }
    IloIntVar totalTravelTime(env);
    totalTravelTime.setName("totalTT");
    char name[64];
    IloInt totalDuration=0;
    for(IloInt i=0; i< _nbVertices; i++){ 
      totalDuration+= _duration[i];
      stops[i] = IloIntervalVar(env, _duration[i]);
      sprintf(name, "%ld", i);
      stops[i].setName(name);
      stops[i].setEndMax(_horizon);
      travelTime[i] = IloIntVar(env, 0, _maxCost);
      sprintf(name, "TT%ld", i);
      travelTime[i].setName(name);
      timeWindow[i] = IloMin(IloDiv(IloEndOf(stops[i]),_lengthTimeWindow), _nbTimeWindows-1);
      costMatrix[i] = IloIntArray2(env, _nbVertices);
      costMatrix2[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      for(IloInt j=0; j<_nbVertices; j++){
        costMatrix[i][j] = IloIntArray(env, _nbTimeWindows);
        for(IloInt k=0; k< _nbTimeWindows; k++){
          costMatrix[i][j][k]=_cost[i][j][k];
          costMatrix2[i][k+j*_nbTimeWindows]=_cost[i][j][k];
        }
      }
    }
    // totalDuration+= _duration[0];
    stops[_nbVertices] = IloIntervalVar(env,_duration[0]);
    sprintf(name, "%ld", _nbVertices);
    stops[_nbVertices].setName(name);
    stops[_nbVertices].setEndMax(_horizon);
    IloIntervalSequenceVar tour(env, stops, locationsArray);
    tour.setName("Tour");  
     
//---------------------------- CONSTRAINTS

    
    model.add(IloFirst(env,tour, stops[0]));
    model.add(IloLast(env,tour, stops[_nbVertices]));
    model.add(IloStartOf(stops[0]) == 0);
    //model.add(totalTravelTime== IloSum(travelTime));
    model.add(totalTravelTime== 0);
    // for(IloInt i=0; i<_nbVertices; i++){
      // IloIntExpr indexInCostMatrix = IloTypeOfNext(tour, stops[i], 1)*_nbTimeWindows + timeWindow[i];
      // model.add(travelTime[i] == costMatrix2[i][indexInCostMatrix]);
      // model.add(IloStartOfNext(tour, stops[i], _horizon) == IloEndOf(stops[i]) + travelTime[i]);
    // }
    model.add(IloNoOverlap(env, tour, minCostMatrix, IloTrue));
    for(IloInt i=0; i<_nbVertices; i++){
      IloIntExpr indexInCostMatrix = IloTypeOfNext(tour, stops[i], 0)*_nbTimeWindows + timeWindow[i];
      model.add(travelTime[i] == costMatrix2[i][indexInCostMatrix]);
      model.add(IloStartOfNext(tour, stops[i], _horizon) >= IloEndOf(stops[i]) + travelTime[i]);
    }
    
//------------------------------ OBJECTIVE
   IloIntExpr obj= IloStartOf(stops[_nbVertices]);//- totalDuration;
   // model.add(obj < 3305);
   model.add(IloMinimize(env, obj));
   //model.add(IloMinimize(env, totalTravelTime));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);  
    
    IloCP cp(model);

//-------------------------------PARAMETERS------------------------------------
    
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::RelativeOptimalityTolerance, 1e-9);
    // cp.setParameter(IloCP::RandomSeed, 1);
    // cp.setParameter(IloCP::RandomSeed, 2);
    // cp.setParameter(IloCP::IntParam::IntervalSequenceInferenceLevel, IloCP::Extended);
    // cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    // if (cp.refineConflict()) {
      // cp.writeConflict(cp.out());
    // } 

//---------------------------SET STARTING POINT
/*
    // IloSolution sol(env);
    ifstream input("tdtsp_100_0_TDNOOVERLAP.log");
    IloInt counter=0,pvisit,visit, tt;
    if (!input.is_open()) {
      std::cerr << "Can't open file '" << endl;
    }
    pvisit=0;
    for(IloInt i=0; i<=_nbVertices+1; i++){
      input >> visit;
      input >> tt;
      counter+= tt + 1;
      if (pvisit!=visit) model.add(IloEndBeforeStart(env, stops[pvisit], stops[visit]));
      // stops[visit].setStartMin(counter);
      // stops[visit].setStartMax(counter);
      pvisit=visit;
    }
    input.close();  
    // cp.setStartingPoint(sol);
*/

   
//-------------------------------SOLVE-----------------------------------------  
    cp.startNewSearch(IloSearchPhase(env, tours));
    IloBool solutionFound=IloFalse;
    // while(cp.next()){
      // solutionFound=IloTrue;
      // cout << "* " << cp.getInfo(IloCP::SolveTime) << " " << cp.getObjValue() << endl;
    // }
    
    // We hack here a little bit for now to get rid of the large amount of time spent at extraction for checking TI
    IloNum t0, t1;
    if (cp.next()) {
      solutionFound=IloTrue;
      t0 = cp.getInfo(IloCP::SolveTime);
      t1 = t0- cp.getInfo(IloCP::ExtractionTime);
      cout << "* " << t1 << " " << cp.getObjValue() << endl;
      cp.setParameter(IloCP::TimeLimit, TIME_LIMIT+t0-t1);
      while (cp.next()){
        cout << "* " << cp.getInfo(IloCP::SolveTime)-t0+t1 << " " << cp.getObjValue() << endl;
      }
    }
    cp.endSearch();
    
    if (!solutionFound) {
      cout << "# NO SOLUTION FOUND" << endl;
    } 
    else {
      cout << "# SOLUTION FOUND" << endl;
    }
    if(DEBUG) {
      cout <<  cp.getInfo(IloCP::NumberOfFails) << endl;
    }
    else{
      if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit || cp.getInfo(IloCP::SolveTime) >= TIME_LIMIT) {
        cout << "# OPTIMALITY NOT PROVED" << endl;
      } else {
        cout << "# OPTIMALITY PROVED" << endl;
      }
      cout << "# BEST SOLUTION: " << cp.getObjValue() << endl;
      cout << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << endl;
      cout << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << endl;
      cout << "## Solution: " << endl;
      cout << _nbVertices << endl;
      IloInt s1=0, counter=0; 
      for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
        s1=cp.getStart(a);
        counter++;
        if(counter== _nbVertices+1)
           cout <<"# BEST TD SOLUTION: " << s1 << std::endl;
        else if(REAL_INDEX)
          cout << _visits[atoi(a.getName())] << " " << s1 << std::endl;
        else
          cout << a.getName() << " " << s1 << std::endl;
      }
      // CPChecker(cp, tour);
    }
    
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
void TDTSPInstance::print(const char* nameBench, const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << " " << nameBench << endl;
    cout << "# ALGORITHM: TDNOOVERLAP RESTART" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
}





  
int main(int argc, char* argv[]) {
		TDTSPInstance tdtsp(argv[1], argv[2]); 
    tdtsp.print(argv[1],argv[2]);
    tdtsp.solve();
}


