// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/iluheuristic.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains IlcMyAssignAlternativeResourceEvaluator, IlcMyAssignAltRCEvaluator, IlcMyRankResourceEvaluator, IlcMyRankProcPlanCostEvaluator,
// and IloMyAssignAlternativeAndRank.

#include<ilsim/iluheuristic.h>
#include<ilsim/iluprocessplan.h>
#include<ilsolver/pccontr.h>



ILOSTLBEGIN

IloInt counterEvaluator = 0;


ILOEVALUATOR0(IlcMyAssignAltRCNbPossibleEvaluator, IlcAltResConstraint, aRCt) {
	return aRCt.getNumberOfPossible();
}


ILOEVALUATOR0(IlcMyAssignAltRCMinCostEvaluator, IlcAltResConstraint, altRCt) {
//	printf("IlcMyAssignAltRCMinCostEvaluator is called.\n");
	IlcActivity act = altRCt.getActivity();
	IlcInt actMinEndTime = act.getEndMin();
	IloSolver solver = act.getSolver();
	IlcScheduler sched(solver);
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloInt queue = infoAct->getQueue();
	IluProcPlan* procPlan = infoAct->getProcPlan();
	IlcFloat betaPhi = procPlan->getBetaPhi();
	IloInt dueDate = procPlan->getDueDate();
	IlcAltResSet altResSet = altRCt.getAltResSet();
	IlcInt altResSetSize = altResSet.getSize();
	IlcFloat minLocalCost = IlcInfinity;
	IloInt* allocCosts = infoAct->getAllocCosts();//(IlcInt*)altRCt.getObject();
	IlcInt i = 0;
	for(i = 0; i < altResSetSize; i++) {
		IlcResource res = altResSet[i];
		if(IlcTrue == altRCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			IlcInt allocCost = allocCosts[resIndex];
			IlcInt temp = IlcMax(0, actMinEndTime + queue - dueDate);
			IlcFloat localCost = allocCost + betaPhi * temp;
			if(localCost < minLocalCost)
				minLocalCost = localCost;
		}
	}
	
//	printf("Min cost = %.14f\n", minLocalCost);
	return minLocalCost;
}


ILOEVALUATOR0(IlcMyAssignAltRCMaxCostEvaluator, IlcAltResConstraint, altRCt) {
//	printf("IlcMyAssignAltRCMaxCostEvaluator is called.\n");
	IlcActivity act = altRCt.getActivity();
	IlcInt actMinEndTime = act.getEndMin();
	IloSolver solver = act.getSolver();
	IlcScheduler sched(solver);
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloInt queue = infoAct->getQueue();
	IluProcPlan* procPlan = infoAct->getProcPlan();
	IlcFloat betaPhi = procPlan->getBetaPhi();
	IloInt dueDate = procPlan->getDueDate();
	IlcAltResSet altResSet = altRCt.getAltResSet();
	IlcInt altResSetSize = altResSet.getSize();
	IlcFloat maxLocalCost = 0.0;
	IloInt* allocCosts = infoAct->getAllocCosts();//(IlcInt*)altRCt.getObject();
	IlcInt i = 0;
	for(i = 0; i < altResSetSize; i++) {
		IlcResource res = altResSet[i];
		if(IlcTrue == altRCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			IlcInt allocCost = allocCosts[resIndex];
			IlcInt temp = IlcMax(0, actMinEndTime + queue - dueDate);
			IlcFloat localCost = allocCost + betaPhi * temp;
			if(localCost > maxLocalCost)
				maxLocalCost = localCost;
		}
	}
	
//	printf("Min cost = %.14f\n", maxLocalCost);
	return maxLocalCost;
}


ILOEVALUATOR0(IlcMyAssignAltRCDeltaCostEvaluator, IlcAltResConstraint, altRCt) {
//	printf("IlcMyAssignAltRCDeltaCostEvaluator is called.\n");
	IlcActivity act = altRCt.getActivity();
	IlcInt actMinEndTime = act.getEndMin();
	IloSolver solver = act.getSolver();
	IlcScheduler sched(solver);
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloInt queue = infoAct->getQueue();
	IluProcPlan* procPlan = infoAct->getProcPlan();
	IlcFloat betaPhi = procPlan->getBetaPhi();
	IloInt dueDate = procPlan->getDueDate();
	IlcAltResSet altResSet = altRCt.getAltResSet();
	IlcInt altResSetSize = altResSet.getSize();
	IlcFloat minLocalCost = IlcInfinity;
	IlcFloat maxLocalCost = 0.0;
	IloInt* allocCosts = infoAct->getAllocCosts();//(IlcInt*)altRCt.getObject();
	IlcInt i = 0;
	for(i = 0; i < altResSetSize; i++) {
		IlcResource res = altResSet[i];
		if(IlcTrue == altRCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			IlcInt allocCost = allocCosts[resIndex];
			IlcInt temp = IlcMax(0, actMinEndTime + queue - dueDate);
			IlcFloat localCost = allocCost + betaPhi * temp;
			if(localCost > maxLocalCost)
				maxLocalCost = localCost;
			if(localCost < minLocalCost)
				minLocalCost = localCost;
		}
	}
	IlcFloat deltaCost = maxLocalCost - minLocalCost;
	
//	printf("Delta cost = %.14f\n", deltaCost);
	return deltaCost;
}


