// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the IluFrontierI class, the IluStackI class, the IluSimulatorI class, and the IluSimulatorOneActivityI class.

#ifndef __SIM_ilusimulatorsimH
#define __SIM_ilusimulatorsimH

#include <ilsim/ilurandom.h>
#include <ilsim/iluactivity.h>
#include <ilsim/iluunaryresource.h>



class IluFrontierI {
	private:
		IluHistogramI** _lastHistogramBreakdown, ** _lastHistogramProcessPlan, ** _lastHistogramResource;
		IloInt _nbProcessPlans, _nbTotalResources, _nbSimulations;

	public:
		IluFrontierI(): _lastHistogramBreakdown(0),
						_lastHistogramProcessPlan(0),
						_lastHistogramResource(0),
						_nbProcessPlans(0),
						_nbTotalResources(0),
						_nbSimulations(0){}
		IluFrontierI(IloInt nbProcessPlans,
					 IloInt nbTotalResources,
					 IluHistogramI** lastHistogramBreakdown,
					 IluHistogramI** lastHistogramProcessPlan,
					 IluHistogramI** lastHistogramResource);
		IluFrontierI(IloEnv env,
					 IloInt nbProcessPlans,
					 IloInt nbTotalResources,
					 IloInt nbSimulations);

		~IluFrontierI() {}

		void setLastHistogramBreakdown(IluHistogramI** lastHistogramBreakdown) { _lastHistogramBreakdown = lastHistogramBreakdown; }
		IluHistogramI** getLastHistogramBreakdown() const { return _lastHistogramBreakdown; }
		void setLastBreakdownEndTime(IloInt resourceId, IloNum realization, IloInt seed, IloInt index) { _lastHistogramBreakdown[resourceId]->setValue(realization, seed, index); }

		void setLastHistogramProcessPlan(IluHistogramI** lastHistogramProcessPlan) { _lastHistogramProcessPlan = lastHistogramProcessPlan; }
		IluHistogramI** getLastHistogramProcessPlan() const { return _lastHistogramProcessPlan; }
		void setLastProcessPlanEndTime(IloInt processPlanId, IloNum realization, IloInt seed, IloInt index) { _lastHistogramProcessPlan[processPlanId]->setValue(realization, seed, index); }

		void setLastHistogramResource(IluHistogramI** lastHistogramResource) { _lastHistogramResource = lastHistogramResource; }
		IluHistogramI** getLastHistogramResource() const { return _lastHistogramResource; }
		void setLastResourceEndTime(IloInt resourceId, IloNum realization, IloInt seed, IloInt index) { _lastHistogramResource[resourceId]->setValue(realization, seed, index); }

		void setNbProcessPlans(IloInt nb) { _nbProcessPlans = nb; }
		IloInt getNbProcessPlans() const { return _nbProcessPlans; }

		void setNbTotalResources(IloInt nb) { _nbTotalResources = nb; }
		IloInt getNbTotalResources() const { return _nbTotalResources; }

		IloInt getNbSimulations() const { return _nbSimulations; }

		IloNum getEarliestProcessPlanEndTime(IloNum currentTime);
		IloNum getMaxStandardDeviationProcessPlan(IloNum currentTime);

		void display(const char* name, IloInt nbPoints = 1000);
};



// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO CREATE LISTS OF ITEMS.
// --------------------------------------------------------------------------
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




