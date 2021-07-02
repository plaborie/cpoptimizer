// -------------------------------------------------------------- -*- C++ -*-
// File: tdnooverlapct.cpp
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

#include <ilcp/cpext.h>

#ifndef __CP_ttimeH
#include <../team/TDTSP/include/ttime.h>
#endif

#ifndef __CP_tdnooverlapctH
#include <../team/TDTSP/include/tdnooverlapct.h>
#endif

#ifndef __CP_ilcclockH
#include <ilcp/ilcclock.h>
#endif

//#define VERBOSE

TDNoOverlapCtI::TDNoOverlapCtI(IloCP cp,
        IlcIntervalSequenceVar s,
        IlcIntVar totalTT,
        IloIntArray2 costs,
        IloIntArray2 iCosts,
        IlcInt propagLevel)
  :IlcConstraintI      (cp)
  ,_s                  (s)
  ,_totalTransitionTime(totalTT)
  ,_costs              (0)
  ,_iCosts              (0)
  ,_propagationLevel   (propagLevel)
  ,_pg                 (0)
  ,_nodeDemands        (cp.getImpl()->getManagerI())
  ,_work               (0)
  ,_workDem            (0)
  ,_sizeWork           (0)
  ,_initialPropagationDone(IlcFalse)
{

  _costs=NULL;// new TDSegmentFunctionMatrix(cp, costs, sTimeStep);
  _iCosts=NULL;
  }


ILCCTDEMON1(CallPropagateDomain, 
  TDNoOverlapCtI, propagateDomain, 
  IlcsDemandI*, demand)
ILCCTDEMON1(CallPropagateStatus, 
  TDNoOverlapCtI, propagateStatus, 
  IlcsDemandI*, demand)
ILCCTDEMON0(CallPropagateWhenRankedFirst, 
  TDNoOverlapCtI, propagateWhenRankedFirst)
ILCCTDEMON0(CallPropagateWhenRankedLast, 
  TDNoOverlapCtI, propagateWhenRankedLast)
ILCCTDEMON0(CallComputePossibleFirstsCallBack, 
  TDNoOverlapCtI, computePossibleFirstsCallBack)
ILCCTDEMON0(CallComputePossibleLastsCallBack, 
  TDNoOverlapCtI, computePossibleLastsCallBack)

void
TDNoOverlapCtI::setToContribute(IlcsDemandI* dem){
  if (_pg) {
    _pg->setToContribute(dem->getPGIndex());
  } else {
    // Not initialized, store until initialization
    ILOS_THROW(IloNoPGException("setToContribute"));
  }
}

void
TDNoOverlapCtI::setToNotContribute(IlcsDemandI* dem){
  if (_pg) {
    _pg->setToNotContribute(dem->getPGIndex());
  } else {
    // Not initialized, store until initialization
    ILOS_THROW(IloNoPGException("setToNotContribute"));
  }
}  
IlcBool
TDNoOverlapCtI::possiblyContributes(const IlcsDemandI* d) const {
  assert(_pg!=0);
  return _pg->getLightNode(d->getPGIndex())->possiblyContributes();
}
void
TDNoOverlapCtI::resizeWork(IlcInt n) {
  IlcAny* array = getManagerI()->getAnyTmpArray(2*n);
  _work    = (IlcInt*) array;
  _workDem = (IlcsDemandI**) &array[n];
}
IlcInt 
TDNoOverlapCtI::computeStartMinIfRankedFirst(IlcsDemandI* d) {
  IlcsIntervalVarI* t = d->getInterval();
  IlcInt smin = t->getStartMinPA();
  IlcInt curr = _pg->getSrfHead();
  if (0==curr) {
    // No surely contributing ranked first
    return smin;
  }
  IlcInt end  = ILCSMAXLPGINDEX;
  assert(curr != end);
  IlcsPrecGraphI::LightNodeI* node = 0;
  IlcsDemandI* dem = 0;
  IlcInt smindtt = IlcIntervalMax;
  while (curr != end) {
    node = _pg->getLightNode(curr);
    dem  = getNodeDemand(curr); 
    curr = node->getPrevInList();
    const IlcInt e = dem->getInterval()->getEndMinPA();
    const IlcInt s = e + _costs->getTransitionTime(dem->getState(), d->getState(), e);
    if (s < smindtt) {
      if (s <= smin) {
        return smin;
      }
      smindtt = s;
    }
  }
  return IlcMaxInt_I(smin, smindtt);
}
IlcInt
TDNoOverlapCtI::computeEndMaxIfRankedLast(IlcsDemandI* d) {
  IlcsIntervalVarI* t = d->getInterval();
  IlcInt emax = t->getEndMaxPA();
  IlcInt curr = _pg->getSrlHead();
  if (1==curr) {
    // No surely contributing ranked last
    return emax;
  }
  IlcInt end  = _pg->_headers[ILCSPOSLISTRL];
  end = _pg->getLightNode(end)->getPrevInList();
  assert(curr != end);
  IlcsPrecGraphI::LightNodeI* node = 0;
  IlcsDemandI* dem = 0;
  IlcInt emaxdtt = IlcIntervalMin;
  while (curr != end) {
    node = _pg->getLightNode(curr);
    dem  = getNodeDemand(curr); 
    curr = node->getPrevInList();
    const IlcInt s = dem->getInterval()->getStartMaxPA();
    const IlcInt e = s - _costs->getInverseTransitionTime(d->getState(),dem->getState(),s);
    if (e > emaxdtt) {
      if (e >= emax) {
        return emax;
      }
      emaxdtt = e;
    }
  }
  return IlcMinInt_I(emax, emaxdtt);
}
void
TDNoOverlapCtI::propagateSuccessorEdge(IlcsDemandI* d0, 
                                         IlcsDemandI* d1,
                                         IlcInt tt01,
                                         IlcInt tt10) {
  IlcsIntervalVarI* t0 = d0->getInterval();
  IlcsIntervalVarI* t1 = d1->getInterval();
  IlcsAssert(d0->isToEndShape());
  IlcsAssert(d1->isFromStartShape());
  if (t0->isPresent()) {
    //if (_pg->surelyContributes(d0->getPGIndex())) { // TODO: if d1 IMPLIES d0
    const IlcInt emin0 = tt01 + t0->getEndMin();
    t1->setStartMin(emin0, Ilcs::PrecedenceGraph);
    t1->setEndMin(emin0 + t1->getLengthMinPA(), Ilcs::PrecedenceGraph);
  }
  if (t1->isPresent()) {
    //if (_pg->surelyContributes(d1->getPGIndex())) { // TODO: if d0 IMPLIES d1
    const IlcInt smax1 = t1->getStartMax() - tt10; 
    t0->setEndMax(smax1, Ilcs::PrecedenceGraph);
    t0->setStartMax(smax1 - t0->getLengthMinPA(), Ilcs::PrecedenceGraph);
  }
}
void
TDNoOverlapCtI::propagateMinOnSuccessors(IlcInt min, IlcsDemandI* d0) {
  IlcsResourceI* _res = _s.getImpl()->getResource();
  const IlcInt s0 = (0==_costs)?0:d0->getState();
  IlcInt transTime01 = 0L;

  if (_pg->isExtended()) {
    IloSched::DemandIteratorFilter succFilter = IloSched::DirectSuccessors;
    for (IlcsPrecGraphCtI::Iterator ite(d0, succFilter); ite.ok(); ++ite) {
      IlcsDemandI* d1 = (*ite);
      if (0 != _costs) {
        transTime01 = _costs->getTransitionTime(s0, d1->getState(), min);
      }
      d1->getInterval()->setStartMin(min+transTime01, Ilcs::PrecedenceGraph);
    }

  } 
  else {
    const IlcsPrecGraphI::LightNodeI* lnode0 = _pg->getLightNode(d0->getPGIndex());
    // Propagate on successors
    IlcsPrecGraphI::LightNodeI::NodeList::ItemI* succs = lnode0->getSuccessors();
    while (succs != 0) {
      IlcsDemandI* d1 = getNodeDemand(succs->getIndex());
      assert (d1 != 0);
      if (0 != _costs) {
        transTime01 = _costs->getTransitionTime(s0, d1->getState(), min);
      }
      d1->getInterval()->setStartMin(min+transTime01, Ilcs::PrecedenceGraph);
      succs = succs->getNext();
    }

    IlcBool keepOn = IlcTrue;
    if (lnode0->isRankedFirst()) {
      // Propagate on ranked first
      IlcsPrecGraphCtI::RankedFirstIterator ite(d0);
      for (--ite; ite.ok(); --ite) {
        IlcsDemandI* d1 = ite.getDemandI();
        if (0 != _costs) {
          transTime01 = _costs->getTransitionTime(s0, d1->getState(), min);
        }
        d1->getInterval()->setStartMin(min+transTime01, Ilcs::PrecedenceGraph);
        const IlcBool surelyCont = ite.surelyContributes();
        if (surelyCont) {
          keepOn = IlcFalse;
          break;
        }
      }
      if (keepOn) {
        // Propagate on unranked
        for (IlcsPrecGraphCtI::UnrankedIterator ite2(_res); ite2.ok(); ++ite2) {
          IlcsDemandI* d1 = ite2.getDemandI();
          if (0 != _costs) {
            transTime01 =_costs->getTransitionTime(s0, d1->getState(), min);
          }
          d1->getInterval()->setStartMin(min+transTime01, Ilcs::PrecedenceGraph);
          const IlcBool surelyCont = ite2.surelyContributes();
          if (surelyCont)
            keepOn = IlcFalse;
        }
      }
    }

    // Propagate on ranked last
    if (keepOn) {
      IlcsPrecGraphCtI::RankedLastIterator ite;
      if (lnode0->isRankedLast()) {
        ite.init(d0);
        ++ite;
      } else {
        ite.init(_res);
      }
      for (; ite.ok(); ++ite) {
        IlcsDemandI* d1 = ite.getDemandI();
        if (0 != _costs) {
          transTime01 = _costs->getTransitionTime(s0, d1->getState(), min);
        }
        d1->getInterval()->setStartMin(min+transTime01, Ilcs::PrecedenceGraph);
        const IlcBool surelyCont = ite.surelyContributes();
        if (surelyCont) {
          keepOn = IlcFalse;
          break;
        }
      }
    }

    // Propagate on future infinite
    IlcsAssert(!_pg->hasFi());
  }
}
void
TDNoOverlapCtI::propagateMaxOnPredecessors(IlcInt max, IlcsDemandI* d0) {
  IlcsResourceI* _res = _s.getImpl()->getResource();
  const IlcInt s0 = (0==_costs)?0:d0->getState();
  IlcsAssert(d0->isOnIntervalShape());
  IlcInt transTime10 = 0L; 
  if (_pg->isExtended()) {
    IloSched::DemandIteratorFilter predFilter = IloSched::DirectPredecessors;
    for (IlcsPrecGraphCtI::Iterator ite(d0, predFilter); ite.ok(); ++ite) {
      IlcsDemandI* d1 = (*ite);
      if (0 != _costs) {
        transTime10 = _costs->getInverseTransitionTime(d1->getState(), s0, max);
      }
      d1->getInterval()->setEndMax(max-transTime10, Ilcs::PrecedenceGraph);
    }

  } else {

    const IlcsPrecGraphI::LightNodeI* lnode0 = _pg->getLightNode(d0->getPGIndex());
    IlcsPrecGraphI::LightNodeI::NodeList::ItemI* preds = lnode0->getPredecessors();
    // Propagate on predecessors
    while (preds != 0) {
      IlcsDemandI* d1 = getNodeDemand(preds->getIndex());
      assert (d1 != 0);
      if (0 != _costs) {
        transTime10 = _costs->getInverseTransitionTime(d1->getState(), s0, max);
      }
      d1->getInterval()->setEndMax(max-transTime10, Ilcs::PrecedenceGraph);
      preds = preds->getNext();
    }

    IlcBool keepOn = IlcTrue;
    if (lnode0->isRankedLast()) {
      // Propagate on ranked last
      IlcsPrecGraphCtI::RankedLastIterator ite;
      ite.init(d0);
      --ite;
      for (; ite.ok(); --ite) {
        IlcsDemandI* d1 = ite.getDemandI();
        if (0 != _costs) {
          transTime10 = _costs->getInverseTransitionTime(d1->getState(), s0, max);
        }
        d1->getInterval()->setEndMax(max-transTime10, Ilcs::PrecedenceGraph);
        const IlcBool surelyCont = ite.surelyContributes();
        if (surelyCont) {
          keepOn = IlcFalse;
          break;
        }
      }
      if (keepOn) {
        // Propagate on unranked
        for (IlcsPrecGraphCtI::UnrankedIterator ite2(_res); ite2.ok(); ++ite2) {
          IlcsDemandI* d1 = ite2.getDemandI();
          if (0 != _costs) {
            transTime10 = _costs->getInverseTransitionTime(d1->getState(), s0, max);
          }
          d1->getInterval()->setEndMax(max-transTime10, Ilcs::PrecedenceGraph);
          const IlcBool surelyCont = ite2.surelyContributes();
          if (surelyCont)
            keepOn = IlcFalse;
        }
      }
    }

    // Propagate on ranked first
    if (keepOn) {
      IlcsPrecGraphCtI::RankedFirstIterator ite;
      if (lnode0->isRankedFirst()) {
        ite.init(d0);
        ++ite;
      } else {
        ite.init(_res);
      }
      //const IlcInt maxminustt = max - ttime->getValueMaxTo(s0); OPTIM QUI N'EST PAS TRES UTILE 
      //LORSQU'ON A PAS DES INTERVALLES OPTIONELS DONC VIRE' DANS 1 PREMIER MOMENT
      for (; ite.ok(); ++ite) {
        IlcsDemandI* d1 = ite.getDemandI();
        const IlcBool surelyCont = ite.surelyContributes();
        if (0 != _costs) {
          transTime10 = _costs->getInverseTransitionTime(d1->getState(), s0, max);
        }
        d1->getInterval()->setEndMax(max - transTime10, Ilcs::PrecedenceGraph);;
        if (surelyCont)
          break;
      }
    }
  }
}
  
