#include <ilcp/cp.h>
//#include <algorithm>
//#include <array>

ILOSTLBEGIN
IloInt TIME_LIMIT = 2*3600;
IloInt INSTANCE_HORIZON = 1073741822;
IloBool REAL_INDEX= true;
IloBool MEDIAN= true;

int compareInt(const void* p1, const void* p2) {
  const IloInt f1 = *(const IloInt*) p1;
  const IloInt f2 = *(const IloInt*) p2;
  if (f1 == f2) return 0;
  return (f1 > f2) ? 1 : -1;
}

class TDTSPInstance {
public:
    TDTSPInstance(const char* nameBench, const char* nameInstance);
  void solve();
  void print(const char* nameBench, const char* nameInstance);  
  // Obsolete:
  IloInt getObjValue(IloCP cp, IloIntervalSequenceVar tour, IloBool print);
  // We want to store the best feasible sequence:
  void fillSequenceArray(IloIntArray& seqArray, IloCP cp, IloIntervalSequenceVar tour);
  IloInt getObjValue(IloIntArray seqArray, IloBool print);
  
private:
  IloInt     _nbVertices;
  IloInt     _nbTimeWindows;
  IloInt     _lengthTimeWindow;
  IloInt**   _windows;
  IloInt*    _nbWindows;
  IloInt***  _cost;    // nbVertices*nbVertices*nbTimeWindows
  IloInt**   _minCost; // nbVertices*nbVertices
  IloInt**   _medianCost; // nbVertices*nbVertices
  IloInt     _maxCost;
  IloInt*    _duration;
  IloInt*    _locations;
  IloInt     _horizon;
  IloInt*    _visits;
};

// We want to store the best feasible sequence:
void TDTSPInstance::fillSequenceArray(IloIntArray& seqArray, IloCP cp, IloIntervalSequenceVar tour) {
  assert(seqArray.getImpl() != NULL);
  IloInt i=0;
  for (IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
    IloInt s1 = atoi(a.getName());
    assert(i<seqArray.getSize());
    seqArray[i] = s1;
    i++;
  }
  assert(i==seqArray.getSize());
}

IloInt TDTSPInstance::getObjValue(IloIntArray seqArray, IloBool print) {
  IloInt s1=0, s2, counter=0, totalTime=0, cost, durS1, time; 
  for (IloInt i=0; i<seqArray.getSize(); ++i) {
    s1 = seqArray[i];
    durS1 = _duration[s1];
    if (s1==_nbVertices)
      s1=0;
    // cout << "total time: " << totalTime << endl;
    if (i!=0){
      if (totalTime/_lengthTimeWindow >= _nbTimeWindows) {
        cost= _cost[s2][s1][_nbTimeWindows-1];
      } else {
        cost= _cost[s2][s1][totalTime/_lengthTimeWindow];
        for(IloInt t=totalTime; t < totalTime+cost; t++){
          time= t/_lengthTimeWindow;
          if (time >= _nbTimeWindows)
            time= _nbTimeWindows-1;
          if ((_cost[s2][s1][time]+ t - totalTime) < cost)
            cost= t-totalTime + _cost[s2][s1][time];
        }
      }
      totalTime+= cost;
    }
    for (IloInt j=1; j<2*_nbWindows[s1]; j+=2) {
      if ( _windows[s1][j-1] < totalTime && _windows[s1][j] > totalTime) {
        if (_windows[s1][j] >= INSTANCE_HORIZON)
          return -1;
        if (j==_nbWindows[s1] -1 || _windows[s1][j+1] > _windows[s1][j] + durS1)
          totalTime=_windows[s1][j];
      }
    }
    
    counter++;
    if (!print && counter== _nbVertices+1) return totalTime;
    if (print){
      if(counter== _nbVertices+1){
        cout <<"# BEST TD SOLUTION: " << totalTime << std::endl;
      }
      else if(REAL_INDEX) cout << _visits[s1] << " " << totalTime << std::endl;
      else cout << s1 << " " << totalTime << std::endl;
    }
    totalTime+= durS1;
    s2=s1;
  }
  return totalTime;
}
  
