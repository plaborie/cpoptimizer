#!/usr/bin/perl
#
# Script-Velu-Qui-Fait-Tout (tm)
#

#
# Todo list:
#
#   - Specifying '-m -user' should do both, instead of just the user
#     makefiles.
#
#   - Some options could be implied by others:
#        -clean  ->   -m
#        -build  ->   -clean
#        -bench  ->   -build
#     Thus doing 'perl makeport.pl -bench stat_sta6' really does
#     'perl makeport.pl -m -clean -build -bench stat_sta6'
#

use Cwd;

#########################################################################
## some global var

$verbose = 0;
$userFiles = 0;   # if =0, deal with lib. if =1, deal with examples.
$verifyFiles = 1;
$checkMakefiles = 0;
$createMakefiles = 0;
$createDeliveries = 0;
$printSpecs = 0;
$createTarZ = 0;
$createInstall = 0;
$cleanLibs = 0;
$buildLibs = 0;
$makeBenches = 0;
$buildingEscrow = 0;
$doHelp = 0;
$buildLocally = 0;

$configFile = "./PORTS.CONFIG";
$productFile = "./PRODUCT.CONFIG";
$escrowConfigFile = "./ESCROW.CONFIG";
$metaFileName = "ishield.setup.meta";
$setupFileName = "ishield.setup";
$workDir = cwd();
$localDir = "..";


#########################################################################
## General default infos (for delivery.*)
#########################################################################

$Product = "None";
$ProductShort = "None";
$Version = "0.0";
$IlmVersion = "2.6";
$IlmFeatures = "";
$Path = "";
$tarzDir = "";
$escrowFile = "escrow.tar.Z";
$LibFileName = "libsim.a";
$Doc = "to be done";
$Validate = "\"make\" in examples directory\n";

$DependsOn = "";
$diff = "";
$echo = "echo";

# load user defined defaults if any
if (-r "perl.site.pl") {
  require "perl.site.pl";
}

if ("$diff" eq "") {
  # Default values if not specified in perl.site.pl
  if ("$^O" eq "MSWin32") {
    $diff = "diff";
  } else {
    $diff = "/nfs/solver/cplus/work/util/mydiff";
  }
}


sub mysystem {
  my $line = $_[0];
  if ($verbose > 2) { print "$line\n"; }
  system($line) == 0
       or die "system $line failed: $? - $!"
}

################################################
#########       port global info      ##########
################################################

sub parse_global
{
  my $line = $_[0];
  if ($line =~ /^\s*[Pp]roduct\s*=\s*(.+)\s*$/) { $Product = $1; }
  elsif ($line =~ /^\s*[Pp]roduct[Ss]hort\s*=\s*(.+)/) { $ProductShort = $1; }
  elsif ($line =~ /^\s*[Ii]lm[V|v]ersion\s*=\s*(.+)/) { $IlmVersion = $1; }
  elsif ($line =~ /^\s*[Vv]ersion\s*=\s*(.+)/) { $Version = $1; }
  elsif ($line =~ /^\s*[Ii]lm[F|f]eatures\s*=\s*(.+)/) { $IlmFeatures = $1; }
  elsif ($line =~ /^\s*[Pp]ath\s*=\s*(.+)\s*$/) { $Path = $1; }
  elsif ($line =~ /^\s*[Pp]ath[Pp]refix\s*=\s*(.+)\s*$/) { $PathPrefix = $1; }
  elsif ($line =~ /^\s*[Tt]ar[Zz][Tt]op[Dd]ir\s*=\s*(.+)\s*$/) { $tarzDir = $1; }
  elsif ($line =~ /^\s*[Ee]scrow[Ff]ile[Nn]ame\s*=\s*(.+)\s*$/) { $escrowFile = $1; }
  elsif ($line =~ /^\s*[Ll]ib[Ff]ile[Nn]ame\s*=\s*(.+)\s*$/) { $LibFileName = $1; }
  elsif ($line =~ /^\s*[Dd]epends[Oo]n\s*=\s*(.+)\s*$/) { $DependsOn = $1; }
  elsif ($line =~ /^\s*[Mm][Tt][Ee]xamples\s*=\s*(.+)\s*$/) { @MtExamples = split(' ', $1); }

  else { return 0; }
  return 1;
}

################################################
##########      port group info     ############
################################################

sub parse_group_info 
{
  my $curGroup = $_[0];
  my $line = $_[1];
  if ($line =~ /^\s*[Pp]ortclass\s*=\s*(.+)\s*$/) { $GportClass{$curGroup} = $1; }
  elsif ($line =~ /^\s*[Pp]orts\s*=\s*(.+)\s*$/) { $portGroups{$curGroup} .= "$1 "; }
  elsif ($line =~ /^\s*[Tt]ar[Zz]\s*=\s*(.+)\s*$/) { $Gtarz{$curGroup} = $1; }
  elsif ($line =~ /^\s*OSname\s*=\s*(.+)\s*$/) { $Gosname{$curGroup} = $1; }
  elsif ($line =~ /^\s*[Hh]ardware\s*=\s*(.+)\s*$/) { $Ghardware{$curGroup} = $1; }
  elsif ($line =~ /^\s*[Cc]ompiler\s*=\s*(.+)\s*$/) { $Gcompiler{$curGroup} = $1; }
  elsif ($line =~ /^\s*[Dd]elivery\s*=\s*(.+)\s*$/) { $Gdelivery{$curGroup} = $1; }
  elsif ($line =~ /^\s*[Tt]ype\s*=\s*unix\s*$/) { $Gtype{$curGroup} = 0; }
  elsif ($line =~ /^\s*[Tt]ype\s*=\s*msvc\s*$/) { $Gtype{$curGroup} = 1; }
  elsif ($line =~ /^\s*[Vv]alidate\s*=\s*(.+)\s*$/) { $Gvalidate{$curGroup} .= "$1\n                          "; }
  elsif ($line =~ /^\s*[Dd]oc\s*=\s*(.+)\s*$/) { $Gdoc{$curGroup} = $1; }
  elsif ($line =~ /^\s*\}\s*/) {
    if ($portGroups{$curGroup} eq "") {
      die ( "group '$curGroup' contains no port\n" );
    }

    ### set up some default values
    if (not $GportClass{$curGroup}) { $GportClass{$curGroup}=$curGroup; }
    if (not $Gtarz{$curGroup}) { $Gtarz{$curGroup}="sim.$GportClass{$curGroup}.tar.gz"; }
    if (not $Gdelivery{$curGroup}) { $Gdelivery{$curGroup}="delivery.$GportClass{$curGroup}"; }
    if (not $Gtype{$curGroup}) { $Gtype{$curGroup}=0; }
    if (not $Gdoc{$curGroup}) { $Gdoc{$curGroup}=$Doc; }
    if (not $Gvalidate{$curGroup}) { $Gvalidate{$curGroup}=$Validate; }

    ### Fill the type of each ports
    my @portNames = split( ' ', $portGroups{$curGroup});
    while(@portNames) {
      $Ptype{$portNames[0]}=$Gtype{$curGroup};
      shift @portNames;
    }

    if ($verbose>0) { print "[$portGroups{$curGroup}]\n"; }
    return 0;
  }
  return 1;
}

################################################
##########     parse port info       ###########
################################################

sub copy_port
{
  my $dst = $_[0];
  my $src = $_[1];

## $Pportdir,$Parchoptions and $P*comments are *not* copied.
## otherwise, there's something wrong in the config

  $Psystem{$dst} = $Psystem{$src};
  $Pformat{$dst} = $Pformat{$src};
  $Plibfilename{$dst} = $Plibfilename{$src};
  $Pos{$dst} = $Pos{$src};
  $Posversion{$dst} = $Posversion{$src};
  $Pcc{$dst} = $Pcc{$src};
  $Pcompiler{$dst} = $Pcompiler{$src};

  $Pliboptions{$dst} = $Pliboptions{$src};
  $Pexoptions{$dst} = $Pexoptions{$src};
  $Pdebug{$dst} = $Pdebug{$src};

  $Pdiff{$dst} = $Pdiff{$src};

  $Gbuilton{$dst} = $Gbuilton{$src};
  $Gbuiltwith{$dst} = $Gbuiltwith{$src};
 
  $Pldmtflags{$dst} = $Pldmtflags{$src};
  $Pmtflagsilm{$dst} = $Pmtflagsilm{$src};
  $Pmtflags{$dst} = $Pmtflags{$src};
  $Pldoptflags{$dst} = $Pldoptflags{$src};

  $Paroptions{$dst} = $Paroptions{$src};
  $Pccname{$dst} = $Pccname{$src};
}

