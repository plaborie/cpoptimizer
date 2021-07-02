#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>

using namespace std;

int compareInt(const void* p1, const void* p2) {
  const int f1 = *(const int*) p1;
  const int f2 = *(const int*) p2;
  if (f1 == f2) return 0;
  return (f1 > f2) ? 1 : -1;
}


class TDTSPBench {
public:
	TDTSPBench(const char* nameBench);
  void printNewBench();
  void printBenchMirrored();
  
// private:
  int     _nbVertices;
	int     _nbTimeWindows;
	int     _lengthTimeWindow;
  int***  _cost;    // nbVertices*nbVertices*nbTimeWindows
  int***  _costMirrored;    // nbVertices*nbVertices*nbTimeWindows
	int**   _minCost; // nbVertices*nbVertices
	int**   _medianCost; // nbVertices*nbVertices
	int*    _maxCost;
};

TDTSPBench::TDTSPBench(const char* nameBench) 
  :_nbVertices       (0)
	,_nbTimeWindows    (0)
	,_lengthTimeWindow (0)
	,_cost             (0)
	,_minCost          (0)
	,_medianCost       (0)
	,_maxCost          (0)
{
  ifstream input_bench(nameBench);
  if (!input_bench.is_open()) {
    std::cout << "Can't open file '" << nameBench << "'!" << endl;
  }
  
  int i,j,k;
   
  int benchSize;
  input_bench >> benchSize;
  input_bench >> _nbTimeWindows;
  input_bench >> _lengthTimeWindow;
  _nbVertices= benchSize;
  
  int*** cost    = new int**[benchSize];
  for(i=0; i<benchSize; i++){
    cost   [i] = new int*[benchSize];
    for(j=0; j<benchSize; j++) {
      cost[i][j] = new int[_nbTimeWindows];
      for (k=0; k<_nbTimeWindows; ++k)
        input_bench >> cost[i][j][k];
    }
  }
  
  input_bench.close();
  
  int mincost = 10000;
  int maxcost = 0;
  int v;
  _cost    = new int**[_nbVertices];
  _costMirrored = new int**[_nbVertices];
  _minCost = new int* [_nbVertices];
  _medianCost = new int* [_nbVertices];
  for(i=0; i<_nbVertices; i++){
    _cost   [i] = new int*[_nbVertices];
    _costMirrored[i] = new int*[_nbVertices];
    _minCost[i] = new int [_nbVertices];
    _medianCost[i] = new int [_nbVertices];
    for(j=0; j<_nbVertices; j++) {
      _cost[i][j] = new int[_nbTimeWindows];
      _costMirrored[i][j] = new int[_nbTimeWindows*2];
      for (k=0; k<_nbTimeWindows; ++k){
        v = cost[i][j][k];
        _cost[i][j][k]= v;
        _costMirrored[i][j][k]= v;
        _costMirrored[i][j][_nbTimeWindows*2 - k -1]= v;
        if (v > maxcost && i!=j) {
          maxcost = v;
        }
        if (v < mincost && i!=j) {
          mincost = v;
        }
      }
      _minCost[i][j]=((i==j)?0:mincost);
      mincost= 10000;
    }
  }
  
  int* p;
  int mid= _nbTimeWindows/2;
  for(i=0; i<_nbVertices; i++)
    for(j=0; j<_nbVertices; j++) {
      p= cost[i][j];
      qsort(p, _nbTimeWindows, sizeof p[0], compareInt);
      _medianCost[i][j] = (p[mid-1] + p[mid]) /2;
    }
  
  
  // for(i=0; i<_nbVertices; i++){
    // for(j=0; j<_nbVertices; j++) {
      // for (k=0; k<_nbTimeWindows; ++k){
        // std::cout << _cost[i][j][k] << " ";
      // }
      // std::cout << std::endl;
    // }
  // }
}

void TDTSPBench::printNewBench(){
  int cost, newcost;
  double contrastRatio= 0.2;
  cout << _nbVertices << " " << _nbTimeWindows << " " << _lengthTimeWindow << endl;
  for(int i=0; i<_nbVertices; i++){
    for(int j=0; j<_nbVertices; j++){
      for(int k=0; k<_nbTimeWindows; k++){
        cost= _cost[i][j][k];
        if(cost == 0)
          cout << 0 << " ";
        else if( cost < _medianCost[i][j]){
          newcost=(int)(cost*(1- contrastRatio));
          if(newcost > 0)
            cout << newcost << " ";
          else cout << 0 << " ";
        }
        else if( cost >= _medianCost[i][j]){
          newcost= (int)(cost*(1+ contrastRatio));
          cout << newcost << " ";
        }
      }
      cout << endl;
    }
    cout << endl;
  }
} 

void TDTSPBench::printBenchMirrored(){
  int cost, newcost;
  cout << _nbVertices << " " << 2*_nbTimeWindows << " " << _lengthTimeWindow << endl;
  for(int i=0; i<_nbVertices; i++){
    for(int j=0; j<_nbVertices; j++){
      for(int k=0; k<2*_nbTimeWindows; k++){
        cout << _costMirrored[i][j][k] << " ";
      }
      cout << endl;
    }
    cout << endl;
  }
} 

void printMerge(TDTSPBench bench1,TDTSPBench bench2){
  int startWindowBench2=15; 
  double scaling;
  int _nbVertices= bench1._nbVertices, _lengthTimeWindow= bench1._lengthTimeWindow, _nbTimeWindows= bench1._nbTimeWindows;
  cout << _nbVertices << " " << 2*_nbTimeWindows - 15 << " " << _lengthTimeWindow << endl;
  for(int i=0; i<_nbVertices; i++){
    for(int j=0; j<_nbVertices; j++){
      for(int k=0; k<_nbTimeWindows; k++){
        cout << bench1._cost[i][j][k] << " ";
      }
      if (bench2._cost[i][j][startWindowBench2]!=0)	scaling= (double)bench1._cost[i][j][_nbTimeWindows-1]/(double) bench2._cost[i][j][startWindowBench2];
      else scaling=1;
      for(int z= startWindowBench2; z <_nbTimeWindows; z++){
        cout << (int) (scaling*bench2._cost[i][j][z]) << " ";
      }
      cout << endl;
    }
    cout << endl;
  }
}  



int main(int argc,  const char* argv[]) {
	TDTSPBench bench1(argv[1]);
	// TDTSPBench bench2(argv[2]);
  // printMerge(bench1, bench2);
  // bench1.printNewBench();
  bench1.printBenchMirrored();
	return 0;
}