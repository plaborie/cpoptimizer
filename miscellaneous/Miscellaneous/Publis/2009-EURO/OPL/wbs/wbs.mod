 using CP;
 tuple Decomposition { int task; {int} subtasks; };
 int n = ...;
 int compulsory[1..n] = ...;
 {Decomposition} Decompositions = ...;
 int nbDecompositions[i in 1..n] = card( {d | d in Decompositions : d.task==i} );
 int nbParents[i in 1..n] = card( {d | d in Decompositions : i in d.subtasks} );
 dvar interval tasks[i in 1..n] optional;
 dvar interval decs[d in Decompositions] optional;
 constraints {
   forall(i in 1..n) {
     if (nbParents[i]==0 && 0<compulsory[i])
       presenceOf(tasks[i]);
     if (nbDecompositions[i]>0) {
       alternative(tasks[i], all(d in Decompositions: d.task==i) decs[d]);
       forall(d in Decompositions: d.task==i) 
         span(decs[d], all(j in d.subtasks) tasks[j]);
     }
   }
   forall(d in Decompositions, j in d.subtasks: 0<compulsory[j])
     presenceOf(decs[d]) => presenceOf(tasks[j]);
 }
