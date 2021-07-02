 using CP;
 int NbTasks    = ...;
 int NbRRsrcs   = ...;
 int NbNRsrcs   = ...;
 range RRsrcIds = 1..NbRRsrcs; 
 range NRsrcIds = 1..NbNRsrcs; 
 int CapRRsrc [RRsrcIds]  = ...;
 int CapNRsrc [NRsrcIds] = ...;
 tuple Task { key int id; {int} succs; }
 {Task} Tasks = ...;
 tuple Mode { int taskId; int id; int pt; int dmdR[RRsrcIds]; int dmdN[NRsrcIds]; }
 {Mode} Modes = ...;
 dvar interval task[t in Tasks];
 dvar interval mode[m in Modes] optional  size m.pt;
 minimize max(t in Tasks) endOf(task[t]);
 subject to {
   forall (t in Tasks) 
     alternative(task[t], all(m in Modes: m.taskId==t.id) mode[m]);
   forall (r in RRsrcIds)
     sum (m in Modes: m.dmdR[r]>0) pulse(mode[m], m.dmdR[r]) <= CapRRsrc[r];
   forall (r in NRsrcIds)
     sum (m in Modes: m.dmdN[r]>0) m.dmdN[r]*presenceOf(mode[m]) <= CapNRsrc[r];    
   forall (t1 in Tasks, t2id in t1.succs)
     endBeforeStart(task[t1], task[<t2id>]);
 }

 