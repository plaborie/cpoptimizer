/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Project scheduling - Version 2
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

tuple Worker {
  key int id;
  string  name;
  float   fixedCost;
  float   propCost;
};

{ Worker } Workers = ...;

tuple Skill {
  key int id;
  string  name;
};

{ Skill } Skills = ...;

tuple Proficiency {
  int workerId;
  int skillId;
  int level;
};
  
{ Proficiency } Proficiencies = ...;

tuple Requirement {
  key int id;
  int     taskId;
};

{ Requirement } Requirements = ...;

tuple RequiredSkill {
  int reqId;
  int skillId;
  int levelMin;
  int levelMax;
};

{ RequiredSkill } RequiredSkills = ...;

{ int } PossibleWorkers[r in Requirements] = 
   { p.workerId | p in Proficiencies, n in RequiredSkills : 
     (n.reqId==r.id) && 
     (p.skillId==n.skillId) && 
     (n.levelMin <= p.level) && 
     (p.level <= n.levelMax) };

tuple Alloc { // possible allocations of workers to requirements, with associated task durations.
  int reqId;
  int workerId;
  int pt;
};

{ Alloc } Allocations = { <r.id, i, t.ptMin> | r in Requirements, t in Tasks, i in PossibleWorkers[r] : t.id==r.taskId };

// KPIs
float MakespanWeight     = ...;
float FixedWeight        = ...;
float ProportionalWeight = ...;

int MaxInterval = (maxint div 2)-1;

/**********************
 * Decision variables *
 **********************/
 
dvar interval task[t in Tasks]       size t.ptMin..MaxInterval;
// FILL IN: Interval variables for workerSpan and worker allocations

/************************
 * Decision expressions *
 ************************/
 
dexpr int makespan = max(t in Tasks) endOf(task[t]);
// FILL IN: decision expressions to calculate worker fixed and proportional cost

/*************
 * Objective *
 *************/
 
minimize MakespanWeight       * makespan 
// FILL IN
;

/***************
 * Constraints *
 ***************/
 
subject to {   
  // Work breakdown structure
  forall (t in Tasks : t.id in Parents)
       span(task[t], all(i in ParentLinks: i.parentId == t.id) task[<i.taskId>]); 
       
   // Precedence constraints
  forall (p in Precedences : p.type == "StartsAfterStart") 
      startBeforeStart(task[<p.beforeId>], task[<p.afterId>], p.delay);
  forall (p in Precedences : p.type == "StartsAfterEnd") 
      endBeforeStart(task[<p.beforeId>], task[<p.afterId>], p.delay);
  
  // Each requirement must be filled by one worker (alternative)
  forall(r in Requirements)
    // FILL IN
    ;

  // A worker can fill only one task requirement at any point in time (non-overlap)
  forall(w in Workers)
    // FILL IN
    ;
    
  // Calculate whether each worker is used in the project (span)
  forall(w in Workers)
    // FILL IN
    ;
    
};
