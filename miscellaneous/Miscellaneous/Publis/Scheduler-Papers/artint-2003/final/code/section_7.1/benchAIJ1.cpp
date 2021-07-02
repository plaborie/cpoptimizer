// -------------------------------------------------------------- -*- C++ -*- 
// File: benchAIJ1.cpp
// --------------------------------------------------------------------------
// Copyright (C) 1999-2001 by ILOG.
// All Rights Reserved.
//
// This code implements the search procedure  described in section 7.1
// of  the paper "Algorithms for   Propagating Resource Constraints in
// A.I.   Planning   and   Scheduling: Existing  Approaches   and  New
// Results",  Artificial  Intelligence Journal.   It   runs with  ILOG
// Concert 1.1/Solver 5.1/Scheduler 5.1 and later releases.
//
// --------------------------------------------------------------------------

#include <ilsched/iloscheduler.h>
// #ifndef WIN32
// #include <strstream.h>
// #else
// #include <strstrea.h>
// #endif

ILOSTLBEGIN

// --------------------------------------------------------------------------
// SECTION 1. FILE READER
// --------------------------------------------------------------------------

IloModel ReadInstance(const char* filename,
		      IloEnv env,
		      IloInt horizon,
		      IloInt& numberOfJobs,
		      IloInt& numberOfResources,
		      IloActivity*&  activities,
		      IloReservoir*& reservoirs) {
  IloModel model(env);
  IloSchedulerEnv schedEnv(env);
  schedEnv.setOrigin(0);
  schedEnv.setHorizon(horizon+1);
  /* Use balance constraint on each resource. */
  schedEnv.getResourceParam().setCapacityEnforcement(IloExtended); 

  IloInt i,j;
  int lineLimit = 1024;
  char buffer[1024];
  char name[128];

  /* Create activities. */
  ifstream file(filename);
  file.getline(buffer, lineLimit);
  istrstream line(buffer, strlen(buffer));
  line >> numberOfJobs;
  line >> numberOfResources;
  activities = new IloActivity[numberOfJobs+2];
  IloNumVar eVar(env, horizon+1, horizon+1, IloNumVar::Int);
  
  IloNumVar ptVar0(env, 1, horizon+1, IloNumVar::Int);
  activities[0] = IloActivity(env, ptVar0);
  model.add(activities[0].startsAt(0));
  
  for (i = 0; i < numberOfJobs+2; i++) {
    sprintf(name, "A%d", i);
    IloNumVar sVar(env, 0, horizon, IloNumVar::Int);
    IloNumVar ptVar(env, 1, horizon+1, IloNumVar::Int);
    activities[i] = IloActivity(env, ptVar);
    activities[i].shareEndWithEnd(activities[0]);
    activities[i].setName(name);
    activities[i].setObject((IloAny)i); // Index in numeric graph
  }

  /* Read temporal constraints. */
  IloInt n, d, q, dummy, nbSucc, delay;
  IloInt* succs = new IloInt[numberOfJobs+2];
  for (i = 0; i < numberOfJobs+1; i++) {
    file.getline(buffer, lineLimit);
    istrstream line(buffer, strlen(buffer));
    line >> n >> dummy >> nbSucc;
    for (j=0; j < nbSucc; ++j)
      line >> succs[j];
    for (j=0; j < nbSucc; ++j) {
      line >> delay;
      model.add(activities[succs[j]].startsAfterStart(activities[i], delay));
    }
  }
  delete[] succs;

  file.getline(buffer, lineLimit);
  file.getline(buffer, lineLimit);

  /* Create reservoirs. */
  reservoirs = new IloReservoir[numberOfResources];
  for (j = 0; j < numberOfResources; j++) {
    sprintf(name, "R%d", j+1);
    reservoirs[j] = IloReservoir(env);
    reservoirs[j].setName(name);
    if (j > 0)
      reservoirs[j].setObject((IloAny)j);
  }

  /* Read activity duration and reservoir usage. */
  for (i = 1; i <= numberOfJobs; i++) {
    file.getline(buffer, lineLimit);
    istrstream line(buffer, strlen(buffer));
    line >> dummy >> d;
    for (j = 0; j < numberOfResources; j++) {
      line >> q;
      if (q != 0) {
	if (0 < q) {
	  model.add(activities[i].provides(reservoirs[j],q, IloAfterStart));
	} else {
	  model.add(activities[i].consumes(reservoirs[j],-q));
	}
      }
    }
  }

  file.getline(buffer, lineLimit);

  /* Read reservoir capacities and initial levels. */
  IloInt* levelMin = new IloInt[numberOfResources];
  file.getline(buffer, lineLimit);
  istrstream lineMin(buffer, strlen(buffer));
  for (j = 0; j < numberOfResources; j++) {
    lineMin >> levelMin[j];
  }
  IloInt* levelMax = new IloInt[numberOfResources];
  file.getline(buffer, lineLimit);
  istrstream lineMax(buffer, strlen(buffer));
  for (j = 0; j < numberOfResources; j++) {
    lineMax >> levelMax[j];
  }
  for (j = 0; j < numberOfResources; j++) {
    IloInt cap = levelMax[j] - levelMin[j];
    IloInt initlevel = - levelMin[j];
    reservoirs[j].setCapacity(cap);

    if (levelMin[j] < 0) {
      model.add(activities[0].provides(reservoirs[j], -levelMin[j], IloAfterStart));
    } else if (levelMin[j] > 0) {
      model.add(activities[0].consumes(reservoirs[j], levelMin[j]));     
    }
  }  
  delete[] levelMin;
  delete[] levelMax;
  file.close();

  return model;
}

