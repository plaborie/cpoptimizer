#!/usr/bin/perl

#########################################################################
# xmpgen.pl - This script uses dswgen.pl, which uses dspgen.pl,
#             to create the project for all examples in the
#             distribution on all platforms.
#
#########################################################################

require "dswgen.pl";

sub doTheJob {
  my $msvcVer = $_[0];
  my $libFormat = $_[1];
  my @prodDirs = @ARGV;
  mkdir "../examples/$msvcVer/$libFormat", 0775;
  unlink <../examples/$msvcVer/$libFormat/*.dsp>;
  unlink <../examples/$msvcVer/$libFormat/examples.dsw>;
  mainDSWgen($msvcVer, $libFormat, @prodDirs);
}


if ($ARGV[0] eq "") {
  print "\n";
  print "Usage:\n";
  print "       xmpgen mainProdVersion usedProdVersion...\n";
  print "The first argument (mainProdVersion) must be the one for which ";
  print "examples are made.\n\n";
  print "Example:\n";
  print "       xmpgen sim01 solver50 concert10\n";
  print "will generate MSVC project files for Sim 0.1 examples.\n\n";
  die;
}

mkdir "../examples/msvc5", 0775;

&doTheJob("msvc5", "stat_md");
&doTheJob("msvc5", "stat_mda");
&doTheJob("msvc5", "stat_mt");
&doTheJob("msvc5", "stat_mta");
&doTheJob("msvc5", "stat_st");
&doTheJob("msvc5", "stat_sta");

mkdir "../examples/msvc6", 0775;
&doTheJob("msvc6", "stat_mda");
&doTheJob("msvc6", "stat_mta");
&doTheJob("msvc6", "stat_sta");


