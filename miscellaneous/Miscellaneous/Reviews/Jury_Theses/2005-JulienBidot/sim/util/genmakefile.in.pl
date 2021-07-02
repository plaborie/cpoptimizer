#!/usr/bin/perl

#####################################################################
# we begin
#####################################################################

my %flags;
my %preldflags;
my %postldflags;
my %runs;
my %objs;
my %bench;
my %benchType;
my %extraobjs;

sub generate_makefile {
  open (OUTPUT, ">makefile.shr") || die ("cannot open makefile.shr");
  if ($makeDepend == 1) {
    open (DEPEND, ">makefile.depend");
  }

  #####################################################################
  # copy makefile.skel1
  #####################################################################
  
  open (INPUT, "makefile.skel1") || die ("cannot open makefile.skel1");
  my $line;
  while ($line = <INPUT>) {
    if ( $line =~ /^\s*<\s*list\s*(\w+)\s*(\w+)\s*>\s*$/ ) {
      my $symbolname = $1;
      my $libname = $2;

      if ( ! defined( $libobjs{$libname} ) ) {
        die ("cannot get a list of files for the lib $libname");
      }
      my $filename;
      my $nextline = "";
      print OUTPUT "$symbolname = \\\n";
      foreach $filename ( split( /\s*:\s*/, $libobjs{$libname}) ) {
        print OUTPUT $nextline;
        if ( $filename =~ /^\s*(\w+)\s*$/ ) {
          $filename = $1;
        }
        print OUTPUT "\t\t\$(TMPDIR)$filename.\$\(O\)";
        $nextline = " \\\n";
      }
      print OUTPUT "\n";
    } else {
      print OUTPUT $line;
    }
  }
  close INPUT;
  
  #####################################################################
  # Generate lib entries
  #####################################################################
  
  # Use a default value if nothing was provided
  if (! @srcSubDir) {
    @srcSubDir = qw (.); 
  }

  $dirLength = @srcSubDir;
  $count = 0;
  while ($count++ < $dirLength) {
    $subDirName = $srcSubDir[$count-1];
    $subDir =  "../src/$subDirName/";
    opendir (DIR, $subDir) || die ("Cannot opendir $subDir\n");
    while ($file = readdir(DIR)) {
      if ($file =~ /(.+).cpp$/) {
        &write_src($subDirName, $1);
      } else {
        if (!$file =~ /CVS/) {
          print "Warning $subDir$file is not a source file\n";
        }
      }
    }
    closedir DIR;    
  }
    
  if ($makeDepend == 1) {
    $dirLength = @srcSubDir;
    $count = 0;
    while ($count++ < $dirLength) {
      $subDirName = $srcSubDir[$count-1];
      $subDir =  "../src/$subDirName/";
      opendir (DIR, $subDir) || die ("Cannot opendir $subDir\n");
      while ($file = readdir(DIR)) {
        if ($file =~ /(.+).cpp$/) {
          &find_and_print_includes($subDirName, $1);
        } else {
          if (!$file =~ /CVS/) {
            print "Warning $subDir$file is not a source file\n";
          }
        }
      }
      closedir DIR;
    }
  }

  #####################################################################
  # read bench.rule
  #####################################################################


  $dirLength = @examplesSubDir;
  $count = 0;
  while ($count++ < $dirLength) {
    $subDirName = $examplesSubDir[$count-1];
    $subDir =  "../examples/$subDirName/";
    opendir (DIR, $subDir) || die ("Cannot opendir $subDir\n");
    # look for make.rule
    if (-r "$subDir/bench.rule") {
      open (MAKERULE, "$subDir/bench.rule") || die ("cannot open make.rule");
      my $mode = 0; ## 0 = before, 1 = in
      my $localfile;
      while ($line = <MAKERULE>) {
        if ($mode == 0) {
          if ($line =~ /\s*Example\s+([^\s]+)\s*\{\s*$/) {
            $mode = 1;
            $localfile = $1;
          } 
        } else {
          if ($line =~ /\s*cflags\s*=\s*(.+)\s*$/ ) {
            $flags{$localfile} = $1;
          }
          if ($line =~ /\s*preldflags\s*=\s*(.+)\s*$/ ) {
            $preldflags{$localfile} = $1;
          }
          if ($line =~ /\s*postldflags\s*=\s*(.+)\s*$/ ) {
            $postldflags{$localfile} = $1;
          }
          if ($line =~ /\s*run\s*=\s*(.+)\s*$/ ) {
            $runs{$localfile} = $1;
          }
          if ($line =~ /\s*objs\s*=\s*(.+)\s*$/ ) {
            $extraobjs{$localfile} = $1;
            my @args = split(/,/, $extraobjs{$localfile});
            my $length = @args;
            if ($length == 0) {
              print "Warning, for file $name,";
              print "objs options is used without files\n";
            } else {
              for ($i = 0 ; $i < $length ; $i++) {
                $exf = $args[$i];
                $exf =~ s/^\s*//;
                $exf =~ s/(\s*)$//;
                #
                # $objs{$exf} != "" means that this is not an executable
                # program but it is part of a bench that has multiple files
                #
                $objs{$exf} = $localfile;
              }
            }
          }
          if ($line =~ /\s*bench\s*=\s*(.+)\s*$/ ) {
            $bench{$localfile} = "$bench{$localfile} $1 ";
            $benchType{$1} = "OK";
          }
      
          if ($line =~ /\s*\}\s*/) {
            $mode = 0;
          }
        }
      }
      close MAKERULE;
    }
    closedir DIR;
  }

  #####################################################################
  # create BENCH variables
  #####################################################################

  my @allBench;
  my $benchDirLength = @examplesBenchDir;
  my $count = 0;
  while ($count++ < $benchDirLength) {
    my $subDirName = $examplesBenchDir[$count-1];
    my $subDir =  "../examples/$subDirName/";
    opendir (DIR, $subDir) || die ("Cannot opendir $subDir\n");
    my $file;
    while ($file = readdir(DIR)) {
      if (is_in_notCompileExamples_list($file)) {next;}
      if ($file eq "benchpch.cpp") {next;}
      if ($file =~ /(.+).cpp$/) {
        if (!$objs{$1}) {
          # This is a bench that has no entry in any rule file
          # It will be in benchdefault
          push (@allBench, $1);      
        }
      } else {
        if (!$file =~ /CVS/) {
          print "Warning $subDir$file is not a source file\n";
        }
      }
    }
    closedir DIR;
  }

  @allBench = sort(@allBench);
  my $benchlength = @allBench;

  print OUTPUT "\nBENCH = ";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUTPUT "\\\n\t$allBench[$count-1].bench";
  }

  print OUTPUT "\n\nBENCHEXE = ";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUTPUT "\\\n\t\$(TMPDIR)$allBench[$count-1]\$\(E)";
  }
  print OUTPUT "\n\n";

  print OUTPUT "\n\nPCV = ";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUTPUT "\\\n\t$allBench[$count-1].purecov.pcv";
  }
  print OUTPUT "\n\n";

  print OUTPUT "\n\nbenchcove: \$\(PCV\)";
  print OUTPUT "\n\tcp $firstTestName.purecov.pcv result.pcv\n";
  print OUTPUT "\tpurecov -merge=all.pcv \*.pcv\n\n\n";

  print OUTPUT "\n\nPUREV = ";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUTPUT "\\\n\t$allBench[$count-1].purev";
  }
  print OUTPUT "\n\n";

  print OUTPUT "\n\nPV = ";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUTPUT "\\\n\t$allBench[$count-1].pv";
  }
  print OUTPUT "\n\n";
  print OUTPUT "benchpure : \$\(PV\)\n\n";

  print OUTPUT "\n\nPFY = ";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUTPUT "\\\n\t$allBench[$count-1].pfy";
  }
  print OUTPUT "\n\n";
  print OUTPUT "benchpfy : \$\(PFY\)\n\n";

  $count3 = 0;
  my @allBenchType;
  @allBenchType = sort keys %benchType;
  my $benchTypeLength = @allBenchType;
  while ($count3++ < $benchTypeLength) {
    my $benchT = $allBenchType[$count3-1];
    print OUTPUT "\n$benchT = ";
    $count = 0;
    while ($count++ < $benchlength) {
      $loopfile = $allBench[$count-1];
      if ($bench{$loopfile}) {
        if ($bench{$loopfile} =~ " $benchT ") {
          print OUTPUT "\\\n\t$loopfile.bench";
        }
      }
    }
    print OUTPUT "\n\n$benchT : \$\($benchT\)\n\n";

  }

  print OUTPUT "\ndefault = ";
  $count = 0;
  while ($count++ < $benchlength) {
    $loopfile = $allBench[$count-1];
    if (!$bench{$loopfile}) {
      print OUTPUT "\\\n\t$loopfile.bench";
    }
  }
  print OUTPUT "\n\ndefault : \$\(default\)\n\n";

  #####################################################################
  #   Generate unitst.pl
  #####################################################################
  
  open (OUT2, ">../lib/unittst.pl");
  open (INPUT, "unittst.skel1") || die ("cannot open unitst.skel1");
  while (<INPUT>) {
    print OUT2 "$_";
  }
  close INPUT;
  
  print OUT2 "\nlocal\(\$testsList\) = \"";
  $count = 0;
  while ($count++ < $benchlength) {
    print OUT2 "$allBench[$count-1] ";
  }
  print OUT2 "\";\n";
  open (INPUT, "unittst.skel2") || die ("cannot open unitst.skel2");
  while (<INPUT>) {
    print OUT2 "$_";
  }
  close INPUT;
  close OUT2;

  #####################################################################
  # copy makefile.skel2
  #####################################################################

  open (INPUT, "makefile.skel2") || die ("cannot open makefile.skel2");
  while (<INPUT>) {
    print OUTPUT "$_";
  }
  close INPUT;

  #####################################################################
  # generate perf entries
  #####################################################################

  my @allPerf;
  my $subDir =  "../examples/perf/";
  if (opendir (DIR, $subDir)) {
    my $file;
    while ($file = readdir(DIR)) {
      if (is_in_notCompileExamples_list($file)) {next;}
      if ($file =~ /(.+).cpp$/) {
	if (!$objs{$1}) {
	  push (@allPerf, $1);
	}
      }
      else {
	if (!$file =~ /CVS/) {
	  print "Warning $subDir$file is not a source file\n";
	}
      }
    }
    closedir DIR;
    
    @allPerf = sort(@allPerf);
    my $perflength = @allPerf;
    
    print OUTPUT "\nPERF = ";
    $count = 0;
    while ($count++ < $perflength) {
      print OUTPUT "\\\n\t$allPerf[$count-1].perf";
    }
    
    print OUTPUT "\n\nPERFEXE = ";
    $count = 0;
    while ($count++ < $perflength) {
      print OUTPUT "\\\n\t\$(TMPDIR)$allPerf[$count-1]\$\(E\)";
    }
    print OUTPUT "\n\n";
  }

  #####################################################################
  # generate bench entries
  #####################################################################

  $dirLength = @examplesSubDir;
  $count = 0;
  while ($count++ < $dirLength) {
    $subDirName = $examplesSubDir[$count-1];
    $subDir =  "../examples/$subDirName/";
    opendir (DIR, $subDir) || die ("Cannot opendir $subDir\n");
    my $file;
    while ($file = readdir(DIR)) {
      if ($file eq "."){next;}
      if ($file eq ".."){next;}
      if ($file =~ /.+.cpp~/) {next}
      if ($file =~ /.+.cpp.bak/) {next}
      if ($file =~ /\#/) {next}
      if ($file =~ /(.+).cpp$/) {
        &write_example($subDirName, $1);
      }
      else {
        if (!$file =~ /CVS/) {
          print "Warning $subDir$file is not a source file\n";
        }
      }
    }
    closedir DIR;
  }
  if ($makeDepend == 1) {
    close DEPEND;  
  }
  close OUTPUT;
}

