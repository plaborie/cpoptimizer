#include <ilsim/iluactivity.h>
#include <ilsim/ilumodel.h>
#include <strstrea.h>




ILCSTLBEGIN





///////////////////////////////////////////////////////////////////////////////
//
// PRINTING OF SOLUTIONS
//
///////////////////////////////////////////////////////////////////////////////

void
PrintSolution(IlcScheduler scheduler)
{
// iteration with solution iterator
  for (IlcActivityIterator ite(scheduler); ite.ok(); ++ite) {
    IlcActivity activity = *ite;
    cout << activity.getName() << " [ ";
	
	if(activity.getStartMin() < activity.getStartMax())
		cout << activity.getStartMin() << ".." << activity.getStartMax();
	else
		cout << activity.getStartMin();
	cout << " -- ";
	
	if(activity.getDurationMin() < activity.getDurationMax())
		cout << activity.getDurationMin() << ".." << activity.getDurationMax();
	else
		cout << activity.getDurationMin();
	cout << " --> ";

	if(activity.getEndMin() < activity.getEndMax())
		cout << activity.getEndMin() << ".." << activity.getEndMax();
	else
		cout << activity.getEndMin();
	cout << " ]" << endl;

  }
}




///////////////////////////////////////////////////////////////////////////////
//
// MAIN FUNCTION
//
///////////////////////////////////////////////////////////////////////////////


int main(int argc, char** argv)
{
	try {  
		IloInt seed = 9;
		if (argc > 1)
			seed = atoi(argv[1]);
		IloNum alpha = 0.2;
		if (argc > 2)
			alpha = atoi(argv[2]);

		IloEnv env;  // Environment creation

		IloModel model(env); // Model creation

		IloRandom randomGenerator(env); // Random numbers generator creation
		randomGenerator.reSeed(seed);

  
  /* CREATE THE ACTIVITIES. */
		IloActivity act1(env, 10,"Activity 1");
		IloActivity act2(env, 15,"Activity 2");

/*CREATE THE RANDOM VARIABLES. */
		IluNormalVariableI::Init();
		
		IloNum average = act1.getDurationMax();
		IloNum min = IloMax(0, average - 5*alpha*average);
		IloNum max = average + 5*alpha*average;
		IloNum stdDev = alpha * average;
		IluNormalVariableI* d1 = new (env) IluNormalVariableI(randomGenerator, min, max, act1.getDurationMax(), stdDev);
		
		average = act2.getDurationMax();
		min = IloMax(0, average - 5*alpha*average);
		max = average + 5*alpha*average;
		stdDev = alpha * average;
		IluNormalVariableI* d2 = new (env) IluNormalVariableI(randomGenerator, min, max, act2.getDurationMax(), stdDev);
		
		
/* CREATE THE IMPRECISE ACTIVITIES. */
		IluActivityI* uncAct1 = new (env) IluActivityI(act1, d1);
		IluActivityI* uncAct2 = new (env) IluActivityI(act2, d2);

// ADD THE TEMPORAL CONSTRAINTS AND THUS THE ACTIVITIES TO THE MODEL.
		model.add(act1.startsAt(20));
		model.add(act2.startsAt(30));


// CREATE THE UNCERTAIN MODEL FROM THE MODEL.
		IluModelI* uncModel = new (env) IluModelI(model);

// Execution simulation of this little schedule.
		//execution(uncModel);
		

		env.out() << endl;


		IluNormalVariableI::End();
		env.end();

	} catch(IloException& exc) {
		cout << exc << endl;
	}

	return 0;
}