// Obsolete:
IloInt TDTSPInstance::getObjValue(IloCP cp, IloIntervalSequenceVar tour, IloBool print){
  IloInt s1=0, s2, counter=0, totalTime=0, startS1, endS1, cost, durS1, time; 
  for (IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
    startS1=cp.getStart(a);
    endS1=cp.getEnd(a);
    durS1= endS1- startS1;
    s1 = atoi(a.getName());
    if(s1==_nbVertices) s1=0;
// cout << "total time: " << totalTime << endl;
    if(a.getImpl() != cp.getFirst(tour).getImpl()){
      if(totalTime/_lengthTimeWindow >= _nbTimeWindows)
        cost= _cost[s2][s1][_nbTimeWindows-1];
      else{
        cost= _cost[s2][s1][totalTime/_lengthTimeWindow];
        for(IloInt t=totalTime; t < totalTime+cost; t++){
          time= t/_lengthTimeWindow;
          if(time >= _nbTimeWindows)
            time= _nbTimeWindows-1;
          if((_cost[s2][s1][time]+ t - totalTime) < cost)
            cost= t-totalTime + _cost[s2][s1][time];
        }
      }
      totalTime+= cost;
    }
    for(IloInt j=1; j<2*_nbWindows[s1]; j+=2){
      if( _windows[s1][j-1] < totalTime && _windows[s1][j] > totalTime) {
        if(_windows[s1][j] >= INSTANCE_HORIZON)
          return -1;
        if(j==_nbWindows[s1] -1 || _windows[s1][j+1] > _windows[s1][j] + durS1)
          totalTime=_windows[s1][j];
      }
    }
    
    counter++;
    if(!print && counter== _nbVertices+1) return totalTime;
    if(print){
      if(counter== _nbVertices+1){
        cout <<"# BEST TD SOLUTION: " << totalTime << std::endl;
      }
      else if(REAL_INDEX) cout << _visits[s1] << " " << totalTime << std::endl;
      else cout << s1 << " " << totalTime << std::endl;
    }
    totalTime+= durS1;
    s2=s1;
  }
  return totalTime;
}

TDTSPInstance::TDTSPInstance(const char* nameBench, const char* nameInstance) 
  :_nbVertices       (0)
  ,_nbTimeWindows    (0)
  ,_lengthTimeWindow (0)
  ,_windows          (0)
  ,_cost             (0)
  ,_minCost          (0)
  ,_medianCost       (0)
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
  _nbWindows = new IloInt[_nbVertices];
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
  _medianCost = new IloInt* [_nbVertices];
  for(i=0; i<_nbVertices; i++){
    _cost   [i] = new IloInt*[_nbVertices];
    _minCost[i] = new IloInt [_nbVertices];
    _medianCost[i] = new IloInt [_nbVertices];
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
  _horizon = 2*_nbVertices*maxcost;
  // _horizon = _nbTimeWindows*_lengthTimeWindow;
  _locations = new IloInt[_nbVertices+1];
  for(i=0; i < _nbVertices; i++) {
    _locations[i]=i;
  }
  _locations[_nbVertices]= 0;
  
  // std::cout << _cost[17][7][5760/_lengthTimeWindow] << " " << cost[_visits[17]][_visits[7]][5760/_lengthTimeWindow]<< std::endl;
  IloInt* p;
  IloInt mid;
  if(_nbTimeWindows%2 == 0)
    mid= _nbTimeWindows/2;
  else mid= (_nbTimeWindows+1)/2;
  for(i=0; i<_nbVertices; i++)
    for(j=0; j<_nbVertices; j++) {
      p= cost[_visits[i]][_visits[j]];
      qsort(p, _nbTimeWindows, sizeof p[0], compareInt);
      _medianCost[i][j] = (p[mid-1] + p[mid]) /2;
    }
    
    // for(i=0; i<_nbVertices; i++)
    // for(j=0; j<_nbVertices; j++) {
      // if (_medianCost[i][j] <= 0) std::cout << i << " " << j << " " << _medianCost[i][j] <<std::endl;;
    // }
  
  // for(i=0; i<_nbVertices; i++){
    // for(j=0; j<_nbVertices; j++) {
      // for (k=0; k<_nbTimeWindows; ++k){
        // std::cout << _cost[i][j][k] << " ";
      // }
      // std::cout << std::endl;
    // }
  // }
  if(_nbVertices<=40) {
      TIME_LIMIT = TIME_LIMIT/2;
    }
}
  
