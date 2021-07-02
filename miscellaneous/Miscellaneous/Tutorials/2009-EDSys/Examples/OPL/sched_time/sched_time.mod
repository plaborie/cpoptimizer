/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building a house. The masonry, roofing, painting,
etc. must be scheduled.  Some tasks must necessarily take place before
others and these requirements are expressed through precedence
constraints.

Moreover, there are earliness and tardiness costs associated with some
tasks. The objective is to minimize these costs.

------------------------------------------------------------ */

//$doc:ALL
//$doc:INC
using CP;
//end:INC

//$doc:TASKS
dvar interval masonry    size 35;
dvar interval carpentry  size 15;
dvar interval plumbing   size 40;
dvar interval ceiling    size 15;
dvar interval roofing    size 5;
dvar interval painting   size 10;
dvar interval windows    size 5;
dvar interval facade     size 10;
dvar interval garden     size 5;
dvar interval moving     size 5;
//end:TASKS

//$doc:OBJ
minimize 400 * maxl(endOf(moving) - 100, 0)     +
         200 * maxl(25 - startOf(masonry), 0)   +
         300 * maxl(75 - startOf(carpentry), 0) +
         100 * maxl(75 - startOf(ceiling), 0);
//end:OBJ

subject to {
//$doc:CSTS
  endBeforeStart(masonry,   carpentry);
  endBeforeStart(masonry,   plumbing);
  endBeforeStart(masonry,   ceiling);
  endBeforeStart(carpentry, roofing);
  endBeforeStart(ceiling,   painting);
  endBeforeStart(roofing,   windows);
  endBeforeStart(roofing,   facade);
  endBeforeStart(plumbing,  facade);
  endBeforeStart(roofing,   garden);
  endBeforeStart(plumbing,  garden);
  endBeforeStart(windows,   moving);
  endBeforeStart(facade,    moving);
  endBeforeStart(garden,    moving);
  endBeforeStart(painting,  moving);
//end:CSTS
}

//$doc:SOLN
execute {
  writeln("Masonry  : " + masonry.start   + ".." + masonry.end);
  writeln("Carpentry: " + carpentry.start + ".." + carpentry.end);
  writeln("Plumbing : " + plumbing.start  + ".." + plumbing.end);
  writeln("Ceiling  : " + ceiling.start   + ".." + ceiling.end);
  writeln("Roofing  : " + roofing.start   + ".." + roofing.end);
  writeln("Painting : " + painting.start  + ".." + painting.end);
  writeln("Windows  : " + windows.start   + ".." + windows.end);
  writeln("Facade   : " + facade.start    + ".." + facade.end);
  writeln("Garden   : " + garden.start    + ".." + garden.end);
  writeln("Moving   : " + moving.start    + ".." + moving.end);
}
//end:SOLN

//end:ALL

/*
//$doc:RESULTS
OBJECTIVE: 5000
Masonry  : 20..55
Carpentry: 75..90
Plumbing : 55..95
Ceiling  : 75..90
Roofing  : 90..95
Painting : 90..100
Windows  : 95..100
Facade   : 95..105
Garden   : 95..100
Moving   : 105..110
//end:RESULTS
*/
