#include <ilcp/cp.h>

ILOSTLBEGIN
int TIME_LIMIT=120;
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
	TDTSPInstance(char* nameInstance); 
  void solve();
  void print(const char* nameInstance);
  
private:
  IloInt     _nbVertices;
	IloInt     _nbTimeWindows;
	IloInt     _lengthTimeWindow;
	IloInt***  _cost;    // nbVertices*nbVertices*nbTimeWindows
	IloInt**   _minCost; // nbVertices*nbVertices
	IloInt*    _maxCost;
	IloInt     _duration;
	IloInt*    _locations;
	IloInt     _horizon;
};

TDTSPInstance::TDTSPInstance(char* nameInstance) 
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
  _maxCost = new IloInt  [_nbVertices];
  IloInt i,j,k;
  for(i=0; i<_nbVertices; i++){
    _cost   [i] = new IloInt*[_nbTimeWindows];
    _minCost[i] = new IloInt [_nbVertices];
    for(j=0; j<_nbTimeWindows; j++) {
      _cost[i][j] = new IloInt[_nbVertices];
    }
  }
  IloInt mincost = IloIntervalMax;
  IloInt maxcost = 0, maxmaxcost = 0;
  i=0; j=0; k=-1;
  IloInt n=0;
  while (input.getline(line, lineLimit)) {
    IloInt* vals = GetIntArray(line, n);
    for (IloInt l=0; l<n; ++l) {
      IloInt v = vals[l];
      k++;
      if (k==_nbTimeWindows) { k=0; _minCost[i][j]=((i==j)?0:mincost); j++; mincost= IloIntervalMax; }
      if (j==_nbVertices)    { j=0; _maxCost[i]=maxcost; i++; maxcost=0; }
      _cost[i][k][j] = v;
      if (v > maxcost && i!=j) {
        maxcost = v;
        if (v > maxmaxcost) {
          maxmaxcost = v;
        }
      }
      if (v < mincost && i!=j) {
        mincost = v;
      }
    }
  }
  _minCost[i][j]=((i==j)?0:mincost);
  _maxCost[i]=maxcost;
  input.close();
  _horizon = _nbVertices*maxmaxcost;
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

    IloIntArray2 costMatrix(env, _nbVertices);
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
    char name[64];
    for(IloInt i=0; i< _nbVertices; i++){ 
      stops[i] = IloIntervalVar(env, _duration);
      sprintf(name, "V%ld", i);
      stops[i].setName(name);
      stops[i].setEndMax(_horizon);
      travelTime[i] = IloIntVar(env, 0, _maxCost[i]);	
      sprintf(name, "TT%ld", i);
      travelTime[i].setName(name);
      timeWindow[i] = IloMin(IloDiv(IloEndOf(stops[i]),_lengthTimeWindow), _nbTimeWindows-1);
      //timeWindow[i] = IloDiv(IloEndOf(stops[i]),_lengthTimeWindow);
      costMatrix[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      for(IloInt j=0; j<_nbVertices; j++){
        for(IloInt k=0; k< _nbTimeWindows; k++){
          costMatrix[i][k*_nbVertices+j]=_cost[i][k][j];
        }
      }
    }
    stops[_nbVertices] = IloIntervalVar(env,_duration);
    sprintf(name, "V%ld", _nbVertices);
    stops[_nbVertices].setName(name);
    stops[_nbVertices].setEndMax(_horizon);
    IloIntervalSequenceVar tour(env, stops, locationsArray);
    tour.setName("Tour");  

//---------------------------- CONSTRAINTS

    model.add(IloNoOverlap(env, tour, minCostMatrix, IloTrue));
    model.add(IloFirst(env,tour, stops[0]));
    model.add(IloLast(env,tour, stops[_nbVertices]));
    for(IloInt i=0; i<_nbVertices; i++){
      IloIntExpr indexInCostMatrix = IloTypeOfNext(tour, stops[i], 1) + timeWindow[i]*_nbVertices;
      model.add(travelTime[i] == costMatrix[i][indexInCostMatrix]);
      model.add(IloStartOfNext(tour, stops[i], _horizon) >= IloEndOf(stops[i]) + travelTime[i]);
    }
    IloIntExpr totalTravelTime= IloSum(travelTime);
    
//------------------------------ OBJECTIVE
    //model.add(IloMinimize(env, totalTravelTime));
     IloIntExpr obj=  IloEndOf(stops[_nbVertices]);//-_duration*(_nbVertices+1);
   // model.add(obj < 4827);
    model.add(IloMinimize(env,obj));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);

    IloCP cp(model);
    
//-------------------------------PARAMETERS------------------------------------
   
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    //cp.setParameter(IloCP::FailLimit, 10);
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    // cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    //cp.setParameter(IloCP::RandomSeed, 2);
    // if (cp.refineConflict()) {
      // cp.writeConflict(cp.out());
    // } 
    
//---------------------------SET STARTING POINT
    // IloSolution sol(env);
    // ifstream input("tdtsp_100_0_TDNOOVERLAP.log");
    // IloInt counter=0,pvisit,visit, tt;
    // if (!input.is_open()) {
      // std::cerr << "Can't open file '" << endl;
    // }
    // pvisit=0;
    // for(IloInt i=0; i<=_nbVertices+1; i++){
      // input >> visit;
      // input >> tt;
      // counter+= tt + 1;
      // if (pvisit!=visit) model.add(IloEndBeforeStart(env, stops[pvisit], stops[visit]));
      // stops[visit].setStartMin(counter);
      // stops[visit].setStartMax(counter);
      // pvisit=visit;
    // }
    // input.close();  
    // cp.setStartingPoint(sol);

//-------------------------------SOLVE----------------------------------------- 
    
    cp.startNewSearch(IloSearchPhase(env, tours));
    IloBool solutionFound=IloFalse;
    while(cp.next()){
      solutionFound=IloTrue;
      cout << "* " << cp.getInfo(IloCP::SolveTime) << " " << cp.getObjValue() << endl;
    }
    if (!solutionFound) {
      cout << "# NO SOLUTION FOUND" << endl;
    } 
    else {
      cout << "# SOLUTION FOUND" << endl;
    }
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
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
void TDTSPInstance::print(const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << endl;
    cout << "# ALGORITHM: CPP_SCHED2" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
}
  	
  
	int main(int argc, char* argv[]) {
		for(IloInt i=1; i< argc; i++){
			TDTSPInstance tdtsp(argv[i]); 
			tdtsp.print(argv[i]);
      tdtsp.solve();
		}
	}


