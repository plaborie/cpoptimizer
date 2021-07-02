// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedJobShopFlex.cs
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

namespace SchedJobShopFlex
{
    public class SchedJobShopFlex
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
            String filename = "../../../../examples/data/jobshopflex_mk08.data";
            int failLimit = 10000;

            if (args.Length > 0)
                filename = args[0];
            if (args.Length > 1)
                failLimit = Convert.ToInt32(args[1]);

            CP cp = new CP();

            DataReader data = new DataReader(filename);
            int nbJobs = data.next();
            int nbMachines = data.next();

            List<IIntervalVar>[] machines = new List<IIntervalVar>[nbMachines];
            for (int j = 0; j < nbMachines; j++)
                machines[j] = new List<IIntervalVar>();
            List<IIntExpr> ends = new List<IIntExpr>();

            for (int i = 0; i < nbJobs; i++)
            {
                int nbOperations = data.next();
                IIntervalVar prec = cp.IntervalVar();
                for (int j = 0; j < nbOperations; j++)
                {
                    int nbOpMachines = data.next();
                    IIntervalVar master = cp.IntervalVar();
                    List<IIntervalVar> members = new List<IIntervalVar>();
                    for (int k = 0; k < nbOpMachines; k++)
                    {
                        int m = data.next();
                        int d = data.next();
                        IIntervalVar member = cp.IntervalVar(d);
                        member.SetOptional();
                        members.Add(member);
                        machines[m - 1].Add(member);
                    }
                    cp.Add(cp.Alternative(master, members.ToArray()));
                    if (j > 0)
                        cp.Add(cp.EndBeforeStart(prec, master));
                    prec = master;
                }
                ends.Add(cp.EndOf(prec));
            }

            for (int j = 0; j < nbMachines; j++)
            {
                cp.Add(cp.NoOverlap(machines[j].ToArray()));
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
    }
}