// --------------------------------------------------------------------------
// THIS   CLASS ALLOWS US TO BUILD A   PRECEDENCE GRAPH ON ACTIVITIES WITH
// RANDOM   PROCESSING TIMES AND TO   USE   THIS GRAPH   TO COMPUTE     THE
// PROBABILITY DISTRIBUTIONS OF THE END TIMES OF ALL ACTIVITIES    USING
// SIMULATION TECHNIQUES. ONE STEP OF SIMULATION IS PERFORMED IN O(n+m+k).
// n IS THE NUMBER OF NODES, m IS THE NUMBER OF ARCS, AND k IS THE NUMBER OF
// BREAKDOWNS.
// EACH NODE OF THE DIGRAPH IS ASSOCIATED WITH AN ACTIVITY DURATION
// VARIABLE, AN INTERBREAKDOWN VARIABLE, AND A BREAKDOWN DURATION VARIABLE.
// --------------------------------------------------------------------------
class IluSimulatorI {
 public:
	class ResourceI{
	private:
		IloInt				_id;						// _Id is the index of the corresponding resource.
		IluRandomVariableI*	_interBreakdownDuration, * _breakdownDuration;
		// _interBreakdownDuration is a random variable describing the time between two consecutive breakdowns.
		// _breakdownDuration is a random variable describing the breakdown duration.
		IloNum				_currentStart, _currentEnd;	// _currentStart is the current breakdown start time.
														// _currentEnd is the current breakdown end time.
		IloInt				_seed;						// _seed is a seed that will be used to generate the next breakdown.
		IloNum				_lastEnd;					// _lastEnd is the end time of the last breakdown.
		IloInt				_nodeId;					// _nodeId is the index of the node that is sorted at the last position on this resource.
		IloNum				_start, _end;				// _start represents an effective resource breakdown start time.
														// _end represents an effective resource breakdown end time.
		IloBool				_brokenDown;				// _brokenDown is equal to IloTrue when the resource is broken down. It is equal to IloFalse when the resource
														// is available.
		IloInt				_execActId;					// _execActId is the index of the activity that is currently executing and allocated to this resource.
	public:
		ResourceI(IloInt id)
			:_start						(IloInfinity),
			 _end						(IloInfinity),
			 _id						(id),
			 _interBreakdownDuration	(0),
			 _breakdownDuration			(0),
			 _currentStart				(IloInfinity),
			 _currentEnd				(IloInfinity),
			 _seed						(0),
			 _lastEnd					(IloInfinity),
			 _nodeId					(IloIntMax),
			 _brokenDown				(IloFalse),
			 _execActId					(IloIntMax) {}
		ResourceI(IloInt id, IloNum currentStart, IloNum currentEnd, IloInt seed, IloNum lastEnd)
			:_start						(IloInfinity),
			 _end						(IloInfinity),
			 _id						(id),
			 _interBreakdownDuration	(0),
			 _breakdownDuration			(0),
			 _currentStart				(currentStart),
			 _currentEnd				(currentEnd),
			 _seed						(seed),
			 _lastEnd					(lastEnd),
			 _nodeId					(IloIntMax),
			 _brokenDown				(IloFalse),
			 _execActId					(IloIntMax) {}
		~ResourceI() {}
		
		void setInterBreakdownDurationVariable(IluRandomVariableI* dur) { _interBreakdownDuration = dur; }
		void setBreakdownDurationVariable(IluRandomVariableI* dur) { _breakdownDuration = dur; }

		IluRandomVariableI* getInterBreakdownDurationVariable() const { return _interBreakdownDuration; }
		IluRandomVariableI* getBreakdownDurationVariable() const { return _breakdownDuration; }

		IloInt getId() const { return _id; }
		void setId(IloInt id) { _id = id; }

		IloNum getCurrentEnd() const { return _currentEnd; }
		IloNum getCurrentStart() const { return _currentStart; }
		IloNum getLastEnd() const { return _lastEnd; }
		IloInt getSeed() const { return _seed; }
		IloInt getNodeId() const { return _nodeId; }
		IloInt getExecActId() const { return _execActId; }
		
		IloNum getStartTime() const { return _start; }
		IloNum getEndTime() const {	return _end; }

		void setCurrentEnd(IloNum currentEnd) { _currentEnd = currentEnd; }
		void setCurrentStart(IloNum currentStart) { _currentStart = currentStart; }
		void setLastEnd(IloNum lastEnd) { _lastEnd = lastEnd; }
		void setSeed(IloInt seed) { _seed = seed; }
		void setNodeId(IloInt nodeId) { _nodeId = nodeId; }
		void setExecActId(IloInt actId) { _execActId = actId; }

		void setStartTime(IloNum t) { _start = t; _brokenDown = IloTrue; }
		void setEndTime(IloNum t) { _end = t; _brokenDown = IloFalse; }

		IloBool isBrokenDown() const { return _brokenDown; }
	};

	class NodeI;
	class ArcI;

