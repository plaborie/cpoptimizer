#!/usr/local/ibin/perl

## --------------------------------------------------------------------------
## unittst.pl
## --------------------------------------------------------------------------

## --------------------------------------------------------------------------
## Global variables
## --------------------------------------------------------------------------
# List of unit tests to be launched


local($testsList) = "empty jspparse test_event_manager test_event_stack test_pgsim test_random_variable test_unc_activity ";
# List of non automatic tests
# These test names should not be in the $testsList variable
local($nonAutoTestsList) = "";
# List of tests which have to be launched with purify
# These test names must be in the $testsList variable.
local($pureTestsList) = "";
# Tests using librairies which don't have dll (ex : gantt)
# These test names must be in the $testsList variable.
local($noDllTests) = "";
# Tests an executable 
# This list specifie the binary that you want to launch
# for example if you want to launch the executable foo with the option -v 2.0 and 
# an other executable toto you must type local($executableList) = "foo -v 2.0 , toto";
local($executableList) = "";

## --------------------------------------------------------------------------
## Required modules
## --------------------------------------------------------------------------
push(@INC, "./..");
push(@INC, "/nfs/solver/scripts/VTT");
require "testutil/perl/tpltests.pl";

## --------------------------------------------------------------------------
main();

