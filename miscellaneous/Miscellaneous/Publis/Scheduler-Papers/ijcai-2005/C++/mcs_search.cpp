#include <ilsched/iloscheduler.h>
#include <ilsched/cumlsrs.h>

ILOSTLBEGIN

#if defined FULL_SHAVING
#define USE_SHAVING
#endif

IlcInt NbMCSs  = 0;
IlcInt NbNodes = 0;
IlcInt NbRecycled = 0;

class IlcMCSI;

IlcMCSI* SelectedMCS_NEW = 0;
IlcMCSI* MCS_NEW;
IlcFloat TimeLimit;

class IlcRCTArrayI {
 private:
  IlcManager _m;
  IlcResourceConstraintI** _array;
  IlcInt _size;
  IlcInt _alloc;
 public:
  IlcRCTArrayI(IlcManager m)
    :_m(m), _array(0), _size(0), _alloc(0) {}
  ~IlcRCTArrayI(){ delete [] _array; }
  void resize(IlcInt n);
  IlcResourceConstraintI*& operator[](IlcInt i) { return _array[i]; }
  void sort();
  IlcInt getSize() const { return _size; }
};

void 
IlcRCTArrayI::resize(IlcInt n) {
  assert (_size <= _alloc);
  _size = n;
  if (_alloc < n) {
    if (_alloc == 0) {
      _alloc = 1;
    }
    while (_alloc < n)
      _alloc *= 2;
    delete [] _array;
    _array = new (_m.getGlobalHeap()) IlcResourceConstraintI*[_alloc];
  }
}

static int CompDecrCapacity(const void* fir, const void* sec) {
  const IlcCapRctI* ctfir = (*((IlcCapRctI**) fir));
  const IlcCapRctI* ctsec = (*((IlcCapRctI**) sec));
  const IlcInt firVal = ctfir->getCapacityMin();
  const IlcInt secVal = ctsec->getCapacityMin();
  return (firVal > secVal)?-1:((firVal < secVal)?1:
			       (ctfir > ctsec)?-1:((ctfir<ctsec)?1:0));
}

void 
IlcRCTArrayI::sort() {
  qsort(_array, _size, sizeof(IlcResourceConstraintI*), CompDecrCapacity);
}

class IlcMCSI {
 public:
  class MCSItemI {
  private:
    IlcResourceConstraintI* _rct;
    MCSItemI*               _next;
  public:
    MCSItemI()
      :_rct(0), _next(0) {}
    ~MCSItemI(){}
    void init(IlcResourceConstraintI* rct, MCSItemI* next) { 
      _rct = rct; _next = next; }
    void setNext(MCSItemI* next) { _next = next; }
    MCSItemI* getNext() const { return _next; }
    IlcResourceConstraintI*  getResourceConstraint() const { return _rct; }
  };

  class ResolverItemI {
  private:
    IlcResourceConstraintI* _before;
    IlcResourceConstraintI* _after;
    ResolverItemI*          _next;
    IlcFloat                _value;
  public:
    ResolverItemI()
      :_before (0),
       _after  (0),
       _next   (0),
       _value  (0.0) {}
    ~ResolverItemI();
    void init(IlcResourceConstraintI* before, 
	      IlcResourceConstraintI* after, 
	      ResolverItemI* next, 
	      IlcFloat value) { 
      _before = before; _after = after; _next = next; _value = value; }
    void setNext(ResolverItemI* next) { _next = next; }
    void setValue(IlcFloat value) { _value = value; }
    IlcFloat                getValue()  const { return _value; }
    IlcResourceConstraintI* getBefore() const { return _before; } 
    IlcResourceConstraintI* getAfter()  const { return _after; } 
    ResolverItemI*          getNext()   const { return _next; }
  };

  class ManagerI {
  private:
    IlcManager     _m;
    MCSItemI*      _recycleRCT;
    ResolverItemI* _recycleResolver;
    IlcMCSI*       _recycleMCS;
  public:
    ManagerI(IlcManager m)
      :_m(m), _recycleRCT(0), _recycleResolver(0), _recycleMCS(0) {}
    ~ManagerI(){}
    IlcManager getManager() const { return _m; }
    void recycle(MCSItemI* first, MCSItemI* last) {
      last->setNext(_recycleRCT);
      _recycleRCT = first;
    }    
    void recycle(ResolverItemI* first, ResolverItemI* last) {
      last->setNext(_recycleResolver);
      _recycleResolver = first;
    }  
    void recycle(IlcMCSI* mcs);
    IlcMCSI*       getNewMCS();
    MCSItemI*      getNewMCSItem();
    ResolverItemI* getNewResolverItem();
  };