######################################################################
# write subroutines
######################################################################

sub write_example {
  my $dir = shift(@_);
  my $name = shift(@_);
  my $noex= $objs{$name};

  &write_example_compilation($dir, $name);

  if (!$noex && !($name eq $benchPchName)) {
    if ($dir =~ /bench/) {&write_example_bench($dir, $name, $arg);}
    elsif ($dir =~ /src/) {&write_example_bench($dir, $name, $arg);}
    elsif ($dir =~ /perf/) {&write_example_perf($name, $arg);}
  }
}

sub write_src {
  my $dir = shift(@_);
  my $name = shift(@_);

  if ($name eq $ilmSrcFile) {
    print OUTPUT "\$(TMPDIR)$name.\$(O): \$(SRCDIR)$dir\$(SEP)$name.cpp \$(LIBPCH_PCH)\n";
    print OUTPUT "\t\$(CCC) -c \$(LIBCFLAGS) \$(PLOMBIT) \$(USE_LIB_PCH) \$(OBJOUTPUT)$name.\$(O) \$(SRCDIR)$dir\$(SEP)$name.cpp\n\n";
  } elsif ($name eq $libPchName) {
    print OUTPUT "\$(TMPDIR)$name.\$(O): \$(SRCDIR)$dir\$(SEP)$name.cpp\n";
    print OUTPUT "\t\$(CCC) -c \$(LIBCFLAGS) \$(CREATE_LIB_PCH) \$(OBJOUTPUT)$name.\$(O) \$(SRCDIR)$dir\$(SEP)$name.cpp\n\n";
  } else {
    print OUTPUT "\$(TMPDIR)$name.\$(O): \$(SRCDIR)$dir\$(SEP)$name.cpp \$(LIBPCH_PCH)\n";
    print OUTPUT "\t\$(CCC) -c \$(LIBCFLAGS) \$(USE_LIB_PCH) \$(OBJOUTPUT)$name.\$(O) \$(SRCDIR)$dir\$(SEP)$name.cpp\n\n";
  }
}


