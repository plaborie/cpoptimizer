#include <ilcp/cp.h>

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
	TDTSPInstance(char* nameInstance); 
  void solve();
  
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
    IloIntArray locationsArray(env, _nbVertices);//+1);
    for(IloInt i=0; i < _nbVertices; i++)
      locationsArray[i]=_locations[i];

    IloIntArray2 costMatrix(env, _nbVertices);
    IloIntervalVarArray stops(env,_nbVertices);//+1);
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
    for(IloInt i=0; i< _nbVertices; i++){ 
      stops[i] = IloIntervalVar(env, _duration);
      stops[i].setEndMax(_horizon);
      travelTime[i] = IloIntVar(env);//, 0, _maxCost[i]);	
      timeWindow[i] = IloMin(IloDiv(IloEndOf(stops[i]),_lengthTimeWindow), _nbTimeWindows-1);
      costMatrix[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      for(IloInt j=0; j<_nbVertices; j++){
        for(IloInt k=0; k< _nbTimeWindows; k++){
          costMatrix[i][k*_nbVertices+j]=_cost[i][k][j];
        }
      }
    }
    //stops[_nbVertices] = IloIntervalVar(env,_duration);
    //stops[_nbVertices].setEndMax(_horizon);
    IloIntervalSequenceVar tour(env, stops, locationsArray);
    model.add(IloNoOverlap(env, tour, minCostMatrix));
    model.add(IloFirst(env,tour, stops[0]));
    //model.add(IloLast(env,tour, stops[_nbVertices]));
    model.add(IloStartOf(stops[0]) == 0);
    for(IloInt i=0; i<_nbVertices; i++){
      IloIntExpr indexInCostMatrix = IloTypeOfNext(tour, stops[i], 1) + timeWindow[i]*_nbVertices;
      model.add(travelTime[i] == costMatrix[i][indexInCostMatrix]);
      model.add(IloStartOfNext(tour, stops[i], _horizon) == IloEndOf(stops[i]) + travelTime[i]);
    }
    IloIntExpr totalTravelTime= IloSum(travelTime);
    // Objective
    model.add(IloMinimize(env, totalTravelTime));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);

    IloCP cp(model);
    cp.setParameter(IloCP::TimeLimit, 30);
    //cp.setParameter(IloCP::FailLimit, 1);
    cp.setParameter(IloCP::Workers, 1);
    cp.solve(IloSearchPhase(env, tours));
    
     
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
	
	int main(int argc, char* argv[]) {
		for(IloInt i=1; i< argc; i++){
			TDTSPInstance tdtsp(argv[i]); 
			tdtsp.solve();
		}
	}