 private:
  ManagerI*      _mngr;
  MCSItemI*      _rcts;
  MCSItemI*      _lastRct;
  ResolverItemI* _resolvers;
  ResolverItemI* _lastResolver;
  IlcInt         _nbRcts;
  IlcInt         _nbResolvers;
  IlcFloat       _value;
  IlcMCSI*       _next;

 public:
  IlcMCSI(ManagerI*);
  ~IlcMCSI();
  void init();
  IlcMCSI* makeClone();
  void setNext(IlcMCSI* next) { _next = next; }
  IlcMCSI* getNext() const { return _next; }
  MCSItemI* addResourceConstraint(IlcResourceConstraintI* rct);
  void clearResourceConstraints();
  IlcFloat getValue() const { return _value; }
  void setValue(IlcFloat value) { _value = value; }
  ResolverItemI* addResolver(IlcResourceConstraintI* before, 
			     IlcResourceConstraintI* after, 
			     IlcFloat value =0.0);
  void clearResolvers();
  IlcInt getNbResolvers() const { return _nbResolvers; }
  IlcInt getSize() const { return _nbRcts; }
  void computeResolvers();
  void sortResolversDecreasingSpace();
  MCSItemI* getResourceConstraints() const { return _rcts; }
  ResolverItemI* getResolvers() const { return _resolvers; }
};
  
IlcMCSI* 
IlcMCSI::makeClone() {
  IlcMCSI* clone = _mngr->getNewMCS();
  for (MCSItemI* mit = _rcts; mit != 0; mit = mit->getNext()) 
    clone->addResourceConstraint(mit->getResourceConstraint());
  for (ResolverItemI* rit = _resolvers; rit != 0; rit = rit->getNext()) 
    clone->addResolver(rit->getBefore(), rit->getAfter(), rit->getValue());
  return clone;
}

IlcMCSI::MCSItemI*
IlcMCSI::addResourceConstraint(IlcResourceConstraintI* rct) {
  IlcMCSI::MCSItemI* item = _mngr->getNewMCSItem();
  item->init(rct, _rcts);
  if (0 == _rcts)
    _lastRct = item;
  _rcts = item;
  _nbRcts++;
  return item;
}

void 
IlcMCSI::clearResourceConstraints() {
  if (0 != _rcts) {
    _mngr->recycle(_rcts, _lastRct);
    _rcts    = 0;
    _lastRct = 0;
    _nbRcts  = 0;
  }
}

IlcMCSI::ResolverItemI*
IlcMCSI::addResolver(IlcResourceConstraintI* before, 
		     IlcResourceConstraintI* after, 
		     IlcFloat value) {
  IlcMCSI::ResolverItemI* item = _mngr->getNewResolverItem();
  item->init(before, after, _resolvers, value);
  if (0 == _resolvers)
    _lastResolver = item;
  _resolvers = item;
  _nbResolvers++;
  return item;
}
  
void 
IlcMCSI::clearResolvers() {
  if (0 != _resolvers) {
    _mngr->recycle(_resolvers, _lastResolver);
    _resolvers    = 0;
    _lastResolver = 0;
    _nbResolvers  = 0;
  }
}

IlcMCSI::MCSItemI* IlcMCSI::ManagerI::getNewMCSItem() {
  IlcMCSI::MCSItemI* item = _recycleRCT;
  if (0 == item) {
    item = new (_m.getGlobalHeap()) MCSItemI();
  } else {
    _recycleRCT = item->getNext();
  }
  return item;
}

IlcMCSI::ResolverItemI* IlcMCSI::ManagerI::getNewResolverItem() {
  IlcMCSI::ResolverItemI* item = _recycleResolver;
  if (0 == item) {
    item = new (_m.getGlobalHeap()) ResolverItemI();
  } else {
    _recycleResolver = item->getNext();
  }
  return item;
}

IlcMCSI* IlcMCSI::ManagerI::getNewMCS() {
  IlcMCSI* item = _recycleMCS;
  if (0 == item) {
    item = new (_m.getGlobalHeap()) IlcMCSI(this);
  } else {
    item->init();
    _recycleMCS = item->getNext();
     NbRecycled--;
  }
  return item;
}

void 
IlcMCSI::ManagerI::recycle(IlcMCSI* mcs) {
  NbRecycled++;
  mcs->clearResourceConstraints();
  mcs->clearResolvers();
  mcs->setNext(_recycleMCS);
  _recycleMCS = mcs;
}

