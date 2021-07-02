// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------

// The file contains the IluPrecGraphI class.

#ifndef __SIM_ilupgsimsimH
#define __SIM_ilupgsimsimH

#include <ilsim/ilurandom.h>

// --------------------------------------------------------------------------
// THIS   CLASS ALLOWS US TO BUILD A   PRECEDENCE GRAPH ON ACTIVITIES WITH
// RANDOM   PROCESSING TIME AND TO   USE   THIS GRAPH   TO COMPUTE     THE
// PROBABILITY DISTRIBUTIONS OF THE END TIMES OF SOME ACTIVITIES    USING
// SIMULATION TECHNIQUES. ONE STEP OF SIMULATION IS PERFORMED IN O(n+m).
// --------------------------------------------------------------------------
class IluPrecGraphI {
 public:
	class ArcI {
	private:
		IloInt _target;		// _target is an index.
		ArcI*  _next;
	public:
		ArcI(IloInt target, ArcI* next)
			:_target (target),
			_next   (next){}
		~ArcI(){}
		IloInt getTarget() const {
			return _target; }
		ArcI* getNext() const {
			return _next; }
	};
	class NodeI {
	private:
		IloInt              _id;		// _id is the index.
		IloNum              _start;		// _start is the effective start time.
		IloNum              _end;		// _end is the effective end time.
		IloNum              _val;		// _val is the minimum start time and can be stored in a histogram.
		IluRandomVariableI* _dur;
		IluHistogramI*      _hist;
		ArcI*               _out;
		IloInt              _indeg;
		IloInt              _curindeg;
	public:
		NodeI(IloInt);
		~NodeI();
		IloNum getVal() const {
			return _val; }
		IloNum getStartTime() const {
			return _start; }
		IloNum getEndTime() const {
			return _end; }
		IluRandomVariableI* getDurationVariable() const {
			return _dur; }
		IloNum getDuration(IloNum t =0) const;
		IluHistogramI* getHistogram() const {
			return _hist; }
		void setVal(IloNum val) {
			_val = val; }
		void cleanVal(IloNum t) {
			if (t < _start)	_val = t;	}
		void setMinVal(IloNum val) {
			if (val > _val) _val = val; }
		void setStartTime(IloNum start) {
			_val   = start;
			_start = start;}
		void setEndTime(IloNum end) {
			_end = end; }
		void setDurationVariable(IluRandomVariableI* dur) {
			_dur = dur; }
		IloNum computeEnd(IloNum t) {
			return _val + getDuration(t); }
		void setHistogram(IluHistogramI* hist) {
			_hist = hist; }
		void addArc(IloInt target) {
			_out = new ArcI(target, _out); }
		IloInt getInDegree() const {
			return _indeg; }
		IloInt getCurrentInDegree() const {
			return _curindeg; }
		void setCurrentInDegree(IloInt i) {
			_curindeg = i; }
		void decrCurrentInDegree() {
			_curindeg--; }
		void incrInDegree() {
			_indeg++; }
		IloInt getId() const {
			return _id; }
		ArcI* getArcs() const {
			return _out; }
	};
	class ItemI {
	private:
		IloInt _id;
		ItemI* _next;
	public:
		ItemI(ItemI* next, IloInt id)
			:_id(id), _next(next){}
		ItemI()
			:_id(0), _next(0){}
		~ItemI(){};
		IloInt getId() const {
			return _id; }
		ItemI* getNext() const {
			return _next; }
		void setNext(ItemI* next) {
			_next = next; }
		void init(ItemI* next, IloInt id) {
			_next = next; _id = id;
		}
	};
	
	IloInt  _nbNodes;
	NodeI** _nodes;
	ItemI*  _work;
	ItemI*  _sortfirst;
	ItemI*  _sortlast;
	
 public:
	IluPrecGraphI();
	IluPrecGraphI(IloInt nbNodes);
	~IluPrecGraphI();

	// DEFINITION OF THE GRAPH 
	void addArc(IloInt source, IloInt target) {
		_nodes[target]->incrInDegree();
		_nodes[source]->addArc(target);	}
	void setDurationVariable(IloInt i, IluRandomVariableI* dur) {
		_nodes[i]->setDurationVariable(dur);	}
	void setHistogram(IloInt i, IluHistogramI* hist) {
		_nodes[i]->setHistogram(hist);	}
	void readJobShop(IluRandomEnv rand, const char*, IloNum alpha);

	// TO BE CALLED ONE TIME ONCE THE GRAPH HAS BEEN DEFINED 
	void topologicalSort();

	// TO UPDATE THE GRAPH DURING SIMULATION
	void setStartTime(IloInt i, IloNum t) {
		_nodes[i]->setStartTime(t);
		_nodes[i]->setCurrentInDegree(-1); } // MARK EXECUTED
	void setEndTime(IloInt i, IloNum t);

	// CHECKS WHETHER AN ACTIVITY CAN BE EXECUTED
	IloBool isExecutable(IloInt i) const {
		return (0 == _nodes[i]->getCurrentInDegree()); }
	
	// ONE STEP OF SIMUTATION
	void simulate(IloNum t = 0.0, IloInt nb = 1000);
	
	// OTHER POTENTIALLY USEFUL FUNCTIONS
	IloNum getStartTime(IloInt i) const {
		return _nodes[i]->getStartTime(); }
	IloNum getVal(IloInt i) const {
		return _nodes[i]->getVal(); }
	IloNum getEndTime(IloInt i) const {
		return _nodes[i]->getEndTime(); }
	IluRandomVariableI* getDurationVariable(IloInt i) const {
		return _nodes[i]->getDurationVariable(); }
	IloNum getDuration(IloInt i, IloNum t=0) const {
		return _nodes[i]->getDuration(t); }
	IloNum getDuration(IloInt i, IloInt d) const {
		return _nodes[i]->getDuration(d); }
	IluHistogramI* getHistogram(IloInt i) const {
		return _nodes[i]->getHistogram(); }
	ItemI* getTopologicalSort() const {
		return _sortfirst; }

 private:
	void stackWork(IloInt i) {
		_work = new ItemI(_work, i); }
	void stackSort(ItemI* it);
	ItemI* unstackWork();
};
#endif
