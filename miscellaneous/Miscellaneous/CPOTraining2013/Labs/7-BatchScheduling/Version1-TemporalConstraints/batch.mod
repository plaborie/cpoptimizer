/******************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 * 
 * Batch scheduling - Version 1                                                   
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
}; 
{productStepT} steps[products] = ...;

int orders[p in products] = ...;

tuple taskT {
  key int productId;
  key int stepNumber;
  key int orderId;
  int duration;
};
{taskT} allTasks = {<p, s.stepNumber, o, 
                     s.duration> 
                    | p in products, s in steps[p], o in 1..orders[p]};

int maxTime = 
  sum(t in allTasks)(t.duration + preparationTime); // upper limit on scheduling horizon

/*********************
* Decision variables *
**********************/

dvar interval operation[t in allTasks] size t.duration..maxTime;

/************
* Objective *
*************/

minimize max(t in allTasks) endOf(operation[t]);

/**************
* Constraints *
***************/

constraints {
  
  // 30 minutes preparation is required between steps for each product order
  forall(t1,t2 in allTasks : t1.productId == t2.productId &&
                             t1.stepNumber + 1 == t2.stepNumber &&
                             t1.orderId == t2.orderId) {
     endBeforeStart(operation[t1], operation[t2], preparationTime);
  }

}