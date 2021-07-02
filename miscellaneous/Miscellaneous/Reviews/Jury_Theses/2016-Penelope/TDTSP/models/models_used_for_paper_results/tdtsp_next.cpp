#include <ilcp/cp.h>

ILOSTLBEGIN
int TIME_LIMIT=2*60*60;

class ChooseSmallestTimeI : public IloIntValueChooserI {
private:
  IloIntVarArray time;
  IloIntVarArray prev;
  IloIntVarArray next;
public:
  ChooseSmallestTimeI(IloEnv env, IloIntVarArray time, IloIntVarArray prev, IloIntVarArray next)
    : IloIntValueChooserI(env),
      time(time),
      prev(prev),
      next(next){ }
  IloInt choose(IloCP cp, IloIntVarArray vars, IloInt i) {
   
   // std::cout << "atPosition: " << i << ", domain: " << cp.domain(vars[i]) << std::endl;
    // for(IloInt v=0; v<vars.getSize(); v++){
      // std::cout << "Visit: " << v << std::endl;
      // std::cout << "Prev Domain: " << cp.domain(prev[v]) << std::endl;
      // std::cout << "Next Domain: " << cp.domain(next[v]) << std::endl;
      // std::cout << "Time Domain: " << cp.domain(time[v]) << std::endl;
    // }

    IloIntVar var = vars[i];
    IloInt best = IloIntervalMax;
    IloInt bestValue = cp.getMin(var);
    for (IloCP::IntVarIterator it(cp, var); it.ok(); ++it) {
      IloInt eval = cp.getMin(time[*it]);
      // std::cout << "time: " << *it << "= " << eval << std::endl;
      if (eval < best) {
        best      = eval;
        bestValue = *it;
      }
    }
    return bestValue;
  }
};
IloIntValueChooser ChooseSmallestTime(IloEnv env, IloIntVarArray time,IloIntVarArray prev, IloIntVarArray next ) {
  return new (env) ChooseSmallestTimeI(env, time, prev, next );
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
	TDTSPInstance(const char* nameBench, const char* nameInstance); 
  void solve();
  void print(const char* nameBench, const char* nameInstance);
  
private:
  IloInt     _nbVertices;
	IloInt     _nbTimeWindows;
	IloInt     _lengthTimeWindow;
	IloInt**   _windows;
  IloInt*    _nbWindows;
  IloInt***  _cost;    // nbVertices*nbVertices*nbTimeWindows
	IloInt**   _minCost; // nbVertices*nbVertices
	IloInt     _maxCost;
	IloInt*    _duration;
	IloInt*    _locations;
	IloInt     _horizon;
  IloInt*    _visits;
};


TDTSPInstance::TDTSPInstance(const char* nameBench, const char* nameInstance) 
  :_nbVertices       (0)
	,_nbTimeWindows    (0)
	,_lengthTimeWindow (0)
	,_windows          (0)
	,_nbWindows        (0)
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
  _nbWindows= new IloInt[_nbVertices];
  for(i=0; i<_nbVertices; i++){
    input >> _visits[i];
    if(_visits[i] > indexMax) indexMax= _visits[i];
    input >> _duration[i];
    input >> nbWindows;
    if(nbWindows!=0){
      _nbWindows[i]= nbWindows;
      _windows[i]= new IloInt[2*nbWindows];
      for(j=0; j<2*nbWindows; j++){
        input >> _windows[i][j];
      }
    }
    else _nbWindows[i]= 0;
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
  _horizon = _nbVertices*maxcost;
  _locations = new IloInt[_nbVertices+1];
  for(i=0; i < _nbVertices; i++) {
    _locations[i]=i;
  }
  _locations[_nbVertices]= 0;
  
  // for(i=0; i<_nbVertices; i++){
    // for(j=0; j<_nbVertices; j++) {
      // for (k=0; k<_nbTimeWindows; ++k){
        // std::cout << _minCost[i][j] << " ";
      // }
      // std::cout << std::endl;
    // }
  // }
}

typedef IloArray< IloIntVarArray > IloIntVarArray2;

