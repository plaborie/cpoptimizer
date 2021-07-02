// -------------------------------------------------------------- -*- C# -*-
// File: examples/src/csharp/SchedRCPSPMM.cs
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

namespace SchedRCPSPMM
{
    public class SchedRCPSPMM
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
            String filename = "../../../../examples/data/rcpspmm_j30_12_8.data";
            int failLimit = 10000;
            int nbTasks, nbRenewable, nbNonRenewable;

            if (args.Length > 0)
                filename = args[0];
            if (args.Length > 1)
                failLimit = Convert.ToInt32(args[1]);

            CP cp = new CP();
            DataReader data = new DataReader(filename);

            nbTasks = data.next();
            nbRenewable = data.next();
            nbNonRenewable = data.next();
            ICumulFunctionExpr[] renewables = new ICumulFunctionExpr[nbRenewable];
            IIntExpr[] nonRenewables = new IIntExpr[nbNonRenewable];
            int[] capRenewables = new int[nbRenewable];
            int[] capNonRenewables = new int[nbNonRenewable];
            for (int j = 0; j < nbRenewable; j++)
            {
                renewables[j] = cp.CumulFunctionExpr();
                capRenewables[j] = data.next();
            }
            for (int j = 0; j < nbNonRenewable; j++)
            {
                nonRenewables[j] = cp.IntExpr();
                capNonRenewables[j] = data.next();
            }

            IIntervalVar[] tasks = new IIntervalVar[nbTasks];
            List<IIntervalVar>[] modes = new List<IIntervalVar>[nbTasks];
            for (int i = 0; i < nbTasks; i++)
            {
                tasks[i] = cp.IntervalVar();
                modes[i] = new List<IIntervalVar>();
            }
            List<IIntExpr> ends = new List<IIntExpr>();
            for (int i = 0; i < nbTasks; i++)
            {
                IIntervalVar task = tasks[i];
                int d = data.next();
                int nbModes = data.next();
                int nbSucc = data.next();
                for (int k = 0; k < nbModes; k++)
                {
                    IIntervalVar alt = cp.IntervalVar();
                    alt.SetOptional();
                    modes[i].Add(alt);
                }
                cp.Add(cp.Alternative(task, modes[i].ToArray()));
                ends.Add(cp.EndOf(task));
                for (int s = 0; s < nbSucc; s++)
                {
                    int succ = data.next();
                    cp.Add(cp.EndBeforeStart(task, tasks[succ]));
                }
            }
            for (int i = 0; i < nbTasks; i++)
            {
                IIntervalVar task = tasks[i];
                List<IIntervalVar> imodes = modes[i];
                for (int k = 0; k < imodes.Count; k++)
                {
                    int taskId = data.next();
                    int modeId = data.next();
                    int d = data.next();
                    imodes[k].SizeMin = d;
                    imodes[k].SizeMax = d;
                    int q;
                    for (int j = 0; j < nbNonRenewable; j++)
                    {
                        q = data.next();
                        if (0 < q)
                        {
                            renewables[j].Add(cp.Pulse(imodes[k], q));
                        }
                    }
                    for (int j = 0; j < nbNonRenewable; j++)
                    {
                        q = data.next();
                        if (0 < q)
                        {
                            nonRenewables[j] = cp.Sum(nonRenewables[j], cp.Prod(q, cp.PresenceOf(imodes[k])));
                        }
                    }
                }
            }

            for (int j = 0; j < nbRenewable; j++)
            {
                cp.Add(cp.Le(renewables[j], capRenewables[j]));
            }

            for (int j = 0; j < nbRenewable; j++)
            {
                cp.Add(cp.Le(nonRenewables[j], capNonRenewables[j]));
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
