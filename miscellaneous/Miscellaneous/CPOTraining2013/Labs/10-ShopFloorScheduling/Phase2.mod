/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Shop-floor scheduling - Phase 2
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

{ int } JobIds = { l.id | l in Jobs };
 
/*********************************************
 * RESULT OF PHASE 1
 *********************************************/

// Machine allocation

string AllocatedMachine[l in JobIds] =...;

// Sequences on machines

tuple StartDate {
  int start;
  int job;
}
sorted { StartDate } SequenceOnMachine[m in Machines] =...;

/*********************************************
 * MODEL OF PHASE 2
 *********************************************/

{string} Positions = 
  { tr.start | tr in Transitions } union 
  { tr.end   | tr in Transitions };
  
int TransitionTime[start in Positions][end in Positions] = 
  min(tr in Transitions: tr.start==start && tr.end==end) tr.duration;

tuple Alternative {
  Job    job;
  string op;
};

{ Alternative } Alternatives = 
  { <l, o> | l in Jobs, o in Operators };

int maxinterval = (maxint div 2)-1;
        
execute {
  cp.param.BranchLimit = 100000;
};

/****************************************************
 * TODO: MODEL OF PHASE 2
 * ...
 ****************************************************/
 
/*********************************************
 * RESULT OF PHASE 2
 *********************************************/
 
// Sequences on operators

tuple StartOpDate {
  key int start;
  int job;
  int op; // 0: before processing, 1: after processing
}

sorted { StartOpDate } SequenceOnOperator[o in Operators];
execute {
 /****************************************************
 * TODO: FILL SequenceOnOperator
 * ...
 ****************************************************/
};

execute {
  for (var o in Operators) {
 /****************************************************
 * TODO: DISPLAY SEQUENCE OF TASKS FOR OPERATOR o
 * ...
 ****************************************************/
   };
};
 
