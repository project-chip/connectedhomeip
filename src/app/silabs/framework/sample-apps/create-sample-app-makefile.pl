#!/usr/bin/perl -w

use strict;

################################################################################
# See $USAGE below for description.

################################################################################
# Modules

use strict;
use Getopt::Long;
use File::Basename;

################################################################################
# Globals

my $THIS = $0; $THIS =~ s%.*/%%;

my $SCRIPT_DIR = dirname($0);

# No trailing slash
my $TEST_SCRIPT_RELATIVE_DIRECTORY = "../test";

my $RUN_AF_SCRIPT_COMMAND = 'java -cp ${CLASSPATH} bsh.Interpreter app/framework/test/af-test.bsh ';

my $USAGE = <<END_OF_TEXT;

  $THIS [ parameters ]

  This script parses the '$TEST_SCRIPT_RELATIVE_DIRECTORY' and generates a makefile
  based on the simulated binaries required for the test to run.  This makefile can
  be used to run Afv2 tests without rebuilding ALL the applications, but
  only those applications that need to be rebuilt.

  -o, --output-file <makefile>
    The makefile where to output the results.

  -d, --debug
    Print additional debug information.

END_OF_TEXT

my $MAKE_COMMAND = "make -f ";

# Build paths relative to the root of the stack dir so that make can find
# all files accordingly.
my $SAMPLE_APP_BUILD_PATH = "app/framework/sample-apps";
my $SCENARIO_APP_BUILD_PATH = "app/framework/scenarios";

my $BUILD_SCRIPT = "./build.pl";
my $SIMULATION_BUILD_PARAMS = "PLAT=unix MICRO=simulation PHY=simulation";
my $SIMULATION_BUILD_PARAMS_USING_TRANSCEIVER = "PLAT=unix MICRO=simulation PHY=simulation_trx";
my $SIMULATION_GB_BUILD_PARAMS = "PLAT=unix MICRO=simulation PHY=simulation_gb";
my $SIMULATION_DUAL_BUILD_PARAMS = "PLAT=unix MICRO=simulation PHY=simulation_dual";
my $DEBUG = 0;

my $BUILD_DIRECTORY = "build/af-test-apps";

# Which beanshell tests do we want to skip?
my %TESTS_TO_SKIP_HASH = (
  "endPointBroadcast.bsh" => 1,
);

# Which beanshell tests can only run in single-hop mode?
my %TESTS_WITH_ONLY_SINGLE_HOP_HASH = (
"address-table" => 1,
"barrier-control" => 1,
"color-control" => 1,
"color-server" => 1,
"color-server-executeifoff" => 1,
"color-server-couple-level" => 1,
"commshub-dual-phy" => 1,
"contact-sensor" => 1,
"custom-ezsp" => 1,
"device-management" => 1,
"dimmer-switch" => 1,
"door-lock" => 1,
"door-lock2" => 1,
"drlc" => 1,
"endpoint" => 1,
"gateway-reference" => 1,
"gbcs" => 1,
"interpan-cbke-real_ecc" => 1,
"key-establishment-283k1-real_ecc" => 1,
"key-establishment-real_ecc" => 1,
"light-ui" => 1,
"mac-address-filtering-test" => 1,
"mirror-sleepy" => 1,
"mn-esi-ipd" => 1,
"mn-ipd-esi" => 1,
"mn-z3-tc-se-ipd" => 1,
"network-child" => 1,
"network-find" => 1,
"network-find-sub-ghz" => 1,
"network-find2" => 1,
"network-tests" => 1,
"ota-dynamic-block-period" => 1,
"ota-eeprom-test-real_ecc" => 1,
"ota-multi-hop" => 1,
"ota-page-request-test" => 1,
"prepayment" => 1,
"reporting-failure" => 1,
"secure-ezsp" => 1,
"smart-outlet" => 1,
"standalone-bootloading" => 1,
"startup-onoff-level-colortemp" => 1,
"sub-ghz" => 1,
"test-harness-z3" => 1,
"test-harness-z3-zll" => 1,
"time" => 1,
"wwah-silabs" => 1,
"wwah-survey" => 1,
"xncp-host" => 1,
"z3-distributed" => 1,
"z3-stress-testing" => 1,
"zigbee-event-logger" => 1,
"zigbee-minimal-app" => 1,
"zll-14778" => 1,
"zll-abort" => 1,
"zll-commissioning" => 1,
"zll-end-device" => 1,
"zll-frame-counter" => 1,
"zll-identify" => 1,
"zll-multiple-lights" => 1,
"zll-network-interoperability" => 1,
"zll-network-interop2" => 1,
"zll-network-update" => 1,
"zll-policy" => 1,
"zll-reset" => 1,
"zll-router-initiator" => 1,
"zll-rx-on-when-idle" => 1,
"zll-scenes" => 1,
"zll-sleepy" => 1,
"em4-sleep" => 1,
);

