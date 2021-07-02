
using CP;

{string} Tasks = ...;
int durations[Tasks] = ...;
int start    [Tasks] = ...;

{string} Groups = ...;

int capacity[Groups] = ...;

{string} mayperform[Tasks] = ...;


tuple OptTask {
  string task;
  string group;
}
{OptTask} optTasks = { <t,g> | t in Tasks, g in mayperform[t] };

{string} Workers = ...;

{string} workers[Groups] = ...;

dvar interval tasks[t in Tasks] size durations[t];
dvar interval opttasks[optTasks] optional;
dvar interval worker[Workers];

cumulFunction group[g in Groups] = 
  sum (w in workers[g]) pulse(worker[w], 1) 
  - sum (<t,g> in optTasks) pulse(opttasks[<t,g>], 1);



execute {
  cp.param.FailLimit = 5000;
}

minimize max(w in Workers) lengthOf(worker[w]);

subject to {
  forall(t in Tasks) /* starts of Tasks */
    startOf(tasks[t]) == start[t];
  
  forall(t in Tasks) 
    alternative(tasks[t], all(<t,g> in optTasks) opttasks[<t,g>]);
  
  forall(g in Groups) {
    0 <= group[g];
    group[g] <= capacity[g];
  }
};

execute {
  for (var w in Workers) 
    writeln(w + " present from " + worker[w].start + " to " + worker[w].end);
}
