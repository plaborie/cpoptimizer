// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedSequence.cs
// --------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------

/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building five houses in different locations. The
masonry, roofing, painting, etc. must be scheduled. Some tasks must
necessarily take place before others and these requirements are
expressed through precedence constraints.

There are two workers, and each task requires a specific worker.  The
time required for the workers to travel between houses must be taken
into account.  

Moreover, there are tardiness costs associated with some tasks as well
as a cost associated with the length of time it takes to build each
house.  The objective is to minimize these costs.

------------------------------------------------------------ */

//$doc:ALL
//$doc:HEADERS
using System;
using System.IO;
using System.Collections.Generic;
using ILOG.CP;
using ILOG.Concert;
//end:HEADERS

namespace SchedSequence
{
    public class SchedSequence
    {
        //$doc:TASKS
        const int nbTasks = 10;

        const int masonry = 0;
        const int carpentry = 1;
        const int plumbing = 2;
        const int ceiling = 3;
        const int roofing = 4;
        const int painting = 5;
        const int windows = 6;
        const int facade = 7;
        const int garden = 8;
        const int moving = 9;

        static String[] taskNames = {
        "masonry  ",
        "carpentry",
        "plumbing ",
        "ceiling  ",
        "roofing  ",
        "painting ",
        "windows  ",
        "facade   ",
        "garden   ",
        "moving   "
    };

        static int[] taskDurations = {
        35,
        15,
        40,
        15,
        05,
        10,
        05,
        10,
        05,
        05,
    };
        //end:TASKS

        static CP cp = new CP();
        static INumExpr cost = cp.NumExpr();

        //$doc:TARDYCOST
        public static INumExpr tardinessCost(IIntervalVar task, int dd, double weight)
        {
            return cp.Prod(weight, cp.Max(0, cp.Diff(cp.EndOf(task), dd)));
        }
        //end:TARDYCOST

//$doc:MAKEHOUSEARG
    public static void MakeHouse (
        List<IIntervalVar> allTasks,
        List<IIntervalVar> joeTasks,
        List<IIntervalVar> jimTasks,
        List<Int32> joeLocations,
        List<Int32> jimLocations,
        int loc,
        int rd,
        int dd,
        double weight)
//end:MAKEHOUSEARG
 {

        /* CREATE THE TIME-INTERVALS. */
//$doc:TASKVAR
        String name = "H" + loc;

        IIntervalVar[] tasks = new IIntervalVar[nbTasks];
        for (int i = 0; i < nbTasks; i++ ) {
            name = "H" + loc + "-" + taskNames[i];
            tasks[i] = cp.IntervalVar(taskDurations[i], name);
            allTasks.Add(tasks[i]);
        }
//end:TASKVAR

        /* SPAN CONSTRAINT */
//$doc:HOUSEVAR
        IIntervalVar house = cp.IntervalVar(name);
        cp.Add(cp.Span(house, tasks));
//end:HOUSEVAR

        /* ADDING TEMPORAL CONSTRAINTS. */
//$doc:CSTS
        house.StartMin = rd;
        cp.Add(cp.EndBeforeStart(tasks[masonry],   tasks[carpentry]));
        cp.Add(cp.EndBeforeStart(tasks[masonry],   tasks[plumbing]));
        cp.Add(cp.EndBeforeStart(tasks[masonry],   tasks[ceiling]));
        cp.Add(cp.EndBeforeStart(tasks[carpentry], tasks[roofing]));
        cp.Add(cp.EndBeforeStart(tasks[ceiling],   tasks[painting]));
        cp.Add(cp.EndBeforeStart(tasks[roofing],   tasks[windows]));
        cp.Add(cp.EndBeforeStart(tasks[roofing],   tasks[facade]));
        cp.Add(cp.EndBeforeStart(tasks[plumbing],  tasks[facade]));
        cp.Add(cp.EndBeforeStart(tasks[roofing],   tasks[garden]));
        cp.Add(cp.EndBeforeStart(tasks[plumbing],  tasks[garden]));
        cp.Add(cp.EndBeforeStart(tasks[windows],   tasks[moving]));
        cp.Add(cp.EndBeforeStart(tasks[facade],    tasks[moving]));
        cp.Add(cp.EndBeforeStart(tasks[garden],    tasks[moving]));
        cp.Add(cp.EndBeforeStart(tasks[painting],  tasks[moving]));
//end:CSTS

        /* ALLOCATING TASKS TO WORKERS */
//$doc:WORKERTASKS
        joeTasks.Add(tasks[masonry]);
        joeLocations.Add(loc);
        joeTasks.Add(tasks[carpentry]);
        joeLocations.Add(loc);
        jimTasks.Add(tasks[plumbing]);
        jimLocations.Add(loc);
        jimTasks.Add(tasks[ceiling]);
        jimLocations.Add(loc);
        joeTasks.Add(tasks[roofing]);
        joeLocations.Add(loc);
        jimTasks.Add(tasks[painting]);
        jimLocations.Add(loc);
        jimTasks.Add(tasks[windows]);
        jimLocations.Add(loc);
        joeTasks.Add(tasks[facade]);
        joeLocations.Add(loc);
        joeTasks.Add(tasks[garden]);
        joeLocations.Add(loc);
        jimTasks.Add(tasks[moving]);
        jimLocations.Add(loc);
//end:WORKERTASKS

        /* DEFINING MINIMIZATION OBJECTIVE */
//$doc:MAKECOST
        cost = cp.Sum(cost, tardinessCost(house, dd, weight));
        cost = cp.Sum(cost, cp.LengthOf(house));
//end:MAKECOST
    }
        