ILOEVALUATOR0(IlcMyAssignAltRCDispersionEvaluator, IlcAltResConstraint, altRCt) {
//	printf("IlcMyAssignAltRCDispersionEvaluator is called.\n");
	IlcActivity act = altRCt.getActivity();
	IlcInt actMinEndTime = act.getEndMin();
	IloSolver solver = act.getSolver();
	IlcScheduler sched(solver);
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloInt queue = infoAct->getQueue();
	IluProcPlan* procPlan = infoAct->getProcPlan();
	IlcFloat betaPhi = procPlan->getBetaPhi();
	IloInt dueDate = procPlan->getDueDate();
	IloInt procPlanEndMin = procPlan->getEndMin();
	IloInt refDate = IloMax(dueDate, procPlanEndMin);
	IlcAltResSet altResSet = altRCt.getAltResSet();
	IlcInt altResSetSize = altResSet.getSize();
	IlcFloat minLocalCost = IlcInfinity;
	IloInt* allocCosts = infoAct->getAllocCosts();//(IlcInt*)altRCt.getObject();
	IlcInt i = 0;
	IlcInt temp = IlcMax(0, actMinEndTime + queue - refDate);
	for(i = 0; i < altResSetSize; i++) {
		IlcResource res = altResSet[i];
		if(IlcTrue == altRCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			IlcInt allocCost = allocCosts[resIndex];
			IlcFloat localCost = allocCost + betaPhi * temp;
			if(localCost < minLocalCost)
				minLocalCost = localCost;
		}
	}

	IlcFloat sum = 0.0;
	for(i = 0; i < altResSetSize; i++) {
		IlcResource res = altResSet[i];
		if(IlcTrue == altRCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
		//	IlcInt* allocCosts = actInfo->getAllocCosts();//(IlcInt*)altRCt.getObject();
			IlcInt allocCost = allocCosts[resIndex];
			IlcFloat localCost = allocCost + betaPhi * temp;
			sum += 1/(localCost - minLocalCost + 1);
		}
	}
	
	IlcFloat localDispersion = 1/sum;
//	printf("Local dispersion = %.14f\n", localDispersion);
	return localDispersion;
}


ILOEVALUATOR0(IlcMyAltRCConstantEvaluator, IlcAltResConstraint, altRCt) {
	return 0.0;
}


ILOCTXEVALUATOR0(IlcMyAssignAltResourceEvaluator, IlcResource, res, IlcAltResConstraint, altRCt) {
//	printf("IlcMyAssignAlternativeResourceEvaluator is called.\n");
	IloSolver solver = res.getSolver();
	IlcScheduler sched(solver);
	IlcActivity act = altRCt.getActivity();
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloResource iloRes = sched.getExtractable(res);
	IloInt resIndex = (IloInt)iloRes.getObject();
	IloInt* allocCosts = infoAct->getAllocCosts();//(IlcInt*)altRCt.getObject();
//	IlcAltResSet altResSet = altRCt.getAltResSet();
//	IlcInt totalNbOfResources = altResSet.getSize();
	IlcInt allocCost = allocCosts[resIndex];
//	IlcInt actEndTime = act.getEndMin();
//	IloInt queue = infoAct->getQueue();
//	IluProcPlan* procPlan = infoAct->getProcPlan();
//	IlcFloat betaPhi = procPlan->getBetaPhi();
//	IloInt dueDate = procPlan->getDueDate();
//	IlcInt temp = IlcMax(0, actEndTime + queue - dueDate);
	IlcFloat localCost = allocCost;// + betaPhi * temp;
//	printf("Local cost = %.14f\n", localCost);
//	IlcInt procPlanIndex = procPlan->getProcPlanIndex();
//	if(0 != temp)
//		printf("Process plan %ld, Resource %ld:\ntemp = %ld\tbeta * phi = %.14f\tallocation cost = %ld\n", procPlanIndex, resIndex, temp, betaPhi, allocCost);
	return localCost;
}


