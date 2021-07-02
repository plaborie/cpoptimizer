#!/usr/local/ibin/perl

#########################################################################
## sim.config.pl - The scripts declares source files and directories
##                   required in order to build the Sim library.
##  Is used by 'genmakefile.pl' and 'libgen.pl'
##  not to call directly.
#########################################################################

#########################################################################
## Configuration part
#########################################################################

# All subdirectories in examples containing .cpp for examples
@examplesSubDir = qw (bench);

# All subdirectories in examples containing .cpp for benchmarks
@examplesBenchDir = qw (bench);

# all subdirectories of src for library
@srcSubDir = qw ();

#all examples that must not be compiled (not in benchs)
@notCompileExamples = qw ();

#name of the include prefix
$includeDirName = "ilsim";

#name of the first example (for purecov)
$firstTestName = "test";

#name of the file containing ilm (for the USEFLEXLM compile flag)
$ilmSrcFile = "";

#name of the files to generate pch for the lib
$libPchName = "libpch";
#name of the files to generate pch for the benches
$benchPchName = "benchpch";

$libobjs{'sim'} =
  "ilurandom : ".
  "ilupgsim : ".
  "ilueventstack : ".
  "ilutimeeventmanager : ".
  "iluactivity : ".
  "ilumodel : ".
  "iluscheduler : ".
  "iluevent : ".
  "ilusimulator ";
#name of all .cpp files in library sim


$libobjs{'sim'} = $libobjs{'sim'};
