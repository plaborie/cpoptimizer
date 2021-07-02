/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Shop-floor scheduling - Phase 1
 ****************************************************/

using CP;

tuple MachineType {
  string type;
  string machine;
};

tuple Transition {
  string start;
  string end;
  int    duration;
};

tuple Job {
  key int id;
  string  machineType;
  int     relDate;
  int     dueDate;
  float   weight; 
};

{ string }      Machines        = ...;
{ MachineType } MachinesTypes   = ...;
{ string }      Operators       = ...;
{ Transition }  Transitions     = ...;
{ Job }         Jobs            = ...;
int             LoadDuration    = 20;
int             ProcessDuration = 3600;
 
int NbOperators = card(Operators);

{ string } Types = { t.type | t in MachinesTypes };
{ string } MachinesOfType[tp in Types] = { t.machine | t in MachinesTypes: t.type==tp };
{ int }    JobIds = { l.id | l in Jobs };

int AvgTransitionTo[tp in Types] = 
  (sum(tr in Transitions: tr.start=="Entry" && tr.end in MachinesOfType[tp]) tr.duration) div card(MachinesOfType[tp]);

int AvgTransitionFrom[tp in Types] = 
  (sum(tr in Transitions: tr.end=="Exit" && tr.start in MachinesOfType[tp]) tr.duration) div card(MachinesOfType[tp]);

int TransitionTo[m in Machines] = 
  max (tr in Transitions: tr.start=="Entry" && tr.end == m) tr.duration;

tuple Alternative {
  Job    job;
  string machine;
};

{ Alternative } Alternatives = 
  { <l, m> | l in Jobs, m in MachinesOfType[l.machineType] };

int maxinterval = (maxint div 2)-1;

execute {
  cp.param.BranchLimit = 100000;
}

/****************************************************
 * TODO: MODEL OF PHASE 1
 * ...
 ****************************************************/

/****************************************************
 * RESULT OF PHASE 1
 ****************************************************/

// Machine allocation

string AllocatedMachine[l in JobIds];
execute {
 /****************************************************
 * TODO: FILL AllocatedMachine THAT WILL BE USED 
 * IN PHASE 2
 * ...
 ****************************************************/
};

// Sequences on machines

tuple StartDate {
  key int start;
  int job;
}

sorted { StartDate } SequenceOnMachine[m in Machines];
execute {
 /****************************************************
 * TODO: FILL SequenceOnMachine THAT WILL BE USED 
 * IN PHASE 2
 * ...
 ****************************************************/
};

execute {
  for (var m in Machines) {
 /****************************************************
 * TODO: DISPLAY SEQUENCE OF JOBS ON MACHINE m
 * ...
 ****************************************************/
  };
};


 
