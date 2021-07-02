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
#include <time.h> 

#include <../team/TDTSP/src/ttime.cpp>

#include <../team/TDTSP/src/tdnooverlapct.cpp>
IloInt TIME_LIMIT = 15*60;
IloBool DEBUG= false;
ILOSTLBEGIN
IloBool REAL_INDEX = true;

IloNum max(IloNum a, IloNum b){
  if(a > b) return a;
  return b;  
}

class TDTSPInstance {
public:
	// TDTSPInstance(const char* nameInstance);
	TDTSPInstance(const char* nameBench, const char* nameInstance);
  void solve();
  void print();

  
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
  char* benchMatrix;
  ifstream input(nameInstance);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << nameInstance << "'!" << endl;
  }
  IloInt matrixChoice= rand()%3;
  if(matrixChoice==0) benchMatrix= "InstancesTDTSP/tdtsp_bench_255_6h00_12h30_mirrored.txt";
  else if (matrixChoice==1) benchMatrix= "InstancesTDTSP/tdtsp_bench_255_6h00_12h30_10p_ti_mirrored.txt";
  else benchMatrix= "InstancesTDTSP/tdtsp_bench_255_6h00_12h30_20p_ti_mirrored.txt";
  
  //using benchMatrix (matrix chosen randomly) instead of nameBench given as input
  ifstream input_bench(benchMatrix);
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
  _horizon = _nbTimeWindows*_lengthTimeWindow;
  _locations = new IloInt[_nbVertices+1];
  for(i=0; i < _nbVertices; i++) {
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
    model.add(totalTravelTime== 0);
    model.add(IloTDNoOverlap(env, tour, totalTravelTime, costMatrix, _lengthTimeWindow, 0));
    //TW constraints
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
   IloIntExpr obj= IloStartOf(stops[_nbVertices]);
   model.add(IloMinimize(env, obj));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);  
    
    IloCP cp(model);

//-------------------------------PARAMETERS------------------------------------
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::RelativeOptimalityTolerance, 1e-9);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);


//-------------------------------SOLVE-----------------------------------------  
    cp.startNewSearch(IloSearchPhase(env, tours));
    IloBool solutionFound=IloFalse;

    IloInt counter=0;
    if (cp.next() && counter <3) {
      counter++;
    }
    cp.endSearch();
    counter=0; 
    IloInt delta=1800, s=0, e=0, pbTW, lTW1, sTW1, eTW1, lTW2, sTW2, eTW2, index, miniHorizon= _horizon;
    IloInt s1=0;
    if( 1.5*cp.getObjValue() < _horizon)
      miniHorizon= 1.5*cp.getObjValue();
////Print instance with tw constraints -------------------------  
    IloInt** TW= new IloInt*[_nbVertices];
    for(IloInt i=0; i < _nbVertices; i++)
      TW[i]= new IloInt[7]; //TW[0]= nbTW + at most 3 TW
    
    for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0, counter < _nbVertices; a =cp.getNext(tour, a)){ 
      pbTW= rand()%100;
      index=atoi(a.getName());
// cout << _visits[index] << " " << _duration[index] << " ";
      TW[index][0]=0;
      if(pbTW <=50){
        lTW1= 3600 + rand()%3600;
        lTW2= 3600 + rand()%3600;
        s=cp.getStart(a);
        e=cp.getEnd(a);
        sTW1= max(0, e - lTW1);
        if(s > sTW1)
          sTW1+= rand()%(s-sTW1);
        eTW1= sTW1+lTW1;
        if(sTW1 > 0.75*cp.getObjValue() && max(0,sTW1- lTW2 -delta)!=0){
          sTW2= rand()%((int) max(0,sTW1- lTW2 -delta));
          pbTW= 26;
        }
        else if(((int) max(0,sTW1- lTW2 -delta)+ (int) max(0,miniHorizon- lTW2-eTW1-delta) ) > 0) sTW2= rand()%((int) max(0,sTW1- lTW2 -delta)+ (int) max(0,miniHorizon- lTW2-eTW1-delta) );
        else sTW2=-1;

        if(sTW2 >= max(0,sTW1- lTW2 -delta)){
          sTW2+= lTW1+ lTW2+ 2*delta;
        }
        eTW2= sTW2 + lTW2;
/// saving TWs
        if(pbTW > 25 && sTW2!=-1){
          if(sTW1 > sTW2){
            IloInt temp;
            temp=sTW1;
            sTW1= sTW2;
            sTW2= temp;
            temp=eTW1;
            eTW1= eTW2;
            eTW2= temp;
          }
          TW[index][0]=3;
          TW[index][1]=0;
          TW[index][2]=sTW1;
          TW[index][3]=eTW1;
          TW[index][4]=sTW2;
          TW[index][5]=eTW2;
          TW[index][6]=1073741822;
          if(sTW1 <=0){
            TW[index][0]--;
            TW[index][1]=eTW1;
            TW[index][2]=sTW2;
            TW[index][3]=eTW2;
            TW[index][4]=1073741822;
          }
          if(eTW2 >=_horizon){
            TW[index][0]--;
          }
        }
        else{
          TW[index][0]=2;
          TW[index][1]=0;
          TW[index][2]=sTW1;
          TW[index][3]=eTW1;
          TW[index][4]=1073741822;
          if(sTW1 <=0){
            TW[index][0]--;
            TW[index][1]=eTW1;
            TW[index][2]=1073741822;
          }
          if( eTW1 >= _horizon){
            TW[index][0]--;
          }
        }
      }
// for(IloInt i=0; i <=2*TW[index][0]; i++){
  // cout << TW[index][i] << " ";
// } 
// cout << endl;
      counter++;
    }
/// printing TWs
    for(IloInt index=0; index <_nbVertices; index++){
      cout << _visits[index] << " " << _duration[index] << " ";
      for(IloInt i=0; i <=2*TW[index][0]; i++){
        if((i+1)%2==0 && TW[index][i] > TW[index][i+1])
          cerr << "error in instance" << endl;
        cout << TW[index][i] << " ";
      } 
      cout << endl;
    }
/// end of print /    
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}
void TDTSPInstance::print(){
  cout << _nbVertices << " 0" << endl;
}





  
int main(int argc, char* argv[]) {
    // cerr << atoi(argv[3]) << endl;
    srand(atoi(argv[3]));
		TDTSPInstance tdtsp(argv[1], argv[2]); 
    tdtsp.print();
    tdtsp.solve();
}


