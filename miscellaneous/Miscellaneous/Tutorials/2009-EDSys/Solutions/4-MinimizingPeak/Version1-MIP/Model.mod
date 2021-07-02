/****************************************************
 * ILOG CP Optimizer Training
 *
 * Minimizing resource peak usage - Version 1
 ****************************************************/

int n = 100;
int horizon = 500;
int capMax = 500;

tuple AStart {
	int i;
	int s;
};

{AStart} allstarts = {<i, s> | i in 1..n, s in 0..(horizon - i)};	
	
dvar int starts[allstarts] in 0..1; // Whether activity i starts at time t

dvar float peak in 0..capMax;
minimize peak;
subject to {
  forall(i in 1..n) { 
    sum (a in allstarts : a.i == i)  starts[a] == 1;
    //forall(t in horizon-i+1..horizon)
      //start[i][t] == 0;
  }
  forall (t in 0..horizon) {
    sum (a in allstarts : a.s <= t && a.s + a.i > t) starts[a]*(n - a.i) <= peak;
  }
};