################################################################################

exit Main();

sub Main
{
  my $OutputFile;

  Getopt::Long::config("bundling");
  GetOptions
  (
   'o|output-file=s' => \$OutputFile,
   'd|debug' => \$DEBUG,
  ) or die $USAGE;

  die "Error: Must specify output file with '-o'.\n" unless $OutputFile;

  my $Status = open(OUTPUT, ">$OutputFile");
  unless ($Status) {
    die "Error: Could not output file '$OutputFile': $!\n";
  }

  $Status = opendir(DIR, "$SCRIPT_DIR/$TEST_SCRIPT_RELATIVE_DIRECTORY");
  unless ($Status) {
    die "Error: Could not open directory '$SCRIPT_DIR/$TEST_SCRIPT_RELATIVE_DIRECTORY': $!\n";
  }

  my @Contents = readdir(DIR);
  closedir(DIR);
  my @Tests = sort grep /\.bsh$/i, @Contents;
  @Tests = grep { "$_" ne "util.bsh" } @Tests;

  print "Generating Sample Applications Makefile\n";

  print "Beanshell Files found: " .  scalar(@Tests) . "\n";
#  print join("\n", @Tests) . "\n";

  my %AppToTestHash;
  my %TestToAppHash;

  foreach my $File ( @Tests ) {
    next if (exists $TESTS_TO_SKIP_HASH{$File});
    my $Command = "grep \\\"build/ $SCRIPT_DIR/$TEST_SCRIPT_RELATIVE_DIRECTORY/$File";
    my @Output = `$Command`;
    foreach my $Line ( @Output ) {
      next if ($Line =~ m%^\s*//%);
      chomp $Line;
      $Line =~ s%^.*"build/%build/%;
      $Line =~ s%".*$%%;
      $AppToTestHash{$Line}{$File} = 1;
      $TestToAppHash{$File}{$Line} = 1;
    }
    my $Command2 = "grep app/ncp/ $SCRIPT_DIR/$TEST_SCRIPT_RELATIVE_DIRECTORY/$File";
    my @Output2 = `$Command2`;
    foreach my $Line ( @Output2 ) {
      next if ($Line =~ m%^\s*//%);
      chomp $Line;
      $Line =~ s%^.*"app/ncp/sample-app/%app/ncp/sample-app/%;
      $Line =~ s%".*$%%;
      $AppToTestHash{$Line}{$File} = 1;
      $TestToAppHash{$File}{$Line} = 1;
    }
  }

  print "Apps needed for all tests: " . scalar(keys %AppToTestHash) . "\n";

  if ($DEBUG) {
    foreach my $App ( sort keys %AppToTestHash ) {
      print "$App:\n";
      print "  " . join("\n  ", sort keys %{$AppToTestHash{$App}}) . "\n";
    }
    print "\n";
  }

  # No single quotes around end-of-text delimiter to allow
  # interpretation of $0 as a Perl variable.
  print OUTPUT <<END_OF_TEXT;
# AUTOMATICALLY GENERATED FILE -- DO NOT BOTHER MODIFYING BY HAND
#
#   Generated by $0
#
# This Makefile details what is necessary to run a specific Afv2 scripted test.

END_OF_TEXT

  # Using single quotes around the end-of-text delimiter prevents
  # interpretation of $ as Perl variables.  
  print OUTPUT <<'END_OF_TEXT';

# Stupid CYGWIN.  The "short" uname still shows up with a long winded
# thing like "CYGWIN_NT-6.1" and on 64-bit Windows it is "CYGWIN_NT-6.1-WOW64".
# Hey CYGWIN, the '-s' flag means SHORT!!!
# Filter the name to remove the suffix.   Linux and Mac report 'Linux' and 'Darwin'
# respectively and are not effected by the sed expression.
UNAME:=$(shell uname -s | sed -e 's/^\(CYGWIN\).*/\1/')

ifneq ($(UNAME),Darwin)
  ifneq ($(UNAME),Linux)
    ifneq ($(UNAME),CYGWIN)
      $(error "Error: Unknown OS '$(UNAME)', won't proceed.")
    endif
  endif
endif

ifndef ($(BEANSHELL_DIR))
  BEANSHELL_DIR := ../../tool/beanshell
endif

ifeq ($(UNAME),CYGWIN)
  # Use ';' for path separator.

  # Something changed and javac now doesn't like absolute Unix paths on Windows.
  # Giving it a standard Windows path seems to solve the problem.
  WINDOWS_BEANSHELL_DIR=$(shell cygpath --mixed $(BEANSHELL_DIR))

  # Also we need quotes surrounding this to prevent Cygwin from interpreting the ';' 
  # as a break between shell commands.
  CLASSPATH:="${WINDOWS_BEANSHELL_DIR}/bsh-2.0b4.jar;build"
else
  # Use ':' for path separator.
  CLASSPATH:=${BEANSHELL_DIR}/bsh-2.0b4.jar:build 
endif

ifeq ($(UNAME),Darwin)
  ECC_SUPPORT=0
endif

.PHONY: all
all: run-all-afv2-tests

scope:
ifndef NO_SCOPE
	make -f Makefile scope
endif

END_OF_TEXT

  foreach my $Test ( sort keys %TestToAppHash ) {
    next if (exists $TESTS_TO_SKIP_HASH{$Test.".bsh"});

    my $SimpleTestName = $Test;
    $SimpleTestName =~ s/\.bsh$//;

    my @TestEccApps; 
    my @TestNonEccApps;
    foreach my $Key ( sort keys %{$TestToAppHash{$Test}} ) {
      if ($Key =~ m/REAL_ECC/
          || $Key =~ m/ecc\-/
          || $Key =~ m/real_ecc/) {
        push @TestEccApps, $Key;
      } else {
        push @TestNonEccApps, $Key;
      }
    }

    if (scalar(@TestEccApps) > 0) {
      my $TestEccAppList = join " \\\n      ", @TestEccApps;
      my $TestMakefileVarName = uc($SimpleTestName) . "_ECC_APPS";
      $TestMakefileVarName =~ s/\-/_/g;

      print OUTPUT <<END_OF_TEXT;
$TestMakefileVarName = 
ifneq (\$(UNAME),Darwin)
  ifndef NO_REAL_ECC
    $TestMakefileVarName = \\
      $TestEccAppList
  endif
endif

END_OF_TEXT
      print OUTPUT ".PHONY: run-$SimpleTestName\n";
      print OUTPUT "run-$SimpleTestName: \\\n    " . "scope \\\n    " . join(" \\\n    ", @TestNonEccApps) . " \\\n    \$($TestMakefileVarName)\n"
    } else {
      print OUTPUT ".PHONY: run-$SimpleTestName\n";
      print OUTPUT "run-$SimpleTestName: \\\n    " . "scope \\\n    " . join (" \\\n    ", @TestNonEccApps) . "\n";
    }
    if (exists $TESTS_WITH_ONLY_SINGLE_HOP_HASH{$SimpleTestName}) {
      print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -d $SimpleTestName\n\n";
      print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -d -h $SimpleTestName\n\n";
    } else {
      print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -d -m $SimpleTestName\n\n";
      print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -d -h -m $SimpleTestName\n\n";
    }
  }

  my @EccApps; 
  my @NonEccApps;
  foreach my $Key ( sort keys %AppToTestHash ) {
    if ($Key =~ m/REAL_ECC/
        || $Key =~ m/ecc\-/
        || $Key =~ m/real_ecc/) {
      push @EccApps, $Key;
    } else {
      push @NonEccApps, $Key;
    }
  }
  print "  ECC Apps:     " . scalar @EccApps . "\n";
  print "  Non-ECC Apps: " . scalar @NonEccApps . "\n";

  my $EccAppList = join " \\\n      ", @EccApps;
  print OUTPUT <<END_OF_TEXT;
ALL_ECC_APPS = 
ifneq (\$(UNAME),Darwin)
  ifndef NO_REAL_ECC
    ALL_ECC_APPS = \\
      $EccAppList
  endif
endif

.PHONY: run-all-afv2-tests
run-all-afv2-tests: \\
END_OF_TEXT

  print OUTPUT "    " . join(" \\\n    ", @NonEccApps) . " \\\n    \$(ALL_ECC_APPS)\n";
  print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -a\n";
  print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -a -h\n";
  print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -a -m\n";
  print OUTPUT "\t$RUN_AF_SCRIPT_COMMAND -a -h -m\n";

  print OUTPUT <<END_OF_TEXT;

###############################################################################
# Sample App Clean

.PHONY: clean
clean:
END_OF_TEXT
  print OUTPUT "\trm -rf $BUILD_DIRECTORY\n";


  print OUTPUT <<END_OF_TEXT;

###############################################################################
# Sample App Build Commands

END_OF_TEXT


  foreach my $App ( sort keys %AppToTestHash ) {
    print OUTPUT ".PHONY: $App\n";
    print OUTPUT "$App:\n";
    # If the path has '-unix' in it this means that the
    # target is still built via Jam.
    my $BuildLine;
    if ($App =~ m/\-unix/) {
      $BuildLine = ParseBuildLineFromStackPath($App);
    } elsif ($App =~ m%app/ncp/sample-app%) {
      $BuildLine = ParseBuildLineFromAfv6Path($App);
    }else {
      $BuildLine = ParseBuildLineFromAfv2Path($App) . " -j";
    }
    print OUTPUT "\t$BuildLine\n\n";
  }  
  close OUTPUT;

  return 0;
}

sub ParseBuildLineFromAfv2Path
{
  my ($Path) = @_;

  $Path =~ s%^$BUILD_DIRECTORY/(.*)/.*%$1%;
  my $Split = "\\-simulation";
  my $Ezsp = "";
  if ($Path =~ m/\-simulation\-ezsp/) {
    $Split = "\\-simulation\\-ezsp";
    $Ezsp = "-ezsp";
  }
  my ($App, $Defines) = split/$Split/, $Path, 2;
  if ($Defines) {
    $Defines =~ s/^\-//;
    $Defines =~ s/\-/ /g;
    $Defines = uc($Defines);
  } else {
    $Defines = "";
  }
  my $AppDirectory = ConvertCamelCaseToDashedWords($App);
  my $ThisIsScenarioApp = CheckForScenarioApp($App);
  my $Prefix = ($ThisIsScenarioApp 
                ? $ThisIsScenarioApp
                : "$SAMPLE_APP_BUILD_PATH/$AppDirectory" );
  my $Line = "$MAKE_COMMAND $Prefix/gen/simulation$Ezsp/Makefile";
  $Line .= " GLOBAL_DEFINES=\"$Defines\"" if $Defines;
  return $Line;
}

sub ParseBuildLineFromAfv6Path
{
  my ($Path) = @_;

  $Path =~ s%build/exe/.*%%;
  #$Path =~ s%^.*\"%%;

  my $Line = "make -C $Path";
  #$Line .= " GLOBAL_DEFINES=\"$Defines\"" if $Defines;
  return $Line;
}

sub ParseBuildLineFromStackPath
{
  my ($Path) = @_;

  $Path =~ s%^build/(.*)/.*%$1%;
  #my ($App, $Defines) = split/-unix\-simulation\b|-unix\-simulation\_trx\b/, $Path, 2;
  my ($App, $Defines) = split/-unix\-simulation\b|-unix\-simulation\_trx\b|-unix\-simulation\_gb\b|-unix\-simulation\_dual\b/, $Path, 2;
  if ($Defines) {
    $Defines =~ s/^\-//;
    $Defines =~ s/\-/ /g;
    $Defines = uc($Defines);
    $Defines =~ s/EMBER_MULTI_NETWORK_STRIPPED/\+EMBER_MULTI_NETWORK_STRIPPED/g;
    $Defines =~ s/DEBUG_STRIPPED/\+DEBUG_STRIPPED/g;
  } else {
    $Defines = "";
  }

  if ($Path =~ m/\-simulation_trx/) {
    return "$BUILD_SCRIPT $App $SIMULATION_BUILD_PARAMS_USING_TRANSCEIVER $Defines";
  } elsif ($Path =~ m/\-simulation_dual/) {
    return "$BUILD_SCRIPT $App $SIMULATION_DUAL_BUILD_PARAMS $Defines";
  } elsif ($Path =~ m/\-simulation_gb/) {
    return "$BUILD_SCRIPT $App $SIMULATION_GB_BUILD_PARAMS $Defines";
  } else {
    return "$BUILD_SCRIPT $App $SIMULATION_BUILD_PARAMS $Defines";
  }
}

sub ConvertCamelCaseToDashedWords
{
  my ($Item) = @_;
  # This will break up the app name based on Uppercase letters AND capture
  # the uppercase letters separately in the array.
  my @Words = split/([A-Z])/, $Item;
  $Item = "";
  foreach my $Word ( @Words ) {
    $Item .= lc($Word);
    # If it is not a single uppercase letter, then add a dash on the end.
    # This means it was not just the "Z" of the "Zll" app name, it is the 
    # "ll" of that name.
    if ($Word && $Word !~ m/[A-Z]{1}/) {
      $Item .= "-";
    }
  }
  # Remove trailing dash
  $Item =~ s/\-$//;
  return $Item;
}

sub CheckForScenarioApp
{
  my ($AppDirectory) = @_;
  my $Command = "find $SCENARIO_APP_BUILD_PATH -type d -name '$AppDirectory'";
  DebugPrint("Find command: $Command\n");
  my @Output = `$Command`;
  die "Error:  Find in scenario application directory failed: $!\n" if $?;
  if ($Output[0]) {
    my $Path = $Output[0];
    chomp $Path;
    DebugPrint("Found scenario app: $Path\n");
    my $SampleAppName = ConvertCamelCaseToDashedWords($AppDirectory);
    if ( -e "$SAMPLE_APP_BUILD_PATH/$SampleAppName/$AppDirectory" . "_callbacks.c" ) {
      die "FATAL:  Scenario app name '$AppDirectory' cannot have the same name as a sample application: '$SampleAppName'.\n";
    }
    return $Path;
  }
  return undef;
}


sub DebugPrint
{
  my ($Line) = @_;
  print $Line if $DEBUG;
}