	class NodeI {
	private:
		IloNum              _start;				// _start represents the effective activity start time.
		IloNum              _end;				// _end represents the effective activity end time.
		IloNum              _val;				// _val is minimum activity start time and can be stored in a histogram.
		IloNum				_executedPart;		// _executedPart is equal to the executed activity duration so far. It is updated each time simualtions are run.
		IluRandomVariableI* _dur;				// _dur represents the random variable associated with the activity duration.
		IluHistogramI*      _hist;				// _hist is used to incrementally store activity start times.
		ArcI*               _out;				// _out is a list of arcs that go out from the node.
		IloInt              _indeg;				// _indeg is equal to the number of arcs going in the node.
		IloInt              _curIndeg;			// _curIndeg represents the number of arcs going in the node and whose source nodes have not been simulated.
		IloInt				_topoIndeg;			// _topoIndeg is used to do the topological sort.
		ResourceI*			_res;				// _res is an object describing the resource associated with this node.
		IloBool				_frontier;			// _frontier indicates if the node belongs to the start frontier or not. It equals IloTrue if it belongs to it.
		IloInt				_processPlanId;		// _processPlanId is the index of the process plan to which this node belongs.
		IloInt				_activityId;		// _activityId is the index of the activity corresponding to this node.
		IloBool				_lastProcessPlan;	// _lastProcessPlan indicates whether the node is ranked last on a process plan or not. It equals IloTrue if it is ranked last on a process plan.
		IloBool				_lastResource;		// _lastResource indicates whether the node is ranked last on a resource or not. It equals IloTrue if it is ranked last on a resource.
		NodeI*				_next;				// _next points to the next node; this order is done arbitrarily.
		NodeI*				_topologicalNext;	// _topologicalNext points to the next node  with respect to the topological sort.
	public:
		NodeI(IloBool frontier)
			:_start				(IloInfinity),
			_end				(IloInfinity),
			_val				(0.0),
			_executedPart		(0.0),
			_dur				(0),
			_hist				(0),
			_out				(0),
			_res				(0),
			_indeg				(0),
			_topoIndeg			(0),
			_curIndeg			(0),
			_frontier			(frontier),
			_processPlanId		(IloIntMax),
			_activityId			(IloIntMax),
			_lastProcessPlan	(IloFalse),
			_lastResource		(IloFalse),
			_next				(0),
			_topologicalNext	(0)	{}

		~NodeI();
		IloNum getStartTime() const { return _start; }
		IloNum getEndTime() const {	return _end; }
		IloNum getVal() const {	return _val; }
		IloNum getExecutedPart() const { return _executedPart; }

		IluRandomVariableI* getDurationVariable() const { return _dur; }
		IloNum getDuration(IloNum t =0) const;
		IluHistogramI* getHistogram() const { return _hist; }

		void setVal(IloNum val) { _val = val; }
		void cleanVal(IloNum t) { if(t < _start) _val = t; }
		void setMinVal(IloNum val) { if(val > _val) _val = val; }
		void setStartTime(IloNum start) { _val = start; _start = start; }
		void setEndTime(IloNum end) { _end = end; }
		void setExecutedPart(IloNum executedPart) { _executedPart = executedPart; }

		void setDurationVariable(IluRandomVariableI* dur) { _dur = dur; }
		IloNum computeEnd(IloNum t);
		void setHistogram(IluHistogramI* hist) { _hist = hist; }
		void addArc(NodeI* target) { _out = new ArcI(target, _out); }
		void addArc(ArcI* arc) { arc->setNext(_out); _out = arc; }
		void setArc(ArcI* arc) { _out = arc; }

		void setIndegree(IloInt i) { _indeg = i; }
		IloInt getIndegree() const { return _indeg; }
		void incrIndegree() { _indeg++; }
		void decrIndegree() { _indeg--; }
		
		IloInt getTopoIndegree() const { return _topoIndeg; }

		void setCurrentIndegree(IloInt i) {	_curIndeg = i; }
		IloInt getCurrentIndegree() const { return _curIndeg; }
		void incrCurrentIndegree() { _curIndeg++; }
		void decrCurrentIndegree() { _curIndeg--; }
		
		void setTopoIndegree(IloInt i) { _topoIndeg = i; }
		void decrTopoIndegree() { _topoIndeg--; }

		ArcI* getArcs() const { return _out; }
		void removeArcs() { _out = 0; _indeg = 0; _curIndeg = 0; }

		void setResource(ResourceI* res) { _res = res; }
		ResourceI* getResource() const { return _res; }

		void setFrontier(IloBool frontier) { _frontier = frontier; }
		IloBool isFrontier() const { return _frontier; }

		void setProcessPlanId(IloInt index) { _processPlanId = index; }
		IloInt getProcessPlanId() const { return _processPlanId; }

		void setActivityId(IloInt index) { _activityId = index; }
		IloInt getActivityId() const { return _activityId; }

		void setLastProcessPlan(IloBool lastProcessPlan) { _lastProcessPlan = lastProcessPlan; }
		IloBool isLastProcessPlan() const {return _lastProcessPlan; }

