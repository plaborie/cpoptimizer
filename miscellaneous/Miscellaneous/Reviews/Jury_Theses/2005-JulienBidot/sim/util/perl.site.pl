#!/usr/bin/perl

#########################################################################
## perl.site.pl - Configuration file used by 'benchcomp.pl',
##                'genmakefile.pl', and 'makeport.pl'
## Used to define $toolPath (where to find the tools dir), margins
## for dircomp.pl, $diff executable to use ('diff' or 'fc'), etc...
#########################################################################

#where to find the tools dir
$toolPath = 'c:\\tools';

#add tools dir for scripts
push(@INC, "$toolPath");


# margins (in percentage) for bench comparisons :
# dircomp.pl and benchcomp.pl
@dcdmarge= ( 0,  # Number of fails
	  0,  # Number of choice points
	  0,  # Number of variables 
	  0,  # Number of Constraints
	  2,  # Reversible stack
	  2,  # Solver heap 
	  2,  # Solver global heap 
	  2,  # And stack
	  2,  # Or stack 
	  2,  # Search Stack
	  2,  # Constraint queue
	  2,  # Total memory used
	  5   # Running time since creation 
	);



