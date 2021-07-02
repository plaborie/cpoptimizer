/****************************************************
 * ILOG CP Optimizer Training
 *
 * Steelmill inventory problem
 ****************************************************/

using CP;

int nbOrders   = ...;
int nbSlabs = ...;
int nbColors   = ...;
int nbCap      = ...;
int capacities[1..nbCap] = ...;
int weight[1..nbOrders] = ...;
int colors[1..nbOrders] = ...;

/****************************************************
 *
 * TODO: CP Optimizer model
 ****************************************************/

int capmax = max (i in 1..nbCap) capacities[i];

dvar int slabs[1..nbSlabs] in 0..capmax; //capacity of bins
dvar int orders[1..nbOrders] in 1..nbSlabs;

execute {
  var f = cp.factory;
  var phase = f.searchPhase(orders);
  cp.setSearchPhases(phase);
};
 
 constraints {
   pack(slabs, orders, weight);
   forall(i in 1..nbSlabs)
    sum (o in 1..nbColors) (or (j in 1..nbOrders : colors[j] == o)(orders[j] == i)) <= 2;
 };