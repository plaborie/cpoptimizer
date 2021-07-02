#!/usr/bin/perl

##########################################################################
## genmsvc.pl - This script generates the projects for creating the
##              bench examples and the lib. It calls dspgen.pl and
##              libgen.pl respectively for the two tasks.
##
##########################################################################

# All subdirectories in examples containing .cpp for examples
my @examplesSubDir = qw (src bench team perf);


require "dspgen.pl";
require "libgen.pl";

&main();

sub genProjectDSPandDSW {
    my $simdir  = $_[0];
    my $concertdir = $_[1];
    my $solverdir  = $_[2];
    my $options    = $_[3];

    my $count = 0;
    my $benchDirLength = @examplesSubDir;

    while ($count++ < $benchDirLength) {
	$subDirName = $examplesSubDir[$count-1];
	my $subDir =  "../examples/$subDirName/";
	my $filename = "../lib/$msvcver/$formatlib/examples_$subDirName.dsw";
	open(DSWFILE, ">$filename") or die ("Cannot open $filename\n");
	print DSWFILE "Microsoft Developer Studio Workspace File, Format Version $ffver\n";
	print DSWFILE "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n";
	&writeSeparator;
	
	opendir (DIR, $subDir) or die ("Cannot opendir $subDir\n");
	my $file;
	while ($file = readdir(DIR)) {
	    if ($file eq "."){next;}
	    if ($file eq ".."){next;}
	    if ($file =~ /.+.cpp~/) {next}
	    if ($file =~ /.+.cpp.bak/) {next}
	    if ($file =~ /\#/) {next}
	    if ($file =~ /(.+).cpp\b/) {
		&writeProjectInDSW($1);

		if ($options) {
		    genProjectDSP($msvcver, $formatlib, $1, $subDirName, "-", "lib", $simdir, $concertdir, $solverdir, $options);
		}
		else {
		    genProjectDSP($msvcver, $formatlib, $1, $subDirName, "-", "lib", $simdir, $concertdir, $solverdir);
		}
	    }
	    else {
		if (!$file =~ /CVS/) {
		    print "Warning $subDir$file is not a source file\n";
		}
	    }
	}

	&writeProjectInDSW("sim");

	&writeGlobalInDSW;
	closedir DIR;
	close DSWFILE  or die ("Cannot close $filename\n");
	print "Wrote $filename\n";
    }
    genLibDSP($simdir, $concertdir, $solverdir, $msvcver, $formatlib);
}

sub writeSeparator {
    print DSWFILE "\n" ."#" x 79 . "\n\n";
}

sub writeProjectInDSW {
    # Write the brief description of a project (ie a dsp file) to be included in a .dsw file

  my $projname = shift(@_);

  print DSWFILE "Project: \"$projname\"=.\\$projname.dsp - Package Owner=<4>\n";
  print DSWFILE "\n";
  print DSWFILE "Package=<5>\n";
  print DSWFILE "{{{\n";
  print DSWFILE "}}}\n";
  print DSWFILE "\n";
  print DSWFILE "Package=<4>\n";
  print DSWFILE "{{{\n";
  #####	print DSWFILE "Begin Project Dependency\n";
  #####	print DSWFILE "Project_Dep_Name sim\n";
  #####	print DSWFILE "End Project Dependency\n";
  print DSWFILE "}}}\n";
  &writeSeparator;
}

sub writeGlobalInDSW {
  print DSWFILE "Global:\n";
  print DSWFILE "\n";
  print DSWFILE "Package=<5>\n";
  print DSWFILE "{{{\n";
  print DSWFILE "}}}\n";
  print DSWFILE "\n";
  print DSWFILE "Package=<3>\n";
  print DSWFILE "{{{\n";
  print DSWFILE "}}}\n";
  &writeSeparator;
}


sub main {
        
    $msvcver       = $ARGV[0];
    $formatlib     = $ARGV[1];
    
    if ($ARGV[1] eq "") {
	print "Usage: genmsvc.pl msvcver formatlib\n";
	print "Example: genmsvc.pl msvc5 stat_st\n";
	die;
    }

    local $ffver;
    if    ($msvcver =~ /5$/) { 
	$ffver = "5.00"; 
    }
    elsif ($msvcver =~ /6$/) { 
	$ffver = "6.00"; 
    }
    else  { die "Microsoft Visual C++ version number not correctly defined\n"; }
    
    mkdir "../examples", 0775;
    mkdir "../examples/$msvcver", 0775;
    mkdir "../examples/$msvcver/$formatlib", 0775;
    

    #------------------------------------------------------------
    # Get the directories of Solver and Sim on Unix and PC
    # 
    
    open (SITE, "makefile.site") or die "Cannot find makefile.site";
    while ($line = <SITE>) {
	$line =~ /^\s*(\w*)\s*=\s*(.*)/;
	if ($1 and $2) {
	    $hvar{$1}= $2;
	}
    }
    close SITE;


    #------------------------------------------------------------
    # Determine whether the script genmsvc.pl is launched from Unix 
    # or from Windows and call genProjectDSPandDSW with the correct
    # variables
    
    # to be improved !
    if (-e "c:" ) { 
	genProjectDSPandDSW($hvar{'SIMDIRPC'}, 
			    $hvar{'CONCERTDIRPC'}, 
			    $hvar{'SOLVERDIRPC'}, 
			    $hvar{'OPTIONS'});
    }
    else {
	genProjectDSPandDSW($hvar{'SIMDIRUNIX'}, 
			    $hvar{'CONCERTDIRUNIX'}, 
			    $hvar{'SOLVERDIRUNIX'}, 
			    $hvar{'OPTIONS'});
    }
    
}






