void TDTSPInstance::solve() {
  IloEnv env;
  try {
    IloModel model(env);
    IloIntArray locationsArray(env, _nbVertices+1);
    for(IloInt i=0; i < _nbVertices+1; i++)
      locationsArray[i]=_locations[i];

    // IloIntArray2 costMatrix(env, _nbVertices);
    IloIntervalVarArray stops(env,_nbVertices+1);
    // IloIntVarArray travelTime(env, _nbVertices);
    IloTransitionDistance minCostMatrix(env, _nbVertices);
    IloIntExprArray timeWindow(env, _nbVertices);
    for (IloInt i=0; i<_nbVertices; ++i) {
      for (IloInt j=0; j<_nbVertices; ++j) {
        if (i!=j) {
          if(MEDIAN)  minCostMatrix.setValue(i, j, _medianCost[i][j]);
          else  minCostMatrix.setValue(i, j, _minCost[i][j]);
        }
      }
    }
   
    char name[64];
    IloInt totalDuration=0;
    for(IloInt i=0; i< _nbVertices; i++){ 
      totalDuration+= _duration[i];
      stops[i] = IloIntervalVar(env, _duration[i]);
      sprintf(name, "%ld", i);
      stops[i].setName(name);
      stops[i].setEndMax(_horizon);
      // travelTime[i] = IloIntVar(env, 0, _maxCost[i]);  
      sprintf(name, "TT%ld", i);
      // travelTime[i].setName(name);
      timeWindow[i] = IloMin(IloDiv(IloEndOf(stops[i]),_lengthTimeWindow), _nbTimeWindows-1);
      //timeWindow[i] = IloDiv(IloEndOf(stops[i]),_lengthTimeWindow);
      // costMatrix[i] = IloIntArray(env, _nbVertices*_nbTimeWindows);
      // for(IloInt j=0; j<_nbVertices; j++){
        // for(IloInt k=0; k< _nbTimeWindows; k++){
          // costMatrix[i][k*_nbVertices+j]=_cost[i][k][j];
        // }
      // }
    }
    totalDuration+= _duration[0];
    stops[_nbVertices] = IloIntervalVar(env,_duration[0]);
    sprintf(name, "%ld", _nbVertices);
    stops[_nbVertices].setName(name);
    stops[_nbVertices].setEndMax(_horizon);
    IloIntervalSequenceVar tour(env, stops, locationsArray);
    tour.setName("Tour");  

//---------------------------- CONSTRAINTS

    model.add(IloNoOverlap(env, tour, minCostMatrix, IloTrue));
    model.add(IloFirst(env,tour, stops[0]));
    model.add(IloLast(env,tour, stops[_nbVertices]));
    // for(IloInt i=0; i<_nbVertices; i++){
      // model.add(travelTime[i] == minCostMatrix.getValue(i,IloTypeOfNext(tour, stops[i], 1)));
      // model.add(IloStartOfNext(tour, stops[i], _horizon) >= IloEndOf(stops[i]) + travelTime[i]);
    // }
    // IloIntExpr totalTravelTime= IloSum(travelTime);
    for(IloInt i=0; i< _nbVertices; i++){
      IloNumToNumStepFunction f(env, 0, _horizon, 1);
      if(_nbWindows[i]!=0){
        for(IloInt j=0; j< 2*_nbWindows[i]; j+=2){
          f.setValue(_windows[i][j], _windows[i][j+1], 0);
        }
        model.add(IloForbidExtent(env, stops[i], f));
      }
    }
    
//------------------------------ OBJECTIVE
    //model.add(IloMinimize(env, totalTravelTime));
     IloIntExpr obj=  IloStartOf(stops[_nbVertices]);//-totalDuration;
    model.add(obj < 9909);
    model.add(IloMinimize(env,obj));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);

    IloCP cp(model);
    
//-------------------------------PARAMETERS------------------------------------
   
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    //cp.setParameter(IloCP::FailLimit, 10);
    cp.setParameter(IloCP::RelativeOptimalityTolerance, 1e-9);
    cp.setParameter(IloCP::Workers, 1);
    // cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet); 
    cp.setParameter(IloCP::LogPeriod, 1000000); 
    // cp.setParameter(IloCP::SolutionLimit, 1);
    // cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    cp.setParameter(IloCP::NoOverlapInferenceLevel, IloCP::Extended);
    cp.setParameter(IloCP::IntervalSequenceInferenceLevel, IloCP::Extended);
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
    
    IloIntArray bestFeasibleSequence(env, _nbVertices+1);
    IloInt bestObj = IloIntMax;
    IloIntArray currSequence(env, _nbVertices+1);
    IloBool feasibleSequenceFound = IloFalse;
    
    cp.startNewSearch(IloSearchPhase(env, tours));
    IloBool solutionFound = IloFalse;
    while (cp.next()){
      solutionFound = IloTrue;
      fillSequenceArray(currSequence, cp, tour);
      IloInt obj = getObjValue(currSequence, IloFalse);
      if (0 <= obj) {
        feasibleSequenceFound = IloTrue;
        if (obj < bestObj) {
          // Store current best feasible TDTSP sequence
          for (IloInt i=0; i<_nbVertices+1; ++i) {
            bestFeasibleSequence[i] = currSequence[i];
          }
        }
      }
      cout << "* " << cp.getInfo(IloCP::SolveTime) << " " << cp.getObjValue() << " " << obj << endl;
    }
    if (!solutionFound) {
      cout << "# NO TSP SOLUTION FOUND" << endl;
    } 
    else {
      cout << "# TSP SOLUTION FOUND" << endl;
    }
    if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit || cp.getInfo(IloCP::SolveTime) >= TIME_LIMIT) {
      cout << "# TSP OPTIMALITY NOT PROVED" << endl;
    } else {
      cout << "# TSP OPTIMALITY PROVED" << endl;
    }
    cout << "# BEST TSP SOLUTION: " << cp.getObjValue() << endl;
    cout << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << endl;
    cout << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << endl;
    if (feasibleSequenceFound) {
      cout << "## Solution with TDTSP times: " << endl;
      cout << _nbVertices << endl;
      getObjValue(bestFeasibleSequence, IloTrue);
    } else {
      cout << "## No TDTSP solution found." << endl;
    }
    
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}

void TDTSPInstance::print(const char* nameBench, const char* nameInstance){
    cout << "# INSTANCE: " << nameInstance << " " << nameBench << endl;
    cout << "# ALGORITHM: TSP MEDIAN NOOVERLAP RESTART" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
}
    
  
int main(int argc, char* argv[]) {
    TDTSPInstance tdtsp(argv[1], argv[2]); 
    tdtsp.print(argv[1], argv[2]);
    tdtsp.solve();
    return 0;
}

