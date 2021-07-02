/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 * 
 * Personal tasks scheduling
 ****************************************************/

//
// This scheduling problem is described in:
// "Managing Personal Tasks with Time Constraints
//  and Preferences", I. Refanidis, 
//  Proc. ICAPS-07, September, 2007. 
// 

using CP;

int Horizon = 500;

tuple Task {
  key int id;     // Unique identifier of the task
  int     loc;    // Location
  int     dur;    // Total duration
  int     smin;   // Minimal duration of a part
  int     smax;   // Maximal duration of a part
  int     dmin;   // Minimal delay between consecutive parts
  int     upref;  // Preference function type
  int     date;   // Date for stepwise preference functions
  { int } dstart; // Start dates of domains
  { int } dend;   // End dates of domains
};
{ Task } Tasks = ...;

int NbLocations = ...;

tuple Distance { int loc1; int loc2; int dist; };
{ Distance } Distances = ...;

tuple Ordering { int pred; int succ; };
{ Ordering } Orderings = ...;

int DomMin  [t in Tasks] = min(x in t.dstart) x;
int DomMax  [t in Tasks] = max(x in t.dend)   x;
int DomSize [t in Tasks] = DomMax[t]-DomMin[t];

tuple TaskPart {
  Task task;
  int  id;
};
{ TaskPart } TaskParts = { <t,i> | t in Tasks, i in 1 .. t.dur div t.smin };

tuple Step { int x; int y; }
sorted { Step } Steps[t in Tasks] = { <x,0> | x in t.dstart } union { <x,100> | x in t.dend };
stepFunction calendar[t in Tasks] = stepwise(s in Steps[t]) {s.y -> s.x; 0};

dvar interval tasks[t in Tasks] in 0..Horizon;
dvar interval parts[p in TaskParts] optional size p.task.smin..p.task.smax;
dvar sequence seq in all(p in TaskParts) parts[p] types all(p in TaskParts) p.task.loc;

dexpr float satisfaction[t in Tasks] = 
  (t.upref==0) ? 1 :
    (1/t.dur)* sum(p in TaskParts: p.task==t)  (
      (t.upref==-2) ? minl(endOf(parts[p]), t.date) - minl(startOf(parts[p]), t.date) : 
      (t.upref==-1) ? lengthOf(parts[p])*(DomMax[t] - (startOf(parts[p])+endOf(parts[p])-1)/2)/DomSize[t] :
      (t.upref== 1) ? lengthOf(parts[p])*((startOf(parts[p])+endOf(parts[p])-1)/2 - DomMin[t])/DomSize[t] :
      (t.upref== 2) ? maxl(endOf(parts[p]), t.date) - maxl(startOf(parts[p]), t.date) : 0);
       
maximize sum(t in Tasks) satisfaction[t];
subject to {
  forall(p in TaskParts) {
    forbidExtent(parts[p], calendar[p.task]);
    forall(s in TaskParts: s.task==p.task && s.id==p.id+1) {
      endBeforeStart(parts[p], parts[s], p.task.dmin);
      presenceOf(parts[s]) => presenceOf(parts[p]);
    }
  }
  forall(t in Tasks) {
    sum(p in TaskParts: p.task==t) sizeOf(parts[p]) == t.dur;
    span(tasks[t], all(p in TaskParts: p.task==t) parts[p]);
  }
  forall(o in Orderings)
    endBeforeStart(tasks[<o.pred>], tasks[<o.succ>]);
  noOverlap(seq, Distances);
};
