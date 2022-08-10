
###########################################################################################
#  function: check_args                                                                   #
#  Purpose: validates arguments passed to the script                                      #
###########################################################################################
function check_args($commandLineArgs) {


    if ($commandLineArgs.Count -eq 0) {
        Write-Output "Build script for EFR32 Matter apps
Format:
./scripts/examples/gn_efr32_example.ps1 <AppRootFolder> <outputFolder> <efr32_board_name> [<Build options>]

<AppRootFolder>
    Root Location of the app e.g: examples/lighting-app/efr32/

<outputFolder>
    Desired location for the output files

<efr32_board_name>
    Identifier of the board for which this app is built
    Currently Supported :
        BRD4161A
        BRD4163A
        BRD4164A
        BRD4166A
        BRD4170A
        BRD4186A
        BRD4187A
        BRD4304A

<Build options> - optional noteworthy build options for EFR32
    chip_build_libshell
        Enable libshell support. (Default false)
    chip_logging
        Current value (Default true)
    chip_openthread_ftd
        Use openthread Full Thread Device, else, use Minimal Thread Device. (Default true)
    efr32_sdk_root
        Location for an alternate or modified efr32 SDK
    enable_heap_monitoring
        Monitor & log memory usage at runtime. (Default false)
    enable_openthread_cli
        Enables openthread cli without matter shell. (Default true)
    kvs_max_entries
        Set the maxium Kvs entries that can be store in NVM (Default 75)
        Thresholds: 30 <= kvs_max_entries <= 255
    show_qr_code
        Enables QR code on LCD for devices with an LCD
    setupDiscriminator
        Discriminatoor value used for BLE connexion. (Default 3840)
    setupPinCode
        PIN code for PASE session establishment. (Default 20202021)
    enable_sleepy_device
        Enable Sleepy end device. (Default false)
        Must also set chip_openthread_ftd=false
    use_rs911x
        Build wifi example with extension board rs911x. (Default false)
    use_wf200
        Build wifi example with extension board wf200. (Default false)
    'import("//with_pw_rpc.gni")'
        Use to build the example with pigweed RPC
    OTA_periodic_query_timeout
        Periodic query timeout variable for OTA in seconds
    Presets
    --sed
        enable sleepy end device, set thread mtd
        For minimum consumption, disable openthread cli and qr code
    --wifi <wf200 | rs911x>
        build wifi example variant for given exansion board
"
        exit
    }
    if ($commandLineArgs.Count -ilt 3) {
        
        Write-Output "ERROR: not enough command line arguments.
         ./scripts/examples/gn_efr32_example.ps1 <AppRootFolder> <outputFolder> <efr32_board_name> [<Build options>]"
        exit
    }
    try {

        if (Split-Path $commandLineArgs[0] -IsAbsolute) { 
            
            $Global:EXAMPLE_DIR = $commandLineArgs[0]
        }
        else {
            $Global:EXAMPLE_DIR = (Join-Path $PROJECT_ROOT_DIR $commandLineArgs[0])

        }
        if (!(Test-Path -Path $Global:EXAMPLE_DIR)) {     
            Write-Output "Path of Example doesnt exist!"     
            exit
        }
    }
    catch {
        Write-Output "Error: Path of Example to build is required" 
        exit
    }
    try {
        if (Split-Path $commandLineArgs[1] -IsAbsolute) { 
            
            $Global:OUT_DIR = $commandLineArgs[1]
        }
        else {
            $Global:OUT_DIR = (Join-Path $PROJECT_ROOT_DIR $commandLineArgs[1])

        }
    }
    catch {
        { "Error: Output folder path is required" }
        exit
    }  

    try {
        if (@("BRD4161A", "BRD4163A", "BRD4164A", "BRD4166A", "BRD4170A", "BRD4186A", "BRD4186A", "BRD4187A", "BRD4304A").Contains($commandLineArgs[2])) {

            $Global:BOARD = $commandLineArgs[2]
            
        }
        else {
            Write-Output "$($commandLineArgs[2]) is not a recognized board"
        }
    }
    catch {
        Write-Output "$($(commandLineArgs[2])) is not a recognized board"
        exit
    }
    if ($commandLineArgs.Count -gt 3) {

        $argsCount = $commandLineArgs.Length
        for ($i = 3; $i -lt $argsCount; $i++) {
            Switch ($commandLineArgs[$i]) {
                --wifi {
                    if ($commandLineArgs.Contains("rs911x") -Or $commandLineArgs.Contains("wf200")) {
                        if ($commandLineArgs.Contains("rs911x")) {
                            $Global:optArgs += "use_rs911x=true "
                        }
                        else {
                            $Global:optArgs += "use_wf200=true "
                        }
                        $Global:USE_WIFI = $true
                    }
                    else {
                        Write-host "--wifi requires rs911x or wf200"
                        exit 1
                    }
   
                }
                --sed {
                    $Global:optArgs += "enable_sleepy_device=true chip_openthread_ftd=false "
                }
                --chip_enable_wifi_ipv4 {
                    $Global:optArgs += "chip_enable_wifi_ipv4=true "
                }
                default {
                    if ("$($commandLineArgs[$i])" -eq "use_rs911x=true" -or $commandLineArgs[$i] -eq "use_wf200=true" ) {
                        $Global:USE_WIFI = $true
                    }
                    if ($commandLineArgs[$i] -eq "rs911x" -or $commandLineArgs[$i] -eq "wf200" ) {
                        continue
                    }
                    $Global:optArgs += "$($commandLineArgs[$i]) "
                }
           
            }

        }
    }
}


###########################################################################################
#  function: generate_binaries                                                            #
#  Purpose:  creates ninja.build file using gn and generates the binaries using ninja     #
###########################################################################################
function generate_binaries() {

    if ($Global:USE_WIFI) {
        
        gn gen --check --fail-on-unused-args --export-compile-commands --root="$Global:EXAMPLE_DIR" --dotfile="$Global:EXAMPLE_DIR\build_for_wifi_gnfile.gn" --args="efr32_board=\`"$Global:BOARD\`" $Global:optArgs" "$Global:OUT_DIR\$Global:BOARD"
    }
    elseif ($Global:optArgs.Length -eq 0) {

        gn gen --check --fail-on-unused-args --export-compile-commands --root="$Global:EXAMPLE_DIR" --args="efr32_board=\`"$Global:BOARD\`"" "$Global:OUT_DIR\$Global:BOARD"
    }
    else {
        gn gen --check --fail-on-unused-args --export-compile-commands --root="$Global:EXAMPLE_DIR" --args="efr32_board=\`"$Global:BOARD\`" $Global:optArgs" "$Global:OUT_DIR\$Global:BOARD"
    }
      
    ninja -v -C "$Global:OUT_DIR\$Global:BOARD"
    arm-none-eabi-size -A $Global:OUT_DIR\$Global:BOARD\*.out
}

try {

    $Global:EXAMPLE_DIR = ""
    $Global:PROJECT_ROOT_DIR = (Split-Path -Path (Split-Path -Path $PSScriptRoot))
    $Global:OUT_DIR = ""
    $Global:BOARD = ""
    $Global:optArgs = ""
    $Global:USE_WIFI = $false

    check_args $args
    . "$Global:PROJECT_ROOT_DIR\scripts\bootstrap.ps1"
    generate_binaries

}
catch {
    "Exception occured $?"
}