		void setLastResource(IloBool lastResource) { _lastResource = lastResource; }
		IloBool isLastResource() const { return _lastResource; }

		void setNext(NodeI* node) { _next = node; }
		NodeI* getNext() const { return _next; }

		void setTopologicalNext(NodeI* node) { _topologicalNext = node;}
		NodeI* getTopologicalNext() const { return _topologicalNext; }
	};
	class ArcI {
	private:
		NodeI* _target;		// _target is a pointer to the target node.
		ArcI*  _next;
	public:
		ArcI(NodeI* target, ArcI* next)
			:_target (target),
			 _next   (next) {}
		~ArcI() {}
		NodeI* getTarget() const { return _target; }
		void setTarget(NodeI* target) { _target = target; }
		ArcI* getNext() const {	return _next; }
		void setNext(ArcI* next) { _next = next; }
	};

 private:	
	IloEnv					_env;
	IloInt					_nbNodes, _nbResources, _nbProcessPlans, _maxNbSimulations;
	NodeI*					_firstNode, * _firstRecyclingNode;					// _firstNode is the first node of a list of nodes and
																				// _firstRecyclingNode is the first node of a recycling list of nodes.
	ArcI*					_firstRecyclingArc;									// _firstRecyclingArc is the first arc of a recycling list of arcs.
	ResourceI**				_resources;
	IluHistogramI*			_firstRecyclingHistogram;							// _firstRecyclingHistogram is the first histogram of a recycling list of histograms.
	NodeI*					_work, * _sortFirst, * _sortLast;					// _work is a list of nodes that have the same indegree.
	IluFrontierI*			_startFrontier, * _endFrontier;
	// _startFrontier is an object containing a set of distributions (end times of resource breakdowns and end times of executed activities) before simulating.
	// _endFrontier is an object containing a set of distributions (end times of resource breakdowns and end times of allocated activities) after simulating.
	// _selectionFrontier is an object containing a set of distributions (end times of resource breakdowns and end times of selected activities) after simulating.
	IloInt*					_lastNodeProcessPlan;	// _lastNodeProcessPlan is an array containing the indexes of the nodes sorted at the last positions on the process plans.
	IloNum					_lastSimulationTime;	// _lastSimulationTime is equal to the last time the member function simulate() was called.
 public:
	IluSimulatorI()
		:_nbNodes					(0),
		 _nbResources				(0),
		 _nbProcessPlans			(0),
		 _maxNbSimulations			(0),
		 _firstNode					(0),
		 _firstRecyclingNode		(0),
		 _firstRecyclingArc			(0),
		 _resources					(0),
		 _firstRecyclingHistogram	(0),
		 _work						(0),
		 _sortFirst					(0),
		 _sortLast					(0),
		 _startFrontier				(0),
		 _endFrontier				(0),
		 _lastNodeProcessPlan		(0),
		 _lastSimulationTime		(IloInfinity) {}

	IluSimulatorI(IloEnv env, IluFrontierI* startFrontier, IluFrontierI* endFrontier, IloInt nbNodes);
	~IluSimulatorI();

	// DEFINITION OF THE GRAPH
	void addArc(IloInt source, IloInt target);
	void setActivityDurationVariable(IloInt i, IluRandomVariableI* dur);
	void setBreakdownDurationVariable(IloInt i, IluRandomVariableI* dur) { _resources[i]->setBreakdownDurationVariable(dur); }
	void setInterBreakdownDurationVariable(IloInt i, IluRandomVariableI* interDur) { _resources[i]->setInterBreakdownDurationVariable(interDur); }
	void setHistogram(IloInt i, IluHistogramI* hist);
	void setProcessPlanId(IloInt i, IloInt processPlanIndex);
	void readJobShop(IluRandomEnv activityRand, const char* filename, IloNum alpha);		

	// TO BE CALLED ONE TIME ONCE THE GRAPH HAS BEEN DEFINED 
	void topologicalSort();

	// TO BE CALLED ONE TIME ONCE THE GRAPH HAS BEEN TOPOLOGICALLY SORTED (GIVEN A JOB-SHOP PROBLEM)
	void addFrontierArcs();

	// TO UPDATE THE GRAPH DURING SIMULATION
	void setStartTime(IloInt i, IloNum t);
	void setEndTime(IloInt i, IloNum t);
	void setBreakdownStartTime(IloInt i, IloNum t) { _resources[i]->setStartTime(t); }	
	void setBreakdownEndTime(IloInt i, IloNum t);

