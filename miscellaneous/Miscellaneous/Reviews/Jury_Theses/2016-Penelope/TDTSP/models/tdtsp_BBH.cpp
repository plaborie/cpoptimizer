#include <algorithm>
#include <stdlib.h>
#include <iostream>
#include <time.h>
#include <fstream>
#include <vector>

using namespace std;

clock_t tClock; // starting time
int TIME_LIMIT= 180;
int DUR_VISIT= 1;

class Elt{
private:
	int vertex;
	int cost;
public:
	Elt(void){}
	int getVertex(void){ return vertex;}
	int getCost(void){ return cost;}
	void setCost(int c){cost = c;}
	void setVertex(int v){vertex = v;}
	bool comp(Elt e){return cost < e.getCost();}
	void print(){
		std::cout << vertex +1 << " :" << cost << " ";
	}
};

bool comp(Elt a, Elt b) { return (a.comp(b)); };

class Graph {
private:
    const int _nbVertices; 
	const int _minArcCost;
	const int _maxArcCost;
	const int _TDminArcCost;
	const int _TDmaxArcCost;
	const int _nbTimeWindows;
	const int _lengthTimeWindow;
	const int _dur;
  int opt;
  int bestSol;
  float endTime;
  int nbBranches;
    int*** _tdcost;
	int** _cost;
	int* TSP;
	int* TDTSP;
	
	void branchAndBoundH(int i, int currentCost, std::vector<Elt> perm, int* currentBound){
		if (i == _nbVertices && currentCost < *currentBound){
				*currentBound = currentCost;
				for (int j=0; j<_nbVertices; j++)
					TSP[j]= perm[j].getVertex();
		}
		else {
			for (int j=i; j<_nbVertices; j++)
				perm[j].setCost(_cost[perm[i-1].getVertex()][perm[j].getVertex()]);
			sort(perm.begin()+i,perm.end(),comp);
			Elt ival = perm[i];
			for (int j=i; ((j<_nbVertices) && ((float)(clock()-tClock)/CLOCKS_PER_SEC < TIME_LIMIT) && 
			(currentCost + perm[j].getCost() + (_nbVertices-i)*(_minArcCost + _dur) < *currentBound)); j++){
				Elt jval = perm[j];
				perm[j] = ival;
				perm[i] = jval;
				branchAndBoundH(i+1, currentCost + _dur + jval.getCost(), perm, currentBound);
				perm[j] = jval;
				perm[i] = ival;
			}
		}
	}
	
