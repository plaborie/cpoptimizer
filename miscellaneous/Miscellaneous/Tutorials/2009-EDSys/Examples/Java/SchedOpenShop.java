// -------------------------------------------------------------- -*- Java -*-
// File: examples/src/java/SchedOpenShop.java
// --------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// --------------------------------------------------------------------------

import ilog.concert.*;
import ilog.cp.*;

import java.io.*;
import java.util.*;

public class SchedOpenShop {

    static class DataReader {

        private StreamTokenizer st;

        public DataReader(String filename) throws IOException {
            FileInputStream fstream = new FileInputStream(filename);
            Reader r = new BufferedReader(new InputStreamReader(fstream));
            st = new StreamTokenizer(r);
        }

        public int next() throws IOException {
            st.nextToken();
            return (int) st.nval;
        }
    }

    static class IntervalVarList extends ArrayList<IloIntervalVar> {
        public IloIntervalVar[] toArray() {
            return (IloIntervalVar[]) this.toArray(new IloIntervalVar[this.size()]);
        }
    }

    public static void main(String[] args) throws IOException {

        String filename = "../../../examples/data/openshop_j8-per10-2.data";
        int failLimit = 10000;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        try {
            IloCP cp = new IloCP();

            DataReader data = new DataReader(filename);
            int nbJobs = data.next();
            int nbMachines = data.next();

            IntervalVarList[] jobs = new IntervalVarList[nbJobs];
            for (int i = 0; i < nbJobs; i++)
                jobs[i] = new IntervalVarList();
            IntervalVarList[] machines = new IntervalVarList[nbMachines];
            for (int j = 0; j < nbMachines; j++)
                machines[j] = new IntervalVarList();

            IloIntExpr[] ends = new IloIntExpr[nbJobs * nbMachines];
            for (int i = 0; i < nbJobs; i++) {
                for (int j = 0; j < nbMachines; j++) {
                    int pt = data.next();
                    IloIntervalVar ti = cp.intervalVar(pt);
                    jobs[i].add(ti);
                    machines[j].add(ti);
                    ends[i * nbMachines + j] = cp.endOf(ti);
                }
            }

            for (int i = 0; i < nbJobs; i++)
                cp.add(cp.noOverlap(jobs[i].toArray()));

            for (int j = 0; j < nbMachines; j++)
                cp.add(cp.noOverlap(machines[j].toArray()));

            IloObjective objective = cp.minimize(cp.max(ends));
            cp.add(objective);

            cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
            System.out.println("Instance \t: " + filename);
            if (cp.solve()) {
                System.out.println("Makespan \t: " + cp.getObjValue());
            } else {
                System.out.println("No solution found.");
            }
            cp.printInformation();

        } catch (IloException e) {
            System.err.println("Error: " + e);
        }
    }


}