IlcMCSI::IlcMCSI(IlcMCSI::ManagerI* mngr) 
  :_mngr         (mngr),
   _rcts         (0),
   _lastRct      (0),
   _resolvers    (0),
   _lastResolver (0),
   _nbRcts       (0),
   _nbResolvers  (0),
   _value        (0.0) {
    NbMCSs++;
    //cout << "NB MCSs = " << NbMCSs << endl;
}

IlcMCSI::~IlcMCSI() {
  _mngr->recycle(this);
}

void IlcMCSI::init() {
  _rcts         = 0;
  _lastRct      = 0;
  _resolvers    = 0;
  _lastResolver = 0;
  _nbRcts       = 0;
  _nbResolvers  = 0;
  _value        = 0.0;
}

void IlcMCSI::computeResolvers() {
  assert (0 != _rcts);
  clearResolvers();
  IlcResourceConstraintI* rct0 = _rcts->getResourceConstraint();
  IlcResourceI* res = rct0->getResource();
  IlcManager m = res->getManager();
  IlcPrecGraphEvtCtI* graph = res->getBalanceConstraint();
  
  for (MCSItemI*  b = _rcts; b != 0; b = b->getNext()) {
    IlcResourceConstraintI* before = b->getResourceConstraint();
    for (MCSItemI*  a = _rcts; a != 0; a = a->getNext()) {
      if (b != a) {
	IlcResourceConstraintI* after = a->getResourceConstraint();

 	if (before->isSucceededBy(after)) {
 	  clearResolvers();
	  return;
 	}

	IlcBool add = IlcTrue;
	IlcScheduleEventI *endI, *startJ;
	if (graph!= 0) {
	  endI = before->getEndEvent();
	  startJ = after->getStartEvent();
	  if (graph->isStrictlySucceededBy(startJ, endI))
	    add = IlcFalse;
	}
	if (add) {
	  IlcActivityI* actI = before->getActivity();
	  IlcActivityI* actJ = after->getActivity();
	  IlcFloat xmin = actI->getEndMin();
	  IlcFloat ymax = actJ->getStartMax();
	  if (actI->getEndMin() > actJ->getStartMax())
	    add = IlcFalse;
	}

#if defined(MINIMIZE_MCS)
	if (add && (graph != 0)) {
	  // TEST MINIMALITY
	  for (MCSItemI*  k = _rcts; k != 0; k = k->getNext()) {
	    if ((k!=a) && (k!=b)) {
	      IlcResourceConstraintI* rctk = k->getResourceConstraint();
	      IlcScheduleEventI* startK = rctk->getStartEvent();
	      IlcScheduleEventI* endK = rctk->getEndEvent();
	      if ((graph->isSucceededBy(startJ, startK) && 
		   (!graph->isSucceededBy(startK, startJ) || (startJ < startK))) ||
		  (graph->isSucceededBy(endK, endI) && 
		   (!graph->isSucceededBy(endI,endK) || (endK < endI)))) {
		add = IlcFalse;
		break;
	      }
	    }
	  }
	}
#endif
      	if (add) {
	  addResolver(before, after, 0);
	}
      }
    }
  }
  if (0 == getNbResolvers())
    m.fail();
}

static int CompDecrSpace(const void* fir, const void* sec) {
  IlcFloat firVal = (*((IlcMCSI::ResolverItemI**) fir))->getValue();
  IlcFloat secVal = (*((IlcMCSI::ResolverItemI**) sec))->getValue();
  return (firVal > secVal)?-1:((firVal < secVal)?1:0);
}

IlcFloat ComputeSpaceEvaluator(IlcResourceConstraintI* before,
			       IlcResourceConstraintI* after);

void 
IlcMCSI::sortResolversDecreasingSpace() {
  IlcInt n = getNbResolvers();
  if (0 == n)
    return;
  IlcAny* array = _mngr->getManager().lockAnyArray(n);
  IlcInt i = 0;
  for (ResolverItemI*  r = _resolvers; r != 0; r = r->getNext(), ++i) {
    array[i] = (IlcAny)r;
    IlcResourceConstraintI* after  = r->getAfter();
    IlcResourceConstraintI* before = r->getBefore();
    r->setValue(ComputeSpaceEvaluator(before, after));
  }
  qsort(array, n, sizeof(ResolverItemI*), CompDecrSpace);
  _resolvers = (ResolverItemI*)array[0];
  for (i=0; i<n-1; ++i)
    ((ResolverItemI*)array[i])->setNext((ResolverItemI*)array[i+1]);
  _lastResolver = (ResolverItemI*)array[n-1];
  _lastResolver->setNext((ResolverItemI*)0);
}