sub write_example_obj {
  my $dir = shift(@_);
  my $name = shift(@_);
  my $target = shift(@_);
  my $incremental = shift(@_);
  my $cflags = $flags{$name};
  if ($dir eq "bench") {
    # Include options for PCH
    if ($name eq $benchPchName) {
      if ($target) {
        print OUTPUT "\$(TMPDIR)$name.\$(O): \$(EXDIR)$dir\$(SEP)$name.cpp\n";
      }
      print OUTPUT "\t\$(CCC) -c \$(CFLAGS) $cflags \$(CREATE_BENCH_PCH) \$(OBJOUTPUT)$name.\$(O) \$(EXDIR)$dir\$(SEP)$name.cpp\n";
    } else {
      if ($target) {
        print OUTPUT "\$(TMPDIR)$name.\$(O): \$(EXDIR)$dir\$(SEP)$name.cpp \$(BENCHPCH_PCH)\n";
      }
      print OUTPUT "\t\$(CCC) -c \$(CFLAGS) $cflags \$(USE_BENCH_PCH) \$(OBJOUTPUT)$name.\$(O) \$(EXDIR)$dir\$(SEP)$name.cpp\n";
    }
  } else {
    # No PCH in other dirs
    if ($target) {
      print OUTPUT "\$(TMPDIR)$name.\$(O): \$(EXDIR)$dir\$(SEP)$name.cpp\n";
      }
    print OUTPUT "\t\$(CCC) -c \$(CFLAGS) $cflags \$(OBJOUTPUT)$name.\$(O) \$(EXDIR)$dir\$(SEP)$name.cpp\n";
  }
}

