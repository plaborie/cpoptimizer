/****************************************************
 * ILOG CP Optimizer Training
 *
 * Transportation scheduling - Version 1
 ****************************************************/

using CP;

tuple Task {
  key int id;
  int     smin;
  int     emax;
  int     duration;
};

int      NbMaxShifts       = ...;
int      MinInterShiftRest = ...;
int      MaxIntraShiftWork = ...;
int      MaxShiftDuration  = ...;
{ Task } Tasks             = ...;

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

sorted { ScheduledTask } ScheduledTasks[Shifts];

execute {
 /****************************************************
 * TODO: FILL ScheduledTasks AND DISPLAY TASKS IN 
 * CHRONOLOGICAL ORDER
 * ...
 ****************************************************/
}