sub parse_port_info 
{
  my $curPort = $_[0];
  my $line = $_[1];
  if ($line =~ /^\s*[Ss]ystem\s*=\s*(.+)\s*$/) { $Psystem{$curPort} = $1; }
  elsif ($line =~ /^\s*[Cc][Cc]\s*=\s*(.+)\s*$/) { $Pcc{$curPort} = $1; }
  elsif ($line =~ /^\s*[Ff]ormat\s*=\s*(.+)\s*$/) { $Pformat{$curPort} = $1; }
  elsif ($line =~ /^\s*[Ll]ib[Ff]ile[Nn]ame\s*=\s*(.+)\s*$/) { $Plibfilename{$curPort} = $1; }
  elsif ($line =~ /^\s*[Cc]ompiler\s*=\s*(.+)\s*$/) { $Pcompiler{$curPort} = $1; }
  elsif ($line =~ /^\s*[Aa]rch[Oo]ptions\s*=\s*(.+)\s*$/) { $Parchoptions{$curPort} = $1; }
  elsif ($line =~ /^\s*[Ll]ib[Oo]ptions\s*=\s*(.+)\s*$/) { $Pliboptions{$curPort} = $1; }
  elsif ($line =~ /^\s*[Ee]x[Oo]ptions\s*=\s*(.+)\s*$/) { $Pexoptions{$curPort} = $1; }
  elsif ($line =~ /^\s*[Dd]ebug\s*=\s*(.+)\s*$/) { $Pdebug{$curPort} = $1; }
  elsif ($line =~ /^\s*[Dd]iff\s*=\s*(.+)\s*$/) { $Pdiff{$curPort} = $1; }
  elsif ($line =~ /^\s*[Dd]elivery[Cc]omment\s*=\s*(.+)\s*$/) { $Pdcomments{$curPort} .="$1\n                "; }
  elsif ($line =~ /^\s*[Uu]ser[Cc]omment\s*=\s*(.+)\s*$/) { $PuserComments{$curPort} .= "##$1"; }

  elsif ($line =~ /^\s*[Cc]omment\s*=\s*(.+)\s*$/) { $Pcomments{$curPort} .= "##$1"; }
  elsif ($line =~ /^\s*[Oo][Ss]version\s*=\s*(.+)\s*$/) { $Posversion{$curPort} = $1; }
  elsif ($line =~ /^\s*[Oo][Ss]\s*=\s*(.+)\s*$/) { $Pos{$curPort} = $1; }

  elsif ($line =~ /^\s*[Pp]ort[Dd]ir\s*=\s*(.+)\s*$/) { $Pportdir{$curPort} = $1; }
  elsif ($line =~ /^\s*[Bb]uilton\s*=\s*(.+)\s*$/) { $Gbuilton{$curPort} = $1; }
  elsif ($line =~ /^\s*[Bb]uiltwith\s*=\s*(.+)\s*$/) { $Gbuiltwith{$curPort} = $1; }

  elsif ($line =~ /^\s*ldmtflags\s*=\s*(.+)\s*$/) { $Pldmtflags{$curPort} = $1; }  
  elsif ($line =~ /^\s*ldoptflags\s*=\s*(.+)\s*$/) { $Pldoptflags{$curPort} = $1; }
  elsif ($line =~ /^\s*mtflagsilm\s*=\s*(.+)\s*$/) { $Pmtflagsilm{$curPort} = $1; }
  elsif ($line =~ /^\s*mtflags\s*=\s*(.+)\s*$/) { $Pmtflags{$curPort} = $1; }
  elsif ($line =~ /^\s*aroptions\s*=\s*(.+)\s*$/) { $Paroptions{$curPort} = $1; }
  elsif ($line =~ /^\s*ccname\s*=\s*(.+)\s*$/) { $Pccname{$curPort} = $1; }

  elsif ($line =~ /^\s*\}\s*/) {
    die ( "port '$curPort' has no system defined\n" ) if (not $Psystem{$curPort});
    die ( "port '$curPort' has no format defined\n" ) if (not $Pformat{$curPort});

    ### set up some default values
    if (not exists $Plibfilename{$curPort}) { $Plibfilename{$curPort}=$LibFileName; }
    if (not exists $Pranlib{$curPort}) { $Pranlib{$curPort}=$echo; }
    if (not exists $Ptemplate{$curPort}) { $Ptemplate{$curPort}=$echo; }
    if (not exists $Pdiff{$curPort}) { $Pdiff{$curPort}=$diff; }
    if (not exists $Pportdir{$curPort}) { $Pportdir{$curPort} = "$Psystem{$curPort}_$Pos{$curPort}_$Pcompiler{$curPort}"; }
    if (not exists $Posversion{$curPort}) { $Posversion{$curPort}=$Pos{$curPort}; }
    return 0;
  }
  #else { print " ??? $line\n"; }
  return 1;
}

################################################
########      read product file         ########
################################################