sub write_example_compilation {
  my $dir = shift(@_);
  my $name = shift(@_);
  my $flags = $flags{$name};
  my $noex = $objs{$name};
  my $cflags = $flags{$name};
  my $postldflags = $postldflags{$name};
  my $preldflags = $preldflags{$name};

  my $extrao = "";
  if ($extraobjs{$name}) {
    my @args = split(/,/, $extraobjs{$name});
    my $length = @args;
    if ($length == 0) {
      print "Warning, for file $name,";
      print "objs options is used without files\n";
    } else {
      for ($i = 0 ; $i < $length ; $i++) {
        $exf = $args[$i];
        $exf =~ s/^\s*//;
        $exf =~ s/(\s*)$//;
        $extrao = "$extrao \$(TMPDIR)$exf.\$(O)";
      }
    }
  }

  if (!$noex && !($name eq $benchPchName)) {
    print OUTPUT "\$(TMPDIR\)$name\$\(E): \$(TMPDIR\)$name.\$(O) $extrao \$(LIBS\)\n";
    print OUTPUT "\t\$(CCC) \$(CFLAGS) \$(TMPDIR)$name.\$(O) $extrao \$(EXEOUTPUT)$name\$(E) $preldflags \$(LDFLAGS) $postldflags\n";
    print OUTPUT "\$(TMPDIR)$name.pure: \$(TMPDIR)$name.\$(O) $extrao \$(LIBS)\n";
    print OUTPUT "\t\$(PURIFY) \$(CCC) \$(CFLAGS) \$(TMPDIR)$name.\$(O) $extrao \$(EXEOUTPUT)$name.pure $preldflags \$(LDFLAGS) $postldflags\n";
    print OUTPUT "\$(TMPDIR)$name.purev: \$(TMPDIR)$name.\$(O) $extrao \$(LIBS)\n";
    print OUTPUT "\t\$(PURIFY) -log-file=$name.pv \$(CCC) \$(CFLAGS) \$(TMPDIR)$name.\$(O) $extrao \$(EXEOUTPUT)$name.purev $preldflags \$(LDFLAGS) $postldflags\n";
    print OUTPUT "\$(TMPDIR)$name.quant: \$(TMPDIR)$name.\$(O) $extrao \$(LIBS)\n";
    print OUTPUT "\t\$(QUANTIFY) \$(CCC) \$(CFLAGS) \$(TMPDIR)$name.\$(O) $extrao \$(EXEOUTPUT)$name.quant $preldflags \$(LDFLAGS) $postldflags\n";
    print OUTPUT "\$(TMPDIR)$name.purecov: \$(TMPDIR)$name.\$(O) $extrao \$(LIBS)\n";
    print OUTPUT "\t\$(PURECOV) \$(CCC) \$(CFLAGS) \$(TMPDIR)$name.\$(O) $extrao \$(EXEOUTPUT)$name.purecov $preldflags \$(LDFLAGS) $postldflags\n";
  }
  
  write_example_obj($dir, $name, 1);
}

