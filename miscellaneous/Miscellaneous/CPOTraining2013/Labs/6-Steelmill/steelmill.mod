/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Steelmill inventory problem
 ****************************************************/

using CP;

int nbOrders   = ...;
int nbSlabs    = ...;
int nbColors   = ...;
int nbCap      = ...;
int capacities[1..nbCap] = ...;
int weight[1..nbOrders] = ...;
int colors[1..nbOrders] = ...;

/****************************************************
 *
 * TODO: CP Optimizer model
 ****************************************************/
