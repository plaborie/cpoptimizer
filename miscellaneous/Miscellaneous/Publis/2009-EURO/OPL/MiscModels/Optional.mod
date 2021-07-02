 using CP;
 int n = ...;
 int IsOptional[1..n] = ...;
 dvar interval task[i in 1..n] optional;
 constraints { 
   forall(i in 1..n)
     if (IsOptional[i]==0) // Compulsory task
       presenceOf(task[i]);
 }
