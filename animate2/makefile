# make makefile
#
# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::GNU C
#  Make: GNU make
all : animate2.exe

animate2.exe : animate2.obj animate2.def animate2.res  
	gcc -Zomf animate2.obj animate2.def animate2.res -o animate2.exe
	wrc animate2.res

animate2.obj : animate2.c animate2.h 
	gcc -Wall -Zomf -c -O2 animate2.c -o animate2.obj

animate2.res : animate2.rc animate2.ico 
	wrc -r animate2.rc

clean :
	rm -rf *exe *res *obj *dll *lib
