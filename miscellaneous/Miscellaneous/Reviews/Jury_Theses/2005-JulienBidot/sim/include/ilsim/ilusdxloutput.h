// --------------------------------------------------------------------------
// Copyright (C) 1999-2004 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------



/////////////////////////////////////////////////////////////////////////
//
// SDXL exporter sample
//
/////////////////////////////////////////////////////////////////////////
/*
   This file contains the implementation of a simple SDXL exporter.
   The DTD corresponding to SDXL format is available in this directory,
   in the file 'sdxl.dtd'.

   This exporter is aimed at being customized to handle other data 
   structures, or peculiarities required by a specific application, 
   whilst remaining moderately complex.

  Description:

   The main call (writeBody()) iterates over objects stored in
  a schedule (IlcScheduler object), selects candidates for output,
  and writes them into an SDXL-compatible format. 

   ILOG Scheduler objects supported are: activities, resources, resource
  constraints and precedence constraints. Since not all such objects can
  be represented using SDXL, some candidates for output are rejected using
  the virtual methods 'canBeSavedXXX(...)' as filters.

  Several helpers (writeDate(), openTag(), writeValue(...), writeName()) 
  are supplied for common XML formatting operations, and can be re-used
  easily. Some others need being specialized to a particular application, 
  if necessary. For instance, writeActivity() could be supplemented
  with support for additional attributes.

  Eventually, the main iteration over ILOG Scheduler model, writeBody(),
  can be rewritten from scratch, given a particular target application.
  Current implementation supports a majority of Getting Started examples
  output.
*/

#ifndef __SIM_ilusdxloutputsimH
#define __SIM_ilusdxloutputsimH

//#include<ilconcert/iloenv.h>
//#include<ilconcert/ilosys.h>
#include <ilsched/iloscheduler.h>


ILOSTLBEGIN



/////////////////////////////////////////////////////////////////////////
// class IloSDXLOutput
/////////////////////////////////////////////////////////////////////////

class IloSDXLOutput {
  private:
    IloInt           _level;  // for indentation
  protected:
    IloEnv            _env;
    ILCSTD(ofstream)* _out;
    void setStream(ILCSTD(ofstream) &str) { 
      _out = &str;
      _level = 0;
    }
    void unsetStream() { _out = 0; }
    ILCSTD(ofstream) &out() const { return *_out; }
    IloEnv getEnv() const { return _env; }
    void indent() { 
      for(IloInt i=0; i<_level; ++i) out() << "  ";
    }


          /* XML tag formatting helpers */

    void openTag(const char *tag) {
      indent();
      out() << "<" << tag;
    }
    void closeTag(IloBool shortTag=IloTrue) {
      if  (shortTag == IloTrue) out() << " />" << endl;
      else {
        _level++;
        out() << ">" << endl;
      }
    }
    void finishTag(const char *tag) {
      _level--;
      indent();
      out() << "</" << tag  << ">" << endl;
    }

         /* low-level data output */

      // saves a string
    void writeValue(const char *name, const char *v) const {
      out() << " " << name << "=\"" << v << "\"";
    }
      // saves numeric values
    void writeValue(const char *name, IloInt v) const {
      out() << " " << name << "=\"" << v << "\"";
    }
    void writeValue(const char *name, IloNum v) const {
      out() << " " << name << "=\"" << v << "\"";
    }

      /* If a variable is not bound, we only save its minimum value */
    IloInt getVarValue(IlcIntVar v) const {
      return v.getMin();
    }
    IloNum getVarValue(IlcFloatVar v) const {
      return v.getMin();
    }


        /* Custom data formatting */


      // customized methods for integer<->date conversion

    virtual const char *getDateFormat() const { 
      return "M-d-yyyy H:m:s";
    }
      // simple IloInt -> h:m:s converter (not robust to large date)
    virtual void writeDate(const char *name, IloInt date) const {      
      date *= 77;
      const IloInt s =  date % 60;
      const IloInt m = (date/60) % 60;
      const IloInt h = (date/(60*60)) % 24;
      const IloInt j =  date/(24*60*60);
      out() << " " << name << "=\"1-" << j+1 << "-2003 " 
            << h << ":" << m << ":" << s << "\"";
    }


        /* customized methods for high level objects (extractables) */

      /* naming utility */
    void writeName(IloExtractable e, const char *baseName) const {
      if (e.getName()!=0)
        writeValue( "name", e.getName() );
      else
        out() << " name = \"" << baseName << e.getId() << "\"";
    }

      /* filters to select elements we are interested in. */
    virtual IloBool canBeSavedAct(IlcActivity act, IlcScheduler sched);
    virtual IloBool canBeSavedRes(IlcResource ilcRes, IlcScheduler sched);
    virtual IloBool canBeSavedPct(IlcPrecedenceConstraint ilcPCt, IlcScheduler sched);
    virtual IloBool canBeSavedRct(IlcResourceConstraint ilcRCt, IlcScheduler sched);

      /* methods to perform the actual writing */
    virtual void writeActivity(IloActivity act, IloSchedulerSolution solution, IloNum ratio);
    virtual void writeResource(IloResource res);
    virtual void writeReservation(IloResourceConstraint rCt, IloSchedulerSolution solution);
    virtual void writeResourceConstraints(IlcResource ilcRes, IlcScheduler sched);
    virtual void writeConstraint(IloPrecedenceConstraint pCt);
    virtual void writePrecedenceConstraints(IloEnv env);


      /* header and trailer */
    virtual void writeProlog(const char *title,
                             const char *description);
    virtual void writeEpilog();

      /* main iteration */
    virtual void writeBody(IloSchedulerSolution, IloNum ratio);

  public:
    IloSDXLOutput(IloEnv env);
    virtual ~IloSDXLOutput();

      /* main call */

    void write(IloSchedulerSolution solution,
               ILCSTD(ofstream)& s,
			   IloNum ratio,
               const char *title=0,
               const char *description=0);
};

#endif