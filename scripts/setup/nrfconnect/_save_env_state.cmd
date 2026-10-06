@echo off
setlocal EnableDelayedExpansion

set "_ENV_STATE_FILE=%~1"
if "%_ENV_STATE_FILE%"=="" exit /b 1

for %%I in ("%_ENV_STATE_FILE%") do if not exist "%%~dpI" mkdir "%%~dpI"

(
echo @echo off
if defined PATH (echo set "PATH=!PATH!"^) else (echo set "PATH="^)
if defined GIT_EXEC_PATH (echo set "GIT_EXEC_PATH=!GIT_EXEC_PATH!"^) else (echo set "GIT_EXEC_PATH="^)
if defined GIT_TEMPLATE_DIR (echo set "GIT_TEMPLATE_DIR=!GIT_TEMPLATE_DIR!"^) else (echo set "GIT_TEMPLATE_DIR="^)
if defined PERL5LIB (echo set "PERL5LIB=!PERL5LIB!"^) else (echo set "PERL5LIB="^)
if defined PERLLIB (echo set "PERLLIB=!PERLLIB!"^) else (echo set "PERLLIB="^)
if defined TCL_LIBRARY (echo set "TCL_LIBRARY=!TCL_LIBRARY!"^) else (echo set "TCL_LIBRARY="^)
if defined TK_LIBRARY (echo set "TK_LIBRARY=!TK_LIBRARY!"^) else (echo set "TK_LIBRARY="^)
if defined NRFUTIL_HOME (echo set "NRFUTIL_HOME=!NRFUTIL_HOME!"^) else (echo set "NRFUTIL_HOME="^)
if defined ZEPHYR_TOOLCHAIN_VARIANT (echo set "ZEPHYR_TOOLCHAIN_VARIANT=!ZEPHYR_TOOLCHAIN_VARIANT!"^) else (echo set "ZEPHYR_TOOLCHAIN_VARIANT="^)
if defined ZEPHYR_SDK_INSTALL_DIR (echo set "ZEPHYR_SDK_INSTALL_DIR=!ZEPHYR_SDK_INSTALL_DIR!"^) else (echo set "ZEPHYR_SDK_INSTALL_DIR="^)
if defined ZEPHYR_BASE (echo set "ZEPHYR_BASE=!ZEPHYR_BASE!"^) else (echo set "ZEPHYR_BASE="^)
if defined PYTHONHOME (echo set "PYTHONHOME=!PYTHONHOME!"^) else (echo set "PYTHONHOME="^)
if defined PYTHONPATH (echo set "PYTHONPATH=!PYTHONPATH!"^) else (echo set "PYTHONPATH="^)
) > "%_ENV_STATE_FILE%"

endlocal
