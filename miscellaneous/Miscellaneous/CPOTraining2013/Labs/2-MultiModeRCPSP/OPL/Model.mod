/****************************************************
 * IBM ILOG CPLEX CP Optimizer Training
 *
 * Multi-Mode Resource Constrained Project Scheduling
 ****************************************************/

using CP;

int NbTasks = ...;
int NbRenewableRsrcs    = ...;
int NbNonRenewableRsrcs = ...;

range RenewableRsrcIds    = 0..NbRenewableRsrcs-1; 
range NonRenewableRsrcIds = 0..NbNonRenewableRsrcs-1; 

int CapRenewableRsrc    [RenewableRsrcIds] = ...;
int CapNonRenewableRsrc [NonRenewableRsrcIds] = ...;

tuple Task {
  key int id;
  int     nmodes;
  {int}   succs; 
}
{Task} Tasks = ...;

tuple Mode {
  key int taskId;
  key int id;
  int pt;
  int dmdRenewable   [RenewableRsrcIds];
  int dmdNonRenewable[NonRenewableRsrcIds];
}
{Mode} Modes = ...;

/****************************************************
 * TODO: CP Optimizer MODEL 
 * ...
 ****************************************************/
