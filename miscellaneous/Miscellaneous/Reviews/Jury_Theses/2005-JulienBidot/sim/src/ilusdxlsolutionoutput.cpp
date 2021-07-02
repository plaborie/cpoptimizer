// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilusdxlsolutionoutput.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include<ilsim/ilusdxlsolutionoutput.h>

ILOSTLBEGIN


IloSDXLSchedSolOutput::IloSDXLSchedSolOutput(IloEnv env) 
  : IloSDXLOutput(env) {}

void
IloSDXLSchedSolOutput::writeProcPlan(IluProcPlan* procPlan, IloSchedulerSolution solution, IloNum ratio) {
  // A process plan is an activity that contains all the shop activities.
	IloActivityArray actArray = procPlan->getActivities();
	IlcInt size = actArray.getSize();
	IloActivity firstActivity = actArray[size-1];
	IloActivity lastActivity = actArray[0];
	openTag("activity");
	writeValue("id", procPlan->getName());
	writeValue("name", procPlan->getName());
	IloInt startMin = (IloInt) IloFloor(solution.getStartMin(firstActivity) / ratio);
	writeDate("start", startMin);
//	printf("Start = %.14f\n", solution.getStartMin(firstActivity));
	IloInt endMin = (IloInt) IloFloor(solution.getEndMin(lastActivity) / ratio);
	writeDate("end", endMin);
//	printf("End = %.14f\n", solution.getEndMin(lastActivity));
	closeTag(IloFalse);
	for(IloInt i = 0; i < size; i++)
		writeActivity(actArray[i], solution, ratio);
	IloInt dueDateTemp = procPlan->getDueDate();
	IloEnv env = solution.getEnv();
	char buffer[128];
	IloInt procPlanId = procPlan->getProcPlanIndex();
	sprintf(buffer, "PP%ldDueDate", procPlanId);
	IloActivity act(env, 0.0, buffer);
	openTag("activity");
	writeValue("id", act.getId());
	writeName(act, "act");
	IloInt dueDate = (IloInt) IloFloor(dueDateTemp / ratio);
	writeDate("start", dueDate);
	writeDate("end",   dueDate);
	closeTag();
	finishTag("activity");
}

void
IloSDXLSchedSolOutput::writeSchedSol(IloSchedulerSolution solution,
									 ILCSTD(ofstream)& s,
									 IloArray<IloActivityArray> actArrays,
									 IloNum ratio,
									 const char *title) {
  // This is the same structure as the class IloSDXLOutput but the body 
  // will change to introduce the notion of process plan.
  setStream(s);
  out() << "<?xml";
  writeValue( "version", "1.0" );
  writeValue( "encoding", "UTF-8" );
  out() << "?>" << endl;
  writeProlog(title, "Scheduling Solution");
  writeProcPlans(solution, actArrays, ratio);//, makespan);
  writeEpilog();
  unsetStream();
}

void
IloSDXLSchedSolOutput::writeProcPlans(IloSchedulerSolution solution,
									  IloArray<IloActivityArray> actArrays,
									  IloNum ratio) {
  // This is the same structure as writeBody but the activities are enclosed in a process plan.
  
  // Loop over all resources, embedded in a global "All Resources" resource.
  
	openTag("resources");
	closeTag(IloFalse);
	// Root "All Resources" resource
	openTag("resource");
	writeValue("id", "All Resources");
	writeValue("name", "All Resources");
	IloNum counter = 0.0;
	for(IloSchedulerSolution::ResourceIterator it11(solution); it11.ok(); ++it11)
		counter++;
	writeValue("quantity", counter);
	closeTag(IloFalse);

	for(IloSchedulerSolution::ResourceIterator it1(solution); it1.ok(); ++it1)
		writeResource( *it1);

	finishTag("resource");
	finishTag("resources");

  // Loop over all process plans, embedded in a global "All Activities" activity.

//	IlcIntVar ilcMakespan = sched.getSolver().getIntVar(makespan);
	openTag("activities");
	writeValue("dateFormat", getDateFormat());
	closeTag(IloFalse);

	openTag("activity");
	writeValue("id", "All Activities");
	writeValue("name", "All Activities");
	IloInt start = IloIntMax;
	IloInt end = 0;
	for(IloSchedulerSolution::ActivityIterator it2(solution); it2.ok(); ++it2) {
		IloActivity act = *it2;
		IloInt startMin = (IloInt) IloFloor(solution.getStartMin(act) / ratio);
		IloInt endMin = (IloInt) IloFloor(solution.getEndMin(act) / ratio);
		if(startMin < start)
			start = startMin;
		if(endMin > end)
			end = endMin;
	}
	writeDate("start", start);
	writeDate("end", end);
	closeTag(IloFalse);

	IloInt numberOfProcPlans = actArrays.getSize();
	IloInt i;
	for(i = 0; i < numberOfProcPlans; i++) {
		IloActivityArray actArray = actArrays[i];
		IloActivity act = actArray[0];
		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		writeProcPlan(procPlan, solution, ratio);
	}
  
  // The makespan is a null duration activity.

//	openTag( "activity" );
//	writeValue( "id", "makespan" );
//	writeValue( "name", "makespan" );
//	writeDate( "start", getVarValue(ilcMakespan));
//	writeDate( "end", getVarValue(ilcMakespan));
//	closeTag();

	finishTag("activity");    
	finishTag("activities");

  // Loop over all process plans to constrain with the makespan.

	openTag( "constraints" );
	closeTag( IloFalse );
//	for(i = 0; i < numberOfProcPlans; i++) {
//		openTag( "constraint" );
//		IloActivityArray actArray = actArrays[i];
//		IloActivity act = actArray[0];
//		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
//		IluProcPlan* procPlan = infoAct->getProcPlan();
//		writeValue( "from", procPlan->getName());
//		writeValue( "to", "makespan");
//		writeValue( "type", "End-Start");
//		closeTag( );
//	}
  // Loop over all precedence constraints, and save them.
	IloEnv env = solution.getEnv();
	writePrecedenceConstraints(env);
	finishTag("constraints");


  // Loop over all resource constraints, and save them.

	openTag("reservations");
	closeTag(IloFalse);
	for(IloSchedulerSolution::ResourceConstraintIterator it4(solution); it4.ok(); ++it4) {
		writeReservation(*it4, solution);
	}
	finishTag("reservations");
}