///////////////////////////////////////////////////////////////////////////////
// COMPLETE SEARCH GOAL
///////////////////////////////////////////////////////////////////////////////

IlcFloat ComputeSpaceEvaluator(IlcResourceConstraintI* before,
			       IlcResourceConstraintI* after) {
  const IlcPrecGraphEvtCtI* g = before->getResource()->getBalanceConstraint();
  if ((g != 0) &&
      (g->isStrictlySucceededBy(after->getStartEvent(), before->getEndEvent())))
      return 0.0;

  const IlcActivityI* actbefore = before->getActivity();
  const IlcActivityI* actafter  = after->getActivity();
  const IlcInt xmin = actbefore->getEndMin();
  const IlcInt ymax = actafter->getStartMax();
  if (xmin > ymax)
    return 0.0;

  const IlcInt xmax = actbefore->getEndMax();
  const IlcInt ymin = actafter->getStartMin();
  if (xmax <= ymin)
    return 1.0;

  const IlcInt A = 2 * (ymax - ymin + 1) * (xmax - xmin + 1);
  const IlcInt LB = ymax - xmin + 1;
  IlcInt B = LB * (LB + 1);
  if (xmin < ymin) {
    const IlcInt LCmin = ymin - xmin;
    B -= LCmin * (LCmin + 1);
  }
  if (xmax < ymax) {
    const IlcInt LCmax = ymax - xmax;
    B -= LCmax * (LCmax + 1);
  }
  return (IlcFloat)B/(IlcFloat)A;
}

ILCGOAL2(SetSuccessor,
         IlcResourceConstraintI*, srct_1,
         IlcResourceConstraintI*, srct_2) {
  assert (!srct_1->isSucceededBy(srct_2));
  IloSolver s = getSolver();
  IlcScheduler sched = IlcScheduler(s);
#if defined(VERBOSE)
  cout << "\tTRYING " << srct_1->getActivity()->getName() 
       << " -> " << srct_2->getActivity()->getName() << endl;
#endif
  srct_1->setSuccessor(srct_2);
  return 0;
}

//---------------------------------------------------------------------------
//-------------------  CODE FOR GENERIC MCS SIZE ----------------------------
//---------------------------------------------------------------------------

IlcFloat computeDeltaMCSSpace(IlcMCSI* mcs, IlcFloat& maxSpace) {
  IlcFloat space = 0.0;
  IlcMCSI::MCSItemI* it0 = mcs->getResourceConstraints();
  IlcResourceConstraintI* rit0 = it0->getResourceConstraint();
  for (IlcMCSI::MCSItemI* it = it0->getNext(); 0 != it; it = it->getNext()) {
    IlcResourceConstraintI* rit = it->getResourceConstraint();
    const IlcFloat beforeSpace = ComputeSpaceEvaluator(rit,rit0);
    const IlcFloat afterSpace  = ComputeSpaceEvaluator(rit0,rit);
#if defined(USE_SHAVING)
    if (beforeSpace > maxSpace)
      maxSpace = beforeSpace;
    if (afterSpace > maxSpace)
      maxSpace = afterSpace;
#endif
    space += (beforeSpace+afterSpace);
  }
  return space;
} 

#if defined(USE_SHAVING)
IlcInt    NbShaved            = 0;
IlcFloat* ShavingHistory;
IlcFloat  ShavingThreshold    = 1;
IlcFloat  ShavingMaxPreserved = 0.5;
IlcInt    HistorySize         = 20;
IlcInt    Current             = 0;
IlcFloat  TargetShavedRatio   = 0.75;
IlcFloat  TargetShavedEpsilon = 0;
IlcFloat  CurrentAvg          = TargetShavedRatio;
IlcFloat  RelativeStep        = 100.0;

void InitShavingHistory() {
  ShavingHistory = new IlcFloat[HistorySize];
  for (IlcInt i=0; i<HistorySize; i++)
    ShavingHistory[i]=TargetShavedRatio;
   Current = 0;
}

void AddShaved() {
  IlcFloat CurrValue = ShavingHistory[Current];
  CurrentAvg = CurrentAvg + (1.0-CurrValue)/(IlcFloat)HistorySize;
  ShavingHistory[Current]=1.0;
  Current = (Current+1)%HistorySize;
  IlcFloat delta = (ShavingThreshold/RelativeStep);
  if (CurrentAvg > TargetShavedRatio + TargetShavedEpsilon)
    ShavingThreshold = ShavingThreshold + delta;
  else if (CurrentAvg < TargetShavedRatio - TargetShavedEpsilon)
    ShavingThreshold = ShavingThreshold - delta;
}

