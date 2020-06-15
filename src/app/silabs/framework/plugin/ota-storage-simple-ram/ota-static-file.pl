#!/usr/bin/perl -w

# See $USAGE below

################################################################################
# Modules

use strict;
use Getopt::Long;
use Cwd 'chdir';

################################################################################
# Globals

my $THIS = $0; $THIS =~ s%.*/%%;

my $USAGE = <<END_OF_TEXT;

  $THIS [ parameters ]

  This script takes a standard Zigbee OTA file as input and converts
  it to C code to be included directly into source code.  This is useful
  for testing small files.

  REQUIRED PARAMETERS

  -i, --input <ota-input-file>
    This is the OTA file to use as input to be converted into C code.

  -o, --output <output-file>
    This is the optional location of the output file to write C code to.
    By default it will write to STDOUT.

  OPTIONAL PARAMETERS

  -e, --extra-comments <image-builder-location>
    Include extra comments in the source file that display detailed
    info about the input-file.  This will run 'image-builder' to obtain
    the information.

  -d, --dont-check-magic
    Don't check the file for the OTA magic number.  Normally the input
    is checked to make sure it is an OTA file.

END_OF_TEXT

# This output must be in the hexdump format (first field is offset)
my $MAGIC_NUMBER_BYTES = "00000000  1e f1 ee 0b ";

################################################################################

exit Main();

sub Main
{
  my $InputFile;
  my $OutputFile;
  my $ImageBuilderExe;
  my $DontCheckMagicNumber = 0;

  Getopt::Long::config("bundling");
  GetOptions
  (
   'i|input=s'      => \$InputFile,
   'o|output=s'     => \$OutputFile,
   'e|extra-comments=s' => \$ImageBuilderExe,
   'd|dont-check-magic' => \$DontCheckMagicNumber,
  ) or die $USAGE;

  # Turn on autoflush.
  $| = 1;

  die "Error: Must specify input file with '-i'.\n" unless $InputFile;
  die "Error: Input file does not exist.\n" unless (-e $InputFile);

  die "Error: Image-builder executable does not exist.\n"
    if ($ImageBuilderExe && ! -e $ImageBuilderExe);

  return ProcessInputFile(!$DontCheckMagicNumber,
                          $InputFile,
                          $OutputFile,
                          $ImageBuilderExe);
}

sub ProcessInputFile
{
  my ($CheckMagicNumber, $InFile, $OutFile, $ImageBuilderExe) = @_;

  my $Status;
  my $Out = *STDOUT;
  if ($OutFile) {
    $Status = open(OUTPUT, ">$OutFile");
    unless ($Status) {
      warn "Error: Could not open output file '$OutFile': $!\n";
      return 1;
    }
    $Out = *OUTPUT;
  }

  # '-v' causes Hexdump to display all data and NOT
  # ignore identical sets of output lines that match the preceeding ones.
  $Status = open(INPUT, "hexdump -vC $InFile |");
  unless ($Status) {
    warn "Error: Could not execute hexdump on file '$InFile': $!\n";
    close OUTPUT if $OutFile;
    return 1;
  }

  my $ExtraOutput = "";

  if ($ImageBuilderExe) {
    $ExtraOutput = GetExtraOutput($InFile, $ImageBuilderExe);
    if (!defined($ExtraOutput)) {
      close OUTPUT if $OutFile;
      return 1;
    }
  }

  print $Out <<END_OF_TEXT;
// Auto-generated C header created by $THIS
// Input file: $InFile

$ExtraOutput

#define STATIC_IMAGE_DATA { \\
END_OF_TEXT

  my $Line;
  my $TotalBytes = 0;

  while ($Line = <INPUT>) {
    chomp $Line;
    if ($CheckMagicNumber) {
      unless ($Line =~ /^$MAGIC_NUMBER_BYTES/i) {
        warn "\nError: File does not have the Zigbee OTA magic number.\n";
        close INPUT;
        close OUTPUT if $OutFile;
        return 1;
      }
      $CheckMagicNumber = 0;
    }
    # remove the |.......| that is the ASCII display at the end of the line
    $Line =~ s/\|.*$//;
    my @Fields = split/\s+/, $Line;

    # The first entry of the list is the offset, which we don't care about
    shift @Fields;

    my $Length = scalar(@Fields);
    $TotalBytes += $Length;

    for (my $i = 0; $i < 2; $i++) {

      # Generally we take the fields 0 .. 7 or 8 .. 15
      # but if the array is shorter than that, we take what we have.
      my $Start = $i * 8;
      my $End = ($i == 0
                 ? ($Length < 8
                    ? $Length - 1
                    : 7)
                 : ($Length < 16
                    ? $Length - 1
                    : 15));
      my @ChunkOf8Bytes = @Fields[$Start..$End];
      print $Out "  0x" . join(", 0x", @ChunkOf8Bytes) . ", \\\n"
        if scalar(@ChunkOf8Bytes);

      last if $Length < 8;
    }
  }
  # We add an "L" here to indicate that this constant is LONG (32-bits)
  print $Out "}\n\n#define STATIC_IMAGE_DATA_SIZE $TotalBytes" . "L\n";

  close INPUT;
  close OUTPUT if $OutFile;

  return 0;
}

sub GetExtraOutput
{
  my ($InputFile, $ImageBuilderExe) = @_;

  my $Command = "$ImageBuilderExe -p $InputFile";
  my @ExtraOutput = `$Command`;
  my $Status = $?;
  if ($Status) {
    warn "Error: Failed to execute image-builder:\n";
    warn "  Command: $Command\n";
    warn "  " . join("  ", @ExtraOutput) . "\n";
    return undef;
  }

  my $ReturnOutput = "";

  foreach my $Line (@ExtraOutput) {
    chomp $Line;
    $ReturnOutput .= "//  $Line\n";
  }
  return $ReturnOutput;
}