// --------------------------------------------------------------------------
// SECTION 2. DISTANCE MATRIX
// --------------------------------------------------------------------------

class NumericGraph {
private:
  IloInt    _nbNodes;
  IloInt**  _matrix;
  
public:
  NumericGraph(IloSolver, IloInt nbNodes);
  ~NumericGraph(){};

  void copy(NumericGraph*);

  IloInt getNbNodes() const {
    return _nbNodes; }

  IloInt getMin(IloInt i, IloInt j) const {
    return _matrix[i][j]; }

  IloInt getMax(IloInt i, IloInt j) const {
    return - _matrix[j][i]; }

  IloBool isBefore(IloInt i, IloInt j) const {
    return (0 <= getMin(i,j)); }
  
  IloBool isStrictlyBefore(IloInt i, IloInt j) const {
    return (0 < getMin(i,j)); }
  
  void setMin(IloInt i, IloInt j, IloInt d);
  
  void setBefore(IloInt i, IloInt j) {
    setMin(i,j,0); }
  
  void setStrictlyBefore(IloInt i, IloInt j) {
    setMin(i,j,1); }
  
  IloBool propagate();
  void synchronize(IlcScheduleEventPrecedenceGraphConstraint);
  void input(IlcScheduleEventPrecedenceGraphConstraint);
  void output(IlcScheduleEventPrecedenceGraphConstraint) const;
};

NumericGraph::NumericGraph(IloSolver solver, IloInt nbNodes)
  :_nbNodes(nbNodes) {
  _matrix = new (solver.getHeap()) IloInt*[nbNodes];
  for (IloInt i = 0; i < nbNodes; i++) {
    _matrix[i] = new (solver.getHeap()) IloInt[nbNodes];
    for (IloInt j = 0; j < nbNodes; j++)
      _matrix[i][j] = - IloIntMax/4;
    _matrix[i][i]=0;
  }
}

void
NumericGraph::copy(NumericGraph* ref){
  for (IloInt i = 0; i < _nbNodes; i++)
    for (IloInt j = 0; j < _nbNodes; j++)
      _matrix[i][j] = ref->_matrix[i][j];
}

void
NumericGraph::setMin(IloInt i, IloInt j, IloInt d) {
  IloInt& d0 = _matrix[i][j];
  if (d0 < d) 
    d0 = d;
}

IloBool 
NumericGraph::propagate() {
  IloInt k;
  for (k = 0; k < _nbNodes; k++) {
    IloInt* matrixk = _matrix[k];
    for (IloInt i = 0; i < _nbNodes; i++) {
      IloInt* matrixi = _matrix[i];
      for (IloInt j = 0; j < _nbNodes; j++) {
	IloInt path_ikj =  matrixi[k] + matrixk[j];
	IloInt& d_ij = matrixi[j];
	if (d_ij < path_ikj) 
	  d_ij = path_ikj;	
      }
    }
  }
  for (k = 0;(k < _nbNodes) && (_matrix[k][k] == 0); k++);
  return (k == _nbNodes);
}

