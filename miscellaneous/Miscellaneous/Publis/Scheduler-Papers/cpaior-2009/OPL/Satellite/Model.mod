using CP;
tuple Station { string name; key int id; int cap; } 
tuple Alternative { string task; int station; int smin; int dur; int emax; } 
{Station} Stations = ...;
{Alternative} Alternatives = ...;
{string} Tasks = { a.task | a in Alternatives };
dvar interval task[t in Tasks] optional;
dvar interval alt[a in Alternatives] optional in a.smin..a.emax size a.dur;
maximize sum(t in Tasks) presenceOf(task[t]);
subject to { 
  forall(t in Tasks)
    alternative(task[t], all(a in Alternatives: a.task==t) alt[a]);
  forall(s in Stations)
    sum(a in Alternatives: a.station==s.id) pulse(alt[a],1) <= s.cap;
}