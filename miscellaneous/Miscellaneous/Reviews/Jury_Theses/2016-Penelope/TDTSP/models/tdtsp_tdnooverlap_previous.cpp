//Previous reading and writing and checking methods for the tdtsp
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
int TIME_LIMIT =3600;
bool DEBUG= false;
ILOSTLBEGIN

IloBool Contains(const char* line, const char* str) {
  return (0 != strstr(line, str)); 
}

IloInt GetIntAfterEqual(char* line) {
  char* str = strchr(line, '=');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloInt result = 0;
  post >> result;
  return result;
}

IloInt* GetIntArray(const char* line, IloInt& n) {
  size_t l = strlen(line)+1;
  char* cpy = new char[l];
  strcpy(cpy, line);
  for (int i=0; i<l-1; i++) {
    if (line[i]<'0' || line[i]>'9') {
      cpy[i]=' ';
    } else {
      cpy[i]=line[i];
    }
  }
  n=0;
  IloInt v=0;
  ILOSTD_ISTREAM num0(cpy);
  while (num0 >> v) { n++; }
  IloInt* result = new IloInt[n];
  ILOSTD_ISTREAM num1(cpy);
  n=0;
  while (num1 >> v) { result[n++]=v; }
  delete [] cpy;
  return result;
}

IloBool IsEmpty(const char* line) {
  return (line[0]==0);
}

class TDTSPInstance {
public:
	TDTSPInstance(const char* nameInstance);
  void solve();
  void print(const char* nameInstance);
  void fileChecker(const char* nameSol);
  void CPChecker(IloCP cp, IloIntervalSequenceVar tour);
  
private:
  IloInt     _nbVertices;
	IloInt     _nbTimeWindows;
	IloInt     _lengthTimeWindow;
	IloInt***  _cost;    // nbVertices*nbVertices*nbTimeWindows
  IloInt**   _minCost; // nbVertices*nbVertices
  IloInt     _maxCost;
	IloInt     _duration;
	IloInt*    _locations;
	IloInt     _horizon;
};

void TDTSPInstance::fileChecker(const char* nameSol){
  ifstream input(nameSol);
  if (!input.is_open()) {
    std::cerr << "Can't open file '" << endl;
  }
  else 
    std::cout << "Starting to check" << endl;
  IloInt counter=0,pvisit,visit, tt, totalTT=_duration, possTT, possT;
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
    totalTT+= possTT + _duration;
    counter+= tt + _duration;
    pvisit=visit;
  }
  std::cout << "End of checker" << endl;
  std::cout << "Total tt: " << totalTT - _nbVertices*_duration << endl;
  std::cout << "Counter tt: " << counter << endl;
    
  input.close();  
  
}

void TDTSPInstance::CPChecker(IloCP cp, IloIntervalSequenceVar tour){
  IloInt counter=_duration,pvisit,visit, tt, totalTT=0, possTT, possT;
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
    counter+= tt + _duration;
    pvisit=visit;
    s1=s2;
  }
  std::cout << "End of checker" << endl;
  std::cout << "Total tt: " << totalTT << endl;
 // std::cout << "Counter tt: " << counter << endl;
}

TDTSPInstance::TDTSPInstance(const char* nameInstance) 
  :_nbVertices       (0)
	,_nbTimeWindows    (0)
	,_lengthTimeWindow (0)
	,_cost             (0)
	,_minCost          (0)
	,_maxCost          (0)
	,_duration         (0)
	,_locations        (0)
	,_horizon          (0)
{
  ifstream input(nameInstance);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << nameInstance << "'!" << endl;
  }
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt r = 0;
  while (r<3 && input.getline(line, lineLimit)) {
    if (Contains(line, "sizeOfTimeStep")) {
      _lengthTimeWindow = GetIntAfterEqual(line);
      r++;
    } else if (Contains(line, "n")) {
       _nbVertices = GetIntAfterEqual(line);
       r++;
    } else if (Contains(line, "m")) {
      _nbTimeWindows = GetIntAfterEqual(line);
      r++;
    } 
  }
  _cost    = new IloInt**[_nbVertices];
  _minCost = new IloInt* [_nbVertices];
  IloInt i,j,k;
  for(i=0; i<_nbVertices; i++){
    _cost   [i] = new IloInt*[_nbVertices];
    _minCost[i] = new IloInt [_nbVertices];
    for(j=0; j<_nbVertices; j++) {
      _cost[i][j] = new IloInt[_nbTimeWindows];
    }
  }
  IloInt mincost = IloIntervalMax;
  IloInt maxcost = 0;
  i=0; j=0; k=-1;
  IloInt n=0;
  while (input.getline(line, lineLimit)) {
    IloInt* vals = GetIntArray(line, n);
    for (IloInt l=0; l<n; ++l) {
      IloInt v = vals[l];
      k++;
      if (k==_nbTimeWindows) { k=0; _minCost[i][j]=((i==j)?0:mincost); j++; mincost= IloIntervalMax; }
      if (j==_nbVertices)    { j=0;i++;}
      _cost[i][j][k] = v;
      if (v > maxcost && i!=j) {
        maxcost = v;
      }
      if (v < mincost && i!=j) {
        mincost = v;
      }
    }
  }
  _minCost[i][j]=((i==j)?0:mincost);
  _maxCost=maxcost;
  input.close();
  _horizon = _nbVertices*maxcost;
  _duration = 1;
  _locations = new IloInt[_nbVertices+1];
  for(IloInt i=0; i < _nbVertices; i++) {
    _locations[i]=i;
  }
  _locations[_nbVertices]= 0;
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
    for(IloInt i=0; i< _nbVertices; i++){ 
      stops[i] = IloIntervalVar(env, _duration);
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
    stops[_nbVertices] = IloIntervalVar(env,_duration);
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
   IloIntExpr obj= IloEndOf(stops[_nbVertices])-_duration*(_nbVertices+1);
   model.add(IloMinimize(env, obj));
   //model.add(IloMinimize(env, totalTravelTime));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);  
    
    IloCP cp(model);

//-------------------------------PARAMETERS------------------------------------
    
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    cp.setParameter(IloCP::Workers, 2);
    //cp.setParameter(IloCP::IntParam::SequenceInferenceLevel, IloCP::Extended);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    //cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    
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
      if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit) {
        cout << "# OPTIMALITY NOT PROVED" << endl;
      } else {
        cout << "# OPTIMALITY PROVED" << endl;
      }
      cout << "# BEST SOLUTION: " << cp.getObjValue() << endl;
      cout << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << endl;
      cout << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << endl;
      IloInt s1=0, s2; 
      for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
        s2=cp.getStart(a);
        cout << a.getName() << " " << (s2- s1 -1) << std::endl;
        s1=s2;
      }
      CPChecker(cp, tour);
    }
    
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
void TDTSPInstance::print(const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << endl;
    cout << "# ALGORITHM: TDNOOVERLAP RESTART" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
}
  
int main(int argc, char* argv[]) {
		TDTSPInstance tdtsp(argv[1]); 
    tdtsp.print(argv[1]);
    if(argc == 2)
      tdtsp.solve();
    else if(argc == 3)
      tdtsp.fileChecker(argv[2]); 
	}


