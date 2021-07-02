/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building five houses in different locations. The
masonry, roofing, painting, etc. must be scheduled. Some tasks must
necessarily take place before others and these requirements are
expressed through precedence constraints.

There are two workers, and each task requires a specific worker.  The
time required for the workers to travel between houses must be taken
into account.  

Moreover, there are tardiness costs associated with some tasks as well
as a cost associated with the length of time it takes to build each
house.  The objective is to minimize these costs.

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

//$doc:DATEDATA
int   ReleaseDate[Houses] = ...; 
int   DueDate    [Houses] = ...; 
float Weight     [Houses] = ...; 
//end:DATEDATA

//$doc:HOUSEVAR
dvar interval houses[h in Houses] in ReleaseDate[h]..(maxint div 2)-1;
//end:HOUSEVAR
//$doc:TASKVAR
dvar interval itvs  [h in Houses][t in TaskNames] size Duration[t];
//end:TASKVAR

//$doc:SEQVAR
dvar sequence workers[w in WorkerNames] in
    all(h in Houses, t in TaskNames: Worker[t]==w) itvs[h][t] types
    all(h in Houses, t in TaskNames: Worker[t]==w) h;
//end:SEQVAR

//$doc:TT
tuple triplet { int loc1; int loc2; int value; }; 
{triplet} transitionTimes = { <i,j, ftoi(abs(i-j))> | i in Houses, j in Houses };
//end:TT

//$doc:LIMIT
execute {
  cp.param.FailLimit = 30000;
}
//end:LIMIT

execute
{
  cp.param.timeLimit=60;
}

//$doc:OBJ
minimize sum(h in Houses) 
  (Weight[h] * maxl(0, endOf(houses[h])-DueDate[h]) + lengthOf(houses[h]));
//end:OBJ
subject to {
//$doc:CSTS
  forall(h in Houses)
    forall(p in Precedences)
      endBeforeStart(itvs[h][p.pre], itvs[h][p.post]);
//end:CSTS
//$doc:SPAN
  forall(h in Houses)
    span(houses[h], all(t in TaskNames) itvs[h][t]);
//end:SPAN
//$doc:NOOVERLAP
  forall(w in WorkerNames)
    noOverlap(workers[w], transitionTimes);
//end:NOOVERLAP
}
//end:ALL

/*
//$doc:RESULTS
OBJECTIVE: 21144
//end:RESULTS
*/
