 using CP;
 stepFunction workingTime = 
   stepwise(i in 0..51, p in 0..1) { 100*p -> (7*i)+(5*p) ; 0 };
 dvar interval task size 9 intensity workingTime;
 constraints {
   startOf(task)==0;
 }
 