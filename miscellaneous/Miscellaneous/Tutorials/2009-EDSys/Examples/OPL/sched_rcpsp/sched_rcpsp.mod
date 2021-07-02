using CP;

int NbTasks = ...;
int NbRsrcs = ...;

range RsrcIds = 0..NbRsrcs-1; 

int Capacity[r in RsrcIds] = ...;

tuple Task {
  key int id;
  int     pt;
  int     dmds[RsrcIds];
  {int}   succs; 
}

{Task} Tasks = ...;

dvar interval itvs[t in Tasks]  size t.pt;

cumulFunction rsrcUsage[r in RsrcIds] = 
  sum (t in Tasks: t.dmds[r]>0) pulse(itvs[t], t.dmds[r]);

execute {
  cp.param.FailLimit = 10000;
}

minimize max(t in Tasks) endOf(itvs[t]);
subject to {
  forall (r in RsrcIds)
    rsrcUsage[r] <= Capacity[r];
  forall (t1 in Tasks, t2id in t1.succs)
    endBeforeStart(itvs[t1], itvs[<t2id>]);
}

execute {
  for (var t in Tasks) {
    writeln("Task " + t.id + " starts at " + itvs[t].start);
  }
}
