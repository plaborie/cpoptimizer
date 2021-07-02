#include <ilcp/cp.h>
#include <../team/TDTSP/src/ttime.cpp>
#include <../team/TDTSP/src/tdnooverlapct.cpp>

IloInt raf= 1;

IloBool Contains(const char* line, const char* str) {
  return (0 != strstr(line, str)); 
}

IloInt GetIntAfterColumn(char* line) {
  char* str = strchr(line, ':');
  if ((str==0) || (str && !*(str+1))) // no ':' or ':' at the end of the line
    return 0;
  str++;
  ILOSTD_ISTREAM post(str);
  IloInt result = 0;
  post >> result;
  return result;
}


class TDTSPInstance {
public:
	TDTSPInstance(const char* nameBench, const char* nameInstance, const char* nameSol);
	TDTSPInstance(const char* nameBench, const char* nameInstance);
  void printInstance();
  void printSolution();
  
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
  IloInt**   _scheduled;
  IloInt     _totalCost;
};


TDTSPInstance::TDTSPInstance(const char* nameBench, const char* nameInstance, const char* nameSol) 
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
  ,_scheduled        (0)
  ,_totalCost        (0)
{
  ifstream input(nameInstance);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << nameInstance << "'!" << endl;
  }
  ifstream input_bench(nameBench);
  if (!input_bench.is_open()) {
    std::cout << "Can't open file '" << nameBench << "'!" << endl;
  }
  ifstream input_sol(nameSol);
  if (!input_sol.is_open()) {
    std::cout << "Can't open file '" << nameSol << "'!" << endl;
  }
  IloInt benchSize;
  IloInt nbWindows=0;
  IloInt nbPrec=0;
  IloInt i,j,k;
  
  input_bench >> benchSize;
  input_bench >> _nbTimeWindows;
  input_bench >> _lengthTimeWindow;
  
  _cost    = new IloInt**[benchSize];
  for(i=0; i<benchSize; i++){
    _cost   [i] = new IloInt*[benchSize];
    for(j=0; j<benchSize; j++) {
      _cost[i][j] = new IloInt[_nbTimeWindows];
      for (k=0; k<_nbTimeWindows; ++k)
        input_bench >> _cost[i][j][k];
    }
  }
  // for(i=222; i<223; i++){
    // for(j=9; j<10; j++) {
      // for (k=0; k<_nbTimeWindows; ++k)
        // cout << _cost[i][j][k] << " ";
    // }
  // }
  input_bench.close();
  
 
  input >> _nbVertices;
  input >> nbPrec;
  _visits= new IloInt[_nbVertices];
  _duration= new IloInt[benchSize];
  _windows= new IloInt*[benchSize];
  _nbWindows= new IloInt[benchSize];
  for(i=0; i<_nbVertices; i++){
    input >> _visits[i];
    input >> _duration[_visits[i]];
    input >> nbWindows;
    if(nbWindows!=0){
      _nbWindows[_visits[i]]= nbWindows;
      _windows[_visits[i]]= new IloInt[2*nbWindows];
      for(j=0; j<2*nbWindows; j++){
        input >> _windows[_visits[i]][j];
      }
    } 
    else _nbWindows[_visits[i]]= 0;
  }
  input.close();
  
    
  const IloInt lineLimit = 131072;
  char line[lineLimit];
  IloInt r = 0;
  while (r<2 && input_sol.getline(line, lineLimit)) {
    if (Contains(line, "BEST SOLUTION") || Contains(line, "BEST TD SOLUTION")) {
      _totalCost = GetIntAfterColumn(line);
      r++;
    }
    else if (Contains(line, "Solution")) {
      IloInt nbVertices;
      input_sol >> nbVertices;
      _scheduled= new IloInt*[2];
      _scheduled[0]= new IloInt[nbVertices]; //visit ID
      _scheduled[1]= new IloInt[nbVertices]; //visit start date
      for(IloInt i=0; i< nbVertices; i++){
        input_sol >> _scheduled[0][i];
        input_sol >> _scheduled[1][i];        
      }
      r++;
    }
  }
  input_sol.close();
 
  _horizon = (_totalCost/60 + 1)*60;
  // _horizon = _nbTimeWindows*_lengthTimeWindow;
    
}

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
  ,_scheduled        (0)
  ,_totalCost        (0)
{
  ifstream input(nameInstance);
  if (!input.is_open()) {
    std::cout << "Can't open file '" << nameInstance << "'!" << endl;
  }
  ifstream input_bench(nameBench);
  if (!input_bench.is_open()) {
    std::cout << "Can't open file '" << nameBench << "'!" << endl;
  }
 
  IloInt benchSize;
  IloInt nbWindows=0;
  IloInt nbPrec=0;
  IloInt i,j,k;
  
  input_bench >> benchSize;
  input_bench >> _nbTimeWindows;
  input_bench >> _lengthTimeWindow;
  
  _cost    = new IloInt**[benchSize];
  for(i=0; i<benchSize; i++){
    _cost   [i] = new IloInt*[benchSize];
    for(j=0; j<benchSize; j++) {
      _cost[i][j] = new IloInt[_nbTimeWindows];
      for (k=0; k<_nbTimeWindows; ++k)
        input_bench >> _cost[i][j][k];
    }
  }
  input_bench.close();
  
 
  input >> _nbVertices;
  input >> nbPrec;
  _visits= new IloInt[_nbVertices];
  _duration= new IloInt[benchSize];
  _windows= new IloInt*[benchSize];
  _nbWindows= new IloInt[benchSize];
  for(i=0; i<_nbVertices; i++){
    input >> _visits[i];
    input >> _duration[_visits[i]];
    input >> nbWindows;
    if(nbWindows!=0){
      _nbWindows[_visits[i]]= nbWindows;
      _windows[_visits[i]]= new IloInt[2*nbWindows];
      for(j=0; j<2*nbWindows; j++){
        input >> _windows[_visits[i]][j];
      }
    } 
    else _nbWindows[_visits[i]]= 0;
  }
  input.close();
  _horizon = 46800;
    
}


