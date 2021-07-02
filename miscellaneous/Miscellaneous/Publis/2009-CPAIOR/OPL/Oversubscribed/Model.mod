using CP;
 
// DATA
 
tuple Station { 
  string name; // Ground station name
  int id;      // Ground station identifier
  int cap;     // Number of available antennas
} 

tuple Opportunity { 
  string task; // Task
  int station; // Ground station
  int smin;    // Start of visibility window of opportunity
  int dur;     // Task duration in this opportunity
  int emax;    // End of visibility window of opportunity
}

{Station} Stations = ...;
{Opportunity} Opportunities = ...;
{string} Tasks = { o.task | o in Opportunities };

// MODEL

dvar interval task[t in Tasks] optional;
dvar interval opp[o in Opportunities] optional in o.smin..o.emax size o.dur;

maximize sum(t in Tasks) presenceOf(task[t]);
subject to {
  forall(t in Tasks)
    alternative(task[t], all(o in Opportunities: o.task==t) opp[o]);
  forall(s in Stations)
    sum(o in Opportunities: o.station==s.id) pulse(opp[o],1) <= s.cap;
}