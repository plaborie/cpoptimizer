// -------------------------------------------------------------- -*- C++ -*-
// File: examples/src/cpp/talent.cpp
// ---------------------------------------------------------------------------
// Copyright (C) 1990-2008 by ILOG.
// All Rights Reserved.
//
// Permission is expressly granted to use this example in the
// course of developing applications that use ILOG products.
// ---------------------------------------------------------------------------

#include <ilcp/cp.h>

// Read talent scheduling problem data from a file
IloBool ReadData(const char * filename,
                 IloIntArray actorPay,
                 IloIntArray sceneDuration,
                 IloArray<IloIntSet> actorInScene) {
  IloEnv env = actorPay.getEnv();
  std::ifstream in(filename);
  if (!in.good())
    return IloFalse;

  IloInt numActors, numScenes, a, s;

  in >> numActors;
  for (a = 0; a < numActors; a++) { 
    IloInt pay;
    in >> pay;
    actorPay.add(pay);
  }

  in >> numScenes;
  for (s = 0; s < numScenes; s++) {
    IloInt duration;
    in >> duration;
    sceneDuration.add(duration);
  }

  for (a = 0; a < numActors; a++) {
    actorInScene.add(IloIntSet(env));
    for (s = 0; s < numScenes; s++) {
      IloBool inScene;
      in >> inScene;
      if (inScene)
        actorInScene[a].add(s);
    }
  }
  if (!in.good())
    return IloFalse;

  in.close();
  return IloTrue;
}

// Build the talent scheduling model
IloModel BuildModel(IloIntVarArray scene,
                    IloIntVar idleCost,
                    IloIntArray actorCost,
                    IloIntArray sceneDuration,
                    IloArray<IloIntSet> actorInScene) {
  IloEnv env = scene.getEnv();
  IloInt numScenes = scene.getSize();
  IloInt numActors = actorCost.getSize();
  IloModel model(env);

  // Make the slot-based secondary model
  IloIntVarArray slot(env, numScenes, 0, numScenes - 1);
  model.add(IloInverse(env, scene, slot));

  // Expression representing the global cost
  IloIntExpr cost(env);

  // Loop over all actors, building cost
  for (IloInt a = 0; a < numActors; a++) {
    // Expression for the waiting time for this actor
    IloIntExpr actorWait(env);

    // Calculate the first and last slots where this actor plays
    IloIntVarArray position(env);
    for (IloIntSet::Iterator it(actorInScene[a]); it.ok(); ++it)
      position.add(slot[*it]);
    IloIntExpr firstSlot = IloMin(position);
    IloIntExpr lastSlot = IloMax(position);

    // If an actor is not in a scene,
    // he waits if he is on set when the scene is filmed
    for (IloInt s = 0; s < numScenes; s++) {
      if (!actorInScene[a].contains(s)) { // not in scene
        IloIntExpr wait = (firstSlot <= slot[s] && slot[s] <= lastSlot);
        actorWait += sceneDuration[s] * wait;
      }
    }

    // Accumulate the cost of waiting time for this actor
    cost += actorCost[a] * actorWait;
  }
  model.add(idleCost == cost);
  model.add(IloMinimize(env, idleCost));
  return model;
}

int main(int argc, const char * argv[]) {
  IloEnv env;
  try {
    const char * inputFile = "../../../examples/data/rehearsal.txt";
    IloNum tlim = 10.0;
    if (argc > 1)
      inputFile = argv[1];
    if (argc > 2)
      tlim = atof(argv[2]);

    IloIntArray actorPay(env);
    IloIntArray sceneDuration(env);
    IloArray<IloIntSet> actorInScene(env);
    IloBool ok = ReadData(inputFile, actorPay, sceneDuration, actorInScene);
    if (!ok) {
      env.out() << "Error reading " << inputFile << std::endl;
    }
    else {
      // Create the decision variables, cost, and the model
      IloInt numScenes = sceneDuration.getSize();
      IloInt numActors = actorPay.getSize();
      IloIntVarArray scene(env, numScenes, 0, numScenes - 1);
      IloIntVar idleCost(env, 0, IloIntMax);
      IloModel model = BuildModel(
        scene, idleCost, actorPay, sceneDuration, actorInScene
      );

      // Create the CP solver
      IloCP cp(model);
      cp.setParameter(IloCP::TimeLimit, tlim);
      cp.solve();

      cp.out() << "Solution of idle cost " << cp.getValue(idleCost) << std::endl; 
      cp.out() << "Order:";
      for (IloInt s = 0; s < numScenes; s++)
        cp.out() << " " << 1 + cp.getValue(scene[s]);
      cp.out() << std::endl;

      // Give more detailed information on the schedule
      for (IloInt a = 0; a < numActors; a++) {
        cp.out() << "|";
        for (IloInt s = 0; s < numScenes; s++) {
          IloInt sc = cp.getValue(scene[s]);
          for (IloInt d = 0; d < sceneDuration[sc]; d++) {
            if (actorInScene[a].contains(sc))
              cp.out() << "X";
            else
              cp.out() << ".";
          }
          cp.out() << "|";
        }
        cp.out() << "  (Rate = " << actorPay[a] << ")" << std::endl;
      }
      cp.end();
    }
  } catch (IloException & ex) {
    env.out() << "Caught: " << ex << std::endl;
  }
  env.end();
  return 0;
}