	// CHECKS WHETHER AN ACTIVITY CAN BE EXECUTED
	IloBool isExecutable(IloInt i);

	// CHECKS WHETHER AN ARC EXISTS
	IloBool hasArc(IloInt source, IloInt target);

	// CHECHS WHETHER A NODE EXISTS
	IloBool nodeExists(IloInt nodeId);

	// CHECKS WHETHER AN ARC CAN BE ADDED
	IloBool arcCanBeAdded(IloInt source, IloInt target);
	
	// ONE STEP OF SIMUTATION
	void simulate(IloNum t = 0.0, IloInt nb = 1000);
	
	// OTHER POTENTIALLY USEFUL FUNCTIONS
	IloEnv getEnv() const { return _env; }
	IloNum getStartTime(IloInt i) const;
	IloNum getVal(IloInt i) const;
	IloNum getEndTime(IloInt i) const;
	IluRandomVariableI* getDurationVariable(IloInt i) const;
	IloNum getDuration(IloInt i, IloNum t) const;
	IloNum getDuration(IloInt i, IloInt d);
	IluHistogramI* getHistogram(IloInt i) const;
	NodeI* getFirstTopologicalSort() const { return _sortFirst; }
	NodeI* getLastTopologicalSort() const {	return _sortLast; }
	IloInt getNbResources() const { return _nbResources; }
	IloInt getNbProcessPlans() const { return _nbProcessPlans; }
	IloInt getNbSimulations() const { return _maxNbSimulations; }
	void decrCurrentIndegree(IloInt nodeId);
	IloInt getCurrentIndegree(IloInt nodeId);

	void requires(IloInt i, IloInt resIndex);

	void graphDisplay();			// This member function permits us to diplay all the nodes, their precedence relations, and their characteristics.

	IluFrontierI* getStartFrontier() const { return _startFrontier; }
	IluFrontierI* getEndFrontier() const { return _endFrontier; }

	NodeI* getFirstNode() const { return _firstNode; }
	NodeI* getFirstRecyclingNode() const { return _firstRecyclingNode; }

	ArcI* getFirstRecyclingArc() const { return _firstRecyclingArc; }

	void setFirstRecyclingHistogram(IluHistogramI* hist) { _firstRecyclingHistogram = hist; }
	IluHistogramI* getFirstRecyclingHistogram() const { return _firstRecyclingHistogram; }

	void addNode(IloInt index, IloBool frontier);		// A node is added to the digraph.
	void recycleNode(IloInt i);							// The node whose index is index is removed from the digraph and recycled.

	ResourceI* getResource(IloInt i) { if(_resources) return _resources[i]; else return 0; }

	void clean();

	void recycleExecutedNodes(IloNum currentTime);

 private:
	void stackWork(NodeI* node);
	void stackSort(NodeI* node);
	NodeI* unstackWork();
};




// --------------------------------------------------------------------------
// THIS CLASS ALLOWS US TO COMPUTE THE PROBABILITY DISTRIBUTION OF THE END
// TIME OF AN ACTIVITY WITH UNCERTAIN PROCESSING TIME AND ALLOCATED TO A
// RESOURCE  THAT CAN BREAK DWON. USING SIMULATION TECHNIQUES. ONE STEP OF
// SIMULATION IS PERFORMED IN O(m+k).
// m IS THE NUMBER OF ARCS, AND k IS THE NUMBER OF BREAKDOWNS.
// --------------------------------------------------------------------------
class IluSimulatorOneActivityI {
public:
	class ResourceI{
	private:
		IloInt				_id;						// _Id is the index of the corresponding resource.
		IluRandomVariableI*	_interBreakdownDuration;	// _interBreakdownDuration is a random variable describing the time between two consecutive breakdowns.
		IluRandomVariableI*	_breakdownDuration;			// _breakdownDuration is a random variable describing the breakdown duration.
		IloNum				_start;						// _start represents an effective resource breakdown start time.
		IloBool				_brokenDown;				// _brokenDown is equal to IloTrue when the resource is broken down. It is equal to IloFalse when the resource
														// is available.
	public:
		ResourceI(IloInt id)
			:_start						(IloInfinity),
			 _id						(id),
			 _interBreakdownDuration	(0),
			 _breakdownDuration			(0),
			 _brokenDown				(IloFalse)	{}
		ResourceI(IloInt id, IloNum currentStart, IloNum currentEnd, IloNum lastEnd)
			:_start						(IloInfinity),
			 _id						(id),
			 _interBreakdownDuration	(0),
			 _breakdownDuration			(0),
			 _brokenDown				(IloFalse)	{}
		~ResourceI() {}
		