void TDTSPInstance::solve() {
  IloEnv env;
  try {
    IloModel model(env);
    IloIntArray2 costMatrix(env, _nbVertices);
    IloIntArray2 costMatrix2(env, _nbVertices);
    IloIntArray2 minCostMatrix(env, _nbVertices);
    IloIntVarArray2 lbTimeNext(env, _nbVertices+1);
    IloIntVarArray atPosition(env,_nbVertices);
    IloIntVarArray positionOf(env,_nbVertices);
    IloIntVarArray travelTime(env, _nbVertices);
    IloIntVarArray next(env,_nbVertices+1);
    IloIntVarArray prev(env,_nbVertices+1);
    IloIntVarArray time(env, _nbVertices+1);
    IloIntVarArray timeEnd(env, _nbVertices+1);
    IloIntVarArray tw(env, _nbVertices+1);
    IloIntExprArray timeWindow(env, _nbVertices);
    IloIntArray locationsArray(env, _nbVertices+1);
    IloIntArray durationsArray(env, _nbVertices+1);
    IloOr* TWConst= new IloOr[_nbVertices];  
    IloIntVarArray td(env,_nbVertices);
    for(IloInt i=0; i < _nbVertices+1; i++){
      locationsArray[i]=_locations[i];
      durationsArray[i]=_duration[i];
    }
   
    char name[64];
    prev[0] = IloIntVar(env, _nbVertices, _nbVertices);
    next[_nbVertices] = IloIntVar(env, 0, 0);
    lbTimeNext[0] = IloIntVarArray(env, _nbVertices);
    for(IloInt i=0; i< _nbVertices; i++){
      td[i]= IloIntVar(env, 0, _maxCost);
      tw[i]= IloIntVar(env, 0, _nbTimeWindows-1);
      atPosition[i] = IloIntVar(env, 0, _nbVertices-1);
      positionOf[i] = IloIntVar(env, 0, _nbVertices-1);
      travelTime[i] = IloIntVar(env, 0, _maxCost);	
      next[i] = IloIntVar(env, 1, _nbVertices);
      prev[i+1] = IloIntVar(env, 0, _nbVertices-1);
      sprintf(name, "pos%ld", i);
      atPosition[i].setName(name);
      sprintf(name, "next%ld", i);
      next[i].setName(name);
      sprintf(name, "prev%ld", i);
      prev[i].setName(name);
      time[i] = IloIntVar(env, 0, _horizon);	
      timeEnd[i] = IloIntVar(env, 0, _horizon);	
      sprintf(name, "time%ld", i);
      time[i].setName(name);
      timeWindow[i] = IloMin(IloDiv(time[i]+durationsArray[i],_lengthTimeWindow), _nbTimeWindows-1);
      costMatrix[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      minCostMatrix[i] = IloIntArray(env, _nbVertices);
      lbTimeNext[i+1] = IloIntVarArray(env, _nbVertices);
      costMatrix2[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      for(IloInt j=0; j<_nbVertices; j++){
        for(IloInt k=0; k< _nbTimeWindows; k++){
          costMatrix[i][k*_nbVertices+j]=_cost[i][j][k];
          costMatrix2[i][k*_nbVertices+j]=_cost[j][i][k];
         // if(j!=i) ts[i].add(IloIntArray(env,3, j,k,_cost[j][k][i]));
        }
        minCostMatrix[i][j]=_minCost[j][i];
        lbTimeNext[i+1][j] = IloIntVar(env, 0, _horizon);
        sprintf(name, "lbTimeNext%ld_%ld", i+1, j);
        lbTimeNext[i+1][j].setName(name);
      }
      lbTimeNext[0][i] = IloIntVar(env, 0, 0);
      sprintf(name, "lbTimeNext0_%ld", i);
      lbTimeNext[0][i].setName(name);
      
    }
    time[_nbVertices] = IloIntVar(env, 0, _horizon);
    timeEnd[_nbVertices] = IloIntVar(env, 0, _horizon);
    sprintf(name, "time%ld", _nbVertices);
    tw[_nbVertices] = IloIntVar(env, 0, _nbTimeWindows-1);
    
//---------------------------- CONSTRAINTS
    
    model.add(atPosition[0]==0);
    model.add(time[0]==0);
    model.add( IloAllDiff(env, atPosition));
    // model.add( IloAllDiff(env, positionOf));
    // model.add( IloAllDiff(env, next));
    model.add( IloAllDiff(env, prev));
    model.add( IloInverse(env, prev, next));
    model.add( IloInverse(env, positionOf, atPosition));
    model.add(next[atPosition[_nbVertices-1]]==_nbVertices);
    model.add(prev[_nbVertices]==atPosition[_nbVertices-1]);
    model.add(prev[atPosition[1]]==0);
    for(IloInt i=0; i<_nbVertices-1; i++){
      model.add(next[atPosition[i]]==atPosition[i+1]);
      // model.add(positionOf[next[i]]==positionOf[i]+1);
      model.add(prev[atPosition[i+1]]==atPosition[i]);
      if(i!=0) model.add(positionOf[prev[i]]+1==positionOf[i]);
    } 
    for(IloInt i=0; i<_nbVertices; i++){
      IloIntExpr indexInCostMatrix = locationsArray[next[i]] + timeWindow[i]*_nbVertices;
      model.add(timeEnd[i] == time[i]+ durationsArray[i]);
      model.add(travelTime[i] == costMatrix[i][indexInCostMatrix]);
      model.add(time[next[i]]>= timeEnd[i] + travelTime[i]);
    }
    for(IloInt i=0; i<_nbVertices; i++){
      for(IloInt j=1; j<=_nbVertices; j++){
        model.add(lbTimeNext[j][i] == timeEnd[i] + minCostMatrix[locationsArray[j]][i]);
      }
      if(i!=0) model.add(time[i]>= lbTimeNext[i][prev[i]]);
    }
    model.add(time[_nbVertices]>= lbTimeNext[_nbVertices][prev[_nbVertices]]);
    model.add(tw[0]==0);
    for(IloInt i=1; i<=_nbVertices; i++){
      IloIntTupleSet ts(env,3);
      for(IloInt j=0; j<_nbVertices; j++){
        for(IloInt k=0; k< _nbTimeWindows; k++){
          if(j!=i) ts.add(IloIntArray(env,3, j,k,_cost[j][locationsArray[i]][k]));
        }
      }
      model.add(tw[i]== timeWindow[prev[i]]);
      model.add(IloAllowedAssignments(env, prev[i], tw[i], td[locationsArray[i]], ts));
      model.add(time[i] >= timeEnd[prev[i]] + td[locationsArray[i]]);
      model.add(td[locationsArray[i]] >= minCostMatrix[locationsArray[i]][prev[i]]);
    }
    for(IloInt i=0; i<=_nbVertices; i++){
      model.add(prev[i]!=i);
      model.add(next[i]!=i);
    }
    for(IloInt i=0; i< _nbVertices; i++){
      if(_nbWindows[i]>0){
        TWConst[i]= IloOr(env);
        TWConst[i].add(timeEnd[i] < _windows[i][0]);
        for(IloInt j=1; j< _nbWindows[i]; j++){
          TWConst[i].add(time[i] >= _windows[i][2*j-1] && timeEnd[i] < _windows[i][2*j]);
        }
        model.add(TWConst[i]);
      }
      
      
      
      
    }
    
//------------------------------ OBJECTIVE
     
    IloIntExpr totalTravelTime= IloSum(travelTime);
    //model.add(totalTravelTime <= time[_nbVertices] - _nbVertices*_duration);
    // Objective
    // model.add(IloMinimize(env, totalTravelTime));
    model.add(IloMinimize(env,time[_nbVertices]));
    
//-----------------------------  SEARCH 
    IloIntValueChooser valChooser = ChooseSmallestTime(env, time, prev, next);
    IloSearchPhase sp1(env, atPosition, IloSelectSmallest(IloVarIndex(env, atPosition)), valChooser);
    IloSearchPhase sp2(env, time, IloSelectSmallest(IloDomainMin(env)), IloSelectSmallest(IloValue(env)));
    IloSearchPhaseArray phases(env);
    phases.add(sp1);
    phases.add(sp2);
    
    IloCP cp(model);
    
//-------------------------------PARAMETERS------------------------------------
     
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::RelativeOptimalityTolerance, 1e-9);
    cp.setParameter(IloCP::LogPeriod, 1); 
    // cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    // if (cp.refineConflict()) {
      // cp.writeConflict(cp.out());
      // cp.exportConflict(cp.out());
    // } 
    
//-------------------------------SOLVE----------------------------------------- 
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
    for(IloInt i=0 ; i< _nbVertices; i++){ 
      cout << _visits[cp.getValue(atPosition[i])] << " " << cp.getValue(time[cp.getValue(atPosition[i])]) << std::endl;
    } 
    cout <<"# BEST TD SOLUTION: " << cp.getValue(time[_nbVertices]) << std::endl;
    
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
void TDTSPInstance::print(const char* nameBench, const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << " " << nameBench << endl;
    cout << "# ALGORITHM: CPP_NEXT" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
}
  	
  
int main(int argc, char* argv[]) {
		TDTSPInstance tdtsp(argv[1], argv[2]); 
    tdtsp.print(argv[1],argv[2]);
    tdtsp.solve();
}

