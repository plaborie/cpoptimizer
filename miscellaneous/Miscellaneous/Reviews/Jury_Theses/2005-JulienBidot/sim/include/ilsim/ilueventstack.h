// --------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------



// This file contains IluEventStackI class and IluEventStackIteratorI class.
// IluEventStackI premits us to manage a linked list of events that is chronologically ordered.
// IluEventStackIteratorI allows us to iterate through a linked list of event.


#ifndef __SIM_ilueventstacksimH
#define __SIM_ilueventatacksimH


#include "iluevent.h"



class IluEventStackI {
	friend class EventStackIteratorI; // The class EventStackIteratorI can access to private attributes of the IluEventStackI class.
private:
	IluEventI* _first, * _last;

public:
	IluEventStackI() : _first(0), _last(0) {}
	IluEventStackI(IluEventI* first, IluEventI* last): _first(first),
													   _last(last) {}
	~IluEventStackI() {}

	void push(IluEventI*);
	void append(IluEventI*);
	void insert(IluEventI*);
	
	IluEventI* pop();

	IloBool isEmpty() { return(_first == 0); }
	
	IluEventI* getFirst() const { return _first; }
	IluEventI* getLast() const { return _last; }

	void setFirst(IluEventI* eCPointer) { _first = eCPointer; }
	void setLast(IluEventI* eCPointer) { _last = eCPointer; }
};


class IluEventStackIteratorI {
private:
	IluEventStackI* _stackP;
	IluEventI* _currentEventP;

public:
	IluEventStackIteratorI() : _stackP(0), _currentEventP(0) {}
	IluEventStackIteratorI(IluEventStackI*);

	~IluEventStackIteratorI() {}

	IloBool ok() const { return (0 != _currentEventP); }
	void operator++();
	IluEventI* operator*() { return _currentEventP; }
};

#endif
