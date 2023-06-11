# make makefile
#
# Tools used:
#  Compile::Watcom Resource Compiler
#  Compile::GNU C
#  Make: GNU make
all : animate1.exe

animate1.exe : animate1.obj animate1.def animate1.res  
	gcc -Zomf animate1.obj animate1.def animate1.res -o animate1.exe
	wrc animate1.res

animate1.obj : animate1.c animate1.h 
	gcc -Wall -Zomf -c -O2 animate1.c -o animate1.obj

animate1.res : animate1.rc animate1.ico 
	wrc -r animate1.rc

clean :
	rm -rf *exe *res *obj *dll *lib
