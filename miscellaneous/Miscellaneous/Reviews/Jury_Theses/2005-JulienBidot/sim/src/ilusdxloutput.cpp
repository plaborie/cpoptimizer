// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilusdxloutput.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

#include<ilsim/ilusdxloutput.h>
#include <ilsched/iloscheduler.h>

ILOSTLBEGIN


IloSDXLOutput::IloSDXLOutput(IloEnv env) 
  : _level(0), _env(env), _out(0) {}

IloSDXLOutput::~IloSDXLOutput() {}

void IloSDXLOutput::write(IloSchedulerSolution solution,
						  ILCSTD(ofstream)& s,
						  IloNum ratio,
						  const char *title,
						  const char *description) {
	setStream(s);

	out() << "<?xml";
	writeValue( "version", "1.0" );
	writeValue( "encoding", "UTF-8" );
	out() << "?>" << endl;

	writeProlog(title, description);
	writeBody(solution, ratio);
	writeEpilog();

	unsetStream();
}

/////////////////////////////////////////////////////////////////////////
// filters and writers
/////////////////////////////////////////////////////////////////////////

  /* Notes: 
     * Testing for an empty handle (e.g. act.getImpl()==0)
     after an IlcScheduler::getExtractable() will select objects
     that are in the original model. Some temporary computation
     objects might have been created during the search, which
     we are not interested in.

     * Although not documented, ILOG Concert Technology's internal ID 
     (IloExtractable::getId()) is used, for convenience.
  */

IloBool IloSDXLOutput::canBeSavedAct(IlcActivity ilcAct, IlcScheduler sched) {
	IloActivity act = IloActivity( sched.getExtractable(ilcAct) );
	if(act.getImpl()==0)
		return IloFalse;   // object was not in the initial model
	return IloTrue;
}

void IloSDXLOutput::writeActivity(IloActivity act, IloSchedulerSolution solution, IloNum ratio) {
	openTag( "activity" );
	writeValue( "id", act.getId() );
	writeName( act, "act" );
	IloInt startMin = (IloInt) IloFloor(solution.getStartMin(act) / ratio);
	IloInt endMin = (IloInt) IloFloor(solution.getEndMin(act) / ratio);
	writeDate( "start", startMin );
	writeDate( "end", endMin );
	closeTag( );
}

/////////////////////////////////////////////////////////////////////////

IloBool IloSDXLOutput::canBeSavedRes(IlcResource ilcRes, IlcScheduler sched) {
    /* We only handle discrete resources, with exception to
       redundant resources. */
	IloResource res = IloResource( sched.getExtractable(ilcRes) );
	if(res.getImpl()==0)
		return IloFalse; // object was not in the initial model
	if(!res.isDiscreteResource())
		return IloFalse;
	IlcCapResource capRes(ilcRes);
	if (capRes.isRedundantResource())
		return IloFalse;
	return IloTrue;
}

void IloSDXLOutput::writeResource(IloResource res) {
  openTag( "resource" );
  writeValue( "id", res.getId() );
  writeName( res, "res" );
  writeValue( "quantity", IloDiscreteResource(res).getCapacity() );
  closeTag( );
}

/////////////////////////////////////////////////////////////////////////

IloBool
IloSDXLOutput::canBeSavedPct(IlcPrecedenceConstraint ilcPct, IlcScheduler sched) {
    // We only select posted precedence constraints, with restricted types.

	if(!ilcPct.isPosted())
		return IloFalse;
	IloPrecedenceConstraint pct = 
	IloPrecedenceConstraint( sched.getExtractable(ilcPct) );
	if(pct.getImpl()==0)
		return IloFalse;   // object was not in the initial model
	if(!canBeSavedAct(ilcPct.getPrecedingActivity(), sched))
		return IloFalse;
	if(!canBeSavedAct(ilcPct.getFollowingActivity(), sched))
		return IloFalse;

	switch( pct.getType()) {
    /* These types are not available in SDXL. */
		case IloStartsAtStart:
			return IloFalse;
		case IloStartsAtEnd:
			return IloFalse;
		case IloEndsAtStart:
			return IloFalse;
		case IloEndsAtEnd:
			return IloFalse;
		default:
			break;
	}
	return IloTrue;
}

void IloSDXLOutput::writeConstraint(IloPrecedenceConstraint pCt) {
	const char *cTType = 0;
	switch(pCt.getType()) {
		case IloStartsAfterStart: cTType = "Start-Start"; break;
		case IloStartsAfterEnd:   cTType = "End-Start";   break;
		case IloEndsAfterStart:   cTType = "Start-End";   break;
		case IloEndsAfterEnd:     cTType = "End-End";     break;
	}
	openTag("constraint");
	writeValue("from", pCt.getPrecedingActivity().getId());
	writeValue("to",   pCt.getFollowingActivity().getId());
	writeValue("type", cTType);
	closeTag( );
}

