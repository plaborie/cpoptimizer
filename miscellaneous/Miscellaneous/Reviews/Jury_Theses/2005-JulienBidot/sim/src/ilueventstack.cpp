// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilueventstack.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------



#include <ilsim/ilueventstack.h>


ILOSTLBEGIN


// push inserts an event at the top of a stack.
void
IluEventStackI::push(IluEventI* eventP) {
//	cout << "Insert a new event at the top of a stack" << endl;
	if(0 == _first) {
		eventP->setNext(0);
		_first = eventP;
		_last = eventP;
	}
	else {
		eventP->setNext(_first);
		_first = eventP;
	}
}


// append inserts an event at the bottom of a stack.
void
IluEventStackI::append(IluEventI* eventP) {
//	cout << "Insert a new event at the bottom of a stack" << endl;
	if(0 != _last) {
		_last->setNext(eventP);
		_last = eventP;
	}
	else {
		_first = eventP;
		_last = eventP;
	}
}


// insert an event at the good place (w.r.t. the occurence date) in the stack.
void
IluEventStackI::insert(IluEventI* newEventP) {
	IluResourceStartEventI* breakEvent = (IluResourceStartEventI*)(_first);
	IluActEndEventI* actEvent = (IluActEndEventI*)(newEventP);
	if ( (0 == _first) || (_first->getTime() > newEventP->getTime()) )
		push(newEventP);
	else if( (_first->getTime() == newEventP->getTime()) && (0 != breakEvent->getUncUnaryResource()) && (0 != actEvent->getUncActivity()) )
		push(newEventP);
	else {
		IloBool inserted = IloFalse;
		IluEventI* eventP = _first;
		while((!inserted) && (0 != eventP)) {
			if (0 != eventP->getNext()) {
				IluEventI* nextEventP = eventP->getNext();
				breakEvent = (IluResourceStartEventI*)(nextEventP);
				if( (nextEventP->getTime() == newEventP->getTime()) && (0 != breakEvent->getUncUnaryResource()) && (0 != actEvent->getUncActivity()) ) {
//					cout << "Insert a new event" << endl;
					newEventP->setNext(nextEventP);
					eventP->setNext(newEventP);
					inserted = IloTrue;
				}
				else if(nextEventP->getTime() > newEventP->getTime()) {
//					cout << "Insert a new event" << endl;
					newEventP->setNext(nextEventP);
					eventP->setNext(newEventP);
					inserted = IloTrue;
				}
			}
			else {
				append(newEventP);
				inserted = IloTrue;
			}
			eventP = eventP->getNext();
		}
	}
}


// pop removes and returns the event on the top of a stack.
IluEventI*
IluEventStackI::pop() {
//	cout << "Remove an event at the top of a stack" << endl;
	if (0 == _first) {
		cout << "The stack is empty." << endl;
		return 0;
	}
	else {
		IluEventI* event = _first;
		_first = _first->getNext();
		event->setNext(0);
		return event;
	}
}


IluEventStackIteratorI::IluEventStackIteratorI(IluEventStackI* stackP) : _stackP(stackP) {
		_currentEventP = stackP->getFirst();
}


void
IluEventStackIteratorI::operator++() {
	if (0 != _currentEventP)
		_currentEventP = _currentEventP->getNext();
}
