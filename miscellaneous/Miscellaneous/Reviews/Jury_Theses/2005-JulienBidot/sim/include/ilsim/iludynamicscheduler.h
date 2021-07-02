// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------



// This file contains the Allocation class, the IluDynamicSchedulerI class, the IluPriorityHeap class, the IluProgressiveSchedulerI class, and the IluReactiveSchedulerI class.



#ifndef __SIM_iludynamicschedulersimH
#define __SIM_iludynamicschedulersimH


#include "iluevent.h"
#include "ilusimulator.h"
#include "ilusolver.h"




///////////////////////////////////////////////////////////////////////////////
//
// THIS CLASS IS USED TO CHECK SOLUTION WITH SCHEDULER FOR A DETERMINISTIC
// PROBLEM.
//
///////////////////////////////////////////////////////////////////////////////

class Allocation {
private:
		IloInt		_id;						// Activity index
		Allocation*	_next;
public:
	Allocation(IloInt id)
		:_id (id), _next(0) {}
	~Allocation(){}
	Allocation* getNext() const { return _next; }
	void setNext(Allocation* next) { _next = next; }
	IloInt getId() const { return _id; }
};




enum DynamicSchedulerType { ProgressiveType, ReactiveType, ProgressiveReactiveType };

// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO BUILD A DYNAMIC SCHEDULER THAT 
// SETS ACTIVITY START TIMES. IT IS USED TO SIMULATE
// SCHEDULE EXECUTION IN A NON-DETERMINISTIC ENVIRONMENT.
// --------------------------------------------------------------------------

class IluDynamicSchedulerI {
private:
	IluSimulatorI*	_allocSim;				// _allocSim permits us to simulate non executed, allocated activities.
	IloInt*			_curAllocations;
	// _curAllocations is an array of size nbTotalActivities + 1 that contains current allocations.
	IloNum			_effectAllocCost;		// _effectAllocCost is the effective partial allocation cost.
	Allocation**	_effectAllocations;		// _effectAllocations is an array of size nbTotalResources that contains effective ordered allocations by resource.

protected:
	IluDynamicSchedulerI(): _allocSim(0), _curAllocations(0), _effectAllocCost(0.0), _effectAllocations(0) {}
	IluDynamicSchedulerI(IluSimulatorI* allocSim, IloInt* curAllocations, Allocation** effectAllocations): _allocSim(allocSim), _curAllocations(curAllocations),
																								_effectAllocCost(0.0), _effectAllocations(effectAllocations) {}

public:
	~IluDynamicSchedulerI() {}
	void execute(IluEventI*, IluTimeEventManagerI*);

	IluSimulatorI* getSimulator() const { return _allocSim; }
	void setSimulator(IluSimulatorI* simulatorP) { _allocSim = simulatorP; }
	virtual IloInt getType() const = 0;
	
	IloInt getCurAllocation(IloInt actIndex) { if(0 != _curAllocations) return _curAllocations[actIndex]; else { printf("The current allocation array is empty!\n"); return -1; } }
	void setCurAllocation(IloInt actIndex, IloInt resIndex) { if(0 != _curAllocations) _curAllocations[actIndex] = resIndex; else printf("The current allocation array is empty!\n"); }
	
	IloNum getEffectAllocCost() const { return _effectAllocCost; }
	void addEffectAllocCost(IloNum allocCost) { _effectAllocCost += allocCost; }

	IloInt getEffectAllocation(IloInt actIndex);// { if(0 != _effectAllocations) return _effectAllocations[actIndex]; else { printf("The effective allocation array is empty!\n"); return -1; } }
	void setEffectAllocation(Allocation* newEffectAllocation, IloInt resIndex);// { if(0 != _effectAllocations) _effectAllocations[actIndex] = resIndex; else printf("The effective allocation array is empty!\n"); }
};




