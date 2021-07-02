/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Minimizing resource peak usage - Version 1
 ****************************************************/

int n = 100;
int horizon = 500;
int capMax = 500;

dvar int start[i in 1..n][t in 0..horizon] in 0..1; // Whether activity i starts at time t
dvar int peak in 0..capMax;
minimize peak;
subject to {
  forall(i in 1..n) { 
    sum (t in 0..horizon-i) start[i][t]==1;
    forall(t in horizon-i+1..horizon)
      start[i][t] == 0;
  }
  forall (t in 0..horizon) {
    sum (i in 1..n, t2 in maxl(0, t-i+1)..t) start[i][t2]*(n-i) <= peak;
  }
};

