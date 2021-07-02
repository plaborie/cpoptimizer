#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <time.h>
#include <math.h>

using namespace std;
int BENCH_SIZE= 255;
int nbVertices, nbTimeWindows, lengthTimeWindow;
// int* tdcost[BENCH_SIZE][BENCH_SIZE];

void printPerm(int* perm){
  for(int i=0; i < BENCH_SIZE; i++){
    cout << perm[i] << " ";
  }
  cout << endl;
}


int * permute(int n) {
  int *a= new int[n];
  int k;
  for (k= 0; k < n; k++)
    a[k] = k;
  for (k= n-1; k > 0; k--) {
    int j = rand() % (k+1);
    int temp = a[j];
    a[j] = a[k];
    a[k] = temp;
  }
  return a;
}

 void printSimpleTDTSPInstance(char* fileName, int size){
  ofstream output(fileName);
  if (!output.is_open()) {
    std::cout << "Can't open file '" << fileName << "'!" << endl;
  }
  int* perm=permute(BENCH_SIZE);
  // int duration=1; //fixed visit durations of 1 
  int forbiddenTW=0, precConstraints=0;
  // printPerm(perm);
  output << size << " " << precConstraints << endl; 
  for(int i=0; i<size; i++){
  int duration=60 + rand()%240;
    output << perm[i] << " " << duration << " " << forbiddenTW << endl;
  }
  output.close();
}

void setFileNames(char* fileName, int cnt, int j){
	char a[10];
	strcpy(fileName, "instance_tdtsp_randomd_");
	sprintf(a, "%d", cnt);
	strcat(fileName, a);
	strcat(fileName, "_");
	sprintf(a, "%d", j);
	strcat(fileName, a);
	strcat(fileName, ".txt");
}

int main(int argc,  const char* argv[]) {
	// readInFile(argv[1]);
	char fileName[100];
 	srand(1);
  int nbInstances=atoi(argv[2]);
  int sizeMinInstances=atoi(argv[1]);
	for(int cnt=sizeMinInstances; cnt<sizeMinInstances+1; cnt+=10){
		for(int j=0; j<nbInstances; j++){
			setFileNames(fileName, cnt, j);
			nbVertices=cnt;
			printSimpleTDTSPInstance(fileName, cnt);
		}
	}
	return 0;
}