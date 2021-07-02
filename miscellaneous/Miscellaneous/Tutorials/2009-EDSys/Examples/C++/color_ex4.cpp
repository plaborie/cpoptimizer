// -------------------------------------------------------------- -*- C++ -*-
// File: examples/src/cpp/color_ex4.cpp
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

The problem involves choosing colors for the countries on a map in 
such a way that at most four colors (blue, white, yellow, green) are 
used and no neighboring countries are the same color. In this exercise, 
you will find a solution for a map coloring problem with seven countries: 
Belgium, Denmark, France, Germany, Luxembourg, the Netherlands, and
Switzerland. 

------------------------------------------------------------ */
#include <ilcp/cp.h>

const char* Names[] = {"blue", "white", "yellow", "green"}; 

int main(int, const char * []){
  IloEnv env;
  try {
    IloModel model(env);
    IloIntVar Belgium(env, 0, 3), Denmark(env, 0, 3), France(env, 0, 3), 
      Germany(env, 0, 3), Luxembourg(env, 0, 3), Netherlands(env, 0, 3),
      Switzerland(env, 0, 3);
    model.add(Belgium != France); 
    model.add(Belgium != Germany); 
    model.add(Belgium != Netherlands);
    model.add(Belgium != Luxembourg);
    model.add(Denmark != Germany ); 
    model.add(France != Germany); 
    model.add(France != Luxembourg); 
    model.add(France != Switzerland);
    model.add(Germany != Switzerland);
    model.add(Germany != Luxembourg);
    model.add(Germany != Netherlands); 
    IloCP cp(model);
    cp.setParameter(IloCP::LogVerbosity, IloCP::Quiet);
    if (cp.solve()){    
      cp.out() << std::endl << cp.getStatus() << " Solution" << std::endl;
      cp.out() << "Belgium:     " << Names[cp.getValue(Belgium)] << std::endl;
      cp.out() << "Denmark:     " << Names[cp.getValue(Denmark)] << std::endl;
      cp.out() << "France:      " << Names[cp.getValue(France)] << std::endl;
      cp.out() << "Germany:     " << Names[cp.getValue(Germany)] << std::endl;
      cp.out() << "Luxembourg:  " << Names[cp.getValue(Luxembourg)] << std::endl;
      cp.out() << "Netherlands: " << Names[cp.getValue(Netherlands)] << std::endl;
      cp.out() << "Switzerland: " << Names[cp.getValue(Switzerland)] << std::endl;
    }  
    cp.printInformation();
  }
  catch (IloException& ex) {
    env.out() << "Error: " << ex << std::endl;
  }
  env.end();
  return 0;
}
/*
Feasible Solution
Belgium:     green
Denmark:     blue
France:      yellow
Germany:     white
Luxembourg:  blue
Netherlands: blue
Switzerland: blue
*/
