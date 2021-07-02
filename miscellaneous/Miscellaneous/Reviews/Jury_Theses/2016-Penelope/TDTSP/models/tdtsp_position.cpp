#include <ilcp/cp.h>

ILOSTLBEGIN
int TIME_LIMIT=180;

class ChooseSmallestTimeI : public IloIntValueChooserI {
private:
  IloIntVarArray time;
public:
  ChooseSmallestTimeI(IloEnv env, IloIntVarArray time)
    : IloIntValueChooserI(env),
      time(time){ }
  IloInt choose(IloCP cp, IloIntVarArray vars, IloInt i) {
    // std::cout << "atPosition: " << i << ", domain: " << cp.domain(vars[i]) << std::endl;
    // for(IloInt v=0; v<vars.getSize(); v++){
      // std::cout << "Visit: " << v << std::endl;
      // std::cout << "Time Domain: " << cp.domain(time[v]) << std::endl;
    // }
    IloIntVar var = vars[i];
    IloInt best = IloIntervalMax;
    IloInt bestValue = cp.getMin(var);
    for (IloCP::IntVarIterator it(cp, var); it.ok(); ++it) {
      IloInt eval = cp.getMin(time[*it]);
      //std::cout << "time: " << *it << "= " << eval << std::endl;
      if (eval < best) {
        best      = eval;
        bestValue = *it;
      }
    }
    return bestValue;
  }
};
IloIntValueChooser ChooseSmallestTime(IloEnv env, IloIntVarArray time) {
  return new (env) ChooseSmallestTimeI(env, time);
}

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
    IloIntArray2 costMatrix(env, _nbVertices);
    IloIntArray2 costMatrix2(env, _nbVertices);
    IloIntArray2 minCostMatrix(env, _nbVertices);
    IloIntVarArray atPosition(env,_nbVertices+1);
    IloIntVarArray travelTime(env, _nbVertices);
    IloIntVarArray time(env, _nbVertices+1);
    IloIntVarArray tw(env, _nbVertices+1);
    IloIntExprArray timeWindow(env, _nbVertices);
    IloIntArray locationsArray(env, _nbVertices+1);
    IloIntVarArray td(env,_nbVertices);
    IloIntVarArray tt(env,_nbVertices);
    for(IloInt i=0; i < _nbVertices+1; i++)
      locationsArray[i]=_locations[i];
   
    char name[64];
    for(IloInt i=0; i< _nbVertices; i++){
      td[i]= IloIntVar(env, 0, _maxCost[i]);
      tt[i]= IloIntVar(env, 0, _maxCost[i]);
      tw[i]= IloIntVar(env, 0, _nbTimeWindows-1);
      atPosition[i] = IloIntVar(env, 0, _nbVertices-1);
      travelTime[i] = IloIntVar(env, 0, _maxCost[i]);	
      sprintf(name, "pos%ld", i);
      atPosition[i].setName(name);
      time[i] = IloIntVar(env, 0, _horizon);	
      sprintf(name, "time%ld", i);
      time[i].setName(name);
      timeWindow[i] = IloMin(IloDiv(time[i],_lengthTimeWindow), _nbTimeWindows-1);
      costMatrix[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      minCostMatrix[i] = IloIntArray(env, _nbVertices);
      costMatrix2[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      for(IloInt j=0; j<_nbVertices; j++){
        for(IloInt k=0; k< _nbTimeWindows; k++){
          costMatrix2[i][k*_nbVertices+j]=_cost[j][k][i];
        }
        minCostMatrix[i][j]=_minCost[j][i];
      }
    }
    time[_nbVertices] = IloIntVar(env, 0, _horizon);
    atPosition[_nbVertices] = IloIntVar(env, _nbVertices, _nbVertices);
    tw[_nbVertices] = IloIntVar(env, 0, _nbTimeWindows-1);
    model.add(atPosition[0]==0);
    model.add(atPosition[_nbVertices]==_nbVertices);
    model.add(time[0]==0);
    model.add( IloAllDiff(env, atPosition));
    model.add(tw[0]==0);
    for(IloInt i=0; i<_nbVertices; i++){
      IloIntTupleSet ts(env,3);
      for(IloInt j=0; j<_nbVertices; j++){
        for(IloInt k=0; k< _nbTimeWindows; k++){
          if(j!=i) ts.add(IloIntArray(env,3, j,k,_cost[i][k][locationsArray[j]]));
        }
      }
      model.add(tw[i]== timeWindow[i]);
      model.add(td[i]== tt[atPosition[i]]);
      if(i!= _nbVertices-1)model.add(IloAllowedAssignments(env, atPosition[i+1], tw[i], td[i], ts));
      model.add(time[i+1] >= time[i] + _duration + tt[atPosition[i]]);
      //model.add(td[locationsArray[i]] >= minCostMatrix[locationsArray[i]][prev[i]]);
    }
    IloIntExpr indexInCostMatrix = atPosition[_nbVertices-1] + timeWindow[atPosition[_nbVertices-1]]*_nbVertices;
    model.add(td[_nbVertices-1]== costMatrix2[0][indexInCostMatrix]);
    IloIntExpr totalTravelTime= IloSum(travelTime);
    model.add(totalTravelTime <= time[_nbVertices] - _nbVertices*_duration);
    // Objective
    // model.add(IloMinimize(env, totalTravelTime));
    model.add(IloMinimize(env,time[_nbVertices] - _nbVertices*_duration));
    
    //SearchPhase
    IloIntValueChooser valChooser = ChooseSmallestTime(env, time);
    IloSearchPhase sp1(env, atPosition, IloSelectSmallest(IloVarIndex(env, atPosition)), valChooser);
    IloSearchPhase sp2(env, time, IloSelectSmallest(IloDomainMin(env)), IloSelectSmallest(IloValue(env)));
    IloSearchPhaseArray phases(env);
    phases.add(sp1);
    phases.add(sp2);
    
    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
     cp.setParameter(IloCP::RandomSeed, 2);
    if (cp.refineConflict()) {
      cp.writeConflict(cp.out());
    } 
    cp.startNewSearch(phases);
    
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
    
    for(IloInt i=0 ; i< _nbVertices; i++){ 
      cout << cp.getValue(atPosition[i]) << " " << cp.getValue(time[i]) << std::endl;
    } 
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
void TDTSPInstance::print(const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << endl;
    cout << "# ALGORITHM: CPP_NEXT" << endl;
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


