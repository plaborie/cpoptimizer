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
  int opId;   // Operation id
  string mch; // Machine
  int pt;     // Processing time
  int state;  // State
};

{Mode} Modes = ...;

int horizon = 24*60;

dvar interval ops[o in Ops]; 
dvar interval modes[md in Modes] optional size md.pt.. horizon;
dvar interval tooluse[t in Tools]; 

stateFunction state[t in Tools];
cumulFunction nbOps[t in Tools] = sum(md in Modes: md.mch == t.mch ) pulse(modes[md],1);

minimize sum(j in Jobs) j.prio * endOf(ops[LastOperation[j]]) + 
		 sum(t in Tools) sizeOf(tooluse[t]);

subject to {
  forall (j in Jobs)
    j.arr <= startOf(ops[FirstOperation[j]]);
    
  forall (o1 in Ops, o2 in Ops: o1.jobId==o2.jobId && o2.pos==1+o1.pos) {
    endBeforeStart(ops[o1],ops[o2]);
    startBeforeEnd(ops[o2],ops[o1],-200); 
  }
  
  forall (o in Ops)
    alternative(ops[o], all(md in Modes: md.opId==o.id) modes[md]); 

  forall (t in Tools)
  	nbOps[t] <= t.batch;

  forall(t in Tools, md in Modes : md.mch == t.mch)
  	alwaysEqual(state[t], modes[md], md.state, 1,1);	

  forall (t in Tools)
  	span(tooluse[t], all(md in Modes: t.mch == md.mch) modes[md]); 
}

execute {
  for (var m in Modes) {
  	for (var o in Ops){
    	if (modes[m].present && o.id == m.opId )
      		writeln("Operation \t"       + m.opId + 
      		        "\t for job \t"      + o.jobId + 
     		        "\t starting at \t"  + modes[m].start + 
      		        "\t finishing at \t" + modes[m].end + 
       		        "\t on machine \t"   + m.mch + 
      		        "\t with state \t"   + m.state );
  	}
  }
}

