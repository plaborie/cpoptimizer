#!/usr/local/ibin/perl

#########################################################################
## genmakefile.pl - Generates the shared makefile 'makefile.shr' (and
##                  'makefile.depend' if applicable)
## options:
##   -d : creates dependency file (makefile.depend)
##   -c : allows benches to be 'c'ontinued, that is they only depend
##        on the source and lib files, not on the presence (or absence)
##        of the final executable.
##  Uses scripts: 'perl.site.pl', 'genmakefile.in.pl'.
##  Uses files: makefile.skel1, makefile.skel2   
#########################################################################

require "perl.site.pl";


# load info on how to build the lib and how to execute examples
require "sim.config.pl";

#########################################################################
## Execution part
#########################################################################

#load the main file
require "genmakefile.in.pl";

$makeDepend = 0;
#$useDependList = 0;
#@dependList = ();

#
# If this variable equals 1, then the benches will depend on the
# source file and the lib, instead of the EXE file. Thus, if the
# run has been stopped, the already succeeded benches will not be
# redone.
#
$continueBenches = 1;

#
# If this variable equals 1, then the result of each bench is
# sent to the program indicated in the make macro $(FILTER) and
# the output of this filter is what gets stored in th .curr
#
$useFilter = 0;

if ($#ARGV >= 0) {
  while ($ARGV[0] =~ /^-/) {
    if ($ARGV[0] =~ /d/) {
      $makeDepend=1;
    };
    shift @ARGV;
  };
};

if (!(-e "makefile.depend")) {
  $makeDepend = 1;
}

#call the generate subroutine
&generate_makefile;

