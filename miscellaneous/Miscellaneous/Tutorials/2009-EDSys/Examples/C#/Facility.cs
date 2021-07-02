// --------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/Facility.cs
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

using System;
using System.IO;
using ILOG.CP;
using ILOG.Concert;

namespace Facility{
    public class Facility
    {
        static void Main(string[] args)
        {
            String filename;
            if (args.Length > 0)
                filename = args[0];
            else
                filename = "../../../../examples/data/facility-bis.dat";

            TextReader reader = File.OpenText(filename);
            string line = reader.ReadLine();
            int nbLocations = Convert.ToInt32(line);
            line = reader.ReadLine();
            int nbStores = Convert.ToInt32(line);

            int[] capacity = new int[nbLocations];
            line = reader.ReadLine();
            string[] numbers = line.Split();
            for (int i = 0; i < nbLocations; i++)
            {
                capacity[i] = Convert.ToInt32(numbers[i]);
            }
            int[] fixedCost = new int[nbLocations];
            line = reader.ReadLine();
            numbers = line.Split();
            for (int i = 0; i < nbLocations; i++)
            {
                fixedCost[i] = Convert.ToInt32(numbers[i]);
            }
           
            int[][] cost = new int[nbStores][];
            for (int i = 0; i < nbStores; i++)
            {
                cost[i] = new int[nbLocations];
                line = reader.ReadLine();
                numbers = line.Split();
                for (int j = 0; j < nbLocations; j++)
                {
                    cost[i][j] = Convert.ToInt32(numbers[j]);
                }
            }

            CP cp = new CP();

            IIntVar[] supplier = new IIntVar[nbStores];
            for (int i = 0; i < nbStores; i++)
            {
                supplier[i] = cp.IntVar(0, nbLocations - 1);
            }
            for (int j = 0; j < nbLocations; j++)
            {
                cp.Add(cp.Le(cp.Count(supplier, j), capacity[j]));
            }
            IIntVar[] open = new IIntVar[nbLocations];
            for (int i = 0; i < nbLocations; i++)
            {
                open[i] = cp.IntVar(0, 1);
            }
            for (int i = 0; i < nbStores; i++)
            {
                cp.Add(cp.Eq(cp.Element(open, supplier[i]), 1));
            }
            IIntExpr obj = cp.Prod(open, fixedCost);
            for (int i = 0; i < nbStores; i++)
            {
                obj = cp.Sum(obj, cp.Element(cost[i], supplier[i]));
            }
            cp.Add(cp.Minimize(obj));
            cp.Solve();

            Console.WriteLine();
            Console.WriteLine("Optimal value: " + cp.GetValue(obj));
            for (int j = 0; j < nbLocations; j++)
            {
                if (cp.GetValue(open[j]) == 1)
                {
                    Console.Write("Facility " + j + " is open, it serves stores ");
                    for (int i = 0; i < nbStores; i++)
                    {
                        if (cp.GetValue(supplier[i]) == j)
                            Console.Write(i + " ");
                    }
                    Console.WriteLine();
                }
            }
        }
    }
}

