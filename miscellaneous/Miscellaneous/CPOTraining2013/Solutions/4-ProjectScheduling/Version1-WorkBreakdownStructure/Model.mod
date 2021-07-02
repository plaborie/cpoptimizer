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
 
dvar interval task[t in Tasks] size t.ptMin..MaxInterval;

/************************
 * Decision expressions *
 ************************/
 
dexpr int makespan = max(t in Tasks) endOf(task[t]);

/*************
 * Objective *
 *************/
 
minimize MakespanWeight * makespan;

/***************
 * Constraints *
 ***************/
 
subject to {   
  // Work breakdown structure: each parent spans all its children
  forall (t in Tasks : t.id in Parents)
    span(task[t], all(i in ParentLinks: i.parentId == t.id) task[<i.taskId>]); 
       
  // Precedence constraints
  // start of task "beforeId" starts at least "delay" hours before start of task "afterId"
  forall (p in Precedences : p.type == "StartsAfterStart") 
    startBeforeStart(task[<p.beforeId>], task[<p.afterId>], p.delay);
  // start of task "beforeId" ends at least "delay" hours before start of task "afterId"
  forall (p in Precedences : p.type == "StartsAfterEnd") 
    endBeforeStart(task[<p.beforeId>], task[<p.afterId>], p.delay);
};