void
NumericGraph::synchronize(IlcScheduleEventPrecedenceGraphConstraint g) {
  input(g);
  if (!propagate()) 
    g.getSolver().fail();
  output(g); 
}

void
NumericGraph::input(IlcScheduleEventPrecedenceGraphConstraint g){
  IlcScheduleEvent x, y;
  IloInt i,j;
  for (IlcScheduleEventIterator ite(g); 
       ite.ok(); 
       ++ite){
    x = *ite;
    i = (IloInt)x.getActivity().getObject();
    for (IlcScheduleEventIterator itep(g, x, IlcPredecessors); 
	 itep.ok(); 
	 ++itep) {
      y = *itep;
      j = (IloInt)y.getActivity().getObject();
      if (i != j) {
	if (g.isStrictlySucceededBy(y,x)) 
	  setStrictlyBefore(j,i);
	else
	  setBefore(j,i);
      }
    }
    for (IlcScheduleEventIterator ites(g, x, IlcSuccessors); 
	 ites.ok(); 
	 ++ites) {
      y = *ites;
      j = (IloInt)y.getActivity().getObject();
      if (i != j) {
	if (g.isStrictlySucceededBy(x,y)) 
	  setStrictlyBefore(i,j);
	else
	  setBefore(i,j);
      }
    }
  }
}

void
NumericGraph::output(IlcScheduleEventPrecedenceGraphConstraint g) const {
  IlcScheduleEvent x, y;
  IloInt i,j;
  for (IlcScheduleEventIterator itx(g); itx.ok(); ++itx){
    x = *itx;
    i = (IloInt)x.getActivity().getObject();
    for (IlcScheduleEventIterator ity(g); ity.ok(); ++ity){
      y = *ity;
      j = (IloInt)y.getActivity().getObject();
      if (i < j) {
	if (isBefore(i,j)) {
	  if (isStrictlyBefore(i,j)) {
	    g.setStrictSuccessor(x,y);
	  } else {
	    g.setSuccessor(x,y);
	  }
	}
	if (isBefore(j,i)) {
	  if (isStrictlyBefore(j,i)) {
	    g.setStrictSuccessor(y,x);
	  } else {
	    g.setSuccessor(y,x);
	  }
	}
      }
    }
  }
}

// --------------------------------------------------------------------------
// SECTION 3. BALANCE CONSTRAINT BOUNDS AND HEURISTICS
// --------------------------------------------------------------------------

class Bounds {
private:
  IlcScheduleEvent _evt;
  IloInt           _q;
  IloInt           _lbmin;
  IloInt           _lbmax;
  IloInt           _lamin;
  IloInt           _lamax;

public:
  static IloInt     CritFunction;
  static void SetCritFunction(IloInt i) {
    CritFunction = i; }
public:
  Bounds(IlcScheduleEvent evt, IloInt q);
  ~Bounds(){};

  IlcScheduleEvent getScheduleEvent() const {
    return _evt; }
  
  void setLbmin(IloInt l) { _lbmin = l; }
  void setLbmax(IloInt l) { _lbmax = l; }
  void setLamin(IloInt l) { _lamin = l; }
  void setLamax(IloInt l) { _lamax = l; }

  IloNum getD()  const { 
    return (_lamin + _lamax - _lbmin - _lbmax)/2.0; }

  IloNum getLackb() const {
    return IloMax(0, - _lbmin); }
  IloNum getLacka() const {
    return IloMax(0, - _lamin); }
  IloNum getExcsb() const {
    return IloMax(0, _lbmax - _q); }
  IloNum getExcsa() const {
    return IloMax(0, _lamax - _q); }

  IloNum getLack() const {
    return IloMax(getLackb(), getLacka()); }
  IloNum getExcs() const {
    return IloMax(getExcsb(), getExcsa()); }

  IloBool isLack() const {
    return (getExcs() <= getLack()); }
  IloBool isProd() const {
    return (getD() > 0); }
  IloBool isCons() const {
    return (getD() < 0); }

