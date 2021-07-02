// -------------------------------------------------------------- -*- Java -*-
// File: examples/src/java/SchedRCPSPMM.java
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

public class SchedRCPSPMM {

    private static class DataReader {

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

    static IloIntExpr[] arrayFromList(List<IloIntExpr> list) {
        return (IloIntExpr[]) list.toArray(new IloIntExpr[list.size()]);
    }

    public static void main(String[] args) throws IOException {

        String filename = "../../../examples/data/rcpspmm_j30_12_8.data";
        int failLimit = 10000;
        int nbTasks, nbRenewable, nbNonRenewable;

        if (args.length > 0)
            filename = args[0];
        if (args.length > 1)
            failLimit = Integer.parseInt(args[1]);

        IloCP cp = new IloCP();
        DataReader data = new DataReader(filename);
        try {
            nbTasks = data.next();
            nbRenewable = data.next();
            nbNonRenewable = data.next();
            IloCumulFunctionExpr[] renewables = new IloCumulFunctionExpr[nbRenewable];
            IloIntExpr[] nonRenewables = new IloIntExpr[nbNonRenewable];
            int[] capRenewables = new int[nbRenewable];
            int[] capNonRenewables = new int[nbNonRenewable];
            for (int j = 0; j < nbRenewable; j++) {
                renewables[j] = cp.cumulFunctionExpr();
                capRenewables[j] = data.next();
            }
            for (int j = 0; j < nbNonRenewable; j++) {
                nonRenewables[j] = cp.intExpr();
                capNonRenewables[j] = data.next();
            }

            IloIntervalVar[] tasks = new IloIntervalVar[nbTasks];
            IntervalVarList[] modes = new IntervalVarList[nbTasks];
            for (int i = 0; i < nbTasks; i++) {
                tasks[i] = cp.intervalVar();
                modes[i] = new IntervalVarList();
            }
            List<IloIntExpr> ends = new ArrayList<IloIntExpr>();
            for (int i = 0; i < nbTasks; i++) {
                IloIntervalVar task = tasks[i];
                int d = data.next();
                int nbModes = data.next();
                int nbSucc = data.next();
                for (int k = 0; k < nbModes; k++) {
                    IloIntervalVar alt = cp.intervalVar();
                    alt.setOptional();
                    modes[i].add(alt);
                }
                cp.add(cp.alternative(task, modes[i].toArray()));
                ends.add(cp.endOf(task));
                for (int s = 0; s < nbSucc; s++) {
                    int succ = data.next();
                    cp.add(cp.endBeforeStart(task, tasks[succ]));
                }
            }
            for (int i = 0; i < nbTasks; i++) {
                IloIntervalVar task = tasks[i];
                IntervalVarList imodes = modes[i];
                for(int k=0; k < imodes.size(); k++) {
                    int taskId = data.next();
                    int modeId = data.next();
                    int d = data.next();
                    imodes.get(k).setSizeMin(d);
                    imodes.get(k).setSizeMax(d);
                    int q;
                    for (int j = 0; j < nbNonRenewable; j++) {
                        q = data.next();
                        if (0 < q) {
                            renewables[j] = cp.sum(renewables[j], cp.pulse(imodes.get(k), q));
                        }
                    }
                    for (int j = 0; j < nbNonRenewable; j++) {
                        q = data.next();
                        if (0 < q) {
                            nonRenewables[j] = cp.sum(nonRenewables[j], cp.prod(q, cp.presenceOf(imodes.get(k))));
                        }
                    }
                }
            }

            for (int j = 0; j < nbRenewable; j++) {
                cp.add(cp.le(renewables[j], capRenewables[j]));
            }

            for (int j = 0; j < nbRenewable; j++) {
                cp.add(cp.le(nonRenewables[j], capNonRenewables[j]));
            }

            IloObjective objective = cp.minimize(cp.max(arrayFromList(ends)));
            cp.add(objective);

            cp.setParameter(IloCP.IntParam.FailLimit, failLimit);
            System.out.println("Instance \t: " + filename);
            if (cp.solve()) {
                System.out.println("Makespan \t: " + cp.getObjValue());
            }
            else {
                System.out.println("No solution found.");
            }
            cp.printInformation();

        } catch (IloException e) {
            System.err.println("Error: " + e);
        }
    }



}