sub parse_product_file
{
  my $portClass;
  foreach $portClass (@classesToDo) {
    $tarAlone{$portClass} = 0;
  }

  open(PRODUCTCONFIG, $productFile) || die( "cannot open $productFile" );
  if ($verbose>0) { print "* Reading $productFile...\n"; }
  my $line;
  my %hash_file_list;
  my $optionsFinished = 0;
  while ($line = <PRODUCTCONFIG>) {
    if ($line =~ /^\s*\#/) { next; }
    if ($line =~ /^\s*$/) { next; }
    if (parse_global($line)) { next; }

    #
    # We have a non empty line that was not handled by parse_global.
    # It must be a line describing which file will make up in the TarZ
    #

    if ($optionsFinished == 0) {
      #
      # Get the correct setup for Path and TarzTopDir
      #
      if ($buildLocally) {
	$Path = $localDir;
      } else {
	if ($Path eq "") {    
	  if ($PathPrefix eq "") {
	    die "You specified no Path nor PathPrefix.\n";
	  }    
	  my $VersionShort = $Version;
	  $VersionShort =~ s/\.//g;
	  $Path = "$PathPrefix/$ProductShort$VersionShort";
	} elsif (! $PathPrefix eq "") {
	  print "Warning: both Path and PathPrefix specified. Using Path.\n";
	} 
      }
      if ($tarzDir eq "") {
	my $VersionShort = $Version;
	$VersionShort =~ s/\.//g;
	$tarzDir = "$ProductShort$VersionShort";
      }
      $optionsFinished = 1;
    }
    
    #if (! $createTarZ) { next; }

    # Look for a port or portClass specification
    my @specificPortClasses;
    if ($line =~ /\(\s*(.*)\s*\)/) {
      @specificPortClasses = split (' ', $1);
      $line =~ s/\(\s*(.*)\s*\)//;
    }
    if (@specificPortClasses) {
      if ($verbose > 2) {print "This line is specific to '@specificPortClasses': $line";}
      # Mark those portClasses so that their tarZ is not built with
      # other classes.
      my $skipThisLine = 1;
      foreach $specificPortClass (@specificPortClasses) {
	$tarAlone{$specificPortClass} = 1;
	if (@classesToDo == 1) {
	  if ($classesToDo[0] eq $specificPortClass) {
	    if ($verbose > 2) {print "Keeping the line: $line";}
	    $skipThisLine = 0;
	  }
	} elsif (@classesToDo > 1) {
	  # Verify that there is not several specific portClass
	  # specified on command line. But do that only if we are
	  # creating the tarZ (it is ok for user's makefile generation
	  if ($createTarZ) {
	    foreach $portClass (@classesToDo) {
	      if ($tarAlone{$portClass}) {
		die "A specific portClass for distributed files ($portClass) must be made alone !\n";
	      }
	    }
	  }
	}
      }
      if ($skipThisLine) { next; }
    }

    my @src = split( ' ', $line);
    if (! defined $src[0]) { next; }
    
    my $removing = 0;
    if ($src[0] eq "-") {
      $removing = 1;
      shift @src;
    } elsif ($src[0] eq "+") {
      shift @src;
    }
    # Several files can be specified on the same line
    while(@src) {
      if ($verbose>0) { 
        if ($removing) {
          print " - $src[0]\n"; 
        } else {
          print " + $src[0]\n"; 
        }
      }
      # Get the list of files that this spec refers to.
      # If -buildlocally was used, $Path is in fact $localdir.
      my @fileList = glob("$Path/$src[0]");
      while (@fileList) {
        my $file = $fileList[0];
        $file =~ s/^$Path\///;
        if ($removing) {
          if ($verbose>1) { print "    $file\n"; }
          delete $hash_file_list{$file};
        } else {
          if ($verbose>1) { print "    $file\n"; }
          $hash_file_list{$file} = 1;
        }
        shift @fileList;
      }
      shift @src;
    }
  }
  
  close PRODUCTCONFIG;

  # The list of distributed files is the sorted list of keys 
  @distribFiles = sort keys %hash_file_list;
  
  # Now compute the list of distributed examples (just the file
  # names, not with the path).
  if ($verbose > 0) { print "List of distributed examples: "; }
  $examplesCount = 0;
  for( $i=0; $i<@distribFiles; $i++ ) {
    my $fileName = $distribFiles[$i];
    if ($fileName =~ /examples\/src\/(.+)\.cpp/) {
      $distributedExamples[$examplesCount++] = $1;
      if ($verbose > 0) { 
	print "$1"; 
	my $index = 0;
	for ($index = 0; $index < @MtExamples; $index++) {
	  if ($1 eq "$MtExamples[$index]") {
	    print "(MT)";
	  }
	}
	print " ";
      }      
    }
  }
  if ($verbose > 0) { print "\n"; }
}

################################################
########    read port configurations    ########
################################################

my $curPort;
my $mode = 0; ## 0 = general, 1=reading Port, 2=reading Group

sub parse_config_file
{
  open(PORTCONFIG, $configFile) || die( "cannot open $configFile" );
  if ($verbose>0) { print "* Reading $configFile...\n"; }
  my $curGroup;
  my $line;
  while ($line = <PORTCONFIG>) {
    if ($line =~ /^\s*\#/) { next; }
    if ($line =~ /^\s*$/) { next; }
    if ($mode == 0) {
      if ($line =~ /\s*[Gg]roup\s+([^\s]+)\s*\{\s*$/)
      {
        $curGroup = $1;
        if (exists $portGroups{$curGroup}) {
          die( "Error: group $curGroup multiply defined...\n" );
        }
        if ($verbose>0) { print " * Group: $curGroup\n"; }
        $portGroups{$curGroup} = ""; ## just make it exists...
        push @groupLabels,$curGroup;
        $mode = 2;
      }
      elsif ($line =~ /\s*[Pp]ort\s*([^\s]+)\s*(.*)\s*\{\s*/)
      {
        $curPort = $1;
        if (exists $ports{$curPort}) {
          die( "Error: port $curPort multiply defined...\n" );
        }
        $ports{$curPort} = 0; ## just make it exists...
        $mode=1;
        if ($2=~ /\s*from\s*([^\s]+)\s*/)
        { 
          if ($verbose>0) { print "     (copying $curPort specs from $1...)\n"; }
          if (not exists $ports{$1}) {
            die( "Error: port $1 is not yet defined...\n" );
          }
          copy_port($curPort, $1);
        }
      }
      else { parse_global($line); }
    }
    elsif ($mode==1) { if (parse_port_info( $curPort, $line )==0) { $mode=0; } }
    elsif ($mode==2) { if (parse_group_info( $curGroup, $line )==0) { $mode=0; } }
    else { die "wow. parse error.\n"; }
  }
  close PORTCONFIG;
}


################################################
#########     store ports to do       ##########
################################################

sub add_port {
  my $type = $_[0]; shift @_;
  while(@_) {
    my $p = $_[0];
    die ("port '$p' is not defined.\n") if (not exists $ports{$p});
    $portsToDo[$portsCount++] = $p;
    $Ptype{$p} = $type;
    shift @_;
  }
}

sub expand {
  my $i;
  foreach  $i (0..scalar @groupLabels) {
    if ($groupLabels[$i] eq $_[0]) {
       my @names = split( ' ', $portGroups{$_[0]} );
       add_port($Gtype{$_[0]}, @names);
       push(@classesToDo, $_[0]);
       return;
    }
  }
  # The type is stored in the group. Let's find which group
  # the port is in.
  $portFound = 0;
  foreach  $i (0..scalar @groupLabels) {
      my @names = split( ' ', $portGroups{$groupLabels[$i]} );
      foreach $j (0..scalar @names) {
          if ($names[$j] eq $_[0]) {
              $portFound = 1;
              add_port($Gtype{$groupLabels[$i]}, $_[0]);
          }
      }
  }
  if (not $portFound) {
    if ("$^O" eq "MSWin32") {
      add_port(1, $_[0]);  # default type = msvc
    } else {
      add_port(0, $_[0]);  # default type = unix
    }
  }
}

################################################
#########      makefile header       ###########
################################################

sub print_makefile_comments {

  print OUTPUT "## automatically generated\n";

  if (exists $Pcomments{$curPort}) {
    print OUTPUT "##\#### comments:\n";
    my @comments = split( '##', $Pcomments{$curPort} );
    while(@comments) {
       print OUTPUT "## $comments[0]\n";
       shift @comments;
    }
    print OUTPUT "##\n";
  }
  print OUTPUT "\n";
}

sub print_usermake_comments {

  if (exists $PuserComments{$curPort}) {
    my @comments = split( '##', $PuserComments{$curPort} );
    while(@comments) {
       print OUTPUT "# $comments[0]\n";
       shift @comments;
    }
    print OUTPUT "# \n\n";
  }
}

################################################
##########        Unix makefiles     ###########
################################################

sub generate_makefile_unix
{
  open (OUTPUT, ">$_[0]") || die ("cannot open $_[0]\n" );

  print_makefile_comments(OUTPUT);

  print OUTPUT "SYSTEM = $Pportdir{$curPort}\n";
  print OUTPUT "LIBFORMAT = $Pformat{$curPort}\n";
  print OUTPUT "\n";
  print OUTPUT "CCC = $Pcc{$curPort}\n";
  if (exists $Parchoptions{$curPort}) {
    print OUTPUT "ARCHOPTIONS = $Parchoptions{$curPort}\n";
  }
  print OUTPUT "LIBOPTIONS = $Pliboptions{$curPort}\n";
  print OUTPUT "DEBUG = $Pdebug{$curPort} -DNDEBUG\n";
  print OUTPUT "\n";

  print OUTPUT "DIFF = $Pdiff{$curPort}\n";
  print OUTPUT "RANLIB = $Pranlib{$curPort}\n";
  print OUTPUT "TEMPLATELIB = $Ptemplate{$curPort}\n";
  print OUTPUT "AROPTIONS = $Paroptions{$curPort}\n";
  print OUTPUT "CCNAME = $Pccname{$curPort}\n";

  print OUTPUT "\n";
  print OUTPUT "MTFLAGS = $Pmtflags{$curPort}\n";
  print OUTPUT "ILMMTFLAGS = $Pmtflagsilm{$curPort}\n"; 
  print OUTPUT "LDMTFLAGS = $Pldmtflags{$curPort}\n";
  print OUTPUT "LDOPTFLAGS = $Pldoptflags{$curPort}\n";
  print OUTPUT "\n";

  if ($curPort eq "alpha") {
    open(MAKEFILEUNIX, "makefile.unix") || die( "cannot open makefile.unix" );
    my $line;
    while ($line = <MAKEFILEUNIX>) {
      print OUTPUT $line
    }
    close MAKEFILEUNIX;
  }
  else {
    print OUTPUT "include ../../../util/makefile.unix\n";
  }

  close OUTPUT;
  print "done:    [$_[0]].\n";
}

sub generate_user_makefile_unix
{
  open (OUTPUT, ">$_[0]") || die ("cannot open $_[0]\n" );

   # used to be the "UserMake" part

  print_usermake_comments(OUTPUT);

  print OUTPUT "SYSTEM = $Pportdir{$curPort}\n";
  print OUTPUT "LIBFORMAT = $Pformat{$curPort}\n";
  print OUTPUT "\n";
  print OUTPUT "CCC = $Pcc{$curPort} $Parchoptions{$curPort} $Pexoptions{$curPort}\n";
  print OUTPUT "\n";

  if (exists $Pldoptflags{$curPort}) {
    print OUTPUT "ADDLDFLAGS = $Pldoptflags{$curPort}\n";
  }
  print OUTPUT "MTFLAGS = $Pmtflags{$curPort}\n";  # *not* $Pmtflagsilm
  print OUTPUT "LDMTFLAGS = $Pldmtflags{$curPort}\n";
  print OUTPUT "\n";

  print OUTPUT "DEBUG = $Pdebug{$curPort} -DNDEBUG\n";
  print OUTPUT "\n";

    # append some common stuff
  
  my $shareFile = "$localDir/util/UserMake.share";
  if (not -r $shareFile) {
    die( "$shareFile does not exist\n" );
  }
  open (INPUT, "<$shareFile") || die ("cannot open $shareFile");
  while ($line = <INPUT>) {
    print OUTPUT $line;
  }
  close INPUT;

  print OUTPUT "#---------------------------------------------------------\n";
  print OUTPUT "# FILES\n";
  print OUTPUT "#---------------------------------------------------------\n";
  print OUTPUT "SCHTEST = ";
  my $index = 0;
  for ($index = 0; $index < @distributedExamples; $index++) {
    print OUTPUT "$distributedExamples[$index] ";
    if ($index % 5 == 4) {
      print OUTPUT "\\\n          ";
    }
  }

  print OUTPUT "\n\n";
  print OUTPUT "execute: \$\(SCHTEST\)\n";
  for ($index = 0; $index < @distributedExamples; $index++) {
    print OUTPUT "\t./$distributedExamples[$index]\n";
  }

  print OUTPUT "\n";
  print OUTPUT "clean:\n";
  print OUTPUT "\t/bin/rm -rf *.o\n";
  print OUTPUT "\t/bin/rm -rf \$\(SCHTEST\)\n";
  print OUTPUT "\ttouch out.out\n";
  print OUTPUT "\t/bin/rm -rf *.out\n";

  print OUTPUT "\n";
  for ($index = 0; $index < @distributedExamples; $index++) {
    my $name = $distributedExamples[$index];
    my $i;
    my $isMT = 0;
    for ($i = 0; $i < @MtExamples; $i++) {
      if ($name eq $MtExamples[$i]) {
	$isMT = 1;
      }
    }
    print OUTPUT "$name.o: \$\(DEMOSRCDIR\)/$name.\$\(SUFFIX\)\n";
    print OUTPUT "\t\$\(CCC\) \$\(CFLAGS\) ";
    if ($isMT) {
      print OUTPUT "\$\(MTFLAGS\) ";
    }
    print OUTPUT "\$\(DEMOSRCDIR\)/$name.\$\(SUFFIX\) -c\n";

    print OUTPUT "$name: $name.o \$\(SCHLIBS\)\n";
    print OUTPUT "\t\$\(CCC\) \$\(CFLAGS\) $name.o -o $name ";
    if ($isMT) {
      print OUTPUT "\$\(COMPLETE_LDMTFLAGS\)";
    } else {
      print OUTPUT "\$\(LDFLAGS\)";
    }
    print OUTPUT "\n\n";
  }

  close OUTPUT;

  print "done:    [$_[0]].\n";
  print "##Warning## Please check that the version numbers for the products\n";
  print "            you depend on are correct in $shareFile.\n";
}

sub check_makefile_unix
{
  my $target = "/tmp/tmpmakefile";
  if ($userFiles) { generate_user_makefile_unix( $target ); }
  else { generate_makefile_unix( $target ); }
  print "diffing with $_[0]\n";
  mysystem "$Pdiff{$curPort} $target $_[0]";
  unlink $target;
}

################################################
##########       MSVC makefiles      ###########
################################################

sub generate_makefile_msvc
{
  open (OUTPUT, ">$_[0]") || die ("cannot open $_[0]\n" );

  print_makefile_comments(OUTPUT);

  print OUTPUT "SYSTEM = $Pportdir{$curPort}\n";
  print OUTPUT "LIBFORMAT = $Pformat{$curPort}\n";
  print OUTPUT "\n";

  if (exists $Parchoptions{$curPort}) {
    print OUTPUT "ARCHOPTIONS = $Parchoptions{$curPort}\n";
  }
  print OUTPUT "LIBOPTIONS = $Pliboptions{$curPort}\n";
  print OUTPUT "\n";
  
  print OUTPUT "DIFF = $Pdiff{$curPort}\n";
  print OUTPUT "RANLIB = $Pranlib{$curPort}\n";
  print OUTPUT "TEMPLATELIB = $Ptemplate{$curPort}\n";
  print OUTPUT "AROPTIONS = $Paroptions{$curPort}\n";
  print OUTPUT "CCNAME = $Pccname{$curPort}\n";
  
  print OUTPUT "\n";
  print OUTPUT "MTFLAGS = $Pmtflags{$curPort} $Pmtflagsilm{$curPort} \n";
  print OUTPUT "LDMTFLAGS = $Pldmtflags{$curPort}\n";
  print OUTPUT "LDOPTFLAGS = $Pldoptflags{$curPort}\n";
  print OUTPUT "\n";
  print OUTPUT "\!include ../../../util/makefile.msv\n";

  close OUTPUT;
  print "done:    [$_[0]].\n";
}

sub generate_user_makefile_msvc
{
    require "dswgen.pl";
    my $VersionShort = $Version;
    $VersionShort =~ s/\.//g;
    my @prodDirs;
    $prodDirs[0] = "$ProductShort$VersionShort";
    my @depends = split(' ', $DependsOn);
    my $i;
    for ($i = 0; $i < @depends; $i++) {
      $prodDirs[$i + 1] = $depends[$i];
    }
    my $location = "$localDir/examples/$Pportdir{$curPort}/$Pformat{$curPort}";
    check_dir("$localDir/examples/$Pportdir{$curPort}");
    check_dir("$localDir/examples/$Pportdir{$curPort}/$Pformat{$curPort}");
    unlink "$location/*.dsp";
    unlink "$location/examples.dsw";
    print "Calling dswgen.pl\n";
    mainDSWgen($Pportdir{$curPort}, $Pformat{$curPort}, @prodDirs, @distributedExamples, @MtExamples);
    
    # DSPs and DSWs must be converted to CR/LF if generated on Unix
    if (!("$^O" eq "MSWin32")) {
        if ($verbose>0) { print "Converting files to DOS :\n"; }
        opendir (DIR, $location) || die ("Cannot opendir $location\n");
        while ($file = readdir(DIR)) {
            if ($file =~ /(.+).ds[pw]$/) {
                if ($verbose>1) { print " - $file\n"; }
                mysystem "perl u2d.pl < $location/$file > $location/$file.dos && mv -f $location/$file.dos $location/$file";
            }
        }
        close DIR;
        if ($verbose>0) { print "ok.\n"; }
    }
}

sub check_makefile_msvc
{
  my $target = "c:\\makefile.tmp";
  generate_makefile_msvc( $target );
  print "diffing with $_[0]\n";
  mysystem "$Pdiff{$curPort} $target $_[0]";
  unlink $target;
}

################# lock ###################

sub check_dir_and_lock
{
  my $file = $_[0];
  if (-r "$file.lock") {
    system "echo !!! Directory [$file] is already locked by `cat $file.lock`.";
    print "If it's ok, remove the file [$file.lock].\n";
    die( "Bailing out.\n" );
  }

  open(LOCK, ">$file.lock") || die( "cannot open lock file $file.lock" );
  close(LOCK);
  mysystem "echo \$USER > $file.lock";
  if (-r "$file") {
    mysystem( "\\rm \-rf $file" );
  }
  mysystem "mkdir $file";
}

sub unlock_dir
{
  my $file = $_[0];
  mysystem "\\rm $file.lock";
}

########### generation part ##########

sub check_dir {
  if (not -r "$_[0]") {
     print "new dir: [$_[0]]...\n";
     die "Error:$!\n" if (not mkdir("$_[0]", 0775));
  }
}

sub check_target {
  check_dir($_[0]);
  check_dir($_[1]);
  my $target = "$_[1]/$_[2]";
  if (-r $target) {
     print "moving:  [$target] to \"$_[2].old\"\n";
     die( "Error:$!\n" ) if (not rename( $target, "$target.old"));
  }
}

######################################

sub deal_with_makefiles
{
  while(@_) {
    $curPort = $_[0];
    my $sysDir = "$localDir/lib/$Pportdir{$curPort}";
    my $endDir = "$sysDir/$Pformat{$curPort}";

    my $exSysDir = "$localDir/examples/$Pportdir{$curPort}";
    my $exEndDir = "$exSysDir/$Pformat{$curPort}";

    if ($Ptype{$curPort} == 0) {
      print "########### Port: $curPort (unix) #################\n";
      if ($checkMakefiles) { 
        if ($userFiles) { check_makefile_unix( "$exEndDir/makefile" ); }
        else { check_makefile_unix( "$endDir/makefile" ); }
      }
      elsif ($createMakefiles) {
        if ($userFiles) {
          check_target( $exSysDir, $exEndDir, "makefile" );
          generate_user_makefile_unix( "$exEndDir/makefile" );
        }
        else {
          check_target( $sysDir, $endDir, "makefile" );
          generate_makefile_unix( "$endDir/makefile" );
          check_dir( "$endDir/bench");
        }
      }
    }
    else {
      print "########### Port: $curPort (msvc) #################\n";
      if ($checkMakefiles) {
        if ($userFiles) { print "DSP's and DSW's can not be checked !\n"; }
        else { check_makefile_msvc( "$endDir/makefile" ); }
      }
      elsif ($createMakefiles) {
        if ($userFiles) {
          generate_user_makefile_msvc( "$exEndDir/examples.dsw" );
        }
        else {
          check_target( $sysDir, $endDir, "makefile" );
          generate_makefile_msvc( "$endDir/makefile" );
          check_dir( "$endDir/bench");
        }
      }
    }
    shift @_;
  }
}

sub makeMakefileSite
{
  my $dir = "$localDir/util";
  if (! -r "$dir/makefile.site") {
    print "Copying makefile.site.gentilly to makefile.site\n";
    open(makeSiteGentillyH, "<$dir/makefile.site.gentilly") or
      die "Cannot open file $dir/makefile.site.gentilly: $!\n";
    open(makeSiteH, ">$dir/makefile.site") or
      die "Cannot open file $dir/makefile.site: $!\n";
    while($line = <makeSiteGentillyH>) {
      print makeSiteH $line;
    }
    close(makeSiteH);
    close(makeSiteGentillyH); 
  }
}

sub makeMakefileShr 
{
  my $dir = "$localDir/util";
  if ("$^O" eq "MSWin32") {
    $dir = "$localDir\\util";
  }  
  #if (! -r "$dir/makefile.shr") {
    # Look in makefile.site to find a command line for genmakefile
    open(makeSiteH, "<$dir/makefile.site") or
      die "Cannot open file $dir/makefile.site: $!\n";
    $genmakefileOptions = "-c";
    while(<makeSiteH>) {
      $genmakefileOptions = $1   if /^\s*GENMAKEFILEOPTIONS\s*=\s*(.+)\s*$/;
    }
    close(makeSiteH);
    if ($verbose>0) { print "Launching genmakefile.pl in directory $dir\n"; }
    mysystem "cd $dir && perl genmakefile.pl $genmakefileOptions";
  #}
}

################################################
############# print ports specs ################
################################################

sub print_port_list
{
  my $i;
  for( $i=0; $i<@groupLabels; $i++)
  {
    print " * Group \"$groupLabels[$i]\":\n";
    my @names = split( ' ', $portGroups{$groupLabels[$i]} );
    while(@names) {
      print "    \"$names[0]\"\n";
      shift @names;
    }
  }
}

sub print_product_specs
{
    print " ======================================\n";
    print "        Product: $Product\n";
    print "        Version: $Version\n";
    print "    Ilm version: $IlmVersion\n";
    print "   Ilm features: $IlmFeatures\n";
    print "           Path: $Path\n";
}

sub print_specs
{
  while(@_) {
    $curPort = $_[0];

    print " --------------------------------------\n";
    if ($Ptype{$curPort} == 0) { print "Port: $curPort (unix)\n"; }
    else { print "Port: $curPort (msvc)\n"; }
    print " - SYSTEM = $Pportdir{$curPort}\n";
    print " - LIBFORMAT = $Pformat{$curPort}\n";
    print " - CCC = $Pcc{$curPort}\n";
    if (exists $Parchoptions{$curPort}) {
      print " - OPTIONS = $Parchoptions{$curPort}\n";
    }
    print " - LIBOPTIONS = $Pliboptions{$curPort}\n";
    print " - DEBUG = $Pdebug{$curPort} -DNDEBUG\n";
    print " - DIFF = $Pdiff{$curPort}\n";
    print " - RANLIB = $Pranlib{$curPort}\n";
    print " - TEMPLATELIB = $Ptemplate{$curPort}\n";
    print " - AROPTIONS = $Paroptions{$curPort}\n";
    print " - CCNAME = $Pccname{$curPort}\n";
    print " - MTFLAGS = $Pmtflags{$curPort}";
    if ($Pmtflagsilm{$curPort}) { print "  ($Pmtflagsilm{$curPort})"; }
    print "\n";
    print " - LDMTFLAGS = $Pldmtflags{$curPort}\n";
    print " - LDOPTFLAGS = $Pldoptflags{$curPort}\n";
    shift @_;
  }
  print " --------------------------------------\n";
}

################################################
######## generation of delivery texts ##########
################################################

sub generate_delivery 
{
  my $group = $_[0];
  print "Creating: $Path/$Gdelivery{$group} <= ";

  my $tarz = "$Path/TarZ/$Gtarz{$group}";
  if (not -r $tarz) {
    print "abort.\n TarZ file [$tarz] is not built!\n";
    return;
  }
  my $csum = "...";
  my @Date = ("-", "-", "-", "-");
  my $Size = "...";
  if ($tarz) {
    open SUM, "sum -r $tarz |";
    if (<SUM> =~ /([0-9]+)\s*([0-9]+)\s*(.+)/) { $csum = "$1 $2"; }
    close SUM;
    my @St = stat "$tarz";
    @Time = localtime($St[9]);
    $Date[0] = (Jan, Feb, Mar, Apr, May, Jun, Jul, Aug, Sep, Oct, Nov, Dec)[$Time[4]];
    $Date[1] = $Time[3];  # month day
    $Date[2] = $Time[2];
    $Date[3] = $Time[1];
    $Size = $St[7];
  }

  my $target = "$Path/$Gdelivery{$group}";
  open (OUTPUT, ">$target") || die ("cannot open $target\n" );
  print OUTPUT "Product Info\n\n";
  print OUTPUT "       Name: $Product\n";
  print OUTPUT "       Version: $Version\n";
  print OUTPUT "       Ilm version: $IlmVersion\n";
  print OUTPUT "       Ilm Feature(s): $IlmFeatures\n";
  print OUTPUT "       Portclass: $GportClass{$group}\n";
  print OUTPUT "       Documentation: $Gdoc{$group}\n";
  print OUTPUT "       Validation script: $Gvalidate{$group}\n";

  print OUTPUT "Archive\n\n";
  print OUTPUT "       Tarfile\n";
  print OUTPUT "       Location: $Path/TarZ\n";
  print OUTPUT "       Name: $Gtarz{$group}\n";
  print OUTPUT "       Date: $Date[0] $Date[1] $Date[2]:$Date[3]\n";
  print OUTPUT "       Size: $Size\n";
  print OUTPUT "       Checksum: $csum\n";

  print OUTPUT "       Diskettes\n       Number:\n       Date:\n\n\n";

  my @Ports = split( ' ', $portGroups{$group} );
  print "[ ";
  while(@Ports) {
    my $curPort = $Ports[0];
    print "$curPort ";
    print OUTPUT "Targets\n\n";
    print OUTPUT "       Portname: $Pportdir{$curPort}\n";
    print OUTPUT "       Built on: $Gbuilton{$curPort}\n";
    print OUTPUT "       Built with: $Gbuiltwith{$curPort}\n";
    print OUTPUT "       Hardware: $Ghardware{$group}\n";
    print OUTPUT "       OS Name: $Gosname{$group}\n";
    print OUTPUT "       OS Version: $Posversion{$curPort}\n";
    print OUTPUT "       C++ Compiler Name: $Gcompiler{$group}\n";
    print OUTPUT "       C++ Compiler Version(s): $Pcompiler{$curPort}\n";
    print OUTPUT "       Window System:\n";
    print OUTPUT "       DBMS:\n";
    print OUTPUT "       Transport:\n";
    print OUTPUT "       Need C compiler:\n";
    print OUTPUT "       Comment: $Pdcomments{$curPort}\n";

    print OUTPUT "\n\n";
    shift @Ports;
  }
  print "]";
  close OUTPUT;

  print "\n";
}

################################################
########### generation of TarZ files ###########
################################################

sub remove_marks_one_file
{
  # The shell code was the following.
  #  sed 	-e '/\/\/\$doc/D' \
  #		-e '/\/\/end/D' \
  #		-e '/\/\/\\begin/D' \
  #		-e 's/\/\/\$rem//g' \
  #		-e 's/\/\/\$REM//g' \
  #		-e 's/\$rem//g' \
  #		-e 's/\$REM//g' \
  #		$OLDFILE > $FILE
  # As only doc-end pairs are currently used, I simplify things
  my $File = $_[0];
  open (INPUT, "$File") || die ("cannot open $File");
  open (OUTPUT, ">$File.nomarks") || die ("cannot open $File.nomarks");
  my $line;
  while ($line = <INPUT>) {
    if ($line !~ /^\s*\/\/end:\s*(\w+)\s*$/ &&
        $line !~ /^\s*\/\/\$doc:\s*(\w+)\s*$/) {
      print OUTPUT $line;
    }
  }
  close INPUT;
  close OUTPUT;
  unlink "$File";
  rename ("$File.nomarks", "$File");
}

sub remove_marks
{
  my $location = $_[0];
  opendir (DIR, $location) || die ("Cannot opendir $location\n");
  while ($file = readdir(DIR)) {
    if ($file =~ /(.+).cpp$/) {
      if ($verbose>1) { print " - $file\n"; }
      remove_marks_one_file("$location/$file");
    }
  }
  close DIR;
}

sub check_distrib_file
{
  my $workDir = $_[0];
  my $file = "$workDir/$tarzDir/$_[1]";
  print "[$_[1]]:\n";
  system( "grep -i \"bug\" $file | grep -v \"debugInfo\" | grep -v -i \"debug mode\" ");
  system( "grep -i \"not yet implemented\" $file" );
  system( "grep -i \"nyi\" $file" );
  system( "grep -i \"todo\" $file" );
  system( "grep -i \"fixme\" $file" );
  system( "grep -i \"phl\" $file" );
  system( "grep \"PM\" $file" );
  system( "grep \"XNO\" $file" );
  system( "grep \"CB\" $file | grep -v \"ILCB\" " );
  system( "grep \"WN\" $file" );
  system( "grep -i \"jerome\" $file" );
  system( "grep -i \"rogerie\" $file" );
  system( "grep -i \"laborie\" $file" );
  system( "grep -i \"wait\" $file" );
  system( "grep -i \"not available\" $file" );
  system( "grep -i \"delayed\" $file" );
  system( "grep -i \"obsolete\" $file" );
  system( "grep -i \"warning\" $file | grep -v \"warning\\(\\)\"");
  system( "grep -i \"question\" $file" );
  system( "grep -i \"dummy\" $file" );

  if ($file =~ /(.+).cpp$/) {
    system( "grep \\\$doc $file" );
    system( "grep \"\\/\\/end\" $file" );
  }
}

sub verify_tarz_files {
  my $workDir = $_[0];
  print "Checking distributed files in $workDir/$tarzDir.\n";
  print "If the following 'grep' is OK, use -noverify option...\n";
  my $i;
  for( $i=0; $i<@distribFiles; $i++ ) {
    check_distrib_file( $workDir, $distribFiles[$i] );
  }
  print "ok.\n";
}

sub make_tarz_name {
  my $group = $_[0];
  my $tarz;

  if ($Gtarz{$group} =~ /(.+)\.gz/) { $tarz = "$Path/TarZ/$1"; }
  else { $tarz = "$Path/TarZ/$Gtarz{$group}"; }
  if ($tarz =~ /(.+)\.exe/) { return ""; }   # it's  XXXXwintel.exe file... Skip it.
  return $tarz;
}

sub copy_tarz_files {
  my $tmpTar = "xYuiDSs.tar";  
  my $workDir = $_[0];
  if ($verbose>0) { print "Moving original files\nFrom: $Path\nTo:   $workDir/$tarzDir\n"; }
  mysystem "cd $Path && tar cf $workDir/$tarzDir/$tmpTar $distribFiles[0]";
  if ($verbose>1) { print " - $distribFiles[0]\n"; }
  my $i;
  for($i=1; $i<@distribFiles; $i++) {
    mysystem "cd $Path && tar rf $workDir/$tarzDir/$tmpTar $distribFiles[$i]";
    if ($verbose>1) { print " - $distribFiles[$i]\n"; }
  }
  mysystem "cd $workDir/$tarzDir && tar xf $tmpTar && rm -f $tmpTar;";
  if ($verbose>0) { print "ok.\n"; }
}

sub generate_tarz
{
  my $group = $_[0];
  my $tarz = make_tarz_name($group);
  my $i;
  if ($tarz eq "") { 
    print "TarZ name not specified for $group !\n";
    return; 
  }

  print "Generating $tarz.gz for $group\n";

  my @Ports = split( ' ', $portGroups{$group} );
  if (@Ports == 0) {
    die ("No ports for the group $group, or maybe you specified a port instead of a group\n");
  }

  for ($i = 0 ; $i < @Ports ; $i++)
  {
    my $curPort = $Ports[$i];

    check_dir("$workDir/$tarzDir/lib");
    check_dir("$workDir/$tarzDir/lib/$Pportdir{$curPort}");
    check_dir("$workDir/$tarzDir/lib/$Pportdir{$curPort}/$Pformat{$curPort}");
    my $dstDir = "$workDir/$tarzDir/lib/$Pportdir{$curPort}/$Pformat{$curPort}";
    my $srcDir = "$Path/lib/$Pportdir{$curPort}/$Pformat{$curPort}";
    if (not -r "$srcDir/$Plibfilename{$curPort}") { 

      die( "Library for $curPort hasn't been built yet. Should be: $srcDir/$Plibfilename{$curPort}\n" ); 

    }
    mysystem "cp -f $srcDir/$Plibfilename{$curPort} $dstDir;";

    check_dir("$workDir/$tarzDir/examples");
    check_dir("$workDir/$tarzDir/examples/$Pportdir{$curPort}");
    check_dir("$workDir/$tarzDir/examples/$Pportdir{$curPort}/$Pformat{$curPort}");
    $dstDir = "$workDir/$tarzDir/examples/$Pportdir{$curPort}/$Pformat{$curPort}";
    $srcDir = "$Path/examples/$Pportdir{$curPort}/$Pformat{$curPort}";

    my $makefile = "makefile";
    if ($Ptype{$curPort} == 1) {
      $makefile = "examples.dsw";
    }
    if (not -r "$srcDir/$makefile") { 
      die( "User makefile for $curPort hasn't been built yet. Should be: $srcDir/$makefile\n" ); 
    }
    mysystem "cp -f $srcDir/$makefile $dstDir;";
    if ($Ptype{$curPort} == 1) {
      mysystem "cp -f $srcDir/*.dsp $dstDir;";;
    }
  }

  if (($Gtype{$group} == 1) && !("$^O" eq "MSWin32")) {
    print "Converting text files to DOS line-ending...\n";
    mysystem "cd $tarzDir && for file in *.txt; do perl $workDir/u2d.pl < \$file > \$file.dos; rm \$file; mv \$file.dos \$file; done;";
  }

  print "Building tar file...\n";
  mysystem "cd $workDir && tar cf $tarz $tarzDir;";
  print "Compressing tar file...\n";
  mysystem "gzip $tarz";

  for ($i = 0 ; $i < @Ports ; $i++)
  {
    my $curPort = $Ports[$i];

    mysystem( "\\rm -rf $workDir/$tarzDir/lib" );
    mysystem( "\\rm -rf $workDir/$tarzDir/examples/$Pportdir{$curPort}" );
  }

  my @St = stat "$tarz.gz";
  my $Size = $St[7];
  if ($Size) { print " Ok. TarZ size: $Size bytes.\n"; }
}


sub create_multiple_tarz {
  if ($tarzDir eq "") {
    die( "\nNo 'TarzTopDir' name specified in product file $productFile.\nThis is mandatory (e.g.: TarzTopDir = concert11)\n" );
  }
  my $i;
  my $lastType = $Gtype{$classesToDo[0]};
  for ($i = 0 ; $i < @classesToDo ; $i++) {
    if ($lastType != $Gtype{$classesToDo[$i]}) {
      die( "Do not mix different classes of ports in the same command (e.g. MSVC vs Unix) !" );
    }
  }
  
      # check if it's worth.

  for ( $i = 0 ; $i < @portsToDo ; $i++)
  {
    my $curPort = $portsToDo[$i];
    my $lib = "$Path/lib/$Pportdir{$curPort}/$Pformat{$curPort}/$Plibfilename{$curPort}";
    if (not -r $lib) { die( "\nLibrary for $curPort hasn't been built yet.\nShould be: $lib\n" ); }
    my $makefile = "$Path/examples/$Pportdir{$curPort}/$Pformat{$curPort}/makefile";
    if ($Ptype{$curPort} == 1) {
      $makefile = "$Path/examples/$Pportdir{$curPort}/$Pformat{$curPort}/examples.dsw";
    }
    if (not -r $makefile) { die( "\nUser makefile for $curPort hasn't been built yet.\nShould be: $makefile\n" ); }
  }
  for ($i = 0 ; $i < @classesToDo ; $i++) {
    my $tarz = make_tarz_name($classesToDo[$i]);

    if ($tarz eq "") { next; }
    if (-r "$tarz.gz") {
      die( "TarZ file [$tarz.gz] already exists.\nMove it first.\n" );
    }
  }
  if (@distribFiles eq 0) { 
    die( "no distribution files specified. Update product file ($productFile).\n" );
  }


      # ok. go for it.


  print "\n*********************************\n";
  print   "**   Generating tarz file(s).  **\n";
  print   "*********************************\n\n";
  print "(Paths will start with [$tarzDir] in archive)\n\n";

  check_dir( "$Path/TarZ" );
  check_dir_and_lock( "$workDir/$tarzDir" );

  print "Copying files...\n";
  copy_tarz_files( $workDir );
  print "Removing marks in distributed examples in $workDir/$tarzDir/examples/src...\n";
  remove_marks("$workDir/$tarzDir/examples/src");
  if ($verifyFiles) { verify_tarz_files($workDir); }

  for ($i = 0 ; $i < @classesToDo ; $i++) {
    generate_tarz($classesToDo[$i]);
  }

  print "Cleaning up $workDir/$tarzDir...\n";
  system( "\\rm -rf $workDir/$tarzDir" );
  print "Done.\n";

  unlock_dir( "$workDir/$tarzDir" );
}


################################################
########### Create an install-shield ###########
################################################

sub make_setup_file {
  my $metaFileName = shift;
  my $setupFileName = shift;

  my $currentDate =`date +%Y-%d-%m`;
  chop $currentDate;
  my $VersionShort = $Version;
  $VersionShort =~ s/\.//g;

  print "Creating setup file...\n";
  if (-e "$setupFileName") {
    die "$setupFileName already exists !";
  }
  open (INPUT, "$metaFileName") || die ("cannot open $metaFileName");
  open (OUTPUT, ">$setupFileName") || die ("cannot open $setupFileName");
  my $line;
  my $lineNumber = 0;
  my $inMetaPair = 0;
  my $metaInclude = 0;
  while ( $line = <INPUT> ) {
    $lineNumber++;
    if ( $line =~ /^\s*#\s*metaBegin\s*:\s*(.*)\s*$/ ) {
      if ( $inMetaPair ) {
	die "$metaFileName($lineNumber): metaBegin-metaEnd pairs can not be nested\n";
      }
      $inMetaPair = 1;
      my $metaPortNames = $1;
      my $portToDo;
      foreach $portToDo (@portsToDo) {
	if ( $metaPortNames =~ $portToDo ) {
	  if ($verbose > 2 && !$metaInclude) {
	    print "$metaFileName($lineNumber): metaBegin ";
	  }
	  $metaInclude = 1;
	  if ($verbose > 2) { print "$portToDo "; }
	}
      }
      if ($verbose > 2 && $metaInclude) { print "\n"; }
    } elsif ( $line =~ /^\s*#\s*metaEnd\s*$/ ) {
      if (! $inMetaPair) {
        die "$metaFileName($lineNumber): metaEnd without a matching metaBegin\n";
      }
      if ($verbose > 2 && $metaInclude) { print "$metaFileName($lineNumber): metaEnd\n"; }
      $inMetaPair = 0;
      $metaInclude = 0;
    } elsif ( (! $inMetaPair) | $metaInclude ) {
      # Line could be output, but may need further processing
      $line =~ s/\$\{PRODUCT_NAME\}/$Product/g;
      $line =~ s/\$\{PRODUCT_NAME_SHORT\}/$ProductShort/g;
      $line =~ s/\$\{PRODUCT_VERSION\}/$Version/g;
      $line =~ s/\$\{PRODUCT_VERSION_SHORT\}/$VersionShort/g;
      $line =~ s/\$\{DATE\}/$currentDate/g;
      print OUTPUT $line;
    }
  }
  if ( $inMetaPair ) {
    die "$metaFileName(EOF): Unmatched metaBegin !\n";
  }
  close OUTPUT;
  close INPUT;
}


sub find_exe_name {
  my $setupFileName = shift;
  my $exeName = `grep finalExeName $setupFileName`;
  chop $exeName;
  # Remove everything up to (and including) "
  $exeName =~ s/[^\"]*\"//;
  # Remove everything from (and including) "
  $exeName =~ s/\".*//;
  return $exeName;
}
  


sub create_install {
  if ($tarzDir eq "") {
    die( "\nNo 'TarzTopDir' name specified in product file $productFile.\nThis is mandatory (e.g.: TarzTopDir = concert11)\n" );
  }
  for ($i = 0 ; $i < @classesToDo ; $i++) {
    if (1 != $Gtype{$classesToDo[$i]}) {
      die( "InstallShield only works for MSVC ports" );
    }
  }
  
      # check if it's worth.

  for ($i = 0 ; $i < @classesToDo ; $i++) {
    my $tarz = make_tarz_name($classesToDo[$i]);

    #if ($tarz eq "") { next; }
    if (! -e "$tarz.gz") {
      die( "TarZ file does not exists.Should be: $tarz.gz\n" );
    }
  }

      # ok. go for it.


  print "\n**************************************\n";
  print   "**   Generating Install Shield(s).  **\n";
  print   "**************************************\n\n";

  for ($i = 0 ; $i < @classesToDo ; $i++) {
    my $tarz = make_tarz_name($classesToDo[$i]);
    make_setup_file($metaFileName, $setupFileName);
    my $exeName = find_exe_name($setupFileName);
    print "Will generate $Path/TarZ/$exeName\n";
    
    print "Launching IBOIS...\n";
    mysystem "perl /nfs/ishield/1.0/ibois_build.pl \$PWD/$setupFileName $tarz.gz";
    
    print "Removing $setupFileName\n";
    unlink "$setupFileName";
  }
}


################################################
########### Launching a make command ###########
################################################

sub launch_make {
  my $curPort = shift;
  my $makeTarget = shift;

  my $buildingDir = "$localDir/lib/$Pportdir{$curPort}/$Pformat{$curPort}";
  my $make = "make";
  if ($Ptype{$curPort} == 1) {
    $make = "nmake /nologo";
    $buildingDir = "$localDir\\lib\\$Pportdir{$curPort}\\$Pformat{$curPort}";
  }
  if (not -r "$buildingDir/makefile") { 
    die( "makefile for $curPort hasn't been built yet. Should be: $buildingDir/makefile\n" ); 
  }

  # WinNT's command processor does not handle commands separated with ;
  $command = "cd $buildingDir && $make $makeTarget";
  mysystem $command;
  return $buildingDir;
}

############################################
########### Building of the libs ###########
############################################

sub build_libs
{
  while(@_) {
    my $curPort = $_[0];
    print "Building $Plibfilename{$curPort} for $curPort...\n";
    my $buildingDir = launch_make($curPort, "$Plibfilename{$curPort} > BUILD.LOG 2>&1");

    my @St = stat "$buildingDir/$Plibfilename{$curPort}";
    my $Size = $St[7];
    print " Ok. Lib size: $Size bytes.\n";

    shift @_;
  }
}

############################################
########### Cleaning of the libs ###########
############################################

sub clean_libs
{
  while(@_) {
    my $curPort = $_[0];
    print "Cleaning build directory for $curPort...\n";
    launch_make($curPort, "realclean");
    shift @_;
  }
}

#############################################
########### Launching the benches ###########
#############################################

sub make_benches
{
  while(@_) {
    my $curPort = $_[0];
    print "Launching benches for $curPort...\n";
    my $buildingDir = launch_make($curPort, "bench > BENCH.LOG 2>&1");
    launch_make($curPort, "diff > DIFF.LOG 2>&1");

    my @St = stat "$buildingDir/DIFF.LOG";
    my $Size = $St[7];
    if ($Size) { print " Ok. Log size: $Size bytes.\n"; }

    shift @_;
  }
}

################################################
######### generation of Escrow files ###########
################################################
    
sub copy_escrow_files {

  # note: list of files to include in the escrow archive can be huge,
  # and exceed the command line's maximum length... That's why we're building
  # the .tar one file after the other with a 'tar rf' (this can be long).

  my $tmpTar = "xYuiDSs.tar";  
  my $workDir = $_[0];
  if ($verbose>0) { print "Moving original files\nFrom: $Path\nTo:   $workDir/$tarzDir\n"; }
  system "cd $Path; tar cf $workDir/$tarzDir/$tmpTar $distribFiles[0]";
  my $i;
  for($i=1; $i<@distribFiles; $i++) {
    system "cd $Path; tar rf $workDir/$tarzDir/$tmpTar $distribFiles[$i]";
    if ($verbose>1) { print " - $distribFiles[$i]\n"; }
  }
  system "cd $workDir/$tarzDir; tar xf $tmpTar; rm -f $tmpTar;";
  print "   ____ _______   \n  /    \\\\  / _/   \n /   / //   /__ _ \n \\____//__/\\__/(_)\n";
}

sub generate_escrow
{
  my $file = $_[0];
  copy_escrow_files( $workDir );
  print "Generating $file.Z\n";

  system "cd $workDir; tar cf $file $tarzDir;";
  system "compress $file;";

  my @St = stat "$file.Z";
  my $Size = $St[7];
  if ($Size) { print " Ok. Tar.Z size: $Size bytes.\n"; }
}

################################################
###############     help     ###################
################################################

sub print_help {
  if ($verbose>1) { print " ____     ____  ____ ______.__   ____  _  __  ____      ____\n/___/    /    \\/  _ \\__  __/__) /    \\/ \\/  \\/  __\\    /___/\n/___/   /   / /   __/ / (  /  (/   / /   \\  /\\_   \\   /___/ \n        \\____/\\__/   (__/  \\___/____/\\__\\__/\\_____/\n\n"; }
  else { print "\n                  -= Options =-\n\n"; }
  print "  -m .................. create makefiles\n";
  print "  -c .................. check makefiles (nothing generated)\n";
  print "  -user ............... apply -m/-c commands to user-examples\n";
  print "  -localdir <dir> ..... process makefiles under local directory <dir>\n";
  print "\n";
  print "  -clean .............. empty the build directory\n";
  print "  -build .............. create the library\n";
  print "  -bench .............. run all the benches\n";
  print "  -tarz ............... create tarz\n";
  print "  -install ............ create an InstallShield\n";
  print "  -delivery ........... create delivery texts\n";
  print "  -escrow ............. create escrow file\n";
  print "  -tmpdir <dir> ....... process tarz under directory <dir> (e.g.:/tmp)\n";
  print "\n";

  print "  -product <name> ..... product filename ('$configFile')\n";
  print "  -config <name> ...... ports config filename ('$productFile)\n";
  print "  -escrowconf <name> .. config filename for escrow ('$escrowConfigFile')\n";
  print "  -specs .............. print port specs (nothing generated)\n";
  print "  -list ............... print list of known ports (nothing generated)\n";
  print "\n";

  print "  -noverify ........... don't verify files distributed in tarz\n";
  print "  -v .................. verbose mode\n";
  print "  -vv ................. even more verbose\n";
  print "  -vvv ................ are you sure you really need this?\n";
  print "  -h .................. this help\n";
  print "\n";

  print "- Generation of makefiles, libs or benches is *local*.\n";
  print "  Current local directory is: '$localDir'.\n";
  print "  To change it, use the -localdir option.\n\n";
  print "- Generation of delivery files, tarz, escrow is *global*, and uses the\n";
  print "  'Path' or 'PathPrefix' strings defined in the productfile '$productFile'.\n";
  print "  Current value for 'Path' is: '$Path'.\n";
  print "  This is to ensure that what's delivered originates from exactly from\n";
  print "  the same source dir.\n";
  print "\n";
  exit;
}

################################################
################ main part #####################
################################################

while(@ARGV) {
  my $command = $ARGV[0];
  if ($command =~ /-h/) { $doHelp=$1; }
  elsif ($command eq "-product") { 
    shift @ARGV;
    if (not $ARGV[0]) { die( "missing filename after $command \n" ); }
    $productFile = $ARGV[0];
  }
  elsif ($command eq "-config") { 
    shift @ARGV;
    if (not $ARGV[0]) { die( "missing filename after $command\n" ); }
    $configFile = $ARGV[0];
  }
  elsif ($command eq "-escrowconf") { 
    shift @ARGV;
    if (not $ARGV[0]) { die( "missing filename after $command\n" ); }
    $escrowConfigFile = $ARGV[0];
  }
  elsif ($command eq "-c") { $checkMakefiles=1; }
  elsif ($command eq "-localdir") { shift @ARGV; $localDir=$ARGV[0]; }

  elsif ($command eq "-locally") { $buildLocally=1; }

  elsif ($command eq "-tmpdir") { shift @ARGV; $workDir=$ARGV[0]; }
  elsif ($command eq "-user") { $userFiles=1; }
  elsif ($command eq "-m") { $createMakefiles=1; }
  elsif ($command eq "-delivery") { $createDeliveries=1; }
  elsif ($command eq "-noverify") { $verifyFiles=0; }
  elsif ($command eq "-tarz") {
    die( "-tarz option is incompatible with -escrow\n" ) if ($buildingEscrow==1);
    $createTarZ=1;
  }
  elsif ($command eq "-clean") {
    die( "-clean option is incompatible with -escrow\n" ) if ($buildingEscrow==1);
    $cleanLibs=1;
  }
  elsif ($command eq "-build") {
    die( "-build option is incompatible with -escrow\n" ) if ($buildingEscrow==1);
    $buildLibs=1;
  }
  elsif ($command eq "-bench") {
    die( "-bench option is incompatible with -escrow\n" ) if ($buildingEscrow==1);
    $makeBenches=1;
  }
  elsif ($command eq "-install") { $createInstall=1; }
  elsif ($command eq "-specs") { $printSpecs=1; }
  elsif ($command eq "-list") { $printSpecs=2; }
  elsif ($command eq "-v") { $verbose=1; }
  elsif ($command eq "-vv") { $verbose=2; }
  elsif ($command eq "-vvv") { $verbose=3; }
  elsif ($command eq "-escrow") {
    die( "-escrow option is incompatible with -tarz\n" ) if ($createTarZ==1);
    die( "-escrow option is incompatible with -bench\n" ) if ($makeBenches==1);
    die( "-escrow option is incompatible with -build\n" ) if ($buildLibs==1);
    $buildingEscrow=1;
    $productFile = $escrowConfigFile;
    $createTarZ=1;
    $verifyFiles=0;
  }
  else { push( @notExpanded, $command); }
  shift @ARGV;
}



parse_config_file;  # read this file first.

## if no specif port/group required, add all known Groups.
## We must only expand all declared Groups, not all Ports
## because there might be some depracted old Port
## hanging around the config file...

while(@notExpanded) {
  expand($notExpanded[0]);
  shift @notExpanded;
}

if (not $portsCount) { 
  my @Classes = keys %portGroups;
  while(@Classes) { 
    expand($Classes[0]);
    shift @Classes; 
  }
}

#
# We must have expanded the parameters to the script because we need to
# know the classes to do when parsing the product file
#
parse_product_file;

#
# Check that there is something to be done
#
if (!($checkMakefiles | $createMakefiles | $createDeliveries | $printSpecs | $createTarZ | $cleanLibs | $buildLibs | $makeBenches | $createInstall) ) { 
  print "No command specified\n";
  print_help;
}

if ($doHelp) {
  print_help;
}

if ($createMakefiles && $checkMakefiles) {
  die( "incompatible '-c' and '-m/-mex' option used.\n" ) 
}

if ($printSpecs==1) {
  print_product_specs;
  print_specs(@portsToDo);
  exit;
}
if ($printSpecs==2) {
  print_port_list;
  exit;
}

if ($createMakefiles) ## check presence of /lib & /examples dir
{
  if ($userFiles) { check_dir( "$localDir/examples" ); }
  else { check_dir( "$localDir/lib" ); }
}

if ($checkMakefiles) {
  if ($userFiles) {
    if (not -r "$localDir/examples") {
      die( "directory $localDir/examples not found for checking user makefiles.\n" )
    }
  }
  else {
    if (not -r "$localDir/lib") {
      die( "directory $localDir/lib not found for checking makefiles.\n" );
    }
  }
}

if ($createMakefiles || $checkMakefiles) {
  deal_with_makefiles(@portsToDo);
}

if ($createMakefiles) {
  makeMakefileSite;
  makeMakefileShr;
}

if ($buildingEscrow) 
{
  if ($tarzDir eq "") {
    die( "\nNo 'TarzTopDir' name specified in product file $productFile.\nThis is mandatory (e.g.: TarzTopDir = concert11)\n" );
  }

  if (-r "$Path/TarZ/$escrowFile.Z") {
    die( "Escrow Tar.Z file [$Path/TarZ/$escrowFile.Z] already exists.\nMove it first.\n" );
  }
  if (@distribFiles eq 0) { 
    die( "no distribution files specified. Update file $productFile.\n" );
  }

  print "\n*********************************\n";
  print   "**     Building Escrow file.   **\n";
  print   "*********************************\n\n";
  print "(Paths will start with [$tarzDir] in Tar.Z)\n\n";

  check_dir("$Path/TarZ");
  check_dir_and_lock("$workDir/$tarzDir");

  generate_escrow("$Path/TarZ/$escrowFile");

  print "Cleaning up $workDir/$tarzDir...\n";
  system( "\\rm -rf $workDir/$tarzDir" );
  print "Done.\n";

  unlock_dir("$workDir/$tarzDir");
  exit; ## $productFile is trashed...
}
else {
  if ($cleanLibs) {
    clean_libs(@portsToDo);
  }
  if ($buildLibs) {
    build_libs(@portsToDo);
  }
  if ($makeBenches) {
    make_benches(@portsToDo);
  }
  if ($createTarZ) {
    create_multiple_tarz();
  }
  if ($createInstall) {
    create_install();
  }
}

if ($createDeliveries) { 
  my $i;
  print "Generating delivery file(s)...\n";
  for ($i = 0 ; $i < @classesToDo ; $i++) {
    generate_delivery($classesToDo[$i]);
  }
}
##### end #####