  IloNum getCrit() const;
  IloNum getCrit1() const;
  IloNum getCrit2() const;
  IloNum getCrit3() const;
};

IloInt Bounds::CritFunction = 1;

IloNum Bounds::getCrit() const {
  switch (CritFunction) {
  case 1:
    return getCrit1();
    break;
  case 2:
    return getCrit2();
    break;
  case 3:
    return getCrit3();
    break;
  default:
    assert(IloFalse);
  }
}

IloNum Bounds::getCrit1() const {
  IloInt tloose = (1 + _evt.getTimeMax() - _evt.getTimeMin());
  return IloMax(getLack(), getExcs())/ (_q * tloose);
}

IloNum Bounds::getCrit2() const {
  IloInt tloose = (1 + _evt.getTimeMax() - _evt.getTimeMin());
  IloNum critb = IloMax(getLackb(), getExcsb())/(_lbmax-_lbmin+1);
  IloNum crita = IloMax(getLacka(), getExcsa())/(_lamax-_lamin+1);
  return IloMax(critb, crita) / tloose;
}

IloNum Bounds::getCrit3() const {
  IloInt tloose = (1 + _evt.getTimeMax() - _evt.getTimeMin());
  IloNum critb = IloMax(getLackb(), getExcsb())/(_lbmax-_lbmin+1);
  IloNum crita = IloMax(getLacka(), getExcsa())/(_lamax-_lamin+1);
  return IloMax(critb, crita) * IloMax(critb, crita) / tloose;
}

Bounds::Bounds(IlcScheduleEvent evt, IloInt q) 
  :_evt   (evt),
   _q     (q),
   _lbmin (IloIntMax),
   _lbmax (IloIntMin),
   _lamin (IloIntMax),
   _lamax (IloIntMin)
{}

// --------------------------------------------------------------------------
// FUNCTION FOR BUILDING BOUNDS AT INITIALIZATION

void BuildBounds(IloSolver solver) {
  IlcScheduler sched(solver);
  for (IlcReservoirIterator ite(sched); ite.ok(); ++ite) {
    IlcReservoir res = (*ite);
    IloInt q = res.getCapacity();
    IlcScheduleEventPrecedenceGraphConstraint balct =
      res.getBalanceConstraintGraph();
    for (IlcScheduleEventIterator ite(balct); ite.ok(); ++ite) {
      IlcScheduleEvent evt = (*ite);
      Bounds* b = new (solver.getHeap()) Bounds(evt, q);
      evt.getResourceConstraint().setObject((IloAny)b);
    }
  }
}

// --------------------------------------------------------------------------
// UPDATE BOUNDS

void UpdateBounds(IlcSchedule sched) {
  for (IlcReservoirIterator ite(sched); ite.ok(); ++ite) {
    IlcReservoir res = (*ite);
    IloInt q = res.getCapacity();
    IlcScheduleEventPrecedenceGraphConstraint balct =
      res.getBalanceConstraintGraph();
    for (IlcScheduleEventIterator ite2(balct); ite2.ok(); ++ite2) {
      IlcScheduleEvent evt = (*ite2);
      Bounds* b = ((Bounds*)evt.getResourceConstraint().getObject());
      b->setLbmin(q - balct.getMaxOptimisticBefore(evt));
      b->setLamin(q - balct.getMaxOptimisticAfter(evt));
    }
    balct.computeLevels();
    for (IlcScheduleEventIterator ite1(balct); ite1.ok(); ++ite1) {
      IlcScheduleEvent evt = (*ite1);
      Bounds* b = ((Bounds*)evt.getResourceConstraint().getObject());
      b->setLbmax(balct.getMaxOptimisticBefore(evt));
      b->setLamax(balct.getMaxOptimisticAfter(evt));
    }
  }
}

// --------------------------------------------------------------------------
// SECTION 4. TEMPORAL HEURISTICS
// --------------------------------------------------------------------------

IloNum Commit(IloInt xmin, IloInt xmax,
	      IloInt ymin, IloInt ymax) {
  IloNum result = (xmax-ymin+1)*(xmax-ymin+1);
  if (xmin > ymin)
    result -= (xmin-ymin)*(xmin-ymin);
  if (xmax > ymax)
    result -= (xmax-ymax)*(xmax-ymax);
  return result / (2*(ymax-ymin+1)*(xmax-xmin+1));
}

