/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Shop-floor scheduling - Main model
 ****************************************************/
 
/*
 *
 * Problem Description
 * -------------------
 *
 * The shop-floor consists of m machines (in the example: m =15).
 *
 * A set of n jobs has to be processed on the machines.
 *
 * Each job has to be processed by one machine among a subset of the m
 * machines,   the subset  of possible   machines depends  on the  job
 * type. In the example there is between 1  and 6 alternative machines
 * for a given job.
 *
 * The processing time of a job on a machine is  the same for each job
 * and each machine (1 hour).
 *
 * A machine can process only one job at a time.
 * 
 * Jobs become available at  a certain date  (release date) in a given
 * fixed position of the shop-floor (known as  "Entry").  They need to
 * be transported  by a human  operator to  the  machine they  will be
 * processed on,  then processed by  this machine and then transported
 * again  by  a  human  operator  to  a  given fixed   position of the
 * shop-floor (known as "Exit").
 *
 * A number of k human  operators are available  to transport the jobs
 * (in the example: k=3).
 *
 * The  possible positions of  an operator are  the input "Entry", the
 * output "Exit" and the position of each machine in the shop-floor.
 *
 * The time taken by operators to move from on  position to another is
 * given by a matrix.
 *
 * A given operator can only transport one job at a time.
 *
 * Each job is associated a due-date before which  it should have been
 * processed by a machine  and transported to  position "Exit".   Each
 * job  is associated a  priority (real number between  0  and 1).  In
 * this simplified version,  the objective is  to minimize the sum  of
 * weighted tardiness over the jobs.
 * 
 * Description of the 2-phase approach
 * -----------------------------------
 *
 * A difficulty of the problem is that the machine on which a job will
 * be processed is  not known in advance  (it is a decision  variable)
 * thus, so is the position at which the job will have to be processed
 * and this complicates  a lot the  handling  of transition times  for
 * operators (although one  can do a  model  with alternatives on  the
 * Cartesian  product possiblePositions  X  operators it  results in a
 * very large number of intervals and does not scale to a large number
 * of jobs).
 *
 * In fact, one can notice  that given the  problem data, machines are
 * the main bottleneck resource in the  shop-floor, other aspects such
 * as operators and transportation times are less critical.
 *
 * So the idea is to (1) first  solve a model  that focuses on machine
 * allocation and sequencing   and  only approximates operators    and
 * transition times and  then (2) solve the  complete model using  the
 * machines and sequences on machines computed in phase 1.
 *
 * For the first  model, the duration  of the transportation  activity
 * before the process activity on the machine (moveIn) is approximated
 * by taking the average transition  time between position "Entry" and
 * each machine that  can process the  lot.  Similar approximation for
 * the transportation activity to position "Exit" after the processing
 * (moveOut).  In  this first  phase,  operators are  only  used  as a
 * discrete resource of capacity k using a cumul function.
 *
 * In  the second phase,  as the machines are  known,  the duration of
 * transportation  activities  are  known  and  the phase   focuses on
 * allocating   operators  to  transportation   activities.  The  only
 * difficulty is that, by definition,  as transportation activities do
 * not start  and  end at  the  same position, one   need to use  null
 * duration  intervals at  the beginning   and  end of  transportation
 * activities to be able to use transition time to model the movements
 * of the operator when he is not carrying any job.
 *
 * The 2-phase approach is  justified a posteriori  as we can see that
 * the total  cost of  the  second phase  is very   close  to the  one
 * approximated in the first phase.
 *
 * Model chaining in OPL
 * ---------------------
 *
 * The information that need to be passed from the first to the second
 * phase is the machine  allocation and the sequence  of jobs on  each
 * machine.
 *
 * There is no special difficulty for the machine allocation.
 *
 * For the sequence of lots on each machine, the model uses the notion
 * of sorted tuple sets to sort the tuples <startTime,index>.
 *
 */

using CP;

/*********************************************
 * DATA
 *********************************************/

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
 
/*********************************************
 * MAIN
 *********************************************/
 
main {
  writeln("!--------------------------------------------");
  writeln("! PHASE 1: Machine allocation & sequencing   ");
  writeln("!--------------------------------------------");
  var model1 = new IloOplModelSource("phase1.mod");
  var cp1    = new IloCP();
  var phase1 = new IloOplModelDefinition(model1);
  var opl1   = new IloOplModel(phase1, cp1);
  opl1.addDataSource(thisOplModel.dataElements);
  opl1.generate();
  cp1.solve();
  opl1.postProcess();
  var dataElements1 = opl1.dataElements;
  
  writeln("!--------------------------------------------");
  writeln("! PHASE 2: Operator allocation & sequencing  ");
  writeln("!--------------------------------------------");
  var model2 = new IloOplModelSource("phase2.mod");
  var cp2    = new IloCP();
  var phase2 = new IloOplModelDefinition(model2);
  var opl2   = new IloOplModel(phase2, cp2);
  opl2.addDataSource(thisOplModel.dataElements);
  var data2 = new IloOplDataElements();
  data2.AllocatedMachine = opl1.AllocatedMachine;
  data2.SequenceOnMachine = opl1.SequenceOnMachine;
  opl2.addDataSource(data2);   
  opl2.generate();
  cp2.solve();
  opl2.postProcess();
};