sub write_example_bench {
  my $dir = shift(@_);
  my $name = shift(@_);
  my $arg = shift(@_);

  my $flags = $flags{$name};
  my $cflags = $flags{$name};
  my $postldflags = $postldflags{$name};
  my $preldflags = $preldflags{$name};
 
  if ($continueBenches == 1) {     
    #
    # Benches depend on their source and the lib, so that we do
    # not need to redo all of them (we deleted the EXE) if they
    # have not all been made
    # 
    print OUTPUT "$name.bench: \$(EXDIR)$dir\$(SEP)$name.cpp \$(LIBS) \$(BENCHPCH_PCH)\n";
  
    # Always build the OBJ
    write_example_obj($dir, $name, 0);
    # Always build the EXE
    my $extrao = "";
    if ($extraobjs{$name}) {
      my @args = split(/,/, $extraobjs{$name});
      my $length = @args;
      if ($length == 0) {
        print "Warning, for file $name,";
        print "objs options is used without files\n";
      }
      else {
        for ($i = 0 ; $i < $length ; $i++) {
          $exf = $args[$i];
          $exf =~ s/^\s*//;
          $exf =~ s/(\s*)$//;
          $extrao = "$extrao \$(TMPDIR)$exf.\$(O)";
        }
      }
    }
    print OUTPUT "\t\$(CCC) \$(CFLAGS) \$(TMPDIR)$name.\$(O) $extrao \$(EXEOUTPUT)$name\$(E) $preldflags \$(LDFLAGS) $postldflags\n";
  } else {
    #
    # Normal behaviour : the bench depends on the exe file so that,
    # because the latter is deleted if the bench succeeded, the bench
    # will always be redone
    #
    print OUTPUT "$name.bench: \$\(TMPDIR\)$name\$(E)\n";
  }
  
  print OUTPUT "\t\$\(TOUCH\) $name.bench\n";
  print OUTPUT "\t\$\(DEL\) $name.bench\n";
  
  @args = split(/;/, $runs{$name});
  $length = @args;
  if ($length == 0) {
    print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name\$\(E\)";
    if ($useFilter == 1) {
      print OUTPUT " | \$\(FILTER\)";
    }
    print OUTPUT " > \$\(BDIR\)\$\(SEP\)$name.curr\n";
  }
  else {
    for ($i = 0 ; $i < $length ; $i++) {
      $args[$i] =~ s/^\s*//;
      $args[$i] =~ s/(\s*)$//;
      print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name\$\(E\) $args[$i]";
      if ($useFilter == 1) {
        print OUTPUT " | \$\(FILTER\)";
      }
      if ($i == 0) { print OUTPUT " > "; }
      else { print OUTPUT " >> "; }
      print OUTPUT "\$\(BDIR\)\$\(SEP\)$name.curr\n";
    }
  }
  print OUTPUT "\t\$\(DIFF\) \$\(BREFDIR\)\$\(SEP\)$name.curr \$\(BDIR\)\$\(SEP\)$name.curr >> $name.bench\n";
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.\$(O)\n";
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name\$(E) \n\n";
  
  print OUTPUT "$name.purecov.pcv: \$\(TMPDIR\)$name.purecov\n";
  
  @args = split(/;/, $runs{$name});
  $length = @args;
  if ($length == 0) {
    print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name.purecov\n";
  }
  else {
    for ($i = 0 ; $i < $length ; $i++) {
      $args[$i] =~ s/^\s*//;
      $args[$i] =~ s/(\s*)$//;
      print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name.purecov $args[$i]\n";
    }
  }
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.\$(O)\n";
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.purecov \n\n";

  print OUTPUT "$name.pv: \$\(TMPDIR\)$name.purev\n";
  
  @args = split(/;/, $runs{$name});
  $length = @args;
  if ($length == 0) {
    print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name.purev\n";
  }
  else {
    for ($i = 0 ; $i < $length ; $i++) {
      $args[$i] =~ s/^\s*//;
      $args[$i] =~ s/(\s*)$//;
      print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name.purev $args[$i]\n";
    }
  }
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.\$(O)\n";
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.purev \n\n";

  #
  # Purify for Windows does not work as the Unix versions
  #
  print OUTPUT "$name.pfy: \$\(TMPDIR\)$name.exe\n";
  @args = split(/;/, $runs{$name});
  $length = @args;
  if ($length == 0) {
    print OUTPUT "\t\$\(PURIFY\) /replace=yes /savedata=$name.pfy /savetextdata=$name.pfy.txt \$\(TMPDIR\).\$\(SEP\)$name.exe\n";
  }
  else {
    for ($i = 0 ; $i < $length ; $i++) {
      $args[$i] =~ s/^\s*//;
      $args[$i] =~ s/(\s*)$//;
      if ($i == 0) {
        # So that we really have a file named $name.pfy
        print OUTPUT "\t\$\(PURIFY\) /replace=yes /savedata=$name.pfy /savetextdata=$name.pfy.txt \$\(TMPDIR\).\$\(SEP\)$name.exe $args[$i]\n";
      } else {
        print OUTPUT "\t\$\(PURIFY\) /replace=yes /savedata=$name.$i.pfy /savetextdata=$name.$i.pfy.txt \$\(TMPDIR\).\$\(SEP\)$name.exe $args[$i]\n";
      }
    }
  }
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.obj\n";
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\)$name.exe \n";
  print OUTPUT "\t\$\(DEL\) \$\(TMPDIR\).\$\(SEP\)$name.exe.BAK\n\n";
}

