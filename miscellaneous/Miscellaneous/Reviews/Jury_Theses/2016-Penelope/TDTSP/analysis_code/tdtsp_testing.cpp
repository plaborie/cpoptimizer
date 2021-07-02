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

#include <../team/TDTSP/src/ttime.cpp>

#include <../team/TDTSP/src/tdnooverlapct.cpp>
IloInt TIME_LIMIT = 1800;
IloBool DEBUG= false;
ILOSTLBEGIN
IloBool REAL_INDEX = true;


class TDTSPInstance {
public:
	// TDTSPInstance(const char* nameInstance);
	TDTSPInstance(const char* nameBench, const char* nameInstance);
  IloBool solve();
  void print(const char* nameInstance);
  void fileChecker(const char* nameSol);
  void CPChecker(IloCP cp, IloIntervalSequenceVar tour);
  
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

void TDTSPInstance::fileChecker(const char* nameSol){
  ifstream input(nameSol);
  if (!input.is_open()) {
    std::cerr << "Can't open file '" << endl;
  }
  else 
    std::cout << "Starting to check" << endl;
  IloInt counter=0,pvisit,visit, tt, totalTT=_duration[0], possTT, possT;
  pvisit=0;
  for(IloInt i=0; i<_nbVertices+1; i++){
    input >> visit;
    input >> tt;
    if(visit== _nbVertices) visit =0;
    possTT= _cost[pvisit][visit][totalTT/_lengthTimeWindow];
    possT= totalTT;
    if (pvisit!=visit){ 
      if(tt != possTT){
        for(IloInt t=totalTT; t <= counter+ tt; t++){
          if( _cost[pvisit][visit][t/_lengthTimeWindow] + t - totalTT < possTT){
             possTT= _cost[pvisit][visit][t/_lengthTimeWindow] + t - totalTT;
             possT= t;
          }
        }
      }
      if(possTT < tt ){
        std::cout << "Not optimal tt "<<tt <<" between: " << pvisit << " and " << visit << " at time " << counter << endl;
        std:: cout << "Optimal is: " << possTT << " taken at " << possT << endl;
        // std:: cout << "Next possTT: " << _cost[pvisit][visit][counter/_lengthTimeWindow + 1] << endl;
      }
      else if(possTT > tt)
        std::cout << "Not possible tt" << endl;
    }
    totalTT+= possTT + _duration[0];
    counter+= tt + _duration[0];
    pvisit=visit;
  }
  std::cout << "End of checker" << endl;
  std::cout << "Total tt: " << totalTT - _nbVertices*_duration[0] << endl;
  std::cout << "Counter tt: " << counter << endl;
    
  input.close();  
  
}

void TDTSPInstance::CPChecker(IloCP cp, IloIntervalSequenceVar tour){
  IloInt counter=_duration[0],pvisit,visit, tt, totalTT=0, possTT, possT;
  pvisit=0;
  IloInt s1=0, s2;
  std::cout << "Starting to check" << endl;
  for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){
    s2=cp.getStart(a);
    visit= (IloInt) atoi(a.getName());
    tt=(s2- s1 -1);
    if(visit== _nbVertices) visit =0;
    possTT= _cost[pvisit][visit][counter/_lengthTimeWindow];
    possT= counter;
    if (pvisit!=visit){ 
      if(tt != possTT){
        for(IloInt t=counter; t<= counter+ tt; t++){
          if( _cost[pvisit][visit][t/_lengthTimeWindow] + t - counter < possTT){
             possTT= _cost[pvisit][visit][t/_lengthTimeWindow] + t - counter;
             possT= t;
          }
        }
      }
      if(possTT < tt ){
        std::cout << "Not optimal tt "<<tt <<" between: " << pvisit << " and " << visit << " at time " << counter << endl;
        std:: cout << "Optimal is: " << possTT << " taken at " << possT << endl;
        // std:: cout << "Next possTT: " << _cost[pvisit][visit][(counter+1)/_lengthTimeWindow] << endl;
      }
      else if(possTT > tt)
        std::cout << "Not possible tt" << endl;
    }
    totalTT+= possTT;
    counter+= tt + _duration[0];
    pvisit=visit;
    s1=s2;
  }
  std::cout << "End of checker" << endl;
  std::cout << "Total tt: " << totalTT << endl;
 // std::cout << "Counter tt: " << counter << endl;
}

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
    _windows[i]= new IloInt[2*nbWindows];
    for(j=0; j<2*nbWindows; j++){
      input >> _windows[i][j];
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
  _horizon = _nbVertices*maxcost;
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


IloBool TDTSPInstance::solve() {
  IloEnv env;
  try {
    ofstream output("logFile.txt");
    if (!output.is_open()) {
      std::cout << "Can't open log file!" << endl;
    }
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
    totalDuration+= _duration[0];
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
    model.add(IloTDNoOverlap(env, tour, totalTravelTime, costMatrix, _lengthTimeWindow, 0));
    // for(IloInt i=0; i<_nbVertices; i++){
      // IloIntExpr indexInCostMatrix = IloTypeOfNext(tour, stops[i], 1)*_nbTimeWindows + timeWindow[i];
      // model.add(travelTime[i] == costMatrix2[i][indexInCostMatrix]);
      // model.add(IloStartOfNext(tour, stops[i], _horizon) == IloEndOf(stops[i]) + travelTime[i]);
    // }
    // model.add(IloNoOverlap(env, tour, minCostMatrix, IloTrue));
    
//------------------------------ OBJECTIVE
   IloIntExpr obj= IloEndOf(stops[_nbVertices])- totalDuration;
   // model.add(obj < 3305);
   model.add(IloMinimize(env, obj));
   //model.add(IloMinimize(env, totalTravelTime));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);  
    
    IloCP cp(model);

//-------------------------------PARAMETERS------------------------------------
    
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    // cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
     cp.setParameter(IloCP::Workers, 1);
    // cp.setParameter(IloCP::RelativeOptimalityTolerance, 1e-9);
    cp.setParameter(IloCP::RandomSeed, 1);
    // cp.setParameter(IloCP::RandomSeed, 2);
    // cp.setParameter(IloCP::IntParam::IntervalSequenceInferenceLevel, IloCP::Extended);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);

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

   IloInt sol1=0, sol2=0, sol3=0;
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
      output << "* " << t1 << " " << cp.getObjValue() << endl;
      cp.setParameter(IloCP::TimeLimit, TIME_LIMIT+t0-t1);
      while (cp.next()){
        output << "* " << cp.getInfo(IloCP::SolveTime)-t0+t1 << " " << cp.getObjValue() << endl;
      }
    }
    cp.endSearch();
    
    if (!solutionFound) {
      output << "# NO SOLUTION FOUND" << endl;
    } 
    else {
      output << "# SOLUTION FOUND" << endl;
    }
    if(DEBUG) {
      output <<  cp.getInfo(IloCP::NumberOfFails) << endl;
    }
    else{
      if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit) {
        output << "# OPTIMALITY NOT PROVED" << endl;
      } else {
        output << "# OPTIMALITY PROVED" << endl;
      }
      output << "# BEST SOLUTION: " << cp.getObjValue() << endl;
      sol1= cp.getObjValue();
      output << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << endl;
      output << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << endl;
      output << "## Solution: " << endl;
      output << _nbVertices << endl;
      IloInt s1=0, counter=0; 
      for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
        s1=cp.getStart(a);
        counter++;
        if(counter== _nbVertices+1)
          output << s1 << std::endl;
        
        else if(REAL_INDEX)
          output << _visits[atoi(a.getName())] << " " << s1 << std::endl;
        else
          output << a.getName() << " " << s1 << std::endl;
      }
      // CPChecker(cp, tour);
    }
    
    //-------------------------------SOLVE2-----------------------------------------  
    cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    cp.setParameter(IloCP::RandomSeed, 2);
    cp.startNewSearch(IloSearchPhase(env, tours));
    solutionFound=IloFalse;
    // while(cp.next()){
      // solutionFound=IloTrue;
      // output << "* " << cp.getInfo(IloCP::SolveTime) << " " << cp.getObjValue() << endl;
    // }
    
    // We hack here a little bit for now to get rid of the large amount of time spent at extraction for checking TI
    if (cp.next()) {
      solutionFound=IloTrue;
      t0 = cp.getInfo(IloCP::SolveTime);
      t1 = t0- cp.getInfo(IloCP::ExtractionTime);
      output << "* " << t1 << " " << cp.getObjValue() << endl;
      cp.setParameter(IloCP::TimeLimit, TIME_LIMIT+t0-t1);
      while (cp.next()){
        output << "* " << cp.getInfo(IloCP::SolveTime)-t0+t1 << " " << cp.getObjValue() << endl;
      }
    }
    cp.endSearch();
    
    if (!solutionFound) {
      output << "# NO SOLUTION FOUND" << endl;
    } 
    else {
      output << "# SOLUTION FOUND" << endl;
    }
    if(DEBUG) {
      output <<  cp.getInfo(IloCP::NumberOfFails) << endl;
    }
    else{
      if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit) {
        output << "# OPTIMALITY NOT PROVED" << endl;
      } else {
        output << "# OPTIMALITY PROVED" << endl;
      }
      output << "# BEST SOLUTION: " << cp.getObjValue() << endl;
      sol2= cp.getObjValue();
      output << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << endl;
      output << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << endl;
      output << "## Solution: " << endl;
      output << _nbVertices << endl;
      IloInt s1=0, counter=0; 
      for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
        s1=cp.getStart(a);
        counter++;
        if(counter== _nbVertices+1)
          output << s1 << std::endl;
        
        else if(REAL_INDEX)
          output << _visits[atoi(a.getName())] << " " << s1 << std::endl;
        else
          output << a.getName() << " " << s1 << std::endl;
      }
      // CPChecker(cp, tour);
    }
    
    //-------------------------------SOLVE3-----------------------------------------  
    cp.startNewSearch(IloSearchPhase(env, tours));
    solutionFound=IloFalse;
    // while(cp.next()){
      // solutionFound=IloTrue;
      // output << "* " << cp.getInfo(IloCP::SolveTime) << " " << cp.getObjValue() << endl;
    // }
    
    // We hack here a little bit for now to get rid of the large amount of time spent at extraction for checking TI
    if (cp.next()) {
      solutionFound=IloTrue;
      t0 = cp.getInfo(IloCP::SolveTime);
      t1 = t0- cp.getInfo(IloCP::ExtractionTime);
      output << "* " << t1 << " " << cp.getObjValue() << endl;
      cp.setParameter(IloCP::TimeLimit, TIME_LIMIT+t0-t1);
      while (cp.next()){
        output << "* " << cp.getInfo(IloCP::SolveTime)-t0+t1 << " " << cp.getObjValue() << endl;
      }
    }
    cp.endSearch();
    
    if (!solutionFound) {
      output << "# NO SOLUTION FOUND" << endl;
    } 
    else {
      output << "# SOLUTION FOUND" << endl;
    }
    if(DEBUG) {
      output <<  cp.getInfo(IloCP::NumberOfFails) << endl;
    }
    else{
      if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit) {
        output << "# OPTIMALITY NOT PROVED" << endl;
      } else {
        output << "# OPTIMALITY PROVED" << endl;
      }
      output << "# BEST SOLUTION: " << cp.getObjValue() << endl;
      sol3= cp.getObjValue();
      output << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << endl;
      output << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << endl;
      output << "## Solution: " << endl;
      output << _nbVertices << endl;
      IloInt s1=0, counter=0; 
      for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
        s1=cp.getStart(a);
        counter++;
        if(counter== _nbVertices+1)
          output << s1 << std::endl;
        
        else if(REAL_INDEX)
          output << _visits[atoi(a.getName())] << " " << s1 << std::endl;
        else
          output << a.getName() << " " << s1 << std::endl;
      }
      // CPChecker(cp, tour);
    }
    output.close();
    if(sol1 != sol2 || sol2 != sol3 || sol1 != sol3 ){
      env.end();
      cout << "Solutions are: " << endl;
      cout << sol1 << " " << sol2 << " " << sol3 << endl;
      return false;
    }
    env.end();
    return true;
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
}
void TDTSPInstance::print(const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << endl;
    cout << "# ALGORITHM: TDNOOVERLAP RESTART" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
}

