// ------------------------------------------------------------- -*- Java -*-
// File: examples/src/java/SchedIntro.java
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
import ilog.concert.*;
import ilog.cp.*;

public class SchedIntro {

    public static void main(String[] args) {
        try {
            IloCP cp = new IloCP();
            
            /* CREATE THE INTERVAL VARIABLES. */
            IloIntervalVar masonry   = cp.intervalVar(35, "masonry   ");
            IloIntervalVar carpentry = cp.intervalVar(15, "carpentry ");
            IloIntervalVar plumbing  = cp.intervalVar(40, "plumbing  ");
            IloIntervalVar ceiling   = cp.intervalVar(15, "ceiling   ");
            IloIntervalVar roofing   = cp.intervalVar(5,  "roofing   ");
            IloIntervalVar painting  = cp.intervalVar(10, "painting  ");
            IloIntervalVar windows   = cp.intervalVar(5,  "windows   ");
            IloIntervalVar facade    = cp.intervalVar(10, "facade    ");
            IloIntervalVar garden    = cp.intervalVar(5,  "garden    ");
            IloIntervalVar moving    = cp.intervalVar(5,  "moving    ");
//end:TASKS

            /* ADDING PRECEDENCE CONSTRAINTS. */
//$doc:CSTS
            cp.add(cp.endBeforeStart(masonry,   carpentry));
            cp.add(cp.endBeforeStart(masonry,   plumbing));
            cp.add(cp.endBeforeStart(masonry,   ceiling));
            cp.add(cp.endBeforeStart(carpentry, roofing));
            cp.add(cp.endBeforeStart(ceiling,   painting));
            cp.add(cp.endBeforeStart(roofing,   windows));
            cp.add(cp.endBeforeStart(roofing,   facade));
            cp.add(cp.endBeforeStart(plumbing,  facade));
            cp.add(cp.endBeforeStart(roofing,   garden));
            cp.add(cp.endBeforeStart(plumbing,  garden));
            cp.add(cp.endBeforeStart(windows,   moving));
            cp.add(cp.endBeforeStart(facade,    moving));
            cp.add(cp.endBeforeStart(garden,    moving));
            cp.add(cp.endBeforeStart(painting,  moving));
//end:CSTS

            /* EXTRACTING THE MODEL AND SOLVING. */
//$doc:SOLVE
            if (cp.solve()) {
                System.out.println(cp.getDomain(masonry));
                System.out.println(cp.getDomain(carpentry));
                System.out.println(cp.getDomain(plumbing));
                System.out.println(cp.getDomain(ceiling));
                System.out.println(cp.getDomain(roofing));
                System.out.println(cp.getDomain(painting));
                System.out.println(cp.getDomain(windows));
                System.out.println(cp.getDomain(facade));
                System.out.println(cp.getDomain(garden));
                System.out.println(cp.getDomain(moving));
            } else {
                System.out.print("No solution found. ");
            }
            cp.printInformation();
        } catch (IloException e) {
            System.err.println("Error " + e );
        }
    }
}
//end:SOLVE
//end:ALL