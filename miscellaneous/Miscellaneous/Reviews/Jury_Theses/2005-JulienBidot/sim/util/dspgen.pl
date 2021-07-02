#!/usr/bin/perl
#########################################################################
## dspgen.pl - This script generates a project for making either
##             a client or a bench example, in both debug and
##             release mode. It is used by both genmsvc.pl and
##             dswgen.pl. The latter is used by xpmgen.pl
##
## This script should not be called directly!
##########################################################################

my %includeDirs=(
     solver  => "ilsolver",
     concert => "ilconcert",
     sim  => "ilsim"
);

my %libraryNames=(
      solver  => "solver.lib",
      concert => "concert.lib",
      sim  => "sim.lib"
);


my %flags;
my %preldflags;
my %postldflags;
my %runs;
my %objs;
my %bench;
my %benchType;
my %extraobjs;

my $msvcver;
my $libFormat;   
my $progname;
my $subdir;  
my $ilogdir; 
my $where;   
my $options; 
my @prodDirs;
my $machine;
my $compat;
my $withDebugOption;
my $ffver;
my $optflt;
my $arch;
my $hexid;
my $para;
my $genopt;
my $stl;
my $filename;

sub includeDir {
  my $prodVer = shift(@_);
  my %list = %includeDirs;
  my $key;
  my $value;
  my $use = 0;
  while( ( $key, $value ) = each( %list ) ) {
    if ( index( $prodVer, $key ) != -1 ) {
      $use = 1;
      last;
    }
  }
  if ( $use == 0 ) {
    die "Unknown product `$prodVer'\n#";
  }
  return( $value );
}  

sub libraryName {
  my $prodVer = shift(@_);
  my %list = %libraryNames;
  my $key;
  my $value;
  my $use = 0;
  while( ( $key, $value ) = each( %list ) ) {
    if ( index( $prodVer, $key ) != -1 ) {
      $use = 1;
      last;
    }
  }

  
  if ( $use == 0 ) {
    die "Unknown product `$prodVer'\nCheck that the directory in `makefile.site' contains the substring `solver', `concert', or `sim'.\nRelative paths are not allowed.\n#";
  }
  return( $value );
}  

