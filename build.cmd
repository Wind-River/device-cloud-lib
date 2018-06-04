:: Copyright (C) 2018 Wind River Systems, Inc. All Rights Reserved.
::
:: Licensed under the Apache License, Version 2.0 (the "License");
:: you may not use this file except in compliance with the License.
:: You may obtain a copy of the License at:
::
::    http://www.apache.org/licenses/LICENSE-2.0
::
:: Unless required by applicable law or agreed to in writing, software
:: distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
:: WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
::
:: --------------------------------------------------------------------
:: Description:
:: This script downloads, builds and installs project dependencies for the
:: device-cloud-lib into sub-directory called "deps". For more information see
:: README.md.
::
:: Usage:
::  $ ./build.cmd --help
:: --------------------------------------------------------------------

setlocal EnableExtensions
setlocal DisableDelayedExpansion

set SCRIPT_PATH=%~dp0
set PYTHON_SCRIPT=build.py

set TARGET_ARCH=x86

:: determine location of visual studio development tools
where /q devenv 2>nul
if %errorlevel% equ 0 goto check_compiler

call "%VS150COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET_ARCH% 2>nul
if %errorlevel% equ 0 goto check_compiler

call "%VS140COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET_ARCH% 2>nul
if %errorlevel% equ 0 goto check_compiler

call "%VS120COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET_ARCH% 2>nul
if %errorlevel% equ 0 goto check_compiler

call "%VS110COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET_ARCH% 2>nul
if %errorlevel% equ 0 goto check_compiler

call "%VS100COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET_ARCH% 2>nul
if %errorlevel% equ 0 goto check_compiler

call "%VS90COMNTOOLS%\..\..\VC\vcvarsall.bat" %TARGET_ARCH% 2>nul
if %errorlevel% equ 0 goto check_compiler

echo Failed to find Visual Studio development tools
goto eof

:check_compiler
where /q cl 2>nul
if %errorlevel% equ 0 goto tools_ok
echo Microsoft Visual Studio C/C++ compiler not found
goto eof

:tools_ok
echo Building with Visual Studio %VisualStudioVersion%

for /f %%i in ('where python') do set PYTHON_PATH=%%i
"%PYTHON_PATH%" "%SCRIPT_PATH%%PYTHON_SCRIPT%" %* || echo "Failed to build project"
exit /B %errorlevel%

:: called on failure
:eof
exit /B 1