void TDTSPInstance::printSolution(){
   // Picture environment
  cout << endl;
  cout << "\\begin{figure}" << endl;
  cout << "\\centering" << endl;
  cout << "\\scriptsize" << endl;
  IlcFloat yscale = 0.3;
  // if (yscale > 0.3)
    // yscale = 0.3;
  // else if(yscale < 0.1) yscale= 0.1;
  IlcFloat xscale = 0.0015;//(IlcFloat)19 / _horizon;
  cout << "\\begin{tikzpicture}[x=" << xscale << "cm, " << "y=" << yscale << "cm]" << endl;
  cout << "\\pgfsetlinewidth{0.3pt}" << endl;

  cout << "% Vertical lines: " << endl;
  IloNum step, scalefactor;
  if (_nbVertices <= 20){ step = 750; scalefactor=1;}
  else { step = 1500; scalefactor=5;} 
  // else if (_horizon < 60000)  step= 3000;
  IlcInt first = 0;
  cout << "%Horizon: " << _horizon << endl;
  for (IlcInt i = 0; i <= _horizon / step; i++) {
    cout << "\\draw[lightgray] (" << (first + i*step)/scalefactor << ", -0.3) -- (" << (first + i*step)/scalefactor << ", " << _nbVertices + 1 +0.5 << ")" 
      << " node[above=0.3] {$" << first + i*step << "$};" << endl;
  }
  cout << endl;
  
  for(IloInt i =0; i< _nbVertices ; i++){
    for(IloInt j=0; j<2*_nbWindows[_scheduled[0][i]]; j+=2){
      if(_windows[_scheduled[0][i]][j] < _horizon){
        cout << "\\def\\twstart{"<<_windows[_scheduled[0][i]][j]/scalefactor<<"}" << endl;
        if(_windows[_scheduled[0][i]][j+1] <= _horizon)
          cout << "\\def\\twend{"<<_windows[_scheduled[0][i]][j+1]/scalefactor<<"}" << endl;
        else
          cout << "\\def\\twend{"<<_horizon/scalefactor<<"}" << endl;
        cout << "\\def\\twypos{"<<_nbVertices - i<<"}" << endl;
        cout << "\\drawtw" << endl;
      }
    }
  }
  
  IloInt possTTStart, possTTDur, idVisit, dateVisit, idNext;
  for(IloInt i =0; i< _nbVertices ; i++){
    // IloInt start;
    idVisit= _scheduled[0][i];
    dateVisit= _scheduled[1][i];
    cout << "\\def\\visitid{"<< idVisit <<"}" << endl;
    cout << "\\def\\visitstart{"<<dateVisit/scalefactor<<"}" << endl;
    cout << "\\def\\visitdur{"<<_duration[idVisit]/scalefactor<<"}" << endl;
    
    possTTStart= dateVisit+_duration[idVisit];
    if((i+1)!=_nbVertices) idNext= _scheduled[0][i+1];
    else idNext= _scheduled[0][0];
    possTTDur= _cost[idVisit][idNext][(dateVisit +_duration[idVisit])/_lengthTimeWindow];
    
    // for(IloInt j= possTTStart; j < possTTDur + possTTStart; j++){
      // if( j+ _cost[idVisit][idNext][j/_lengthTimeWindow] < possTTStart + possTTDur){
        // possTTDur = _cost[idVisit][idNext][j/_lengthTimeWindow];
        // possTTStart= j;
      // }
    // }
    cout << "\\def\\visitttstart{"<<possTTStart/scalefactor<<"}" << endl;
    cout << "\\def\\visitttdur{"<<possTTDur/scalefactor<<"}" << endl;
    
    IloInt nbColors=5;
    if(_scheduled[0][i]%nbColors==0)
      cout << "\\def\\visitcolor{green}" << endl;
    else if(_scheduled[0][i]%nbColors==1)
      cout << "\\def\\visitcolor{blue}" << endl;
    else if(_scheduled[0][i]%nbColors==2)
      cout << "\\def\\visitcolor{orange}" << endl;
    else if(_scheduled[0][i]%nbColors==3)
      cout << "\\def\\visitcolor{red}" << endl;
    else if(_scheduled[0][i]%nbColors==4)
      cout << "\\def\\visitcolor{pink}" << endl;
    
    cout << "\\def\\visitypos{"<<_nbVertices - i<<"}" << endl;
    cout << "\\drawvisit  " << endl;
  }
  cout << "\\end{tikzpicture}" << endl;
  cout << "\\end{figure}" << endl;
}


