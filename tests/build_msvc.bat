@echo off

REM usage: build.bat [config]
REM config - { debug, release, preprocessor }, default=debug

REM 1) Setup configuration
set config=%1
if "%config%" == "" set config="debug"
set rootFolder=%cd%
set source=%cd%/main.c
set libs=ws2_32.lib
set outputName=%cd%/output/main_msvc.exe

REM /nologo                 - suppress startup banner
REM /Za                     - disable language extensions not compatible with C89/C90
REM /Tc                     - enforces compilation with C
REM /Od                     - disable optimisations
REM /O2                     - full optimisations
REM /Zi                     - generate seprate pdb file
REM /W4                     - warning level 4 
REM /WX                     - threat warnings as errors 
REM /OPT:REF                - remove functions which are never referenced
REM /SUBSYSTEM:CONSOLE      - executable is console application
REM /P                      - output the preprocessor result to a file 
set debugOpts=/nologo /Od /W4 /WX /Zi /Za /Tc
set releaseOpts=/nologo /O2 /W4 /WX /Za /Tc 
set linkOpts=/OPT:REF /SUBSYSTEM:CONSOLE

set opts=%debugOpts%
if %config% == "preprocessor" set opts=/P %debugOpts%
if %config% == "release" set opts=%releaseOpts%

REM 2) Create output directories
if not exist output mkdir output
if not exist build mkdir build
cd build

REM 3) Setup msvc cl

REM First check if cl has already been setup
where /q cl
IF %ERRORLEVEL% == 0 goto compileSetup
set VC_OPTS=amd64
set "LIB="

REM Microsoft stupidity. Search system for cl.
set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Professional
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2019\Community
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Professional
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\Auxiliary\Build\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 14.0
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 13.0
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 12.0
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 11.0
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\vcvarsall.bat" %VC_OPTS%))

set VC_PATH=C:\Program Files (x86)\Microsoft Visual Studio 10.0
if not defined LIB (if exist "%VC_PATH%" (call "%VC_PATH%\VC\vcvarsall.bat" %VC_OPTS%))

:compileSetup

REM 4) Compile executable
call cl %opts% %source% /I"%rootFolder%" /link %linkOpts% %libs% /out:%outputName%

cd ..