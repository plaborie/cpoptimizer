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


/****************************************************
 * TODO: MODEL
 * ...
 ****************************************************/
 