sub genExampleConfiguration {
    local( $debug, $nodefaultlib ) = @_;
    
    print DSPFILE "# PROP BASE Use_MFC 0\n";
    if ( $debug ) {
      print DSPFILE "# PROP BASE Use_Debug_Libraries 0\n";
      print DSPFILE "# PROP BASE Output_Dir \".\\$progname\\Debug\"\n";
      print DSPFILE "# PROP BASE Intermediate_Dir \".\\$progname\\Debug\"\n";
    } else {
      print DSPFILE "# PROP BASE Use_Debug_Libraries 0\n";
      print DSPFILE "# PROP BASE Output_Dir \".\\$progname\\Release\"\n";
      print DSPFILE "# PROP BASE Intermediate_Dir \".\\$progname\\Release\"\n";
    }
    print DSPFILE "# PROP BASE Target_Dir \".\\$progname\"\n";
    print DSPFILE "# PROP Use_MFC 0\n";

    if ( $debug ) {
      print DSPFILE "# PROP Use_Debug_Libraries 1\n";
      print DSPFILE "# PROP Output_Dir \"$ilogdir$prodDirs[0]\\$where\\$msvcver\\$libFormat\\Debug\"\n";
      print DSPFILE "# PROP Intermediate_Dir \"$ilogdir$prodDirs[0]\\$where\\$msvcver\\$libFormat\\Debug\"\n";
    } else {
      print DSPFILE "# PROP Use_Debug_Libraries 0\n";
      print DSPFILE "# PROP Output_Dir \"$ilogdir$prodDirs[0]\\$where\\$msvcver\\$libFormat\"\n";
      print DSPFILE "# PROP Intermediate_Dir \"$ilogdir$prodDirs[0]\\$where\\$msvcver\\$libFormat\"\n";
    }
    print DSPFILE "# PROP Ignore_Export_Lib 0\n";
    print DSPFILE "# PROP Target_Dir \".\\$progname\"\n";
    if ( $debug ) {
      if ($options) { # this (strange) construction is necessary to avoid the warning
        print DSPFILE "# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_CONSOLE\" /YX /GZ /c $options\n";
      } else {
        print DSPFILE "# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D \"WIN32\" /D \"_DEBUG\" /D \"_CONSOLE\" /YX /GZ /c\n";
      }

      print DSPFILE "# ADD CPP /nologo$genopt /W3 /Gm /GX /ZI ";
      for ($i = 0; $i < @prodDirs; $i++) {
        print DSPFILE "/I \"$ilogdir$prodDirs[$i]\\include\" ";
        if ($prodDirs[$i] =~ /sim/) {
          print DSPFILE "/I \"$ilogdir$prodDirs[$i]\\examples\\include\" ";
        }
      }
      
    } else {  # No debug
      
      if ($options) {
        print DSPFILE "# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" /YX /c $options\n";
      } else {
        print DSPFILE "# ADD BASE CPP /nologo /W3 /GX /O2 /D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" /YX /c\n";
      }

      print DSPFILE "# ADD CPP /nologo$genopt /W3 /GX /O2 $optflt ";
      for ($i = 0; $i < @prodDirs; $i++) {
        print DSPFILE "/I \"$ilogdir$prodDirs[$i]\\include\" ";
        if ($prodDirs[$i] =~ /sim/) {
          print DSPFILE "/I \"$ilogdir$prodDirs[$i]\\examples\\include\" ";
        }
      }
  
      if ($progname eq "durablem") {
        print DSPFILE "/D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" $stl $para $compat /FD /Zm500 /c\n";
      } else {
        print DSPFILE "/D \"WIN32\" /D \"NDEBUG\" /D \"_CONSOLE\" $stl $compat /FD /Zm500 /c\n";
      }
    }
    
    print DSPFILE "# SUBTRACT CPP /X /YX\n";
    if ( $debug ) {
      print DSPFILE "# ADD BASE RSC /l 0x409 /d \"_DEBUG\"\n";
      print DSPFILE "# ADD RSC /l 0x409 /d \"_DEBUG\"\n";
    } else {
      print DSPFILE "# ADD BASE RSC /l 0x409 /d \"NDEBUG\"\n";
      print DSPFILE "# ADD RSC /l 0x409 /d \"NDEBUG\"\n";
    }
    print DSPFILE "BSC32=bscmake.exe\n";
    print DSPFILE "# ADD BASE BSC32 /nologo\n";
    print DSPFILE "# ADD BSC32 /nologo\n";
    print DSPFILE "LINK32=link.exe\n";
    if ( $debug ) {
      if ( $nodefaultlib ) {
          $nodefaultlib = "/nodefaultlib:\"$nodefaultlib\"";
      } else {
          $nodefaultlib = '';
      }
      my $debuglibdir = '\\Debug';
      if ($distrib_examples) {
          $debuglibdir = '';
      }
      print DSPFILE "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:$machine /pdbtype:sept\n";
  
  
      print DSPFILE "# ADD LINK32";
      for ($i = 0; $i < @prodDirs; $i++) {
        print DSPFILE " $ilogdir$prodDirs[$i]\\lib\\$msvcver\\$libFormat$debuglibdir\\";
        print DSPFILE libraryName($prodDirs[$i]);
      }
      print DSPFILE " kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /debug /machine:$machine $nodefaultlib /pdbtype:sept\n";
    } else {
      print DSPFILE "# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:$machine\n";
      
      
      print DSPFILE "# ADD LINK32";
      for ($i = 0; $i < @prodDirs; $i++) {
          print DSPFILE " $ilogdir$prodDirs[$i]\\lib\\$msvcver\\$libFormat\\";
          print DSPFILE libraryName($prodDirs[$i]);
      }
      print DSPFILE " kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib wsock32.lib /nologo /subsystem:console /machine:$machine\n";
    }
}

