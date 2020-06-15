#!/usr/bin/perl -w

################################################################################
# See $USAGE below for description.

################################################################################
# Modules

use strict;
use Getopt::Long;
use File::Basename;
use File::Copy;

# For some reason the built-in 'chdir' does not correctly change the path for
# all child processes.  This was particular a problem for 'p4'.  By importing
# this module we override the default mechanism and that did the right thing
# such that Perforce was using the same directory as the parent process
# when executing commands.  This may no longer be necessary with Git, but it
# doesn't seem to hurt.
use Cwd 'chdir';

################################################################################
# Globals

my $DEFAULT_SAMPLE_APPS_DIRECTORY = "app/framework/sample-apps";
my $SCENARIO_SPECIAL_DIRECTORY = "app/framework/scenarios";

my $EXECUTABLE_PATH=$0;
my $THIS=$0; $THIS=~s%.*/%%;
my $USAGE = <<END_OF_TEXT;

  $THIS [ options ]

  This script will re-generate one or more App Builder sample applications that
  live in app/framework/sample-apps/ or app/framework/scenarios/ for the
  simulation and simulation-ezsp targets.

  It looks for an ISC file in the sample-app directory (first for '*35x.isc'
  and then for '*.isc') and will use that with AppBuilder to regenerate.

  It does the following:
    1. Copy the ISC file to a temp location and modify it for SOC and/or Host.
    2. Run AppBuilder against the sample/scenario application(s)
    3. Update generated Makefile to correct paths based on the location
       of the generated files and to handle some special cases.
       (e.g. ECC library location)
    4. Update test-headers directory for unit tests.
    5. Generate an Info file for AppBuilder to use when presenting the user
       with a list of sample / scenario applications to chose from.

  Options

    -a, --all
      Regenerate all sample/scenario applications.  This script must be run
      from either $DEFAULT_SAMPLE_APPS_DIRECTORY or $SCENARIO_SPECIAL_DIRECTORY

    -r, --regenerate-isc
      Do not generate all the files, only rewrite ISC files to a latest version.
      This switch should be used in conjuction with -a or -d.

    -d, --directory <sample-app-directory>
      Re-generate files only for the specified sample application directory.
      Multiple '-d' options allowed.

    --debug
      Print debugging output. Includes output from all commands.

    -k, --keep-going
      If possible, don't stop on an error.  Keep going.

    --skip-appbuilder
      Skip running AppBuilder.  Useful for debugging this script.

    --appbuilder-output
      Normally AppBuilder output is suppressed unless there is an error or '--debug'
      is passed in.  This prints the AppBuilder output in addition to normal script
      output.

    --no-delete
      Do not delete backup files, temp files, or anything else the script
      normally cleans up.

    --target <soc|host|all>
      Generate data for SOC simulation platform or host simulation or both.
      Optional argument.  Default is all.

END_OF_TEXT

my $DEBUG = 0;
my $KEEP_GOING = 0;
my $REGENERATE_ISC = 0;
my $LOAD_STACK = 0;

my $CREATE_SAMPLE_APP_MAKEFILE_SCRIPT = "create-sample-app-makefile.pl";
my $CREATE_SAMPLE_APP_MAKEFILE_OUTPUT = "sample-apps.mak";

my $ISD_LOCATION    = "../../../../../../ember-desktop/build/InSightDesktop";
my $STUDIO_LOCATION = "../../../../../../studio/build/studio";
my $ZNET_LOCATION   = Cwd::realpath(Cwd::getcwd . "/../../../");

my $ISD_EXECUTABLE_WINDOWS="InSightDesktop.exe";
my $ISD_EXECUTABLE_LINUX="InSightDesktop";
my $ISD_EXECUTABLE_MACOSX="Eclipse.app/Contents/MacOS/eclipse";
my $STUDIO_EXECUTABLE_WINDOWS="studio.exe";
my $STUDIO_EXECUTABLE_LINUX="studio";
my $STUDIO_EXECUTABLE_MACOSX="Simplicity Studio.app/Contents/MacOS/studio";

my @SPECIAL_HOST_HEADER_LIST = (
                                "se-full-th/SeFullTh260.h",
                                "se-ota-eeprom-test/SeOtaEepromTest260.h",
                               );

my $ISC_FILE_FOR_TEST_HEADERS = "full-th/gen/simulation/FullTh.isc";

my $TEST_HEADERS_DIR = "../test/headers";

my %TARGET_HASH = (
                     "Soc" => {
                               frameworkVariant => "simulation",
                               PlatformTarget => "simulation",
                               DirectorySuffix => "simulation",
                              },
                     "Host" => {
                                frameworkVariant => "simulationezsp",
                                PlatformTarget => "simulation-ezsp",
                                DirectorySuffix => "simulation-ezsp",
                                HostTarget =>     "simulation-ezsp",
                               },
                     );

my $GENERATED_ROOT_DIR = "gen";

# We should take care to make sure these excluded generated projects
# are not used in any unit tests!
my %EXCLUDED_GENERATED_FILES = (
  "XncpHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # This sample-app is not used in SoC tests.
  "SecureEzspHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # This sample-app is not used in any beanshell tests.
  "XncpLedHost.isc" => {
    "PlatformTarget" => "All",
  },
  # The 1.2b ESI is an OTA server, so it must be a host app.
  "SeSample12bEsiHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The Communications hub has too many clusters and attributes
  # and other GBCS specific functionality to make it fit on an SOC.
  # All known Comms Hub implementors are using a co-processor model.
  "SeSampleCommsHubHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The Communications hub has too many clusters and attributes
  # and other GBCS specific functionality to make it fit on an SOC.
  # All known Comms Hub implementors are using a co-processor model.
  # Se14CommsHub is only for simulation testing and we always use
  # host version.
  "Se14CommsHub.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The handheld terminal has too many clusters and attributes
  # and other GBCS specific functionality to make it fit on an SOC.
  # All known HHT implementors are using a co-processor model.
  "SeSampleHHTHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The GSME utilizes the SE 1.2 Key Establishment procedure so
  # to avoid issues with references the the ECC 163k1 and 283k1
  # libraries we will not build for the SOC.
  "SeSampleGSMESoc.isc" => {
    "PlatformTarget" => "Soc",
  },
  #GPTestDevice is only for simulation testing and we always use the soc version
  "GpTestDevice.isc" => {
    "PlatformTarget" => "Host",
  },
  # The Z3 Gateway can only be compiled as host
  "Z3GatewayHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The Z3 Gateway With WWAH can only be compiled as host
  "Z3GatewayWithWwahHost.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The Z3 Gateway With WWAH wo/ TC Keep Alive can only be compiled as host
  "Z3GatewayWithWwahNoTcKeepAlive.isc" => {
    "PlatformTarget" => "Soc",
  },
  # The dimmer switch can only be compiled as soc
  "Z3CapSenseDimmerSwitchSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The lighting reference designs can only be compiled as soc
  "Z3ColorControlLightSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The lighting reference designs can only be compiled as soc
  "Z3ColorTempLightSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The lighting reference designs can only be compiled as soc
  "Z3DimmableLightSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The contact sensor can only be compiled as soc
  "Z3ContactSensorSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The contact sensor can only be compiled as soc
  "Z3ContactSensorWithWwahSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The occupancy sensor can only be compiled as soc
  "Z3OccupancySensorSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The smart outlet can only be compiled as soc
  "Z3SmartOutletSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The minimal application can only be compiled as soc
  "ZigbeeMinimalSoc.isc" => {
    "PlatformTarget" => "Host",
  },
  # The SwitchedMultiprotocolJoiningDeviceSoc app shouldn't be generated at all
  # because we don't use it in the repo
  "SwitchedMultiprotocolJoiningDeviceSoc.isc" => {
    "PlatformTarget" => "All",
  },
  # The TestHarnessZ3 app will only be used as SoC
  "TestHarnessZ3.isc" => {
    "PlatformTarget" => "Host",
  },
  # The TestHarnessZ3Sleepy app will only be used as SoC
  "TestHarnessZ3Sleepy.isc" => {
    "PlatformTarget" => "Host",
  },
  # The DMP apps lack simulation support. They are also only for SoC
  "DynamicMultiprotocolZigBeeZllLightBleSoc.isc" => {
    "PlatformTarget" => "All",
  },
  "DynamicMultiprotocolLightSoc.isc" => {
    "PlatformTarget" => "All",
  },
  "DynamicMultiprotocolLightSedSoc.isc" => {
    "PlatformTarget" => "All",
  },
  "DynamicMultiprotocolSwitchSoc.isc" => {
    "PlatformTarget" => "All",
  },
  # Don't generate anything for DmpSqaTpca and DmpSqaTpsa because we don't run
  # them in simulation
  "DmpSqaTpca.isc" => {
    "PlatformTarget" => "All",
  },
  "DmpSqaTpsa.isc" => {
    "PlatformTarget" => "All",
  },
  # The Z3 Test Harness can only be compiled as soc
  "TRaC_TestHarnessZ3.isc" => {
    "PlatformTarget" => "Host",
  },
  # The Z3 Test Harness End Device can only be compiled as soc
  "TRaC_TestHarnessZ3EndDevice.isc" => {
    "PlatformTarget" => "Host",
  },
  # The Z3 Test Harness Sleepy End Device can only be compiled as soc
  "TRaC_TestHarnessZ3SleepyEndDevice.isc" => {
    "PlatformTarget" => "Host",
  },

);

