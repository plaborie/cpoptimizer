#include <ilcp/cp.h>
#include <../team/TDTSP/src/ttime.cpp>
#include <../team/TDTSP/src/tdnooverlapct.cpp>

IloInt TIME_LIMIT = 3600;
IloInt FACTOR_CONVERSION = 1;
IloBool DEBUG= false;
ILOSTLBEGIN


class TDTSPInstanceFormat {
public:
  TDTSPInstanceFormat(const char* nameInstance, const char* namePattern); //formatting from Arigliano2014 instances http://www.emanuela.guerriero.unisalento.it/Downloads.html
  
  void print(const char* nameInstance, const char* namePattern);
  void solve(const char* nameSol);
  
private:
  IloEnv     _env;
  IloInt     _nbVertices;
  IloInt     _nbTimeSteps;
  IloInt     _nbClusters;
  IloNum     _Horizon;
  IloInt*    _locations;
  IloNum**   _dist;
  IloNum**   _times;
  IloNum**   _speed;
  IloInt**   _clusters;
  IloInt**   _nbValues;
  IloNumArray**  _xValues;
  IloNumArray**  _vValues;
  IloNumToNumSegmentFunction** _costMatrix; 
};



TDTSPInstanceFormat::TDTSPInstanceFormat(const char* nameInstance, const char* namePattern) 
  :_env        (IloEnv())
  ,_nbVertices (0)
  ,_nbTimeSteps(0)
  ,_nbClusters (0)
  ,_Horizon    (0)
  ,_dist       (0)
  ,_times      (0)
  ,_speed      (0)
  ,_clusters   (0)
  ,_nbValues   (0)
  ,_xValues    (0)
  ,_vValues    (0)
  ,_locations  (0)
  ,_costMatrix (0)
{
  std::ifstream input(nameInstance);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << nameInstance << "'!" << std::endl;
  }
  std::ifstream input_pattern(namePattern);
  if (!input_pattern.is_open()) {
    std::cout << "Can't open file '" << namePattern << "'!" << std::endl;
  }
  
  input.ignore(256, ':');
  input >> _nbVertices;
  input.ignore(256, ':');
  input >> _nbTimeSteps;
  input.ignore(256, ':');
  input >> _nbClusters;
  input.ignore(256, ':');
  
  _dist= new IloNum*[_nbVertices+2];
  _times= new IloNum*[_nbTimeSteps];
  _speed= new IloNum*[_nbTimeSteps];
  _clusters= new IloInt*[_nbVertices+2];
  
  IloNum temp;
  
  for(IloInt i=0; i < _nbVertices+2; i++){
   _dist[i]= new IloNum[_nbVertices+2];
   for(IloInt j=0; j < _nbVertices+2; j++){
    input >> temp;
    _dist[i][j]= temp;
    }
  }
  input.ignore(256, ':');
  for(IloInt i=0; i < _nbTimeSteps; i++){
    _times[i]= new IloNum[2];
    input >> temp;
    _times[i][0]= temp;
    input >> temp;
    _times[i][1]= temp;
  }
  _Horizon= _times[_nbTimeSteps-1][1]+1;
  input.ignore(256, ':');
  for(IloInt i=0; i < _nbTimeSteps; i++){
   _speed[i]= new IloNum[_nbClusters];
   for(IloInt j=0; j < _nbClusters; j++){
    input >> temp;
    _speed[i][j]= temp;
    }
  }
  input.ignore(256, ':');
  input.ignore(256, ':');
  input.ignore(256, ':');
  input.ignore(256, ':');
  for(IloInt i=0; i < _nbVertices+2; i++){
   _clusters[i]= new IloInt[_nbVertices+2];
   for(IloInt j=0; j < _nbVertices+2; j++){
    input >> _clusters[i][j];
   }
  }
  input.close();
  
  //READ JAM PATTERN
  
  for(IloInt k=0; k < _nbTimeSteps; k++){
   for(IloInt c=0; c < _nbClusters; c++){
      input_pattern >> temp;
      _speed[k][c]= _speed[k][c]*temp;
    }
  }
  input_pattern.close();

  //INITIALIZE XVALUES, VVALUES AND NBVALUES
  _nbValues= new IloInt*[_nbVertices+2];
  _xValues = new IloNumArray*[_nbVertices+2];
  _vValues = new IloNumArray*[_nbVertices+2];
  _costMatrix = new IloNumToNumSegmentFunction*[_nbVertices+2];
  for(IloInt i=0; i < _nbVertices+2; i++){
   _nbValues[i]= new IloInt[_nbVertices+2];
   _xValues[i] = new IloNumArray[_nbVertices+2];
   _vValues[i] = new IloNumArray[_nbVertices+2];
   _costMatrix[i] = new IloNumToNumSegmentFunction[_nbVertices+2];
   for(IloInt j=0; j < _nbVertices+2; j++){
    _xValues[i][j] = IloNumArray(_env);
    _vValues[i][j] = IloNumArray(_env);
    
    IloInt steps=0,c=_clusters[i][j]-1;
    if(i!=j && _dist[i][j] !=0 && c!=-1){
      for(IloInt k=0; k< _nbTimeSteps-1; k++){
        IloNum t= _dist[i][j]/_speed[k][c];
        _xValues[i][j].add(_times[k][0]);
        _vValues[i][j].add(t);
        steps++;
        if(_times[k][0] + t < _times[k+1][0]){
          _xValues[i][j].add(_times[k+1][0]-t);
          _vValues[i][j].add(t);
          steps++;
        }
        else if(_times[k][0] + t > _times[k+1][0]){
          IloInt nextK= k+1;
          IloNum distLeft, tLeft= t- _times[nextK][0];
          while(tLeft > 0 && nextK < _nbTimeSteps-1){
            t=_times[nextK][0];
            distLeft= tLeft*_speed[nextK-1][c];
            tLeft= distLeft/_speed[nextK][c];
            nextK++;
            if(t + tLeft > _times[nextK][0]){
              tLeft= t + tLeft - _times[nextK][0];
            }
            else{
              t+= tLeft;
              tLeft=0;
            }
          }
          if( _vValues[i][j][steps-1] > t + EPSILON || _vValues[i][j][steps-1] < t- EPSILON ){
            _vValues[i][j][steps-1]= t;
          }
        }
      }
      if(_xValues[i][j][steps-1] < _times[_nbTimeSteps-1][0]){
        _xValues[i][j].add(_times[_nbTimeSteps-1][0]);
        _vValues[i][j].add(_dist[i][j]/_speed[_nbTimeSteps-1][c]);
        steps++;
      }
      _xValues[i][j].add(_Horizon);
      _vValues[i][j].add(_dist[i][j]/_speed[_nbTimeSteps-1][c]);
      steps++;
    }
    else{
      _xValues[i][j].add(0);
      _xValues[i][j].add(_Horizon);
      _vValues[i][j].add(0);
      _vValues[i][j].add(0);
      steps=2;
    }
    if(_vValues[i][j].getSize() !=steps || _vValues[i][j].getSize() != _xValues[i][j].getSize())
      std::cout << "error" << std::endl;
    
  for(int k=0; k<steps; k++){
    _xValues[i][j][k]*=FACTOR_CONVERSION;
    _vValues[i][j][k]*=FACTOR_CONVERSION;
  }
    _nbValues[i][j]=steps;
    _costMatrix[i][j]= IloNumToNumSegmentFunction(_env, 0, _Horizon, 0);
    _costMatrix[i][j].setPoints(_xValues[i][j], _vValues[i][j]);
    }
  }
  _locations = new IloInt[_nbVertices+2];
  for(IloInt i=0; i < _nbVertices+2; i++) {
    _locations[i]=i;
  }
  // cout << _costMatrix[0][26] << endl;
  // cout << _costMatrix[10][4].getValue(393) << endl;
    
  _Horizon*=FACTOR_CONVERSION;
}

