// ---------------------------------------------------------------- -*- C++ -*-
// File: ilsim/ilupgsim.cpp
// ----------------------------------------------------------------------------
//  Copyright (C) 2003-2004 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// ----------------------------------------------------------------------------

#include <ilsim/ilupgsim.h>
#include <stdio.h>

#if defined(ILOUSESTL)
#include <fstream>
#else
#include <fstream.h>
#endif
#if defined(ILO_WIN32)
#include <strstrea.h>
#else
#include <strstream.h>
#endif

IluPrecGraphI::NodeI::NodeI(IloInt id)
	:_id    (id),
	 _start (IloIntMax),
	 _end   (IloIntMax),
	 _val   (0),
	 _dur   (0),
	 _hist  (0),
	 _out   (0),
	 _indeg (0),
	 _curindeg(0) {}

IloNum
IluPrecGraphI::NodeI::getDuration(IloNum t) const {
	IloNum dur;
	if (_start <= t ) {
		dur = _dur->getValue(t - _start);
	} else {
		dur = _dur->getValue();
	}
	return dur;
}


IluPrecGraphI::NodeI::~NodeI() {
	ArcI* it = _out;
	ArcI* del;
	while (it != 0) {
		del = it;
		it = it->getNext();
		delete del;
	}
}
	

IluPrecGraphI::IluPrecGraphI(IloInt nbNodes)
	:_nbNodes   (nbNodes),
	 _nodes     (0),
	 _work      (0),
	 _sortfirst (0),
	 _sortlast  (0) {
	_nodes = new NodeI*[nbNodes];
	for (IloInt i=0; i<nbNodes; ++i) 
		_nodes[i] = new NodeI(i);
}

IluPrecGraphI::IluPrecGraphI()
	:_nbNodes   (0),
	 _nodes     (0),
	 _work      (0),
	 _sortfirst (0),
	 _sortlast  (0) {}

IluPrecGraphI::~IluPrecGraphI() {
	assert (0 == _work);
	ItemI* it = _sortfirst;
	ItemI* del;
	while (it != 0) {
		del = it;
		it = it->getNext();
		delete del;
	}
	for (IloInt i=0; i<_nbNodes; ++i)
		delete _nodes[i];
	delete [] _nodes;
}

void
IluPrecGraphI::readJobShop(IluRandomEnv rand,
						   const char* fileName,
						   IloNum alpha) {
	// FORMAT OF THE INPUT FILE:
	// nbJobs	nbResources
	// nodeId(1)						AvgDuration(1)						...	nodeId(nbResources)			AvgDuration(nbResources)
	// ...	
	// nodeId(nbResources*(nbJobs-1))	AvgDuration(nbResources*(nbJobs-1))	...	nodeId(nbResources*nbJobs)	AvgDuration(nbResources*nbJobs)
	
	IluNormalVariableI::Init();
	IloInt nbJobs, nbResources, i, j, index, nodeId;
	ifstream file(fileName);
	int lineLimit = 1024;
	char buffer[1024];
	file.getline(buffer, lineLimit);
	istrstream line(buffer, strlen(buffer));
	line >> nbJobs >> nbResources;
	IloInt nbTotalActivities = nbJobs * nbResources;

	// CREATE nbTotalActivities + 1 NODES
	_nbNodes = nbTotalActivities + 1;
	_nodes = new NodeI*[_nbNodes];
	for (i = 0; i < _nbNodes; ++i) {
		NodeI* node = new NodeI(i);
		_nodes[i] = node;
	}
	
	// READ nbTotalActivities NODES		
	IluRandomVariableI* randomVar;
	IloNum avg, min, max, sigma;
	for (i = 0 ; i < nbJobs ; ++i) {
		file.getline(buffer, lineLimit);
		istrstream line(buffer, strlen(buffer));
		addArc(nbResources * (i + 1), 0);								// makespan
	//	printf("An arc is added between node %ld and node 0.\n", nbResources * (i + 1));

		for(j = 0 ; j < nbResources ; j++) {
			nodeId = j + nbResources * i + 1;
			line >> index >> avg;
			sigma = alpha * avg;
			min = IloMax(1.0, avg - 5 * sigma);
			max = avg + 5 * sigma;
			randomVar = new IluNormalVariableI(rand.getEnv(), rand, min, max, avg, sigma);
			setDurationVariable(nodeId, randomVar);
			if(j > 0) {
				addArc(nodeId - 1, nodeId);								// precedence
			//	printf("An arc is added between node %ld and node %ld.\n", nodeId - 1, nodeId);
			}
		}
	}
	IluNormalVariableI* randVar0 = new (rand.getEnv()) IluNormalVariableI(rand.getEnv(), rand, 0.0, 0.0, 0.0, 0.0);
	// This is a dummy variable used for computing makespan.
	setDurationVariable(0, randVar0);
}

void
IluPrecGraphI::stackSort(ItemI* it) {
	if (0 != _sortfirst) {
		_sortlast->setNext(it);
		it->setNext(0);
		_sortlast = it;
	} else {
		_sortfirst = it;
		_sortlast  = it;
	}
}

IluPrecGraphI::ItemI*
IluPrecGraphI::unstackWork() {
	ItemI* it = _work;
	_work = _work->getNext();
	return it;
}

void
IluPrecGraphI::topologicalSort() {
	assert (0 == _sortfirst);
	IloInt* inDegrees = new IloInt[_nbNodes];
	for (IloInt i=0; i<_nbNodes; ++i) {
		inDegrees[i] = _nodes[i]->getInDegree();
		_nodes[i]->setCurrentInDegree(inDegrees[i]);
		if (0 == inDegrees[i])
			stackWork(i);
	}
	while (_work != 0) {
		ItemI* it = unstackWork();
		stackSort(it);
		NodeI* node = _nodes[it->getId()];
		for (ArcI* out = node->getArcs(); 0 != out; out = out->getNext()) {
			IloInt j = out->getTarget();
			inDegrees[j]--;
			if (0 == inDegrees[j])
				stackWork(j);
		}
	}
	delete [] inDegrees;
}

void
IluPrecGraphI::setEndTime(IloInt i, IloNum t) {
	NodeI* node = _nodes[i];
	node->setEndTime(t);
	for (ArcI* out = node->getArcs(); out != 0; out = out->getNext())
		_nodes[out->getTarget()]->decrCurrentInDegree();
}

void
IluPrecGraphI::simulate(IloNum t, IloInt nb) {
	for(IloInt j = 0 ; j < nb ; j++) {
		for(IloInt i = 0; i < _nbNodes; i++)
			_nodes[i]->cleanVal(t);
		for(ItemI* it = getTopologicalSort(); it != 0; it = it->getNext()) {
			NodeI* node = _nodes[it->getId()];
			IluHistogramI* h = node->getHistogram();
			if(h != 0)
				h->setValue(node->getVal(), 1);
			if(node->getEndTime() > t ) {
				IloNum end = node->computeEnd(t);
				for (ArcI* out = node->getArcs(); out != 0; out = out->getNext())
					_nodes[out->getTarget()]->setMinVal(end);
			}
		}
	}
}