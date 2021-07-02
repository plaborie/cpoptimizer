/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Minimizing resource peak usage - Version 2
 ****************************************************/

using CP;

int n = 100;
int horizon = 500;
int capMax = 500;

dvar interval a[i in 1..n] in 0..horizon size i;
cumulFunction level = sum(i in 1..n) pulse(a[i],n-i);
dvar int peak in 0..capMax;

execute {
  //cp.setSearchPhases(cp.factory.searchPhase(peak));
}  

minimize peak;
subject to {
  level <= peak;
}
