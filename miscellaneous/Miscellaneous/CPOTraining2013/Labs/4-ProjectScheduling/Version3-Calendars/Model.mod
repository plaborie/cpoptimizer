/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Project scheduling - Version 3
 ****************************************************/

using CP;
 
/********
 * Data *
 ********/
 
tuple Task {
  key int id;
  string  name;
  int     ptMin;
};

{ Task } Tasks = ...;
int TopLevelTask = ...;

tuple ParentLink {
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

tuple Alloc {
  int reqId;
  int workerId;
  int pt;
};

{ Alloc } Allocations = { <r.id, i, t.ptMin> | r in Requirements, t in Tasks, i in PossibleWorkers[r] : t.id==r.taskId };

tuple WorkerBreak {
  int workerId;
  int start; 
  int end;
};

{ WorkerBreak } WorkerBreaks = ...;

// KPIs
float MakespanWeight     = ...;
float FixedWeight        = ...;
float ProportionalWeight = ...;

int MaxInterval = (maxint div 2)-1;

tuple Step { // represents steps for stepwise calendar
  int v;
  key int x; // must be key to sort
};
sorted {Step} Steps[w in Workers] = // input to stepwise must be sorted
   { <100, b.start> | b in WorkerBreaks : b.workerId==w.id } union 
   { <0,   b.end>   | b in WorkerBreaks : b.workerId==w.id };
   
// FILL IN
stepFunction           ;

/**********************
 * Decision variables *
 **********************/
 
dvar interval task[t in Tasks]       size t.ptMin..MaxInterval;
dvar interval alts[a in Allocations] optional // FILL IN ;
dvar interval workerSpan[w in Workers] optional;

/************************
 * Decision expressions *
 ************************/
 
dexpr int makespan = max(t in Tasks) endOf(task[t]);
dexpr int workerPropCost[w in Workers] = sum(a in Allocations: a.workerId==w.id) sizeOf(alts[a],0);
dexpr float workersFixedCost = sum(w in Workers) w.fixedCost * presenceOf(workerSpan[w]); 
dexpr float workersPropCost = sum(w in Workers) w.propCost*workerPropCost[w];

/*************
 * Objective *
 *************/
 
minimize MakespanWeight       * makespan 
         + FixedWeight        * workersFixedCost 
         + ProportionalWeight * workersPropCost;

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
  
   // Alternatives of workers who can fulfil task requirement (each requirement must be filled by one worker)
  forall(r in Requirements)
    alternative(task[<r.taskId>], all(a in Allocations: a.reqId==r.id) alts[a]);

   // Calculate whether each worker is used in the project using span constraint
  forall(w in Workers)
    span(workerSpan[w], all(a in Allocations: a.workerId==w.id) alts[a]);
    
  // A worker can fill only one task requirement at any point in time
  forall(w in Workers)
    noOverlap(all(a in Allocations: a.workerId==w.id) alts[a]);
    
};
