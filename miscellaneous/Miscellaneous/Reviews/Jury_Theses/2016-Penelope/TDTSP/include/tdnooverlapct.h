// -------------------------------------------------------------- -*- C++ -*-
// File: tdnooverlapct.h
// --------------------------------------------------------------------------
//
// IBM Confidential
// OCO Source Materials
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2013
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
//
// ---------------------------------------------------------------------------

#ifndef __CP_tdnooverlapctH
#define __CP_tdnooverlapctH

#include <ilcp/cpext.h>

#ifndef __CP_ttimeH
#include <../team/TDTSP/include/ttime.h>
#endif

#ifndef __CP_ilcssequenceiH
#include <ilcp/ilcssequencei.h>
#endif

#ifndef __CP_ilcspgiH
#include <ilcp/ilcspgi.h>
#endif

#ifndef __CP_ilcspgctiH
#include <ilcp/ilcspgcti.h>
#endif

class IlcsPrecGraphI;

class TDNoOverlapCtI : public IlcConstraintI{
private:
  IlcIntervalSequenceVar _s;
  IlcIntVar              _totalTransitionTime;
  IloTDMatrixI*          _costs;
  IlcInt                 _propagationLevel;
  IlcsPrecGraphI*        _pg;
  IlcsDemandListI        _nodeDemands;
  IlcInt*                _work;
  IlcsDemandI**          _workDem;
  IlcInt                 _sizeWork;
  IlcBool                _initialPropagationDone;
  IlcBool                _propagateDisjunctive;
  
public:
  TDNoOverlapCtI(IloCP         cp,
        IlcIntervalSequenceVar s,
        IlcIntVar              totalTT,
        IloTDMatrixI*          matrix,
        IlcInt                 propagLevel);
        
  IlcIntervalSequenceVar getSequence() const {
    return _s;
  }
  IlcIntVar getTotalTransitionTime() const {
    return _totalTransitionTime;
  }
  void setPropagateDisjunctive(IlcBool v, IlcBool reversible =IlcTrue);
  IlcsDemandI* getNodeDemand(IlcInt nodeId) const { return _nodeDemands[nodeId]; }
  IlcsDemandListI* getNodeDemands() { return &_nodeDemands; }
// constraint interface
  virtual void post();
  virtual void propagate();
  virtual void display(ILOSTD(ostream) &) const;
  
  // graph maintenance callback
  void computePossibleFirstsCallBack();
  void computePossibleLastsCallBack();

  IlcsDemandI* getOldLastRankedLast() const;
  IlcsDemandI* getOldLastRankedFirst() const;
  void setToContribute(IlcsDemandI* dem);
  void setToNotContribute(IlcsDemandI* dem);
  IlcBool possiblyContributes(const IlcsDemandI* d) const;
  void resizeWork(IlcInt n);
  IlcInt computeStartMinIfRankedFirst(IlcsDemandI* d);
  IlcInt computeEndMaxIfRankedLast(IlcsDemandI* d);
  void updateUnrankedTT();
  void rankNotFirst       (IlcsDemandI*, IlcsDemandI* =0L);
  void rankFirst       (IlcsDemandI*, IlcsDemandI* =0L);
  void rankNotLast        (IlcsDemandI*, IlcsDemandI* =0L);
  IlcBool checkNewRankedFirst();
  void propagateMaxOnPredecessors   (IlcInt max, IlcsDemandI* d0);
  void propagateMinOnSuccessors     (IlcInt min, IlcsDemandI* d0);                          
  void propagateMaxOnNewPredecessors(IlcInt max, IlcsDemandI* d0);
  void propagateMinOnNewSuccessors  (IlcInt min, IlcsDemandI* d0);                          
  void propagateSuccessorEdge(IlcsDemandI* d0, 
                              IlcsDemandI* d1,
                              IlcInt       tt01,
                              IlcInt       tt10);
  // propagation demons
  void propagateDomain(IlcsDemandI* d);
  void propagateStatus(IlcsDemandI* d);
  void fixSequenceTimes();
  void propagateWhenRankedFirst();
  void propagateWhenRankedLast();  
  void propagateNewLightSuccessors   (IlcInt);
  void propagateNewLightPredecessors (IlcInt);
  void propagateNewSuccessors        (IlcInt);
  void propagateNext                 (IlcInt);
  void propagatePrev                 (IlcInt);
  void propagateDisjunctive(IlcsDemandI* d);
  void initialPropagationRankedFirst();
  void initialPropagationRankedLast();
  
};

/////////////////////////////////////////////////////////////
// MODEL-ENGINE CONSTRAINT WRAPPING
////////////////////////////////////////////////////////////

IloConstraint IloTDNoOverlap(IloEnv cp, IloIntervalSequenceVar s, IloIntVar totalTT, IloInt*** costs, IlcInt sTimeStep, IloInt propagLevel, const char*);

IloConstraint IloTDNoOverlapPiecewiseLinear(IloEnv cp, IloIntervalSequenceVar s, IloIntVar totalTT, IloTDSegmentFunctionMatrix* costs, IloInt propagLevel, const char*);

#endif
