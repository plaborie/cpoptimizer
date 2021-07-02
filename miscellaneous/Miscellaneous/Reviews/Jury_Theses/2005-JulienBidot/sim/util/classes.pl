#!/usr/bin/perl
#
# 

$location = "/usr/work/sim/include";
$classfile = "$location/CLASSES";

sub parse_header
{
  my $file = $_[0];
  open(HEADERFILE, $file) || die( "cannot open $file" );
  my $line;
  while ($line = <HEADERFILE>) {
    if ($line =~ /[ ]*class[ ]*(Il[c|o][a-z|A-Z|0-9]+I)[. {:]*/) { system "echo \"class $1;\" >> $classfile"; }
  }
  close HEADERFILE;
}
  
system "rm -f $classfile";
system "touch $classfile";
opendir (DIR, $location) || die ("Cannot opendir $location\n");
while ($file = readdir(DIR)) {
  if ($file =~ /(.+).h$/) {
    #print " - $file\n"; 
    parse_header($file);
  }
}
close DIR;
system "sort -u $classfile";
system "rm -f $classfile";

