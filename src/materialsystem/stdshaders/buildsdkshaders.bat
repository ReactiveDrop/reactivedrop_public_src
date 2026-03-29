@echo off
setlocal

REM ======================================
REM ==== PATH CONFIGURATIONS

REM == Setup path to nmake.exe, from vc common tools directory
call "G:\Program Files\Microsoft Visual Studio\18\Community\Common7\Tools\VsDevCmd.bat"

rem == Set the absolute path to your mod's game directory here
set GAMEDIR="G:\PENDRIVE\Steam\steamapps\common\Alien Swarm Reactive Drop\reactivedrop"

REM == Set the Path to your mods root source code
REM == "..\.." should already be correct, accepts relative paths only!
set SOURCEDIR=..\..

REM ==== PATH CONFIGURATIONS END
REM ======================================

REM ======================================
REM ==== SHADER BUILD CONFIGURATION

REM == Set force=1 to force shader recompilation 
REM set FORCE=1

REM == Set dynamic_shaders=1 to build .inc files only
REM set dynamic_shaders=1

REM == Number of threads used by ShaderCompile:
REM ==  <= 0 or unset = all threads (default)
REM ==  > 0           = force specific number of threads
REM ==  Set to override the default behavior.
REM set THREADS=-1

REM ==  Optimization level used by ShaderCompile:
REM ==  <= 0          = no optimization
REM ==  1–2           = intermediate optimization
REM ==  >= 3 or unset = full optimization (default)
REM ==  Set to override the default behavior.
REM set OPTIMIZE=3

REM ==== SHADER BUILD CONFIGURATION END
REM ======================================

REM ======================================
REM ==== RUN

set BUILD_SHADER=call buildshaders.bat

%BUILD_SHADER% stdshader_dx9_20b -game %GAMEDIR% -source %SOURCEDIR%
%BUILD_SHADER% stdshader_dx9_20b_new -game %GAMEDIR% -source %SOURCEDIR%
%BUILD_SHADER% stdshader_dx9_30 -game %GAMEDIR% -source %SOURCEDIR% -force30 