void
IloSDXLOutput::writePrecedenceConstraints(IloEnv env) {
	for(IloIterator<IloPrecedenceConstraint> it(env); it.ok(); ++it) {
		writeConstraint(*it);
	}
}

/////////////////////////////////////////////////////////////////////////

IloBool
IloSDXLOutput::canBeSavedRct(IlcResourceConstraint ilcRct, IlcScheduler sched) {
	/* We only handle posted, non-ignored, resource constraints with 
      time extent 'IlcFromStartToEnd', with exception to unselected 
      alternative resource constraint. */

	IloResourceConstraint rct = IloResourceConstraint( sched.getExtractable(ilcRct) );
	if(rct.getImpl()==0)
		return IloFalse;   // object was not in the initial model        
	if(!ilcRct.isPosted())
		return IloFalse;

    /* filter upon time extent */
	if (ilcRct.getTimeExtent()!=IlcFromStartToEnd)
		return IloFalse;

    /* reject ignored rct */
	if (ilcRct.isVariableResourceConstraint()) {
		if ( IlcCapRct(ilcRct).getCapacityVariable().getMin()==0 )
			return IloFalse;
	}
	else if (IlcCapRct(ilcRct).getCapacity()==0)
		return IloFalse;

	/* reject non-selected alternatives */
	if (!canBeSavedRes(ilcRct.getResource(), sched))
		return IloFalse;
	if (!canBeSavedAct(ilcRct.getActivity(), sched))
		return IloFalse;
	return IloTrue;
}

void
IloSDXLOutput::writeReservation(IloResourceConstraint rCt, IloSchedulerSolution solution) {
	IloResource iloRes = solution.getSelected(rCt);
	IloActivity iloAct = rCt.getActivity();
	openTag("reservation");
	writeValue("resource", iloRes.getId());
	writeValue("activity", iloAct.getId());
	closeTag( );
}

void
IloSDXLOutput::writeResourceConstraints(IlcResource ilcRes, IlcScheduler sched) {
	assert(canBeSavedRes(ilcRes, sched));

/*	for(IlcResourceConstraintIterator it(ilcRes); it.ok(); ++it)
		if (canBeSavedRct( *it, sched ))
			writeReservation( *it, sched );*/
}


void
IloSDXLOutput::writeProlog(const char *title,
						   const char *description) {
	out() << "<!DOCTYPE schedule SYSTEM \"sdxl.dtd\">" << endl;

	openTag( "schedule" );
	writeValue( "version", "3.5" );
	closeTag( IloFalse );

	if (title!=0)
		out() << "<title>" << title << "</title>" << endl;
	if (description!=0)
		out() << "<desc>" << description << "</desc>" << endl;
}


void
IloSDXLOutput::writeBody(IloSchedulerSolution solution, IloNum ratio) {
    // Loop over all resources, embedded in a global "All Resources" resource.

	openTag("resources");
	closeTag( IloFalse );
	// Root "All Resources" resource
	openTag("resource");
	writeValue( "id", "All Resources" );
	writeValue( "name", "All Resources" );
	IloNum nbTotalResources = 0.0;
	for(IloSchedulerSolution::ResourceIterator it11(solution); it11.ok(); ++it11)
		nbTotalResources++;
	writeValue("quantity", nbTotalResources);
	closeTag(IloFalse);

	for(IloSchedulerSolution::ResourceIterator it1(solution); it1.ok(); ++it1)
		writeResource(*it1);

	finishTag("resource");
	finishTag("resources");


	// Loop over all activities, embedded in a global "All Activities" activity.

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
		IloInt startMin = (IloInt) IloFloor(act.getStartMin() / ratio);
		IloInt endMin = (IloInt) IloFloor(act.getEndMin() / ratio);
		if(startMin < start)
			start = startMin;
		if(endMin > end)
			end = endMin;
	}
	writeDate("start", start);
	writeDate("end", end);
	closeTag(IloFalse);

	for(IloSchedulerSolution::ActivityIterator it22(solution); it22.ok(); ++it22)
		writeActivity(*it22, solution, ratio);

	finishTag( "activity" );    
	finishTag( "activities" );


    // Loop over all precedence constraints, and save them.

	openTag("constraints");
	closeTag(IloFalse);
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

void
IloSDXLOutput::writeEpilog() {
	finishTag("schedule");
}