void AddNotShaved() {
  IlcFloat CurrValue = ShavingHistory[Current];
  CurrentAvg = CurrentAvg - CurrValue/(IlcFloat)HistorySize;
  ShavingHistory[Current]=0.0;
  Current = (Current+1)%HistorySize;
  IlcFloat delta = (ShavingThreshold/RelativeStep);
  if (CurrentAvg > TargetShavedRatio + TargetShavedEpsilon)
    ShavingThreshold = ShavingThreshold + delta;
  else if (CurrentAvg < TargetShavedRatio - TargetShavedEpsilon)
    ShavingThreshold = ShavingThreshold - delta;
}

void ShaveMCS(IloSolver solver, IlcMCSI* mcs) {
#if defined(VERBOSE_SHAVING)
  //cout << ShavingThreshold << endl;
#endif
  mcs->computeResolvers();
  if (mcs->getNbResolvers() == 0)
    solver.fail();
  mcs->sortResolversDecreasingSpace();
  IlcMCSI::ResolverItemI* r0 = mcs->getResolvers();
  IlcFloat thr = ShavingThreshold; 
  for (IlcMCSI::ResolverItemI* r = r0->getNext(); 0 != r; r = r->getNext())
    if (solver.solve(SetSuccessor(solver, r->getBefore(), r->getAfter()),IlcTrue)) {
      AddNotShaved();
#if defined(VERBOSE_SHAVING)
      cout << "!";
#endif
      return;
    }
  AddShaved();
  ++NbShaved;
#if defined(VERBOSE_SHAVING)
  cout << ".";
#endif
  r0->getBefore()->setSuccessor(r0->getAfter());
}
#endif // USE_SHAVING

#if defined(USE_SHAVING)
// ****** FULL SHAVING FONCTIONALITY

void RecursiveShaveMCS(IlcDiscreteResource res,
		       IlcMCSI* mcs,
		       IlcInt q,
		       IlcFloat& maxSpace,
		       IlcFloat space) {
  IloSolver solver = res.getSchedule().getSolver();
  if (solver.getTime() > TimeLimit) {
    // Time out in MCS selection
    mcs->~IlcMCSI();
    return;
  }

  if (q > res.getCapacity()) {
    IlcFloat shavingOpp = space - maxSpace;
    if (ShavingThreshold > shavingOpp)
      ShaveMCS(solver, mcs);
    mcs->~IlcMCSI();
    return;
  }

  if (q == 0) {
    IlcRCTArrayI* rctarray = (IlcRCTArrayI*)res.getObject();
    for (IlcInt i=0; i<rctarray->getSize(); ++i) {
      IlcMCSI* clone = mcs->makeClone();
      IlcResourceConstraintI* rct = rctarray->operator[](i);
      clone->addResourceConstraint(rct);
      IlcFloat maxSpaceClone = maxSpace;
      RecursiveShaveMCS(res, clone, 
			 ((IlcCapRctI*)rct)->getCapacityMin(), 
			 maxSpaceClone, 0);
    }
    mcs->~IlcMCSI();
  } else {
    IlcMCSI::MCSItemI* it0 = mcs->getResourceConstraints();
    assert (0 != it0);
    IlcResourceConstraint rct0 = it0->getResourceConstraint();
    IlcRCTArrayI* rct0array = (IlcRCTArrayI*)rct0.getObject();

    it0 = it0->getNext();
    for (IlcInt i=0; i<rct0array->getSize(); ++i) {
      IlcBool consider = IlcTrue;
      IlcResourceConstraintI* rct = rct0array->operator[](i);
      for (IlcMCSI::MCSItemI* it = it0; 0 != it; it = it->getNext()) {
	IlcResourceConstraintI* rcti = it->getResourceConstraint();
	if ((rct->isSucceededBy(rcti)) ||
	    (rcti->isSucceededBy(rct))) {
	  consider = IlcFalse;
	  break;
	}
      }
      if (consider) {
	IlcMCSI* clone = mcs->makeClone();
	IlcMCSI::MCSItemI* id = clone->addResourceConstraint(rct);
	IlcFloat maxSpaceClone = maxSpace;
	IlcFloat deltaSpace = computeDeltaMCSSpace(clone, maxSpaceClone);
	RecursiveShaveMCS(res, clone, 
			  q+((IlcCapRctI*)rct)->getCapacityMin(), 
			  maxSpaceClone,
			  space + deltaSpace);
      }
    }
    mcs->~IlcMCSI();
  }
}

