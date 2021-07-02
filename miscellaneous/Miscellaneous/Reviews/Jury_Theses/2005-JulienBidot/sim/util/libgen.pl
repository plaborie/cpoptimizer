#!/usr/bin/perl

##########################################################################
## libgen.pl - This script generates sim.dsp, which is used to create
##             Sim lib in release and debug. It is called by genmsvc.pl
##
##########################################################################

$nomessage = "please";

require 'sim.config.pl';

sub genLibConfiguration {
  local( $debug, $nodefaultlib ) = @_;

  print OUTPUT "# PROP BASE Use_MFC 0\n";
  if ( $debug ) {
    print OUTPUT "# PROP BASE Use_Debug_Libraries 1\n";
    print OUTPUT "# PROP BASE Output_Dir \".\\sim\\Debug\"\n";
    print OUTPUT "# PROP BASE Intermediate_Dir \".\\sim\\Debug\"\n";
  } else {
    print OUTPUT "# PROP BASE Use_Debug_Libraries 0\n";
    print OUTPUT "# PROP BASE Output_Dir \".\\sim\\Release\"\n";
    print OUTPUT "# PROP BASE Intermediate_Dir \".\\sim\\Release\"\n";
  }
  print OUTPUT "# PROP BASE Target_Dir \".\\sim\"\n";
  print OUTPUT "# PROP Use_MFC 0\n";
  if ( $debug ) {
    print OUTPUT "# PROP Use_Debug_Libraries 1\n";
    print OUTPUT "# PROP Output_Dir \"$simdir\\lib\\$msvcver\\$type\\Debug\"\n";
    print OUTPUT "# PROP Intermediate_Dir \"$simdir\\lib\\$msvcver\\$type\\Debug\"\n";
  } else {
    print OUTPUT "# PROP Use_Debug_Libraries 0\n";
    print OUTPUT "# PROP Output_Dir \"$simdir\\lib\\$msvcver\\$type\"\n";
    print OUTPUT "# PROP Intermediate_Dir \"$simdir\\lib\\$msvcver\\$type\"\n";
  }
  print OUTPUT "# PROP Ignore_Export_Lib 0\n";
  print OUTPUT "# PROP Target_Dir \".\\sim\"\n";
  if ( $debug ) {
    print OUTPUT "# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"ILSOLVER3\" /D \"_CONSOLE\" /YX /GZ /c\n";
    print OUTPUT "# ADD CPP /nologo$genopt /W3 /Gm /GX /ZI /I \"$solverdir\\include\" /I \"$simdir\\include\"/I \"$concertdir\\include\"  /D \"WIN32\" /D \"_DEBUG\" /D \"ILSOLVER3\" /D \"_WINDOWS\" /D \"_CONSOLE\" $stl $para $compat /FD /c\n";
  } else {
    print OUTPUT "# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"ILSOLVER3\" /D \"_CONSOLE\" /YX /c\n";
    print OUTPUT "# ADD CPP /nologo$genopt /W3 /GX /O1 $optflt /I \"$solverdir\\include\" /I \"$simdir\\include\"/I \"$concertdir\\include\"  /D \"WIN32\" /D \"_DEBUG\" /D \"ILSOLVER3\" /D \"_WINDOWS\" /D \"_CONSOLE\" $stl $para $compat /FD /c\n";
  }
  print OUTPUT "# SUBTRACT CPP /X /YX\n";
  if ( $debug ) {
    print OUTPUT "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\n";
    print OUTPUT "# ADD RSC /l 0x409 /d \"_DEBUG\"\n";
  } else {
    print OUTPUT "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\n";
    print OUTPUT "# ADD RSC /l 0x409 /d \"NDEBUG\"\n";
  }
  print OUTPUT "BSC32=bscmake.exe\n";
  print OUTPUT "# ADD BASE BSC32 /nologo\n";
  print OUTPUT "# ADD BSC32 /nologo\n";
  print OUTPUT "LIB32=link.exe -lib\n";
  if ( $debug ) {
    print OUTPUT "# ADD BASE LIB32 /nologo\n";
    print OUTPUT "# ADD LIB32 /nologo\n";
  } else {
    print OUTPUT "# ADD BASE LIB32 /nologo\n";
    print OUTPUT "# ADD LIB32 /nologo\n";
  }
}