ILOEVALUATOR0(IlcMyResourceDueDateEvaluator, IlcResource, res) {
//	printf("IlcMyRankResourceEvaluator is called.\n");
	IloSolver solver = res.getSolver();
	IlcScheduler sched(solver);
//	IloResource iloRes = sched.getExtractable(res);
//	IloInt resIndex = (IloInt)iloRes.getObject();
	IlcFloat localCost = 0.0;
	for(IlcResourceConstraintIterator rCIte(res); rCIte.ok(); ++rCIte) {
		IlcResourceConstraint resConstraint = *rCIte;
		IlcActivity act = resConstraint.getActivity();
		IlcInt actMinEndTime = act.getEndMin();
		IloActivity iloAct = sched.getExtractable(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
		IloInt queue = infoAct->getQueue();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat phi = procPlan->getPhi();
		IloInt dueDate = procPlan->getDueDate();
		localCost += phi * (IlcMax(0.0, IlcFloat(actMinEndTime + queue - dueDate)) + gamma * (actMinEndTime + queue - dueDate));
	}
//	printf("Resource %ld: local cost = %.14f\n", resIndex, localCost);
	return localCost;
}


ILOEVALUATOR0(IlcMyResourceRefDateEvaluator, IlcResource, res) {
//	printf("IlcMyRankResourceEvaluator is called.\n");
	IloSolver solver = res.getSolver();
	IlcScheduler sched(solver);
//	IloResource iloRes = sched.getExtractable(res);
//	IloInt resIndex = (IloInt)iloRes.getObject();
	IlcFloat localCost = 0.0;
	for(IlcResourceConstraintIterator rCIte(res); rCIte.ok(); ++rCIte) {
		IlcResourceConstraint resConstraint = *rCIte;
		IlcActivity act = resConstraint.getActivity();
		IlcInt actMinEndTime = act.getEndMin();
		IloActivity iloAct = sched.getExtractable(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
		IloInt queue = infoAct->getQueue();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat phi = procPlan->getPhi();
		IloInt dueDate = procPlan->getDueDate();
		IloInt procPlanMinEndTime = procPlan->getEndMin();
		IloInt refDate = IloMax(dueDate, procPlanMinEndTime);
		localCost += phi * (IlcMax(0.0, IlcFloat(actMinEndTime + queue - refDate)) + gamma * (actMinEndTime + queue - refDate));
	}
//	printf("Resource %ld: local cost = %.14f\n", resIndex, localCost);
	return localCost;
}


ILOEVALUATOR0(IlcMyNothingResourceConstraintEvaluator, IlcResourceConstraint, resCt) {
	IlcManager manager = resCt.getActivity().getSchedule().getManager();
	if(10000 == counterEvaluator) {
		printf("Heap: memory = %ld\tglobal heap: memory = %ld\n", manager.getHeap()->getSize(), manager.getGlobalHeap()->getSize());
		counterEvaluator = 0;
	}
	counterEvaluator++;
	return 1;
}



ILOEVALUATOR0(IlcMyProcPlanCostDueDateEvaluator, IlcResourceConstraint, resCt) {
//	printf("IlcMyRankRCEvaluator is called.\n");
	IloSolver solver = resCt.getSolver();
	IlcManager manager = solver.getManager();
	IlcScheduler sched(solver);
	IlcActivity act = resCt.getActivity();
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IluProcPlan* procPlan = infoAct->getProcPlan();
	IlcFloat processPlanCost = procPlan->getCost();;
	IlcStamp stamp = manager.getStamp();
//	IluMyStamp* iluStamp = (IluMyStamp*)solver.getObject();
	IlcStamp lastStamp = procPlan->getStamp();
//	IlcStamp lastStamp = iluStamp->getStamp();
//	IloInt procPlanIndex = procPlan->getProcPlanIndex();
	if((stamp != lastStamp)||(-IlcInfinity == processPlanCost)) {
//		processPlanCost = 0.0;
		IloNum phi = procPlan->getPhi();
		IloNum gamma = procPlan->getGamma();
		IloInt dueDate = procPlan->getDueDate();
		IloActivityArray actArray = procPlan->getActivities();
//		IloInt nbAct = actArray.getSize();
//		for(IloInt i = 0; i < nbAct; i++) {
		IloActivity activity = actArray[0];
		IlcInt actMinEndTime = activity.getEndMin();
		IluInfoActivity* infoActivity = (IluInfoActivity*)activity.getObject();
	//	IloInt queue = infoActivity->getQueue();
		IlcFloat localCost = phi * (IlcMax(0.0, IlcFloat(actMinEndTime - dueDate)) + gamma * (actMinEndTime - dueDate));
		processPlanCost = localCost;
//		}
		procPlan->setCost(processPlanCost);
//		iluStamp->setStamp(stamp);
		procPlan->setStamp(stamp);
	}
	else
		processPlanCost = procPlan->getCost();
//	printf("Process plan %ld: cost = %.14f\n", procPlanIndex, processPlanCost);
	return processPlanCost;
}


ILOEVALUATOR0(IlcMyProcPlanCostRefDateEvaluator, IlcResourceConstraint, resCt) {
//	printf("IlcMyRankRCEvaluator is called.\n");
	IloSolver solver = resCt.getSolver();
	IlcManager manager = solver.getManager();
	IlcScheduler sched(solver);
	IlcActivity act = resCt.getActivity();
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IluProcPlan* procPlan = infoAct->getProcPlan();
	IlcFloat processPlanCost = procPlan->getCost();
	IlcStamp stamp = manager.getStamp();
//	IluMyStamp* iluStamp = (IluMyStamp*)solver.getObject();
	IlcStamp lastStamp = procPlan->getStamp();
//	IlcStamp lastStamp = iluStamp->getStamp();
//	IloInt procPlanIndex = procPlan->getProcPlanIndex();
	if((stamp != lastStamp)||(-IlcInfinity == processPlanCost)) {
		processPlanCost = 0.0;
		IloNum phi = procPlan->getPhi();
		IloNum gamma = procPlan->getGamma();
		IloInt dueDate = procPlan->getDueDate();
		IloInt procPlanEndMin = procPlan->getEndMin();
		IloInt refDate = IloMax(dueDate, procPlanEndMin);
		IloActivityArray actArray = procPlan->getActivities();
//		IloInt nbAct = actArray.getSize();
//		for(IloInt i = 0; i < nbAct; i++) {
		IloActivity activity = actArray[0];
		IlcInt actMinEndTime = activity.getEndMin();
		IluInfoActivity* infoActivity = (IluInfoActivity*)activity.getObject();
//		IloInt queue = infoActivity->getQueue();
		processPlanCost = phi * (IlcMax(0.0, IlcFloat(actMinEndTime - refDate)) + gamma * (actMinEndTime - refDate));
//		}
		procPlan->setCost(processPlanCost);
//		iluStamp->setStamp(stamp);
		procPlan->setStamp(stamp);
	}
	else
		processPlanCost = procPlan->getCost();
//	printf("Process plan %ld: cost = %.14f\n", procPlanIndex, processPlanCost);
	return processPlanCost;
}


ILOEVALUATOR1(IlcMyAltResConstraintRandomEvaluator, IlcAltResConstraint, altRCt, IloRandom, randGenerator) {
//	IlcActivity activity = altRCt.getActivity();
//	IlcSchedule sched = activity.getSchedule();
//	IlcManager manager = sched.getManager();
//	IloSolver solver = sched.getSolver();
//	IloEnv env = solver.getEnv();
//	manager = solver.getManager();
//	manager.getStamp();
//	if(manager.isInRecomputeMode()) {
//		IloNum temp;
//		manager.readRecomputeInfo(temp);
	//	IlcFloat val = randGenerator.getFloat();
	//	if(temp != val)
	//		printf("Warning: IlcMyAltResConstraintRandomEvaluator: random generator does not work properly!\n");
//		return temp;
//	}
//	else {
//		IloNum val = randGenerator.getFloat();
//		manager.writeRecomputeInfo(val);
//		return val;
//	}
	return randGenerator.getFloat();
}


ILOEVALUATOR1(IlcMyAltResConstraintMyRandomEvaluator, IlcAltResConstraint, altRCt, MyRandom*, randGenerator) {
	return randGenerator->getFloat();
}


ILOEVALUATOR0(IlcMyAltResConstraintAddressEvaluator, IlcAltResConstraint, altRCt) {
	return (IloNum)(IlcInt)altRCt.getImpl();
}


ILOEVALUATOR0(IlcMyResourceConstraintStartMinEvaluator, IlcResourceConstraint, resCt) {
//	IloSolver solver = resCt.getSolver();
//	IlcScheduler sched(solver);
	IlcActivity act = resCt.getActivity();
	IlcInt startMin = act.getStartMin();
	return startMin;
}


ILOEVALUATOR0(IlcMyResourceConstraintEndMaxEvaluator, IlcResourceConstraint, resCt) {
//	IloSolver solver = resCt.getSolver();
//	IlcScheduler sched(solver);
	IlcActivity act = resCt.getActivity();
	IlcInt endMax = act.getEndMax();
	return endMax;
}


ILOEVALUATOR0(IlcMyResourceConstraintDurationMinEvaluator, IlcResourceConstraint, resCt) {
	IlcActivity act = resCt.getActivity();
	IlcInt durationMin = act.getDurationMin();
	return durationMin;
}


ILOEVALUATOR0(IlcMyResourceConstraintAddressEvaluator, IlcResourceConstraint, resCt) {
	return (IloNum)(IlcInt)resCt.getImpl();
}


ILOEVALUATOR1(IlcMyResourceConstraintRandomEvaluator, IlcResourceConstraint, resCt, IloRandom, randGenerator) {
	return randGenerator.getFloat();
}


ILOEVALUATOR1(IlcMyResourceConstraintMyRandomEvaluator, IlcResourceConstraint, resCt, MyRandom*, randGenerator) {
	return randGenerator->getFloat();
}


ILOEVALUATOR1(IlcMyResourceRandomEvaluator, IlcResource, res, IloRandom, randGenerator) {
	return randGenerator.getFloat();
}


ILOEVALUATOR1(IlcMyResourceMyRandomEvaluator, IlcResource, res, MyRandom*, randGenerator) {
	return randGenerator->getFloat();
}


ILOEVALUATOR0(IlcMyResourceAddressEvaluator, IlcResource, res) {
	return (IloNum)(IlcInt)res.getImpl();
}


ILOCTXEVALUATOR0(IloMyGlobalCostPPEvaluator, IloActivityArray, actArray, IloSchedulerLargeNHood, nhood) {
	IloSchedulerSolution solution = nhood.getCurrentSolution();
	IloInt size = actArray.getSize();
	IloNum globalCost = 0.0;
	IloInt i;
	if(size > 0) {
		IloActivity act = actArray[0];
		IloNum actMinEndTime = solution.getEndMin(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
		IloInt queue = infoAct->getQueue();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat betaPhi = procPlan->getBetaPhi();
		IloInt dueDate = procPlan->getDueDate();
		IloNum tardi = IlcMax(0.0, IlcFloat(actMinEndTime + queue - dueDate)) + gamma * (actMinEndTime + queue - dueDate);
		IloInt currentAllocCost = 0;
		for(i = 1; i < size; i++) {
			IloActivity actPP = actArray[i];
			IluInfoActivity* infoAct = (IluInfoActivity*)actPP.getObject();
			IloInt actPPIndex = infoAct->getActIndex();
			IloInt* allocCosts = infoAct->getAllocCosts();
			IloResourceConstraint iloResCt = infoAct->getResCt();
			IloResource res = solution.getSelected(iloResCt);
			IloInt resIndex = (IloInt)res.getObject();
			currentAllocCost += allocCosts[resIndex];
		}
		globalCost =  currentAllocCost + betaPhi * tardi;
	}
	return globalCost;
}


ILOCTXEVALUATOR0(IloMyGlobalDeltaCostPPEvaluator, IloActivityArray, actArray, IloSchedulerLargeNHood, nhood) {
	IloSchedulerSolution solution = nhood.getCurrentSolution();
	IloInt nbTotalResources = 0;
	for(IloSchedulerSolution::ResourceIterator resIte(solution); resIte.ok(); ++resIte)
		nbTotalResources++;
	IloInt size = actArray.getSize();
	IloNum globalDeltaCost = 0.0;
	IloInt allocCostGain = 0;
	IloInt i, j;
	if(size > 0) {
		IloActivity act = actArray[0];
		IloNum actMinEndTime = solution.getEndMin(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat betaPhi = procPlan->getBetaPhi();
		IloInt dueDate = procPlan->getDueDate();
		IloNum tardi = IlcMax(0.0, IlcFloat(actMinEndTime - dueDate)) + gamma * (actMinEndTime - dueDate);
		IloInt* allocCosts = infoAct->getAllocCosts();
		IloResourceConstraint iloResCt = infoAct->getResCt();
		IloResource res = solution.getSelected(iloResCt);
		IloInt resIndex = (IloInt)res.getObject();
		IloInt currentAllocCost = allocCosts[resIndex];
		IloInt tempMinAllocCost = IloIntMax;
		for(j = 0; j < nbTotalResources; j++) {
			if((allocCosts[j] < IloIntMax)&&(allocCosts[j] < tempMinAllocCost))
				tempMinAllocCost = allocCosts[j];
		}
		allocCostGain = currentAllocCost - tempMinAllocCost;
		for(i = 1; i < size; i++) {
			IloActivity actPP = actArray[i];
			IluInfoActivity* infoAct = (IluInfoActivity*)actPP.getObject();
			IloInt actPPIndex = infoAct->getActIndex();
			allocCosts = infoAct->getAllocCosts();
			IloResourceConstraint iloResCt = infoAct->getResCt();
			res = solution.getSelected(iloResCt);
			resIndex = (IloInt)res.getObject();
			currentAllocCost += allocCosts[resIndex];
			tempMinAllocCost = IloIntMax;
			for(j = 0; j < nbTotalResources; j++) {
				if((allocCosts[j] < IloIntMax)&&(allocCosts[j] < tempMinAllocCost))
					tempMinAllocCost = allocCosts[j];
			}
			allocCostGain += currentAllocCost - tempMinAllocCost;
		}
		globalDeltaCost =  allocCostGain + betaPhi * tardi;
	}
	return globalDeltaCost;
}


ILOCTXEVALUATOR0(IloMyGainGlobalCostPPEvaluator, IloActivityArray, actArray, IloSchedulerLargeNHood, nhood) {
	IloSchedulerSolution solution = nhood.getCurrentSolution();
	IloInt nbTotalResources = 0;
	for(IloSchedulerSolution::ResourceIterator resIte(solution); resIte.ok(); ++resIte)
		nbTotalResources++;
	IloInt size = actArray.getSize();
	IloNum globalCost = 0.0;
	IloInt allocCostGain = 0;
	IloInt i, j;
	if(size > 0) {
		IloActivity act = actArray[0];
		IloNum actMinEndTime = solution.getEndMin(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat betaPhi = procPlan->getBetaPhi();
		IloInt dueDate = procPlan->getDueDate();
		IlcInt endMin = procPlan->getEndMin();
		IlcInt endRef = IlcMax(dueDate, endMin);
		IloNum tardi = IlcMax(0.0, IlcFloat(actMinEndTime - endRef)) + gamma * (actMinEndTime - endRef);

		IloInt* allocCosts = infoAct->getAllocCosts();
		IloResourceConstraint iloResCt = infoAct->getResCt();
		IloResource res = solution.getSelected(iloResCt);
		IloInt resIndex = (IloInt)res.getObject();
		IloInt currentAllocCost = allocCosts[resIndex];
		IloInt tempMinAllocCost = IloIntMax;
		for(j = 0; j < nbTotalResources; j++) {
			if((allocCosts[j] < IloIntMax)&&(allocCosts[j] < tempMinAllocCost))
				tempMinAllocCost = allocCosts[j];
		}
		allocCostGain = currentAllocCost - tempMinAllocCost;
		for(i = 1; i < size; i++) {
			IloActivity actPP = actArray[i];
			IluInfoActivity* infoAct = (IluInfoActivity*)actPP.getObject();
			IloInt actPPIndex = infoAct->getActIndex();
			allocCosts = infoAct->getAllocCosts();
			IloResourceConstraint iloResCt = infoAct->getResCt();
			res = solution.getSelected(iloResCt);
			resIndex = (IloInt)res.getObject();
			currentAllocCost += allocCosts[resIndex];
			tempMinAllocCost = IloIntMax;
			for(j = 0; j < nbTotalResources; j++) {
				if((allocCosts[j] < IloIntMax)&&(allocCosts[j] < tempMinAllocCost))
					tempMinAllocCost = allocCosts[j];
			}
			allocCostGain += currentAllocCost - tempMinAllocCost;
		}
		globalCost =  allocCostGain + betaPhi * tardi;
	}
	return globalCost;
}


ILOCTXEVALUATOR0(IloMyRandomPPEvaluator,
				  IloActivityArray, actArray,
				  IloSchedulerLargeNHood, nhood) {
	IloSchedulerSolution solution = nhood.getCurrentSolution();
	IloEnv env = solution.getEnv();
	IloRandom rand = env.getRandom();
	return rand.getFloat();
}


ILOCTXEVALUATOR0(IloMyTardiCostPPEvaluator, IloActivityArray, actArray, IloSchedulerLargeNHood, nhood) {
	IloSchedulerSolution solution = nhood.getCurrentSolution();
	IloInt size = actArray.getSize();
	IloNum tardiCost = 0.0;
	if(size > 0) {
		IloActivity act = actArray[0];
		IloNum actMinEndTime = solution.getEndMin(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat phi = procPlan->getPhi();
		IloInt dueDate = procPlan->getDueDate();
		tardiCost = phi * (IlcMax(0.0, IlcFloat(actMinEndTime - dueDate)) + gamma * (actMinEndTime - dueDate));
	}
	return tardiCost;
}


ILOCTXEVALUATOR0(IloMyGainTardiCostPPEvaluator, IloActivityArray, actArray, IloSchedulerLargeNHood, nhood) {
	IloSchedulerSolution solution = nhood.getCurrentSolution();
	IloInt size = actArray.getSize();
	IloNum tardiCost = 0.0;
	if(size > 0) {
		IloActivity act = actArray[0];
		IloNum actMinEndTime = solution.getEndMin(act);
		IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		IlcFloat gamma = procPlan->getGamma();
		IlcFloat phi = procPlan->getPhi();
		IloInt dueDate = procPlan->getDueDate();
		IlcInt procPlanEndMin = procPlan->getEndMin();
		IlcInt endRef = IlcMax(dueDate, procPlanEndMin);
		tardiCost = phi * (IlcMax(0.0, IlcFloat(actMinEndTime - endRef)) + gamma * (actMinEndTime - endRef));
	}
	return tardiCost;
}


ILOTRANSLATOR(IloMyRCtTranslator, IlcResourceConstraint, IlcActivity, act) {
	IlcAltResConstraintIterator altRCtIte(act);
	assert(altRCtIte.ok());				// only in DEBUG MODE
	IlcAltResConstraint altRCt = *altRCtIte;
	return altRCt.getResourceConstraint(altRCt.getSelected());
}


ILCGOAL1(IlcMyInstantiateIntVar,
		 IlcIntVar, var) {
	var.setMax(var.getMin());
	return 0;
}


ILCGOAL1(IlcMyInstantiateFloatVar,
		 IlcFloatVar, var) {
	var.setMax(IloCeil(var.getMin()));
	return 0;
}


ILOCPGOALWRAPPER1(IloMyInstantiate, solver,
				  IloNumVar, var) {
	if(var.getType() == IloNumVar::Int)
		return IlcMyInstantiateIntVar(solver, solver.getIntVar(var));
	else
		return IlcMyInstantiateFloatVar(solver, solver.getFloatVar(var));
}


ILCGOAL2(IlcMyMinimizeIntVar,
		 IlcIntVar, var,
		 IlcInt&, val) {
	var.setMax(val-1);
	return 0;
}


ILOCPGOALWRAPPER2(IloMyMinimizeIntVar,
				  solver,
				  IloNumVar, var,
				  IloInt&, val) {
	return IlcMyMinimizeIntVar(solver, solver.getIntVar(var), (IlcInt&)val);
}


ILCGOAL2(IlcMyMinimizeFloatVar,
		 IlcFloatVar, var,
		 IlcFloat&, val) {
	var.setMax(val-1.0);
	return 0;
}


ILOCPGOALWRAPPER2(IloMyMinimizeFloatVar,
				  solver,
				  IloNumVar, var,
				  IloNum&, val) {
		return IlcMyMinimizeFloatVar(solver, solver.getFloatVar(var), (IlcFloat&)val);
}


ILCGOAL0(IlcMyAssignDone) {
	printf("Assign is now done.\n");
	return 0;
}


ILCGOAL0(IlcMyRankDone) {
	printf("Rank is now done.\n");
	return 0;
}


ILOCPGOALWRAPPER1(IloMyAssignAlternativeAndRank, solver, IluHeuristic, heuristic) {
//	printf("Goal wrapper\n");
	IloSelector<IlcAltResConstraint, IlcAltResSet> assignAltRCSelector = heuristic.getAssignAltRCSelector();
	if(0 == assignAltRCSelector.getImpl()) {
		assignAltRCSelector = IlcAltResConstraintSel(solver);
		IloPredicate<IlcAltResConstraint> assignAltRCPredicate = heuristic.getAssignAltRCPredicate();
		if(assignAltRCPredicate.getImpl())
			assignAltRCSelector.setPredicate(assignAltRCPredicate);
		IloComparator<IlcAltResConstraint> assignAltRCComparator = heuristic.getAssignAltRCComparator();
		if(assignAltRCComparator.getImpl())
			assignAltRCSelector.setComparator(assignAltRCComparator);
	}

	IloSelector<IlcResource, IlcAltResConstraint> assignAltRSelector = heuristic.getAssignAltRSelector();
	if(0 == assignAltRSelector.getImpl()) {
		assignAltRSelector = IlcAltResourceSel(solver);
		IloPredicate<IlcResource> assignAlternativeResPredicate = heuristic.getAssignAltRPredicate();
		if(assignAlternativeResPredicate.getImpl())
			assignAltRSelector.setPredicate(assignAlternativeResPredicate);
		IloComparator<IlcResource> assignAlternativeResComparator = heuristic.getAssignAltRComparator();
		if(assignAlternativeResComparator.getImpl())
			assignAltRSelector.setComparator(assignAlternativeResComparator);
	}

	IloSelector<IlcResource, IlcSchedule> rankResSelector = heuristic.getRankResSelector();
	if(0 == rankResSelector.getImpl()) {
		rankResSelector = IlcResourceInScheduleSelector(solver);
		IloPredicate<IlcResource> rankResPredicate = heuristic.getRankResPredicate();
		if(rankResPredicate.getImpl())
			rankResSelector.setPredicate(rankResPredicate);
		IloComparator<IlcResource> rankResComparator = heuristic.getRankResComparator();
		if(rankResComparator.getImpl())
			rankResSelector.setComparator(rankResComparator);
	}

	IloSelector<IlcResourceConstraint, IlcResource> rankRCSelector = heuristic.getRankRCSelector();
	if(0 == rankRCSelector.getImpl()) {
		rankRCSelector = IlcResourceConstraintInResourceSelector(solver);
		IloPredicate<IlcResourceConstraint> rankRCPredicate = heuristic.getRankRCPredicate();
		if(rankRCPredicate.getImpl())
			rankRCSelector.setPredicate(rankRCPredicate);
		IloComparator<IlcResourceConstraint> rankRCComparator = heuristic.getRankRCComparator();
		if(rankRCComparator.getImpl())
			rankRCSelector.setComparator(rankRCComparator);
	}

	IlcScheduler sched(solver);
	IlcAltResSet altResSet;
	IlcAltResSetIterator altResSetIte(sched);
	altResSet = *altResSetIte;
	/*	for(IlcAltResConstraintIterator altResCtIte(altResSet); altResCtIte.ok(); ++altResCtIte) {
			IlcAltResConstraint altResCt = *altResCtIte;
			IlcActivity ilcAct = altResCt.getActivity();
			IloActivity iloAct = sched.getExtractable(ilcAct);
			printf("");
			for(IlcResourceIterator resIte(sched); resIte.ok(); ++resIte) {
				IlcResource ilcRes = *resIte;
				if(altResCt.isPossible(ilcRes))
					printf("A resource is still possible.\n");
			}
		}*/

	IlcGoal goal =
	//	IlcAnd(IlcAssignAlternative(altResSet, assignAltRSelector, assignAltRCSelector), IlcRank(sched, rankResSelector, rankRCSelector))
		IlcAnd(IlcAssignAlternative(altResSet, assignAltRSelector, assignAltRCSelector), IlcMyRank(sched, heuristic.getRankRCComparator()))
//		IlcAnd(IlcAssignAlternative(altResSet, assignAltRSelector, assignAltRCSelector), IlcMyAssignDone(solver), IlcRank(sched, rankResSelector, rankRCSelector),
//			IlcMyRankDone(solver))
	//	IlcAssignAlternative(altResSet, assignAltRSelector, assignAltRCSelector)
	//	IlcRank(sched, rankResSelector, rankRCSelector)
		;

	return goal;
}


ILOCPGOALWRAPPER1(IloMyAssignAlternativeAndSetTimes, solver, IluHeuristic, heuristic) {
	
	IloSelector<IlcAltResConstraint, IlcAltResSet> assignAltRCSelector = heuristic.getAssignAltRCSelector();
	if(0 == assignAltRCSelector.getImpl()) {
		assignAltRCSelector = IlcAltResConstraintSel(solver);
		IloPredicate<IlcAltResConstraint> assignAltRCPredicate = heuristic.getAssignAltRCPredicate();
		if(assignAltRCPredicate.getImpl())
			assignAltRCSelector.setPredicate(assignAltRCPredicate);
		IloComparator<IlcAltResConstraint> assignAltRCComparator = heuristic.getAssignAltRCComparator();
		if(assignAltRCComparator.getImpl())
			assignAltRCSelector.setComparator(assignAltRCComparator);
	}

	IloSelector<IlcResource, IlcAltResConstraint> assignAltRSelector = heuristic.getAssignAltRSelector();
	if(0 == assignAltRSelector.getImpl()) {
		assignAltRSelector = IlcAltResourceSel(solver);
		IloPredicate<IlcResource> assignAlternativeResPredicate = heuristic.getAssignAltRPredicate();
		if(assignAlternativeResPredicate.getImpl())
			assignAltRSelector.setPredicate(assignAlternativeResPredicate);
		IloComparator<IlcResource> assignAlternativeResComparator = heuristic.getAssignAltRComparator();
		if(assignAlternativeResComparator.getImpl())
			assignAltRSelector.setComparator(assignAlternativeResComparator);
	}

	IloSelector<IlcActivity, IlcSchedule> setTimesActSelector = heuristic.getSetTimesActSelector();
	if(0 == setTimesActSelector.getImpl()) {
		setTimesActSelector = IlcActivityInScheduleSelector(solver);
		IloPredicate<IlcActivity> setTimesActPredicate = heuristic.getSetTimesActPredicate();
		if(setTimesActPredicate.getImpl())
			setTimesActSelector.setPredicate(setTimesActPredicate);
		IloComparator<IlcActivity> setTimesActComparator = heuristic.getSetTimesActComparator();
		if(setTimesActComparator.getImpl())
			setTimesActSelector.setComparator(setTimesActComparator);
	}

	IlcScheduler sched(solver);
	IlcAltResSet altResSet;
	IlcAltResSetIterator altResSetIte(sched);
	altResSet = *altResSetIte;

	IlcGoal goal =
		IlcAnd(IlcAssignAlternative(altResSet, assignAltRSelector, assignAltRCSelector), IlcSetTimes(sched))		// by default StartMin then EndMax
	//	IlcAnd(IlcAssignAlternative(altResSet, assignAltRSelector, assignAltRCSelector), IlcSetTimes(sched, setTimesActSelector))
		;

	return goal;
}


ILOSELECTOR1(IloMyAllocCostResourceInAltResCtSelector,
			 IlcResource,
			 IlcAltResConstraint, altResCt,
			 IloRandom, randGenerator) {
	IloSolver solver = altResCt.getSolver();
	IlcScheduler sched(solver);
	IlcAltResSet altResSet = altResCt.getAltResSet();
	IlcActivity act = altResCt.getActivity();
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloNum omega = infoAct->getProcPlan()->getOmega();
	IloInt* allocCosts = infoAct->getAllocCosts();
//	IloNum* allocProbs = infoAct->getAllocProbs();
	IloInt numberOfResources = altResSet.getSize();
	IlcResource bestRes;
	IloInt i;
	IloNum allocMean = 0.0;
	IloInt allocMin = IloIntMax;
	IloInt nbPossible = 0;

	for(i = 0; i < numberOfResources; i++) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			nbPossible++;
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			allocMean +=  allocCosts[resIndex];
			if(allocCosts[resIndex] < allocMin)
				allocMin = allocCosts[resIndex];
		}
	}
	
	allocMean /= nbPossible;
	IloNum denominator = (allocMean - omega * allocMin) / (omega - 1.0);

	IloNum sum = 0.0;
	for(i = 0; i < numberOfResources; i++) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			sum += 1 / (denominator + allocCosts[resIndex]);
		}
	}
	IloNum nominator = 1 / sum;

	IloNum val = randGenerator.getFloat();
	i = 0;
	while(val > 0.0) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			bestRes = res;
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			val -= nominator / (denominator + allocCosts[resIndex]);
		}
		i++;
	}

	setCurrentBest(bestRes);
}


