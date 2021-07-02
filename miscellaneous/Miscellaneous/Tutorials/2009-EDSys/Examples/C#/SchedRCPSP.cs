// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedRCPSP.cs
// --------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------

using System;
using System.IO;
using System.Collections.Generic;
using ILOG.CP;
using ILOG.Concert;

namespace SchedRCPSP
{
    public class SchedRCPSP
    {
        class DataReader
        {
            private int index = -1;
            private string[] datas;

            public DataReader(String filename)
            {
                StreamReader reader = new StreamReader(filename);
                datas = reader.ReadToEnd().Split((char[])null, StringSplitOptions.RemoveEmptyEntries);
            }

            public int next()
            {
                index++;
                return Convert.ToInt32(datas[index]);
            }
        }

        public static void Main(String[] args)
        {
            String filename = "../../../../examples/data/rcpsp_j120_1_2.data";
            int failLimit = 10000;
            int nbTasks, nbResources;

            if (args.Length > 0)
                filename = args[0];
            if (args.Length > 1)
                failLimit = Convert.ToInt32(args[1]);

            CP cp = new CP();
            DataReader data = new DataReader(filename);
            try
            {
                nbTasks = data.next();
                nbResources = data.next();
                List<IIntExpr> ends = new List<IIntExpr>();
                ICumulFunctionExpr[] resources = new ICumulFunctionExpr[nbResources];
                int[] capacities = new int[nbResources];

                for (int j = 0; j < nbResources; j++)
                {
                    capacities[j] = data.next();
                    resources[j] = cp.CumulFunctionExpr();
                }
                IIntervalVar[] tasks = new IIntervalVar[nbTasks];
                for (int i = 0; i < nbTasks; i++)
                {
                    tasks[i] = cp.IntervalVar();
                }
                for (int i = 0; i < nbTasks; i++)
                {
                    IIntervalVar task = tasks[i];
                    int d, nbSucc;
                    d = data.next();
                    task.SizeMin = d;
                    task.SizeMax = d;
                    ends.Add(cp.EndOf(task));
                    for (int j = 0; j < nbResources; j++)
                    {
                        int q = data.next();
                        if (q > 0)
                            resources[j].Add(cp.Pulse(task, q));
                    }
                    nbSucc = data.next();
                    for (int s = 0; s < nbSucc; s++)
                    {
                        int succ = data.next();
                        cp.Add(cp.EndBeforeStart(task, tasks[succ - 1]));
                    }
                }

                for (int j = 0; j < nbResources; j++)
                {
                    cp.Add(cp.Le(resources[j], capacities[j]));
                }

                IObjective objective = cp.Minimize(cp.Max(ends.ToArray()));
                cp.Add(objective);

                cp.SetParameter(CP.IntParam.FailLimit, failLimit);
                Console.WriteLine("Instance \t: " + filename);
                if (cp.Solve())
                {
                    Console.WriteLine("Makespan \t: " + cp.ObjValue);
                }
                else
                {
                    Console.WriteLine("No solution found.");
                }
                cp.PrintInformation();

            }
            catch (IloException e)
            {
                Console.WriteLine(" ERROR: " + e);
            }
        }
    }
}