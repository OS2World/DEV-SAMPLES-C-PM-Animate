@echo off
rem -- animate1 -- GCC build script ----------------------------------------
rem
rem  Compiler : GCC 9.2 for OS/2 (-Zomf produces OMF-format objects)
rem  Linker   : wlink via GCC front-end (EMXOMFLD_TYPE=WLINK)
rem  Resources: wrc (Watcom Resource Compiler)
rem
rem  Requires:
rem    yum install gcc libc-devel binutils watcom-wlink-hll watcom-wrc kbuild-make
rem
rem  Output : bin-gcc\animate1.exe
rem  Log    : bin-gcc\compile.log

if not exist bin-gcc md bin-gcc

rem Tell GCC's linker front-end to call wlink instead of ilink.
rem wlink links kLIBC (C runtime + startup) automatically and reads the
rem .def file to produce a WINDOWAPI (PM) LX executable.
set EMXOMFLD_TYPE=WLINK
set EMXOMFLD_LINKER=wl.exe

echo Compiling... > bin-gcc\compile.log
gcc -Zomf -Wall -O2 -c -o bin-gcc\animate1.obj src\animate1.c >> bin-gcc\compile.log 2>&1

echo. >> bin-gcc\compile.log
echo Linking... >> bin-gcc\compile.log
gcc -Zomf -o bin-gcc\animate1.exe bin-gcc\animate1.obj src\animate1.def >> bin-gcc\compile.log 2>&1

echo. >> bin-gcc\compile.log
echo Compiling resources... >> bin-gcc\compile.log
rem -Isrc lets wrc find frame1.bmp..frame16.bmp and animate1.h in src\
wrc -r -I%WATCOM%\h\os2 -I%WATCOM%\h -Isrc -fo=bin-gcc\animate1.res src\animate1.rc >> bin-gcc\compile.log 2>&1
wrc bin-gcc\animate1.res bin-gcc\animate1.exe >> bin-gcc\compile.log 2>&1

echo. >> bin-gcc\compile.log
if exist bin-gcc\animate1.exe echo BUILD SUCCESSFUL >> bin-gcc\compile.log
if not exist bin-gcc\animate1.exe echo BUILD FAILED >> bin-gcc\compile.log
