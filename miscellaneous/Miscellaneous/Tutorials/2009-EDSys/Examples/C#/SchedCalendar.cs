/* ------------------------------------------------------------

// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedCalendar.cs
// --------------------------------------------------------------------------
// Copyright (C) 1990-2007 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------
*/

/* ------------------------------------------------------------

Problem Description
-------------------

This is a problem of building five houses. The masonry, roofing,
painting, etc. must be scheduled.  Some tasks must necessarily take
place before others and these requirements are expressed through
precedence constraints.

There are two workers and each task requires a specific worker.  The
worker has a calendar of days off that must be taken into account. The
objective is to minimize the overall completion date.

------------------------------------------------------------ */

//$doc:ALL
//$doc:HEADERS
using System;
using System.IO;
using System.Collections.Generic;
using ILOG.CP;
using ILOG.Concert;

namespace SchedCalendar
{
    public class SchedCalendar
    {
        //end:HEADERS

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

        //$doc:MAKEHOUSEARG
        public static void MakeHouse(
            CP cp,
            int id,
            List<IIntExpr> ends,
            List<IIntervalVar> allTasks,
            List<IIntervalVar> joeTasks,
            List<IIntervalVar> jimTasks
            )
        //end:MAKEHOUSEARG
        {
            /// CREATE THE TIME-INTERVALS. ///
            //$doc:TASKVAR
            String name;
            IIntervalVar[] tasks = new IIntervalVar[nbTasks];
            for (int i = 0; i < nbTasks; i++)
            {
                name = "H" + id + "-" + taskNames[i];
                tasks[i] = cp.IntervalVar(taskDurations[i], name);
                allTasks.Add(tasks[i]);
            }
            //end:TASKVAR

            /// ADDING PRECEDENCE CONSTRAINTS. ///
            //$doc:CSTS
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

            /// ADDING WORKER TASKS. ///
            //$doc:WORKERTASKS
            joeTasks.Add(tasks[masonry]);
            joeTasks.Add(tasks[carpentry]);
            jimTasks.Add(tasks[plumbing]);
            jimTasks.Add(tasks[ceiling]);
            joeTasks.Add(tasks[roofing]);
            jimTasks.Add(tasks[painting]);
            jimTasks.Add(tasks[windows]);
            joeTasks.Add(tasks[facade]);
            joeTasks.Add(tasks[garden]);
            jimTasks.Add(tasks[moving]);
            //end:WORKERTASKS

            /// DEFINING MINIMIZATION OBJECTIVE ///
            //$doc:ENDCOST
            ends.Add(cp.EndOf(tasks[moving]));
            //end:ENDCOST
        }

        //$doc:MAIN
        public static void Main(String[] args)
        {
            //$doc:CP
            CP cp = new CP();
            //end:CP
            //end:MAIN

            //$doc:VARS
            int nbHouses = 5;
            List<IIntExpr> ends = new List<IIntExpr>();
            List<IIntervalVar> allTasks = new List<IIntervalVar>();
            List<IIntervalVar> joeTasks = new List<IIntervalVar>();
            List<IIntervalVar> jimTasks = new List<IIntervalVar>();
            //end:VARS

            //$doc:MAKEHOUSE
            for (int h = 0; h < nbHouses; h++)
            {
                MakeHouse(cp, h, ends, allTasks, joeTasks, jimTasks);
            }
            //end:MAKEHOUSE

            //$doc:NOOVERLAP
            IIntervalVar[] test = joeTasks.ToArray();
            IConstraint cont = cp.NoOverlap(test);

            cp.Add(cp.NoOverlap(joeTasks.ToArray()));
            cp.Add(cp.NoOverlap(jimTasks.ToArray()));
            //end:NOOVERLAP

            //$doc:EFFFN
            INumToNumStepFunction joeCalendar = cp.NumToNumStepFunction(0, 2 * 365, 1);
            INumToNumStepFunction jimCalendar = cp.NumToNumStepFunction(0, 2 * 365, 1);
            //end:EFFFN

            //$doc:FNVAL
            // Week ends
            for (int w = 0; w < 2 * 52; w++)
            {
                joeCalendar.SetValue(5 + (7 * w), 7 + (7 * w), 0);
                jimCalendar.SetValue(5 + (7 * w), 7 + (7 * w), 0);
            }
            // Holidays
            joeCalendar.SetValue(5, 12, 0);
            joeCalendar.SetValue(124, 131, 0);
            joeCalendar.SetValue(215, 236, 0);
            joeCalendar.SetValue(369, 376, 0);
            joeCalendar.SetValue(495, 502, 0);
            joeCalendar.SetValue(579, 600, 0);
            jimCalendar.SetValue(26, 40, 0);
            jimCalendar.SetValue(201, 225, 0);
            jimCalendar.SetValue(306, 313, 0);
            jimCalendar.SetValue(397, 411, 0);
            jimCalendar.SetValue(565, 579, 0);
            //end:FNVAL

            //$doc:FORBID
            for (int i = 0; i < joeTasks.Count; i++)
            {
                joeTasks[i].SetIntensity(joeCalendar, 1);
                cp.Add(cp.ForbidStart(joeTasks[i], joeCalendar));
                cp.Add(cp.ForbidEnd(joeTasks[i], joeCalendar));
            }
            for (int i = 0; i < jimTasks.Count; i++)
            {
                jimTasks[i].SetIntensity(jimCalendar, 1);
                cp.Add(cp.ForbidStart(jimTasks[i], jimCalendar));
                cp.Add(cp.ForbidEnd(jimTasks[i], jimCalendar));
            }
            //end:FORBID

            //$doc:OBJ
            cp.Add(cp.Minimize(cp.Max(ends.ToArray())));
            //end:OBJ

            /// EXTRACTING THE MODEL AND SOLVING.///
            //$doc:SOLVE
            cp.SetParameter(CP.IntParam.FailLimit, 10000);
            if (cp.Solve())
            {
                //end:SOLVE
                //$doc:SOLN
                for (int i = 0; i < allTasks.Count; i++)
                {
                    Console.WriteLine(cp.GetDomain(allTasks[i]));
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