void ShaveMCS(IlcDiscreteResource res) {
  IlcInt n=0;
  for (IlcResourceConstraintIterator ite1(res); ite1.ok(); ++ite1) {
    ++n;
    IlcResourceConstraint rct_1 = *ite1;
    IlcCapRctI* rct_1i = (IlcCapRctI*)rct_1.getImpl();
    const IlcInt cmini = rct_1i->getCapacityMin();
    IlcInt u = 0;
    for (IlcResourceConstraintIterator iteu1(rct_1, IlcUnranked); iteu1.ok(); ++iteu1) {
      if ((*iteu1).getImpl() > rct_1i)
	++u;
    }
    IlcRCTArrayI* rct_1array = (IlcRCTArrayI*)rct_1.getObject();
    rct_1array->resize(u);
    u = 0;
    for (IlcResourceConstraintIterator iteu2(rct_1, IlcUnranked); iteu2.ok(); ++iteu2) {
      IlcCapRctI* urk = (IlcCapRctI*)((*iteu2).getImpl());
      const IlcInt cminu = urk->getCapacityMin();
      if ((cminu < cmini) || ((cminu == cmini) && (urk < rct_1i))) {
	rct_1array->operator[](u)=urk;
	++u;
      }
    }
    rct_1array->sort();
  }
  IlcRCTArrayI* rctarray = (IlcRCTArrayI*)res.getObject();
  rctarray->resize(n);
  n=0;
  for (IlcResourceConstraintIterator ite2(res); ite2.ok(); ++ite2) {
    rctarray->operator[](n)=(*ite2).getImpl();
    ++n;
  }
  rctarray->sort();

  IlcMCSI* mcs = MCS_NEW->makeClone();
  IlcFloat maxSpace = 0;
  RecursiveShaveMCS(res, mcs, 0, maxSpace, 0);
}

IlcInt ShaveMCS(IlcScheduler sched) {
  NbShaved = 0;
  for (IlcResourceIterator ite(sched); ite.ok(); ++ite) {
    IlcResource res = *ite;
    if (res.isDiscreteResource()) {
      IlcDiscreteResource dres = res;
      ShaveMCS(dres);
    }
  }
  return NbShaved;
}
 
void FullShaving(IlcScheduler sched) {
  IlcInt totalNbShaved = 0;
  IlcInt nb = ShaveMCS(sched);
  while (nb > 0) {
    totalNbShaved += nb;
    nb = ShaveMCS(sched);
  }
  //#if defined(VERBOSE_SHAVING)
  cout << "TOTAL NB SHAVED = " << totalNbShaved;
  //#endif
}
#endif

void RecursiveSelectMCS(IlcDiscreteResource res,
			IlcDiscreteResource& selres,
			IlcMCSI*& selmcs,
			IlcMCSI* mcs,
			IlcInt q,
			IlcFloat& minSpace,
			IlcFloat& maxSpace,
			IlcFloat space) {
  IloSolver solver = res.getSchedule().getSolver();
  if (solver.getTime() > TimeLimit) {
    // Time out in MCS selection
    mcs->~IlcMCSI();
    return;
  }

#if defined(USE_SHAVING)
#if !defined(FULL_SHAVING)
  if (q > res.getCapacity()) {
    IlcFloat shavingOpp = space - maxSpace;
    if ((ShavingThreshold > shavingOpp) && 
	(maxSpace < ShavingMaxPreserved))  {
      ShaveMCS(solver, mcs);
    }
  }
#endif
#endif

  if (space >= minSpace) {
    mcs->~IlcMCSI();
    return;
  }

  if (q > res.getCapacity()) {
    
    if (space < minSpace) {
      selres = res;
      minSpace = space;
      if (selmcs != 0)
	selmcs->~IlcMCSI();
      selmcs = mcs;
    } else {
      mcs->~IlcMCSI();
    }
    return;
  }
  if (q == 0) {
    IlcRCTArrayI* rctarray = (IlcRCTArrayI*)res.getObject();
    for (IlcInt i=0; i<rctarray->getSize(); ++i) {
      IlcMCSI* clone = mcs->makeClone();
      IlcResourceConstraintI* rct = rctarray->operator[](i);
      clone->addResourceConstraint(rct);
      IlcFloat maxSpaceClone = maxSpace;
      RecursiveSelectMCS(res, selres, selmcs, clone, 
			 ((IlcCapRctI*)rct)->getCapacityMin(), 
			 minSpace, maxSpaceClone, 0);
    }
    mcs->~IlcMCSI();
  } else {
    IlcMCSI::MCSItemI* it0 = mcs->getResourceConstraints();
    assert (0 != it0);
    IlcResourceConstraint rct0 = it0->getResourceConstraint();
    IlcRCTArrayI* rct0array = (IlcRCTArrayI*)rct0.getObject();

    it0 = it0->getNext();
    for (IlcInt i=0; i<rct0array->getSize(); ++i) {
      IlcBool consider = IlcTrue;
      IlcResourceConstraintI* rct = rct0array->operator[](i);
      for (IlcMCSI::MCSItemI* it = it0; 0 != it; it = it->getNext()) {
	IlcResourceConstraintI* rcti = it->getResourceConstraint();
	if ((rct->isSucceededBy(rcti)) ||
	    (rcti->isSucceededBy(rct))) {
	  consider = IlcFalse;
	  break;
	}
      }
      if (consider) {
	IlcMCSI* clone = mcs->makeClone();
	IlcMCSI::MCSItemI* id = clone->addResourceConstraint(rct);
	IlcFloat maxSpaceClone = maxSpace;
	IlcFloat deltaSpace = computeDeltaMCSSpace(clone, maxSpaceClone);
	RecursiveSelectMCS(res, selres, selmcs, clone, 
			   q+((IlcCapRctI*)rct)->getCapacityMin(), 
			   minSpace, maxSpaceClone,
			   space + deltaSpace);
      }
    }
    mcs->~IlcMCSI();
  }
}