int * permute(int n) {
  int *a= new int[n];
  int k;
  for (k= 0; k < n; k++)
    a[k] = k;
  for (k= n-1; k > 0; k--) {
    int j = rand() % (k+1);
    int temp = a[j];
    a[j] = a[k];
    a[k] = temp;
  }
  return a;
}

void printSimpleTDTSPInstance(char* fileName, int size){
  ofstream output(fileName);
  if (!output.is_open()) {
    std::cout << "Can't open file '" << fileName << "'!" << endl;
  }
  int* perm=permute(255);
  int duration=1; //fixed visit durations of 1 
  int forbiddenTW=0, precConstraints=0;
  // printPerm(perm);
  output << size << " " << precConstraints << endl; 
  for(int i=0; i<size; i++){
    output << perm[i] << " " << duration << " " << forbiddenTW << endl;
  }
  output.close();
}

int main(int argc, char* argv[]) {
  IloBool restartOk= true;
  char* testInstance = "instance.txt";
  IloInt instancesCounter=0;
  while(restartOk){
    printSimpleTDTSPInstance(testInstance, atoi(argv[1]));
    TDTSPInstance tdtsp(argv[2], testInstance); 
    cout << instancesCounter++ << endl;
    restartOk= tdtsp.solve();
  }
 return 0; 
}