//-----------------------------------------------------------------
// THIS CLASS ALLOWS US TO GET ELIGIBLE ACTIVITIES WITH RESPECT
// TO THEIR PRIORITY.
//-----------------------------------------------------------------

class IluPriorityHeap {
public:
	class Node {
		IloNum	_key;
		IloAny	_object;
		IloInt	_position;
	public:
		Node() {}
		Node(IloAny object, IloNum key = IloInfinity) 
			: _key(key), _object(object), _position(-1) {}
		~Node() {}
		IloInt getPosition() const { return _position; }
		void setPosition(IloInt position) { _position = position; }
		inline IloNum getKey() const { return _key; }
		IloAny getObject() const { return _object; }
		void setKey(IloNum key) { _key = key; }
	};
private:
  Node**  _elems;
  IloInt  _maxSize;
  IloInt  _size;

  void downHeap(IloInt);
  void upHeap(IloInt);
  void resize();

public:
	IluPriorityHeap() : _elems(0), _maxSize(-1), _size(0) {}
	IluPriorityHeap(IloInt size);
	~IluPriorityHeap();
	IloBool contains(IluPriorityHeap::Node* node) const { 
		IloInt position = ((Node*)node)->getPosition();
		return position != -1 && position <= _size && _elems[position] == node; 
	}
	void insert(IluPriorityHeap::Node* node);
	IloInt getSize() const { return _size; }
	IloBool isEmpty() const { return (_size == 0); }
	IluPriorityHeap::Node* minimum() const { return _elems[1]; }
	IluPriorityHeap::Node* extractMin();
	void empty() { _size = 0; }
	void changeKey(IluPriorityHeap::Node* node, IloNum key) {
		if(node->getKey() > key) {
			node->setKey(key);
			upHeap(((Node*)node)->getPosition());
		}
		else {
			node->setKey(key);
			downHeap(((Node*)node)->getPosition());
		}

	}
};






// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO BUILD A PROGRESSIVE SCHEDULER THAT
// SETS ACTIVITY START TIMES. IT IS USED TO SIMULATE
// SCHEDULE EXECUTION IN A NON-DETERMINISTIC ENVIRONMENT.
// THE PARAMETER deltaTProgressMin SETS THE MIN
// HORIZON UNDER WHICH WE TRY TO SELECT A SUBSET OF PENDING ACTIVITIES.
// THE PARAMETER sigmaTProgressMin SETS THE STANDARD DEVIATION OF THE
// ALLOCATED ACTIVITY UNDER WHICH WE TRY TO SELECT A SUBSET OF PENDING
// ACTIVITIES. AN INSTANCE OF THIS CLASS FINDS A FIRST SOLUTION.
// --------------------------------------------------------------------------

