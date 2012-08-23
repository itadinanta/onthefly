LDOPTS=-lGL -lGLU -lglut -lm

onthefly: onthefly.C
	gcc -ansi -pedantic -O3 -fnew-ra -fssa-dce onthefly.C $(LDOPTS) -o onthefly
	strip onthefly

debug: onthefly.debug

onthefly.debug: onthefly.C
	gcc -ggdb -pg onthefly.C $(LDOPTS) -o onthefly.debug

test: onthefly
	./onthefly

onthefly.ps2: onthefly.C
	gcc -O3 onthefly.C -DPS2 -lGL -lps2dev $(LDOPTS) -lstdc++ -o onthefly.ps2

onthefly.p4: onthefly.C
	gcc -O3 onthefly.C -march=pentium4 -mfpmath=sse $(LDOPTS) -o onthefly.p4
	strip onthefly.p4

clean: 
	rm onthefly onthefly.ps2 onthefly.opt onthefly.debug onthefly.p4 || true