IloNum Commit(IlcScheduleEvent x, 
	      IlcScheduleEvent y, 
	      IloBool strict,
	      NumericGraph* matrix) {
  if (matrix == 0) {
    if (strict) 
      return Commit(x.getTimeMin(), x.getTimeMax(),
		    y.getTimeMin()-1, y.getTimeMax()-1);
    else
      return Commit(x.getTimeMin(), x.getTimeMax(),
		    y.getTimeMin(), y.getTimeMax());
  } else {
    IloInt i = (IloInt)x.getActivity().getObject();
    IloInt j = (IloInt)y.getActivity().getObject();
    IloInt lb =  matrix->getMin(i,j);
    IloInt ub =  matrix->getMax(i,j);
    if (strict) 
      return Commit(0, 0, lb+1, ub+1);
    else
      return Commit(0, 0, lb, ub);
  }
}

// --------------------------------------------------------------------------
// SECTION 5. CRITICAL EVENT SELECTION
// --------------------------------------------------------------------------

IlcScheduleEvent SelectCriticalEvent(IlcSchedule sched) {
  IloNum critMax = 0;
  IlcScheduleEvent selected;
  for (IlcReservoirIterator ite(sched); ite.ok(); ++ite) {
    IlcReservoir res = (*ite);
    IlcScheduleEventPrecedenceGraphConstraint graph =
      res.getBalanceConstraintGraph();
    for (IlcResourceConstraintIterator ite1(*ite); ite1.ok(); ++ite1) {
      Bounds* b = (Bounds*)((*ite1).getObject());
      IlcScheduleEvent evt = b->getScheduleEvent();
      IlcScheduleEventIterator urk(graph, evt, IlcUnranked);
      if (urk.ok()) {
	/* We only condider not completely ranked events. */
	IloNum crit = b->getCrit();
	if (crit > critMax) {
	  critMax  = crit;
	  selected = evt;
	}
      }
    }
  }
  return selected;
}

// --------------------------------------------------------------------------
// SECTION 6. EVENT ORDERING BASIC GOALS
// --------------------------------------------------------------------------

ILCGOAL3(SetSuccessor,
	 IlcScheduleEventPrecedenceGraphConstraint, graph,
	 IlcScheduleEvent, x,
	 IlcScheduleEvent, y) {
  graph.setSuccessor(x,y);
  return 0;
}

ILCGOAL3(SetStrictSuccessor,
	 IlcScheduleEventPrecedenceGraphConstraint, graph,
	 IlcScheduleEvent, x,
	 IlcScheduleEvent, y) {
  graph.setStrictSuccessor(x,y);
  return 0;
}

ILCGOAL2(TrySetSuccessor,
	 IlcScheduleEvent, x,
	 IlcScheduleEvent, y) {
  IloSolver solver = getSolver();
  IlcReservoir res = (IlcReservoir)x.getResource();
  IlcScheduleEventPrecedenceGraphConstraint graph =
    res.getBalanceConstraintGraph();
  return IlcOr(SetSuccessor(solver,graph,x,y),
	       SetStrictSuccessor(solver,graph,y,x));
}

ILCGOAL2(TrySetStrictSuccessor,
	 IlcScheduleEvent, x,
	 IlcScheduleEvent, y) {
  IloSolver solver = getSolver();
  IlcReservoir res = (IlcReservoir)x.getResource();
  IlcScheduleEventPrecedenceGraphConstraint graph =
    res.getBalanceConstraintGraph();
  return IlcOr(SetStrictSuccessor(solver,graph,x,y),
	       SetSuccessor(solver,graph,y,x));
}

// --------------------------------------------------------------------------
// SECTION 7. INITIALIZATION GOAL
// --------------------------------------------------------------------------

IlcScheduleEventPrecedenceGraphConstraint
BuildGlobalPrecedenceGraph(IloSolver solver) {
  IlcScheduler sched(solver);
  IlcScheduleEventPrecedenceGraphConstraint evtpg(sched, IloFalse, -1, IloIntMax);
  for (IlcActivityIterator ite(sched); ite.ok(); ++ite)
    evtpg.addEvent(IlcScheduleEvent(IlcStartEvent, *ite)); 
  solver.add(evtpg);
  return evtpg;
}

