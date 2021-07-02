#!/usr/local/ibin/perl
#
#########################################################################
## dircomp.in.pl - Only to be called by 'dircomp.pl'.
#########################################################################

# lecture des flags et des repertoires selectionnes

$dcdextension=".curr";
$dcddirres=".";
$dcddirref=".";

while ($ARGV[0] =~ /^-/) {
  if ($ARGV[0] =~ /d/) {
    $dcddiff=1;
  };
  if ($ARGV[0] =~ /fm/) {
    $dcdforce_mem=1;
  };
  if ($ARGV[0] =~ /ft/) {
    $dcdforce_time=1;
  };
  if ($ARGV[0] =~ /fa/) {
    $dcdforce_mem=1;
    $dcdforce_time=1;
  };
  if ($ARGV[0] =~ /-m/) {
    $dcdmemmult=2;
  };
  if ($ARGV[0] =~ /-t/) {
    shift @ARGV;
    $dcdtimemult=$ARGV[0];
  };
  shift @ARGV;
};
$dcddirres=$ARGV[0];
$dcddirres =~ s/(.*)\/$/$1/;
$dcddirres .= "/";
$dcddirref=$ARGV[1];
$dcddirref =~ s/(.*)\/$/$1/;
$dcddirref .= "/";

# ouverture d'un fichier de sortie pour le chargement de dcdiff.pl

undef @ARGV;


require "dcdiff.pl";  

# lecture des fichiers contenus dans les repertoires choisis
# et suppression des extensions

opendir DRES, "$dcddirres";
@dcdfichiers_res = grep s/(.*)\.curr$/$1/, readdir DRES;
closedir DRES;
opendir DREF, "$dcddirref";
@dcdfichiers_ref = grep s/(.*)$dcdextension$/$1/, readdir DREF;
closedir DREF;

# intersection des listes de fichiers et listes de fichiers manquants

$dcdi=0;
$dcdj=0;
@dcdfichiers_res= sort @dcdfichiers_res;
@dcdfichiers_ref= sort @dcdfichiers_ref;

while (($dcdi <= scalar @dcdfichiers_res - 1) && 
       ($dcdj <= scalar @dcdfichiers_ref - 1)) {
  if (($dcdfichiers_res[$dcdi] cmp $dcdfichiers_ref[$dcdj])==1) {
    push @dcdmissing_res, $dcdfichiers_ref[$dcdj];
    $dcdj++;
  } 
  elsif (($dcdfichiers_res[$dcdi] cmp $dcdfichiers_ref[$dcdj])==-1) {
    push @dcdmissing_ref , $dcdfichiers_res[$dcdi];
    $dcdi++;
  }
  else {
    push @dcdintersection, $dcdfichiers_res[$dcdi];
    $dcdi++;
    $dcdj++;
  };
};
if ( $dcdi <= scalar @dcdfichiers_res ) {
  foreach $dcdk ($dcdi..scalar @dcdfichiers_res - 1) {
    push @dcdmissing_ref, $dcdfichiers_res[$dcdk];
  };
};
if ( $dcdj <= scalar @dcdfichiers_ref ) {
  foreach $dcdk ($dcdj..scalar @dcdfichiers_ref - 1) {
    push @dcdmissing_res, $dcdfichiers_ref[$dcdk];
  };
};

# comparaison des tests contenus dans l'intersection

foreach $dcdtbc (@dcdintersection) {
  $dcdf_res=$dcddirres.$dcdtbc.".curr";
  $dcdf_ref=$dcddirref.$dcdtbc.$dcdextension;
  diff ("-n", $dcdtbc, $dcdf_res, $dcdf_ref);
};
print "\n";
foreach $dcdtmrf (@dcdmissing_ref) {
  print "$dcdtmrf$dcdextension ";
}; 
if (scalar @dcdmissing_ref) { print "missing in $dcddirref\n"; };
foreach $dcdtmrs (@dcdmissing_res) {
  print "$dcdtmrs.curr ";
}; 
if (scalar @dcdmissing_res) { print "missing in $dcddirres\n"; };
print "\n%%% Bilan pour $dcddirres\n%%% et $dcddirref\n";
print "\necarts moyens: ";
foreach $dcdIBN (0..12) {
  if ($dcdsumref[$dcdIBN]) {
    $dcdecart=100*
      ($dcdmult[$dcdIBN]*$dcdsumres[$dcdIBN]-$dcdsumref[$dcdIBN])
	/$dcdsumref[$dcdIBN];
  };
  if ($dcdsumres[$dcdIBN]==$dcdsumref[$dcdIBN]) {
    $dcdecart=0;
  };
  if (!$dcdsumref[$dcdIBN] && $dcdsumres[$dcdIBN]) {
    print  $dcdchamps_i[$dcdIBN],": NaN, ";
  } else {
    printf "%1s: %1.2f\%, ", $dcdchamps_i[$dcdIBN], $dcdecart;
  };
};
print "\ntests KO: $dcdnbtestsKO/$dcdnbtests";


# effectuer le diff sur les sorties si elles sont differentes

if ($^O =~ /win/i) {
  $dcddiff_command="diff";
} else {
  $dcddiff_command="diff";
};
if ($dcddiff) {
  unlink 'temp_dircomp1', 'temp_dircomp2';
  foreach $dcdtest (sort keys %dcdsortie_ref) {
    print "\n\n============= Diff of $dcdtest: =============\n";
    open ( TMP1, ">temp_dircomp_1" );
    open ( TMP2, ">temp_dircomp_2" );
    print TMP1 $dcdsortie_res{$dcdtest};
    print TMP2 $dcdsortie_ref{$dcdtest};
    close TMP1;
    close TMP2;
    print `$dcddiff_command temp_dircomp_1 temp_dircomp_2`;
    unlink 'temp_dircomp_1', 'temp_dircomp_2';
  };
};
print "\n";

