/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Transportation scheduling - Version 1
 ****************************************************/

using CP;

/*******
* Data *
********/

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

// Element of the cartesian product (Task x Shift)
tuple Alternative {
  Task task;
  int  shift;
};

{ Alternative } Alternatives = { <t,s> | t in Tasks, s in Shifts };

/*********************
* Decision variables *
**********************/

// FILL IN

/************
* Objective *
*************/

maximize // FILL IN;

/**************
* Constraints *
***************/

subject to {
  // Each task scheduled on at most one shift  forall (t in Tasks) 
    // FILL IN

  // Shift sequence  forall (s in 1..NbMaxShifts-1)
    // FILL IN

  forall (s in Shifts) {
    // Shift spanning interval
    // FILL IN
    // Max intra-shift work constraint
    // FILL IN
    // Worker unary capacity during shift
    // FILL IN
  }
}

/********************
*  Solution display *
*********************/

tuple ScheduledTask {
  int start;
  int end;
  int id;
};
sorted { ScheduledTask } ScheduledTasks[Shifts];

execute {
  // Tasks sorted by increasing start time
  for (var a in Alternatives)
    if (alt[a].present)
      ScheduledTasks[a.shift].add(task[a.task].start, task[a.task].end, a.task.id);
  // Displays shifts and tasks
  var sLoad = 0;
  var nShifts = 0;
  for (var s in Shifts) {
    var work = 0;
    if (shift[s].present) {
      write("\t[");
      for (var t in ScheduledTasks[s]) {
        work += (t.end - t.start);
        write("(T", t.id, ",", t.start, "->", t.end, ")");
      }
      var load = ((100*work) / MaxIntraShiftWork);
      writeln("] \t Load=", Opl.round(load), "%");
      sLoad += load;
      nShifts++;
    }
  }
  var avgLoad = sLoad / nShifts;
  writeln("Average shift load=", Opl.round(avgLoad),"%");
}