sub genLibProject{
  local( $nodefaultlib ) = '';
  if ( $type eq 'stat_sta' ) {
    $nodefaultlib = 'libcpd.lib';
  } elsif ( $type eq 'stat_mta' ) {
    $nodefaultlib = 'libcpmtd.lib';
  }
  $dspfilename = "../lib/$msvcver/$type/sim.dsp";
  $projectname = 'sim';
  open(OUTPUT, ">$dspfilename") or die ("Cannot open $dspfilename\n");
  
  
  print OUTPUT "# Microsoft Developer Studio Project File - Name=\"sim\" - Package Owner=<4>\n";
  print OUTPUT "# Microsoft Developer Studio Generated Build File, Format Version $ffver\n";
  print OUTPUT "# ** DO NOT EDIT **\n";
  print OUTPUT "\n";
  print OUTPUT "# TARGTYPE \"Win32 ($arch) Static Library\" $hexid\n";
  print OUTPUT "\n";
  print OUTPUT "CFG=sim - Win32 Release\n";
  print OUTPUT "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n";
  print OUTPUT "!MESSAGE use the Export Makefile command and run\n";
  print OUTPUT "!MESSAGE \n";
  print OUTPUT "!MESSAGE NMAKE /f \"sim.mak\".\n";
  print OUTPUT "!MESSAGE \n";
  print OUTPUT "!MESSAGE You can specify a configuration when running NMAKE\n";
  print OUTPUT "!MESSAGE by defining the macro CFG on the command line. For example:\n";
  print OUTPUT "!MESSAGE \n";
  print OUTPUT "!MESSAGE NMAKE /f \"sim.mak\" CFG=\"sim - Win32 Release\"\n";
  print OUTPUT "!MESSAGE \n";
  print OUTPUT "!MESSAGE Possible choices for configuration are:\n";
  print OUTPUT "!MESSAGE \n";
  print OUTPUT "!MESSAGE \"sim - Win32 Release\" (based on \"Win32 ($arch) Static Library\")\n";
  print OUTPUT "!MESSAGE \"sim - Win32 Debug\" (based on \"Win32 ($arch) Static Library\")\n";
  print OUTPUT "!MESSAGE \n";
  print OUTPUT "\n";
  print OUTPUT "# Begin Project\n";
  if ($ffver =~ /6/) { print OUTPUT "# PROP AllowPerConfigDependencies 0\n"; }
  print OUTPUT "# PROP Scc_ProjName \"\"\n";
  print OUTPUT "# PROP Scc_LocalPath \"\"\n";
  print OUTPUT "CPP=cl.exe\n";
  print OUTPUT "RSC=rc.exe\n";

  print OUTPUT "\n";
  print OUTPUT "!IF  \"\$(CFG)\" == \"$projectname - Win32 Release\"\n";
  print OUTPUT "\n";
  &genLibConfiguration(0);
  print OUTPUT "\n";
  print OUTPUT "!ELSEIF  \"\$(CFG)\" == \"$projectname - Win32 Debug\"\n";
  print OUTPUT "\n";
  &genLibConfiguration(1, $nodefaultlib);
  print OUTPUT "\n";
  print OUTPUT "!ENDIF\n";
  print OUTPUT "\n";

  print OUTPUT "# Begin Target\n";
  print OUTPUT "\n";
  print OUTPUT "# Name \"sim - Win32 Release\"\n";
  print OUTPUT "# Name \"sim - Win32 Debug\"\n";
  print OUTPUT "\n";
  
  #recuperer liste des fichiers src dans LIBOBJS de makefile.skel1
  
  #####	$srcDir="../src";
  #####	opendir (DIR, $srcDir) or die ("Cannot opendir $srcDir\n");
  #####	my $file;
  #####	while ($file = readdir(DIR)) {
      #####	if ($file eq "."){next;}
  	#####	if ($file eq ".."){next;}
  	#####	if ($file =~ /.+.cpp~/) {next}
  	#####	if ($file =~ /.+.cpp.bak/) {next}
  	#####	if ($file =~ /\#/) {next}
          #####	if ($file eq "altmode.cpp"){next;}
          #####	if ($file eq "ttblcont.cpp"){next;}
  	#####	if ($file =~ /(.+).cpp\b/) {
  	    #####	print OUTPUT "# Begin Source File\n";
  	    #####	print OUTPUT "\n";
  	    #####	print OUTPUT "SOURCE=..\\..\\$srcDir\\$file\n";
  	    #####	print OUTPUT "# End Source File\n";
  	#####	}
      #####	}
  
  $ilcSrcDir="..\\src\\ilc";
  $iloSrcDir="..\\src\\ilo";
  my $filename;
  
  if ( ! defined( $libobjs{'msvc-graph'} ) ) {
    die "the list of files for msvc is not defined.";
  }
  foreach $filename ( split( /\s*:\s*/, $libobjs{'msvc-graph'} ) ) {
    if ( $filename =~ /^\s*(\w+)\s*$/ ) {
      $filename = $1;
    }
    print OUTPUT "# Begin Source File\n";
    print OUTPUT "\n";
    print OUTPUT "SOURCE=..\\..\\$ilcSrcDir\\$filename.cpp\n";
    print OUTPUT "# End Source File\n";
  }
  foreach $filename ( split( /\s*:\s*/, $libobjs{'msvc-ilosim'} ) ) {
    if ( $filename =~ /^\s*(\w+)\s*$/ ) {
      $filename = $1;
    }
    print OUTPUT "# Begin Source File\n";
    print OUTPUT "\n";
    print OUTPUT "SOURCE=..\\..\\$iloSrcDir\\$filename.cpp\n";
    print OUTPUT "# End Source File\n";
  }
  
  print OUTPUT "# End Target\n";
  print OUTPUT "# End Project\n";
  
  close OUTPUT or die "Cannot close $dspfilename";
  print "Wrote $dspfilename\n";
}