	void TDbranchAndBoundH(int i, int currentCost, std::vector<Elt> perm, int* currentBound){
    nbBranches++;
		int currentTimeWindow = currentCost/_lengthTimeWindow;
    if(currentTimeWindow > _nbTimeWindows) currentTimeWindow= _nbTimeWindows-1;
    if (i == _nbVertices){
      currentCost+= _tdcost[perm[i-1].getVertex()][0][currentTimeWindow] + _dur;
      if(currentCost < *currentBound){
        *currentBound = currentCost;
        for (int j=0; j<_nbVertices; j++)
          TDTSP[j]= perm[j].getVertex();
        endTime= (float)(clock()-tClock)/CLOCKS_PER_SEC;
      }
    }
		else {
			for (int j=i; j<_nbVertices; j++)
				perm[j].setCost(_tdcost[perm[i-1].getVertex()][perm[j].getVertex()][currentTimeWindow]);
			//perm[_nbVertices].setCost(_cost[perm[i-1].getVertex()][1]);
			sort(perm.begin()+i,perm.end(),comp);
			Elt ival = perm[i];
      endTime = (float)(clock()-tClock)/CLOCKS_PER_SEC;
			for (int j=i; ((j<_nbVertices) && (endTime < TIME_LIMIT) && 
			(currentCost + perm[j].getCost() + (_nbVertices-i)*(_TDminArcCost + _dur) < *currentBound)); j++){
				Elt jval = perm[j];
				perm[j] = ival;
				perm[i] = jval;
				TDbranchAndBoundH(i+1, currentCost + _dur + jval.getCost(), perm, currentBound);
				perm[j] = jval;
				perm[i] = ival;
        endTime= (float)(clock()-tClock)/CLOCKS_PER_SEC;
			}
      endTime= (float)(clock()-tClock)/CLOCKS_PER_SEC;
    }
	}	
public:
	Graph(int nbVertices,int TDminArcCost,int TDmaxArcCost,int minArcCost,int maxArcCost,int nbTimeWindows,int lengthTimeWindow,int*** tdcost, int** cost, int duration)
	: _nbVertices(nbVertices),_TDminArcCost(TDminArcCost), _TDmaxArcCost(TDmaxArcCost), _minArcCost(minArcCost), _maxArcCost(maxArcCost), _nbTimeWindows(nbTimeWindows),
	_lengthTimeWindow(lengthTimeWindow), _tdcost(tdcost), _cost(cost), _dur(duration)
	{ 	TSP= new int[nbVertices];
		TDTSP= new int[nbVertices];
  }
	Graph(int nbVertices,int TDminArcCost,int TDmaxArcCost,int nbTimeWindows,int lengthTimeWindow,int*** tdcost, int duration)
	: _nbVertices(nbVertices),_TDminArcCost(TDminArcCost), _TDmaxArcCost(TDmaxArcCost), _minArcCost(0), _maxArcCost(TDmaxArcCost), _nbTimeWindows(nbTimeWindows),
	_lengthTimeWindow(lengthTimeWindow), _tdcost(tdcost), _cost(NULL), _dur(duration)
	{ 	TSP= new int[nbVertices];
		TDTSP= new int[nbVertices];
  }
	void printTDCost(void){
		for (int i=0; i<_nbVertices; i++){
			for (int j=0; j<_nbVertices; j++){
				std::cout << "i=" << i << " j=" << j << ": ";
				for (int k=0; k<_nbTimeWindows; k++)
					std::cout << _tdcost[i][j][k] << " ";
				std::cout << "\n";
			}
		}
	}
	void printCost(void){
		for (int i=0; i<_nbVertices; i++){
			std::cout << i << ": ";
			for (int j=0; j<_nbVertices; j++)
				std::cout << _cost[i][j] << " ";
			std::cout << "\n";
		}
	}
	void printTDTSPSol(){
		for(int i=0; i<_nbVertices; i++)
			cout << TDTSP[i] << " ";
		cout << endl;
	}
	void printTSPSol(){
		for(int i=0; i<_nbVertices; i++)
			cout << TSP[i] << " ";
		cout << endl;
	}
	int printTDTSPTimes(){
		int currentCost=_dur;
		int currentTimeWindow=0;
		for(int i=0; i<_nbVertices-1; i++){
			currentTimeWindow = currentCost/_lengthTimeWindow;
			currentCost+=_tdcost[TDTSP[i]][TDTSP[i+1]][currentTimeWindow]+ _dur;
		//	cout << _tdcost[TDTSP[i]][TDTSP[i+1]][currentTimeWindow] << " ";
			for (int j=0; j < _nbTimeWindows; j++){
				if(j==currentTimeWindow) cout << _tdcost[TDTSP[i]][TDTSP[i+1]][j];// << "* ";
				//else cout << _tdcost[TDTSP[i]][TDTSP[i+1]][j] << " ";
			}
			cout << endl;
		//	cout <<"from " << TDTSP[i] << " to " << TDTSP[i+1] << " at " << currentTimeWindow<<" "<< _tdcost[TDTSP[i]][TDTSP[i+1]][currentTimeWindow] << endl;
		
		}
	
		return currentCost - _dur*_nbVertices;
	}
	int printTSPTimes(){
		int currentCost=_dur;
		int currentTimeWindow;
		for(int i=0; i<_nbVertices-1; i++){
			currentTimeWindow = currentCost/_lengthTimeWindow;
			currentCost+=_tdcost[TSP[i]][TSP[i+1]][currentTimeWindow]+ _dur;
			cout << _tdcost[TSP[i]][TSP[i+1]][currentTimeWindow] << " ";
		}
	
		return currentCost- _dur*_nbVertices;
	}
  
