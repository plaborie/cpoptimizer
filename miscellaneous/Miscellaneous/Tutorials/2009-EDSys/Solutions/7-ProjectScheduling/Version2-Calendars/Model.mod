/****************************************************
 * ILOG CP Optimizer Training
 *
 * Project scheduling - Version 2
 ****************************************************/

using CP;
 
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

tuple Actor {
  key int id;
  string  name;
  float   fixedCost;
  float   propCost;
};

{ Actor } Actors = ...;

tuple Skill {
  key int id;
  string  name;
};

{ Skill } Skills = ...;

tuple Proficiency {
  int actorId;
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

{ int } PossibleActors[r in Requirements] = 
   { p.actorId | p in Proficiencies, n in RequiredSkills : 
     (n.reqId==r.id) && 
     (p.skillId==n.skillId) && 
     (n.levelMin <= p.level) && 
     (p.level <= n.levelMax) };

tuple Alloc {
  int reqId;
  int actorId;
  int pt;
};

{ Alloc } Allocations = { <r.id, i, t.ptMin> | r in Requirements, t in Tasks, i in PossibleActors[r] : t.id==r.taskId };

tuple ActorBreak {
  int actorId;
  int start; 
  int end;
};

{ ActorBreak } ActorBreaks = ...;

// KPIs
float MakespanWeight     = ...;
float FixedWeight        = ...;
float ProportionalWeight = ...;

int MaxInterval = (maxint div 2)-1;

tuple Step {
  int v;
  key int x; // For sort
};
sorted {Step} Steps[w in Actors] = 
   { <100, b.start> | b in ActorBreaks : b.actorId==w.id } union 
   { <0,   b.end>   | b in ActorBreaks : b.actorId==w.id };
   
stepFunction calendar[w in Actors] = stepwise (s in Steps[w]) { s.v -> s.x; 100 };

dvar interval task[t in Tasks]       size t.ptMin..MaxInterval;
dvar interval alts[a in Allocations] optional size a.pt..MaxInterval intensity calendar[<a.actorId>];
dvar interval actorSpan[w in Actors] optional;

dexpr int makespan = max(t in Tasks) endOf(task[t]);
dexpr int actorPropCost[w in Actors] = sum(a in Allocations: a.actorId==w.id) sizeOf(alts[a],0);
dexpr float actorsFixedCost = sum(w in Actors) w.fixedCost*presenceOf(actorSpan[w]); 
dexpr float actorsPropCost = sum(w in Actors) w.propCost*actorPropCost[w];

minimize MakespanWeight       * makespan 
         + FixedWeight        * actorsFixedCost 
         + ProportionalWeight * actorsPropCost;

subject to {   
  // Work breakdown structure
  forall (t in Tasks)
    if (t.id in Parents)
      span(task[t], all(i in ParentLinks: i.parentId == t.id) task[<i.taskId>]); 
  
  // Precedence constraints
  forall (p in Precedences) {
    if (p.type == "StartsAfterStart") {
      startBeforeStart(task[<p.beforeId>], task[<p.afterId>], p.delay);
    } else if (p.type == "StartsAfterEnd") {
      endBeforeStart(task[<p.beforeId>], task[<p.afterId>], p.delay);
    }
  }
  
  // Alternatives
  forall(r in Requirements)
    alternative(task[<r.taskId>], all(a in Allocations: a.reqId==r.id) alts[a]);

  // Actors spans
  forall(w in Actors)
    span(actorSpan[w], all(a in Allocations: a.actorId==w.id) alts[a]);
    
  // Actors non-overlap
  forall(w in Actors)
    noOverlap(all(a in Allocations: a.actorId==w.id) alts[a]);
};
