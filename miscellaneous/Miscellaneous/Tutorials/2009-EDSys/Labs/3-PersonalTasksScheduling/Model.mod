/****************************************************
 * ILOG CP Optimizer Training
 *
 * Personal tasks scheduling
 *
 * This scheduling problem is described in:
 * "Managing Personal Tasks with Time Constraints
 *  and Preferences", I. Refanidis, 
 *  Proc. ICAPS-07, September, 2007. 
 * 
 ****************************************************/

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

// The distance matrix between locations
tuple Distance { int loc1; int loc2; int dist; };
{ Distance } Distances = ...;

// The set of tasks ordering
tuple Ordering { int pred; int succ; };
{ Ordering } Orderings = ...;

// The minimal, maximal value and size of each task domain
int DomMin  [t in Tasks] = min(x in t.dstart) x;
int DomMax  [t in Tasks] = max(x in t.dend)   x;
int DomSize [t in Tasks] = DomMax[t]-DomMin[t];

// The envelope of all possible task parts
tuple TaskPart {
  Task task;
  int  id;
};
{ TaskPart } TaskParts = { <t,i> | t in Tasks, i in 1 .. t.dur div t.smin };

// The step function defining a task domain
// FILL IN

// Declare decision variables of type interval
// FILL IN

// Declare decision variables of type sequence
// FILL IN

// The expression measuring the satisfaction of a task
// FILL IN

// Declare the objective function
// FILL IN

subject to {
  // Each task must execute within its domain
  // FILL IN

  // Each task decomposes into a chain of task parts
  // FILL IN

  // The duration of a task is the sum of the duration of its parts
  // FILL IN

  // A task starts with its first parts and ends with its last part
  // FILL IN

  // Task ordering
  // FILL IN

  // Transition times between tasks
  // FILL IN

}
