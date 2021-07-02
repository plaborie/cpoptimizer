#include <ilsim/ilutimeeventmanager.h>
#include <stdlib.h>


ILCSTLBEGIN



void testTimeEventManager() {
	IloEnv env;
	IloRandom startTimeGenerator(env);
	
	// controllable event list creation (given by the controller)
	env.out() << "Controllable event list creation" << endl << endl;
	IluActStartEventI* eventArray[100];
	IloInt i;
	for(i = 0 ; i < 100 ; i++) {
		eventArray[i] = new (env) IluActStartEventI();
		eventArray[i]->setTime(startTimeGenerator.getInt(20)+i*10);
	}
	
	// printing of activity start times
/*	cout << endl << "Printing of activity start times" << endl;
	for(i = 0 ; i < 100 ; i++)
		cout << eventArray[i]->getTime() << endl;*/
	
	
	// dynamic scheduler creation
	IluDynamicScheduler* sim = new (env) IluDynamicScheduler();


	// time event manager creation
	cout << "Time event manager creation" << endl << endl;
	IluTimeEventManagerI* tEMgrP = new (env) IluTimeEventManagerI(sim);

	tEMgrP->addEvents((IluEventI**)eventArray, 100);


	while(tEMgrP->executableEvents())
		tEMgrP->executeNext();
	env.end();
}







int main() {
	try {
		testTimeEventManager();
	
	} catch (IloException& exc) {
		cout << exc << endl;
	}
	return 0;
}