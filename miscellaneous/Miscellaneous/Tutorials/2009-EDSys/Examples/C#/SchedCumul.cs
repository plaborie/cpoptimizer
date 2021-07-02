// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedCumul.cpp
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

There are three workers, and each task requires a worker.  There is
also a cash budget which starts with a given balance.  Each task costs
a given amount of cash per day which must be available at the start of
the task.  A cash payment is received periodically.  The objective is
to minimize the overall completion date.

------------------------------------------------------------ */

//$doc:ALL
//$doc:HEADERS
using System;
using System.IO;
using System.Collections.Generic;
using ILOG.CP;
using ILOG.Concert;

namespace SchedCumul
{
    public class SchedCumul
    {
        //$doc:TASKS
        const int nbWorkers = 3;
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

        //$doc:CP
        static CP cp = new CP();
        //end:CP

        //$doc:MAKEHOUSEARG
        public static void MakeHouse(
                    int id,
                    int rd,
                    ICumulFunctionExpr workersUsage,
                    ICumulFunctionExpr cash,
                    List<IIntExpr> ends,
                    List<IIntervalVar> allTasks)
        {
            //end:MAKEHOUSEARG

            /* CREATE THE TIME-INTERVALS. */
            //$doc:TASKVAR
            String name;
            IIntervalVar[] tasks = new IIntervalVar[nbTasks];
            for (int i = 0; i < nbTasks; ++i)
            {
                name = "H" + id + "-" + taskNames[i];
                IIntervalVar task = cp.IntervalVar(taskDurations[i], name);
                tasks[i] = task;
                allTasks.Add(task);
                cp.Sum(workersUsage, cp.Pulse(task, 1));
                cp.Diff(cash, cp.StepAtStart(task, 200 * taskDurations[i]));
            }

            //end:TASKVAR

            /* ADDING TEMPORAL CONSTRAINTS. */
            //$doc:CSTS
            tasks[masonry].StartMin = rd;
            cp.Add(cp.EndBeforeStart(tasks[masonry], tasks[carpentry]));
            cp.Add(cp.EndBeforeStart(tasks[masonry], tasks[plumbing]));
            cp.Add(cp.EndBeforeStart(tasks[masonry], tasks[ceiling]));
            cp.Add(cp.EndBeforeStart(tasks[carpentry], tasks[roofing]));
            cp.Add(cp.EndBeforeStart(tasks[ceiling], tasks[painting]));
            cp.Add(cp.EndBeforeStart(tasks[roofing], tasks[windows]));
            cp.Add(cp.EndBeforeStart(tasks[roofing], tasks[facade]));
            cp.Add(cp.EndBeforeStart(tasks[plumbing], tasks[facade]));
            cp.Add(cp.EndBeforeStart(tasks[roofing], tasks[garden]));
            cp.Add(cp.EndBeforeStart(tasks[plumbing], tasks[garden]));
            cp.Add(cp.EndBeforeStart(tasks[windows], tasks[moving]));
            cp.Add(cp.EndBeforeStart(tasks[facade], tasks[moving]));
            cp.Add(cp.EndBeforeStart(tasks[garden], tasks[moving]));
            cp.Add(cp.EndBeforeStart(tasks[painting], tasks[moving]));
            //end:CSTS

            /* DEFINING MINIMIZATION OBJECTIVE */
            //$doc:ENDCOST
            ends.Add(cp.EndOf(tasks[moving]));
            //end:ENDCOST
        }

        //$doc:MAIN
        public static void Main(String[] args)
        {
                //end:MAIN

                //$doc:VARS
                ICumulFunctionExpr workersUsage = cp.CumulFunctionExpr();
                ICumulFunctionExpr cash = cp.CumulFunctionExpr();
                List<IIntExpr> ends = new List<IIntExpr>();
                List<IIntervalVar> allTasks = new List<IIntervalVar>();

                /* CASH PAYMENTS */
                for (int p = 0; p < 5; ++p)
                    cp.Sum(cash, cp.Step(60 * p, 30000));

                //end:VARS

                //$doc:MAKEHOUSE
                MakeHouse( 0, 31, workersUsage, cash, ends, allTasks);
                MakeHouse( 1, 0, workersUsage, cash, ends, allTasks);
                MakeHouse( 2, 90, workersUsage, cash, ends, allTasks);
                MakeHouse( 3, 120, workersUsage, cash, ends, allTasks);
                MakeHouse( 4, 90, workersUsage, cash, ends, allTasks);
                //end:MAKEHOUSE

                //$doc:CASHCST
                cp.Add(cp.Le(0, cash));
                //end:CASHCST

                //$doc:WORKERCST
                cp.Add(cp.Le(workersUsage, nbWorkers));
                //end:WORKERCST

                //$doc:OBJ
                cp.Add(cp.Minimize(cp.Max(ends.ToArray())));
                //end:OBJ

                /* EXTRACTING THE MODEL AND SOLVING. */
                //$doc:SOLVE
                cp.SetParameter(CP.IntParam.FailLimit, 10000);
                if (cp.Solve())
                {
                    //end:SOLVE
                    //$doc:SOLN
                    Console.WriteLine("Solution with objective " + cp.ObjValue + ":");
                    for (int i = 0; i < allTasks.Count; i++)
                    {
                        Console.WriteLine(cp.GetDomain(allTasks[i]));
                    }
                    int segs = cp.GetNumberOfSegments(cash);
                    for (int i = 0; i < segs; i++)
                    {
                        Console.WriteLine(
                          "Cash is " + cp.GetSegmentValue(cash, i) +
                          " from " + cp.GetSegmentStart(cash, i) +
                          " to " + (cp.GetSegmentEnd(cash, i) - 1)
                        );
                    }
                    //end:SOLN
                } 
                else
                {
                    Console.WriteLine("No Solution found.");
                }
                cp.PrintInformation();
        }
    }
}

//end:ALL