# The makefile normally wants to live at the root of our stack since
# all references are relative to that location.  We rewrite the
# Makefile so it can be stored somewhere else.
my %MAKEFILE_VARIABLE_REPLACEMENT_HASH = (
                                          # {MAKEFILE_DIR} is the location of the generated makefile
                                          # {SAMPLE_APP_DIR} is the location of the directory in
                                          #   source control where the sample-app lives.
                                          # {PLATFORM_TARGET} is either simulation or simulation-ezsp.
                                          APP_BUILDER_OUTPUT_DIRECTORY => "{MAKEFILE_DIR}",
                                          APP_CALLBACK_FILE_DIRECTORY  => "{SAMPLE_APP_DIR}",
                                          OUTPUT_DIR => "\$(AF_TEST_APPS_DIR)/{APP_NAME}-{PLATFORM_TARGET}",
                                         );

my %SPECIAL_ISC_HANDLING = (
  "se-full-th" => {
    "simulation-ezsp" => {
      "Replacement" => {
        # Remove this option as it is not allowed on the host.
        "AppPluginOption:EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,true"
          => "AppPluginOption:EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,false",
          # Remove this option as it is not allowed on the host.
        "AppPluginOption(simulation):EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,true"
          => "AppPluginOption(simulation-ezsp):EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,false",
      },
    },
  },
  "ha-home-gateway" => {
    "simulation-ezsp" => {
      "Replacement" => {
        # Recording LQI/RSSI requires the packet-handoff plugin, which isn't supported on the host
        "AppPluginOption:EMBER_AF_PLUGIN_STACK_DIAGNOSTICS_RECORD_LQI_RSSI,true"
          => "AppPluginOption:EMBER_AF_PLUGIN_STACK_DIAGNOSTICS_RECORD_LQI_RSSI,false",
      },
    },
  },
  "dynamic-ota-server" => {
    "simulation-ezsp" => {
      "Replacement" => {
        # Remove this option as it is not allowed on the host.
        "AppPluginOption:EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,true"
          => "AppPluginOption:EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,false",
          "AppPluginOption(simulation):EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,true"
          => "AppPluginOption(simulation-ezsp):EMBER_AF_PLUGIN_INTERPAN_ALLOW_APS_ENCRYPTED_MESSAGES,false",
      },
    },
  },
  "zse/SeSampleEsi" => {
    "simulation" => {
      "Replacement" => {
        # Say that this app uses the 163k1 ECC library in simulation so that
        # appbuilder will not throw a ConfigurationException.
        "UseEcc163k1:false"
        => "UseEcc163k1:true\n" .
           "Ecc163k1Path:build/ecc-library-internal-unix-gcc-simulation-simulation/ecc-library-internal.a",
      },
    },
  },
  "zse/SeSamplePCT" => {
    "simulation" => {
      "Replacement" => {
        # Say that this app uses the 163k1 ECC library in simulation so that
        # appbuilder will not throw a ConfigurationException.
        "UseEcc163k1:false"
        => "UseEcc163k1:true\n" .
           "Ecc163k1Path:build/ecc-library-internal-unix-gcc-simulation-simulation/ecc-library-internal.a",
      },
    },
  },
  "zse/SeSample12bEsi" => {
    "simulation" => {
      "Replacement" => {
        # Say that this app uses the 163k1 ECC library in simulation so that
        # appbuilder will not throw a ConfigurationException.
        "UseEcc163k1:false"
        => "UseEcc163k1:true\n" .
           "Ecc163k1Path:build/ecc-library-internal-unix-gcc-simulation-simulation/ecc-library-internal.a",
        # Say that this app uses the 283k1 ECC library in simulation so that
        # appbuilder will not throw a ConfigurationException.
        "UseEcc283k1:false"
        => "UseEcc283k1:true\n" .
           "Ecc283k1Path:build/ecc-library-283k1-internal-unix-gcc-simulation-simulation/ecc-library-283k1-internal.a",
      },
    },
  },
  "zse/SeSample12bIhd" => {
    "simulation" => {
      "Replacement" => {
        # Say that this app uses the 163k1 ECC library in simulation so that
        # appbuilder will not throw a ConfigurationException.
        "UseEcc163k1:false"
        => "UseEcc163k1:true\n" .
           "Ecc163k1Path:build/ecc-library-internal-unix-gcc-simulation-simulation/ecc-library-internal.a",
        # Say that this app uses the 283k1 ECC library in simulation so that
        # appbuilder will not throw a ConfigurationException.
        "UseEcc283k1:false"
        => "UseEcc283k1:true\n" .
           "Ecc283k1Path:build/ecc-library-283k1-internal-unix-gcc-simulation-simulation/ecc-library-283k1-internal.a",
      },
    },
  },
  "z3/Z3Gateway" => {
    "simulation-ezsp" => {
      "Replacement" => {
        # We are going to turn on non-compliant behavior when we run the Z3Gateway
        # app in beanshell. We are not going to include this plugin in the
        # Z3Gateway sample app by default since it contains non-compliant
        # behavior and therefore customers may run into certification issues if
        # they use this sample app as a template for their final design. We are
        # also not going to create a new sample app to test this functionality
        # because that would just add another sample app to our (currently) 71
        # existing apps.
        "appPlugin:update-ha-tc-link-key,false"
        => "appPlugin:update-ha-tc-link-key,true",
        # These are plugins that the above plugin needs to function.
        "appPlugin:device-database,false"
        => "appPlugin:device-database,true",
        "appPlugin:device-query-service,false"
        => "appPlugin:device-query-service,true",
      },
    },
  },
  # The NVM3 plugin needs the NVM3 plugin when built for simulation. This is
  # because we need to build its library using GCC/clang and also to pick up
  # some simulation-specific macros
  # We take stack-diagnostics to print child tables in testing
  "z3/Z3Light" => {
    "simulation" => {
      "Replacement" => {
        "appPlugin:nvm3,true"
        => "appPlugin:nvm3,true\n" .
           "appPlugin:nvm3-simulation,true",
        "appPlugin:nvm3-simulation,false"
        => "",
        "appPlugin:stack-diagnostics,false"
        => "appPlugin:stack-diagnostics,true",
      },
    },
  },
  # The NVM3 plugin needs the NVM3 plugin when built for simulation. This is
  # because we need to build its library using GCC/clang and also to pick up
  # some simulation-specific macros
  "z3/Z3SleepyDoorLockWithWwah" => {
    "simulation" => {
      "Replacement" => {
        "appPlugin:nvm3,true"
        => "appPlugin:nvm3,true\n" .
           "appPlugin:nvm3-simulation,true",
        "appPlugin:nvm3-simulation,false"
        => "",
      },
    },
  },
  # The NVM3 plugin needs the NVM3 plugin when built for simulation. This is
  # because we need to build its library using GCC/clang and also to pick up
  # some simulation-specific macros
  "z3/Z3Switch" => {
    "simulation" => {
      "Replacement" => {
        "appPlugin:nvm3,true"
        => "appPlugin:nvm3,true\n" .
           "appPlugin:nvm3-simulation,true",
        "appPlugin:nvm3-simulation,false"
        => "",
      },
    },
  },
  # The NVM3 plugin needs the NVM3 plugin when built for simulation. This is
  # because we need to build its library using GCC/clang and also to pick up
  # some simulation-specific macros
  "z3/ZigbeeMinimal" => {
    "simulation" => {
      "Replacement" => {
        "appPlugin:nvm3,true"
        => "appPlugin:nvm3,true\n" .
           "appPlugin:nvm3-simulation,true",
        "appPlugin:nvm3-simulation,false"
        => "",
      },
    },
  },


  # We need a ota storage simple plugin for ota server in order to load a file
  # from different directory than current one. Furthermore, ota storage simple
  # plugin requires ota-storage-simple-eeprom and eeprom plugins.
  # Currently ota-storage-posix-filesystem does not support loading ota file
  # other directory than current one.
  "z3/Z3GatewayWithWwah" => {
    "simulation-ezsp" => {
      "Replacement" => {
        "appPlugin:ota-storage-posix-filesystem,true"
        => "appPlugin:ota-storage-posix-filesystem,false",
        "appPlugin:ota-storage-simple,false"
        => "appPlugin:ota-storage-simple,true",
        "appPlugin:ota-storage-simple-eeprom,false"
        => "appPlugin:ota-storage-simple-eeprom,true\n" .
           "AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT,true",
        "appPlugin:eeprom,false"
        => "appPlugin:eeprom,true",
      },
    },
  },

  # We need a ota storage simple plugin for ota server in order to load a file
  # from different directory than current one. Furthermore, ota storage simple
  # plugin requires ota-storage-simple-eeprom and eeprom plugins.
  # Currently ota-storage-posix-filesystem does not support loading ota file
  # other directory than current one.
  "Z3GatewayWithWwahNoTcKeepAlive" => {
    "simulation-ezsp" => {
      "Replacement" => {
        "appPlugin:ota-storage-posix-filesystem,true"
        => "appPlugin:ota-storage-posix-filesystem,false",
        "appPlugin:ota-storage-simple,false"
        => "appPlugin:ota-storage-simple,true",
        "appPlugin:ota-storage-simple-eeprom,false"
        => "appPlugin:ota-storage-simple-eeprom,true\n" .
           "AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT,true",
        "appPlugin:eeprom,false"
        => "appPlugin:eeprom,true",
      },
    },
  },
  # We need simple eeprom read modify write support to true for simulation to perform ota downlaod. 
  "z3/Z3DoorLockWithWwah" => {
    "simulation" => {
      "Replacement" => {
        "AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT,false"
        => "AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT,true",
        "AppPluginOption(simulation):EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT,false"
        => "AppPluginOption(simulation):EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_READ_MODIFY_WRITE_SUPPORT,true",
      },
    },
  },
);

