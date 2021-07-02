/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 * 
 * Satellite scheduling
 ****************************************************/

// This scheduling problem is described in:
// "Understanding Performance Tradeoffs in Algorithms
//  for Solving Oversubscribed Scheduling", 
//  L. Kramer, L. Barbulescu, and S. Smith
//  Proc. AAAI-07, July, 2007. 
// 
// Note that in this version of the model task
// priorities are not handled.

using CP;

tuple GroundStation {
  string  name;        // name of the station
  key int id;          // station identifier
  int     capacity;    // number of antennas
};
 
tuple Alternative {
  string task;         // task identifier
  int    station;      // station identifier
  int    smin;         // earliest possible start time
  int    duration;     // task duration
  int    emax;         // latest possible end time
};
 
// The list of stations
{ GroundStation } Stations     = ...;
// The list of alternative communication tasks
{ Alternative   } Alternatives = ...;
 
 // The list of communication tasks
{ string } Tasks = { a.task | a in Alternatives };
int NbTasks = card(Tasks);

dvar interval task[t in Tasks] optional;
dvar interval alt[a in Alternatives] optional in a.smin..a.emax size a.duration;

cumulFunction load[s in Stations] = 
  sum(a in Alternatives: a.station==s.id) pulse(alt[a],1);

maximize sum(t in Tasks) presenceOf(task[t]);

subject to {
  forall(t in Tasks)
    alternative(task[t], all(a in Alternatives: a.task==t) alt[a]);
  forall(s in Stations)
     load[s] <= s.capacity;
};

execute {
  // Solution display
  writeln("All ", NbTasks, " tasks have been scheduled except for: ");
  for (var t in Tasks)
    if (!task[t].present)
      write(t, ",");
}