void SelectMCS(IlcDiscreteResource res,
	       IlcFloat& minSpace,
	       IlcDiscreteResource& selres,
	       IlcMCSI*& selmcs) {
  IlcInt n=0;
  for (IlcResourceConstraintIterator ite1(res); ite1.ok(); ++ite1) {
    ++n;
    IlcResourceConstraint rct_1 = *ite1;
    IlcResourceConstraintI* rct_1i = rct_1.getImpl();
    IlcInt u = 0;
    for (IlcResourceConstraintIterator iteu1(rct_1, IlcUnranked); iteu1.ok(); ++iteu1) {
      if ((*iteu1).getImpl() > rct_1i)
	++u;
    }
    IlcRCTArrayI* rct_1array = (IlcRCTArrayI*)rct_1.getObject();
    rct_1array->resize(u);
    u = 0;
    for (IlcResourceConstraintIterator iteu2(rct_1, IlcUnranked); iteu2.ok(); ++iteu2) {
      IlcResourceConstraintI* urk = (*iteu2).getImpl();
      if (urk > rct_1i) {
	rct_1array->operator[](u)=urk;
	++u;
      }
    }
    rct_1array->sort();
  }
  IlcRCTArrayI* rctarray = (IlcRCTArrayI*)res.getObject();
  rctarray->resize(n);
  n=0;
  for (IlcResourceConstraintIterator ite2(res); ite2.ok(); ++ite2) {
    rctarray->operator[](n)=(*ite2).getImpl();
    ++n;
  }
  rctarray->sort();

  IlcMCSI* mcs = MCS_NEW->makeClone();
  IlcFloat maxSpace = 0;
  RecursiveSelectMCS(res, selres, selmcs, mcs, 0, minSpace, maxSpace, 0);
}

IlcBool SelectMCS(IlcScheduler sched,
		  IlcFloat& minSpace,
		  IlcDiscreteResource& selres,
		  IlcMCSI*& selmcs) {
  NbNodes++;
#if defined(FULL_SHAVING)
  FullShaving(sched);
#endif
  for (IlcResourceIterator ite(sched); ite.ok(); ++ite) {
    IlcResource res = *ite;
    if (res.isDiscreteResource()) {
      IlcDiscreteResource dres = res;
      SelectMCS(dres, minSpace, selres, selmcs);
    }
  }
  return (minSpace <= IloIntMax/2);
}

ILCGOAL2(NewRecursiveApplyResolver,
	 IlcMCSI*, mcs,
	 IlcMCSI::ResolverItemI*, resolver) {
  IlcManager m = getManager();
  if (0 == resolver)
    return 0;
  if (0 == resolver->getNext())
    return SetSuccessor(m, resolver->getBefore(), resolver->getAfter());
  else
    return IlcOr(SetSuccessor(m, resolver->getBefore(), resolver->getAfter()),
		 NewRecursiveApplyResolver(m,  mcs, resolver->getNext()));
}

