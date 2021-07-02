#include <ilsim/ilurandom.h>

#if defined(ILOUSESTL)
#include <fstream>
#include <stdio>
#else
#include <fstream.h>
#include <stdio.h>
#endif
#if defined(ILO_WIN32)
#include <strstrea.h>
#else
#include <strstream.h>
#endif





typedef IloNum (*functX)(IloNum);


IloNum
stepFunction(IloNum num) {
	if ((num >= 1)&&(num < 3))
		return 0.2;
	else
	if ((num >= 3)&&(num < 5))
		return 0.1;
	else
	if ((num >= 5)&&(num < 6))
		return 0.04;
	else {
		printf("This function is not defined for this value.\n");
		return IloInfinity;
	}
}

IloNum
constantFunction(IloNum num) {





return 1;

}


void
testUniformVariable(IloRandom randGene) {
	IluUniformVariableI* varP1 = new (randGene.getEnv()) IluUniformVariableI(randGene.getEnv(), randGene, 5, 10);

	for(IloInt i = 0 ; i<10 ; i++) {
		cout << varP1->getValue() << endl;
		cout << varP1->getValue(9) << endl << endl;
	}
	cout << endl;
}


void
testFunctionVariable(IloRandom randGene) {
	functX f1;
	f1 = &constantFunction;
	IluFunctionVariableI* varP1 = new (randGene.getEnv()) IluFunctionVariableI(randGene.getEnv(), randGene, 1, 11, f1);
	

/*	for(IloInt i = 0 ; i < 10 ; i++) {
		printf("Random value = %f\n", varP1->getValue());
		printf("Random value (current time is 2) = %f\n\n", varP1->getValue(2));
	}
	for(IloInt j = 0 ; j < 10 ; j++) {
		printf("Random value = %f\n", varP1->getValue());
		printf("Random value (current time is 6) = %f\n\n", varP1->getValue(6));
	}*/

	printf("Average = %f\n", varP1->getAverage());
	printf("Average (execution for 2 units)  = %f\n", varP1->getAverage(2));
	printf("Average (execution for 2.5 units) = %f\n", varP1->getAverage(2.5));

	
	printf("Value that covers 55 percent = %f\n", varP1->getCoverValue(0.55));
	printf("Value that covers 75 percent = %f\n", varP1->getCoverValue(0.75));
	printf("Value that covers 60 percent (execution for 4 units) = %f\n\n", varP1->getCoverValue(4,0.6));
	
}



void testNormalVariable(IloRandom randGene) {
	IluNormalVariableI::Init(); 
	
	IluNormalVariableI* varP1 = new (randGene.getEnv()) IluNormalVariableI(randGene.getEnv(), randGene, 5, 15, 8, 0.7);
	IluNormalVariableI* varP2 = new (randGene.getEnv()) IluNormalVariableI(randGene.getEnv(), randGene, 2,  6, 4, 0.6);
	

	for(IloInt i = 0 ; i<10 ; i++) {
		cout << "Variable 1: [" << varP1->getMin() << "," << varP1->getMax() << "]" << endl;
		cout << "Standard deviation: " << varP1->getStandardDev() << endl;
		cout << "Value 1: " << varP1->getValue() << endl;
		cout << "Value 2: " << varP1->getValue(9) << endl << endl;

		cout << "Variable 2: [" << varP2->getMin() << "," << varP2->getMax() << "]" << endl;
		cout << "Standard deviation: " << varP2->getStandardDev() << endl;
		cout << "Value 1: " << varP2->getValue() << endl;
		cout << "Value 2: " << varP2->getValue(5) << endl << endl;
	}

	IluNormalVariableI::End();
}





int main(int argc, char** argv)
{
	try {
		IloEnv env;
		IloRandom randomGenerator(env);
		
//		testUniformVariable(randomGenerator);
		testFunctionVariable(randomGenerator);

//		testNormalVariable(randomGenerator);
		
		env.end();
	} catch(IloException& exc) {
		cout << exc << endl;
	}

	return 0;
}