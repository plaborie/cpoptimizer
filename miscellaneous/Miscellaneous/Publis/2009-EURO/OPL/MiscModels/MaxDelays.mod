 using CP;
 dvar interval task[1..2];
 int dmin = ...; int dmax = ...;
 constraints {
   endBeforeStart(task[1],task[2], dmin);
   startBeforeEnd(task[2],task[1],-dmax);
 }
