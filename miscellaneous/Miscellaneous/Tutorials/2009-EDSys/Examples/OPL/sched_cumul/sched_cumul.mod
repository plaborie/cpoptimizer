/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building five houses in different locations. The
masonry, roofing, painting, etc. must be scheduled. Some tasks must
necessarily take place before others and these requirements are
expressed through precedence constraints.

There are three workers, and each task requires a worker.  There is
also a cash budget which starts with a given balance.  Each task costs
a given amount of cash per day which must be available at the start of
the task.  A cash payment is received periodically.  The objective is
to minimize the overall completion date.

------------------------------------------------------------ */

//$doc:ALL
//$doc:HEADERS
using CP;
//end:HEADERS

//$doc:TASKS
int NbWorkers = ...;
int NbHouses  = ...; 
range Houses  = 1..NbHouses;

{string} TaskNames   = ...;

int Duration [t in TaskNames] = ...;
//end:TASKS

//$doc:PRECDATA
tuple Precedence {
   string pre;
   string post;
};

{Precedence} Precedences = ...;
//end:PRECDATA

//$doc:HOUSEDATA
int ReleaseDate[Houses] = ...; 
//end:HOUSEDATA

//$doc:TASKVAR
dvar interval itvs[h in Houses][t in TaskNames] in ReleaseDate[h]..(maxint div 2)-1 size Duration[t];
//end:TASKVAR

//$doc:WORKERCUMUL
cumulFunction workersUsage = 
   sum(h in Houses, t in TaskNames) pulse(itvs[h][t],1);
//end:WORKERCUMUL

//$doc:CASHCUMUL
cumulFunction cash = 
  sum (p in 0..5) step(60*p, 30000)
  - sum(h in Houses, t in TaskNames) stepAtStart(itvs[h][t], 200*Duration[t]);
//end:CASHCUMUL

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
  forall(h in Houses)
    forall(p in Precedences)
      endBeforeStart(itvs[h][p.pre], itvs[h][p.post]);
//end:CSTS

//$doc:WORKERCST
  workersUsage <= NbWorkers;
//end:WORKERCST

//$doc:CASHCST
  cash >= 0;
//end:CASHCST
}