ILOSELECTOR1(IloMyAllocCostResourceInAltResCtMySelector,
			 IlcResource,
			 IlcAltResConstraint, altResCt,
			 MyRandom*, randGenerator) {
	IloSolver solver = altResCt.getSolver();
	IlcScheduler sched(solver);
	IlcAltResSet altResSet = altResCt.getAltResSet();
	IlcActivity act = altResCt.getActivity();
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloNum omega = infoAct->getProcPlan()->getOmega();
	IloInt* allocCosts = infoAct->getAllocCosts();
//	IloNum* allocProbs = infoAct->getAllocProbs();
	IloInt numberOfResources = altResSet.getSize();
	IlcResource bestRes;
	IloInt i;
	IloNum allocMean = 0.0;
	IloInt allocMin = IloIntMax;
	IloInt nbPossible = 0;

	for(i = 0; i < numberOfResources; i++) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			nbPossible++;
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			allocMean +=  allocCosts[resIndex];
			if(allocCosts[resIndex] < allocMin)
				allocMin = allocCosts[resIndex];
		}
	}
	
	allocMean /= nbPossible;
	IloNum denominator = (allocMean - omega * allocMin) / (omega - 1.0);

	IloNum sum = 0.0;
	for(i = 0; i < numberOfResources; i++) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			sum += 1 / (denominator + allocCosts[resIndex]);
		}
	}
	IloNum nominator = 1 / sum;

	IloNum val = randGenerator->getFloat();
	i = 0;
	while(val > 0.0) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			bestRes = res;
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			val -= nominator / (denominator + allocCosts[resIndex]);
		}
		i++;
	}

	setCurrentBest(bestRes);
}


