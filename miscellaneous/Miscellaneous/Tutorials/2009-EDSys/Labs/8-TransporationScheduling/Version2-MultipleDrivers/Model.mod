/****************************************************
 * ILOG CP Optimizer Training
 *
 * Transportation scheduling - Version 2
 ****************************************************/

using CP;

tuple Task {
  key int id;
  int     smin;
  int     emax;
  int     duration;
};

int      NbWorkers         = ...;
int      NbMaxShifts       = ...;
int      MinInterShiftRest = ...;
int      MaxIntraShiftWork = ...;
int      MaxShiftDuration  = ...;
{ Task } Tasks             = ...;

range Workers = 1..NbWorkers;
range Shifts  = 1..NbMaxShifts;

/****************************************************
 * TODO: MODEL 
 * ...
 ****************************************************/

// Solution display

tuple ScheduledTask {
  int start;
  int end;
  int id;
};

sorted { ScheduledTask } ScheduledTasks[Workers][Shifts];

execute {
 /****************************************************
 * TODO: FILL ScheduledTasks AND DISPLAY EACH WORKER
 * TASKS IN CHRONOLOGICAL ORDER
 * ...
 ****************************************************/
}