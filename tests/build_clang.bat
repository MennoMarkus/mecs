
@echo off

REM usage: build.bat [config] [languageVersion]
REM config - { debug, release, preprocessor }, default=debug
REM languageVersion - { c, cpp }, default=c

REM 1) Setup configuration
set config=%1
set languageVersion=%2
if "%config%" == "" set config="debug"
if "%languageVersion%" == "" set languageVersion="c"
set rootFolder=%cd%
set source=%cd%/main.c
set outputName=%cd%/output/main_clang.exe

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
set debugOpts=-Wall -Werror -pedantic-errors -O3 -g -gcodeview -I"%rootFolder%" -o%outputName%
set releaseOpts=-Wall -Werror -pedantic-errors -Ofast -I"%rootFolder%" -o%outputName%

set opts=%debugOpts%
if %config% == "preprocessor" set opts=%debugOpts% -E
if %config% == "release" set opts=%releaseOpts%

REM 2) Set the language version
REM -std=c89                - set C version to C89
REM -std=c++98              - set C++ version to C++98/C++03
REM -x                      - compile C files as C++
if %languageVersion% == "c" set opts=-std=c89 %opts%
if %languageVersion% == "cpp" set opts=-std=c++98 -x c++ %opts%

REM 3) Create output directory
if not exist output mkdir output

REM 4) Compile executable
call clang %opts% %source% 