/*ILOSELECTOR1(IloMyLoadResourceInAltResCtMySelector,
			 IlcResource,
			 IlcAltResConstraint, altResCt,
			 MyRandom*, randGenerator) {
	IloSolver solver = altResCt.getSolver();
	IlcScheduler sched(solver);
	IlcAltResSet altResSet = altResCt.getAltResSet();
	IlcActivity act = altResCt.getActivity();
	IloActivity iloAct = sched.getExtractable(act);
	IluInfoActivity* infoAct = (IluInfoActivity*)iloAct.getObject();
	IloNum omega = infoAct->getProcPlan()->getOmega();
	IloInt numberOfResources = altResSet.getSize();
	IlcResource bestRes;
	IloInt i;
	IloNum loadMean = 0.0;
	IloInt loadMin = IloIntMax;
	IloInt nbPossible = 0;

	for(i = 0; i < numberOfResources; i++) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			nbPossible++;
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			allocMean +=  allocCosts[resIndex];
			if(allocCosts[resIndex] < allocMin)
				allocMin = allocCosts[resIndex];
		}
	}
	
	allocMean /= nbPossible;
	IloNum denominator = (allocMean - omega * allocMin) / (omega - 1.0);

	IloNum sum = 0.0;
	for(i = 0; i < numberOfResources; i++) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			sum += 1 / (denominator + allocCosts[resIndex]);
		}
	}
	IloNum nominator = 1 / sum;

	IloNum val = randGenerator->getFloat();
	i = 0;
	while(val > 0.0) {
		IlcResource res = altResSet[i];
		if(altResCt.isPossible(res)) {
			bestRes = res;
			IloResource iloRes = sched.getExtractable(res);
			IloInt resIndex = (IloInt)iloRes.getObject();
			val -= nominator / (denominator + allocCosts[resIndex]);
		}
		i++;
	}

	setCurrentBest(bestRes);
}*/


