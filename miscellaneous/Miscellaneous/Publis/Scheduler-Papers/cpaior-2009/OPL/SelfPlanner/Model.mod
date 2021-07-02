using CP;
tuple Task { key int id; int loc; int dur; int smin; int smax; int dmin; int f; int date; {int} ds; {int} de; } 
{Task} Tasks = ...;
tuple Distance { int loc1; int loc2; int dist; };
{Distance} Distances = ...;
tuple Ordering { int pred; int succ; };
{Ordering} Orderings = ...;
int L[t in Tasks] = min(x in t.ds) x;
int R[t in Tasks] = max(x in t.de) x;
int S[t in Tasks] = R[t]-L[t];
tuple Part { Task task; int id; } 
{Part} Parts = { <t,i> | t in Tasks, i in 1 .. t.dur div t.smin };
tuple Step { int x; int y; } 
sorted {Step} Steps[t in Tasks] =
{<x,0> | x in t.ds} union {<x,1> | x in t.de};
stepFunction holes[t in Tasks] = stepwise(s in Steps[t]) {s.y -> s.x; 0};
dvar interval tasks[t in Tasks] in 0..500;
dvar interval a[p in Parts] optional size p.task.smin..p.task.smax;
dvar sequence seq in all(p in Parts) a[p] types all(p in Parts) p.task.loc;
dexpr float satisfaction[t in Tasks] = (t.f==0)? 1 :
  (1/t.dur)* sum(p in Parts: p.task==t)
    (t.f==-2)? maxl(endOf(a[p]),t.date)-maxl(startOf(a[p]),t.date) :
    (t.f==-1)? lengthOf(a[p])*(R[t]-(startOf(a[p])+endOf(a[p])-1)/2)/S[t] :
    (t.f== 1)? lengthOf(a[p])*((startOf(a[p])+endOf(a[p])-1)/2-L[t])/S[t] :
    (t.f== 2)? minl(endOf(a[p]),t.date)-minl(startOf(a[p]),t.date) : 0;
maximize sum(t in Tasks) satisfaction[t];
subject to { 
  forall(p in Parts) { 
    forbidExtent(a[p], holes[p.task]);
    forall(s in Parts: s.task==p.task && s.id==p.id+1) { 
      endBeforeStart(a[p], a[s], p.task.dmin);
      presenceOf(a[s]) => presenceOf(a[p]);
    } 
  } 
  forall(t in Tasks) { 
    t.dur == sum(p in Parts: p.task==t) lengthOf(a[p]);
    span(tasks[t], all(p in Parts: p.task==t) a[p]);
  } 
  forall(o in Orderings)
    endBeforeStart(tasks[<o.pred>], tasks[<o.succ>]);
  noOverlap(seq, Distances);
}