my %UART_HOST_APPLICATIONS = (
                              "SecureEzspHost.isc" => "uart",
                              "Z3GatewayWithWwahHost.isc" => "uart",
                              "Z3GatewayWithWwahNoTcKeepAlive.isc" => "uart",
                             );

my $ALL_SAMPLE_APPS_MAKEFILE = "all-apps.mak";

my %INFO_FILENAME = (
                     "$SCENARIO_SPECIAL_DIRECTORY" => "ScenarioAppsSoC.info",
                     "$DEFAULT_SAMPLE_APPS_DIRECTORY" => "SampleApps.info",
                     );

# Substitutions to turn directories into pretty app names for the .info file
my %PRETTY_NAME_RULES = (
  "12" => {
    "Substitution" => "1.2",
  },
  "14" => {
    "Substitution" => "1.4",
  },
  "#esi" => {
    "Substitution" => " Energy Services Interface",
  },
  "#ha" => {
    "Substitution" => " Home Automation",
  },
  "#hc" => {
    "Substitution" => " Health Care",
  },
  "#ipd" => {
    "Substitution" => " In-premise Display (IPD)",
  },
  "#mn" => {
    "Substitution" => " Multi-Network",
  },
  "#rf4ce" => {
    "Substitution" => " RF4CE",
  },
  # Match se12 or se14 or se (without matching words that start with se)
  "#se(?=\\s?\\d\.?\\d)|#se(?!\\w)" => {
    "Substitution" => " Smart Energy ",
  },
  "#tc" => {
    "Substitution" => " Trust Center",
  },
  "#th" => {
    "Substitution" => " Test Harness",
  },
  "#zll" => {
    "Substitution" => " ZigBee Light Link",
  },
  "#z3" => {
    "Substitution" => " Zigbee 3.0",
  },
);