ILCGOAL1(IlcPrintIntLowerBound,
		 IlcIntVar, var) {
	printf("After first propagation: lower bound = %ld\n", var.getMin());
	return 0;
}


ILCGOAL1(IlcPrintFloatLowerBound,
		 IlcFloatVar, var) {
	printf("After first propagation: lower bound = %.14f\n", var.getMin());
	return 0;
}


ILOCPGOALWRAPPER1(IloPrintLowerBound,
				  solver,
				  IloNumVar, costVar) {
	if(costVar.getType() == IloNumVar::Int)
		return IlcPrintIntLowerBound(solver, solver.getIntVar(costVar));
	else
		return IlcPrintFloatLowerBound(solver, solver.getFloatVar(costVar));
}


ILCGOAL1(IlcProcPlanEndMin,
		 IloArray<IloActivityArray>, actArrays) {
	IloSolver solver = getSolver();
	IlcScheduler sched(solver);
	IloInt nbProcPlans = actArrays.getSize();
	IloInt i;
	printf("Minimum completion time of each job is stored.\n");
	for(i = 0; i < nbProcPlans; i++) {
		IloActivityArray actArray = actArrays[i];
		IloActivity activity = actArray[0];
		IlcActivity ilcAct = sched.getActivity(activity);
		IluInfoActivity* infoAct = (IluInfoActivity*)activity.getObject();
		IluProcPlan* procPlan = infoAct->getProcPlan();
		procPlan->setEndMin(ilcAct.getEndMin());
	}
	return 0;
}


