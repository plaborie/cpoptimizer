 using CP;
 int NbTasks = ...;
 int NbRsrcs = ...;
 range RsrcIds = 1..NbRsrcs; 
 int Capacity[RsrcIds] = ...;
 tuple Task { key int id; int pt; int dmds[RsrcIds]; {int} succs; }
 {Task} Tasks = ...;
 dvar interval itvs[t in Tasks] size t.pt;
 minimize max(t in Tasks) endOf(itvs[t]);
 subject to {
   forall (r in RsrcIds)
     sum (t in Tasks: t.dmds[r]>0) pulse(itvs[t], t.dmds[r]) <= Capacity[r];
   forall (t1 in Tasks, t2id in t1.succs)
     endBeforeStart(itvs[t1],itvs[<t2id>]);
 }

