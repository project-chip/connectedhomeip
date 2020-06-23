#!/usr/bin/perl

# Regenerate the AF documentation included in generated files.

use Cwd 'chdir';

my $USAGE = "usage: regenerate-documentation.pl variant\n";

my $DEBUG = 0;
sub DebugPrintln { print "@_\n" if $DEBUG; }

my $DocumentationTypePlaceholder = "__DocType__";
my $DocumentationFilePlaceholder = "__DocFile__";
my $DocumentationFrameworkPlaceholder = "__DocFramework__";

my $ZNET_LOCATION = Cwd::realpath(Cwd::getcwd . "/../../../../");
if (exists($ENV{ZNET_LOCATION_DOC})) {
  $ZNET_LOCATION = $ENV{ZNET_LOCATION_DOC};
  print "Using ZNET_LOCATION from environment variable ZNET_LOCATION_DOC.\n";
} else {
  print "No ZNET_LOCATION_DOC environment variable.\n";
}
print "ZNET_LOCATION=$ZNET_LOCATION\n";

my $ISD_EXECUTABLE_WINDOWS="InSightDesktop.exe";
my $ISD_EXECUTABLE_LINUX="InSightDesktop";
my $ISD_EXECUTABLE_MACOSX="Eclipse.app/Contents/MacOS/eclipse";
my $STUDIO_EXECUTABLE_WINDOWS="studio.exe";
my $STUDIO_EXECUTABLE_LINUX="studio";
my $STUDIO_EXECUTABLE_MACOSX="Simplicity Studio.app/Contents/MacOS/studio";
my $APPBUILDER_OPTIONS = "-application com.ember.app_x3.application -nosplash "
                        ."-dumpDocumentation=$DocumentationTypePlaceholder "
                        ."-file=$DocumentationFilePlaceholder "
                        ."-framework=$DocumentationFrameworkPlaceholder "
                        ."-stack=$ZNET_LOCATION "
                        ." -vmargs -Dequinox.scr.waitTimeOnBlock=100";

my $ISD_LOCATION    = "../../../../../../../ember-desktop/build/InSightDesktop";
my $STUDIO_LOCATION = "../../../../../../../studio/build/studio";

my %AF_DOCUMENTATION_HASH = (
  "cli"        => "cli.doc",
  "nonclustercallback"   => "non-cluster-callback.doc",
  "clustercallback"   => "cluster-callback.doc",
  "plugincallback"   => "plugin-callback.doc",
  "halcallback"   => "hal-callback.doc",
  "enum"       => "enums.doc",
  "client_api" => "client-command-macro.doc"
);

################################################################################
# Main
################################################################################

my $Framework;
my $Executable;

# Check the user's input.
$Framework = $ARGV[0] unless !$ARGV[0];
if (!$Framework) {
  print $USAGE;
  exit 1;
}

# Find the location of AppBuilder. Always fun.
if (exists($ENV{ISD_LOCATION})) {
  $ISD_LOCATION = $ENV{ISD_LOCATION};
  DebugPrintln "Using ISD_LOCATION from environment variable.";
} else {
  DebugPrintln "No ISD_LOCATION environment variable.";
}
DebugPrintln "ISD_LOCATION=$ISD_LOCATION";
if (exists($ENV{STUDIO_LOCATION})) {
  $STUDIO_LOCATION = $ENV{STUDIO_LOCATION};
  DebugPrintln "Using STUDIO_LOCATION from environment variable.";
} else {
  DebugPrintln "No STUDIO_LOCATION environment variable.";
}
DebugPrintln "STUDIO_LOCATION=$STUDIO_LOCATION";

# Find the location of the executable. Even more fun.
if ( -x "$STUDIO_LOCATION/$STUDIO_EXECUTABLE_WINDOWS" ) {
  $Executable = "$STUDIO_LOCATION/$STUDIO_EXECUTABLE_WINDOWS";
  $Cygwin = 1;
} elsif ( -x "$STUDIO_LOCATION/$STUDIO_EXECUTABLE_LINUX" ) {
  $Executable = "$STUDIO_LOCATION/$STUDIO_EXECUTABLE_LINUX";
} elsif ( -x "$STUDIO_LOCATION/$STUDIO_EXECUTABLE_MACOSX" ) {
  $Executable = "$STUDIO_LOCATION/$STUDIO_EXECUTABLE_MACOSX";
  $Prefix = `pwd`;
  chomp $Prefix;
  $Prefix .= '/';
} elsif ( -x "$ISD_LOCATION/$ISD_EXECUTABLE_WINDOWS" ) {
  $Executable = "$ISD_LOCATION/$ISD_EXECUTABLE_WINDOWS";
  $Cygwin = 1;
} elsif ( -x "$ISD_LOCATION/$ISD_EXECUTABLE_LINUX" ) {
  $Executable = "$ISD_LOCATION/$ISD_EXECUTABLE_LINUX";
} elsif ( -x "$ISD_LOCATION/$ISD_EXECUTABLE_MACOSX" ) {
  $Executable = "$ISD_LOCATION/$ISD_EXECUTABLE_MACOSX";
  $Prefix = `pwd`;
  chomp $Prefix;
  $Prefix .= '/';
} else {
  die "Error: Cannot determine Studio or ISD executable!\n";
}
DebugPrintln "Executable: $Executable";

# Create the documentation.
my $PWD = `pwd`;
chomp $PWD;
my $DocumentationDir = "$PWD";
$DocumentationDir =~ s/cygdrive\/c\///g;
foreach my $DocumentType (keys %AF_DOCUMENTATION_HASH) {
  my $Command = "'$Executable' $APPBUILDER_OPTIONS";
  my $DocumentationFile = "$DocumentationDir/$AF_DOCUMENTATION_HASH{$DocumentType}";
  $Command =~ s/$DocumentationTypePlaceholder/$DocumentType/;
  $Command =~ s/$DocumentationFilePlaceholder/$DocumentationFile/;
  $Command =~ s/$DocumentationFrameworkPlaceholder/$Framework/;
  DebugPrintln "Command: $Command";
  print `$Command`;
}
