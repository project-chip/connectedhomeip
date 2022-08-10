###########################################################################################
#  function: generate_pc_file                                                             #
#  Purpose: creates a "package config (.pc)" file for openssl                             #
###########################################################################################

function generate_pc_file($prefix) {

    New-Item "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config\openssl.pc" -ErrorAction SilentlyContinue > $null

    Set-Content "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config\openssl.pc" "prefix= $prefix"
    Add-Content "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config\openssl.pc" 'exec_prefix=${prefix}
libdir=${exec_prefix}/lib
includedir=${prefix}/include

Name: OpenSSL
Description: Secure Sockets Layer and cryptography
Version: 3.0.5
Requires:
Libs: -L${libdir} -lssl -lcrypto
Libs.private: -ldl -lz
Cflags: -I${includedir} '

}
###########################################################################################
#  function: clean_up_env                                                                 #
#  Purpose: removes environment variables and directories created by this script          #
###########################################################################################
function clean_up_env() {

    if (Test-Path env:PKG_CONFIG_PATH) { 

        Remove-Item Env:\PKG_CONFIG_PATH  
    } 
    if (Test-Path "$Global:PROJECT_ROOT_DIR\dependencies") {

        Remove-Item "$Global:PROJECT_ROOT_DIR\dependencies" -Recurse
    }
    
}
function install_dependencies() {

    $pkgConfigUrl = "http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/pkg-config_0.26-1_win32.zip"
    $pkgConfigDownloadFolder = "$Global:PROJECT_ROOT_DIR\dependencies\pkg-config_0.26-1_win32.zip"
    $binFolderPath = "$Global:PROJECT_ROOT_DIR\dependencies"

    $getTextRuntimeUrl = "http://ftp.gnome.org/pub/gnome/binaries/win32/dependencies/gettext-runtime_0.18.1.1-2_win32.zip"
    $getTextRuntimeDownloadFolder = "$Global:PROJECT_ROOT_DIR\dependencies\gettext-runtime_0.18.1.1-2_win32.zip"    
    
    $glibUrl = "http://ftp.acc.umu.se/pub/gnome/binaries/win32/glib/2.28/glib_2.28.8-1_win32.zip"
    $glibDownloadFolder = "$Global:PROJECT_ROOT_DIR\dependencies\glib_2.28.8-1_win32.zip"

    $ninjaDownloadUrl = "https://github.com/ninja-build/ninja/releases/download/v1.11.0/ninja-win.zip"
    $ninjaDownloadFolder = "$Global:PROJECT_ROOT_DIR\dependencies\ninja-win.zip"

    $gnDownloadUrl = "https://chrome-infra-packages.appspot.com/dl/gn/gn/windows-amd64/+/latest"
    $gnDownloadFolder = "$Global:PROJECT_ROOT_DIR\dependencies\gn-windows-amd64.zip"

    if (!(Test-Path -Path "$binFolderPath\bin\pkg-config.exe" -PathType Leaf)) {

        try {
            (New-Object Net.WebClient).DownloadFile($pkgConfigUrl, $pkgConfigDownloadFolder)
        }
        catch {
            { "Net.WebClient.DownloadFile Object method exited with error code $LASTEXITCODE" }
        }

        try {
            Expand-Archive -LiteralPath $pkgConfigDownloadFolder -DestinationPath $binFolderPath -Force | Out-Null
        }
        catch {
            { "error code $LASTEXITCODE" }
            
        }

    }
    if (!(Test-Path -Path "$binFolderPath\bin\libglib-2.0-0.dll" -PathType Leaf)) {

        
        try {
            (New-Object Net.WebClient).DownloadFile($glibUrl, $glibDownloadFolder)
        }
        catch {
            { "Net.WebClient.DownloadFile Object method exited with error code $LASTEXITCODE" }
        }
        
        try {
            Expand-Archive -LiteralPath $glibDownloadFolder -DestinationPath $binFolderPath -Force | Out-Null
        }
        catch {
            { "error code $LASTEXITCODE" }
        }
        

    }
    if (!(Test-Path -Path "$binFolderPath\bin\intl.dll" -PathType Leaf)) {

        try {
            (New-Object Net.WebClient).DownloadFile($getTextRuntimeUrl, $getTextRuntimeDownloadFolder)
        }
        catch {
            { "Net.WebClient.DownloadFile Object method exited with error code $LASTEXITCODE" }
        }

        try {
            Expand-Archive -LiteralPath $getTextRuntimeDownloadFolder -DestinationPath $binFolderPath -Force | Out-Null
        }
        catch {
            { "error code $LASTEXITCODE" }
        }


    } 
    if (!(Test-Path -Path "$binFolderPath\ninja.exe" -PathType Leaf)) {

        try {
            (New-Object Net.WebClient).DownloadFile($ninjaDownloadUrl, $ninjaDownloadFolder)
        }
        catch {
            { "Net.WebClient.DownloadFile Object method exited with error code $LASTEXITCODE" }
        }

        try {
            Expand-Archive -LiteralPath $ninjaDownloadFolder -DestinationPath $binFolderPath -Force | Out-Null
        }
        catch {
            { "error code $LASTEXITCODE" }
        }
    }
    if (!(Test-Path -Path "$binFolderPath\gn.exe" -PathType Leaf)) {

        try {
            (New-Object Net.WebClient).DownloadFile($gnDownloadUrl, $gnDownloadFolder)
        }
        catch {
            { "Net.WebClient.DownloadFile Object method exited with error code $LASTEXITCODE" }
        }

        try {
            Expand-Archive -LiteralPath $gnDownloadFolder -DestinationPath $binFolderPath -Force | Out-Null
        }
        catch {
            { "error code $LASTEXITCODE" }
        }
    }        
    $a = ""
    $b = ""

    foreach ($loc in ($env:Path -split ";")) {

        if ($loc -eq "$Global:PROJECT_ROOT_DIR\dependencies\bin" -and $a -ne "true") {
            $a = "true"
            $env:Path += "$binFolderPath\bin;" 
        }
        if ($loc -eq "$Global:PROJECT_ROOT_DIR\dependencies" -and $b -ne "true") {
            $b = "true"
            $env:Path += "$binFolderPath;" 
        }  
       
    }
    if ($a -ne "true") {
        $env:Path += "$binFolderPath\bin;"  
    }
    if ($b -ne "true") {
        $env:Path += "$binFolderPath;"  
    }

}