  void printLog(const char* instanceName){
        
    cout << "# INSTANCE: " << instanceName << endl;
    cout << "# ALGORITHM: B&BH" << endl;
    cout << "# NUMBER OF VISITS: " << _nbVertices << endl;
    cout << "# NUMBER OF TIME WINDOWS: " << _nbTimeWindows << endl;
    cout << "# TIME LIMIT: " << TIME_LIMIT << endl;
    if (bestSol ==  (_nbVertices*_TDmaxArcCost + _dur*_nbVertices)) {
      cout << "# NO SOLUTION FOUND" << endl;
    } 
    else {
      cout << "# SOLUTION FOUND" << endl;
    }
    if (opt) {
      cout << "# OPTIMALITY PROVED" << endl;
    } else {
      cout << "# OPTIMALITY NOT PROVED" << endl;
    }
    cout << "# BEST SOLUTION: " << bestSol << endl;
    cout << "# NUMBER OF BRANCHES: " << nbBranches << endl;
    cout << "# END TIME: " << endTime << endl;
  }
		
	int branchAndBoundH(){
		Elt e;
		std::vector<Elt> perm(_nbVertices, e);
		for (int i=0; i<_nbVertices; i++)
			perm[i].setVertex(i);
		int currentBound = _nbVertices*_maxArcCost+ _dur*_nbVertices;
		branchAndBoundH(1, _dur, perm, &currentBound);
		return currentBound;
	}
	int TDbranchAndBoundH(){
		Elt e;
		std::vector<Elt> perm(_nbVertices, e);
		for (int i=0; i<_nbVertices; i++)
			perm[i].setVertex(i);
		int currentBound = _nbVertices*_TDmaxArcCost + _dur*_nbVertices;
    nbBranches=0;
    endTime=0;
    TDbranchAndBoundH(1, _dur, perm, &currentBound);
		if(endTime >= TIME_LIMIT) opt=0;
    else opt=1;
    bestSol= currentBound - _dur*(_nbVertices+1);
		return currentBound;
	}	
};


Graph* generateGraphs(const char* file1,const char* file2){
	int nbVertices, nbTimeWindows, lengthTimeWindow, mincost, maxcost, tdmincost, tdmaxcost;
  mincost= INT_MAX; tdmincost= INT_MAX; maxcost=0; tdmaxcost=0;
	int*** tdcost;
	int** cost;
	FILE* myfile;
	myfile=fopen(file1, "r");
	if (myfile != NULL){
		fscanf(myfile, "n= %d;", &nbVertices);
		fscanf(myfile, " m= %d;", &nbTimeWindows);
		fscanf(myfile, " sizeOfTimeStep= %d;", &lengthTimeWindow);
		fscanf(myfile, " costMatrix=[");
		tdcost = new int**[nbVertices];
		for(int i=0; i<nbVertices; i++){
			tdcost[i] = new int*[nbVertices];
			fscanf(myfile, " [");
			for(int j=0; j<nbVertices; j++){
				tdcost[i][j] = new int[nbTimeWindows];
				fscanf(myfile, " [");
				for(int k=0; k< nbTimeWindows; k++){
					fscanf(myfile," %d", &tdcost[i][j][k]);
					//tdcost[i][j][k]/=2;
					if(tdcost[i][j][k] > tdmaxcost && i!=j)
						tdmaxcost= tdcost[i][j][k];
					else if(tdcost[i][j][k] < tdmincost && i!=j){
						tdmincost= tdcost[i][j][k];
					}
				}
				fscanf(myfile, "%*s");
			}
			fscanf(myfile, "%*s");
		}
		fclose(myfile);
	}
	else cout << "Unable to open file";
	
	myfile=fopen(file2, "r");
	if (myfile != NULL){
		fscanf(myfile, "n= %d;", &nbVertices);
		fscanf(myfile, " costMatrix=[");
		cost = new int*[nbVertices];
		for(int i=0; i<nbVertices; i++){
			cost[i] = new int[nbVertices];
			fscanf(myfile, " [");
			for(int j=0; j<nbVertices; j++){
				fscanf(myfile," %d", &cost[i][j]);
				if(cost[i][j] > maxcost && i!=j)
					maxcost= cost[i][j];
				else if(cost[i][j] < mincost && i!=j){
					mincost= cost[i][j];
				}
			}
			fscanf(myfile, "%*s");
		}
		fclose(myfile);
	}
	else std::cout << "Unable to open file";
	return new Graph(nbVertices,tdmincost,tdmaxcost, mincost, maxcost,nbTimeWindows,lengthTimeWindow, tdcost, cost,DUR_VISIT);

}


