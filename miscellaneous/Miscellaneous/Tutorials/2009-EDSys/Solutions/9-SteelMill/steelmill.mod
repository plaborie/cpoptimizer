
using CP;

int nbOrders   = ...;
int nbSlabs = ...;
int nbColors   = ...;
int nbCap      = ...;
int capacities[1..nbCap] = ...;
int weight[1..nbOrders] = ...;
int colors[1..nbOrders] = ...;

int maxLoad = sum(i in 1..nbOrders) weight[i];
int maxCap  = max(i in 1..nbCap) capacities[i];

int loss[c in 0..maxCap] = min(i in 1..nbCap : capacities[i] >= c) capacities[i] - c; 


dvar int where[1..nbOrders] in 1..nbSlabs;
dvar int load[1..nbSlabs] in 0..maxLoad;

execute{
   writeln("loss = ", loss);   
   writeln("maxLoad = ", maxLoad);   
   writeln("maxCap = ", maxCap);

}

execute {
   var f = cp.factory;
   cp.setSearchPhases(f.searchPhase(where));
}
dexpr int totalLoss = sum(s in 1..nbSlabs) loss[load[s]];

minimize totalLoss;
subject to {  
  pack(load, where, weight);
  forall(s in 1..nbSlabs)
    sum (c in 1..nbColors) (or(o in 1..nbOrders : colors[o] == c) (where[o] == s)) <= 2; 
}