void TDTSPInstanceFormat::print(const char* nameInstance, const char* namePattern){
  std::cout << _nbVertices+2 << std::endl;
  std::cout << _Horizon << std::endl;
  
  for(IloInt i=0; i < _nbVertices+2; i++){
    for(IloInt j=0; j < _nbVertices+2; j++){
      std::cout << "(" << i << "," << j << ") " << _nbValues[i][j];
      for(IloInt k=0; k < _nbValues[i][j]; k++){
        std::cout << " ";
        std::cout << _xValues[i][j][k];
        std::cout << " ";
        std::cout << _vValues[i][j][k];
        std::cout << " ";
        if(k== _nbValues[i][j]-1) std::cout << _vValues[i][j][k];
        else std::cout << _vValues[i][j][k+1];
      }
      std::cout << std::endl;
    }
  }
  
}

void TDTSPInstanceFormat::solve(const char* nameSol) {
  IloEnv env;
  try {
    IloModel model(env);
    IloIntArray locationsArray(env, _nbVertices+2);
    for(IloInt i=0; i < _nbVertices+2; i++)
      locationsArray[i]=_locations[i];

    IloIntervalVarArray stops(env,_nbVertices+2);
    IloIntVarArray travelTime(env, _nbVertices+1);
 
    IloIntVar totalTravelTime(env);
    totalTravelTime.setName("totalTT");
    char name[64];
    for(IloInt i=0; i< _nbVertices+1; i++){ 
      // stops[i] = IloIntervalVar(env);
      stops[i] = IloIntervalVar(env, (IloInt) 0);
      sprintf(name, "%ld", i);
      stops[i].setName(name);
      stops[i].setEndMax(_Horizon-1);
      travelTime[i] = IloIntVar(env, 0, _Horizon-1);
      sprintf(name, "TT%ld", i);
      travelTime[i].setName(name);
      
    }
    stops[_nbVertices+1] = IloIntervalVar(env, (IloInt) 0);
    sprintf(name, "%ld", _nbVertices+1);
    stops[_nbVertices+1].setName(name);
    stops[_nbVertices+1].setEndMax(_Horizon-1);
    IloIntervalSequenceVar tour(env, stops, locationsArray);
    tour.setName("Tour");  
     
//---------------------------- CONSTRAINTS

    
    model.add(IloFirst(env,tour, stops[0]));
    model.add(IloLast(env,tour, stops[_nbVertices+1]));
    model.add(IloStartOf(stops[0]) == 0);
    model.add(totalTravelTime== 0);
    model.add(IloTDNoOverlapPiecewiseLinear(env, tour, totalTravelTime, _costMatrix, _nbVertices+2, _Horizon, 0));
    
    
//------------------------------ OBJECTIVE
   IloIntExpr obj= IloStartOf(stops[_nbVertices+1]);
   model.add(IloMinimize(env, obj));
    
    IloIntervalSequenceVarArray tours(env);
    tours.add(tour);  
    
    IloCP cp(model);

//-------------------------------PARAMETERS------------------------------------
    // cp.setParameter(IloCP::SolutionLimit, 1);
    cp.setParameter(IloCP::TimeLimit, TIME_LIMIT);
    // cp.setParameter(IloCP::SearchType, IloCP::DepthFirst);
    cp.setParameter(IloCP::Workers, 1);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    // cp.setParameter(IloCP::FailureDirectedSearch, IloCP::Off);
//-------------------------------SOLVE-----------------------------------------  
    
    // cp.propagate();
    // cp.prettyPrintSchedule(std::cout);
    
    cp.startNewSearch(IloSearchPhase(env, tours));
    IloBool solutionFound=IloFalse;
    
    /*
    while(cp.next()){
      solutionFound=IloTrue;
      std::cout << "* " << cp.getInfo(IloCP::SolveTime) << " " << cp.getObjValue() << std::endl;
    }
    */
    
    // We hack here a little bit for now to get rid of the large amount of time spent at extraction for checking TI
    IloNum t0, t1;
    if (cp.next()) {
      solutionFound=IloTrue;
      t0 = cp.getInfo(IloCP::SolveTime);
      IloNum extrTime = cp.getInfo(IloCP::ExtractionTime);
      t1 = t0 - extrTime;
      std::cout << "# EXTRACTION TIME: " << extrTime << std::endl;
      std::cout << "* " << t1 << " " << cp.getObjValue();
      // if (cp.getObjValue() > _nbTimeWindows*_lengthTimeWindow)
      //  std::cout << " -1";
      std::cout << std::endl;
      cp.setParameter(IloCP::TimeLimit, TIME_LIMIT+t0-t1);
      while (cp.next()){
        std::cout << "* " << cp.getInfo(IloCP::SolveTime)-t0+t1 << " " << cp.getObjValue();
        // if(cp.getObjValue() > _nbTimeWindows*_lengthTimeWindow)
        //   std::cout << " -1";
        std::cout << std::endl;
      }
    }
    cp.endSearch();
    
    if (!solutionFound) {
      // cp.refineConflict();
      // cp.writeConflict(std::cout);
      std::cout << "# NO SOLUTION FOUND" << std::endl;
    } 
    else {
      std::cout << "# SOLUTION FOUND" << std::endl;
    }
    if(DEBUG) {
      std::cout <<  cp.getInfo(IloCP::NumberOfFails) << std::endl;
    }
    else{
      if (cp.getInfo(IloCP::FailStatus)==IloCP::SearchStoppedByLimit || cp.getInfo(IloCP::SolveTime) >= TIME_LIMIT) {
        std::cout << "# OPTIMALITY NOT PROVED" << std::endl;
      } else {
        std::cout << "# OPTIMALITY PROVED" << std::endl;
      }
      std::cout << "# BEST SOLUTION: " << cp.getObjValue() << std::endl;
      std::cout << "# NUMBER OF BRANCHES: " << cp.getInfo(IloCP::NumberOfBranches) << std::endl;
      std::cout << "# END TIME: " << cp.getInfo(IloCP::SolveTime) << std::endl;
      std::cout << "## Solution: " << std::endl;
      std::cout << _nbVertices << std::endl;
      IloInt s1=0, counter=0; 
      for(IloIntervalVar a = cp.getFirst(tour); a.getImpl()!=0; a =cp.getNext(tour, a)){ 
        s1=cp.getStart(a);
        s1/=FACTOR_CONVERSION;
        counter++;
        if(counter== _nbVertices+2)
           std::cout <<"# BEST TD SOLUTION: " << s1 << std::endl;
        else
          std::cout << a.getName() << " " << s1 << std::endl;
      }
      // CPChecker(cp, tour);
    }
    
  } catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
}


int main(int argc, char* argv[]) {
    TDTSPInstanceFormat tdtsp(argv[1], argv[2]); 
    // tdtsp.print(argv[1],argv[2]);
    tdtsp.solve(NULL);
}


