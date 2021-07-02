/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building five houses. The masonry, roofing,
painting, etc. must be scheduled.  Some tasks must necessarily take
place before others and these requirements are expressed through
precedence constraints.

There are two workers and each task requires a specific worker.  The
worker has a calendar of days off that must be taken into account. The
objective is to minimize the overall completion date.

------------------------------------------------------------ */
 
//$doc:ALL
//$doc:HEADERS
using CP;
//end:HEADERS

//$doc:TASKS
int NbHouses = ...; 
range Houses = 1..NbHouses;

{string} WorkerNames = ...;  
{string} TaskNames   = ...;

int    Duration [t in TaskNames] = ...;
string Worker   [t in TaskNames] = ...;
//end:TASKS

//$doc:PRECDATA
tuple Precedence {
  string pre;
  string post;
};

{Precedence} Precedences = ...;
//end:PRECDATA

//$doc:BREAKDATA
tuple Break {
  int s;
  int e;
};

{Break} Breaks[WorkerNames] = ...; 
//end:BREAKDATA

// Set of break steps
//$doc:EFFN
tuple Step {
  int v;
  key int x;
};
sorted {Step} Steps[w in WorkerNames] = 
   { <100, b.s> | b in Breaks[w] } union 
   { <0, b.e>   | b in Breaks[w] };
   
stepFunction Calendar[w in WorkerNames] = 
  stepwise (s in Steps[w]) { s.v -> s.x; 100 };
//end:EFFN

//$doc:TASKVAR
dvar interval itvs[h in Houses, t in TaskNames] 
  size      Duration[t]
  intensity Calendar[Worker[t]];
//end:TASKVAR

//$doc:LIMIT
execute {
  cp.param.FailLimit = 10000;
}
//end:LIMIT

//$doc:OBJ
minimize max(h in Houses) endOf(itvs[h]["moving"]);
//end:OBJ

subject to {
//$doc:CSTS
  forall(h in Houses) {
    forall(p in Precedences)
      endBeforeStart(itvs[h][p.pre], itvs[h][p.post]);
//end:CSTS
//$doc:FORBID
    forall(t in TaskNames) {
      forbidStart(itvs[h][t], Calendar[Worker[t]]);
      forbidEnd  (itvs[h][t], Calendar[Worker[t]]);
    }
  }
//end:FORBID
//$doc:NOOVERLAP
  forall(w in WorkerNames)
    noOverlap( all(h in Houses, t in TaskNames: Worker[t]==w) itvs[h][t]);
//end:NOOVERLAP
}
//end:ALL
