/****************************************************
 * ILOG CP Optimizer Training
 *
 * Shop-floor scheduling - Phase 2
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

{ int } JobIds = { l.id | l in Jobs };
 
/*********************************************
 * RESULT OF PHASE 1
 *********************************************/

// Machine allocation

string AllocatedMachine[l in JobIds] =...;

// Sequences on machines

tuple StartDate {
  int start;
  int job;
}
sorted { StartDate } SequenceOnMachine[m in Machines] =...;

/*********************************************
 * MODEL OF PHASE 2
 *********************************************/

{string} Positions = 
  { tr.start | tr in Transitions } union 
  { tr.end   | tr in Transitions };
  
int TransitionTime[start in Positions][end in Positions] = 
  min(tr in Transitions: tr.start==start && tr.end==end) tr.duration;

tuple Alternative {
  Job    job;
  string op;
};

{ Alternative } Alternatives = 
  { <l, o> | l in Jobs, o in Operators };

int maxinterval = (maxint div 2)-1;

// Main activities
dvar interval moveIn[l in Jobs] 
  in l.relDate..maxinterval 
  size 2*LoadDuration + TransitionTime["Entry"][AllocatedMachine[l.id]]; 
dvar interval process [l in Jobs] size ProcessDuration;
dvar interval moveOut[l in Jobs] 
  size 2*LoadDuration + TransitionTime[AllocatedMachine[l.id]]["Exit"]; 

// Alternatives & sequences for operator unary capacity
dvar interval moveInOp[a in Alternatives] optional;
dvar interval moveOutOp[a in Alternatives] optional;

dvar sequence operator[o in Operators]
  in append( all(a in Alternatives: a.op==o) moveInOp[a],
             all(a in Alternatives: a.op==o) moveOutOp[a] );

// Alternatives & sequences for operator transitions
dvar interval moveInStart[l in Jobs] size 0;
dvar interval moveInEnd  [l in Jobs] size 0;
dvar interval moveOutStart[l in Jobs] size 0;
dvar interval moveOutEnd  [l in Jobs] size 0;
dvar interval moveInOpStart[a in Alternatives] optional;
dvar interval moveInOpEnd  [a in Alternatives] optional;
dvar interval moveOutOpStart[a in Alternatives] optional;
dvar interval moveOutOpEnd  [a in Alternatives] optional;

// Compile transition triples 
int PositionValue[p in Positions] = ord(Positions, p);
tuple Triplet {
  int src;
  int dst;
  int val;
}

{Triplet} TransitionMatrix = 
{ <PositionValue[s],PositionValue[d],v> | <s,d,v> in Transitions };
  
dvar sequence transition[o in Operators]
  in append( all(a in Alternatives: a.op==o) moveInOpStart[a],
             all(a in Alternatives: a.op==o) moveInOpEnd[a],
             all(a in Alternatives: a.op==o) moveOutOpStart[a],
             all(a in Alternatives: a.op==o) moveOutOpEnd[a] )
  types append( all(a in Alternatives: a.op==o) PositionValue["Entry"],
                all(a in Alternatives: a.op==o) PositionValue[AllocatedMachine[a.job.id]],
                all(a in Alternatives: a.op==o) PositionValue[AllocatedMachine[a.job.id]],
                all(a in Alternatives: a.op==o) PositionValue["Exit"] );
                    
execute {
  cp.param.NoOverlapInferenceLevel = "Medium";
  cp.param.BranchLimit = 100000;
};

minimize sum(l in Jobs) (l.weight * maxl(0, endOf(moveOut[l])-l.dueDate));
subject to {
  forall (l in Jobs) {
    endBeforeStart(moveIn[l],process[l]);
    endBeforeStart(process[l],moveOut[l]);
  };
  
  // Precedences from sequences of phase 1
  forall (m in Machines) {
    forall (s in SequenceOnMachine[m]) {
      if (s != first(SequenceOnMachine[m])) {
        endBeforeStart(process[<prev(SequenceOnMachine[m],s).job>], process[<s.job>]);
      }
    }
  }
  
  // Alternatives & sequences for operator unary capacity
  forall (l in Jobs) {
      alternative(moveIn[l],  all(<l,m> in Alternatives) moveInOp[<l,m>]);
      alternative(moveOut[l], all(<l,m> in Alternatives) moveOutOp[<l,m>]);  
  }
  
  forall (o in Operators)
     noOverlap(operator[o]);
  
  // Alternatives & sequences for operator transitions
  forall (l in Jobs) {
     startAtStart(moveInStart[l],  moveIn[l]);
     endAtEnd    (moveInEnd[l],    moveIn[l]);
     startAtStart(moveOutStart[l], moveOut[l]);
     endAtEnd    (moveOutEnd[l],   moveOut[l]);
     alternative(moveInStart[l],  all(<l,m> in Alternatives) moveInOpStart[<l,m>]);
     alternative(moveInEnd[l],    all(<l,m> in Alternatives) moveInOpEnd[<l,m>]);
     alternative(moveOutStart[l], all(<l,m> in Alternatives) moveOutOpStart[<l,m>]);
     alternative(moveOutEnd[l],   all(<l,m> in Alternatives) moveOutOpEnd[<l,m>]);
  }
  
  forall (a in Alternatives) {
     presenceOf(moveInOp[a]) == presenceOf(moveInOpStart[a]);
     presenceOf(moveInOp[a]) == presenceOf(moveInOpEnd[a]);
     presenceOf(moveOutOp[a])== presenceOf(moveOutOpStart[a]);
     presenceOf(moveOutOp[a])== presenceOf(moveOutOpEnd[a]);
   }

   forall (o in Operators)
     noOverlap(transition[o], TransitionMatrix);
     
};
 
/*********************************************
 * RESULT OF PHASE 2
 *********************************************/
 
// Sequences on operators

tuple StartOpDate {
  key int start;
  int job;
  int op; // 0: before processing, 1: after processing
}

sorted { StartOpDate } SequenceOnOperator[o in Operators];
execute {
  for (var a in Alternatives) {
    if (moveInOp[a].present)
       SequenceOnOperator[a.op].add(moveIn[a.job].start, a.job.id, 0);
    if (moveOutOp[a].present)
       SequenceOnOperator[a.op].add(moveOut[a.job].start, a.job.id, 1);
    }
};

execute {
  for (var o in Operators) {
    writeln(o, ": ");
    for (var s in SequenceOnOperator[o]) {
      write("\tJob ", s.job);
      if (s.op==0) {
        write(" [Entry->", AllocatedMachine[s.job], "]");
      } else {
        write(" [",AllocatedMachine[s.job],"->Exit]");
      }
      if (s != Opl.last(SequenceOnOperator[o]))
        writeln(" -> ");
    }
    writeln();
  };
};
 
