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
ILOSTLBEGIN

IloBool Contains(const char* line, const char* str) {
  return (0 != strstr(line, str)); 
}

class TDTSPChecker {
public:
	TDTSPChecker(const char* nameBench, const char* nameInstance);
  void solChecker(const char* nameSol);
  IloBool isFeasibleWithTW(const char* nameSol);
  
private:
  IloInt     _nbVertices;
	IloInt     _nbTimeWindows;
	IloInt     _lengthTimeWindow;
  IloInt**   _windows;
  IloInt*    _nbWindows;
  IloInt*    _visits;
  IloInt*    _indices;
	IloInt***  _cost;    // benchSize*benchSize*nbTimeWindows
  IloInt*    _duration;
  IloInt     _v0;
};

IloBool TDTSPChecker::isFeasibleWithTW(const char* nameSol){
  ifstream input(nameSol);
  if (!input.is_open()) {
    std::cerr << "Can't open file '" << endl;
  }
  else {
    std::cout << nameSol << " is:" << endl;
  }
  IloInt nbV, visit, start, index;
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  while (input.getline(line, lineLimit)) {
    if (Contains(line, "## Solution:")) {
      input >> nbV;
      for(IloInt i=0; i< nbV; i++){
        input >> visit;
        input >> start;
        index= _indices[visit];
        for(IloInt j=0; j< _nbWindows[index]; j+=2){
          /*/debug 
          std::cout << visit << " " << start << endl; 
          std::cout << "TW: " << _windows[index][j] << " " << _windows[index][j+1] << endl;
          *///end debug
          if(start > _windows[index][j] && start < _windows[index][j+1]){
            std::cout << "Infeasible with TW constraints" << endl;
            return false;
          }
        }
      }
      
    }
  }
  std::cout << "Feasible with TW constraints" << endl;
  return true;  
}

void TDTSPChecker::solChecker(const char* nameSol){
  ifstream input(nameSol);
  if (!input.is_open()) {
    std::cerr << "Can't open file '" << endl;
  }
  else {
    input >> _nbVertices;
    std::cout << "Starting to check" << endl;
  }
  IloInt previousVisit, visit, startOfVisit, startOfPrevious=0,totalTime=0, possTT=0, possTime, totalDuration=0;
  previousVisit=_v0;
  for(IloInt i=0; i<=_nbVertices; i++){
    if(i==_nbVertices){
      input >> startOfVisit;
      visit= _v0;
    }
    else{
      input >> visit;
      input >> startOfVisit;
    }
    if (previousVisit!=visit){ 
      totalTime+= _duration[_indices[previousVisit]];
      possTT= _cost[previousVisit][visit][totalTime/_lengthTimeWindow];
      possTime= totalTime + possTT;
      //check for minimum transition times
      if(startOfVisit < (startOfPrevious + possTT + _duration[_indices[previousVisit]])) std::cout << "ERROR!! previousStart: " << startOfPrevious << " tt=" << possTT << " possible start=" << (startOfPrevious + possTT + _duration[_indices[previousVisit]]) << " unfeasible start= " << startOfVisit  << std::endl;
      if(startOfVisit != possTime){
        // std::cout << "startOfVisit!= possTime " << startOfVisit << " " << possTime << std::endl;
        
        for(IloInt t=totalTime; t <= startOfVisit; t++){
          if( _cost[previousVisit][visit][t/_lengthTimeWindow] + t < possTime)
          {
            // std::cout << t << " " << _cost[previousVisit][visit][t/_lengthTimeWindow] << std::endl;
             possTT= _cost[previousVisit][visit][t/_lengthTimeWindow] + t - totalTime;
             possTime= _cost[previousVisit][visit][t/_lengthTimeWindow] + t;
          }
        }
      }
      if(possTime < startOfVisit ){
        // std::cout << "Start of visit " << startOfVisit << " could be " << possTime << std::endl;
        // std::cout << "Not optimal transition time "<<startOfVisit-startOfPrevious-_duration[i] <<" between: " << previousVisit << " and " << visit << endl;
        // std:: cout << "Optimal is: " << possTT << " taken at " << possTime - possTT << endl;
        
      }
      else if(possTime > startOfVisit)
        std::cout << "Not possible departure from visit" << endl;
        
      //check for time-window constraints
      //check for precedence constraints
    }
    totalTime+= possTT;
    std::cout << totalTime << " " << startOfVisit << endl;
    previousVisit=visit;
    startOfPrevious= startOfVisit;
    totalDuration+= _duration[_indices[previousVisit]];
  }
  
  std::cout << "End of checker" << endl;
  std::cout << "End time: " << totalTime << endl;
  std::cout << "Total transition time: " << totalTime - totalDuration<< endl;
  
    
  input.close();  
  
}

TDTSPChecker::TDTSPChecker(const char* nameBench, const char* nameInstance) 
  :_nbVertices       (0)
	,_nbTimeWindows    (0)
	,_lengthTimeWindow (0)
	,_nbWindows        (0)
	,_windows          (0)
	,_visits           (0)
	,_indices          (0)
	,_cost             (0)
	,_duration         (0)
  ,_v0               (0)
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
  _indices= new IloInt[260];
  _duration= new IloInt[_nbVertices+1];
  _windows= new IloInt*[_nbVertices];
  _nbWindows= new IloInt[_nbVertices];
  for(i=0; i<_nbVertices; i++){
    input >> _visits[i];
    _indices[_visits[i]]= i;
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
  
  _cost    = new IloInt**[benchSize];
  for(i=0; i<benchSize; i++){
    _cost   [i] = new IloInt*[benchSize];
    for(j=0; j<benchSize; j++) {
      _cost[i][j] = new IloInt[_nbTimeWindows];
      for (k=0; k<_nbTimeWindows; ++k)
        input_bench >> _cost[i][j][k];
    }
  }
  input.close();
  input_bench.close();
  _v0= _visits[0];
  _duration[_nbVertices]=0;
}

  
int main(int argc, char* argv[]) {
		TDTSPChecker tdtsp(argv[1], argv[2]); 
    tdtsp.solChecker(argv[3]); 
    // tdtsp.isFeasibleWithTW(argv[3]); 
}


