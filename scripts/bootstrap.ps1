
###########################################################################################
#  function: Invoke-CmdScript                                                             #
#  Purpose: when a batch file is invoked within a powershell script, it is started in a   #
#           child process in cmd, any change to eniroment variables don't take effect     #
#           this function, parses the script and executes the commands in the scope of    #
#           this script                                                                   #
###########################################################################################

function Invoke-CmdScript {
    param(
        [String] $scriptName
    )
    $cmdLine = """$scriptName"" $args & set"
    & $Env:SystemRoot\system32\cmd.exe /c $cmdLine |
    Select-String '^([^=]*)=(.*)$' |
    ForEach-Object {
        $varName = $_.Matches[0].Groups[1].Value
        $varValue = $_.Matches[0].Groups[2].Value
        Set-Item Env:$varName $varValue
    }
}
###########################################################################################
#  function: check_for_git                                                                #
#  Purpose: checks for git and exits script if none exists                                #
###########################################################################################
function check_for_git() {


    try {
        git --version > $null
    }
    catch {
        { "Error: Git was not found. please make sure there is a valid git installation present'
        and added to the PATH"}
        exit
    }

    
}
###########################################################################################
#  function: pw_(color)                                                                   #
#  Purpose:  prints input in the color specified by the name of the function              #
###########################################################################################
function pw_red($inputText) {
    Write-Host  "$inputText" -ForegroundColor Red
}

function pw_yellow($inputText) {
    Write-Host  "$inputText" -ForegroundColor Yellow
}

function pw_cyan($inputText) {
    Write-Host  "$inputText" -ForegroundColor Cyan
}

function pw_magenta($inputText) {
    Write-Host  "$inputText" -ForegroundColor Magenta
}

function pw_white($inputText) {
    Write-Host  "$inputText" -ForegroundColor White -BackgroundColor Black
}

function pw_green($inputText) {
    Write-Host  "$inputText" -ForegroundColor Green
}

function pw_activate() {
    _pw_hello "  ACTIVATOR! This sets your shell environment variables.\n"
    $Global:_PW_ENV_SETUP_STATUS = 0
}
###########################################################################################
#  function: pw_finalize                                                                  #
#  Purpose:  activates the pigweed environment                                            #
###########################################################################################
function pw_finalize($arg1, $arg2) {

    $_PW_NAME = "$arg1"
    $_PW_SETUP_SH = "$arg2"
    
    if ("$Global:_PW_ENV_SETUP_STATUS" -ne 0 ) {
        return
    }

    if (Test-Path -Path $_PW_SETUP_SH -PathType Leaf) {

        . $_PW_SETUP_SH
        Invoke-CmdScript $_PW_SETUP_SH

        if ($?) {
            if ($_PW_NAME -eq "bootstrap" -and $env:PW_ENVSETUP_QUIET.length -eq 0) {
                Write-Output "To reactivate this environment in the future, run this in your "
                Write-Output "terminal:"
                Write-Output
                pw_green "  source ./activate.sh"
                Write-Output
                Write-Output "To deactivate this environment, run this:"
                Write-Output
                Write-Output "  deactivate"
                Write-Output
            }
        }
        else {
            pw_red "Error during $_PW_NAME--see messages above."
        }
    }
    else {
        pw_red "Error during $_PW_NAME--see messages above."
    }
}
function _pw_hello($arg_1) {

    if ($PW_BANNER_FUNC.Length -gt 0) {

        $_PW_BANNER_FUNC = "PW_BANNER_FUNC"
    }

    if ($PW_ENVSETUP_QUIET.Length -eq 0) {
        pw_green " WELCOME TO...`n" 
        pw_magenta $_PW_BANNER
        pw_green "$arg_1"
    }

}
function pw_eval_sourced($arg_1, $arg_2) {

    if ("$arg_1" -eq 0) {
        
        if ([string]::IsNullOrEmpty($arg_2)) {

            $GLOBAL:_PW_NAME = [System.IO.Path]::GetFileNameWithoutExtension($arg_2)   
        }
        else {
            $GLOBAL:_PW_NAME = [System.IO.Path]::GetFileNameWithoutExtension($Global:BOOTSTRAP_PATH)   
        }


        pw_red "  Error: Attempting to $_PW_NAME in a subshell
         Since $_PW_NAME.sh modifies your shell's environment variables,
         it must be sourced rather than executed. In particular, 
         'bash $_PW_NAME.sh' will not work since the modified 
         environment will get destroyed at the end of the script.
         Instead, source the script's contents in your shell:
         \$ source $_PW_NAME.sh"
    }
}

function pw_check_root($arg_1) {

    If ($arg_1 -match " ") {
        pw_red "  Error: The Pigweed path contains spaces
         The path '$arg_1' contains spaces. 
         Pigweed's Python environment currently requires Pigweed to be 
         at a path without spaces. Please checkout Pigweed in a 
         directory without spaces and retry running bootstrap."
    }
}  

function pw_get_env_root() {
    # PW_ENVIRONMENT_ROOT allows developers to specify where the environment
    # should be installed. bootstrap.sh scripts should not use that variable to
    # store the result of this function. This separation allows scripts to assume
    # PW_ENVIRONMENT_ROOT came from the developer and not from a previous
    # bootstrap possibly from another workspace.

    if ($Env:PW_ENVIRONMENT_ROOT.Length -eq 0) {
        

        if ($env:PW_PROJECT_ROOT.Length -gt 0) {
            
            "$env:PW_PROJECT_ROOT\.environment"

        }
        else {
            "$env:PW_ROOT\.environment"
        }
    }
    else {

        "$Env:PW_ENVIROMENT_ROOT"
    }
}

function pw_bootstrap() {
    Param(
        [parameter(Mandatory = $true)]
        [String]
        $shell_file,
        [parameter(Mandatory = $true)]
        [String]
        $install_dir,
        [parameter(Mandatory = $true)]
        [String]
        $config_file,
        [parameter(Mandatory = $true)]
        [String]
        $virtualenv_gn_out_dir
    )

    _pw_hello "  BOOTSTRAP! Bootstrap may take a few minutes; please be patient. "

    if ((Test-Path alias:python)) {

        pw_red "Error: 'python' is an alias
             The shell has a 'python' alias set. This causes many obscure
             Python-related issues both in and out of Pigweed. Please remove
             the Python alias from your shell init file or at least run the
             following command before bootstrapping Pigweed.

             Remove-Item Alias:python (powershell version 5.x)"

        exit
    }

     
    $local:pythonAlias = @("python", "python3", "python2")

    if ($PW_BOOTSTRAP_PYTHON.Length -gt 0) {

        $_PW_PYTHON = "$PW_BOOTSTRAP_PYTHON"

    }
    else {
        For ($i = 0; $i -lt $pythonAlias.Length; $i++) {

            if ((Get-Command $pythonAlias[$i]) 2> $null) {
                
                $_PW_PYTHON = $pythonAlias[$i]
                break
            }
            
        }
        if ([string]::IsNullOrEmpty($_PW_PYTHON)) {

            pw_red "Error: Python is not present on the system
       Pigweed's bootstrap process requires a local system Python.
       Please install Python on your system, add it to your PATH
       and re-try running bootstrap."
            return
        }

    }
    pip install mypy-protobuf > $null
    if ($Env:_PW_ENV_SETUP.Length -gt 0) {
        ${$_PW_PYTHON $Env:_PW_ENV_SETUP $shell_file $install_dir $config_file $virtualenv_gn_out_dir}
        $_PW_ENV_SETUP_STATUS = $LASTEXITCODE
    }
    else {
        & $_PW_PYTHON "$env:PW_ROOT/pw_env_setup/py/pw_env_setup/env_setup.py" --shell-file "$shell_file" `
            --install-dir "$install_dir" `
            --config-file "$Global:PROJECT_ROOT_DIR\$config_file" `
            --virtualenv-gn-out-dir "$virtualenv_gn_out_dir" 
            
        $_PW_ENV_SETUP_STATUS = $LASTEXITCODE 
    }


    # Create the environment README file. Use quotes to prevent alias expansion.
    Copy-Item "$env:PW_ROOT/pw_env_setup/destination.md" -Destination "$env:PW_ACTUAL_ENVIRONMENT_ROOT/README.md"
}
  

function _pw_banner() {

    #checks if enviroment variables PW_ENVSETUP_QUIET and PW_ENVSETUP_NO_BANNER are not set by user otherwise display banner
    if ($PW_ENVSETUP_QUIET.Length -eq 0 -and $PW_ENVSETUP_NO_BANNER -eq 0) {

        pw_magenta $_PW_BANNER
    }

}
function cleanup_env() {


    Remove-Item -Path Env:\PW_PROJECT_ROOT 
    Remove-Item -Path Env:\PW_ROOT 
    Remove-Item -Path Env:\PW_DOCTOR_SKIP_CIPD_CHECKS 

    
}
function bootstrap_or_activate($arguments) {

    if ($arguments.Count -eq 0) {

        $BOOTSTRAP_PATH = $MyInvocation.MyCommand.Definition 
        $BOOTSTRAP_NAME = (Split-Path $MyInvocation.MyCommand.Definition -Leaf)
    }
    else {
        $BOOTSTRAP_PATH = $arguments[0]
        $BOOTSTRAP_NAME = (Split-Path $BOOTSTRAP_PATH -Leaf)
    }
   
    $_CONFIG_FILE = "\scripts\environment.json"

    if (Test-Path env:PW_CONFIG_FILE) {

        $_CONFIG_FILE = $env:PW_CONFIG_FILE 
    }
    $directoryInfo = Get-ChildItem $Global:PROJECT_ROOT_DIR\third_party\pigweed\repo | Measure-Object

    check_for_git
    if ($BOOTSTRAP_NAME -eq "bootstrap.ps1" -Or $directoryInfo.count -eq 0) {
        try {
            git config --system core.longpaths true
            git submodule update --init
        }
        catch {
            throw "bootsrapping failed with error code $LASTEXITCODE"
        }
    }


    New-Item -Path Env:\PW_PROJECT_ROOT -Value "$Global:PROJECT_ROOT_DIR" -ErrorAction SilentlyContinue > $null
    New-Item -Path Env:\PW_ROOT -Value "$Global:PROJECT_ROOT_DIR\third_party\pigweed\repo" -ErrorAction SilentlyContinue > $null
    New-Item -Path Env:\PW_ACTIVATE_SKIP_CHECKS -Value '1' -ErrorAction SilentlyContinue > $null

    
    $PW_ACTUAL_ENVIRONMENT = pw_get_env_root
    $_SETUP_BAT = "$PW_ACTUAL_ENVIRONMENT\activate.bat"

 
    if (("$BOOTSTRAP_NAME" -eq "bootstrap.bat") -or (!(Test-Path -Path $_SETUP_BAT))) {

        pw_bootstrap `
            -shell_file "$_SETUP_BAT" `
            -install_dir "$PW_ACTUAL_ENVIRONMENT" `
            -config_file "$_CHIP_ROOT\$_CONFIG_FILE" `
            -virtualenv_gn_out_dir "$PW_ACTUAL_ENVIRONMENT\gn_out"

        pw_finalize bootstrap "$_SETUP_BAT"
    } 
    else {

        pw_activate
        pw_finalize activate "$_SETUP_BAT"
    }
}

try {
      
    bootstrap_or_activate $args
}
catch {

    Write-Output "Exception occured"
}
