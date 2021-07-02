/******************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *                                                    
 * Batch scheduling - Version 3
 ******************************************************/
 
using CP;

/*******
* Data *
********/

int numberProducts = ...;
int preparationTime = ...;
range products = 1..numberProducts;

tuple productStepT {
   key int stepNumber;
   int duration;
   int temperatureCode;
}; 
{productStepT} steps[products] = ...;

int orders[p in products] = ...;

tuple taskT {
  key int productId;
  key int stepNumber;
  key int orderId;
  int duration;
  int temperatureCode;
};
{taskT} allTasks = {<p, s.stepNumber, o, 
                     s.duration, s.temperatureCode> 
                    | p in products, s in steps[p], o in 1..orders[p]};
                    
int numberOvens = ...;
range ovens = 1..numberOvens;

int ovenCapacity[ovens] = ...;

tuple transitionTimeT {
  key int code1;
  key int code2;
  int transitionTime;
};
{transitionTimeT} transitionTimes = ...;

int maxTime = sum(t in allTasks)(t.duration + preparationTime);

/*********************
* Decision variables *
**********************/

dvar interval tasks[t in allTasks] in preparationTime..maxTime size t.duration;
dvar interval taskOvenAlts[t in allTasks][o in ovens] optional;

cumulFunction ovenUsage[o in ovens] = sum(t in allTasks) pulse(taskOvenAlts[t][o], 1);
stateFunction ovenTemperature[o in ovens] with transitionTimes;

/************
* Objective *
*************/

minimize max(t in allTasks) endOf(tasks[t]);

/**************
* Constraints *
***************/

constraints {
  // 30 minutes preparation is required between steps for each product order  
  forall(t1,t2 in allTasks : t1.productId == t2.productId &&
                             t1.stepNumber + 1 == t2.stepNumber &&
                             t1.orderId == t2.orderId) {
     endBeforeStart(tasks[t1], tasks[t2], preparationTime);
  }

  // for each oven, ovenUsage cannot exceed oven capacity
  forall(o in ovens)
    ovenUsage[o] <= ovenCapacity[o];  
 
  // each task can be executed on at most one oven
  forall(t in allTasks) 
    alternative(tasks[t], all (o in ovens) taskOvenAlts[t][o]);
  
  // the temperatureCode on an oven should correspond to the temperature of the task scheduled on the oven at any point in time
  forall(t in allTasks, o in ovens)
    alwaysEqual(ovenTemperature[o], taskOvenAlts[t][o], t.temperatureCode, true, true);  
}

int usedCapacity = sum(t in allTasks) t.duration;
int availPower = sum(f in ovens) ovenCapacity[f];

execute {
  writeln("Shortest possible makespan = " + (preparationTime + usedCapacity / availPower));
} 