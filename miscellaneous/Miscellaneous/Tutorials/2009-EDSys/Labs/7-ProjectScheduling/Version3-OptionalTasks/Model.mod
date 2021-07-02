/****************************************************
 * ILOG CP Optimizer Training
 *
 * Project scheduling - Version 3
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

tuple OptionalTask {
  int   taskId;
  float nonExecCost;
};

{OptionalTask} OptionalTasks = ...;

// KPIs
float MakespanWeight     = ...;
float FixedWeight        = ...;
float ProportionalWeight = ...;
float NonExecutionWeight = ...;

int MaxInterval = (maxint div 2)-1;

/****************************************************
 * TODO: MODEL 
 * ...
 ****************************************************/
