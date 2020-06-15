#!/usr/bin/perl -w

use strict;
use Getopt::Long;
use File::Basename;

################################################################################

my $THIS=$0; $THIS=~s%.*/%%;

my $USAGE = <<END_OF_TEXT;

  $THIS [ options ]

  This script parses all plugin properties files and creates a CSV or Wiki file
  with information about each plugin.

  -o, --output-file <file>
     The location of the output file.

  -v, --verbose
    Be more verbose about what is going on.

  -w, --wiki
    Output data as a Wiki table with '|' delimiters.  Without this option,
    the default is CSV.

END_OF_TEXT

my @PROPERTIES = ( "Name", "QualityCategory", "QualityString", "Description" );
my $VERBOSE = 0;
my $WIKI = 0;

################################################################################

exit Main();

sub VerbosePrint
{
  if ($VERBOSE) {
    print @_;
  }
}

sub Main
{
  my $OutputFile;

  Getopt::Long::config("bundling");
  GetOptions
  (
   "o|output-file=s" => \$OutputFile,
   "v|verbose" => \$VERBOSE,
   "w|wiki" => \$WIKI,
  ) or die $USAGE;

  die "Error: Must specify output file with '-o'.\n" unless $OutputFile;

  my $ExecutableDir = dirname($0);

  my $Command = "find $ExecutableDir -iname 'plugin.properties'";
  VerbosePrint("Executing: $Command\n");
  my @Files = `$Command`;
  my $Status = $?;
  if ($Status) {
    die "Error: Failed to get list of plugins.\n";
  }
  VerbosePrint("Found " . scalar(@Files) . "files\n");

  VerbosePrint("Opening output file '$OutputFile'\n");

  $Status = open(OUTPUT, ">$OutputFile");
  unless ($Status) {
    die "Error: Failed to open output file '$OutputFile': $!\n";
  }
  if ($WIKI) {
    print OUTPUT "|*" . join("*|*", @PROPERTIES) . "*|\n";
  } else {
    print OUTPUT join(",", @PROPERTIES) . "\n";
  }

  my $Errors = 0;
  my $FilesParsed = 0;
  foreach my $File ( sort @Files ) {
    if (ParsePluginProperties(\*OUTPUT, $File)) {
      $Errors++;
    }
    $FilesParsed++;
  }

  close OUTPUT;
  print "$FilesParsed files parsed with $Errors errors.\n";
  return $Errors;
}

sub ParsePluginProperties
{
  my ($Output, $File) = @_;

  my $Delimiter = ( $WIKI ? "|" : "," );

  VerbosePrint("Opening '$File'\n");
  my $Status = open(IN, $File);
  unless ($Status) {
    warn "Error: Could not open file '$File': $!\n";
    return 1;
  }

  my $LineNumber = 0;

  my %Hash;

  while (my $Line = <IN> ) {
    chomp $Line;
    $LineNumber++;
    next unless $Line =~ /\s*\w+\s*=/;
    my ($Param, $Value) = split/=/, $Line, 2;
    if (grep /^$Param$/i, @PROPERTIES) {
      VerbosePrint("Found property '$Param' on line $LineNumber\n");
      $Hash{lc($Param)} = $Value;
    }
  }
  close IN;
  VerbosePrint("Closed file\n");
  print $Output $Delimiter if $WIKI;
  foreach my $Id ( @PROPERTIES ) {
    if (exists($Hash{lc($Id)})) {
      print $Output "\"" unless $WIKI;
      print $Output $Hash{lc($Id)};
      print $Output "\"" unless $WIKI;
    } elsif ($WIKI) {
      print $Output " ";
    }
    print $Output $Delimiter;
  }
  print $Output "\n";
  return 0;
}
