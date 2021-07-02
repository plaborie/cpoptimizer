// ------------------------------------------------------------- -*- java -*-
// File: examples/src/java/Facility.java
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

A company has 10 stores.  Each store must be supplied by one warehouse. The 
company has five possible locations where it has property and can build a 
supplier warehouse: Bonn, Bordeaux, London, Paris, and Rome. The warehouse 
locations have different capacities. A warehouse built in Bordeaux or Rome 
could supply only one store. A warehouse built in London could supply two 
stores; a warehouse built in Bonn could supply three stores; and a warehouse 
built in Paris could supply four stores. 

The supply costs vary for each store, depending on which warehouse is the 
supplier. For example, a store that is located in Paris would have low supply 
costs if it were supplied by a warehouse also in Paris.  That same store would 
have much higher supply costs if it were supplied by the other warehouses.

The cost of building a warehouse varies depending on warehouse location.

The problem is to find the most cost-effective solution to this problem, while
making sure that each store is supplied by a warehouse.

------------------------------------------------------------ */

import ilog.cp.*;
import ilog.concert.*;

import java.io.*;

public class Facility {
    public static String[] Names = {"blue", "white", "yellow", "green"}; 
    public static void main(String[] args) {
	String filename;
	if (args.length > 0)
	    filename = args[0];
	else
	    filename = "../../../examples/data/jfacility.dat";
        try {
	    IloCP cp = new IloCP();
	    int i,j;

	    FileInputStream fstream = new FileInputStream(filename);
	    Reader r = new BufferedReader(new InputStreamReader(fstream));
	    StreamTokenizer st = new StreamTokenizer(r);
	    st.nextToken();
	    int nbLocations = (int)st.nval;
	    st.nextToken();
	    int nbStores = (int)st.nval;
	    int[] capacity = new int[nbLocations];
	    int[] fixedCost = new int[nbLocations];
	    int[][] cost = new int[nbStores][];
	    for (i=0; i< nbStores; i++)
		cost[i] = new int[nbLocations];
	    int nbToRead = nbLocations;
	    while (nbToRead > 0) {
		if (st.nextToken() == StreamTokenizer.TT_NUMBER) {
		    capacity[nbLocations-nbToRead]= (int)st.nval;
		    nbToRead--;
		}
	    }
	    nbToRead = nbLocations;
	    while (nbToRead > 0) {
		if (st.nextToken() == StreamTokenizer.TT_NUMBER) {
		    fixedCost[nbLocations-nbToRead]= (int) st.nval;
		    nbToRead--;
		}
	    }
	    int linesToRead = nbStores;
	    while (linesToRead > 0) {
		nbToRead = nbLocations;
		while (nbToRead > 0) {
		    if (st.nextToken() == StreamTokenizer.TT_NUMBER) {
		    cost[nbStores-linesToRead][nbLocations-nbToRead]= (int) st.nval;
		    nbToRead--;
		    }
		}
		linesToRead--;
	    }

	    IloIntVar[] supplier = cp.intVarArray(nbStores, 0, nbLocations - 1);
	    for (j = 0; j < nbLocations; j++)
		cp.add(cp.le(cp.count(supplier, j), capacity[j]));

	    IloIntVar[] open = cp.intVarArray(nbLocations, 0, 1);

	    for (i = 0; i < nbStores; i++)
		cp.add(cp.eq(cp.element(open,supplier[i]), 1));

	    IloIntExpr obj = cp.prod(open, fixedCost);
	    for (i = 0; i < nbStores; i++)
		obj = cp.sum(obj, cp.element(cost[i],supplier[i]));

	    cp.add(cp.minimize(obj));

	    cp.solve();
	    
	    System.out.println();
	    System.out.println("Optimal value: " + (int)cp.getValue(obj) );
	    for (j = 0; j < nbLocations; j++){
		if (cp.getValue(open[j]) == 1){
		    System.out.print("Facility " + j + " is open, it serves stores ");
		    for (i = 0; i < nbStores; i++){
			if (cp.getValue(supplier[i]) == j)
			    System.out.print(i+ " ");
		    }
		    System.out.println();
		}
	    }
	} catch (IloException e) {
            System.err.println("Error " + e);
        }
	catch(FileNotFoundException exc) {
	    System.out.println("The file "+filename+" is not found");
	} catch(IOException exc) {
	    System.out.println("Error when reading the file");
	}
    }
}

