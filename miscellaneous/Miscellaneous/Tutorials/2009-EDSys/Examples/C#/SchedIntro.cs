// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedIntro.cs
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

This is a basic problem that involves building a house. The masonry,
roofing, painting, etc.  must be scheduled. Some tasks must
necessarily take place before others, and these requirements are
expressed through precedence constraints.

------------------------------------------------------------ */

//$doc:ALL
//$doc:TASKS
using System;
using System.IO;
using ILOG.CP;
using ILOG.Concert;

namespace SchedIntro
{
    public class SchedIntro
    {

        static void Main()
        {
            CP cp = new CP();
            /// CREATE THE TIME-INTERVALS.///
            IIntervalVar masonry = cp.IntervalVar(35, "masonry   ");
            IIntervalVar carpentry = cp.IntervalVar(15, "carpentry ");
            IIntervalVar plumbing = cp.IntervalVar(40, "plumbing  ");
            IIntervalVar ceiling = cp.IntervalVar(15, "ceiling   ");
            IIntervalVar roofing = cp.IntervalVar(5, "roofing   ");
            IIntervalVar painting = cp.IntervalVar(10, "painting  ");
            IIntervalVar windows = cp.IntervalVar(5, "windows   ");
            IIntervalVar facade = cp.IntervalVar(10, "facade    ");
            IIntervalVar garden = cp.IntervalVar(5, "garden    ");
            IIntervalVar moving = cp.IntervalVar(5, "moving    ");
            //end:TASKS

            /// ADDING TEMPORAL CONSTRAINTS.///
            //$doc:CSTS
            cp.Add(cp.EndBeforeStart(masonry, carpentry));
            cp.Add(cp.EndBeforeStart(masonry, plumbing));
            cp.Add(cp.EndBeforeStart(masonry, ceiling));
            cp.Add(cp.EndBeforeStart(carpentry, roofing));
            cp.Add(cp.EndBeforeStart(ceiling, painting));
            cp.Add(cp.EndBeforeStart(roofing, windows));
            cp.Add(cp.EndBeforeStart(roofing, facade));
            cp.Add(cp.EndBeforeStart(plumbing, facade));
            cp.Add(cp.EndBeforeStart(roofing, garden));
            cp.Add(cp.EndBeforeStart(plumbing, garden));
            cp.Add(cp.EndBeforeStart(windows, moving));
            cp.Add(cp.EndBeforeStart(facade, moving));
            cp.Add(cp.EndBeforeStart(garden, moving));
            cp.Add(cp.EndBeforeStart(painting, moving));
            //end:CSTS

            /// EXTRACTING THE MODEL AND SOLVING.///
            //$doc:SOLVE
            if (cp.Solve())
            {
                Console.WriteLine(cp.GetDomain(masonry));
                Console.WriteLine(cp.GetDomain(carpentry));
                Console.WriteLine(cp.GetDomain(plumbing));
                Console.WriteLine(cp.GetDomain(ceiling));
                Console.WriteLine(cp.GetDomain(roofing));
                Console.WriteLine(cp.GetDomain(painting));
                Console.WriteLine(cp.GetDomain(windows));
                Console.WriteLine(cp.GetDomain(facade));
                Console.WriteLine(cp.GetDomain(garden));
                Console.WriteLine(cp.GetDomain(moving));
            }
            else
            {
                Console.Write("No solution found. ");
            }
            cp.PrintInformation();
        }
    }
}
//end:SOLVE
//end:ALL