sub write_example_perf {
  my $name = shift(@_);
  my $arg = shift(@_);

  print OUTPUT "$name.perf: \$\(TMPDIR\)$name\$(E)\n";
  print OUTPUT "\t\$\(TOUCH\) $name.perf\n";
  print OUTPUT "\t\$\(DEL\) $name.perf\n";

  @args = split(/;/, $runs{$name});
  $length = @args;
  if ($length == 0) {
    print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name";
    print OUTPUT " > $name.perf\n";
  }
  else {
    for ($i = 0 ; $i < $length ; $i++) {
      $args[$i] =~ s/^\s*//;
      $args[$i] =~ s/(\s*)$//;
      print OUTPUT "\t\$\(TMPDIR\).\$\(SEP\)$name $args[$i]";
      if ($i == 0) { print OUTPUT " > "; }
      else { print OUTPUT " >> "; }
      print OUTPUT "$name.perf\n";
    }
  }
}

######################################################################
#  test element in a list
######################################################################

sub is_in_notCompileExamples_list {
  my $filename = shift(@_);
  foreach  $i (0..$#notCompileExamples) {
    if ("$notCompileExamples[$i].cpp" eq $filename) {
      return 1;
    }
  }
  return 0;
}

######################################################################
#  inclusion  subroutines
######################################################################

