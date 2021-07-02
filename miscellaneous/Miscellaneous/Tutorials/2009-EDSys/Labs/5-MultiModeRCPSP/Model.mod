/****************************************************
 * ILOG CP Optimizer Training
 *
 * Minimizing resource peak usage - Version 1
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
  int taskId;
  int id;
  int pt;
  int dmdRenewable   [RenewableRsrcIds];
  int dmdNonRenewable[NonRenewableRsrcIds];
}
{Mode} Modes = ...;

/****************************************************
 * TODO: CP Optimizer MODEL 
 * ...
 ****************************************************/