ILOCPGOALWRAPPER1(IloProcPlanEndMin, solver, IloArray<IloActivityArray>, actArrays) {
	return IlcProcPlanEndMin(solver, actArrays);
}


ILCGOAL1(IlcPrintIntSolution,
		 IlcIntVar, var) {
	printf("Lower bound = %ld\n", var.getMin());
	printf("Upper bound = %ld\n", var.getMax());
	IloSolver solver = var.getSolver();
	IloEnv env = solver.getEnv();
	IlcScheduler sched(solver);
	for(IloIterator<IloActivity> ite(env); ite.ok(); ++ite)
		env.out() << sched.getActivity(*ite) << endl;
	return 0;
}


ILCGOAL1(IlcPrintFloatSolution,
		 IlcFloatVar, var) {
	printf("Lower bound = %.14f\n", var.getMin());
	printf("Upper bound = %.14f\n", var.getMax());
	IloSolver solver = var.getSolver();
	IlcScheduler sched(solver);
	IloEnv env = solver.getEnv();
	for(IloIterator<IloActivity> ite(env); ite.ok(); ++ite)
		env.out() << sched.getActivity(*ite) << endl;
	return 0;
}


ILOCPGOALWRAPPER1(IloPrintSolution, solver, IloNumVar, costVar) {
	if(costVar.getType() == IloNumVar::Int)
		return IlcPrintIntSolution(solver, solver.getIntVar(costVar));
	else
		return IlcPrintFloatSolution(solver, solver.getFloatVar(costVar));
}