# Descriptions for the .info file.
# Ideally the descriptions of the sample-apps / scenario apps would be listed in
# the ISC file.  However there is currently no support for that.  So we record
# them here.
my %DESCRIPTIONS = (
  # Scenarios
  "multiprotocol/SwitchedMultiprotocolJoiningDevice" => "This is a sample application demonstrating switched multiprotocol, acting as a joining device. This application uses zigbee 3.0 security. Requires IAR.",
  "multiprotocol/DynamicMultiprotocolZigBeeZllLightBle" => "This is a sample application demonstrating dynamic multiprotocol (ZigBee + BLE). Requires IAR.",
  "multiprotocol/DynamicMultiprotocolLight" => "This is a sample application demonstrating a light application using dynamic multiprotocol (ZigBee + BLE) and NVM3 for persistent storage. Requires IAR.",
  "multiprotocol/DynamicMultiprotocolLightSed" => "This is a sample application demonstrating a sleepy light application using dynamic multiprotocol (ZigBee + BLE) and NVM3 for persistent storage. Requires IAR.",
  "multiprotocol/DynamicMultiprotocolSwitch" => "This is a Zigbee switch application using NVM3 designed to work with the dynamic multiprotocol demonstration light. Requires IAR.",
  "rf4ce/Rf4ceDemo" => "This is a demonstration of an RF4CE/HA dual network device.",
  "rf4ce/Rf4ceMsoController" => "This is a remote control application for an RF4CE MSO network.",
  "rf4ce/Rf4ceMsoSetTopBox" => "This is a set top box application for an RF4CE MSO network.",
  "rf4ce/Rf4ceZrc11Controller" => "This is a controller application for an RF4CE ZRC 1.1 network.",
  "rf4ce/Rf4ceZrc11Target" => "This is a target application for an RF4CE ZRC 1.1 network.",
  "rf4ce/Rf4ceZrc20Controller" => "This is a controller application for an RF4CE ZRC 2.0 network.",
  "rf4ce/Rf4ceZrc20Target" => "This is a target application for an RF4CE ZRC 2.0 network.",
  "zha/HaSampleGateway" => "This is a gateway application for a Home Automation Network.",
  "zha/HaSampleLight" => "This is an on/off light for a Home Automation network.",
  "zha/HaSampleSwitch" => "This is a switch application for a Home Automation Network.",
  "zha/StandardizedRfTesting" => "This is a pre-standardization implementation of Zigbee's RF testing standard. It utilizes the TIS (Total Isotropic Sensivity)/ TRP (Total Radiated Power) testing interfaces and is optional for Zigbee certifications.",
  "z3/TRaC_TestHarnessZ3" => "This is a test harness router/coordinator application for Zigbee 3.0 compliance testing",
  "z3/TRaC_TestHarnessZ3EndDevice" => "This is a test harness end-device application for Zigbee 3.0 compliance testing",
  "z3/TRaC_TestHarnessZ3SleepyEndDevice" => "This is a test harness sleepy-end-device application for Zigbee 3.0 compliance testing",
  "z3/Z3DimmableLight" => "This is a Zigbee 3.0 light application.  The application provides pwm control for a dimming function.  It is intended to be used with the Z3Gateway sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kits RD-0035-0601 (EM3585), RD-0085-0401 (EFR32MG1P732F256IM32), RD-0098-0401 (EFR32MG12P432F1024IM32), WSTK (EFR32) and EM35X-DEV (EM358x).",
  "z3/Z3CapSenseDimmerSwitch" => "This is a Zigbee 3.0 sensor application.  The application provides open/close state of a magnetic reed switch and device tamper detection.  It is intended to be used with the Z3Gateway, available as a sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kit RD-0039-0201 (EM3587).",
  "z3/Z3ColorTempLight" => "This is a Zigbee 3.0 light application.  The application provides pwm control for dimming, and pwm mixing for color temperature functions.  It is intended to be used with the Z3Gateway sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kits RD-0035-0601 (EM3585), RD-0085-0401 (EFR32MG1P732F256IM32) and RD-0098-0401 (EFR32MG12P432F1024IM32).",
  "z3/Z3ColorControlLight" => "This is a Zigbee 3.0 light application.  The application provides pwm control for dimming, and pwm mixing for color functions.  It is intended to be used with the Z3Gateway sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kits RD-0035-0601 (EM3585), RD-0085-0401 (EFR32MG1P732F256IM32) and RD-0098-0401 (EFR32MG12P432F1024IM32).",
  "z3/Z3ContactSensor" => "This is a Zigbee 3.0 sensor application.  The application provides open/close state of a magnetic reed switch and device tamper detection.  It is intended to be used with the Z3Gateway, available as a sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kit RD-0030-0201 (EM3587).",
  "z3/Z3ContactSensorWithWwah" => "This is the Zigbee 3.0 contact sensor application with the Works With All Hubs cluster enabled.",
  "z3/Z3DoorLockWithWwah" => "This is a Zigbee 3.0 end device door-lock application with the Works With All Hubs cluster enabled.",
  "z3/Z3LightWithWwah" => "This is a Zigbee 3.0 router light application with the Works With All Hubs cluster enabled.",
  "z3/Z3SleepyDoorLockWithWwah" => "This is a Zigbee 3.0 sleepy end device door-lock application with the Works With All Hubs cluster enabled.",
  "z3/Z3SmartOutlet" => "This is a Zigbee 3.0 sensor and actuator application.  The application provides outlet on/off control; over-current and over-temperature shutdown; and power, temperature, relative humidity and illuminance measurements.    It is intended to be used with the Z3Gateway sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kits RD-0051-0201 (EFR32MG1P232F256GM48) and RD-0100-0201 (EFR32MG12P432F1024GM48).",
  "z3/Z3OccupancySensor" => "This is a Zigbee 3.0 sensor application.  The application provides occupancy sensing, temperature measurement, relative humidity measurement, and illuminance measurement.  It is intended to be used with the Z3Gateway sample application, or with the Zigbee gateway reference designs RD-0001-0201 and RD-0002-0201.  Use the application with kits RD-0078-0201 (EFR32MG1P232F256GM48) and RD-0099-0201 (EFR32MG12P432F1024GM48).",
  "zse/SeSampleCommsHub" => "This is a Communications Hub application for a Smart Energy 1.x network. NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "zse/SeSampleEsi" => "This is an Energy Services Interface application for a Smart Energy 1.x network. NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "zse/SeSampleGSME" => "This is a gas meter application for a Smart Energy 1.x network. NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "zse/SeSamplePCT" => "This is a Programmable Communicating Thermostat application for a Smart Energy 1.x network. NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "zse/SeSampleHHT" => "This is a Handheld Terminal application referenced in the Great Britain Companion Specification (GBCS). NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "zse/SeSample12bEsi" => "This is an Energy Services Interface application for a Smart Energy 1.2b network. NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "zse/SeSample12bIhd" => "This is an In-Home Display application for a Smart Energy 1.2b network. NOTE: Appropriate CBKE and ECC libraries MUST be enabled or this application will not build.",
  "xncp/XncpSensorHost" => "This is an example with a UNIX HOST communicating with a custom NCP using custom EZSP commands.",
  "z3/Z3Light" => "This is a ZigBee 3.0 light application using NVM3 as the persistent storage.",
  "z3/Z3Switch" => "This is a ZigBee 3.0 switch application using NVM3 as the persistent storage.",
  "z3/Z3LightGPCombo" => "This is a ZigBee 3.0 light application with Green Power endpoint, Green Power Proxy and Sink functionality.",
  "z3/ZigbeeMinimal" => "This is a ZigBee minimal network-layer application suitable as a starting point for new application development.",
  # Sample Apps
  "coulomb-counter-test" => "Coulomb Counter plugin barebone",
  "dynamic-ota-server" => "SE Full Test Harness with Dynamic OTA downloads",
  "full-th" => "Smart Energy Test Harness",
  "ha-combined-interface" => "Home Automation Combined Interface Device",
  "ha-door-lock" => "Home Automation Door Lock",
  "ha-home-gateway" => "Home Automation Gateway",
  "ha-light" => "Home Automation On/Off Light",
  "ha-range-extender" => "Home Automation Range Extender",
  "ha-switch" => "Home Automation On/Off Switch",
  "hc-weight-scale" => "Health Care Weight Scale",
  "mn-esi-ipd" => "Multi-network SE ESI on one network, IPD on another network.",
  "mn-z3-tc-se-ipd" => "Mutli-network Z3 coordinator on network 1, SE sleepy IPD on network 2.",
  "se-esp" => "Smart Energy Energy Service Portal",
  "se-full-th" => "Smart Energy Full Test Harness",
  "se-ipd" => "Smart Energy In-Premise Display",
  "se-meter-gas" => "Smart Energy Gas Meter",
  "se-meter-gas-sleepy" => "Smart Energy Sleepy Gas Meter",
  "se-meter-mirror" => "Smart Energy Mirror Device for Sleepy Meter",
  "se-ota-eeprom-test" => "Smart Energy Over-the-air Bootload Cluster Device with EEPROM",
  "se-pct" => "Smart Energy Procgrammable Control Thermostat",
  "se-range-extender" => "Smart Energy Range Extender",
  "sleepy-generic" => "Generic Sleepy device for testing fragmentation",
  "xncp-host" => "Host application connected to an Extendable NCP device.",
  "zll-color-light" => "ZigBee Light Link Color Light",
  "zll-color-scene-remote" => "ZigBee Light Link Color Scene Remote",
  "zll-color-scene-remote-sleepy" => "ZigBee Light Link Color Scene Remote (sleepy)",
  "zll-control-bridge" => "ZigBee Light Link Control Bridge",
  "zll-control-bridge-z3" => "ZigBee 3.0 Touchlink Control Bridge",
  "zll-on-off-light" => "ZigBee Light Link On/Off Light",
  "zll-on-off-switch" => "ZigBee Light Link On/Off Switch",
  "z3-coord-soc" => "Zigbee SoC Coordinator for testing",  
  "se12-comms-hub" => "SE1.2 CommsHub",
  "se12-esi" => "SE1.2 ESI",
  "se12-ihd" => "SE1.2 In Home Display",
  "se12-meter-electric" => "SE1.2 Electric Meter",
  "se12-meter-electric-alt-mac" => "SE1.2 Electric Meter Alt Mac",
  "se12-meter-gas" => "SE1.2 Gas Meter",
  "se14-comms-hub" => "SE1.4 CommsHub",
  "se14-ihd" => "SE1.4 In Home Display",
  "se14-meter-gas" => "SE1.4 Gas Meter",
  ## There is no HHT sample app
  "gp-proxy-basic" => "Green Power Proxy Basic",
  "gp-combo-basic" => "Green Power Combo Basic",
  "gp-test-device" => "Green Power Test Device",
);

# These are treated as prefixes for an item on a line.
my %CBKE_AND_ECC_LIBRARY_MAPPING = (
  "cbke-library-core" => "cbke-stub-library",
  "cbke-library-dsa-sign" => "cbke-library-dsa-sign-stub",
  "cbke-library-dsa-verify-283k1" => "cbke-library-dsa-verify-283k1-stub",
  "cbke-library-dsa-verify" => "cbke-library-dsa-verify-stub",
  "cbke-163k1-library" => "cbke-163k1-stub-library",
  "cbke-283k1-library" => "cbke-283k1-stub-library",
  "ecc-library-internal" => "ecc-stub-library",
  "ecc-library-283k1-internal" => "ecc-library-283k1-stub"
  );

my $EXCLUDE_FROM_INFO_FILE_MARKER = "exclude-from-info-file.txt";

my $STACK_ROOT_DIRECTORY = "../../..";
my $OLD_HOST_PLUGIN_DIRECTORY = "app/framework/plugin";
my $NEW_HOST_PLUGIN_DIRECTORY = "app/framework/plugin-host";
my @HOST_PLUGINS = `ls $STACK_ROOT_DIRECTORY/$NEW_HOST_PLUGIN_DIRECTORY`;
chomp @HOST_PLUGINS;

################################################################################

exit Main();

