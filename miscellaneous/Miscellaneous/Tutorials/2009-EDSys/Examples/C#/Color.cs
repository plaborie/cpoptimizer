// --------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/Color.cs
// --------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------

using System;
using System.IO;
using ILOG.CP;
using ILOG.Concert;

/* ------------------------------------------------------------
Problem Description
-------------------

A map-coloring problem involves choosing colors for the 
countries on a map in such a way that at most four colors are 
used and no two neighboring countries are the same color.  

For our example, we will consider six countries: Belgium, 
Denmark, France, Germany, Netherlands, and Luxembourg.  The 
colors can be blue, white, red or green.
------------------------------------------------------------ */


namespace Color {
  public class Color {
    public static string[] Names = {"blue", "white", "red", "green"}; 
    static void Main() {   

      CP cp = new CP();
      IIntVar Belgium = cp.IntVar(0, 3);
      IIntVar Denmark = cp.IntVar(0, 3); 
      IIntVar France = cp.IntVar(0, 3);
      IIntVar Germany = cp.IntVar(0, 3); 
      IIntVar Netherlands = cp.IntVar(0, 3);
      IIntVar Luxembourg = cp.IntVar(0, 3);
 
 
      cp.Add(cp.Neq(Belgium , France)); 
      cp.Add(cp.Neq(Belgium , Germany)); 
      cp.Add(cp.Neq(Denmark , Germany)); 
      cp.Add(cp.Neq(France , Germany)); 
      cp.Add(cp.Neq(Belgium , Netherlands)); 
      cp.Add(cp.Neq(Germany , Netherlands)); 
      cp.Add(cp.Neq(France , Luxembourg));  
      cp.Add(cp.Neq(Luxembourg , Germany));
      cp.Add(cp.Neq(Luxembourg , Belgium));
  
      // Search for a solution
      if (cp.Solve()) {    
        Console.WriteLine("Solution: ");
        Console.WriteLine("Belgium:     " + Names[ cp.GetIntValue( Belgium ) ] );
        Console.WriteLine("Denmark:     " + Names[ cp.GetIntValue( Denmark ) ] );
        Console.WriteLine("France:      " + Names[ cp.GetIntValue( France ) ] );
        Console.WriteLine("Germany:     " + Names[ cp.GetIntValue( Germany ) ] );
        Console.WriteLine("Netherlands: " + Names[ cp.GetIntValue( Netherlands ) ] );
        Console.WriteLine("Luxembourg:  " + Names[ cp.GetIntValue( Luxembourg ) ] );
        cp.PrintInformation();
      }
    }
  }
}
  /*
  FEASIBLE Solution
  Belgium:     blue
  Denmark:     blue
  France:      white
  Germany:     red
  Netherlands: white
  Luxembourg:  green
  OPTIMAL Solution
  Belgium:     white
  Denmark:     blue
  France:      blue
  Germany:     white
  Netherlands: blue
  Luxembourg:  red
  */