###########################################################################################
#  function: install_openssl                                                              #
#  Purpose: installs openssl package                                                      #
###########################################################################################

function install_openssl() {

    
    if ($env:PROCESSOR_ARCHITECTURE -eq "AMD64" -or $env:PROCESSOR_ARCHITEW6432 -eq "AMD64") {
        $bitness = "64"
        $opensslInstallPath = "C:\Program Files\OpenSSL-Win${bitness}"
    }
    else {
        $bitness = "32"
        $opensslInstallPath = "C:\Program Files\OpenSSL-Win${bitness}"
    }

    $openssl_install_url = "https://slproweb.com/download/Win${bitness}OpenSSL-3_0_5.exe"
    $opensslDownloadFolder = "$Global:PROJECT_ROOT_DIR\dependencies\Win${bitness}OpenSSL-3_0_5.exe"
    
    if (Test-Path -Path $opensslInstallPath) {

        if (!(Test-Path -Path "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config\openssl.pc")) {

            generate_pc_file $opensslInstallPath
        }
        $env:PKG_CONFIG_PATH += "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config;" 
        return
    }
    try {
        (New-Object Net.WebClient).DownloadFile($openssl_install_url, $opensslDownloadFolder)
    }
    catch {
        { "Net.WebClient.DownloadFile Object method exited with error code $LASTEXITCODE" }
    }
    try {
        & $opensslDownloadFolder /passive InstallAllUsers=0 PrependPath=1 Include_test=0 TargetDir=$opensslInstallPath | Out-Null
    }
    catch {
        { "Openssl installer exited with error code $LASTEXITCODE" }
    }

    generate_pc_file $opensslInstallPath
    $env:PKG_CONFIG_PATH += "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config;" 

}

function setup() {

    New-Item -Path "$Global:PROJECT_ROOT_DIR\dependencies" -ItemType Directory -ErrorAction SilentlyContinue > $null
    New-Item -Path "$Global:PROJECT_ROOT_DIR\dependencies\pkg_config" -ItemType Directory -ErrorAction SilentlyContinue > $null
    New-Item -Path Env:\PKG_CONFIG_PATH -Value "$PKG_CONFIG_PATH" -ErrorAction SilentlyContinue > $null
}


try {
    if ($($args[0]) -eq "--setup") {
        setup
        install_dependencies
        install_openssl
        
    }
    elseif ($($args[0]) -eq "--cleanup") {
        clean_up_env
    }
    else {
        Write-Host "Format: ./script/windows_setup.ps1 <--setup or --cleanup>"
    }
}
catch {
    { "Exception occured " }
}