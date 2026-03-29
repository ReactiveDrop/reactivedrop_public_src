@echo off
setlocal

rem ======================================
rem === PATH CONFIGURATIONS

rem == Setup path to nmake.exe, from vc common tools directory
call "G:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"

rem == Set the absolute path to your mod's game directory here
set GAMEDIR="G:\PENDRIVE\Steam\steamapps\common\Alien Swarm Reactive Drop\reactivedrop"

rem == Set the Path to your mods root source code
rem == "..\.." should already be correct, accepts relative paths only!
set SOURCEDIR=..\..

rem === PATH CONFIGURATIONS END
rem ======================================
rem ======================================
rem === SHADER BUILD CONFIGURATION

rem == Set force=1 to force shader (re)compilation 
REM set FORCE=1

rem == Set dynamic_shaders=1 to build .inc files only
REM set dynamic_shaders=1

rem == Number of threads used by ShaderCompile:
rem ==  N <= 0 or unset = all threads (default)
rem ==  N > 0           = force specific number of threads
rem == Set to override the default behavior.
REM set THREADS=

rem Optimization level used by ShaderCompile:
rem   0          = no optimization
rem   1–2        = intermediate optimization
rem   3 or unset = full optimization (default)
rem Set to override the default behavior.
REM set OPTIMIZE=3

rem === SHADER BUILD CONFIGURATION END
rem ======================================

rem ======================================
rem === RUN

set BUILD_SHADER=call buildshaders.bat

%BUILD_SHADER% stdshader_dx9_20b -game %GAMEDIR% -source %SOURCEDIR%
%BUILD_SHADER% stdshader_dx9_20b_new -game %GAMEDIR% -source %SOURCEDIR%
%BUILD_SHADER% stdshader_dx9_30 -game %GAMEDIR% -source %SOURCEDIR% -force30 