void 
TDNoOverlapCtI::propagateDomain(IlcsDemandI* d0) {
#if defined(VERBOSE)
  std::cout << "Calling propagateDomain(";
  d0->getInterval()->display(std::cout);
  std::cout << ")" << std::endl;
#endif
  if (d0->isCondition() || (d0->getPGIndex()<0))
    return;
  IlcsPrecGraphI::LightNodeI* lnode0 = _pg->getLightNode(d0->getPGIndex());
  
  IlcsAssert(d0->isFromStartShape());
  IlcsAssert(d0->isToEndShape());
  const IlcsIntervalVarI* act0 = d0->getInterval();
  if (act0->isPresent()) {
    if (act0->hasDeltaStartMax()) {
      const IlcInt max = act0->getStartMax();
      propagateMaxOnPredecessors(max, d0);
      push();
    }
    if (act0->hasDeltaEndMin()) {
      const IlcInt min = act0->getEndMin();
      propagateMinOnSuccessors(min, d0);
      push();
    }
  } 
  if (!lnode0->isRanked()) { // ADDED
    push();
  }


}

void 
TDNoOverlapCtI::propagateStatus(IlcsDemandI* d0) {
#if defined(VERBOSE)
  std::cout << "Calling propagateStatus(";
  d0->getInterval()->display(std::cout);
  std::cout << ")" << std::endl;
#endif
  if (d0->isCondition() || (d0->getPGIndex()<0))
    return;

  IlcsAssert(d0->isFromStartShape());
  IlcsAssert(d0->isToEndShape());
  IlcsIntervalVarI* act0 = d0->getInterval();
  if (act0->isAbsent()) {
    setToNotContribute(d0);
    return;
  }

  IlcsAssert(act0->isPresent());
  setToContribute(d0);

  if (!possiblyContributes(d0))
    return;
  
  propagateMaxOnPredecessors(act0->getStartMax(), d0);
  propagateMinOnSuccessors(act0->getEndMin(), d0);
 
#if defined(ILCS_UPDATE_TIMENET)
  if (_isExtended) {
    if (d0->getInterval()->canPGUpdatePrecCt())
      updateTimeNet(d0);
  }
#endif

  push();
}
void TDNoOverlapCtI::initialPropagationRankedFirst(){

#if defined(VERBOSE)
  std::cout << "Calling initialPropagationRankedFirst()" << std::endl;
#endif
 // FOR GLOBAL TRANSITION TIME THAT VERIFY TRIANGULAR INEQUALITY
  IlcsIntervalSequenceVarI* seq = _s.getImpl();
  IlcsResourceI* resource = seq->getResource();
  IlcsDemandI* newRF0 = 0;
  IlcInt chainMinLength = 0;
  IlcInt posSrfHead = _pg->getLightNode(_pg->getSrfHead())->getPosition();

  for (IlcsPrecGraphCtI::RankedFirstIterator ite(resource, IlcFalse); ite.ok(); --ite) {
    IlcBool keepOn = IlcTrue;
    IlcsDemandI* newRF = ite.getDemandI();
    IlcInt index = newRF->getPGIndex();
    IlcsPrecGraphI::LightNodeI* nodeNewRF = _pg->getLightNode(index);
    if (0 != newRF0) {
      IlcInt newRF0Type= newRF0->getState(); IlcInt newRFType= newRF->getState();
      const IlcInt dtt = _costs->getTransitionTime(newRF0Type, newRFType, newRF0->getInterval()->getEndMin());
      const IlcInt dtti = _costs->getInverseTransitionTime(newRF0Type, newRFType, newRF->getInterval()->getStartMax());
      propagateSuccessorEdge(newRF0, newRF, dtt, dtti);
    }      
    if (nodeNewRF->surelyContributes())
      chainMinLength += newRF->getInterval()->getLengthMinPA();
    if (posSrfHead <= nodeNewRF->getPosition()) {
      for (IlcsPrecGraphCtI::UnrankedIterator itepf(resource); itepf.ok(); ++itepf) {
        IlcsDemandI* dem = itepf.getDemandI();
        IlcInt newRFType= newRF->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(newRFType, demType, newRF->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(newRFType, demType, dem->getInterval()->getStartMax());
        propagateSuccessorEdge(newRF, dem, tt, tti);
        if (itepf.surelyContributes())
          keepOn = IlcFalse;
      }
    } else {
      keepOn = IlcFalse;
    }
    // Propagate with ranked last
    if (keepOn) {
      for (IlcsPrecGraphCtI::RankedLastIterator iteRL(resource); iteRL.ok(); ++iteRL) {
        IlcsDemandI* dem = iteRL.getDemandI();
        IlcInt newRFType= newRF->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(newRFType, demType, newRF->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(newRFType, demType, dem->getInterval()->getStartMax());
        propagateSuccessorEdge(newRF, dem, tt, tti);
        if (iteRL.surelyContributes())
          break;
      }
    }
    newRF0 = newRF;
  }
  IlcsDemandI* endChain = getNodeDemand(_pg->getSrfHead());
  if (0 != endChain) {
    IlcInt chainEndMax = endChain->getInterval()->getEndMaxPA();
    for (IlcsPrecGraphCtI::RankedFirstIterator ite2(resource, IlcFalse); ite2.ok(); --ite2) {
      newRF0 = ite2.getDemandI();
      IlcInt index0 = newRF0->getPGIndex();
      if (_pg->getLightNode(index0)->surelyContributes()) {
        chainMinLength -= newRF0->getInterval()->getLengthMinPA();
        newRF0->getInterval()->setEndMax(chainEndMax - chainMinLength, Ilcs::PrecedenceGraph);
      }
    }
  }
  push();
}
void TDNoOverlapCtI::initialPropagationRankedLast(){
#if defined(VERBOSE)
  std::cout << "Calling initialPropagationRankedLast()" << std::endl;
#endif

  // FOR GLOBAL TRANSITION TIME THAT VERIFY TRIANGULAR INEQUALITY
  IlcsIntervalSequenceVarI* seq = _s.getImpl();
  IlcsResourceI* resource = seq->getResource();
  IlcsDemandI* newRL0 = 0;
  IlcInt chainMinLength = 0;
  IlcInt posSrlHead = _pg->getLightNode(_pg->getSrlHead())->getPosition();
  for (IlcsPrecGraphCtI::RankedLastIterator ite(resource, IlcFalse); ite.ok(); --ite) {
    IlcBool keepOn = IlcTrue;
    IlcsDemandI* newRL = ite.getDemandI();
    IlcInt index = newRL->getPGIndex();
    IlcsPrecGraphI::LightNodeI* nodeNewRL = _pg->getLightNode(index);
    if (0 != newRL0) {
    IlcInt newRL0Type= newRL0->getState(); IlcInt newRLType= newRL->getState();
      const IlcInt dtt = _costs->getTransitionTime(newRLType, newRL0Type, newRL->getInterval()->getEndMin());
      const IlcInt dtti = _costs->getInverseTransitionTime(newRLType, newRL0Type, newRL0->getInterval()->getStartMax());
      propagateSuccessorEdge(newRL, newRL0, dtt, dtti);
    }
    if (nodeNewRL->surelyContributes())
      chainMinLength += newRL->getInterval()->getLengthMinPA();
    if (posSrlHead <= nodeNewRL->getPosition()) {
      for (IlcsPrecGraphCtI::UnrankedIterator itepl(resource); itepl.ok(); ++itepl) {
        IlcsDemandI* dem = itepl.getDemandI();
        IlcInt newRLType= newRL->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(demType, newRLType, dem->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(demType, newRLType, newRL->getInterval()->getStartMax());
        propagateSuccessorEdge(dem, newRL, tt, tti);
        if (itepl.surelyContributes())
          keepOn = IlcFalse;
      }
    } else {
      keepOn = IlcFalse;
    }

    // Propagate with ranked first
    if (keepOn) {
      for (IlcsPrecGraphCtI::RankedFirstIterator iteRF(resource); iteRF.ok(); ++iteRF) {
        IlcsDemandI* dem = iteRF.getDemandI();
        IlcInt newRLType= newRL->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(demType, newRLType, dem->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(demType, newRLType, newRL->getInterval()->getStartMax());
        propagateSuccessorEdge(dem, newRL, tt, tti);
        if (iteRF.surelyContributes()) {
          break;
        }
      }
    } 
    newRL0 = newRL;
  }
  IlcsDemandI* startChain = getNodeDemand(_pg->getSrlHead());
  if (0 != startChain) {
    IlcInt chainStartMin = startChain->getInterval()->getStartMinPA();
    for (IlcsPrecGraphCtI::RankedLastIterator ite2(resource, IlcFalse); ite2.ok(); --ite2) {
      newRL0 = ite2.getDemandI();
      IlcInt index0 = newRL0->getPGIndex();
      if (_pg->getLightNode(index0)->surelyContributes()) {
        chainMinLength -= newRL0->getInterval()->getLengthMinPA();
        newRL0->getInterval()->setStartMin(chainStartMin + chainMinLength, Ilcs::PrecedenceGraph);
      }
    }
  }
 push();

}
void TDNoOverlapCtI::propagateWhenRankedFirst() {
#if defined(VERBOSE)
  std::cout << "Calling propagateWhenRankedFirst()" << std::endl;
#endif
 // FOR GLOBAL TRANSITION TIME THAT VERIFY TRIANGULAR INEQUALITY
  IlcsIntervalSequenceVarI* seq = _s.getImpl();
  IlcsResourceI* resource = seq->getResource();
  IlcsDemandI* newRF0 = 0;
  IlcInt chainMinLength = 0;
  IlcInt posSrfHead = _pg->getLightNode(_pg->getSrfHead())->getPosition();
  for (IlcsPrecGraphCtI::RankedFirstDeltaIterator ite(resource); ite.ok(); ++ite) {
    IlcBool keepOn = IlcTrue;
    IlcsDemandI* newRF = ite.getDemandI();
    IlcInt index = newRF->getPGIndex();
    IlcsPrecGraphI::LightNodeI* nodeNewRF = _pg->getLightNode(index);
    if (0 != newRF0) {
      IlcInt newRF0Type= newRF0->getState(); IlcInt newRFType= newRF->getState();
      const IlcInt dtt = _costs->getTransitionTime(newRF0Type, newRFType, newRF0->getInterval()->getEndMin());
      const IlcInt dtti = _costs->getInverseTransitionTime(newRF0Type, newRFType, newRF->getInterval()->getStartMax());
      propagateSuccessorEdge(newRF0, newRF, dtt, dtti);
    }      
    if (nodeNewRF->surelyContributes())
      chainMinLength += newRF->getInterval()->getLengthMinPA();
    if (posSrfHead <= nodeNewRF->getPosition()) {
      for (IlcsPrecGraphCtI::UnrankedIterator itepf(resource); itepf.ok(); ++itepf) {
        IlcsDemandI* dem = itepf.getDemandI();
        IlcInt newRFType= newRF->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(newRFType, demType, newRF->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(newRFType, demType, dem->getInterval()->getStartMax());
        propagateSuccessorEdge(newRF, dem, tt, tti);
        if (itepf.surelyContributes())
          keepOn = IlcFalse;
      }
    } else {
      keepOn = IlcFalse;
    }
    // Propagate with ranked last
    if (keepOn) {
      for (IlcsPrecGraphCtI::RankedLastIterator iteRL(resource); iteRL.ok(); ++iteRL) {
        IlcsDemandI* dem = iteRL.getDemandI();
        IlcInt newRFType= newRF->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(newRFType, demType, newRF->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(newRFType, demType, dem->getInterval()->getStartMax());
        propagateSuccessorEdge(newRF, dem, tt, tti);
        if (iteRL.surelyContributes())
          break;
      }
    }
    newRF0 = newRF;
  }
  IlcsDemandI* endChain = getNodeDemand(_pg->getSrfHead());
  if (0 != endChain) {
    IlcInt chainEndMax = endChain->getInterval()->getEndMaxPA();
    for (IlcsPrecGraphCtI::RankedFirstDeltaIterator ite2(resource); ite2.ok(); ++ite2) {
      newRF0 = ite2.getDemandI();
      IlcInt index0 = newRF0->getPGIndex();
      if (_pg->getLightNode(index0)->surelyContributes()) {
        chainMinLength -= newRF0->getInterval()->getLengthMinPA();
        newRF0->getInterval()->setEndMax(chainEndMax - chainMinLength, Ilcs::PrecedenceGraph);
      }
    }
  }
  push();
}
  
void TDNoOverlapCtI::propagateWhenRankedLast() {
#if defined(VERBOSE)
  std::cout << "Calling propagateWhenRankedLast()" << std::endl;
#endif

  // FOR GLOBAL TRANSITION TIME THAT VERIFY TRIANGULAR INEQUALITY
  IlcsIntervalSequenceVarI* seq = _s.getImpl();
  IlcsResourceI* resource = seq->getResource();
  IlcsDemandI* newRL0 = 0;
  IlcInt chainMinLength = 0;
  IlcInt posSrlHead = _pg->getLightNode(_pg->getSrlHead())->getPosition();
  for (IlcsPrecGraphCtI::RankedLastDeltaIterator ite(resource); ite.ok(); ++ite) {
    IlcBool keepOn = IlcTrue;
    IlcsDemandI* newRL = ite.getDemandI();
    IlcInt index = newRL->getPGIndex();
    IlcsPrecGraphI::LightNodeI* nodeNewRL = _pg->getLightNode(index);
    if (0 != newRL0) {
    IlcInt newRL0Type= newRL0->getState(); IlcInt newRLType= newRL->getState();
      const IlcInt dtt = _costs->getTransitionTime(newRLType, newRL0Type, newRL->getInterval()->getEndMin());
      const IlcInt dtti = _costs->getInverseTransitionTime(newRLType, newRL0Type, newRL0->getInterval()->getStartMax());
      propagateSuccessorEdge(newRL, newRL0, dtt, dtti);
    }
    if (nodeNewRL->surelyContributes())
      chainMinLength += newRL->getInterval()->getLengthMinPA();
    if (posSrlHead <= nodeNewRL->getPosition()) {
      for (IlcsPrecGraphCtI::UnrankedIterator itepl(resource); itepl.ok(); ++itepl) {
        IlcsDemandI* dem = itepl.getDemandI();
        IlcInt newRLType= newRL->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(demType, newRLType, dem->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(demType, newRLType, newRL->getInterval()->getStartMax());
        propagateSuccessorEdge(dem, newRL, tt, tti);
        if (itepl.surelyContributes())
          keepOn = IlcFalse;
      }
    } else {
      keepOn = IlcFalse;
    }

    // Propagate with ranked first
    if (keepOn) {
      for (IlcsPrecGraphCtI::RankedFirstIterator iteRF(resource); iteRF.ok(); ++iteRF) {
        IlcsDemandI* dem = iteRF.getDemandI();
        IlcInt newRLType= newRL->getState(); IlcInt demType= dem->getState();
        const IlcInt tt = _costs->getTransitionTime(demType, newRLType, dem->getInterval()->getEndMin());
        const IlcInt tti = _costs->getInverseTransitionTime(demType, newRLType, newRL->getInterval()->getStartMax());
        propagateSuccessorEdge(dem, newRL, tt, tti);
        if (iteRF.surelyContributes()) {
          break;
        }
      }
    } 
    newRL0 = newRL;
  }
  IlcsDemandI* startChain = getNodeDemand(_pg->getSrlHead());
  if (0 != startChain) {
    IlcInt chainStartMin = startChain->getInterval()->getStartMinPA();
    for (IlcsPrecGraphCtI::RankedLastDeltaIterator ite2(resource); ite2.ok(); ++ite2) {
      newRL0 = ite2.getDemandI();
      IlcInt index0 = newRL0->getPGIndex();
      if (_pg->getLightNode(index0)->surelyContributes()) {
        chainMinLength -= newRL0->getInterval()->getLengthMinPA();
        newRL0->getInterval()->setStartMin(chainStartMin + chainMinLength, Ilcs::PrecedenceGraph);
      }
    }
  }
 push();
}
ILCSAFEBILINEARCLOCK(IlcClock_PrecGraphCt_computePossibleFirsts_1a);
ILCSAFEBILINEARCLOCK(IlcClock_PrecGraphCt_computePossibleFirsts_1b);
ILCSAFEBILINEARCLOCK(IlcClock_PrecGraphCt_computePossibleFirsts_2);
ILCSAFELINEARCLOCK(IlcClock_PrecGraphCt_computePossibleFirsts_3a);
ILCSAFELINEARCLOCK(IlcClock_PrecGraphCt_computePossibleFirsts_3b);

void TDNoOverlapCtI::computePossibleFirstsCallBack() {
#if defined(VERBOSE)
  std::cout << "Calling computePossibleFirstsCallBack()" << std::endl;
#endif
  assert (!_pg->isExtended());
  IlcInt next = _pg->_headers[ILCSPOSLISTNPFPL];
  IlcInt end  = _pg->_headers[ILCSPOSLISTRL];
  // NO NOT POSSIBLE FIRSTS
  if (next == end)
    return;
  IlcsResourceI* res = _s.getImpl()->getResource();
  
  if (0 != _costs) {

    // 1. Use a necessary condition using transition time max
    IlcClock_PrecGraphCt_computePossibleFirsts_1a clock1a(getManagerI());

    IlcInt minStartMaxUR       = IlcIntMax;
    IlcInt almostMinStartMaxUR = IlcIntMax;
    IlcInt idMinStartMaxUR     = ILCSMAXLPGINDEX;
    for (IlcsPrecGraphCtI::UnrankedIterator ite(res); ite.ok(); ++ite) {
      if (ite.surelyContributes()) {
        IlcInt startMax = ite.getDemandI()->getInterval()->getStartMax();
        if (startMax < minStartMaxUR) {
          almostMinStartMaxUR = minStartMaxUR;
          minStartMaxUR   = startMax;
          idMinStartMaxUR = ite.getIndex();
        } else if (startMax < almostMinStartMaxUR) {
          almostMinStartMaxUR = startMax;
        }
        clock1a.tick();
      } else
        clock1a.tock();
    }
    IlcClock_PrecGraphCt_computePossibleFirsts_1b clock1b(clock1a.passEnd());
    IlcInt j = next;
    IlcsDemandI* dem;
    IlcsPrecGraphI::LightNodeI* node;
    IlcInt endMin;
    while (j != end) {
      node = _pg->getLightNode(j);
      next = node->getNextInList();
      if (_pg->isPossibleFirstLight(j)) {
        dem  = getNodeDemand(j); 
        const IlcInt s = dem->getState();
        IlcsIntervalVarI* itv = dem->getInterval();
        endMin =computeStartMinIfRankedFirst(dem) + itv->getLengthMinPA();
        IlcInt minStartMax = minStartMaxUR;
        if (idMinStartMaxUR == j)
          minStartMax = almostMinStartMaxUR;
        const IlcInt tt = _costs->getCostMax(); // OPTIM POSSIBLE: CALCULER UN MAX MOINS GLOBAL
        if (endMin + tt <= minStartMax)
          _pg->markPossibleFirstLight(j);
        clock1b.tick();
      } else
        clock1b.tock();
      j = next;
    }

    // 2. Use the real condition using transition time between pairs
    // of demands

    next = _pg->_headers[ILCSPOSLISTNPFPL];
    end  = _pg->_headers[ILCSPOSLISTRL];
    // NO MORE NOT POSSIBLE FIRSTS
    if (next == end)
      return;

    IlcClock_PrecGraphCt_computePossibleFirsts_2 clock2(clock1b.passEnd());

    // STORE UNRANKED
    IlcInt nbPossFirst = res->getPossibleChanges().getSize();
    resizeWork(nbPossFirst);
    IlcInt* startMaxFirsts = _work;
    IlcsDemandI** firsts   = _workDem;
    IlcInt size = 0;
    for (IlcsPrecGraphCtI::UnrankedIterator ite(res); ite.ok(); ++ite) {
      clock2.tick();
      if (ite.surelyContributes()) {
        dem = ite.getDemandI();
        startMaxFirsts[size] = dem->getInterval()->getStartMax();
        firsts[size] = dem;
        ++size;
      }
    }
    j = next;

    while (j != end) {
      clock2.tock();
      node = _pg->getLightNode(j);
      next = node->getNextInList();
      dem  = getNodeDemand(j);
      const IlcInt s = dem->getState();
      IlcsIntervalVarI* itv = dem->getInterval();
      endMin = computeStartMinIfRankedFirst(dem) + itv->getLengthMinPA();
      IlcBool isPossFirst = _pg->isPossibleFirstLight(j);
      if (isPossFirst) {
        const IlcInt maxtt = _costs->getCostMax();
        for (IlcInt i=0; isPossFirst && (i < size); ++i) {
          if (firsts[i] != dem) {
            const IlcInt diff = startMaxFirsts[i] - endMin;
            // To avoid costly call of getValueMin we first check using maximum transition time:
            if (maxtt > diff) {
              const IlcInt tt = _costs->getInverseTransitionTime(s, firsts[i]->getState(), startMaxFirsts[i]);
              if (tt > diff) {
                isPossFirst = IlcFalse;
                break;
              }
            }
          }
        }
        if (isPossFirst)
          _pg->markPossibleFirstLight(j);
      }
      j = next;
    }
    getManagerI()->returnTmpArray((IlcAny*)_work);

  }
}
  
void TDNoOverlapCtI::computePossibleLastsCallBack() {
#if defined(VERBOSE)
  std::cout << "Calling computePossibleLastsCallBack()" << std::endl;
#endif
   assert (!_pg->isExtended());

  IlcInt next1 = _pg->_headers[ILCSPOSLISTPFNPL];
  IlcInt end1  = _pg->_headers[ILCSPOSLISTPFPL];
  IlcInt next2 = _pg->_headers[ILCSPOSLISTNPFNPL];
  IlcInt end2  = _pg->_headers[ILCSPOSLISTRL];

  // NO NOT POSSIBLE LASTS
  if ((next1 == end1) && (next2 == end2))
    return;
  IlcsResourceI* res = _s.getImpl()->getResource();
 
  if (0 != _costs) {

    // 1. Use a necessary condition using transition time max

    IlcInt maxEndMinUR       = IlcIntMin;
    IlcInt almostMaxEndMinUR = IlcIntMin;
    IlcInt idMaxEndMinUR     = ILCSMAXLPGINDEX;
    for (IlcsPrecGraphCtI::UnrankedIterator ite(res); ite.ok(); ++ite)
      if (ite.surelyContributes()) {
        IlcInt endMin = ite.getDemandI()->getInterval()->getEndMin();
        if (endMin > maxEndMinUR) {
          almostMaxEndMinUR = maxEndMinUR;
          maxEndMinUR   = endMin;
          idMaxEndMinUR = ite.getIndex();
        } else if (endMin > almostMaxEndMinUR) {
          almostMaxEndMinUR = endMin;
        }
      }

    IlcInt j = next1;
    IlcsDemandI* dem;
    IlcsPrecGraphI::LightNodeI* node;
    IlcInt startMax;
    while (j != end1) {
      node = _pg->getLightNode(j);
      next1 = node->getNextInList();
      if (_pg->isPossibleLastLight(j)) {
        dem  = ILC_GET_NODE_ID_DEMAND(node, j);
        const IlcInt s = dem->getState();
        IlcsIntervalVarI* itv = dem->getInterval();
        startMax = computeEndMaxIfRankedLast(dem) - itv->getLengthMinPA();
        IlcInt maxEndMin = maxEndMinUR;
        if (idMaxEndMinUR == j)
          maxEndMin = almostMaxEndMinUR;
        const IlcInt tt = _costs->getCostMax(); 
        if (maxEndMin + tt <= startMax)
          _pg->markPossibleLast(j);
      }
      j = next1;
    }

    j = next2;
    while (j != end2) {
      node = _pg->getLightNode(j);
      next2 = node->getNextInList();
      if (_pg->isPossibleLastLight(j)) {
        dem  = getNodeDemand(j);
        const IlcInt s = dem->getState();
        IlcsIntervalVarI* itv = dem->getInterval();
        startMax = computeEndMaxIfRankedLast(dem) - itv->getLengthMinPA();
        IlcInt maxEndMin = maxEndMinUR;
        if (idMaxEndMinUR == j)
          maxEndMin = almostMaxEndMinUR;
        const IlcInt tt =  _costs->getCostMax(); 
        if (maxEndMin + tt <= startMax)
          _pg->markPossibleLast(j);
      }
      j = next2;
    }

    // 2. Use the real condition using transition time between pairs
    // of demands

    next1 = _pg->_headers[ILCSPOSLISTPFNPL];
    end1  = _pg->_headers[ILCSPOSLISTPFPL];
    next2 = _pg->_headers[ILCSPOSLISTNPFNPL];
    end2  = _pg->_headers[ILCSPOSLISTRL];
    // NO MORE NOT POSSIBLE LASTS
    if ((next1 == end1) && (next2 == end2))
      return;

    IlcInt nbPossLast = res->getPossibleChanges().getSize();
    resizeWork(nbPossLast);
    IlcInt* endMinLasts = _work;
    IlcsDemandI** lasts = _workDem;

    IlcInt size = 0;
    for (IlcsPrecGraphCtI::UnrankedIterator ite(res); ite.ok(); ++ite)
      if (ite.surelyContributes()) {
        dem = ite.getDemandI();
        endMinLasts[size] = dem->getInterval()->getEndMin();
        lasts[size] = dem;
        ++size;
      }

    j = next1;
    while (j != end1) {
      node = _pg->getLightNode(j);
      dem  = getNodeDemand(j);
      next1 = node->getNextInList();
      IlcsIntervalVarI* itv = dem->getInterval();
      startMax = computeEndMaxIfRankedLast(dem) - itv->getLengthMinPA();
      IlcBool isPossLast = _pg->isPossibleLastLight(j);
      for (IlcInt i=0; isPossLast && (i < size); ++i) {
        if (lasts[i] != dem) {
          IlcInt tt = 0;
          if (0 != _costs) {
            tt = _costs->getTransitionTime(lasts[i]->getState(), dem->getState(), endMinLasts[i]);
          }
          if (endMinLasts[i] + tt > startMax) {
            isPossLast = IlcFalse;
          }
        }
      }
      if (isPossLast)
        _pg->markPossibleLastLight(j);
      j = next1;
    }

    j = next2;
    while (j != end2) {
      node = _pg->getLightNode(j);
      dem  = getNodeDemand(j);
      next2 = node->getNextInList();
      IlcsIntervalVarI* itv = dem->getInterval();
      startMax = computeEndMaxIfRankedLast(dem) - itv->getLengthMinPA();
      IlcBool isPossLast = _pg->isPossibleLastLight(j);
      for (IlcInt i=0; isPossLast && (i < size); ++i) {
        if (lasts[i] != dem) {
          IlcInt tt = 0;
          if (0 != _costs) {
            tt = _costs->getTransitionTime(lasts[i]->getState(), dem->getState(), endMinLasts[i]);
          }
          if (endMinLasts[i] + tt > startMax) {
            isPossLast = IlcFalse;
          }
        }
      }
      if (isPossLast)
        _pg->markPossibleLastLight(j);
      j = next2;
    }
    getManagerI()->returnTmpArray((IlcAny*)_work);
  } else {

    IlcInt maxEndMinUR       = IlcIntMin;
    IlcInt almostMaxEndMinUR = IlcIntMin;
    IlcInt idMaxEndMinUR     = ILCSMAXLPGINDEX;

    for (IlcsPrecGraphCtI::UnrankedIterator ite(res); ite.ok(); ++ite)
      if (ite.surelyContributes()) {
        IlcInt endMin = ite.getDemandI()->getInterval()->getEndMin();
        if (endMin > maxEndMinUR) {
          almostMaxEndMinUR = maxEndMinUR;
          maxEndMinUR   = endMin;
          idMaxEndMinUR = ite.getIndex();
        } else if (endMin > almostMaxEndMinUR) {
          almostMaxEndMinUR = endMin;
        }
      }

    IlcInt j = next1;
    IlcsDemandI* dem;
    IlcsPrecGraphI::LightNodeI* node;
    IlcInt startMax;
    while (j != end1) {
      node = _pg->getLightNode(j);
      next1 = node->getNextInList();
      if (_pg->isPossibleLastLight(j)) {
        dem  = getNodeDemand(j);
        startMax = dem->getInterval()->getStartMaxPA();
        IlcInt maxEndMin = maxEndMinUR;
        if (idMaxEndMinUR == j)
          maxEndMin = almostMaxEndMinUR;
        if(maxEndMin <= startMax)
          _pg->markPossibleLast(j);
      }
      j = next1;
    }

    j = next2;
    while (j != end2) {
      node = _pg->getLightNode(j);
      next2 = node->getNextInList();
      if (_pg->isPossibleLastLight(j)) {
        dem  = getNodeDemand(j);
        startMax = dem->getInterval()->getStartMaxPA();
        IlcInt maxEndMin = maxEndMinUR;
        if (idMaxEndMinUR == j)
          maxEndMin = almostMaxEndMinUR;
        if(maxEndMin <= startMax)
          _pg->markPossibleLast(j);
      }
      j = next2;
    }
  }
}

ILCBILINEARCLOCK(IlcClock_PrecGraphCt_updateUnrankedTT_1);
ILCLINEARCLOCK(IlcClock_PrecGraphCt_updateUnrankedTT_2);
ILCBILINEARCLOCK(IlcClock_PrecGraphCt_updateUnrankedTT_3);

void
TDNoOverlapCtI::updateUnrankedTT() {
  IlcClock_PrecGraphCt_updateUnrankedTT_1 clock1(getManagerI());
  IlcsResourceI* res = _s.getImpl()->getResource();
  // Only surely contributing unranked demand are considered in these sets
  IlcInt minStartMin              = IlcIntMax;
  IlcInt aminStartMin             = IlcIntMax;
  IlcsDemandI* minStartMinD       = 0;
  IlcInt minStartMax              = IlcIntMax;
  IlcInt aminStartMax             = IlcIntMax;
  IlcsDemandI* minStartMaxD       = 0;
  IlcInt maxEndMin                = IlcIntMin;
  IlcInt amaxEndMin               = IlcIntMin;
  IlcsDemandI* maxEndMinD         = 0;
  IlcInt minLengthFirst           = IlcIntMax; // Includes TTMinFrom
  IlcInt minLengthLast            = IlcIntMax; // Includes TTMinTo
  IlcInt totalLength              = 0;         // Includes TTMinFrom
  IlcInt sureNumber               = 0;         // Number of contributing
  IlcBool sureFirst               = IlcTrue;   // All the possible first contribute
  IlcBool sureLast                = IlcTrue;   // All the possible lasts contribute
  IlcInt minStartMaxMTTMin        = IlcIntMax;
  IlcInt aminStartMaxMTTMin       = IlcIntMax;
  IlcsDemandI* minStartMaxMTTMinD = 0;
  IlcInt maxStartMaxMTTMin        = IlcIntMin;
  IlcInt maxEndMinPTTMin          = IlcIntMin;
  IlcInt amaxEndMinPTTMin         = IlcIntMin;
  IlcsDemandI* maxEndMinPTTMinD   = 0;
  IlcInt minEndMinPTTMin          = IlcIntMax;
  IlcInt minStartMaxMTTMax        = IlcIntMax;
  IlcInt aminStartMaxMTTMax       = IlcIntMax;
  IlcsDemandI* minStartMaxMTTMaxD = 0;
  IlcInt maxEndMaxPTTMin          = IlcIntMin;
  IlcInt amaxEndMaxPTTMin         = IlcIntMin;
  IlcsDemandI* maxEndMaxPTTMinD   = 0;
  IlcInt maxEndMinPTTMax          = IlcIntMin;
  IlcInt amaxEndMinPTTMax         = IlcIntMin;
  IlcsDemandI* maxEndMinPTTMaxD   = 0;
  IlcsDemandI** dmds  = 0;
  IlcInt* sMaxs = 0;
  IlcInt* eMins = 0;
  IlcInt nbMaxUnranked = res->getPossibleChanges().getSize();
  // This function may be called very often and then IlcManagerI::getAnyTmpArray may be a bottleneck. 
  // Therefore we use IlcsScheduleI::getTmpMemory instead. It's up to 10x speedup for big models.
  IlcsScheduleI* sched = res->getSchedule();
  sched->reserveTmpMemory(3 * nbMaxUnranked * sizeof(IlcAny));
  dmds = (IlcsDemandI**) sched->getTmpMemory(nbMaxUnranked * sizeof(IlcAny));
  sMaxs = (IlcInt*) sched->getTmpMemory(2 * nbMaxUnranked * sizeof(IlcAny));
  eMins = sMaxs + nbMaxUnranked;


  // These ones are used when there is a direct transition time and are computed 
  // on ALL unranked demands
  IlcInt maxStartMaxMTTMinAll        = IlcIntMin; // +
  IlcInt amaxStartMaxMTTMinAll       = IlcIntMin; // +
  IlcsDemandI* maxStartMaxMTTMinAllD = 0;         // +
  IlcInt minEndMinPTTMinAll          = IlcIntMax; // +
  IlcInt aminEndMinPTTMinAll         = IlcIntMax; // +
  IlcsDemandI* minEndMinPTTMinAllD   = 0;         // +

  IlcInt size=0;
  IlcsPrecGraphCtI::UnrankedIterator ite;
  for (ite.init(res); ite.ok(); ++ite) {
    const IlcsPrecGraphI::LightNodeI* lnode = ite.getLightNode();
    const IlcBool sure   = lnode->surelyContributes();
    const IlcBool pfirst = lnode->isPossibleFirst();
    const IlcBool plast  = lnode->isPossibleLast();
    IlcsDemandI* ct = ite.getDemandI();
    const IlcInt s = ct->getState();
    const IlcBool unique = ct->isTypeUniqueInSequence();
    const IlcsIntervalVarI* act = ct->getInterval();
    const IlcInt smax = act->getStartMaxPA();
    const IlcInt emin = act->getEndMinPA();
    IlcInt ttMinTo   = 0;
    IlcInt ttMinFrom = 0;
    if (unique) {
      ttMinTo   = _costs->getMinTo(s, 0); 
      ttMinFrom = _costs->getMinFrom(s, 0);
    } else {
      ttMinTo   = _costs->getMinTo(s, 1); //min (costsMinTo, min sur t (cost[s][s][t]))
      ttMinFrom = _costs->getMinFrom(s, 1);
    }
    if (sure) {
      // Surely contributing
      clock1.tick();
      const IlcInt smin = act->getStartMinPA();
      const IlcInt lmin = act->getLengthMinPA();
      IlcInt ttMaxTo   = _costs->getCostMax(); // OPTIM POSSIBLE: Keep actual max values as for min
      IlcInt ttMaxFrom = _costs->getCostMax();
      sureNumber++;
      totalLength += (lmin + ttMinFrom);
      if (smin < minStartMin) {
        aminStartMin  = minStartMin;
        minStartMin   = smin;
        minStartMinD  = ct;
      } else if (smin < aminStartMin) {
        aminStartMin   = smin;
      }
      if (smax < minStartMax) {
        aminStartMax = minStartMax;
        minStartMax  = smax;
        minStartMaxD = ct;
      } else if (smax < aminStartMax) {
        aminStartMax = smax;
      }
      if (emin > maxEndMin) {
        amaxEndMin = maxEndMin;
        maxEndMin  = emin;
        maxEndMinD = ct;
      } else if (emin > amaxEndMin) {
        amaxEndMin = emin;
      }
      if (smax + lmin + ttMinFrom > maxEndMaxPTTMin) {
        amaxEndMaxPTTMin = maxEndMaxPTTMin;
        maxEndMaxPTTMin  = smax + lmin + ttMinFrom;
        maxEndMaxPTTMinD = ct;
      } else if (smax + lmin + ttMinFrom > amaxEndMaxPTTMin) {
        amaxEndMaxPTTMin = smax + lmin + ttMinFrom;
      }

      if (smax - ttMinTo > maxStartMaxMTTMinAll) {
        amaxStartMaxMTTMinAll = maxStartMaxMTTMinAll;
        maxStartMaxMTTMinAll  = smax - ttMinTo;
        maxStartMaxMTTMinAllD = ct;
      } else if (smax - ttMinTo > amaxStartMaxMTTMinAll) {
        amaxStartMaxMTTMinAll = smax - ttMinTo;
      }

      if (emin + ttMinFrom < minEndMinPTTMinAll) {
        aminEndMinPTTMinAll = minEndMinPTTMinAll;
        minEndMinPTTMinAll  = emin + ttMinFrom;
        minEndMinPTTMinAllD = ct;
      } else if (emin + ttMinFrom < aminEndMinPTTMinAll) {
        aminEndMinPTTMinAll = emin + ttMinFrom;
      }
 
     if (smax - ttMinTo < minStartMaxMTTMin) {
        aminStartMaxMTTMin = minStartMaxMTTMin;
        minStartMaxMTTMin  = smax - ttMinTo;
        minStartMaxMTTMinD = ct;
      } else if (smax - ttMinTo < aminStartMaxMTTMin) {
        aminStartMaxMTTMin = smax - ttMinTo;
      }
      if (smax - ttMaxTo < minStartMaxMTTMax) {
        aminStartMaxMTTMax = minStartMaxMTTMax;
        minStartMaxMTTMax  = smax - ttMaxTo;
        minStartMaxMTTMaxD = ct;
      } else if (smax - ttMaxTo < aminStartMaxMTTMax) {
        aminStartMaxMTTMax = smax - ttMaxTo;
      }
      if (emin + ttMinFrom > maxEndMinPTTMin) {
        amaxEndMinPTTMin = maxEndMinPTTMin;
        maxEndMinPTTMin  = emin + ttMinFrom;
        maxEndMinPTTMinD = ct;
      } else if (emin + ttMinFrom > amaxEndMinPTTMin) {
        amaxEndMinPTTMin = emin + ttMinFrom;
      }
      if (emin + ttMaxFrom > maxEndMinPTTMax) {
        amaxEndMinPTTMax = maxEndMinPTTMax;
        maxEndMinPTTMax  = emin + ttMaxFrom;
        maxEndMinPTTMaxD = ct;
      } else if (emin + ttMaxFrom > amaxEndMinPTTMax) {
        amaxEndMinPTTMax = emin + ttMaxFrom;
      }
      if (pfirst) {
        if (emin + ttMinFrom < minEndMinPTTMin) {
          minEndMinPTTMin  = emin + ttMinFrom;
        }
        if (lmin + ttMinFrom < minLengthFirst) {
          minLengthFirst = lmin + ttMinFrom;
        }
      }
      if (plast) {
        if (smax - ttMinTo > maxStartMaxMTTMin) {
          maxStartMaxMTTMin  = smax - ttMinTo;
        }
        if (lmin + ttMinFrom < minLengthLast) {
          minLengthLast = lmin + ttMinFrom;
        }
      }
      dmds [size] = ct;
      sMaxs[size] = smax;
      eMins[size] = emin;
      ++size;
    

    } else {
      // Not surely contributing
      clock1.tock();
      if (pfirst) {
        sureFirst = IlcFalse;
      }
      if (plast) {
        sureLast = IlcFalse;
      }
      if (emin + ttMinFrom < minEndMinPTTMinAll) {
        aminEndMinPTTMinAll = minEndMinPTTMinAll;
        minEndMinPTTMinAll  = emin + ttMinFrom;
        minEndMinPTTMinAllD = ct;
      } else if (emin + ttMinFrom < aminEndMinPTTMinAll) {
        aminEndMinPTTMinAll = emin + ttMinFrom;
      }
      if (smax - ttMinTo > maxStartMaxMTTMinAll) {
        amaxStartMaxMTTMinAll = maxStartMaxMTTMinAll;
        maxStartMaxMTTMinAll  = smax - ttMinTo;
        maxStartMaxMTTMinAllD = ct;
      } else if (smax - ttMinTo > amaxStartMaxMTTMinAll) {
        amaxStartMaxMTTMinAll = smax - ttMinTo;
      }
    }
  }

  if (sureNumber > 1) {
    if (maxEndMaxPTTMin - minStartMin < totalLength) {
      _pg->fail();
      return;
    }

    IlcClock_PrecGraphCt_updateUnrankedTT_2 clock2(getManagerI());
    IlcInt emin, smax;
    IlcInt sminRF = IlcIntervalMax;
    IlcInt emaxRL = IlcIntervalMin;

    const IlcInt lRFId = _pg->getRfHead();
    IlcsDemandI* lRF = getNodeDemand(lRFId);
    const IlcBool sLRF =  (lRF!=0) && _pg->surelyContributes(lRFId) && (minLengthFirst < IlcIntMax);
    IlcsIntervalVarI* lRFact = 0;
    if (sLRF) {
      lRFact = lRF->getInterval();
    }
#if !defined(ILCS_NO_LPG_EXTRA_PROPAGATION)
    // Added:
    if (lRF) {  
      IlcInt curr = lRFId;
      IlcInt end  = _pg->getSrfHead();
      IlcsPrecGraphI::LightNodeI* currNode = 0;
      IlcsDemandI* currDem = 0;
      for (; ; curr = currNode->getNextInList()) {
        if (curr == 0)
          break;
        currNode = _pg->getLightNode(curr);
        if (currNode->possiblyContributes()) {
          currDem = getNodeDemand(curr);
          currDem->getInterval()->setEndMax(maxEndMaxPTTMin - totalLength, Ilcs::PrecedenceGraph);
        }
        if (curr == end)
          break;
      }
    }
#endif
    const IlcInt lRLId = _pg->getRlHead();
    IlcsDemandI* lRL = getNodeDemand(lRLId);
    const IlcBool sLRL =  (lRL!=0) && _pg->surelyContributes(lRLId) && (minLengthLast < IlcIntMax);
    IlcsIntervalVarI* lRLact = 0;
    if (sLRL) {
      lRLact = lRL->getInterval();
    }
    // Added:
#if !defined(ILCS_NO_LPG_EXTRA_PROPAGATION)
    if (lRL) {  
      IlcInt curr = lRLId;
      IlcInt end  = _pg->getSrlHead();
      IlcsPrecGraphI::LightNodeI* currNode = 0;
      IlcsDemandI* currDem = 0;
      for (; ; curr = currNode->getNextInList()) {
        if (curr == 1)
          break;
        clock2.tick();
        currNode = _pg->getLightNode(curr);
        if (currNode->possiblyContributes()) {
          currDem = getNodeDemand(curr);
          currDem->getInterval()->setStartMin(minStartMin + totalLength, Ilcs::PrecedenceGraph);
        }
        if (curr == end)
          break;
      }
    }
#endif
    IlcClock_PrecGraphCt_updateUnrankedTT_3 clock3(clock2.passEnd());
    for (ite.init(res); ite.ok(); ++ite) {
      clock3.tick();
      IlcsDemandI* ct = ite.getDemandI();
      if (ct->getCapacityMin()) {
        const IlcsPrecGraphI::LightNodeI* lnode = ite.getLightNode();
        IlcsIntervalVarI* act = ct->getInterval();
        const IlcBool sure   = lnode->surelyContributes();
        const IlcInt s = ct->getState();
        const IlcBool unique = ct->isTypeUniqueInSequence();
        const IlcInt tt  = (unique)?_costs->getMinFrom(s,0):_costs->getMinFrom(s,1);
        const IlcInt lmin = act->getLengthMinPA() + tt;
        const IlcInt energyOthers = totalLength - lmin;
        // CONSIDER POSSIBLE FIRST DEMANDS AND TRY TO DISCOVER WHEN
        // IT CANNOT BE FIRST
        const IlcBool posFirst = lnode->isPossibleFirst();
        if (posFirst) {
          sminRF = computeStartMinIfRankedFirst(ct);
          emin = sminRF + act->getLengthMinPA();
          IlcInt emax = maxEndMaxPTTMin;
          if (ct == maxEndMaxPTTMinD) {
            emax = amaxEndMaxPTTMin;
          }
          IlcBool keepOn = IlcTrue;
          IlcBool noDeduction = IlcFalse;
          if (emax < emin + tt + energyOthers) {
            rankNotFirst(ct);
            keepOn = IlcFalse;
          }
          if (keepOn) {
            IlcInt lminStartMaxMTTMax = minStartMaxMTTMax;
            if (minStartMaxMTTMaxD == ct)
              lminStartMaxMTTMax = aminStartMaxMTTMax;
            if (emin <= lminStartMaxMTTMax) {
              keepOn = IlcFalse;
              noDeduction = IlcTrue;
            }
          }
          IlcInt lminStartMax = minStartMax;
          if (keepOn) {
            if (minStartMaxD == ct) {
              lminStartMax = aminStartMax;
            } else {
              // Propagate disjunction between ct and minStartMaxD
              const IlcInt ttctminstartmax = _costs->getTransitionTime(s, minStartMaxD->getState(), act->getEndMinPA());
              if (act->getEndMinPA() + ttctminstartmax > minStartMax) {//then act cannot come before minStartMaxD
                ct->getInterval()->setStartMin(minStartMaxD->getInterval()->getEndMin()+
                                               _costs->getTransitionTime(minStartMaxD->getState(), s, minStartMax), Ilcs::PrecedenceGraph);
                rankNotFirst(ct);
                keepOn = IlcFalse;
              }
            }
            IlcInt eminTTMax = emin + _costs->getCostMax();
            if (eminTTMax <= lminStartMax) {
              keepOn = IlcFalse;
              noDeduction = IlcTrue;
            }
          }
          if (keepOn) {
            IlcInt lminStartMaxMTTMin = minStartMaxMTTMin;
            if (minStartMaxMTTMinD == ct)
              lminStartMaxMTTMin = aminStartMaxMTTMin;
            if (emin > lminStartMaxMTTMin) {
              rankNotFirst(ct);
              keepOn = IlcFalse;
            }
          }
          if (keepOn) {
            IlcInt eminTTMin = emin;
            if (ct->isTypeUniqueInSequence()) {
              eminTTMin += _costs->getMinFrom(s, 0); 
            } else {
              eminTTMin += _costs->getMinFrom(s, 1);
            }
            if (eminTTMin > lminStartMax) {
              rankNotFirst(ct);
              keepOn = IlcFalse;
            }
          }
          if ((dmds != 0) && keepOn) {
            const IlcInt maxtti = _costs->getCostMax();
            for (IlcInt i=0; i < size; ++i) {
              clock3.tock();
              if (dmds[i] != ct) {
                const IlcInt diff = sMaxs[i] - emin;
                if (maxtti > diff) {
                  // To avoid costly call getValueMin we first compare with max transition time from ct:
                  const IlcInt tt1 = _costs->getTransitionTime(s, dmds[i]->getState(), emin);
                  if (tt1 > diff) {
                    rankNotFirst(ct);
                    keepOn = IlcFalse;
                    break;
                  }
                }
              }
            }
          }

          if (keepOn || noDeduction) {
            // If ct has not been proved to be not first, it can either be first or not first
           IlcInt lminEndMinPTTMinAll = minEndMinPTTMinAll;
            if (minEndMinPTTMinAllD == ct)
              lminEndMinPTTMinAll = aminEndMinPTTMinAll;
            act->setStartMin(IlcMinInt_I(lminEndMinPTTMinAll, sminRF), Ilcs::PrecedenceGraph);
          }

        } else { // !posFirst
          if (sureFirst) {
            if (sLRF && sure) {
              IlcInt ttFromRanked = 0;
              if (lRF->isTypeUniqueInSequence()) {
                ttFromRanked = _costs->getMinFrom(lRF->getState(), 0);
              } else {
                ttFromRanked = _costs->getMinFrom(lRF->getState(), 1);
              }
              lRFact->setEndMax(act->getStartMax() - ttFromRanked - minLengthFirst, Ilcs::PrecedenceGraph);
            }
            act->setStartMin(minEndMinPTTMin, Ilcs::PrecedenceGraph);
          }
        }

        // CONSIDER POSSIBLE LAST DEMANDS AND TRY TO DISCOVER WHEN
        // IT CANNOT BE LAST
        const IlcBool posLast = lnode->isPossibleLast();
        if (posLast) {
          IlcInt smin = minStartMin;
          emaxRL = computeEndMaxIfRankedLast(ct);
          smax = emaxRL - act->getLengthMinPA();
          if (ct == minStartMinD) {
            smin = aminStartMin;
          }
          IlcBool keepOn = IlcTrue;
          IlcBool noDeduction = IlcFalse;
          if (smax < smin + energyOthers) {
            rankNotLast(ct);
            keepOn = IlcFalse;
          }

          if (keepOn) {
            IlcInt lmaxEndMinPTTMax = maxEndMinPTTMax;
            if (maxEndMinPTTMaxD == ct)
              lmaxEndMinPTTMax = amaxEndMinPTTMax;
            if (smax >= lmaxEndMinPTTMax) {
              keepOn = IlcFalse;
              noDeduction = IlcTrue;
            }
          }

          IlcInt lmaxEndMin = maxEndMin;
          if (keepOn) {
            if (maxEndMinD == ct) {
              lmaxEndMin = amaxEndMin;
            } else {
              // Propagate disjunction between ct and maxEndMinD
              const IlcInt ttmaxendminct = _costs->getTransitionTime(maxEndMinD->getState(), s, maxEndMin);
             if (act->getStartMaxPA() < maxEndMin + ttmaxendminct) {//then act cannot start after maxEndMinD
                IlcInt maxEndMinDStartMax= maxEndMinD->getInterval()->getStartMax();
                ct->getInterval()->setEndMax(maxEndMinDStartMax- 
                                             _costs->getInverseTransitionTime(s, maxEndMinD->getState(),maxEndMinDStartMax), 
                                                Ilcs::PrecedenceGraph);
                rankNotLast(ct);
                keepOn = IlcFalse;
              }
            }
            IlcInt smaxTTMax = smax - _costs->getCostMax();
            if (smaxTTMax >= lmaxEndMin) {
              keepOn = IlcFalse;
              noDeduction = IlcTrue;
            }
          }

          if (keepOn) {
            IlcInt lmaxEndMinPTTMin = maxEndMinPTTMin;
            if (maxEndMinPTTMinD == ct)
              lmaxEndMinPTTMin = amaxEndMinPTTMin;
            if (smax < lmaxEndMinPTTMin) {
              rankNotLast(ct);
              keepOn = IlcFalse;
            }
          }

          if (keepOn) {
            IlcInt smaxTTMin = 0;
            if (ct->isTypeUniqueInSequence()) {
              smaxTTMin = smax - _costs->getMinFrom(s, 0);
            } else {
              smaxTTMin = smax - _costs->getMinFrom(s, 1);
            }
            if (smaxTTMin < lmaxEndMin) {
              rankNotLast(ct);
              keepOn = IlcFalse;
            }
          }

          if ((dmds != 0) && keepOn) {
            const IlcInt maxtti = _costs->getCostMax();
            for (IlcInt i=0; i < size; ++i) {
              clock3.tock();
              if (dmds[i] != ct) {
                const IlcInt diff = smax - eMins[i];
                // To avoid costly call of getValueMin we first compare with maximum transition time to ct:
                if (maxtti > diff) {
                  const IlcInt tt1 = _costs->getTransitionTime(dmds[i]->getState(),s, eMins[i]);
                  if (tt1 > diff) {
                    rankNotLast(ct);
                    keepOn = IlcFalse;
                    break;
                  }
                }
              }
            }

          }
          
          if (keepOn || noDeduction) {
            // If ct has not been proved to be not last, it can either be last or not last
            IlcInt lmaxStartMaxMTTMinAll = maxStartMaxMTTMinAll;
            if (maxStartMaxMTTMinAllD == ct)
              lmaxStartMaxMTTMinAll = amaxStartMaxMTTMinAll;
            act->setEndMax(IlcMaxInt_I(lmaxStartMaxMTTMinAll, emaxRL), Ilcs::PrecedenceGraph);
         }

        } else {  // !posLast
          if (sureLast) {
            if (sLRL && sure)
              lRLact->setStartMin(act->getEndMinPA() + tt + minLengthLast, Ilcs::PrecedenceGraph);
            act->setEndMax(maxStartMaxMTTMin, Ilcs::PrecedenceGraph);
         }
        }

        }
    }

  }

  if (dmds != 0)
    res->getSchedule()->releaseTmpMemory();
}

void
TDNoOverlapCtI::rankNotFirst(IlcsDemandI* d1, IlcsDemandI* d2){
  if (_pg) {
    _pg->rankNotFirst(d1->getPGIndex(), d2 ? d2->getPGIndex() : -1L);
    push();
  } else {
    ILOS_THROW(IloNoPGException("rankNotFirst"));
  }
}

void
TDNoOverlapCtI::rankNotLast(IlcsDemandI* d1, IlcsDemandI* d2){
  if (_pg) {
    _pg->rankNotLast(d1->getPGIndex(), d2 ? d2->getPGIndex() : -1L);
    push();
  } else {
    ILOS_THROW(IloNoPGException("rankNotLast"));
  }
}

void
TDNoOverlapCtI::rankFirst(IlcsDemandI* d1, IlcsDemandI* d2){
  if (_pg) {
    _pg->rankFirst(d1->getPGIndex(), d2 ? d2->getPGIndex() : -1L);
    push();
  } else {
    ILOS_THROW(IloNoPGException("rankFirst"));
  }
}

// This is a shortcut for a very usual case where we discover an
// interval that has been fixed at a date that makes it first in the
// sequence. In this case we directly set if first instead of letting
// the graph discover that all interval except this one is notFirst,
// then automatically rank it first and reset all the intervals as
// possible first!
IlcBool TDNoOverlapCtI::checkNewRankedFirst() {
  IlcInt minStartMaxUR = IlcIntMax;
  IlcsDemandI* demCandidateFirst = 0;
  IlcsResourceI* res = _s.getImpl()->getResource();
  IlcsPrecGraphCtI::UnrankedIterator ite;
  for (ite.init(res); ite.ok(); ++ite) {
    IlcsDemandI* ct = ite.getDemandI();
    IlcsIntervalVarI* act = ct->getInterval();
    if (act->getStartMaxPA() < minStartMaxUR) {
      demCandidateFirst = ct;
      minStartMaxUR  = act->getStartMaxPA();
    }
  }
  if (0==demCandidateFirst)
    return IlcFalse;

  IlcInt endMin;
  const IlcInt ttoptim = minStartMaxUR - _costs->getMinTo(demCandidateFirst->getState(), 1);
  for (ite.init(res); ite.ok(); ++ite) {
    IlcsDemandI* ct = ite.getDemandI();
    if (ct != demCandidateFirst) {
      IlcsIntervalVarI* act = ct->getInterval();
      endMin = computeStartMinIfRankedFirst(ct) + act->getLengthMinPA();
      // To avoid costly call getValueMin we first compare with min possible transition time:
      if (endMin <= ttoptim && endMin <= minStartMaxUR - _costs->getTransitionTime(ct->getState(),demCandidateFirst->getState(), endMin))
        return IlcFalse;
    }
  }

  rankFirst(demCandidateFirst);
  return IlcTrue;
}

/////////////////////////////////////////////////////////////
// Members Interface of constraint class
////////////////////////////////////////////////////////////

void TDNoOverlapCtI::post() {
#if defined(VERBOSE)
  std::cout << "Calling post()" << std::endl;
#endif
  IloCP cp = getCP();
  IlcManagerI* m = cp.getImpl()->getManagerI();
  IlcsIntervalSequenceVarI* seq = _s.getImpl();
  _pg = seq->getResource()->getPG();
  assert(_pg != 0);
  _nodeDemands.add(m, (IlcsDemandI*)0); // No demand at index 0
  _nodeDemands.add(m, (IlcsDemandI*)0); // No demand at index 1
  IlcInt n = seq->getSize();
  for (IlcInt j=0; j<n; ++j) {
    IlcsDemandI* d = seq->getDemand(seq->getInterval(j));
    d->setStateEdit(seq->getType(j));
    _nodeDemands.add(m, d);
    d->getInterval()->whenIntervalDomain(CallPropagateDomain(cp, this, d).getImpl());
    d->getInterval()->whenStatus(CallPropagateStatus(cp, this, d).getImpl());
  }
  _s.whenExtendHead(CallPropagateWhenRankedFirst(cp, this));
  _s.whenExtendTail(CallPropagateWhenRankedLast(cp, this));
  _pg->whenRankedFirstPrioritary(CallComputePossibleFirstsCallBack(cp, this).getImpl());
  _pg->whenRankedLastPrioritary(CallComputePossibleLastsCallBack(cp, this).getImpl());
}

void TDNoOverlapCtI::propagate() {
#if defined(VERBOSE)
  std::cout << "Calling propagate()" << std::endl;
#endif
  IlcsIntervalSequenceVarI* seq = _s.getImpl();
  IlcsPrecGraphCtI* pgct = seq->getResource()->getPGCt();
  if(!_initialPropagationDone){
    pgct->scanTimeNet();
    initialPropagationRankedFirst();
    initialPropagationRankedLast();
    getManager().saveValue((IlcInt*)&(_initialPropagationDone));
    _initialPropagationDone=IlcTrue;
  }
  // Check if sequence has been fixed
 
 if (_pg->isRanked()) {
    if ((0!=seq) && (!seq->isFixed())) {
      seq->computeFixedStatus();
    }
  }
  if (seq->getResource()->isClosed() && checkNewRankedFirst())
    return;
  updateUnrankedTT();
}

void TDNoOverlapCtI::display(ILOSTD(ostream) &str) const {
  if (getName())
    str << getName();
  else
    str << "TDNoOverlapCtI";
  str << "[ " << getSequence()
      << ", " << getTotalTransitionTime() << "]";
}


/////////////////////////////////////////////////////////////
// MODEL-ENGINE CONSTRAINT WRAPPING
////////////////////////////////////////////////////////////

ILOCPCONSTRAINTWRAPPER5(IloTDNoOverlap, cp,
                        IloIntervalSequenceVar, s,
                        IloIntVar, totalTT,
                        IloIntArray3, costs,
                        IloInt, sTimeStep,
                        IloInt, propagLevel){
#if defined(VERBOSE)
  std::cout << "Extracting constraint IloTDNoOverlap" << std::endl;
#endif
  // Extraction from model data.
  // Extracting sequence, it also extracts intervals of sequences
  use(cp, s);
  use(cp, totalTT);

  // Create constraint
  IlcIntervalSequenceVar cs = cp.getIntervalSequence(s);
  IlcIntVar ctotalTT = cp.getIntVar(totalTT);
  return new (cp.getHeap())
    TDNoOverlapCtI(cp, cs, ctotalTT, costs, sTimeStep, propagLevel);
}
