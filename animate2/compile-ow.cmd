@echo off
rem -- animate2 -- OpenWatcom build script ----------------------------------
rem
rem  Compiler : OpenWatcom C (wcc386)
rem  Linker   : wlink (included with OpenWatcom)
rem  Resources: wrc (Watcom Resource Compiler)
rem
rem  Requires: OpenWatcom installed with WATCOM env var pointing to its root.
rem  Output  : bin-ow\animate2.exe
rem  Log     : bin-ow\compile.log

if not exist bin-ow md bin-ow

echo Compiling with OpenWatcom... > bin-ow\compile.log
wcc386 src\animate2.c -bt=os2 -mf -3 -zq -i=%WATCOM%\h;%WATCOM%\h\os2 -fo=bin-ow\animate2.obj >> bin-ow\compile.log 2>&1

echo. >> bin-ow\compile.log
echo Linking... >> bin-ow\compile.log
wlink system os2v2_pm name bin-ow\animate2.exe file { bin-ow\animate2.obj } option stack=16384 option description '@#OS2World:1.02#@##1## 23 Jul 2026 00:00:00     ARCAOS-507::::::v1.02@@Animate2 - PM animation with a single sprite-sheet bitmap and a timer' >> bin-ow\compile.log 2>&1

echo. >> bin-ow\compile.log
echo Compiling resources... >> bin-ow\compile.log
wrc -r -I%WATCOM%\h\os2 -I%WATCOM%\h -Isrc -fo=bin-ow\animate2.res src\animate2.rc >> bin-ow\compile.log 2>&1
wrc bin-ow\animate2.res bin-ow\animate2.exe >> bin-ow\compile.log 2>&1

echo. >> bin-ow\compile.log
if exist bin-ow\animate2.exe echo BUILD SUCCESSFUL >> bin-ow\compile.log
if not exist bin-ow\animate2.exe echo BUILD FAILED >> bin-ow\compile.log
