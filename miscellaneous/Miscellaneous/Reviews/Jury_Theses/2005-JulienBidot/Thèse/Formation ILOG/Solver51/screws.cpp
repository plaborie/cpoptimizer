// --------------------------------------------------------------------------
//  Copyright (C) 1990-2000 by ILOG.
//  All Rights Reserved.
//
//  N O T I C E
//
//  THIS MATERIAL IS CONSIDERED A TRADE SECRET BY ILOG.
//  UNAUTHORIZED ACCESS, USE, REPRODUCTION OR DISTRIBUTION IS PROHIBITED.
// --------------------------------------------------------------------------


/* ------------------------------------------------------------

Problem Description
-------------------

A company has to assign hostesses and stewards to flights. There 
are 10 flights a day.  A crew is composed of hostesses and stewards 
This company employs 20 hostesses and stewards.

------------------------------------------------------------ */

#include <string.h>
#include <ilsolver/ilosolver.h>


ILOSTLBEGIN

// requirement constraint
void TeamConstraints(IloModel model, IloNumSetVar crew,
                     IloInt n, IloNumArray nbMembers, 
                     IloNumSetVarArray memberSets,
                     IloNumArray staff) {

   IloEnv env = model.getEnv();
   // constraint : each crew must include a specific number of attendants
   model.add(IloCard(crew) == n);
   
   IloInt size = nbMembers.getSize();
   
   for (IloInt i = 0; i < size ; i++) {
      // express the minimal number of a certain kind of attendant
      
      IloNumSetVar inter(env, staff);
      model.add(IloEqIntersection(env, inter, crew, memberSets[i]));
      model.add(IloCard(inter) >= nbMembers[i]);
   }
}

// displaying solution
void Print(IloSolver solver, IloNumSetVar E, char** Names) {
   for(IloNumSet::Iterator iter(solver.getIntSetValue(E)); iter.ok(); ++iter){
      solver.out() << Names[(IlcInt)*iter] << " ";
   }
   solver.out() << endl;
}