# find lines like #include <$includeDirName/ilcint.h>
sub inc_files {
  # open the file
  local $ref_fichier= shift;
  local $file = shift;
  local *FILE;
  open (FILE, $ref_fichier);
  local $fh=\*FILE;
  
  # find the includes
  
  while ( $ligne=<$fh>) {
    if ( $ligne =~ m/<$includeDirName\/(.*\.h)>/ ) {
      my $inc_detecte=$1;
      if (-e "../include/$includeDirName/$1") {
        if (!$inc_hash{$inc_detecte}) {
          $inc_hash{$inc_detecte}=$nbh;
          $nbh++;
          inc_files("../include/$includeDirName/$inc_detecte"); 
        } 
      #} elsif (-e "../examples/include/$1") {
      #  # This must be the libpch.h file. Don't recurse in it 
      #  # or people not using pch will recompile everything either
      } else {
        print "warning, include file $inc_detecte not found for file $ref_fichier\n";
      }
    } 
    # BIG HACK
    elsif ( $ligne =~ m/<\.\.\/src\/ilc\/(.*\.cpp)>/ ) {
      $inc_detecte="../../src/ilc/$1";
      if (!$inc_hash{$inc_detecte}) {
	$inc_hash{$inc_detecte}=$nbh;
	$nbh++;
      }
    };
  };
  close (FILE);
  
}

sub find_and_print_includes {
  my $subDirName = shift(@_);
  my $file = shift(@_);

  $fichier_initial="../src/$subDirName/$file.cpp";
  %inc_hash=();
  %path_hash=();
  $nbh=0;   
  # call for the first time inc_files
  inc_files("$fichier_initial", $file);
  @inc_liste=sort keys %inc_hash;

  $incLength = @inc_liste;
  if ($incLength > 0) {
    $i = 0;
    print DEPEND "\$(TMPDIR)$file.\$\(O\): ";
    while ($i++ < $incLength) {
      $incName = $inc_liste[$i-1];
      if ($path_hash{$incName}) {
        print DEPEND "$path_hash{$incName}\$\(SEP\)$incName ";
      } else {
        print DEPEND "\$\(INCDIR\)\$\(SEP\)$includeDirName\$\(SEP\)$incName ";
      }
    }
    print DEPEND "\n\n";
  }
}

if ( ! defined( $nomessage ) ) {
  print "Generating makefile.shr\n";
}

1;

