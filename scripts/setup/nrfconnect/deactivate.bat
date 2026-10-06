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
:; echo "ERROR: Attempting to run Windows deactivate.bat from a Unix/POSIX shell!"
:; echo "Instead, run the following command."
:; echo ""
:; echo "    source scripts/setup/nrfconnect/deactivate.sh"
:; echo ""
:<<"::WINDOWS_ONLY"

set "_SCRIPT_DIR=%~dp0"
if "%_SCRIPT_DIR:~-1%"=="\" set "_SCRIPT_DIR=%_SCRIPT_DIR:~0,-1%"

pushd "%_SCRIPT_DIR%\..\..\.." >nul || exit /b 1
set "_CHIP_ROOT=%CD%"
popd

set "_ENV_STATE_FILE=%_CHIP_ROOT%\.environment\nrfconnect\env_state.cmd"
set "_SDK_DIR=%_CHIP_ROOT%\.environment\nrfconnect\sdk"

if not exist "%_ENV_STATE_FILE%" (
    python -c "import sys; from pathlib import Path; root=Path(sys.argv[1]); sys.exit(0 if any(root.glob('toolchains/*/nrfutil/home/locked')) else 1)" "%_SDK_DIR%"
    if errorlevel 1 (
        echo No saved nRF Connect environment state found. 1>&2
        exit /b 1
    )
    python -c "import sys; from pathlib import Path; [p.unlink(missing_ok=True) for p in Path(sys.argv[1]).glob('toolchains/*/nrfutil/home/locked')]" "%_SDK_DIR%"
    echo Removed stale nRF Connect environment lock.
    exit /b 0
)

call "%_ENV_STATE_FILE%"
del /f /q "%_ENV_STATE_FILE%"
python -c "import sys; from pathlib import Path; [p.unlink(missing_ok=True) for p in Path(sys.argv[1]).glob('toolchains/*/nrfutil/home/locked')]" "%_SDK_DIR%"
echo nRF Connect environment deactivated.
::WINDOWS_ONLY