ILCGOAL1(NewIlcSolveMCS, IlcMCSI*, mcs) {
#if defined(VERBOSE_SHAVING)
#if defined(USE_SHAVING)
  cout << endl;
#endif
#endif
  IlcManager m = getManager();
  mcs->computeResolvers();
  mcs->sortResolversDecreasingSpace();
#if defined(VERBOSE)
  for (IlcMCSI::ResolverItemI* rit = SelectedMCS_NEW->getResolvers(); 0 != rit; rit = rit->getNext()) {
    cout << "RESOLVER : " 
	 << (IlcResourceConstraint)rit->getBefore() << " -> " 
	 << (IlcResourceConstraint)rit->getAfter() << "\t space= " << rit->getValue() << endl;
  }
#endif
  return NewRecursiveApplyResolver(m, mcs, mcs->getResolvers());
}

ILCGOAL0(NewIlcDoSolveProblem) {
  IloSolver s = getSolver();
  IlcScheduler sched = IlcScheduler(s);
  IlcManager m = getManager();
  IlcDiscreteResource sres;
  IlcFloat minSpace = IloIntMax;
  SelectedMCS_NEW = 0;
  if (SelectMCS(sched, minSpace, sres, SelectedMCS_NEW)) {
#if defined(VERBOSE)
    cout << "SELECTED MCS:" << endl;
    for (IlcMCSI::MCSItemI* it = SelectedMCS_NEW->getResourceConstraints(); 0 != it; it = it->getNext()) {
      cout << "\t" << IlcActivity(it->getResourceConstraint()->getActivity()) << endl;
    }
    cout << "\t minSpace = " << minSpace << endl;
#endif
    return (IlcAnd(NewIlcSolveMCS(m, SelectedMCS_NEW), this));
  } else {
#if defined(VERBOSE)
    cout << "NOTHING SELECTED" << endl;
#endif
    return 0;
  }
}

ILCGOAL1(StoreSolution, IloSchedulerSolution, sol) {
  IloSolver s = getSolver();
  IlcScheduler sched(s);
  sol.store(sched);
  return 0;
}

ILCGOAL1(NewIlcSolveProblem, IloSchedulerSolution, sol) {
  IloSolver s = getSolver();
  IlcManager m = s;
  IlcScheduler sched(s);
  for (IlcResourceIterator ite(sched); ite.ok(); ++ite) {
    IlcDiscreteResource res = (IlcDiscreteResource)*ite;
    IlcRCTArrayI* rctarray = new (m.getGlobalHeap()) IlcRCTArrayI(m);
    res.setObject((IlcAny)rctarray);
    for (IlcResourceConstraintIterator itect(res); itect.ok(); ++itect) {
      IlcResourceConstraint rct = *itect;
      IlcRCTArrayI* rct_1array = new (m.getGlobalHeap()) IlcRCTArrayI(m);
      rct.setObject((IlcAny)rct_1array);
    }
    
#if !defined(NO_BALANCE)
    s.add(res.makeBalanceConstraint());
#endif

#if defined(VERBOSE)
    cout << "RESOURCE " << *ite << endl;
    for (IlcResourceConstraintIterator ite1(*ite); ite1.ok(); ++ite1) {
      cout << "\t" << (*ite1).getActivity() << endl;
    }
#endif
  }
  return IlcAnd(IlcAnd(NewIlcDoSolveProblem(s), IlcSetTimes(IlcScheduler(s))), StoreSolution(s, sol));
}

ILOCPGOAL1(NewSolveProblem, IloSchedulerSolution, sol) {
  IloSolver s = getSolver();
  return NewIlcSolveProblem(s, sol);
}

IlcBool SolveProblem(IloModel model, 
		     IloIntVar makespan, 
		     IloSchedulerSolution sol,
		     IloInt ub, 
		     IlcFloat timeLimit,
		     IlcBool& solution, IlcFloat& time, IlcInt& nbFails) {
  makespan.setUb(ub);

  IloSolver solver(model);
  solver.setTimeLimit(timeLimit);
  IloEnv env = model.getEnv();

  IloGoal goal = NewSolveProblem(env, sol);

  IlcMCSI::ManagerI* mng = new IlcMCSI::ManagerI(solver);
  MCS_NEW = new IlcMCSI(mng);
  TimeLimit = solver.getTime() + timeLimit;

  IlcBool solved = solver.solve(goal);
  if (solver.getTime() >= timeLimit) {
    time     = timeLimit;
    nbFails  = 0;
    solution = IlcTrue;
    return IlcFalse;
  }
  if (solved)
    solution = IlcTrue;
  else
    solution = IlcFalse;

  time = solver.getTime();
  nbFails = solver.getNumberOfFails();
  
#if defined(VERBOSE)
  solver.printInformation();
#endif

  solver.end();
  return IlcTrue;
}