sub genExampleProject {
  $withDebugOption = shift(@_);
  local( $nodefaultlib ) = '';
  if ( $libFormat eq 'stat_sta' ) {
    $nodefaultlib = 'libcpd.lib';
  } elsif ( $libFormat eq 'stat_mta' ) {
    $nodefaultlib = 'libcpmtd.lib';
  }

  $filename = "../$where/$msvcver/$libFormat/$progname.dsp";
  open(DSPFILE, ">$filename") or die ("Cannot open $filename\n");
  
  print DSPFILE "# Microsoft Developer Studio Project File - Name=\"$progname\" - Package Owner=<4>\n";
  print DSPFILE "# Microsoft Developer Studio Generated Build File, Format Version $ffver\n";
  print DSPFILE "# ** DO NOT EDIT **\n";
  print DSPFILE "\n";
  print DSPFILE "# TARGTYPE \"Win32 ($arch) Console Application\" $hexid\n";
  print DSPFILE "\n";
  print DSPFILE "CFG=$progname - Win32 Release\n";
  print DSPFILE "!MESSAGE This is not a valid makefile. To build this project using NMAKE,\n";
  print DSPFILE "!MESSAGE use the Export Makefile command and run\n";
  print DSPFILE "!MESSAGE \n";
  print DSPFILE "!MESSAGE NMAKE /f \"$progname.mak\".\n";
  print DSPFILE "!MESSAGE \n";
  print DSPFILE "!MESSAGE You can specify a configuration when running NMAKE\n";
  print DSPFILE "!MESSAGE by defining the macro CFG on the command line. For example:\n";
  print DSPFILE "!MESSAGE \n";
  print DSPFILE "!MESSAGE NMAKE /f \"$progname.mak\" CFG=\"$progname - Win32 Release\"\n";
  print DSPFILE "!MESSAGE \n";
  print DSPFILE "!MESSAGE Possible choices for configuration are:\n";
  print DSPFILE "!MESSAGE \n";
  print DSPFILE "!MESSAGE \"$progname - Win32 Release\" (based on \"Win32 ($arch) Console Application\")\n";
  if ($withDebugOption) {
    print DSPFILE "!MESSAGE \"$progname - Win32 Debug\" (based on \"Win32 ($arch) Console Application\")\n";
  }
  print DSPFILE "!MESSAGE \n";
  print DSPFILE "\n";
  print DSPFILE "# Begin Project\n";
  if ($ffver =~ /6/) { print DSPFILE "# PROP AllowPerConfigDependencies 0\n"; }
  print DSPFILE "# PROP Scc_ProjName \"\"\n";
  print DSPFILE "# PROP Scc_LocalPath \"\"\n";
  print DSPFILE "CPP=cl.exe\n";
  print DSPFILE "RSC=rc.exe\n";

  if ($withDebugOption) {
    print DSPFILE "\n";
    print DSPFILE "!IF  \"\$(CFG)\" == \"$progname - Win32 Release\"\n";
    print DSPFILE "\n";
  }
  &genExampleConfiguration(0);
  if ($withDebugOption) {
    print DSPFILE "\n";
    print DSPFILE "!ELSEIF  \"\$(CFG)\" == \"$progname - Win32 Debug\"\n";
    print DSPFILE "\n";
    &genExampleConfiguration(1, $nodefaultlib);
    print DSPFILE "\n";
    print DSPFILE "!ENDIF \n";
    print DSPFILE "\n";
  }
  
  print DSPFILE "# Begin Target\n";
  print DSPFILE "\n";
  print DSPFILE "# Name \"$progname - Win32 Release\"\n";
  if ($withDebugOption) {
    print DSPFILE "# Name \"$progname - Win32 Debug\"\n";
  }
  print DSPFILE "# Begin Group \"Source Files\"\n";
  print DSPFILE "\n";
  print DSPFILE "# PROP Default_Filter \"cpp;c;cxx;rc;def;r;odl;hpj;bat;for;f90\"\n";
  print DSPFILE "# Begin Source File\n";
  print DSPFILE "\n";
  print DSPFILE "SOURCE=..\\..\\..\\examples\\$subdir\\$progname.cpp\n";
  print DSPFILE "# End Source File\n";
  print DSPFILE "# End Group\n";
  print DSPFILE "# Begin Group \"Header Files\"\n";
  print DSPFILE "\n";
  print DSPFILE "# PROP Default_Filter \"h;hpp;hxx;hm;inl;fi;fd\"\n";
  print DSPFILE "# End Group\n";
  print DSPFILE "# Begin Group \"Resource Files\"\n";
  print DSPFILE "\n";
  print DSPFILE "# PROP Default_Filter \"ico;cur;bmp;dlg;rc2;rct;bin;cnt;rtf;gif;jpg;jpeg;jpe\"\n";
  print DSPFILE "# End Group\n";
  print DSPFILE "# End Target\n";
  print DSPFILE "# End Project\n";
  
  close DSPFILE or die ("Cannot close $filename");
}

sub genProjectDSP {
  $msvcver    = $_[0];
  $libFormat  = $_[1];
  $progname   = $_[2];
  $subdir     = $_[3];
  $ilogdir    = $_[4];
  $where      = $_[5];
  $options    = $_[9];

  if ( $ilogdir =~ /^\s*\w:/ ) {
    $ilogdir = $ilogdir.'\\';
  } else {
    $ilogdir = '';
  }
  
  @prodDirs = @_[6..8];
  
  if ($_[6] eq "") {
    die "Bad usage. You must use xmpgen.pl";
  } 
  
  if ($where =~ /^lib/) {
      $compat = "/DILSOLVER3";
      $withDebugOption = 1;
  } else {
    # We generate user examples, so no compat flag, and no debug option
      $compat = "";
      $withDebugOption = 0;
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
  elsif ($msvcver =~ /^ia64/) { $arch = "x86"; }
  else  { die "Microsoft Visual C++ target platform not correctly defined"; }
  
  if ($arch =~ /^x86/)   { $machine = "I386";  $hexid="0x0103"; }
  if ($arch =~ /^ALPHA/) { $machine = "ALPHA"; $hexid="0x0601"; }
  
  if    ($libFormat =~ /_mt/) { $genopt = " /MT"; $para = " /DILCUSEMT"; } # don't forget space in front
  elsif ($libFormat =~ /_md/) { $genopt = " /MD"; $para = " /DILCUSEMT"; } # don't forget space in front
  elsif ($libFormat =~ /_st/) { $genopt = " /ML"; $para = ""; }
  else                        { $genopt = "";     $para = ""; }
  
  if ($libFormat =~ /a$/) { $stl = " /D \"IL_STD\""; }
  else               { $stl = ""; }
  
  $endl      = "
  \n";

  &genExampleProject($withDebugOption);
}


1;
