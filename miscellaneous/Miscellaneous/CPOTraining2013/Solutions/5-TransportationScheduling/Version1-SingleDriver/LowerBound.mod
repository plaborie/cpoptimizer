/*********************************************
 * OPL 12.2 Model
 * Author: P_Laborie
 * Creation Date: Sep 13, 2010 at 1:34:24 PM
 *********************************************/

 using CP;
 
 /*******
* Data *
********/

tuple Task {
  key int id;
  int     smin;
  int     emax;
  int     duration;
};

int      NbMaxShifts       = ...;
int      MinInterShiftRest = ...;
int      MaxIntraShiftWork = ...;
int      MaxShiftDuration  = ...;
{ Task } Tasks             = ...;

range Shifts  = 1..NbMaxShifts;
 
range TaskIds = 1..100;
int SumD = sum(t in Tasks) t.duration;
dvar int shift[t in TaskIds] in 1..NbMaxShifts+1; // Last with large size
int weight[t in TaskIds] = max(i in Tasks: i.id == t) i.duration;
dvar int load[s in 1..NbMaxShifts+1] in 0..SumD;

maximize SumD - load[NbMaxShifts+1];
subject to {
   pack(load, shift, weight); 
   forall(s in 1..NbMaxShifts)
     load[s]<=MaxIntraShiftWork;
   forall(t1, t2 in Tasks : t1.id != t2.id && t1.emax - t1.duration + MaxShiftDuration < t2.smin + t2.duration) {
     (shift[t1.id]==shift[t2.id])==(shift[t1.id]== NbMaxShifts+1);
   }     
}  