class IluProgressiveSchedulerI: public IluDynamicSchedulerI {
private:
	IloSchedulerSolution		_solution;		// _solution represents a partial solution of a submodel of the global model of the complete problem.
	IloInt						_deltaTProgressMin, _deltaTProgressMax;
	IloNum						_sigmaTProgressMin, _sigmaTProgressMax;
	// _deltaTProgressMin and _sigmaTProgressMin permit us to determine when we try to
	// select a subset of pending activities (the selection process starts).
	// _deltaTProgressMax and _sigmaTProgressMin permit us to determine when we stop
	// the selection process. There might possibly be several selection processes during a schedule execution.
	// _maxPriorityActIndex gives the index of the eligible activity that has the biggest priority currently.
	IluSimulatorOneActivityI*	_selecSim;	// _selecSim permits us to simulate a selected activity.
	IluFrontierI*				_currentFrontier, * _allocationFrontier, * _selectionFrontier;
	// _currentFrontier is the frontier at the current time
	// _allocationFrontier is the frontier at the end of the last allocated activity
	// _selectionFrontier is the frontier at the end of the last selected activity
	IluLocalSearchSolverI*		_localSSolver;
	IluHistogramI*				_earliestEndTimeResource, * _earliestBreakdown, * _currentEndTimeResource, * _currentBreakdown;
	// _earliestEndTimeResource, _earliestBreakdown, _currentEndTimeResource, and _currentBreakdown permit us to allocate a selected activity.
	IloNum*						_allocCostInvSum, * _energyResources, * _criticities, * _extendedMDur, * _endPExp, * _tardiExp, * _weights,* _pendingDur,
										* _meanPendingDur;
	// _allocCostInvSum is an array of size nbTotalActivities + 1.
	// _energyResources is an array of size nbTotalResources.
	// _criticities is an array of size nbTotalActivities + 1.
	// _extendedMDur is an array of size nbTotalActivities + 1.
	// _endPExp is an array of size nbProcessPlans.
	// _tardiExp is an array of size nbProcessPlans.
	// _weights is an array of size nbProcessPlans.
	// _pendingDur is an array of size nbProcessPlans;
	// _meanPendingDur is an array of size nbProcessPlans;
	IloNum**					_energyActivities;		// _energyActivities is a matrix of size (nbTotalActivities + 1)*(nbResources).
	ItemI*						_recycling, * _eligible, * _pending, * _lastSelectedActivity;
	// _recycling is a list that contains some ItemI objects recycled from _eligible.
	// _eligible is a list that contains indices of the currently elibible activities.
	// _pending is a list that contains indices of the currently pending activities.
	// _lastSelectedActivity contains the index of the last activity that has been selected.
	IluPriorityHeap				_priorityHeap;
	IluPriorityHeap::Node**		_nodes;			// _nodes is an array of size nbTotalActivities + 1 and contains the nodes corresponding to eligible activities.
		
public:
	IluProgressiveSchedulerI()
		: IluDynamicSchedulerI		(),
		  _deltaTProgressMin		(-1),
		  _deltaTProgressMax		(-1),
		  _sigmaTProgressMin		(0.0),
		  _sigmaTProgressMax		(0.0),
		  _selecSim					(),
		  _currentFrontier			(0),
		  _allocationFrontier		(0),
		  _selectionFrontier		(0),
		  _localSSolver				(0),
		  _earliestEndTimeResource	(0),
		  _earliestBreakdown		(0),
		  _currentEndTimeResource	(0),
		  _currentBreakdown			(0),
		  _allocCostInvSum			(0),
		  _energyActivities			(0),
		  _energyResources			(0),
		  _criticities				(0),
		  _extendedMDur				(0),
		  _endPExp					(0),
		  _tardiExp					(0),
		  _weights					(0),
		  _pendingDur				(0),
		  _meanPendingDur			(0),
		  _recycling				(0),
		  _eligible					(0),
		  _pending					(0),
		  _lastSelectedActivity		(0),
		  _priorityHeap				(0),
		  _nodes					(0)	{}
	IluProgressiveSchedulerI(IluLocalSearchSolverI*	lSSolver,
							 IloInt					deltaTProgressMin,
							 IloInt					deltaTProgressMax,
							 IloNum					sigmaTProgressMin,
							 IloNum					sigmaTProgressMax,
							 IluSimulatorI*			simP,
							 IloInt*				curAllocations,
							 Allocation**			effectAllocations);

	~IluProgressiveSchedulerI() {}

	void updatePG(IloEnv env, IloNum currentTime);		// Initialization of _allocSim by retrieving precedence constraints of _solution
														// and some activities of the model. This member funciton is called each time the selection process
														// is run.

	void updateAllocFrontier();	// The member function updateAllocFrontiers() updates the allocation frontier after having selected
									// a new subset of pending activities and optimized the solution to this subproblem.

	IloBool timeForSelection(IloNum currentTime);

	void updateCurFrontier();		// The member function updateCurFrontier() updates the current frontier with respect to executed activities.

	IloInt select(IloInt currentTime);	// A heuristic is used to compute priorities, associate them with the eligible activities, and return
										// the index of the eligible activity with the highest priority or -1 if no activity is selected.
	IloInt selectActivities(IloNum time); // selectActivities(IloNum time) selects some activities and returns the number of activities that have been selected.

