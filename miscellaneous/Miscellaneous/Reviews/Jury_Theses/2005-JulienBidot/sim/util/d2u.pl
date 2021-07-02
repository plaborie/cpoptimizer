#!/bin/perl

#########################################################################
## d2u.pl - perl-scrip equivalent of 'dos2unix -ascii' command available on
##          Sparcs: removes the trailing ^M.
##          Doesn't work under Windows.
#########################################################################

if ("$^O" eq "MSWin32") {
  die("This script does not run correctly on Windows.\nUse 'dos2unix -ascii' on Sparcs.");
}
while (<>) {
  $line = $_;
  $line =~ s/\015//;
  print "$line"; 
}

