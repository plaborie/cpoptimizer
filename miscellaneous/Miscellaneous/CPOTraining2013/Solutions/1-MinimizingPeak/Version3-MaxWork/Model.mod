/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Minimizing resource peak usage - Version 3
 ****************************************************/

using CP;

int n = 100;
int horizon = 500;
int capMax = 306;

dvar interval a[i in 1..n] optional in 0..horizon size i;
cumulFunction level = sum(i in 1..n) pulse(a[i],n-i);

maximize sum(i in 1..n) presenceOf(a[i]);
subject to {
  level <= capMax;
}