	void computeEligiblePriorities(IloNum currentTime);

	IluSimulatorOneActivityI* getSimulatorOneActivity() const { return _selecSim; }

	void updateSolution();				// The member function updateSolution() updates the scheduler solution _solution after having selected an activity.

	void initializeSolution() { if(0 != _localSSolver) _solution = IloSchedulerSolution(_localSSolver->getEnv());
								else printf("The local search solver has not yet been created!\n"); }

	IloBool eligibleActivities() { if(_eligible != 0) return IloTrue; else return IloFalse; }

	IluFrontierI* getAllocationFrontier() const { return _allocationFrontier; }
	IluFrontierI* getSelectionFrontier() const { return _selectionFrontier; }

	IloSchedulerSolution getSolution() const { return _solution; }

	ItemI* getItem(IloInt index, ItemI* itemList);

	ItemI* getFirstRecyclingItem() const { return _recycling; }
	ItemI* getFirstEligibleItem() const { return _eligible; }
	ItemI* getFirstPendingItem() const { return _pending; }

	IloInt getType() const { return ProgressiveType; }

	IloNum computeEffectTardiCost();

	IluLocalSearchSolverI* getLocalSearchSolver() const { return _localSSolver; }
};




/*

// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO BUILD A REACTIVE SCHEDULER THAT
// SETS ACTIVITY START TIMES. IT IS USED TO SIMULATE SCHEDULE
// EXECUTION IN A NON-DETERMINISTIC ENVIRONMENT. THE PARAMETER sensitivity
// SETS THE RE-SCHEDULING THRESHOLD threshold.
// --------------------------------------------------------------------------

class IluReactiveSchedulerI: public IluDynamicSchedulerI {
private:
	IluTreeSearchSolverI*	_treeSSolver;
	IlcScheduler			_sched;									// _sched is a complete solution found by using a tree search.
	IluHistogramI*			_histP;									// _hist stores realizations of the makespan random variable.
	IluNormalVariableI*		_normVarP;							// _normVarP is associated to the makespan.
	IloNum					_threshold, _sensitivity, _processingTimeMean;	// _threshold = mean makespan of the indicative schedule
															// _sensitivity is the sensitivity factor.
															// _processingTimeMean is the mean of all the processing times of
															// the activities of the deterministic problem.
	IlcInt					_nbActivities;							// _nbActivities is the number of activities that have been executing
													// since the last rescheduling occurred and are going to execute.
	IloInt					_criterion;								// criterion number,	0: no rescheduling occurs
													//						1: makespan monitoring
													//						2: absolute makespan monitoring
													//						3: monitoring of the estimated activity start times
													//						4: monitoring of the effective activity start times
													//						5: monitoring of the estimated activity start times with standard deviations

public:
	IluReactiveSchedulerI(): IluDynamicSchedulerI(), _histP(0), _normVarP(0) {}
	IluReactiveSchedulerI(IloEnv env, IlcScheduler sched, IloRandom randGen, IloNum sensitivity, IloInt criterion);
	
	~IluReactiveSchedulerI() {}

	void initPG(IloEnv, IlcScheduler, IloNum);

	IloNum getThreshold() const { return _threshold; }

	IluHistogramI* getHist() { return _histP; }

	IlcScheduler getSched() { return _sched; }

	IlcBool reschedule(IlcScheduler sched);

	IloInt getType() const { return ReactiveType; }
};

  
	
	  
		
		  
// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO BUILD A PROGRESSIVE AND REACTIVE SCHEDULER THAT
// SETS ACTIVITY START TIMES. IT IS USED TO SIMULATE
// SCHEDULE EXECUTION IN A NON-DETERMINISTIC ENVIRONMENT.
// THE PARAMETER deltaTProgressMin SETS THE MIN
// HORIZON UNDER WHICH WE TRY TO SELECT A SUBSET OF PENDING ACTIVITIES.
// THE PARAMETER sigmaTProgressMin SETS THE STANDARD DEVIATION OF THE
// ALLOCATED ACTIVITY UNDER WHICH WE TRY TO SELECT A SUBSET OF PENDING
// ACTIVITIES. AN INSTANCE OF THIS CLASS FINDS A FIRST SOLUTION.
// THE PARAMETER sensitivity SETS THE RE-SCHEDULING THRESHOLD threshold.
// --------------------------------------------------------------------------			


class IluProgressiveReactiveSchedulerI: public IluDynamicSchedulerI {
public:
	class ItemI {
	private:
		IloInt	_index;
		ItemI*	_next;
	public:
		ItemI() {}
		ItemI(IloInt index): _index(index) {}
		IloInt getIndex() const { return _index; }
		void setIndex(IloInt index) { _index = index; }
		ItemI* getNext() const { return _next; }
		void setNext(ItemI* next) { _next = next; }
	};
private:
	IloSchedulerSolution		_solution;		// _solution represents a partial solution of a submodel of the global model of the complete problem.
	IloInt						_deltaTProgressMin, _deltaTProgressMax;
	IloNum						_sigmaTProgressMin, _sigmaTProgressMax;
	// _deltaTProgressMin and _sigmaTProgressMin permit us to determine when we try to
	// select a subset of pending activities (the selection process starts).
	// _deltaTProgressMax and _sigmaTProgressMin permit us to determine when we stop
	// the selection process. There might possibly be several selection processes during a schedule execution.
	// _maxPriorityActIndex gives the index of the eligible activity that has the biggest priority currently.
	IluSimulatorOneActivityI*	_selecSim;	// _selecSim permits us to simulate a selected activity.
	IluFrontierI*				_currentFrontier, * _allocationFrontier, * _selectionFrontier;
	// _currentFrontier is the frontier at the current time
	// _allocationFrontier is the frontier at the end of the last allocated activity
	// _selectionFrontier is the frontier at the end of the last selected activity
	IluLocalSearchSolverI*		_localSSolver;
	IluHistogramI*				_earliestEndTimeResource, * _earliestBreakdown, * _currentEndTimeResource, * _currentBreakdown;
	// _earliestEndTimeResource, _earliestBreakdown, _currentEndTimeResource, and _currentBreakdown permit us to allocate a selected activity.
	IloNum*						_allocCostInvSum, * _energyResources, * _criticities, * _extendedMDur, * _endPExp, * _tardiExp, * _weights;
	// _priorities is an array whose size equals the number of process plans.
	// _allocCostInvSum is an array of size nbTotalActivities + 1.
	// _energyResources is an array of size nbTotalResources.
	// _criticities is an array of size nbTotalActivities + 1.
	// _extendedMDur is an array of size nbTotalActivities + 1.
	// _endPExp is an array of size nbProcessPlans.
	// _tardiExp is an array of size nbProcessPlans.
	// _weights is an array of size nbProcessPlans.
	IloNum**					_energyActivities;		// _energyActivities is a matrix of size (nbTotalActivities + 1)*(nbResources).
	IloNum*						_pendingDur, * _meanPendingDur;
	// _pendingDur is an array of size nbProcessPlans;
	// _meanPendingDur is an array of size nbProcessPlans;
	ItemI*						_recycling, * _eligible, * _pending, * _executed, * _executing, * _allocated, * _selected, * _virtual, * _lastSelectedActivity;
	// _recycling is a list that contains some ItemI objects recycled from _eligible.
	// _eligible is a list that contains indices of the currently elibible activities.
	// _pending is a list that contains indices of the currently pending activities.
	// _executed is a list that contains indices of the currently executed activities.
	// _executing is a list that contains indices of the currently executing activities.
	// _allocated is a list that contains indices of all the allocated activities (possibly selected during several selection processes).
	// _selected is a list that contains indices of the activities that were selected during the last selection process
	// (each of these activities is associated with a resource).
	// _virtual  is a list that contains indices of the currently virtual activities (a virtual activity can be used to compute the makespan).
	// _lastSelectedActivity contains the index of the last activity that has been selected.
	IluPriorityHeap				_priorityHeap;
	IluPriorityHeap::Node**		_nodes;			// _nodes is an array of size nbTotalActivities + 1 and contains the nodes corresponding to eligible activities.

	IluTreeSearchSolverI*	_treeSSolver;
	IlcScheduler			_sched;							// _sched is a complete solution found by using a tree search.
	IluHistogramI*			_histP;							// _hist stores realizations of the makespan random variable.
	IluNormalVariableI*		_normVarP;						// _normVarP is associated to the makespan.
	IloNum					_threshold, _sensitivity, _processingTimeMean;	// _threshold = mean makespan of the indicative schedule
															// _sensitivity is the sensitivity factor.
															// _processingTimeMean is the mean of all the processing times of
															// the activities of the deterministic problem.
	IlcInt					_nbActivities;					// _nbActivities is the number of activities that have been executing
															// since the last rescheduling occurred and are going to execute.
	IloInt					_criterion;						// criterion number,	0: no rescheduling occurs
															//						1: makespan monitoring
															//						2: absolute makespan monitoring
															//						3: monitoring of the estimated activity start times
															//						4: monitoring of the effective activity start times
															//						5: monitoring of the estimated activity start times with standard deviations


public:
	IluProgressiveReactiveSchedulerI(): IluDynamicSchedulerI(), _histP(0), _normVarP(0) {}
	IluProgressiveReactiveSchedulerI(IloEnv env, IlcScheduler sched, IloRandom randGen, IloNum sensitivity, IloInt criterion);
	
	~IluProgressiveReactiveSchedulerI() {}

	void initPG(IloEnv, IlcScheduler, IloNum);

	IloNum getThreshold() const { return _threshold; }

	IluHistogramI* getHist() { return _histP; }

	IlcScheduler getSched() { return _sched; }

	IlcBool reschedule(IlcScheduler sched);

	void updatePG(IloEnv env, IloNum currentTime);		// Initialization of _allocSim by retrieving precedence constraints of _solution
														// and some activities of the model. This member funciton is called each time the selection process
														// is run.

	void updateAllocFrontier();	// The member function updateAllocFrontiers() updates the allocation frontier after having selected
									// a new subset of pending activities and optimized the solution to this subproblem.

	IloBool timeForSelection(IloNum currentTime);

	void updateCurFrontier();		// The member function updateCurFrontier() updates the current frontier with respect to executed activities.

	IloInt select(IloInt currentTime);	// A heuristic is used to compute priorities, associate them with the eligible activities, and return
										// the index of the eligible activity with the highest priority or -1 if no activity is selected.

	void computeEligiblePriorities(IloNum currentTime);

	IluSimulatorOneActivityI* getSimulatorOneActivity() const { return _selecSim; }

	void updateSolution();				// The member function updateSolution() updates the scheduler solution _solution after having selected an activity.

	void initializeSolution() { if(0 != _localSSolver) _solution = IloSchedulerSolution(_localSSolver->getEnv());
								else printf("The local search solver has not yet been created!\n"); }

	IloBool eligibleActivities() { if(_eligible != 0) return IloTrue; else return IloFalse; }

	IluFrontierI* getAllocationFrontier() const { return _allocationFrontier; }
	IluFrontierI* getSelectionFrontier() const { return _selectionFrontier; }

	IloSchedulerSolution getSolution() const { return _solution; }

	IluProgressiveReactiveSchedulerI::ItemI* getItem(IloInt index, ItemI* itemList);

	IloInt getType() const { return ProgressiveReactiveType; }
};


  
	
	  
*/








#endif
