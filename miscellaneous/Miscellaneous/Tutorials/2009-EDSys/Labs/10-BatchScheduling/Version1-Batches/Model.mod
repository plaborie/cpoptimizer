/****************************************************
 * ILOG CP Optimizer Training
 *
 * Batch scheduling - Version 1
 ****************************************************/

using CP;

tuple Job {
  key int id; // Job id
  int arr;    // Arrival time at station
  int prio;   // Priority of job
};
{ Job } Jobs = ...;

tuple Operation {
  key int id; // Operation id
  int jobId;  // Job id
  int pos;    // Position in job
};
{Operation} Ops   = ...;

int NbOps[j in Jobs] = card({o | o in Ops : o.jobId==j.id});
Operation FirstOperation[j in Jobs] = first({o | o in Ops : o.jobId==j.id && o.pos==1});
Operation LastOperation [j in Jobs] = first({o | o in Ops : o.jobId==j.id && o.pos==NbOps[j]});;

tuple Tool {
  key string mch; // Machine
  int batch;      // Batch size
};
{Tool} Tools = ...;

tuple Mode {
  int    opId;  // Operation id
  string mch;   // Machine
  int    pt;    // Processing time
  int    state; // State
};

{Mode} Modes = ...;

int horizon = 24*60;

/****************************************************
 * TODO: MODEL
 * ...
 ****************************************************/
 
execute {

/****************************************************
 * TODO: DISPLAY SOLUTION
 * ...
 ****************************************************/

}

