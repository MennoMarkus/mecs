
@echo off

REM usage: build.bat [config]
REM config - { debug, release, preprocessor }, default=debug

REM 1) Setup configuration
set config=%1
if "%config%" == "" set config="debug"
set rootFolder=%cd%
set source=%cd%/main.c
set outputName=%cd%/output/main_clang.exe

REM -std=c89                - set C version to C89
REM -Wall                   - enable all warnings
REM -Werror                 - turn warnings into errors
REM -pedantic-errors        - error on language extensions 
REM -O3                     - enable minimal optimizations
REM -Ofast                  - enable all optimizations for speed 
REM -g                      - generate debug information
REM -gcodeview              - generate code view debug information for visual studio
REM -I                      - specify include directory
REM -o                      - specify output executable name
REM -E                      - ouput the result form the preprocessor to a file
set debugOpts=-std=c89 -Wall -Werror -pedantic-errors -O3 -g -gcodeview -I"%rootFolder%" -o%outputName%
set releaseOpts=-std=c89 -Wall -Werror -pedantic-errors -Ofast -I"%rootFolder%" -o%outputName%

set opts=%debugOpts%
if %config% == "preprocessor" set opts=%debugOpts% -E
if %config% == "release" set opts=%releaseOpts%

REM 2) Create output directory
if not exist output mkdir output

REM 3) Compile executable
call clang %opts% %source% 