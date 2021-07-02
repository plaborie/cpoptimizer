// -------------------------------------------------------------- -*- C++ -*-
// File: internal.h
// --------------------------------------------------------------------------
//
// IBM Confidential
// OCO Source Materials
// 5724-Y48 5724-Y49 5724-Y54 5724-Y55 5725-A06 5725-A29
// Copyright IBM Corp. 1990, 2013
// The source code for this program is not published or otherwise
// divested of its trade secrets, irrespective of what has
// been deposited with the U.S. Copyright Office.
//
// ---------------------------------------------------------------------------

#ifndef __CP_internalH
#define __CP_internalH

#ifndef __CP_cpextH
#include <ilcp/cpext.h>
#endif

#ifndef __CP_ilcssequenceiH
#include <ilcp/ilcssequencei.h>
#endif

IlcInt IlcGetNumberOfIntervals(IlcIntervalSequenceVar seq) {
  return seq.getImpl()->getSize();
}

IlcIntervalVar IlcGetInterval(IlcIntervalSequenceVar seq, IlcInt i) {

}

#endif