		void setId(IloInt id) { _id = id; }
		void setInterBreakdownDurationVariable(IluRandomVariableI* dur) { _interBreakdownDuration = dur; }
		void setBreakdownDurationVariable(IluRandomVariableI* dur) { _breakdownDuration = dur; }
		void setStartTime(IloNum t) { _start = t; _brokenDown = IloTrue; }

		IloInt getId() const { return _id; }
		IluRandomVariableI* getInterBreakdownDurationVariable() const { return _interBreakdownDuration; }
		IluRandomVariableI* getBreakdownDurationVariable() const { return _breakdownDuration; }
		IloNum getStartTime() const { return _start; }

		IloBool isBrokenDown() const { return _brokenDown; }

		void setBroken(IloBool brokenDown) { _brokenDown = brokenDown; }
	};

private:
	IloEnv			_env;
	IloInt			_maxNbSimulations, _processPlanId, _bestResourceId;
	IluHistogramI*	_lastEndTimeBreakdown, * _lastEndTimeProcessPlan, * _bestLastEndTimeBreakdown, * _bestLastEndTimeProcessPlan;
	IloNum			_bestGlobalCost;
	// _lastEndTimeBreakdown is the histogram of the last resource breakdown end time and _lastEndTimeProcessPlan is the histogram of the last activity end time
	// obtained after calling simulateOneActivity() with an allocated activity.
	// _bestLastEndTimeBreakdown is the histogram of the last resource breakdown end time and _bestLastEndTimeProcessPlan is the histogram of the last activity
	// end time such that their average is the miminum average obtained after calling simulateOneActivity() so far.
	// _bestGlobalCost is the expected cost implied by assigning a resource to an activity. It is used to choose an activity heuristically.
	IluFrontierI*	_selectionFrontier;
	ResourceI*		_resource;		// _resource is the resource that is currently associated with _activity.
	IluActivityI*	_activity;		// _activity is the activity whose execution is currently simulated.

public:
	IluSimulatorOneActivityI()
	 : _maxNbSimulations			(0),
	   _processPlanId				(-1),
	   _bestResourceId				(-1),
	   _lastEndTimeBreakdown		(0),
	   _lastEndTimeProcessPlan		(0),
	   _bestLastEndTimeBreakdown	(0),
	   _bestLastEndTimeProcessPlan	(0),
	   _bestGlobalCost				(0.0),
	   _selectionFrontier			(0),
	   _resource					(0),
	   _activity					(0) {}

	IluSimulatorOneActivityI(IloEnv env, IluFrontierI* frontier);

	void setAllocatedActivity(IluActivityI* act, IluUnaryResourceI* res, IloInt processPlanId);

	void simulate(IloNum t, IloInt nb = 1000);		// The member function simulate simulates the execution of an activity allocated to a resource.

	void updateBest(IloNum beta, IloNum allocCost, IloInt dueDate, IloNum phi, IloNum pendingDur, IloNum extendedMDur);
							// The member function updateBest updates the histograms
							// and the resource index so that the allocation minimizes the global cost.
	void updateSelectionFrontier();					// The member function updateSelectionFrontier updates the selection frontier.

	IloInt getBestResourceId() const { return _bestResourceId; }
	void setBestResourceId(IloInt bestResourceId) { _bestResourceId = bestResourceId; }

	void setProcessPlanId(IloInt processPlanId) { _processPlanId = processPlanId; }

	IluHistogramI* getBestLastEndTimeBreakdown() const { return _bestLastEndTimeBreakdown; }
	IluHistogramI* getBestLastEndTimeProcessPlan() const { return _bestLastEndTimeProcessPlan; }

	IluFrontierI* getSelectionFrontier() const { return _selectionFrontier; }

	IloInt getNbSimulations() const { return _maxNbSimulations; }
	IloEnv getEnv() const { return _env; }

	IluActivityI* getActivity() const { return _activity; }
	void setActivity(IluActivityI* uncAct) { _activity = uncAct; }

	IloNum getBestGlobalCost() const { return _bestGlobalCost; }
	void setBestGlobalCost(IloNum bestGlobalCost) { _bestGlobalCost = bestGlobalCost; }

	IloInt getResourceId() const { return _resource->getId(); }
};



#endif