ILCGOAL1(IlcMessageGoal,
		 const char*, message) {
	IloSolver solver = getSolver();
	solver.out() << message << endl;
	return 0;
}


ILOCPGOALWRAPPER0(IloMyPrintEndSubGoal, solver) {
	return IlcMessageGoal(solver, "Subgoal ends.");
}


ILOCPGOALWRAPPER0(IloMyPrintStartSubGoal, solver) {
	return IlcMessageGoal(solver, "Subgoal starts.");
}


ILCGOAL2(IlcMyRankIntern,
		 IlcSchedule, schedule,
		 IloComparator<IlcResourceConstraint>, rankRCComparator) {
	IloNum bestValue = IloInfinity;
	IlcResourceConstraint bestRCt;
	for(IlcUnaryResourceIterator rIte(schedule); rIte.ok(); ++rIte) {
		IlcUnaryResource res = *rIte;
		for(IlcResource::ResourceConstraintIterator rCtIte(res, IlcResource::PossibleFirst); rCtIte.ok(); ++rCtIte) {
			IlcResourceConstraint rCt = *rCtIte;
			if((0 == bestRCt.getImpl())||(rankRCComparator(bestRCt, rCt) > 0))
				bestRCt = rCt;
		}	
	}
	if(0 == bestRCt.getImpl())
		return 0;
	else
		return(IlcAnd(IlcTryRankFirst(bestRCt), this));
}


IlcGoal IlcMyRank(IlcSchedule schedule, IloComparator<IlcResourceConstraint> rankRCComparator) {
	IloSolver solver = schedule.getSolver();
	return IlcMyRankIntern(solver, schedule, rankRCComparator);
}