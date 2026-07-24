@echo off
rem -- animate2 -- GCC build script ----------------------------------------
rem
rem  Compiler : GCC 9.2 for OS/2 (-Zomf produces OMF-format objects)
rem  Linker   : wlink via GCC front-end (EMXOMFLD_TYPE=WLINK)
rem  Resources: wrc (Watcom Resource Compiler)
rem
rem  Requires:
rem    yum install gcc libc-devel binutils watcom-wlink-hll watcom-wrc kbuild-make
rem
rem  Output : bin-gcc\animate2.exe
rem  Log    : bin-gcc\compile.log

if not exist bin-gcc md bin-gcc

set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe

echo Compiling... > bin-gcc\compile.log
gcc -Zomf -Wall -O2 -c -o bin-gcc\animate2.obj src\animate2.c >> bin-gcc\compile.log 2>&1

echo. >> bin-gcc\compile.log
echo Linking... >> bin-gcc\compile.log
gcc -Zomf -o bin-gcc\animate2.exe bin-gcc\animate2.obj src\animate2.def >> bin-gcc\compile.log 2>&1

echo. >> bin-gcc\compile.log
echo Compiling resources... >> bin-gcc\compile.log
rem -Isrc lets wrc find frames.bmp and animate2.h in src\
wrc -r -I%WATCOM%\h\os2 -I%WATCOM%\h -Isrc -fo=bin-gcc\animate2.res src\animate2.rc >> bin-gcc\compile.log 2>&1
wrc bin-gcc\animate2.res bin-gcc\animate2.exe >> bin-gcc\compile.log 2>&1

echo. >> bin-gcc\compile.log
if exist bin-gcc\animate2.exe echo BUILD SUCCESSFUL >> bin-gcc\compile.log
if not exist bin-gcc\animate2.exe echo BUILD FAILED >> bin-gcc\compile.log
