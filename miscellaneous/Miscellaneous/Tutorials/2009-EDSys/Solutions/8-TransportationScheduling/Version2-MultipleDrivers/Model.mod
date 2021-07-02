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

// Element of the cartesian product (Task x Worker x Shift)
tuple Alternative {
  Task task;
  int  worker;
  int  shift;
};

{ Alternative } Alternatives = { <t,w,s> | t in Tasks, w in Workers, s in Shifts };

dvar interval task [t in Tasks] in t.smin..t.emax size t.duration;
dvar interval alt  [a in Alternatives]         optional;
dvar interval shift[w in Workers][s in Shifts] optional size 0..MaxShiftDuration;

// Minimize total number of opened shifts
minimize sum(w in Workers, s in Shifts) presenceOf(shift[w][s]);
subject to {
  // For each task, alternatives on Workers x Shifts
  forall (t in Tasks) 
    alternative(task[t], all(a in Alternatives: a.task==t) alt[a]);

  // Shift sequence for each worker
  forall (w in Workers, s in 1..NbMaxShifts-1)
    endBeforeStart(shift[w][s], shift[w][s+1], MinInterShiftRest);

  // Shift optionality chain for each worker
  forall (w in Workers, s in 1..NbMaxShifts-1)
    presenceOf(shift[w][s+1]) => presenceOf(shift[w][s]);

  forall (w in Workers, s in Shifts) {
    // Shift spanning interval
    span(shift[w][s], all(a in Alternatives: a.worker==w && a.shift==s) alt[a]);
    // Max intra-shift work constraint
    sum(a in Alternatives: a.worker==w && a.shift==s) lengthOf(alt[a]) <= MaxIntraShiftWork;
    // Worker unary capacity during shift
    noOverlap(all(a in Alternatives: a.worker==w && a.shift==s) alt[a]);
  }
}

// Solution display

tuple ScheduledTask {
  int start;
  int end;
  int id;
};

sorted { ScheduledTask } ScheduledTasks[Workers][Shifts];
execute {
  // Tasks sorted by increasing start time
  for (var a in Alternatives)
    if (alt[a].present)
      ScheduledTasks[a.worker][a.shift].add(task[a.task].start, task[a.task].end, a.task.id);
  // Displays shifts and tasks for each worker
  var sLoad = 0;
  var nShifts = 0;
  for (var w in Workers) {
    writeln("Worker", w, ": ");
    for (var s in Shifts) {
      var work = 0;
      if (shift[w][s].present) {
        write("\t[");
        for (var t in ScheduledTasks[w][s]) {
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
  }
  writeln("Average shift load=", Opl.round(100*avgLoad)/100,"%");
}