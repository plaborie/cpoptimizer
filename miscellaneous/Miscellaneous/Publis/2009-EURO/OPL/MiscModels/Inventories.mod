 using CP;
 int MaxLevel = ...;
 int NP = ...; int NC = ...;
 int QProd[1..NP]=...;
 int QCons[1..NC]=...;
 dvar interval producer[1..NP];
 dvar interval consumer[1..NC];
 cumulFunction inventoryLevel = 
   sum(p in 1..NP) stepAtEnd  (producer[p], QProd[p]) -
   sum(c in 1..NC) stepAtStart(consumer[c], QCons[c]);
 constraints {
   inventoryLevel <= MaxLevel;
 }
 