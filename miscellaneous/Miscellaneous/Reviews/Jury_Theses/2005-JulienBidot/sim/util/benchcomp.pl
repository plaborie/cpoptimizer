#!/usr/local/ibin/perl
#

# The next 3 lines are required if this script
# may be launched from outsite the directory
# where it resides
use File::Basename;
fileparse_set_fstype("MSDOS");
push(@INC, dirname("$0"));

require "perl.site.pl";
require "benchcomp.in.pl";