sub Main
{
  my $All;
  my @Directories;
  my $SkipAppBuilder;
  my $AppBuilderOutput;
  my $NoDelete;
  my $TreatAsScenario;
  my $SpecifiedTarget = "all";

  Getopt::Long::config("bundling");
  GetOptions
  (
   'a|all' => \$All,
   'd|directory=s' => \@Directories,
   'debug' => \$DEBUG,
   'k|keep-going' => \$KEEP_GOING,
   'skip-appbuilder' => \$SkipAppBuilder,
   'appbuilder-output' => \$AppBuilderOutput,
   'r|regenerate-isc' => \$REGENERATE_ISC,
   'no-delete'       => \$NoDelete,
   'treat-as-scenario' => \$TreatAsScenario,
   'target=s' => \$SpecifiedTarget,
  ) or die $USAGE;

  my $Cwd = `pwd`;
  chomp $Cwd;
  my $StackRelativeDir;
  if ($Cwd =~ m%$SCENARIO_SPECIAL_DIRECTORY$%) {
    $StackRelativeDir = $SCENARIO_SPECIAL_DIRECTORY;
  } elsif ($Cwd =~ m%$DEFAULT_SAMPLE_APPS_DIRECTORY$%) {
    $StackRelativeDir = $DEFAULT_SAMPLE_APPS_DIRECTORY;
  } else {
    die "Error: This script must be run from either $DEFAULT_SAMPLE_APPS_DIRECTORY"
      . " or $SCENARIO_SPECIAL_DIRECTORY\n";
  }
  unless (scalar(@Directories) || $All) {
    die "Error: Must specify '-a' or '-d <directory>'\n";
  }

  if (scalar(@Directories) && $All) {
    die "Error: Cannot specify both '-a' and '-d <directory>'\n";
  }

  my $Temp = ucfirst(lc($SpecifiedTarget));
  if ($Temp !~ /^All$/
      && !exists($TARGET_HASH{"$Temp"})) {
    die "Error: Unknown target '$SpecifiedTarget'.  Must be one of:\n"
      . "  " . lc(join(", ", keys %TARGET_HASH)) . ", all\n";
  }
  $SpecifiedTarget = $Temp;

  my @IscFiles;
  my %ScenarioApps;
  my @RealIscFiles;

  if ($REGENERATE_ISC) {
    # Don't copy the ISC files to a temporary location because we want
    # to modify them in place.  Studio should not generate any other files.

    # The @Directories may be empty in which case the subroutine will
    # use the CWD.
    @IscFiles = FindIscFiles(@Directories);
  } else {
    my ($IscFilesRef, $ScenarioAppsRef, $RealIscFilesRef)
      = GenerateTempIscFiles($All,
                             $StackRelativeDir,
                             $SpecifiedTarget,
                             @Directories);
    @IscFiles = @$IscFilesRef;
    %ScenarioApps = %$ScenarioAppsRef;
    @RealIscFiles = @$RealIscFilesRef;
  }

  my $Total = scalar(@IscFiles);
  die "Error: No ISC files found.\n" unless ($Total > 0);
  print "Generated $Total ISC files ";
  if ($SpecifiedTarget eq "All") {
    print "(both SOC and host targets).\n";
  } else {
    print "($SpecifiedTarget files only).\n";
  }

  if (exists($ENV{ISD_LOCATION})) {
    $ISD_LOCATION = $ENV{ISD_LOCATION};
    print "Using ISD_LOCATION from environment variable.\n";
  } else {
    print "No ISD_LOCATION environment variable.\n";
  }
  print "ISD_LOCATION=$ISD_LOCATION\n";

  if (exists($ENV{STUDIO_LOCATION})) {
    $STUDIO_LOCATION = $ENV{STUDIO_LOCATION};
    print "Using STUDIO_LOCATION from environment variable.\n";
  } else {
    print "No STUDIO_LOCATION environment variable.\n";
  }
  print "STUDIO_LOCATION=$STUDIO_LOCATION\n";

  my $Status = 0;
  unless ($SkipAppBuilder) {
    print "Running AppBuilder on " . scalar(@IscFiles) . " files\n";
    $Status = RunAppBuilder($AppBuilderOutput, @IscFiles);
  } else {
    print "Skipping running AppBuilder due to --skip-appbuilder option.\n";
  }

  if ($REGENERATE_ISC) {
    print "ISC Regeneration complete.\n";
    return $Status;
  }

  my @AllMakefiles = PostProcessGeneratedFiles($StackRelativeDir,
                                               $NoDelete,
                                               \%ScenarioApps);

  if ($All) {
    GenerateSampleAppInfoFile($StackRelativeDir, @RealIscFiles);
  }

  $Status = UpdateTestHeaders(@IscFiles)
    if ($StackRelativeDir eq $DEFAULT_SAMPLE_APPS_DIRECTORY);

  if ($Status) {
    warn "Errors encountered.\n";
  } else {
    print "All steps completed successfully.\n";
  }

  return $Status;
}

