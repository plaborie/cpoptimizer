#!/bin/perl

#########################################################################
## u2d.pl - perl-scrip equivalent of 'unix2dos -ascii' command available on
##          Sparcs: adds the trailing ^M.
##          Doesn't work under Windows.
#########################################################################            
                
if ("$^O" eq "MSWin32") {
  die("This script does not run correctly on Windows.\nUse 'unix2dos -ascii' on Sparcs.");
}
while (<>) {
  chop;
  print "$_\015\n"; 
}

