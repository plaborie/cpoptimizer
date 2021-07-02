#include <ilsched/iloscheduler.h>


int main() {
  IloEnv env;
  IloSolver cp(env);
  IlcScheduler sched(cp);

  IloEvaluator<IlcActivity> smin = IlcActivityStartMinEvaluator(cp);
  IloEvaluator<IlcActivity> emax = IlcActivityStartMinEvaluator(cp);
  IloEvaluator<IlcActivity> dur  = IlcActivityStartMinEvaluator(cp);
  IloEvaluator<IlcActivity> slack = emax - smin - dur;

  IloSelector<IlcActivity,IlcSchedule> sel = 
    IloBestSelector<IlcActivity,IlcSchedule> (IloComposeLexical(smin, slack));

  IlcGoal strategy = IlcSetTimes(sched, sel);

}
