/****************************************************
 * ILOG CP Optimizer Training
 *
 * Shop-floor scheduling - Phase 1
 ****************************************************/

using CP;

tuple MachineType {
  string type;
  string machine;
};

tuple Transition {
  string start;
  string end;
  int    duration;
};

tuple Job {
  key int id;
  string  machineType;
  int     relDate;
  int     dueDate;
  float   weight; 
};

{ string }      Machines        = ...;
{ MachineType } MachinesTypes   = ...;
{ string }      Operators       = ...;
{ Transition }  Transitions     = ...;
{ Job }         Jobs            = ...;
int             LoadDuration    = 20;
int             ProcessDuration = 3600;
 
int NbOperators = card(Operators);

{ string } Types = { t.type | t in MachinesTypes };
{ string } MachinesOfType[tp in Types] = { t.machine | t in MachinesTypes: t.type==tp };
{ int }    JobIds = { l.id | l in Jobs };

int AvgTransitionTo[tp in Types] = 
  (sum(tr in Transitions: tr.start=="Entry" && tr.end in MachinesOfType[tp]) tr.duration) div card(MachinesOfType[tp]);

int AvgTransitionFrom[tp in Types] = 
  (sum(tr in Transitions: tr.end=="Exit" && tr.start in MachinesOfType[tp]) tr.duration) div card(MachinesOfType[tp]);

int TransitionTo[m in Machines] = 
  max (tr in Transitions: tr.start=="Entry" && tr.end == m) tr.duration;

tuple Alternative {
  Job    job;
  string machine;
};

{ Alternative } Alternatives = 
  { <l, m> | l in Jobs, m in MachinesOfType[l.machineType] };

int maxinterval = (maxint div 2)-1;

dvar interval moveIn [l in Jobs] in l.relDate..maxinterval size 2*LoadDuration + AvgTransitionTo[l.machineType]; 
dvar interval process[l in Jobs] size ProcessDuration;
dvar interval moveOut[l in Jobs] size 2*LoadDuration + AvgTransitionFrom[l.machineType]; 

dvar interval alts[<l,m> in Alternatives] optional in l.relDate+2*LoadDuration+TransitionTo[m] .. maxinterval;
dvar sequence machines[m in Machines] in all(<l,m> in Alternatives) alts[<l,m>]; 

execute {
  cp.param.BranchLimit = 100000;
}

minimize sum(l in Jobs) (l.weight * maxl(0, endOf(moveOut[l])-l.dueDate));
subject to {
  forall (l in Jobs) {
    endBeforeStart(moveIn[l],process[l]);
    endBeforeStart(process[l],moveOut[l]);
  };
  
  forall (l in Jobs)
    alternative(process[l], all(<l,m> in Alternatives) alts[<l,m>]);
  
  forall (m in Machines)
    noOverlap(machines[m]);
  
  sum(l in Jobs) (pulse(moveIn[l],1) + pulse(moveOut[l],1)) <= NbOperators;
};
 
/*********************************************
 * RESULT OF PHASE 1
 *********************************************/

// Machine allocation

string AllocatedMachine[l in JobIds];
execute {
  for (var a in Alternatives)
    if (alts[a].present)
      AllocatedMachine[a.job.id]=a.machine;
};

// Sequences on machines

tuple StartDate {
  key int start;
  int job;
}

sorted { StartDate } SequenceOnMachine[m in Machines];
execute {
  for (var a in Alternatives)
    if (alts[a].present)
       SequenceOnMachine[a.machine].add(process[a.job].start, a.job.id);
};

execute {
  for (var m in Machines) {
    write(m, ": ");
    for (var s in SequenceOnMachine[m]) {
      write("Job ", s.job);
      if (s != Opl.last(SequenceOnMachine[m]))
        write(" -> ");
    }
    writeln();
  };
};


 
