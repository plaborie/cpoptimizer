
#include <ilcp/cpext.h>

class NoOverlapCtI : public IlcConstraintI{
private:
//intervals 
  IlcIntervalVar _itv1;
  IlcIntervalVar _itv2;
public:
  NoOverlapCtI(IloCP cp,
        IlcIntervalVar itv1,
        IlcIntervalVar itv2);
  IlcIntervalVar getItv1() const {
    return _itv1;
  }
  IlcIntervalVar getItv2() const {
    return _itv2;
  }
// constraint interface
  virtual void post();
  virtual void propagate();
  virtual void display(ILOSTD(ostream) &) const;
  
// propagation demons
  void propagate1To2() {
  if(_itv2.getEndMin() > _itv1.getStartMax())
    _itv2.setStartMin(_itv1.getEndMin());
  if(_itv1.getEndMin() > _itv2.getStartMax())
    _itv2.setEndMax(_itv1.getStartMax());
  }
  void propagate2To1() {
  if(_itv1.getEndMin() > _itv2.getStartMax())
    _itv1.setStartMin(_itv2.getEndMin());
  if(_itv2.getEndMin() > _itv1.getStartMax())
    _itv1.setEndMax(_itv2.getStartMax());
  }
};

NoOverlapCtI::NoOverlapCtI(IloCP cp,
 IlcIntervalVar itv1,
 IlcIntervalVar itv2)
  :IlcConstraintI(cp)
  ,_itv1(itv1)
  ,_itv2(itv2)
{}


ILCCTDEMON0(CallPropagate1To2,
  NoOverlapCtI, propagate1To2)
ILCCTDEMON0(CallPropagate2To1,
  NoOverlapCtI, propagate2To1)


/////////////////////////////////////////////////////////////
//
//
// Members Interface of constraint class
//
//
////////////////////////////////////////////////////////////

void NoOverlapCtI::post() {
  IloCP cp = getCP();
  IlcIntervalVar itv1 = getItv1();
  IlcIntervalVar itv2 = getItv2();
  itv1.whenIntervalDomain(CallPropagate1To2(cp, this));
  itv2.whenIntervalDomain(CallPropagate2To1(cp, this));
}

void NoOverlapCtI::propagate() {
  NoOverlapCtI::propagate1To2();
  NoOverlapCtI::propagate2To1();
}

void NoOverlapCtI::display(ILOSTD(ostream) &str) const {
  if (getName())
    str << getName();
  else
    str << "NoOverlapCtI";
  str << "[ " << getItv1()
      << ", " << getItv2() << "]";
}


/////////////////////////////////////////////////////////////
//
//
// MODEL-ENGINE CONSTRAINT WRAPPING
//
//
////////////////////////////////////////////////////////////

ILOCPCONSTRAINTWRAPPER2(IloNoOverlap2, cp,
                        IloIntervalVar, itv1,
                        IloIntervalVar, itv2) {
  // Extraction from model data.
  // Extracting sequence, it also extract intervals of sequences
  use(cp, itv1);
  use(cp, itv2);
  
  // Create constraint
  IlcIntervalVar citv1 = cp.getInterval(itv1);
  IlcIntervalVar citv2 = cp.getInterval(itv2);
  return new (cp.getHeap())
    NoOverlapCtI(cp, citv1, citv2);
}

class FileError: public IloException {
public:
  FileError() : IloException("Cannot open data file") {}
};

int main(int argc, const char* argv[]){
  IloEnv env;
  try {
    const char* filename = "../../../examples/team/TDTSP/data/noOverlap.data";
    IloInt failLimit = 10000;
    if (argc > 1)
      filename = argv[1];
    if (argc > 2)
      failLimit = atoi(argv[2]);
    std::ifstream file(filename);
    if (!file){
      env.out() << "usage: " << argv[0] << " <file> <failLimit>" << std::endl;
      throw FileError();
    }

    IloModel model(env);
    char name[128];
    IloInt nbIntervals;
    file >> nbIntervals;
    IloIntervalVarArray intervals(env, nbIntervals);
    IloIntExprArray ends(env);
    for (IloInt i = 0; i < nbIntervals; i++) {
    IloInt d, s, e;
    file >> d  >> s >> e;
    intervals[i]= IloIntervalVar(env, d);
        intervals[i].setStartMin(s);
    intervals[i].setEndMax(e);
  }
    for (IloInt i = 0; i < nbIntervals; i++) {
    for (IloInt j = 0; j < nbIntervals; j++) {
      if(i!=j) model.add(IloNoOverlap2(env, intervals[i], intervals[j]));
    }
  }
    IloCP cp(model);
    cp.setParameter(IloCP::FailLimit, failLimit);
    cp.out() << "Instance \t: " << filename << std::endl;
    if (cp.solve()) {
    for(int i=0; i< nbIntervals; i++)
      cp.out() << "Start: " << cp.getStart(intervals[i]) << " " << cp.getEnd(intervals[i]) << std::endl;
    } else {
      cp.out() << "No solution found."  << std::endl;
    }
  } catch(IloException& e){
    env.out() << " ERROR: " << e << std::endl;
  }
  env.end();
  return 0;
}
