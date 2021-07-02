// --------------------------------------------------------------------------
// Licensed Materials - Property of IBM
//
// 5725-A06 5725-A29 5724-Y48 5724-Y49 5724-Y54 5724-Y55
// Copyright IBM Corporation 1998, 2010. All Rights Reserved.
//
// Note to U.S. Government Users Restricted Rights:
// Use, duplication or disclosure restricted by GSA ADP Schedule
// Contract with IBM Corp.
// --------------------------------------------------------------------------

// Problem Description : model of manpower scheduling.
//
// A resource is associated with a ResourceType as a capabilities
// to execute a task. 
// A request is associated with a RequestType as the product of its
// execution and a due date for its earliness from its end.
// A task is a TaskType and a processing time. 
// The recipies associate a RequestType, the tasks to perform, and the 
// temporal depencies between tasks.
// A requirement is a TaskType, a ResourceType and a quantity
// as number of resources to be used by a task. Then an operation 
// is a task and a resource deduced from the types of the requirement. 
//
// Model and Constraints
//
// Each request, task, and operation is an interval variable. 
// A request spans its tasks. Operations are optional 
// and synchronized with their tasks.
// Each resource is a sequence of non-overlaping intervals.
// the sum of execution of operations for a task and a ResourceType
// is equal to the quantity of the requirement. 
//
// Redundant cumul.
//
// A ResourceType is compatible with several resources and a task actually
// requires a quantity of ResourceType. A cumul function per ResourceType 
// is defined for the tasks whose a requirement needs the ResourceType 
// for the requirement quantity.
// The cumul is limited by the number of resources of the ResourceType.
// These redundant cumuls are crucials as they enforce a stronger constraint 
// while the whole set of resources of the tasks is not chosen. 
// For more complex problems, e.g resources with several ResourceType, 
// other partitions of the resources define efficient redundant cumul. 

using CP;

// Data source for resources, requests and tasks

{string} ResourceTypes = ...;
{string} RequestTypes  = ...;
{string} TaskTypes     = ...;

tuple RequestDat {
    key int id;
    string  type;
    int     duedate;
    string  name;
};

tuple ResourceDat {
    key int id;
    string  type;
    string name;
};

tuple TaskDat {
    key int id;
    string  type;
    int     ptime;
    string  name;
};

{RequestDat}  requests  = ...;
{ResourceDat} resources = ...;
{TaskDat}     tasks     = ...;

// Data source for template recipes, dependencies and requirements

tuple Recipe {
    string request;
    string task;
};

tuple Dependency {
    string request;
    string taskb;
    string taska;
    int    delay;
};

tuple Requirement {
    string task;
    string resource;
    int    quantity;
};

{Recipe}      recipes      = ...;
{Dependency}  dependencies = ...;
{Requirement} requirements = ...;

// set of demands (task of a request) and operations (task using a resource)

tuple Demand {
    RequestDat request;
    TaskDat    task;
};

tuple Operation {
    Demand      dmd;
    ResourceDat resource;
};

{Demand} demands = {<r, t> | r in requests,  m in recipes, t in tasks : r.type == m.request && t.type == m.task};

{Operation} opers = {<d, r> | d in demands, m in requirements, r in resources : d.task.type == m.task && r.type == m.resource};

dvar interval tirequests[requests];
dvar interval titasks[d in demands] size d.task.ptime;
dvar interval tiopers[opers] optional;

dvar sequence workers[r in resources] in all(o in opers : o.resource == r) tiopers[o];

int levels[rt in ResourceTypes] = sum (r in resources : r.type == rt) 1;

cumulFunction cumuls[r in ResourceTypes] =
  sum (rc in requirements, d in demands : rc.resource == r && d.task.type == rc.task) pulse(titasks[d], rc.quantity);


minimize sum (t in requests) maxl(0, endOf(tirequests[t]) - t.duedate);
subject to {
  forall(r in requests) {
    span(tirequests[r], all(d in demands : d.request == r) titasks[d]);
    forall (d in demands : d.request == r) {
      synchronize(titasks[d], all(o in opers : o.dmd == d) tiopers[o]);
      span(titasks[d], all(o in opers : o.dmd == d) tiopers[o]);
      forall(rc in requirements : rc.task == d.task.type) {
	sum (o in opers : o.dmd == d && o.resource.type == rc.resource) presenceOf(tiopers[o]) == rc.quantity;
      }
      forall(tc in dependencies: tc.request == r.type && tc.taskb == d.task.type) {
	forall(d2 in demands : d2.request == r && tc.taska == d2.task.type) {
	  endBeforeStart(titasks[d], titasks[d2], tc.delay);    
	}
      }
    }   
  }
  forall(r in resources)
    noOverlap(workers[r]);
  forall(r in ResourceTypes)
    if (levels[r] > 1)
      cumuls[r] <= levels[r];
};
