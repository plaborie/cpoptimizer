/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Project scheduling - Version 1
 ****************************************************/

using CP;
 
/********
 * Data *
 ********/

tuple Task {
  key int id;
  string  name;
  int     ptMin; // minimum duration
};

{ Task } Tasks = ...;
int TopLevelTask = ...;

tuple ParentLink {  // hierarchy data
  int taskId;
  int parentId;
};

{ ParentLink } ParentLinks = ...;
{ int } Parents = { p.parentId | p in ParentLinks };

tuple Precedence {
  int    beforeId;
  int    afterId;
  string type;
  int    delay;
};

{ Precedence } Precedences = ...;

// KPIs
float MakespanWeight     = ...;

int MaxInterval = (maxint div 2)-1; // upper bound on the size of an interval

/**********************
 * Decision variables *
 **********************/
 
// FILL IN: interval variables for tasks

/************************
 * Decision expressions *
 ************************/
 
// FILL IN: decision expression to calculate makespan

/*************
 * Objective *
 *************/
 
// FILL IN: Minimize makespan

/***************
 * Constraints *
 ***************/
 
subject to {   
  // Work breakdown structure: each parent spans all its children
  // FILL IN 
       
  // Precedence constraints
  // start of task "beforeId" starts at least "delay" hours before start of task "afterId"
  //FILL IN
  
  // start of task "beforeId" ends at least "delay" hours before start of task "afterId"
  // FILl IN
};
 