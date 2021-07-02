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

dvar interval task [t in Tasks] optional in t.smin..t.emax size t.duration;
dvar interval alt  [a in Alternatives] optional;
dvar interval shift[s in Shifts] size 0..MaxShiftDuration;

/************
* Objective *
*************/

maximize sum(t in Tasks) lengthOf(task[t]);

/**************
* Constraints *
***************/

dvar sequence shiftSeq[s in Shifts] in all(a in Alternatives: a.shift==s) alt[a];

subject to {
  // Each task scheduled on at most one shift
  forall (t in Tasks) 
    alternative(task[t], all(a in Alternatives: a.task==t) alt[a]);

  // Shift sequence
  forall (s in 1..NbMaxShifts-1)
    endBeforeStart(shift[s], shift[s+1], MinInterShiftRest); 

  forall (s in Shifts) {
    // Shift spanning interval
    span(shift[s], all(a in Alternatives: a.shift==s) alt[a]);
    // Max intra-shift work constraint
    sum(a in Alternatives: a.shift==s) lengthOf(alt[a]) <= MaxIntraShiftWork;
    // Worker unary capacity during shift
    //noOverlap(all(a in Alternatives: a.shift==s) alt[a]);
    noOverlap(shiftSeq[s]);
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