        public static void Main(String[] args)
        {
        try {
//end:MAIN
            cp = new CP();
//$doc:VARS
            cost = cp.NumExpr();
            List<IIntervalVar> allTasks = new List<IIntervalVar>();
            List<IIntervalVar> joeTasks = new List<IIntervalVar>();
            List<IIntervalVar> jimTasks = new List<IIntervalVar>();

            List<Int32> joeLocations = new List<Int32>();
            List<Int32> jimLocations = new List<Int32>();
//end:VARS

//$doc:MAKEHOUSE
            MakeHouse( allTasks, joeTasks, jimTasks, joeLocations, jimLocations, 0, 0,   120, 100.0);
            MakeHouse( allTasks, joeTasks, jimTasks, joeLocations, jimLocations, 1, 0,   212, 100.0);
            MakeHouse( allTasks, joeTasks, jimTasks, joeLocations, jimLocations, 2, 151, 304, 100.0);
            MakeHouse( allTasks, joeTasks, jimTasks, joeLocations, jimLocations, 3, 59,  181, 200.0);
            MakeHouse( allTasks, joeTasks, jimTasks, joeLocations, jimLocations, 4, 243, 425, 100.0);
//end:MAKEHOUSE

//$doc:TT
            ITransitionDistance tt = cp.TransitionDistance(5);
            for (int i = 0; i < 5; ++i)
                for (int j = 0; j < 5; ++j)
                    tt.SetValue(i, j, Math.Abs(i - j));
//end:TT

//$doc:SEQVAR
            Console.WriteLine(joeTasks.Count + " et " + joeLocations.Count);

            IIntervalSequenceVar joe = cp.IntervalSequenceVar(joeTasks.ToArray(), joeLocations.ToArray(), "Joe");
            IIntervalSequenceVar jim = cp.IntervalSequenceVar(jimTasks.ToArray(), jimLocations.ToArray(), "Jim");
//end:SEQVAR

//$doc:NOOVERLAP
            cp.Add(cp.NoOverlap(joe, tt));
            cp.Add(cp.NoOverlap(jim, tt));
//end:NOOVERLAP

//$doc:OBJ
            cp.Add(cp.Minimize(cost));
//end:OBJ

//$doc:SOLVE

            cp.SetParameter(CP.IntParam.FailLimit, 50000);
            /* EXTRACTING THE MODEL AND SOLVING. */
            if (cp.Solve()) {
//end:SOLVE
//$doc:SOLN
                for (int i = 0; i < allTasks.Count; ++i)
                    Console.WriteLine(cp.GetDomain(allTasks[i]));
//end:SOLN
            } else {
                Console.WriteLine("No solution found.");
            }
            cp.PrintInformation();

        } catch (IloException e) {
            Console.WriteLine(" ERROR: " + e);
        }
        }
    }
}