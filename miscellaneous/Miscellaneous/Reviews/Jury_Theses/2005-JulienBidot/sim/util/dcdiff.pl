#!/usr/bin/perl
#
#########################################################################
## dcdiff.pl - Only to be called by 'dircomp.pl'.
#########################################################################
sub diff {
  %dcdindice_h= (
	      "Number of fa" =>  0,
	      "Number of cho" =>  1,
	      "Number of va" =>  2,
	      "Number of co" =>  3,
	      "Reversible s" =>  4,
	      "Solver heap " =>  5,
	      "Solver globa" =>  6,
	      "And stack (b" =>  7,
	      "Or stack (by" =>  8,
	      "Search Stack" =>  9,
	      "Constraint q" => 10,
	      "Total memory" => 11,
	      "Running time" => 12,
	      "Elapsed time" => 12
	     );
  @dcdchamps_i= (
	      "fails",
	      "choice pts",
	      "vars",
	      "const",
	      "R.stack",
	      "Solv.heap",
	      "Solv.glob.heap",
	      "&stack",
	      "|stack",
	      "S.stack",
	      "Const.q",
	      "Tot.mem",
	      "time"
	     );
  if (!(defined $dcddiff)) {
    $dcddiff= 0;
  };
  if (!(defined $dcdforce_mem)) {
    $dcdforce_mem= 0;
  };
  if (!(defined $dcdforce_time)) {
    $dcdforce_time= 0;
  };
  if (!(defined $dcdmemmult)) {
    $dcdmemmult= 1;
  };
  if (!(defined $dcdtimemult)) {
    $dcdtimemult= 1;
  };
  @dcdforce= ( 0, 0, 0, 0, $dcdforce_mem, $dcdforce_mem, $dcdforce_mem, 
	       $dcdforce_mem, $dcdforce_mem, $dcdforce_mem, $dcdforce_mem, 
	       $dcdforce_mem, $dcdforce_time );
  @dcdmult= ( 1, 1, 1, 1, $dcdmemmult, $dcdmemmult, $dcdmemmult, $dcdmemmult, 
	      $dcdmemmult, $dcdmemmult, $dcdmemmult, $dcdmemmult, 
	      $dcdtimemult);
  if (!(defined $dcdnbtestsKO)) {
    $dcdnbtestsKO= 0;
  };
  if (!(defined $dcdnbtests)) {
    $dcdnbtests= 1;
  } else {
    $dcdnbtests++;
  };
  if (!(defined @dcdsumres)) {
    @dcdsumres= ( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  };
  if (!(defined @dcdsumref)) {
    @dcdsumref= ( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  };
  if (!(defined @dcdnb_ecarts)) {
    @dcdnb_ecarts= ( 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
  };
  if (!(defined @dcdmarge)) {
    @dcdmarge= ( 0, 0, 0, 0, 5, 5, 5, 5, 5, 5, 5, 5, 10 ); 
  };
  @dcdemptytab= ( -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 );
  undef @dcdsortie;
  undef @dcdtableau;
  push @{ $dcdtableau[0][0] }, @dcdemptytab;
  push @{ $dcdtableau[1][0] }, @dcdemptytab;
  $dcdsortieKO=0;
  $dcdKO=0;
  $dcdisthereanyIB=0;
  $dcdprintedname=0;
  $dcdprintname=0;
  $dcdname="";
  
  # lecture des flags et du test selectionne
  
  if ( $_[0] =~ /^-N/) { 
    $dcdname=$_[1];
    shift @_;
    shift @_;
  };
  if ( $_[0] =~ /^-n/) { 
    $dcdname=$_[1];
    $dcdprintname=1;
    shift @_;
    shift @_;
  };
  if ( $_[0] =~ /^-d/) {
    $dcddiff=1;
    shift @_;
  };
  $dcdbenchres=$_[0];
  $dcdbenchref=$_[1];
  
  # TESTS DEBUG
  
#  print STDERR "\nTT: DIFF: $diff\n"; #T
#  print STDERR "TT: PRINTNAME: $printname\n"; #T
#  print STDERR "TT: NAME: $name\n"; #T
  



  # ouverture des fichiers a comparer
  
  open (SER,"$dcdbenchres");
  open (FER,"$dcdbenchref");
  
  ######### lecture des donnes du resultat puis de la reference
  foreach $dcdentree (0, 1) {
    $dcdIlisteIB=0;
    if ($dcdentree) { $dcdfichier= \*SER } else { $dcdfichier= \*FER };
    while ( $dcdligne=<$dcdfichier> )
      {
	if ( $dcdligne =~ m/^(Number of fa|Number of cho|Number of va|Number of co|Reversible s|Solver heap |Solver globa|And stack \(b|Or stack \(by|Search Stack|Constraint q|Total memory|Running time|Elapsed time).*[^\d\.](\d+\.?\d*|\.\d+)$/ )
	  { 
	    if ($dcdtableau[$dcdentree][$dcdIlisteIB][$dcdindice_h{$1}]!=-1) {
	      # traiter le cas d'IB multiples
	      $dcdIlisteIB++;
	      push @{ $dcdtableau[$dcdentree][$dcdIlisteIB] }, @dcdemptytab;
	    };
	    # enregistrer l'IB
	    $dcdtableau[$dcdentree][$dcdIlisteIB][$dcdindice_h{$1}]=$2;
	    $dcdisthereanyIB=1;
	  }
	elsif ( $dcdligne =~ m/^Elapsed Time [^:]/ )
	  {
	    # eliminer les lignes "Elapsed Time ..." dans certains benchs 
	  }
	elsif ( $dcdligne =~ m/ILOG/ ) {
	}
	else {			
	  # homogeneiser les infinis 
	  $dcdligne =~ s/\+\+/Inf/;
          $dcdligne =~ s/1.\#INF/Inf/;
	  $dcdligne =~ s/\+INF/Inf/;
          $dcdligne =~ s/INF/Inf/;
          $dcdligne =~ s/Infinity/Inf/;
          $dcdligne =~ s/infinity/Inf/;
          $dcdligne =~ s/inf/Inf/;
          $dcdligne =~ s/ *Inf/Inf/;
          $dcdligne =~ s/ *-Inf/-Inf/;
	  # enregistrer la sortie
	  $dcdsortie[$dcdentree].=$dcdligne;
	};
      };
    $dcdNlisteIB[$dcdentree]=$dcdIlisteIB;
  };
    
  ####### comparaison des donnees
  
  # similitude des sorties  
  if ($dcdsortie[0] ne $dcdsortie[1]) {
    #    print STDERR "\nTT: sorties differentes\n"; #T
    if ($dcddiff) {
      $dcdsortie_res{$dcdname}=$dcdsortie[1];
      $dcdsortie_ref{$dcdname}=$dcdsortie[0];
    } 
    else 
      {
      if (!$dcdprintedname && $dcdprintname) {
      print "\n$dcdname : ";
      $dcdprintedname=1;
    };
    print "sortie KO; ";
    };
    $dcdsortieKO=1;
  };
  
  # egalite des IB
  
  if ($dcdisthereanyIB) { 
    # coherence du nombres de listes d'IB
 #   if ($NlisteIB[0]!=$dcdNlisteIB[1]) {
 #     $dcdwarning++;
 #   };
    foreach $dcdlisteIB (0..$dcdNlisteIB[0]) {
      foreach $dcdIBN (0..12) {
	# gestion des IB manquantes
	if ($dcdtableau[0][$dcdlisteIB][$dcdIBN]==-1 and 
	    $dcdtableau[1][$dcdlisteIB][$dcdIBN]==-1) {
	}
	elsif  ($dcdtableau[0][$dcdlisteIB][$dcdIBN]==-1 xor 
		$dcdtableau[1][$dcdlisteIB][$dcdIBN]==-1) {
	    $dcdwherenoIB="Ref";
	    if ($dcdtableau[1][$dcdlisteIB][$dcdIBN]==-1) {
	      $dcdwherenoIB="Res";
	    };
	    # afficher l'IB incoherente "proprement"
	    if (!$dcdprintedname && $dcdprintname) {
	      print "\n$dcdname : ";
	      $dcdprintedname=1;
	    };
	    printf  "%4s%2d: Not in $dcdwherenoIB, ", $dcdchamps_i[$dcdIBN], 
	    $dcdlisteIB + 1;
	    $dcdKO++;
	}
	else {
	  $dcdsumref[$dcdIBN]+=$dcdtableau[0][$dcdlisteIB][$dcdIBN];
	  $dcdsumres[$dcdIBN]+=$dcdtableau[1][$dcdlisteIB][$dcdIBN];
	  $dcdnb_ecarts[$dcdIBN]++;
	  if ($dcdtableau[0][$dcdlisteIB][$dcdIBN]==0 && 
	      $dcdtableau[1][$dcdlisteIB][$dcdIBN]!=0 && 
	      !$dcdforce[$dcdIBN])
	    {
	      # gerer la division par zero
	      if (!$dcdprintedname && $dcdprintname) {
		print "\n$dcdname : ";
		$dcdprintedname=1;
	      };
	      printf  "%4s%2d: NaN, ", $dcdchamps_i[$dcdIBN], $dcdlisteIB + 1;
	      $dcdKO++;
	    }
	  elsif ($dcdtableau[1][$dcdlisteIB][$dcdIBN]!=
		 $dcdmult[$dcdIBN]*$dcdtableau[0][$dcdlisteIB][$dcdIBN] && 
		 !$dcdforce[$dcdIBN])
	    {
	      $dcdecart=100 * 
		($dcdmult[$dcdIBN]*$dcdtableau[1][$dcdlisteIB][$dcdIBN] - 
		 $dcdtableau[0][$dcdlisteIB][$dcdIBN]) 
		  / $dcdtableau[0][$dcdlisteIB][$dcdIBN];
	      # gerer la marge d'erreur
	      if (abs $dcdecart >= $dcdmarge[$dcdIBN]) { 
		if (!$dcdprintedname && $dcdprintname) {
		  print "\n$dcdname : ";
		  $dcdprintedname=1;
		};
		$dcdKO ++;
		printf "%4s%2d: %1.2f\%, ", $dcdchamps_i[$dcdIBN], 
		       $dcdlisteIB + 1, $dcdecart;
	      };
	    };
	};
      };
    };
  };
  # bilan du test
  if ($dcdKO || $dcdsortieKO) {
    $dcdnbtestsKO++;
  };

#  print STDERR "\nTT: sortieKO: $dcdsortieKO\n"; #T
};

sub main {
  if (defined @ARGV) {
    diff (@ARGV);
    print "\n";
  };
  1;
}; 

main ();
