#include <ilsim/ilueventstack.h>


ILCSTLBEGIN


void testEvent() {
	cout << "Event class test" << endl;
	
	IluActStartEventI* aP = new IluActStartEventI();
	aP->setTime(10);
	cout << "Event a occurs at " << aP->getTime() << endl;
	
	IluActEndEventI* bP = new IluActEndEventI();
	bP->setTime(20);
	cout << "Event b occurs at " << bP->getTime() << endl << endl;

	delete aP;
	delete bP;
}



void testEventStack() {
	cout << "EventStack class test" << endl;
	IluActStartEventI* eP1, * eP2, * eP3, * eP4, * eP5, * eP6, * eP7;
	eP1 = new IluActStartEventI();
	eP2 = new IluActStartEventI();
	eP3 = new IluActStartEventI();
	eP4 = new IluActStartEventI();
	eP5 = new IluActStartEventI();
	eP6 = new IluActStartEventI();
	eP7 = new IluActStartEventI();

	eP1->setTime(5);
	eP2->setTime(15);
	eP3->setTime(20);
	eP4->setTime(18);
	eP5->setTime(35);
	eP6->setTime(46);
	eP7->setTime(42);
	
	IluEventStackI* sP = new IluEventStackI();
	sP->push(eP2);
	sP->append(eP3);
	sP->insert(eP4);
	sP->insert(eP1);
	sP->insert(eP5);
	sP->insert(eP6);
	sP->insert(eP7);
	
	cout << "The first event of the event stack occurs at " << sP->getFirst()->getTime() << endl;
	cout << "The last event of the event stack occurs at " << sP->getLast()->getTime() << endl << endl;

	cout << "We empty the stack now." << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl;
	assert(!sP->isEmpty());
		cout << "The event at the top of the stack occurs at " << sP->pop()->getTime() << endl << endl;

	//sP->updatePreviousPointer();
}


void testEventStackIterator() {
	cout << "EventStackIterator class test" << endl;
	IluActStartEventI* eP1, * eP2, * eP3;

	eP1 = new IluActStartEventI();
	eP2 = new IluActStartEventI();
	eP3 = new IluActStartEventI();
	
	eP1->setTime(5);
	eP2->setTime(15);
	eP3->setTime(20);
	
	IluEventStackI* sP = new IluEventStackI();
	sP->push(eP1);
	sP->push(eP2);
	sP->append(eP3);

	for(IluEventStackIteratorI ite(sP) ; ite.ok() ; ++ite)
		cout << "This event occurs at " << (*ite)->getTime() << endl;
	cout << endl;
	delete eP1, eP2, eP3;
	delete sP;
}






int main() {
	try {
		testEvent();
		testEventStack();
		testEventStackIterator();
	} catch (IloException& exc) {
		cout << exc << endl;
	}
	return 0;
}