// --------------------------------------------------------------------------
// SECTION 8. SELECTION OF AN ORDERING 
// --------------------------------------------------------------------------

static IloInt OrderCritFunction = 1;

IloInt GetType(IloBool isLackX, 
	       IloBool isProdX, 
	       IloBool isProdY) {
  return 4*isLackX + 2*isProdX + isProdY;
}

IloBool IsLackX(IloInt type) {
  return (0 != (type & 4L));
}

IloBool IsProdX(IloInt type) {
  return (0 != (type & 2L));
}

IloBool IsProdY(IloInt type) {
  return (0 != (type & 1L));
}

IloNum GetCrit(IlcScheduleEvent x, IlcScheduleEvent y, 
	       IloBool strict, IloNum q,
	       NumericGraph* matrix) {
  if (q == 0)
    return IloIntMin + 1;
  switch (OrderCritFunction) {
  case 1: // critA
    return IloMin(Commit(x,y,strict,matrix), Commit(y,x,!strict,matrix)) * q;
    break;
  case 2: // critB
    return Commit(x,y,strict,matrix) * q;
    break;
  case 3: // critC
    return - Commit(x,y,strict,matrix) / q;
    break;
  default:
    assert(IloFalse);
  }
}

IloNum ComputeCrit(IlcScheduleEvent x, IlcScheduleEvent y, 
		   IloInt type, IloNum q,
		   NumericGraph* matrix) {
  IloBool strict = (IsProdX(type) == IsProdY(type));
  if (IsLackX(type) == IsProdY(type))
    return GetCrit(y, x, strict, q, matrix);
  else
    return GetCrit(x, y, strict, q, matrix);
}

void SelectOrderedEvent(IlcScheduleEvent x,
			IlcScheduleEvent& selected,
			IloInt& selectedType,
			NumericGraph* matrix) {
  IloNum maxCrit = IloIntMin;
  IloNum crit;
  IloInt type;
  IlcReservoir res = (IlcReservoir)x.getResource();
  IlcScheduleEventPrecedenceGraphConstraint graph =
    res.getBalanceConstraintGraph();
  Bounds* bx = ((Bounds*)x.getResourceConstraint().getObject());
  IloBool isLackX = bx->isLack();
  IloBool isProdX = bx->isProd();
  for (IlcScheduleEventIterator ite1(graph, x, IlcUnranked); ite1.ok(); ++ite1) {
    IlcScheduleEvent y = (*ite1);
    Bounds* ey = ((Bounds*)y.getResourceConstraint().getObject());
    type = GetType(isLackX, isProdX, ey->isProd());
    crit = ComputeCrit(x, y, type, IloAbs(ey->getD()), matrix);
    if (crit > maxCrit) {
      maxCrit = crit;
      selected = y;
      selectedType = type;
    }
  }
}

// --------------------------------------------------------------------------
// SECTION 9. SEARCH GOAL
// --------------------------------------------------------------------------

static IloInt TemporalEnforcement = 0;

ILCGOAL3(Solve, 
	 NumericGraph*, matrix0,
	 NumericGraph*, matrix,
	 IlcScheduleEventPrecedenceGraphConstraint, evtpg) {
  IloSolver solver = getSolver();
  IlcScheduler sched(solver);
  if (matrix0 != 0) {
    matrix->copy(matrix0);
    matrix->synchronize(evtpg);
  }
  UpdateBounds(sched);
  IlcScheduleEvent x = SelectCriticalEvent(sched);  
  if (0 == x.getImpl()) 
    return 0;
  IloInt type;
  IlcScheduleEvent y;
  SelectOrderedEvent(x, y, type, matrix);
  assert(y.getImpl() != 0);
  IloBool strict = (IsProdX(type) == IsProdY(type));
  if (IsLackX(type) == IsProdY(type))
    if (strict) 
      return (IlcAnd(TrySetStrictSuccessor(solver, y, x), this));
    else 
      return (IlcAnd(TrySetSuccessor(solver, y, x), this));
  else 
    if (strict) 
      return (IlcAnd(TrySetStrictSuccessor(solver, x, y), this));
    else 
      return (IlcAnd(TrySetSuccessor(solver, x, y), this));
  return 0;
}

