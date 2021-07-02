/****************************************************
 * ILOG CP Optimizer Training
 *
 * Minimizing resource peak usage - Version 2
 ****************************************************/

using CP;

int n = 100;
int horizon = 500;
int capMax = 500;

dvar interval a[i in 1..n] in 0..horizon size i;

dvar interval cover in 0..horizon size horizon;

cumulFunction free = pulse(cover, 0, capMax);
cumulFunction level = sum(i in 1..n) pulse(a[i],n-i) + free;

dexpr int peak = capMax - heightAtStart(cover, free);

minimize peak;
subject to {
  level <= capMax;
}