void TDTSPInstance::printInstance(){
   // Picture environment
  cout << endl;
  cout << "\\begin{figure}" << endl;
  cout << "\\centering" << endl;
  cout << "\\scriptsize" << endl;
  IlcFloat yscale = 0.3;
  // if (yscale > 0.3)
    // yscale = 0.3;
  // else if(yscale < 0.1) yscale= 0.1;
  IlcFloat xscale = 0.0015;//(IlcFloat)19 / _horizon;
  cout << "\\begin{tikzpicture}[x=" << xscale << "cm, " << "y=" << yscale << "cm]" << endl;
  cout << "\\pgfsetlinewidth{0.3pt}" << endl;

  cout << "% Vertical lines: " << endl;
  IlcInt step, scalefactor;
  step = 3000; scalefactor=5;
  IlcInt first = 0;
  cout << "%Horizon: " << _horizon << endl;
  for (IlcInt i = 0; i <= _horizon / step; i++) {
    if(i%5==0)
      cout << "\\draw[red] (" << (first + i*step)/scalefactor << ", -0.3) -- (" << (first + i*step)/scalefactor << ", " << _nbVertices + 1 +0.5 << ")" << " node[above=0.3] {$" << first + i*step << "$};" << endl; 
    else 
      cout << "\\draw[lightgray] (" << (first + i*step)/scalefactor << ", -0.3) -- (" << (first + i*step)/scalefactor << ", " << _nbVertices + 1 +0.5 << ")" << " node[above=0.3] {$" << first + i*step << "$};" << endl;
  }
  cout << endl;
  
  for(IloInt i =0; i< _nbVertices ; i++){
    for(IloInt j=0; j<2*_nbWindows[_visits[i]]; j+=2){
      cout << "\\def\\twstart{"<<_windows[_visits[i]][j]/scalefactor<<"}" << endl;
      cout << "\\def\\twend{"<<_windows[_visits[i]][j+1]/scalefactor<<"}" << endl;
      cout << "\\def\\twypos{"<<_nbVertices - i<<"}" << endl;
      cout << "\\drawtw" << endl;
    }
  }
  cout << "\\end{tikzpicture}" << endl;
  cout << "\\end{figure}" << endl;
}




  
int main(int argc, char* argv[]) {
    if(argc== 3){
      TDTSPInstance tdtsp(argv[1], argv[2]); //arg 1 = costs matrix arg 2 = instance  
      tdtsp.printInstance();
    }
    else{
      TDTSPInstance tdtsp(argv[1], argv[2], argv[3]); //arg 1 = costs matrix arg 2 = instance arg 3 = solution 
      tdtsp.printSolution();
    }
   }