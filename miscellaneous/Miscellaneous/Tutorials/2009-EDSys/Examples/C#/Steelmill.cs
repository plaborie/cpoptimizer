// --------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/Steelmill.cs
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

The problem is to build steel coils from slabs that are available in a 
work in process inventory of semi finished products. We assume that 
there is no limitation in the number of slabs that we can request, but 
only a finite number of slab sizes is available (sizes 12, 14, 17, 18, 
19, 20, 23, 24 , 25, 26, 27, 28, 29, 30, 32, 35, 39, 42, 43, 44). The 
problem is to select a number of slabs to build the coil orders, and to 
satisfy the following constraints:

    * Each coil order requires a specific process to build it from a
      slab. This process is encoded by a color.
    * A coil order can be built from only one slab
    * Several coil order can be built from the same slab. But a slab can
      be used to produce at most two different ``colors'' of coils
    * The sum of the sizes of each coil order built from a slab must no
      exceed the slab size.

Finally, the production plan should minimize the unused capacity of the 
selected slabs.

------------------------------------------------------------ */
using System;
using System.IO;
using ILOG.CP;
using ILOG.Concert;

namespace Steelmill{
   public class Steelmill{
      static int nbOrders = 12; 
  	  static int   nbSlabs = 12; 
	  static int   nbColors   = 8;
	  static int[] capacities=  {0, 12, 14, 17, 18, 19, 20, 
	  			               23, 24, 25, 26, 27, 28, 29, 
					           30, 32, 35, 39, 42, 43, 44}; 
	  static int[] weight = {22, 9, 8, 7, 7, 5, 4, 4, 3, 3, 3, 2};
	  static int[] colors = { 1, 2, 4, 3, 7, 3, 0, 6, 5, 3, 5, 5};

      static void Main(string[] args){
        CP cp = new CP();

	    int weightSum=0;
	    for (int o = 0; o < nbOrders; o++)
		  weightSum += weight[o];

	    IIntVar[] where = new IIntVar[nbOrders];
        for(int o = 0; o < nbOrders; o++)
          where[o] = cp.IntVar(0, nbSlabs-1);
	    IIntVar[] load = new IIntVar[nbSlabs];
        for(int m = 0; m < nbSlabs; m++)
          load[m] = cp.IntVar(0, weightSum);
	    
	    // Pack constraint
	    cp.Add(cp.Pack(load, where, weight));
	    
	    // Color constraints 
	    for(int m = 0; m < nbSlabs; m++) {
		  IIntExpr[] colorExpArray = new IIntExpr[nbColors]; 
		  for(int c = 0; c < nbColors; c++) {
		    IConstraint orCt =  cp.FalseConstraint();
		    for(int o = 0; o < nbOrders; o++){
			  if (colors[o] == c){
			    orCt = cp.Or(orCt, cp.Eq(where[o], m));
			  }
		    } 
		    colorExpArray[c] =  cp.IntExpr(orCt);
		  }
		  cp.Add(cp.Le(cp.Sum(colorExpArray), 2)); 
	    }

	    // Objective function 
	    int sizeLossValues = capacities[capacities.Length-1] - capacities[0] + 1;
	    int[] lossValues  = new int[sizeLossValues]; 
	    lossValues[0]= 0;
	    int indexValue= 1;
	    for(int q = 1; q < capacities.Length; q++){ 
		  for(int p = capacities[q-1] + 1; p <= capacities[q]; p++){
		    lossValues[indexValue] = capacities[q] - p;
		    indexValue++;
		  }
	    }
	    IIntExpr obj = cp.Constant(0);
	    for(int m = 0; m < nbSlabs; m++){
		  obj = cp.Sum(obj,  cp.Element(lossValues, load[m]));  
	    }
	    cp.Add(cp.Minimize(obj));

	    // - A symmetry breaking constraint that is useful for small instances
	    for(int m = 1; m < nbSlabs; m++){
		  cp.Add(cp.Ge(load[m-1],load[m])); 
	    }
	    
        if (cp.Solve()){
          Console.WriteLine("Optimal value: " + cp.GetValue(obj));
	  for (int m = 0; m < nbSlabs; m++) {
	    int p = 0;
	    for (int o = 0; o < nbOrders; o++)
	      if (cp.GetValue(where[o]) == m) ++p;
	    if (p == 0) continue;
	    Console.Write("Slab " + m + " is used for order");
	    if (p > 1) Console.Write("s");
	    Console.Write(" :");
	    for (int o = 0; o < nbOrders; o++) {
	      if (cp.GetValue(where[o]) == m)
		Console.Write(" " + o);
	    }
	    Console.WriteLine();
	  }
	}
        cp.PrintInformation();
     }
    }
}
