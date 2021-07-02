/*********************************************
 * OPL 12.3 Model
 * 
 * Creation Date: Sep 30, 2011 at 10:15:32 AM
 *********************************************/

/* Addition chain puzzle.
 
 Let a sequence v[i], i in Z^+, such that :
  * v[0] = 1
  * for all k > 1, there exists i,j<k such that v[k] = v[i] + v[j]
 
 Given a target S in Z^+, find the smallest position x such that v[x]=S.

*/

using CP;
 
int S = 10000; //10000;

// Compute [pmin,pmax]: Lower/upper bounds on optimal position
int pmin = ftoi(floor(lg(S)));
int pmax = 2*pmin;
 
tuple Index {
  int position;
  int last;
};
 
{Index} Indices = {<p,l> | p in 1..pmax, l in 0..1};

// v is strictly growing by at least 1
// target+pmax-pmin: once target obtained we constrain growth=1
dvar int v[k in 0..pmax] in (k+1)..(S+pmax-pmin);

// Indices used for building v[p]
// v[p] == v[ k[<p,0>] ] + v[ k[<p,1>] ]
dvar int k[i in Indices] in 0..maxl(i.position-1, 0);

dvar int x in pmin..pmax; // v cannot grow faster

execute {
  var f = cp.factory;
  cp.setSearchPhases(f.searchPhase(k))
}

minimize x;

subject to {
  // Initial and target values
  v[0] == 1;
  v[x] == S;
  
  forall(p in 1..pmax) {
    
    v[p] == v[k[<p,0>]] + v[k[<p,1>]];
    v[p-1] < v[p];
  }    
}  

int solv [p in 0..pmax] = v[ p ];
int solv0[p in 1..pmax] = v[ k[<p,0>] ];
int solv1[p in 1..pmax] = v[ k[<p,1>] ];

execute {
  var optimal = (cp.info.FailStatus == 13);
  writeln("Objective value: " + cp.getObjValue() +  ",\tOptimality Proof: " + optimal);
  for (var p=1; p<=x; ++p) {
    writeln(solv[p], " = ", solv0[p], " + ", solv1[p]);
  }    
}  