int main() {
   IloEnv env;
   try {
      IloModel model(env);
      
      IloInt i;
      
      // names of employees
      char* Names[20];
      
      // allocation
      for (i=0; i < 20; i++){
         Names[i]=new (env) char[10];
      }
      
      // initialization
      strcpy(Names[0],"Tom");
      strcpy(Names[1],"David");
      strcpy(Names[2],"Jeremy");
      strcpy(Names[3],"Ron");
      strcpy(Names[4],"Joe");
      strcpy(Names[5],"Bill");
      strcpy(Names[6],"Fred");
      strcpy(Names[7],"Bob");
      strcpy(Names[8],"Mario");
      strcpy(Names[9],"Ed");
      strcpy(Names[10],"Carol");
      strcpy(Names[11],"Janet");
      strcpy(Names[12],"Tracy");
      strcpy(Names[13],"Marilyn");
      strcpy(Names[14],"Carolyn");
      strcpy(Names[15],"Cathy");
      strcpy(Names[16],"Inez");
      strcpy(Names[17],"Jean");
      strcpy(Names[18],"Heather");
      strcpy(Names[19],"Juliet");
      
      enum Employee { Tom, David, Jeremy, Ron, Joe, 
                      Bill, Fred, Bob, Mario, Ed, 
                      Carol, Janet, Tracy, Marilyn, Carolyn, 
                      Cathy, Inez, Jean, Heather, Juliet};
      
      IloNumArray Staff(env, 20, 
                        Tom, David, Jeremy, Ron, Joe, 
                        Bill, Fred, Bob, Mario, Ed, 
                        Carol, Janet, Tracy, Marilyn, Carolyn, 
                        Cathy, Inez, Jean, Heather, Juliet);
      
      // variables for employees
      IloNumArray stewardsArray(env, 10, 
                                Tom, David, Jeremy, Ron, Joe, 
                                Bill, Fred, Bob, Mario, Ed);
      IloNumSetVar Stewards(env, stewardsArray, stewardsArray);
      
      IloNumArray hostessesArray(env, 10, 
                                 Carol, Janet, Tracy, Marilyn, 
                                 Carolyn, Cathy, Inez, Jean, 
                                 Heather, Juliet);
      IloNumSetVar Hostesses(env, hostessesArray, hostessesArray);
      
      // variables for languages qualification
      IloNumArray frenchArray(env, 4, Inez, Bill, Jean, Juliet);
      IloNumSetVar French(env, frenchArray, frenchArray);
      
      IloNumArray germanArray(env, 5, Tom, Jeremy, Mario, Cathy, Juliet);
      IloNumSetVar German(env, germanArray, germanArray);
      
      IloNumArray spanishArray(env, 7, 
                               Bill, Fred, Joe, Mario, 
                               Marilyn, Inez, Heather);
      IloNumSetVar Spanish(env, spanishArray, spanishArray);
      
      // all variables in an array
      IloNumSetVarArray memberSets(env, 5, Stewards, Hostesses,
                                   Spanish, French, German);
      
      // variables for flights	
      IloNumSetVarArray crews(env, 10);
      for(i = 0; i < 10; i++)
         crews[i] = IloNumSetVar(env, Staff);
      
      // Constraint : none of the attendants should be obliged to work on two successive flights
      for(i = 0 ; i < 9 ; i++) {
         model.add(IloNullIntersect(env, crews[i], crews[i+1]));
         if (i+2 < 10) 
            model.add(IloNullIntersect(env, crews[i], crews[i+2]));
      }
      
      // Constraint : crew member requirements
      // flight #1 : 4 members, stewards >= 1, hostesses >= 1, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[OL], 4, 
                      IloNumArray(env, 5, 1, 1, 1, 1, 1), memberSets, Staff);
      // flight #2 : 5 members, stewards >= 1, hostesses >= 1, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[1],  5, 
                      IloNumArray(env, 5, 1, 1, 1, 1, 1), memberSets, Staff);
      // flight #3 : 5 members, stewards >= 1, hostesses >= 1, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[2],  5, 
                      IloNumArray(env, 5, 1, 1, 1, 1, 1), memberSets, Staff);
      // flight #4 : 6 members, stewards >= 2, hostesses >= 2, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[3],  6, 
                      IloNumArray(env, 5, 2, 2, 1, 1, 1), memberSets, Staff);
      // flight #5 : 7 members, stewards >= 3, hostesses >= 3, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[4],  7, 
                      IloNumArray(env, 5, 3, 3, 1, 1, 1), memberSets, Staff);
      // flight #6 : 4 members, stewards >= 1, hostesses >= 1, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[5],  4, 
                      IloNumArray(env, 5, 1, 1, 1, 1, 1), memberSets, Staff);
      // flight #7 : 5 members, stewards >= 1, hostesses >= 1, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[6],  5, 
                      IloNumArray(env, 5, 1, 1, 1, 1, 1), memberSets, Staff);
      // flight #8 : 6 members, stewards >= 1, hostesses >= 1, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[7],  6, 
                      IloNumArray(env, 5, 1, 1, 1, 1, 1), memberSets, Staff);
      // flight #9 : 6 members, stewards >= 2, hostesses >= 2, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[8],  6, 
                      IloNumArray(env, 5, 2, 2, 1, 1, 1), memberSets, Staff);
      // flight #10 : 7 members, stewards >= 3, hostesses >= 3, spanish >= 1, french >= 1, german >= 1,
      TeamConstraints(model, crews[9],  7, 
                      IloNumArray(env, 5, 3, 3, 1, 1, 1), memberSets, Staff);
      
      
      IloSolver solver(env);
      solver.extract(model);
      
      // searching for a solution
      if (solver.solve(IloGenerate(env, crews))){
         solver.out() << "Solution" << endl;
         for (IloInt j=0 ; j< crews.getSize() ; j++) {
            solver.out() << "Crew #" << (j+1) << ": " ;
            Print(solver, crews[j], Names);
         }
      }
      else
         solver.out() << "No solution" << endl;
      
      solver.printInformation();
      
   }
   catch (IloException& ex) {
      cerr << "Error: " << ex << endl;
   }
   env.end();
   return 0;
}

/*
Solution
Crew #1: Tom David Jeremy Inez
Crew #2: Ron Joe Bill Fred Cathy
Crew #3: Bob Mario Ed Carol Jean
Crew #4: Tom David Jeremy Janet Tracy Inez
Crew #5: Ron Joe Bill Fred Marilyn Carolyn Cathy
Crew #6: Bob Mario Ed Jean
Crew #7: Tom David Jeremy Carol Inez
Crew #8: Ron Joe Bill Fred Janet Cathy
Crew #9: Bob Mario Ed Tracy Marilyn Jean
Crew #10: Tom David Jeremy Carol Carolyn Inez Heather
Number of fails               : 34
Number of choice points       : 86
Number of variables           : 190
Number of constraints         : 292
Reversible stack (bytes)      : 32184
Solver heap (bytes)           : 160824
Solver global heap (bytes)    : 4044
And stack (bytes)             : 4044
Or stack (bytes)              : 4044
Search Stack (bytes)          : 4044
Constraint queue (bytes)      : 11144
Total memory used (bytes)     : 220328
Elapsed time since creation   : 0.16
*/