sub genLibDSP {
  $simdir = $_[0];
  $concertdir= $_[1];
  $solverdir = $_[2];
  $msvcver   = $_[3];
  $type      = $_[4];
  
  
  if ($type eq "") {
    print "Usage: libgen sim_version concert_version solver_version msvcver porttype\n";
    print "Example: libgen sim01 solver44 msvc5 stat_st\n";
    die;
  }
  
  if    ($msvcver =~ /5$/) { $ffver = "5.00"; }
  elsif ($msvcver =~ /6$/) { $ffver = "6.00"; }
  else  { die "Microsoft Visual C++ version number not correctly defined"; }
  
  
  if    ($msvcver =~ /^msvc5$/) { $optflt = ""; }
  elsif ($msvcver =~ /^alphavc5$/) { $optflt = "/QAieee /QArd "; }
  elsif ($msvcver =~ /6$/) { $optflt = "/Op"; }
  else  { die "Microsoft Visual C++ version number not correctly defined"; }
  
  if    ($msvcver =~ /^msvc/)  { $arch = "x86"; }
  elsif ($msvcver =~ /^alpha/) { $arch = "ALPHA"; }
  else  { die "Microsoft Visual C++ target platform not correctly defined"; }
  
  if ($arch =~ /^x86/)   { $machine = "I386";  $hexid="0x0104"; }
  if ($arch =~ /^ALPHA/) { $machine = "ALPHA"; $hexid="0x0601"; }
  
  if    ($type =~ /_mt/) { $genopt = " /MT"; $para = " /DILCUSEMT"; } # don't forget space in front
  elsif ($type =~ /_md/) { $genopt = " /MD"; $para = " /DILCUSEMT"; } # don't forget space in front
  elsif ($type =~ /_st/) {
      $para = "";
      if ($msvcver =~ /^msvc5/) { $genopt = " /ML"; } # don't forget space in front
      else                      { $genopt = " /ML"; } # don't know why it was empty before, but didn't work that way
  }
  else                   { $genopt = ""; }
  
  
  if ($type =~ /a$/) { $stl = " /D \"IL_STD\""; }
  else               { $stl = ""; }

  &genLibProject();
}

1;
