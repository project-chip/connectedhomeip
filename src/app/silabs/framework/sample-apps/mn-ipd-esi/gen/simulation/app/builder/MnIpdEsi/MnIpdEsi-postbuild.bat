@echo off
setlocal enableextensions enabledelayedexpansion
SET allParams=%*
SET params=%allParams:' '=","%
SET params=%params:'="%
set /a count=0
for %%i in (%params%) do (
  set /a count+=1
  set param[!count!]=%%i
)

set "studioInstallationDir=%param[4]:"=%"
set "projDir=%~dp0"
set "projDir=%projDir:"=%"

set "projDir=%projDir:\=/%"
set "projDir=%projDir:Z:=%"
start /unix "/Library/Java/JavaVirtualMachines/jdk1.8.0_91.jdk/Contents/Home/jre/bin/java" -jar "%studioInstallationDir%/plugins/com.silabs.external.jython_2.7.0.qualifier/external_jython/2.7.0/jython-2.7.0.jar"  "%projDir%/MnIpdEsi-postbuild.py" %param[1]%  %param[2]%  "wine start /unix " "wine cmd /C" %param[3]% 