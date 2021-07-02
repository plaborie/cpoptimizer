// ------------------------------------------------------------- -*- java -*-
// File: examples/src/java/Steelmill.java
// --------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------
/*

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

import ilog.cp.*;
import ilog.concert.*;

public class Steelmill {
    public static void main(String[] args) {
        try { 
	    IloCP cp = new IloCP();
	    int m, o, c, q;
	    int         nbOrders   = 12; 
	    int         nbSlabs = 12; 
	    int         nbColors   = 8;
	    int[]    capacities=  {0, 12, 14, 17, 18, 19, 20, 
					  23, 24, 25, 26, 27, 28, 29, 
					  30, 32, 35, 39, 42, 43, 44}; 
	    int[] weight = {22, 9, 8, 7, 7, 5, 4, 4, 3, 3, 3, 2};
	    int[] colors = { 1, 2, 4, 3, 7, 3, 0, 6, 5, 3, 5, 5};

	    int weightSum=0;
	    for (int i = 0; i < nbOrders; i++)
		weightSum += weight[i];

	    IloIntVar[] where = cp.intVarArray(nbOrders, 0, nbSlabs-1);
	    IloIntVar[] load = cp.intVarArray(nbSlabs, 0, weightSum);
	    
	    // Pack constraint
	    cp.add(cp.pack(load, where, weight));
	    
	    // Color constraints 
	    for(m = 0; m < nbSlabs; m++) {
		IloIntExpr[] colorExpArray = cp.intExprArray(nbColors); 
		for(c = 0; c < nbColors; c++) {
		    IloConstraint orCt =  cp.falseConstraint();
		    for(o = 0; o < nbOrders; o++){
			if (colors[o] == c){
			    orCt = cp.or(orCt, cp.eq(where[o], m));
			}
		    } 
		    colorExpArray[c]=  cp.intExpr(orCt);
		}
		cp.add(cp.le(cp.sum(colorExpArray), 2)); 
	    }

	    // Objective function 
	    int sizeLossValues = capacities[capacities.length-1] - capacities[0] + 1;
	    int[] lossValues  = new int[sizeLossValues ]; 
	    lossValues[0]= 0;
	    int indexValue= 1;
	    for(q = 1; q < capacities.length; q++){ 
		for(int p = capacities[q-1] + 1; p <= capacities[q]; p++){
		    lossValues[indexValue]= capacities[q] - p;
		    indexValue++;
		}
	    }
	    IloIntExpr obj = cp.constant(0);
	    for(m = 0; m < nbSlabs; m++){
		obj = cp.sum(obj,  cp.element(lossValues, load[m]));  
	    }
	    cp.add(cp.minimize(obj));

	    // - A symmetry breaking constraint that is useful for small instances
	    for(m = 1; m < nbSlabs; m++){
		cp.add(cp.ge(load[m-1],load[m])); 
	    }
	    
            if (cp.solve()){
                System.out.println("Optimal value: " + (int)cp.getValue(obj));

		for (m = 0; m < nbSlabs; m++) {
		    int p = 0;
		    for (o = 0; o < nbOrders; o++)
			if (cp.getValue(where[o]) == m) ++p;
		    if (p == 0) continue;
		    System.out.print("Slab " + m + " is used for order");
		    if (p > 1) System.out.print("s");
		    System.out.print(" :");
		    for (o = 0; o < nbOrders; o++) {
			if (cp.getValue(where[o]) == m)
			    System.out.print(" " + o);
		    }
		    System.out.println();
		}
	    }
            cp.printInformation();
        } catch (IloException e) {
            System.err.println("Error " + e);
        }
    }
}