sub GenerateTempIscFiles
{
  my $All = shift @_;
  my $StackRelativeDir = shift @_;
  my $SpecifiedTarget = shift @_;
  my @Directories = @_;

  my @TempIscFiles;
  my %RealIscFilesHash;
  my %ScenarioApps;

  foreach my $TargetType ( keys %TARGET_HASH ) {
    DebugPrint("looking at target $TargetType\n");
    next unless ($SpecifiedTarget eq "All"
                 || $TargetType eq $SpecifiedTarget);

    my $PlatformTarget = $TARGET_HASH{"$TargetType"}{"PlatformTarget"};
    my $frameworkVariant = $TARGET_HASH{"$TargetType"}{"frameworkVariant"};
    my $GeneratedFilesDirectory = "$GENERATED_ROOT_DIR/" . $TARGET_HASH{"$TargetType"}{"DirectorySuffix"};
    my $HostTarget = (exists($TARGET_HASH{"$TargetType"}{"HostTarget"})
                      ? $TARGET_HASH{"$TargetType"}{"HostTarget"}
                      : "");

    # We need to add a path of '.' to the find shell command otherwise
    # find returns the full path.
    if ($All) {
      $Directories[0] = ".";
    }

    die "Error: Can only specify single directory (-d option) with scenario apps.\n"
      if (scalar(@Directories) && (scalar(@Directories) > 1));
    my @Files = FindFiles($Directories[0] || $StackRelativeDir, "*.isc");
    foreach my $File ( sort @Files ) {
      # Exclude files in generated directories since that is what we are
      # overwriting.
      next if $File =~ m%$GENERATED_ROOT_DIR/%;

      chomp $File;
      # If we added a '.' to the find command path then we want to strip
      # it out otherwise it looks silly in our concatenation (in theory
      # there should be no functional problem):
      #   foo/bar/./some/other/directory
      $File =~ s%^./%%;

      # Use a hash since this loops twice.  Otherwise we will get two entries
      # for ISC files, one for SOC and one for Host.
      $RealIscFilesHash{$File} = 1;

      my $Dir = dirname($File);
      my $Filename = basename($File);
      if (exists($EXCLUDED_GENERATED_FILES{"$Filename"}{"PlatformTarget"})) {
        my $platformTarget = $EXCLUDED_GENERATED_FILES{"$Filename"}{"PlatformTarget"};
        if ($TargetType eq $platformTarget || $platformTarget =~ /^all$/i) {
          print "Skipping $Filename for target $PlatformTarget.\n";
          RunCommand("rm -rf $Dir/$GeneratedFilesDirectory");
          next;
        }
      }

      DebugPrint("Analyzing scenario ISC: $File\n");

      # Insure directory does not have 'gen' in the name.
      #   ignore if it does.
      next if ($Dir =~ m%$GENERATED_ROOT_DIR/%);

      my $GenDir = $Dir . "/" . $GeneratedFilesDirectory;

      DebugPrint("  Creating gen dir '$GenDir'\n");
      my $Status = RunCommand("mkdir -p $GenDir");
      if ($Status) {
        WarnOrDie("Failed to create directory '$GenDir'");
        next;
      }
      DebugPrint("  Creating temp ISC file\n");
      my $TempFilename = $Filename;
      # Remove the EM3xx designator to prevent confusion
      $TempFilename =~ s%SoC%-simulation%i;
      my $TempIsc = $GenDir . "/" . $TempFilename;
      $Status = open(TEMP_ISC, ">$TempIsc");
      unless ($Status) {
        WarnOrDie("Failed to open temp ISC file '$TempIsc' for writing: $!\n");
        next;
      }
      $Status = open(ORIGINAL_ISC, "$File");
      unless ($Status) {
        close(TEMP_ISC);
        WarnOrDie("Failed to open original ISC file '$File' for reading: $!\n");
        next;
      }
      my $Line;
      my $LineNumber = 0;
      my $DeviceName;
      my $TokenFile;
      my $AlreadyPrinted = 0;
      while ($Line = <ORIGINAL_ISC>) {
        # I always keep track of line numbers when parsing a file this way,
        # even if they are not used.  They might be used later.
        $LineNumber++;

        # Get rid of all CRLF.  This causes problems when using end-of-line
        # in regexes, as the CR is not considered part of the end of the line
        # (at least not on those systems without the requirement of LF).
        $Line =~ s/[\x0A\x0D]//g;

        if ($Line =~ /^GenerationDirectory:.*/) {
          $Line = "GenerationDirectory:__SAMPLE_APPLICATION__\n";
        } elsif ($Line =~ /^frameworkVariant:.*/) {
          $Line = "frameworkVariant: " . $frameworkVariant . "\n";
        } elsif ($Line =~ /^DeviceName:(\w+)\s*$/) {
          $DeviceName = $1;
        } elsif ($Line =~ m%^TokenPath:([\w\-\_\.\\/]+)\s*$%) {
          $TokenFile = $1;
        } elsif ($Line =~ /^{setupId:token.*/) {
          my $nextLine = <ORIGINAL_ISC>;
          $nextLine =~ s/[\x0A\x0D]//g;
          if ($nextLine =~ /^PATH.*?:(.*)/) {
            $TokenFile = $1;
          }
          print TEMP_ISC "$Line\n";
          print TEMP_ISC "$nextLine\n";
          next;
        } elsif ($Line =~ m%^Platform:%) {
          $Line = "Platform:$PlatformTarget\n";
        } elsif ($Line =~ m%^Host:%) {
          if ($HostTarget) {
            $Line = "Host:$PlatformTarget\n";
          } else {
            $Line = "\n";
          }
        # Our host sample applications use UART for hardware, but we use SPI in
        # simulation.
        } elsif ($Line =~ m%^appPlugin:ezsp-spi,false$% and !exists($UART_HOST_APPLICATIONS{"$Filename"})) {
          $Line = "appPlugin:ezsp-spi,true\n";
          DebugPrint("Overriding ISC ezsp-spi configuration to true\n");
        } elsif ($Line =~ m%^appPlugin:ezsp-uart,true$% and !exists($UART_HOST_APPLICATIONS{"$Filename"})) {
          $Line = "appPlugin:ezsp-uart,false\n";
          DebugPrint("Overriding ISC ezsp-uart configuration to false\n");
        # We don't want the file-descriptor-dispatch, gateway, or unix-printf
        # plugins for simulation
        } elsif ($Line =~ m%^appPlugin:file-descriptor-dispatch,true$%) {
          $Line = "file-descriptor-dispatch,false\n";
        } elsif ($Line =~ m%^appPlugin:gateway,true$%) {
          $Line = "appPlugin:gateway,false\n";
        } elsif ($Line =~ m%^appPlugin:unix-printf,true$%) {
          $Line = "appPlugin:unix-printf,false\n";
        # Since we remove unix-printf (if present), we do need ember-minimal-
        # printf to make the apps compile
        } elsif ($Line =~ m%^appPlugin:ember-minimal-printf,false$%) {
          $Line = "appPlugin:ember-minimal-printf,true\n";
        # The slot manager plugin is not compilable for simulation
        } elsif ($Line =~ m%^appPlugin:slot-manager,true$%) {
          $Line = "appPlugin:slot-manager,false\n";
        # The mbedtls plugin is not needed for the host
        # TODO: This restriction may go away with moving RNG out of stack.
        #       Should be checked for EMZIGBEE-3407.
        } elsif (($PlatformTarget eq "simulation-ezsp") && ($Line =~ m%^appPlugin:mbedtls,true$%)){          
          $Line = "appPlugin:mbedtls,false\n";
        } elsif (($PlatformTarget eq "simulation-ezsp") && ($Line =~ m%^appPlugin:strong-random,true$%)){
                  $Line = "appPlugin:strong-random,false\n";
        # Like slot manager, slots are not usable for the OTA plugin in simulation
        } elsif ($Line =~ m%^AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_GECKO_BOOTLOADER_STORAGE_SUPPORT.*$%) {
          if (!($Line =~ m%^AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_GECKO_BOOTLOADER_STORAGE_SUPPORT,Do not use slots$%)) {
            $Line = "AppPluginOption:EMBER_AF_PLUGIN_OTA_STORAGE_SIMPLE_EEPROM_GECKO_BOOTLOADER_STORAGE_SUPPORT,Do not use slots\n";
          }
        } elsif ($Line =~ m%^useHwConfigurator=true$%) {
          # We shouldn't be using HW Configurator for simulation. This normally
          # gets turned off by AppBuilder when we change architecture to a non
          # HW Configurator supported architecture, but our simulation apps are
          # so janky that they don't get this AppBuilder logic run on them.
          $Line = "useHwConfigurator=false";
        } else {
          $Line =~ s%^\s+%%;
          $Line =~ s%\s+$%%;
          unless ($AlreadyPrinted) {
#            print "Considering SPECIAL_ISC_HANDLING of $Dir $PlatformTarget\n";
            $AlreadyPrinted = 1;
          }
          if (exists($SPECIAL_ISC_HANDLING{"$Dir"}{"$PlatformTarget"}{"Replacement"}{"$Line"})) {
            DebugPrint("Performing Special ISC handling of $Dir for $PlatformTarget\n");
            my $Replacement = $SPECIAL_ISC_HANDLING{"$Dir"}{"$PlatformTarget"}{"Replacement"}{"$Line"};
            $Line = $Replacement;
            print TEMP_ISC "# This line modified by the script $THIS\n";
          }
        }

        print TEMP_ISC "$Line\n";
      }

      close TEMP_ISC;
      close ORIGINAL_ISC;

      if ($TokenFile) {
        my $TokenFileBaseName = basename($TokenFile);
        WarnOrDie("Token file '$TokenFile' referenced by ISC file '$File', does not exist in sample app directory."
          . "  It also cannot live in 'gen' directory.\n")
            unless (-e "$Dir/$TokenFileBaseName");
      }

      WarnOrDie("Could not determine device name for $File") unless $DeviceName;

      DebugPrint("  Wrote $TempIsc\n");
      $ScenarioApps{$TempIsc}{Name} = $DeviceName;
      $ScenarioApps{$TempIsc}{GenDir} = $GenDir;
      $ScenarioApps{$TempIsc}{RealDir} = $Dir;
      $ScenarioApps{$TempIsc}{TokenFile} = $TokenFile if $TokenFile;
      $ScenarioApps{$TempIsc}{PlatformTarget} = $PlatformTarget;
      DebugPrint("  DeviceName: $DeviceName\n");
      DebugPrint("  GenDir:     $GenDir\n");
      DebugPrint("  RealDir:    $Dir\n");
      DebugPrint("  TokenFile:  $TokenFile\n") if $TokenFile;
    }
    @TempIscFiles = sort keys %ScenarioApps;
  }
  my @RealIscFiles = sort keys %RealIscFilesHash;
  print "Found " . scalar(@RealIscFiles) . " real isc files.\n";
  DebugPrint("  " . join("\n  ", @RealIscFiles) . "\n");

  return (\@TempIscFiles, \%ScenarioApps, \@RealIscFiles);
}

sub PostProcessGeneratedFiles
{
  my ($StackRelativeDir, $NoDelete, $ScenarioAppsRef) = @_;
  my %ScenarioApps = %$ScenarioAppsRef;

  my @AllMakefiles;

  print "Cleaning up generation data for Scenario apps.\n";
  foreach my $Key ( sort keys %ScenarioApps ) {
    # Remove '*.bak' files generated by AppBuilder.
    # Remove copied ISC file
    # Copy callbacks.c to parent directory if it doesn't exist.
    # Copy tokens file to gen directory

    # Limit finding the backup files to maxdepth 1 just in case we put
    # gen directory at a non-leaf location
    # (i.e. it has subdirectories with other files)
    my @BackupFiles = FindFiles($ScenarioApps{$Key}{GenDir},
                                "*.bak",
                                undef,  # extra options
                                "-maxdepth 1");
    if (scalar(@BackupFiles) > 0 && !$NoDelete) {
      for (my $i = 0; $i < scalar(@BackupFiles); $i++) {
        chomp $BackupFiles[$i];
      }
      DebugPrint("Removing backup files in $ScenarioApps{$Key}{GenDir}\n");
      unlink @BackupFiles || WarnOrDie("Could not delete backup files: $!");
    }

    unless ($NoDelete) {
      DebugPrint("Removing temp ISC file: $Key\n");
      unlink "$Key" or WarnOrDie("Could not delete ISC file '$Key': $!");
    }

    my $GenCallbacksFile = $ScenarioApps{$Key}{GenDir}
    . "/" . $ScenarioApps{$Key}{Name} . "_callbacks.c";
    my $RealCallbacksFile = $ScenarioApps{$Key}{RealDir}
    . "/" . $ScenarioApps{$Key}{Name} . "_callbacks.c";
    if (-e $RealCallbacksFile) {
      if (-e $GenCallbacksFile && !$NoDelete) {
        DebugPrint("Removing newly generated callbacks file in favor of existing one.\n");
        unlink $GenCallbacksFile
        or WarnOrDie("Could not remove callbacks file '$GenCallbacksFile': $!");
      }
    } else {
      move($GenCallbacksFile, $RealCallbacksFile);
    }

    if (exists($ScenarioApps{$Key}{TokenFile})) {
      my $RealTokenFile = $ScenarioApps{$Key}{RealDir} . "/" . $ScenarioApps{$Key}{TokenFile};
      my $GenTokenFile =  $ScenarioApps{$Key}{GenDir}  . "/" . $ScenarioApps{$Key}{TokenFile};
      copy($RealTokenFile, $GenTokenFile);
    }

    push @AllMakefiles,
    UpdateMakefile($StackRelativeDir,
                   $ScenarioApps{$Key}{Name},
                   $ScenarioApps{$Key}{GenDir},
                   $ScenarioApps{$Key}{RealDir},
                   $ScenarioApps{$Key}{PlatformTarget});
  }
  return @AllMakefiles;
}

sub DebugPrint
{
  my $Message = shift @_;
  if ($DEBUG) {
    print "$Message";
  }
}

sub WarnOrDie
{
  my ($Message, $WarningSuffix, $ErrorSuffix) = @_;
  $ErrorSuffix = "" unless $ErrorSuffix;
  $WarningSuffix = "" unless $WarningSuffix;
  my $CompleteMessage = ($KEEP_GOING ? "Warning: " : "Error: ")
    . $Message . " "
      . ($KEEP_GOING ? $WarningSuffix : $ErrorSuffix) . "\n";
  if ($KEEP_GOING) {
    warn $CompleteMessage;
  } else {
    die $CompleteMessage;
  }
}

sub FindFiles
{
  my ($Dir, $Name, $ExtraOptions, $PathOptions) = @_;
  $ExtraOptions = "" unless $ExtraOptions;
  $PathOptions = "" unless $PathOptions;
  my $Cmd = "find $Dir $PathOptions -type f -iname '$Name' $ExtraOptions";
  DebugPrint("Command: $Cmd\n");
  my @Output = `$Cmd`;
  my $Status = $?;
  if ($Status) {
    die "Error:  find command failed.\n";
  }
  return @Output;
}

sub FindIscFiles
{
  my @Directories = @_;

  push @Directories, "." unless (scalar(@Directories));
  my @FileList;

  foreach my $Dir (@Directories) {
    DebugPrint("Looking for ISC file(s) in '$Dir'\n");
    my @Output = FindFiles($Dir, '*.isc');

    foreach my $Item ( @Output ) {
      chomp $Item;
      if ($Item =~ m%$GENERATED_ROOT_DIR/%) {
        DebugPrint("Skipping ISC file in generated directory: $Item\n");
        next;
      }

      DebugPrint("Found ISC file: $Item\n");
      push @FileList, $Item;
    }
  }
  return @FileList;
}

sub RunCommand
{
  my ($Command) = @_;
  DebugPrint("Command: $Command\n");
  my @Output = `$Command`;
  my $Status = $?;

  DebugPrint("Output:  \n  " . join("  ", @Output) . "\n");

  return ($Status, @Output);
}

sub RunAppBuilder
{
  my $AppBuilderOutput = shift @_;
  my @Files = @_;
  my $Executable;
  my $Cygwin = 0;
  my $Prefix = '';

  if (exists($ENV{ZNET_LOCATION})) {
    $ZNET_LOCATION = $ENV{ZNET_LOCATION};
    print "Using ZNET_LOCATION from environment variable.\n";
  } else {
    print "No ZNET_LOCATION environment variable.\n";
  }
  print "ZNET_LOCATION=$ZNET_LOCATION\n";

  my $APPBUILDER_OPTIONS = "-application com.ember.app_x3.application -nosplash --launcher.suppressErrors -stack=$ZNET_LOCATION -noHwConf";

  die "Error: Passed in ISC file is empty.\n" unless (scalar (@Files));

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
  DebugPrint "Executable = $Executable, Cygwin = $Cygwin\n";

  my ($Status, @Output) = RunCommand("'$Executable' $APPBUILDER_OPTIONS "
                                     . ($REGENERATE_ISC ? " -regenerateIsc " : "" )
                                     . "-isc=$Prefix"
                                     . join(" -isc=$Prefix", @Files)
                                     . " -vmargs -Dequinox.scr.waitTimeOnBlock=100"
                                     . " 2>&1" );

  # AppBuilder does not usually return proper error codes.
  # So we need to check the output for a stack trace
  # indicating an exception was hit.
  if ($Status
      || (grep/(stack trace|error:)/i, @Output)) {
    WarnOrDie("AppBuilder encountered an error:\n  "
              . join("  ", @Output) . "\n",
              "Skipping",
              "");
    return 1;
  }

  if ($AppBuilderOutput) {
    print "AppBuilder Output:\n  ";
    print join("  ", @Output) . "\n";
  }

  return 0;
}

# Sample app path is something like 'zll-color-scene-remote' to 'ZllColorSceneRemote'
sub ConvertSampleAppPathToAppName
{
  my ($Dir) = @_;

  $Dir =~ s%^./%%;
  my @Temp = split/\-/, $Dir;
  my $Name = "";
  foreach my $Item ( @Temp ) {
    $Name .= ucfirst($Item);
  }
  return $Name;
}

sub UpdateTestHeaders
{
  my (@IscFiles) = @_;

  print "Update test headers\n";

  if (grep/^$ISC_FILE_FOR_TEST_HEADERS$/, @IscFiles) {
    print "Updating files in '$TEST_HEADERS_DIR' from '$ISC_FILE_FOR_TEST_HEADERS'\n";
    my $DestDir = $TEST_HEADERS_DIR;

    my $AppDir = dirname($ISC_FILE_FOR_TEST_HEADERS);
    my $AppName = basename($ISC_FILE_FOR_TEST_HEADERS);
    DebugPrint("App Name: $AppName\n");

    my @Output = FindFiles($AppDir, '*.[ch]', "! -name '$AppName*' ! -name 'custom*'");

    my $Files = "";
    for (my $i=0; $i < scalar(@Output); $i++) {
      chomp $Output[$i];
      $Files .= $Output[$i] . " ";
    }

    my $Status;
    ($Status, @Output) = RunCommand("cp $Files $DestDir");
    return 1 if $Status;
  }
  return 0;
}

sub UpdateMakefile
{
  my ($StackRelativeDir, $App, $MakefileDir, $AppDir, $PlatformTarget) = @_;

  my $TargetMakefile = "$MakefileDir/Makefile";
  my $TempFile = $TargetMakefile . ".tmp";
  die "Error: Makefile '$TargetMakefile' does not exist.\n"
    unless (-e $TargetMakefile);
  move($TargetMakefile, $TempFile);

  die "Error: Move failed ($TargetMakefile -> $TempFile).\n"
    unless (-e $TempFile);

  my $Status = open(INPUT, "$TempFile");
  die "Error: Could not open file '$TempFile' for reading.\n"
    unless ($Status);

  $Status = open(OUTPUT, ">$TargetMakefile");
  die "Error: Could not open file '$TargetMakefile' for writing.\n"
    unless ($Status);

  DebugPrint("Modifying makefile for app '$App'\n");
  DebugPrint("Looking to replace: "
             . join(", ", keys %MAKEFILE_VARIABLE_REPLACEMENT_HASH) . "\n");
  my $LineNumber = 0;
  my @SpecialEccLibraryLines;
  my $RealEccDef = 0;
  while (my $Line = <INPUT>) {
    $LineNumber++;

    if ($Line =~ /^\s*(\w+)\s*=/) {
      my $Variable = $1;
      DebugPrint("Considering variable: '$Variable'\n");
      if (grep /^$Variable$/, keys %MAKEFILE_VARIABLE_REPLACEMENT_HASH) {
        my $Value = $MAKEFILE_VARIABLE_REPLACEMENT_HASH{"$Variable"};

        my %ReplacementHash = (
          "MAKEFILE_DIR"    => $StackRelativeDir . "/" . $MakefileDir,
          "SAMPLE_APP_DIR"  => $StackRelativeDir . "/" . $AppDir,
          "APP_NAME"        => $App,
          "PLATFORM_TARGET" => $PlatformTarget,
        );
        foreach my $Key ( sort keys %ReplacementHash ){
          my $Replacement = $ReplacementHash{$Key};
          $Value =~ s/\{$Key\}/$Replacement/g;
        }
        DebugPrint("Found makefile variable to replace on line $LineNumber\n");
        print OUTPUT "# Makefile Variable replacement by $THIS\n";
        print OUTPUT "$Variable=$Value\n";
        next;
      }

    # Special handling (ie. ugly workaround) for EMZIGBEE-3510
    } elsif ($Line =~ /(.*-I)((?>\.\.\/){8,}.*)/ ) {
      DebugPrint("Found EMZIGBEE-3535 misery on line $LineNumber .\n");
      # print OUTPUT "# Applied ugly workaround to address EMZIGBEE-3510.\n";
      $Line = $1 . "./" . $StackRelativeDir . "/" . $MakefileDir . "/" . $2 . "\n";

    # Special handling of the ECC Library.  The path will often be absolute
    # and particular to a person's machine.  This is not helpful when
    # trying to allow all developers to build.  So rewrite the Makefile
    # to use a relative path instead.
    #
    # We also need to work around bug EMINSIGHT-2673 which defines an issue
    # where AppBuilder on Windows produces the makefile with backslashes in the path
    # to the ECC libraries.
    } elsif ($Line =~ m%.*[/\\](build[/\\]ecc-library-internal.*)$% ) {
      DebugPrint("Rewriting ECC library path.\n");
      $Line = "  " . $1 . "\n";
    } elsif ($Line =~ m%.*[/\\](build[/\\]ecc-library-283k1-internal.*)$% ) {
      DebugPrint("Rewriting ECC 283k1 library path.\n");
      $Line = "  " . $1 . "\n";
    }

    foreach my $Item ( keys %CBKE_AND_ECC_LIBRARY_MAPPING ) {
      if ($Line =~ m%\s*build[/\\]$Item%
          && $Line !~ m%stub% ) {
        chomp $Line;
        $Line =~ s%^\s+%%;
        $Line =~ s%\s*(\\\s*)+$%%;
        $Line =~ s%\\%/%g;
        push @SpecialEccLibraryLines, $Line;
        $Line = "  \\\n";
      }
    }

    if ( $Line =~ m%^ifdef\s+REAL_ECC\s*% ) {
      $RealEccDef = 1;
    } elsif ( $RealEccDef && $Line =~ m%\s*else\s*% ) {
        $RealEccDef = 0;
    }

    if ( $Line =~ m%^\s*CBKE_AND_ECC_LIBRARIES%
        && scalar(@SpecialEccLibraryLines) ) {

      print OUTPUT "  CBKE_AND_ECC_LIBRARIES= \\\n";
      if ($RealEccDef) {
        $Line = "    " . join(" \\\n    ", @SpecialEccLibraryLines) . "\n";
      } else {
        $Line = "";
        foreach my $Item ( @SpecialEccLibraryLines ) {
          # Sort the hashmap by key string length, then reverse it
          # This is so that we replace by cbke-library-dsa-verify-283k1 by
          # cbke-library-dsa-verify-283k1-stub and not
          # cbke-library-dsa-verify-stub instead, which searches on
          # cbke-library-dsa-verify
          foreach my $Key (reverse sort { length($a) <=> length($b) } keys %CBKE_AND_ECC_LIBRARY_MAPPING ) {
            if ($Item =~ m%\s*$Key\s*%) {
              my $Value = $CBKE_AND_ECC_LIBRARY_MAPPING{$Key};
              $Item =~ s/$Key/$Value/g;
              last;   # Don't want other matching prefixes to hit
            }
          }
          $Line .= "    $Item \\\n";
        }
        # Add an extra carriage return because we always append a line-contiunation
        # marker but don't have any more data.
        $Line .= "\n";
      }
    }

    if ($Line =~ "Makefile Variable replacement" ) {
      next;
    }

    print OUTPUT $Line;
  }
  close INPUT;
  close OUTPUT;
  unlink $TempFile;
  return $TargetMakefile;
}

sub GenerateSampleAppInfoFile
{
  my ($StackRelativeDir, @IscFiles) = @_;

  print "Generating info file for AppBuilder sample/scenario apps.\n";

  die "Error: Unknown stack relative dir '$StackRelativeDir'.\n"
    unless (exists($INFO_FILENAME{"$StackRelativeDir"}));

  my $Filename = $INFO_FILENAME{"$StackRelativeDir"};

  my $Status = open(OUTPUT, ">$Filename");
  die "Error: Could not open file '$Filename': $!\n"
    unless $Status;

  print OUTPUT<<END_OF_TEXT;
# Auto-generated file, Do not modify!
# Generated by $THIS

END_OF_TEXT
  foreach my $Item ( sort @IscFiles ) {
    my $AppEntryName = basename($Item);
    $AppEntryName =~ s/SoC//i;
    $AppEntryName =~ s/\.isc$//i;
    my $AppDirName = dirname($Item);
    my $AppPrettyName = $AppDirName;
    my $AppCallbacks=basename("$AppPrettyName") . "_callbacks.c";
    $AppPrettyName =~ s%.*/%%;

    # Replace "-" with "#" for easy regex matching
    $AppPrettyName =~ s/\-/#/g;
    if ( -e "$AppDirName/$EXCLUDE_FROM_INFO_FILE_MARKER") {
      print "Found '$AppDirName/$EXCLUDE_FROM_INFO_FILE_MARKER' file."
        . "  Not including app in '" . $INFO_FILENAME{"$StackRelativeDir"} . "' file.\n";
      next;
    }

    $AppPrettyName = "#$AppPrettyName";

    foreach my $Rule ( keys %PRETTY_NAME_RULES ) {
      my $BeforeName = $AppPrettyName;
      my $Replacement = $PRETTY_NAME_RULES{"$Rule"}{"Substitution"};
      if (exists($PRETTY_NAME_RULES{"$Rule"}{"Beginning of Line"})) {
        $AppPrettyName =~ s/^$Rule/$Replacement/;
      } elsif (exists($PRETTY_NAME_RULES{"$Rule"}{"End of Line"})) {
        $AppPrettyName =~ s/$Rule$/$Replacement/;
      } else {
        $AppPrettyName =~ s/$Rule/$Replacement/;
      }
      if (!($AppPrettyName eq $BeforeName)) {
        DebugPrint("$Rule | $BeforeName->$AppPrettyName\n");
      }
    }

    # Remove straggling "#"
    $AppPrettyName =~ s/#/ /g;
    # Trim whitespace
    $AppPrettyName =~ s/^\s+|\s+$//g;
    my @Words = split /\s+/, $AppPrettyName;
    $AppPrettyName = "";
    foreach my $Word ( @Words ) {
      $AppPrettyName .= ucfirst($Word) . " ";
    }
    # Chop off the final space that was just added in the foreach loop
    chop($AppPrettyName);

    DebugPrint("$AppDirName->$AppPrettyName\n");
    my $Description = "Error: Unknown description.  Edit the $THIS script.";
    if (exists($DESCRIPTIONS{"$AppDirName"})) {
      $Description = $DESCRIPTIONS{"$AppDirName"};
    }

    # Adding demos for certain applications
    my $Demo = "";
    if ($AppEntryName eq "Z3Light" || $AppEntryName eq "Z3Switch" ) {
      $Demo = "demo.1.bin=" . $AppDirName . "/efr32mg1p232f256gm48/" . $AppEntryName . ".s37\n  ";
      $Demo = $Demo . "demo.1.arch=efr32mg1p232f256gm48+brd4151a\n  ";
      $Demo = $Demo . "demo.2.bin=" . $AppDirName . "/efr32mg1p132f256gm48/" . $AppEntryName . ".s37\n  ";
      $Demo = $Demo . "demo.2.arch=efr32mg1p132f256gm48+brd4153a\n  ";
      $Demo = $Demo . "demo.3.bin=" . $AppDirName . "/efr32mg12p432f1024gl125-brd4161a/" . $AppEntryName . ".s37\n  ";
      $Demo = $Demo . "demo.3.arch=efr32mg12p432f1024gl125+brd4161a\n  ";
      $Demo = $Demo . "demo.4.bin=" . $AppDirName . "/efr32mg12p332f1024gl125-brd4162a/" . $AppEntryName . ".s37\n  ";
      $Demo = $Demo . "demo.4.arch=efr32mg12p332f1024gl125+brd4162a";
    }

    my $IncludeLine = $AppCallbacks;
    # If there's a hardware config directory present, add it to the include line
    if (-d "$AppDirName/.internal_hwconfig") {
      $IncludeLine = $IncludeLine . ", .internal_hwconfig"
    }

    # Build system cannot build and package these DMP demo binaries because they
    # have a dependency on Bluetooth which is not set up yet on build server
    #
    if ($AppEntryName eq "DynamicMultiprotocolLight" || $AppEntryName eq "DynamicMultiprotocolSwitch" || $AppEntryName eq "DynamicMultiprotocolLightSed") {
      $Demo = "demo.1.bin=" . $AppDirName . "/efr32mg12p432f1024gl125-brd4161a/" . $AppEntryName . ".s37\n  ";
      $Demo = $Demo . "demo.1.arch=efr32mg12p432f1024gl125+brd4161a\n  ";
      # Add additional architectures supported here
      $Demo = $Demo ."demo.2.bin=". $AppDirName . "/efr32mg12p332f1024gl125-brd4162a/" . $AppEntryName . ".s37\n  ";
      $Demo = $Demo . "demo.2.arch=efr32mg12p332f1024gl125+brd4162a";
      # Add additional architectures supported here
    }

    my $Architecture = "";
    if ($AppEntryName eq "DynamicMultiprotocolZigBeeZllLightBle"
        || $AppEntryName eq "DynamicMultiprotocolLightSed"
        || $AppEntryName eq "DynamicMultiprotocolLight"
        || $AppEntryName eq "DynamicMultiprotocolSwitch") {
      $Architecture = "iar";
    }

    my $printline ="";
    if ($Architecture eq "") {
      $printline = $Description;
    } else {
      $printline = "architecture=".$Architecture. "\n  " .$Description;
    }

    if( $Demo eq "") {
      print OUTPUT <<END_OF_TEXT;
$AppEntryName {
  name=$AppPrettyName
  dir=$AppDirName
  isc=$Item
  include=$IncludeLine
  $printline
}

END_OF_TEXT
      next;
    }
    print OUTPUT <<END_OF_TEXT;
$AppEntryName {
  name=$AppPrettyName
  dir=$AppDirName
  isc=$Item
  include=$IncludeLine
  $Demo
  $printline
}

END_OF_TEXT
  }
  close OUTPUT;

  return 0;
}
