// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilumakespan.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


#include<ilsim/iluprocessplan.h>
#include<ilsim/iluprintsolution.h>
#include<ilsim/ilusdxlsolutionoutput.h>

ILOSTLBEGIN



// The file contains the PrintRange function, the PrintSolution function,



///////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////

void
PrintRange(IloEnv& env, IloNum min, IloNum max) {
	if (min == max)
		env.out() << (IlcInt)min;
	else
		env.out() << (IlcInt)min << ".." << (IlcInt)max;
}

void
PrintSolution(const IloSchedulerSolution solution,
			  const IloNumVar costVar) {
	IloEnv env = solution.getEnv();
	if (solution.contains(costVar)) {
		env.out() << "Solution with ";
		if (costVar.getName())
			env.out() << costVar.getName();
		else
			env.out() << "cost";
		env.out() << " ["
		<< solution.getMin(costVar) << ".." 
		<< solution.getMax(costVar) << "]" << endl;
	}

	for (IloSchedulerSolution::ResourceConstraintIterator iter(solution); iter.ok(); ++iter) {
		IloResourceConstraint rCt = *iter;
		if (!solution.isResourceSelected(rCt))
			IloSchedulerException("No resource assigned!");

		IloActivity activity = rCt.getActivity();
		env.out() << activity.getName() << "[";
		PrintRange(env, 
               solution.getStartMin(activity),  
               solution.getStartMax(activity));
		env.out() << " -- ";
		PrintRange(env, 
               solution.getDurationMin(activity),
               solution.getDurationMax(activity));
		env.out() << " --> ";
		PrintRange(env, 
               solution.getEndMin(activity),
               solution.getEndMax(activity));
		env.out() << "]: " << solution.getSelected(rCt).getName()
              << endl;
	}
}

void
PrintSolution(const							IloSchedulerSolution solution,
			  IloArray<IloActivityArray>	actArrays,
			  const IloNumVar				costVar,
			  bool							sdxlOutput) {
	IloEnv env = solution.getEnv();
	if(solution.contains(costVar)) {
		env.out() << "Solution with ";
		if (costVar.getName())
			env.out() << costVar.getName();
		else
		  env.out() << "cost";
		env.out() << " ["
			<< solution.getMin(costVar) << ".." 
			<< solution.getMax(costVar) << "]" << endl;
	}
	
	IloInt nbTotalResources = 0;
	for(IloSchedulerSolution::ResourceIterator resIte(solution); resIte.ok(); ++resIte)
		nbTotalResources++;

	IloNum makespan = 0.0;
	for(IloSchedulerSolution::ActivityIterator actIter(solution); actIter.ok(); ++actIter) {
		IloActivity activity = *actIter;
		IloNum tempEnd = solution.getEndMin(activity);
		if(tempEnd > makespan)
			makespan = tempEnd;
	}
	printf("Makespan is equal to %f\n", makespan);

	IloInt nbProcPlans = actArrays.getSize();
	if(nbProcPlans > 0) {
		for(IloInt i = 0; i < nbProcPlans; i++) {
			IloActivityArray actArray = actArrays[i];
			IloActivity act = actArray[0];
			IloInt actMinEndTime = solution.getEndMin(act);
			IluInfoActivity* infoAct = (IluInfoActivity*)act.getObject();
			IluProcPlan* infoProcPlan = infoAct->getProcPlan();
			IloInt procPlanIndex = infoProcPlan->getProcPlanIndex();
			IloInt dueDate = infoProcPlan->getDueDate();
			if(actMinEndTime > dueDate) {
				IloNum phi = infoProcPlan->getPhi();
				printf("Process plan %ld is late:\tdue date = %ld\tphi = %.4f\t", procPlanIndex, dueDate, phi);
				IloNum tardiCost = phi * (actMinEndTime - dueDate);
				printf("tardiness cost is equal to %.14f.\n", tardiCost);
			}
		}
	}

	printf("\n");

	for(IloSchedulerSolution::ResourceConstraintIterator iter(solution); iter.ok(); ++iter) {
		IloResourceConstraint rCt = *iter;
		if (!solution.isResourceSelected(rCt))
			IloSchedulerException("No resource assigned!");

		IloActivity activity = rCt.getActivity();
		IluInfoActivity* infoAct = (IluInfoActivity*)activity.getObject();
		IloInt actIndex = infoAct->getActIndex();
		IloInt* allocCosts = infoAct->getAllocCosts();
		IloInt minAlloc = IloIntMax;
		IloInt maxAlloc = 0;
		if(allocCosts != 0) {
			for(IloInt i = 0; i < nbTotalResources; i++) {
				IloInt tempCost = allocCosts[i];
				if(tempCost < IloIntMax) {
					if(tempCost < minAlloc)
						minAlloc = tempCost;
					if(tempCost > maxAlloc)
						maxAlloc = tempCost;
				}
			}
		}

		IloResource resource = solution.getSelected(rCt);
		IloInt resIndex = (IloInt)resource.getObject();

		IloNum percent = 0.0;
		if(allocCosts != 0) {
			IloInt curAlloc = allocCosts[resIndex];
			if(maxAlloc != minAlloc)
				percent = 100 - 100 * (maxAlloc - curAlloc) / (maxAlloc - minAlloc);
		}

//		env.out() << "resource index = " << resIndex << endl;
/*		env.out() << activity.getName() << "[";
		PrintRange(env,
				   solution.getStartMin(activity),
				   solution.getStartMax(activity));
		env.out() << " -- ";
		PrintRange(env,
				   solution.getDurationMin(activity),
				   solution.getDurationMax(activity));
		env.out() << " --> ";
		PrintRange(env,
				   solution.getEndMin(activity),
				   solution.getEndMax(activity));
		env.out() << "]: " << resource.getName();
		if(allocCosts != 0)
			env.out() << "; relative allocation cost = " << percent << "%";
		env.out() << endl;*/
	}
	
	IloNum ratio = makespan / 33000;
	if(IloTrue == sdxlOutput) {
		IloSDXLSchedSolOutput output(env);
		printf("An XML file is created.\n");
		ofstream outFile("solution.xml");
		output.writeSchedSol(solution, outFile, actArrays, ratio);
		outFile.close();
	}
}


void
PrintSolution(IloEnv& env,
			  const IlcScheduler& scheduler) {
	for(IloIterator<IloActivity> ite(env); ite.ok(); ++ite) {
		env.out() << scheduler.getActivity(*ite) << endl;
	}
}