Graph* generateGraph(const char* file1){
	int nbVertices, nbTimeWindows, lengthTimeWindow,tdmincost, tdmaxcost;
  tdmincost= INT_MAX; tdmaxcost=0;
	int*** tdcost;
	FILE* myfile;
	myfile=fopen(file1, "r");
	if (myfile != NULL){
		fscanf(myfile, "n= %d;", &nbVertices);
		fscanf(myfile, " m= %d;", &nbTimeWindows);
		fscanf(myfile, " sizeOfTimeStep= %d;", &lengthTimeWindow);
		fscanf(myfile, " costMatrix=[");
		tdcost = new int**[nbVertices];
		for(int i=0; i<nbVertices; i++){
			tdcost[i] = new int*[nbVertices];
			fscanf(myfile, " [");
			for(int j=0; j<nbVertices; j++){
				tdcost[i][j] = new int[nbTimeWindows];
				fscanf(myfile, " [");
				for(int k=0; k< nbTimeWindows; k++){
					fscanf(myfile," %d", &tdcost[i][j][k]);
					//tdcost[i][j][k]/=2;
					if(tdcost[i][j][k] > tdmaxcost && i!=j)
						tdmaxcost= tdcost[i][j][k];
					else if(tdcost[i][j][k] < tdmincost && i!=j){
						tdmincost= tdcost[i][j][k];
					}
				}
				fscanf(myfile, "%*s");
			}
			fscanf(myfile, "%*s");
		}
		fclose(myfile);
	}
	else cout << "Unable to open file";
	return new Graph(nbVertices,tdmincost,tdmaxcost, nbTimeWindows,lengthTimeWindow, tdcost, DUR_VISIT);
}

int main(int argc,  const char* argv[]) {
	float time;
	int tspcost, tdtspcost;
	Graph* g;
	if(argc > 2) g=generateGraphs(argv[1], argv[2]);
  else if(argc == 2) g= generateGraph(argv[1]);
	else{
    cerr << "not enough files" << endl;
    return -1;
  }
  
	tClock=clock(); // starting time B&BH
	tdtspcost= g->TDbranchAndBoundH();
	g->printLog(argv[1]);
  g->printTDTSPSol();
  g->printTDTSPTimes();
  
  /*std::cout << endl << "TDTSP: " << tdtspcost << " " << (float)(clock()-tClock)/CLOCKS_PER_SEC << endl;
	tdtspcost=g->printTDTSPTimes();
	cout << endl <<"Total TDTSP: "<< tdtspcost <<endl;*/
  
  if(argc > 2){
    time = (float)(clock()-tClock)/CLOCKS_PER_SEC;
    tClock=clock(); // starting time B&BH
    tspcost= g->branchAndBoundH();
    std::cout << endl << "TSP: " << tspcost << " " << (float)(clock()-tClock)/CLOCKS_PER_SEC << "\n";
    g->printTSPSol();
    tspcost= g->printTSPTimes();
    cout << endl << "Total TSP: "<< tspcost <<endl << endl;
    
    cout << "Gain TD: " << (tspcost-tdtspcost)*100/tspcost << "%" << endl;
    time = (float)(clock()-tClock)/CLOCKS_PER_SEC;
  }
	return 0;
}