ILCGOAL1(SearchGoalIlc,
	 IloInt, numberOfJobs) {
  IloSolver solver = getSolver();
  IlcScheduler sched(solver);
  IlcScheduleEventPrecedenceGraphConstraint evtpg = 
    BuildGlobalPrecedenceGraph(solver);
  NumericGraph* matrix0 = 0;
  NumericGraph* matrix = 0;
  if (TemporalEnforcement != 0) {
    matrix0 = new (solver.getHeap()) NumericGraph(solver, numberOfJobs+ 2);
    for (IlcActivityIterator ite(sched); ite.ok(); ++ite) {
      IloInt id1 = (IloInt)(*ite).getObject();
      for (IlcFollowingActivityIterator pite(*ite); pite.ok();
	   ++pite) {
	IlcPrecedenceConstraint pct = pite.getPrecedenceConstraint();
	IloInt id2 = (IloInt)(*pite).getObject();
	IloInt min = pct.getDelay();
	matrix0->setMin(id1, id2, min);
      }
      matrix0->setMin(id1, 0, -(*ite).getStartMax()); 
    }
    if (TemporalEnforcement == 1) {
      matrix0->synchronize(evtpg);
      matrix0 = 0;
    } else {
      matrix = new (solver.getHeap()) NumericGraph(solver, numberOfJobs+ 2);
    }
  }
  BuildBounds(solver);
  return Solve(solver, matrix0, matrix, evtpg);
}

ILOCPGOALWRAPPER1(SearchGoal, solver,
		  IloInt, numberOfJobs) {
  return IlcAnd(SearchGoalIlc(solver, numberOfJobs),
		IlcSetTimes(IlcScheduler(solver)));
}

// --------------------------------------------------------------------------
// SECTION 10. DISPLAY SOLUTION
// --------------------------------------------------------------------------

void PrintSolution(IloSolver solver, 
		   IloActivity* activities,
		   IloInt numberOfJobs) {
  IlcScheduler sched(solver);
  cout << "Instance Set    :ubo" << endl;
  cout << "Instance Name   :psp" << endl;
  cout << "Makespan        :" << sched.getActivity(activities[numberOfJobs+1]).getStartVariable().getValue()  << endl;
  cout << "------------------------------------------------------------------" << endl;
  cout << "Solution" << endl;
  cout << "Job \tStart Time" << endl;
  cout << "------------------------------------------------------------------" << endl;

  for (IloInt i = 0; i < numberOfJobs+2; i++)
    solver.out() << i << "\t" 
		 << sched.getActivity(activities[i]).getStartVariable().getValue() << endl;

  cout << "------------------------------------------------------------------" << endl;
}

// --------------------------------------------------------------------------
// SECTION 11. MAIN FUNCTION
// --------------------------------------------------------------------------

int main(int argc, char** argv){ 
  IloInt numberOfJobs, numberOfResources;
  char* filename = argv[1];
  IloInt horizon = atoi(argv[2]);
  if (argc > 3)
    Bounds::SetCritFunction(atoi(argv[3]));
  if (argc > 4)
    OrderCritFunction = atoi(argv[4]);
  if (argc > 5)
    TemporalEnforcement =  atoi(argv[5]);

  IloActivity* activities;
  IloReservoir* reservoirs;
  IloEnv env;
  IloModel model = ReadInstance(filename,
				env,
				horizon,
				numberOfJobs,
				numberOfResources,
				activities,
				reservoirs);

  IloSolver solver(model);
  solver.out() << "INSTANCE " << filename 
	       << ",\t MAKESPAN <= " << horizon 
	       << " :" << endl;

  IloSearchLimit timeLimit = IloTimeLimit(env, 120);

  IloGoal goal =  IloLimitSearch(env, 
				 SearchGoal(env, numberOfJobs),
				 timeLimit);
  
  IloNum t0 = solver.getTime();
  if (solver.solve(goal))
    PrintSolution(solver, activities, numberOfJobs);
  solver.out() << "CPU TIME = " << solver.getTime() - t0 << endl;
  env.end();
  return 0;
}
