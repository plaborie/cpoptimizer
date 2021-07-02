#!/usr/bin/perl

#########################################################################
## dswgen.pl - This script uses dspgen.pl for examples in the distribution 
##             for a particular combination of platforms and subplatforms. 
## It calls dspgen.pl and is called by xpmgen.pl
##
#########################################################################
require "dspgen.pl";

#----------------------------------------------------------------------
#
# List of variables used in this file 
#
#------------------------------------------------------------

my $msvcver;    
my $libFormat;  
my @prodDirs;   
my $ffver;
my $ilogdir     = "c:\\ilog";  # Default ILOG products directory
my $subDir      = "src";       # Customer examples directory

sub mainDSWgen {

    $msvcver     = $_[0];
    $libFormat   = $_[1];
    @prodDirs    = @_[2..4]; # WARNING: 4 should be changed into :number of parameters"

    if ($_[2] eq "") {
      die "Bad usage. You must use xmpgen.pl";
    }
    
    if    ($msvcver =~ /5$/) { $ffver = "5.00"; }
    elsif ($msvcver =~ /6$/) { $ffver = "6.00"; }
    else  { die "Microsoft Visual C++ version number not correctly defined\n"; }
    
    opendir (DIR, "../examples/$subDir") or die ("Cannot opendir ../examples/$subDir\n");
    
    $filename = "../examples/$msvcver/$libFormat/examples.dsw";
    open(EXAMPLE_DSP_FILE, ">$filename") or die ("Cannot open $filename\n");
    
    print EXAMPLE_DSP_FILE "Microsoft Developer Studio Workspace File, Format Version $ffver\n";
    print EXAMPLE_DSP_FILE "# WARNING: DO NOT EDIT OR DELETE THIS WORKSPACE FILE!\n";
    &separator;
    my $file;
    while ($file = readdir(DIR)) {
      if ($file eq "."){next;}
      if ($file eq ".."){next;}
      if ($file eq "findbugs.cpp"){next;}
      if ($file =~ /.+.cpp.+/) {next}
      if ($file =~ /\#/) {next}
      if ($file =~ /(.+).cpp\b/) {
        &project($1);
        my $distrib_examples = "yes";
        genProjectDSP($msvcver, $libFormat, $1, "src", $ilogdir, "examples", @prodDirs);
      }
    }
    
    &global;
    
    close EXAMPLE_DSP_FILE;
    closedir DIR;
    print "done:    [$filename].\n";
}

sub separator {
  print EXAMPLE_DSP_FILE "\n" . "#" x 79 . "\n\n";
}

sub project {
  my $projname = shift(@_);
  if ($projname eq "findbugs") {
      return;
  }
  if ($projname eq "nqexcept") {
      return;
  }
  print EXAMPLE_DSP_FILE "Project: \"$projname\"=.\\$projname.dsp - Package Owner=<4>\n";
  print EXAMPLE_DSP_FILE "\n";
  print EXAMPLE_DSP_FILE "Package=<5>\n";
  print EXAMPLE_DSP_FILE "{{{\n";
  print EXAMPLE_DSP_FILE "}}}\n";
  print EXAMPLE_DSP_FILE "\n";
  print EXAMPLE_DSP_FILE "Package=<4>\n";
  print EXAMPLE_DSP_FILE "{{{\n";
  print EXAMPLE_DSP_FILE "}}}\n";
  &separator;
}

sub global {
  print EXAMPLE_DSP_FILE "Global:\n";
  print EXAMPLE_DSP_FILE "\n";
  print EXAMPLE_DSP_FILE "Package=<5>\n";
  print EXAMPLE_DSP_FILE "{{{\n";
  print EXAMPLE_DSP_FILE "}}}\n";
  print EXAMPLE_DSP_FILE "\n";
  print EXAMPLE_DSP_FILE "Package=<3>\n";
  print EXAMPLE_DSP_FILE "{{{\n";
  print EXAMPLE_DSP_FILE "}}}\n";
  &separator;
}

1;
