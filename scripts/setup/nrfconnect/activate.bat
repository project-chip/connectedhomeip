:<<"::WINDOWS_ONLY"
@echo off
:: Copyright (c) 2026 Project CHIP Authors
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at
::
::     http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS,
:: WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
:: See the License for the specific language governing permissions and
:: limitations under the License.
::WINDOWS_ONLY
:; echo "ERROR: Attempting to run Windows activate.bat from a Unix/POSIX shell!"
:; echo "Instead, run the following command."
:; echo ""
:; echo "    source scripts/setup/nrfconnect/activate.sh"
:; echo ""
:<<"::WINDOWS_ONLY"

set "_SCRIPT_DIR=%~dp0"
if "%_SCRIPT_DIR:~-1%"=="\" set "_SCRIPT_DIR=%_SCRIPT_DIR:~0,-1%"

pushd "%_SCRIPT_DIR%\..\..\.." >nul || exit /b 1
set "_CHIP_ROOT=%CD%"
popd

set "_ENV_STATE_FILE=%_CHIP_ROOT%\.environment\nrfconnect\env_state.cmd"
set "_DEACTIVATE_BAT=%_SCRIPT_DIR%\deactivate.bat"

if exist "%_ENV_STATE_FILE%" (
    if not defined NRFCONNECT_SKIP_ENV_STATE (
        echo nRF Connect environment appears to be active. 1>&2
        echo Deactivate it first with: 1>&2
        echo   call "%_DEACTIVATE_BAT%" 1>&2
        exit /b 1
    )
)

if not defined NRFCONNECT_SKIP_ENV_STATE call "%_SCRIPT_DIR%\_save_env_state.cmd" "%_ENV_STATE_FILE%"

if defined ZEPHYR_BASE (
    if exist "%ZEPHYR_BASE%" goto :load_zephyrrc
)

set "_ZEPHYRRC="
for /d %%D in ("%_CHIP_ROOT%\.environment\nrfconnect\sdk\*") do (
    if exist "%%D\.zephyrrc.cmd" (
        set "ZEPHYR_BASE=%%D\zephyr"
        set "_ZEPHYRRC=%%D\.zephyrrc.cmd"
        goto :load_zephyrrc
    )
)

echo ZEPHYR_BASE is not set and .zephyrrc.cmd was not found under .environment\nrfconnect\sdk. 1>&2
echo Run python scripts/setup/nrfconnect/setup.py first. 1>&2
exit /b 1

:load_zephyrrc
if not exist "%_ZEPHYRRC%" (
    echo Missing %_ZEPHYRRC%. Run python scripts/setup/nrfconnect/setup.py. 1>&2
    exit /b 1
)

call "%_ZEPHYRRC%"

where bash >nul 2>&1
if errorlevel 1 (
    echo Git Bash is required to activate the Matter build environment on Windows. 1>&2
    echo Install Git for Windows and ensure bash is on PATH, then re-run activation. 1>&2
    exit /b 1
)

set "_MATTER_ENV=%TEMP%\nrfconnect_matter_env_%RANDOM%.cmd"
python "%_SCRIPT_DIR%\export_matter_env.py" "%_CHIP_ROOT%" "%_MATTER_ENV%"
if errorlevel 1 (
    if exist "%_MATTER_ENV%" del /f /q "%_MATTER_ENV%"
    exit /b 1
)
call "%_MATTER_ENV%"
del /f /q "%_MATTER_ENV%"

if defined VIRTUAL_ENV (
    set "PYTHONHOME="
    if exist "%VIRTUAL_ENV%\Lib\site-packages" (
        set "PYTHONPATH=%VIRTUAL_ENV%\Lib\site-packages"
    ) else (
        for /f "delims=" %%V in ('python -c "import sys; print(f'{sys.version_info.major}.{sys.version_info.minor}')"') do set "PYTHONPATH=%VIRTUAL_ENV%\lib\python%%V\site-packages"
    )
) else (
    set "PYTHONHOME="
    set "PYTHONPATH="
)

if not defined NRFCONNECT_SKIP_ENV_STATE (
    echo.
    echo To deactivate, run:
    echo   call "%_DEACTIVATE_BAT%"
)
::WINDOWS_ONLY
