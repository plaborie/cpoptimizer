# Shortest paths with nagative cycle detection
# Written by Andrew V. Goldberg and Boris V. Cherkassky, 1996, 1998

CCOMP = gcc
#CFLAGS = -g
CFLAGS = -O4 -DNDEBUG -DWALL -DXRUN
#CFLAGS = -O4

GENS = ../bin/crand ../bin/grid ../bin/tor ../bin/cnacyc
PROGS = ../bin/bfcf ../bin/bfcs ../bin/bfct ../bin/bfcm ../bin/bfctn ../bin/simp ../bin/gorc ../bin/ghk ../bin/ghk1 ../bin/ghk2 ../bin/ghk3 ../bin/palt ../bin/bfch


install: gens progs

clean:
	rm -f $(GENS) $(PROGS)

gens: $(GENS)

../bin/crand: crand.c random.c
	$(CCOMP) $(CFLAGS) -o ../bin/crand crand.c

../bin/grid: grid.c random.c
	$(CCOMP) $(CFLAGS) -o ../bin/grid grid.c

../bin/tor: tor.c random.c
	$(CCOMP) $(CFLAGS) -o ../bin/tor tor.c

../bin/cnacyc: nacyc.c random.c
	$(CCOMP) $(CFLAGS) -o ../bin/cnacyc nacyc.c

progs: $(PROGS)

../bin/ghk: types_ghk.h queue.h parser.c globals.h ghk.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/ghk -Dghk spc.c

../bin/ghk1: types_ghk.h queue.h parser.c globals.h ghk1.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/ghk1 -Dghk1 spc.c

../bin/ghk2: types_ghk.h queue.h parser.c globals.h ghk2.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/ghk2 -Dghk2 spc.c

../bin/ghk3: types_ghk.h queue.h parser.c globals.h ghk3.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/ghk3 -Dghk3 spc.c

../bin/bfcf: types_bf.h queue.h parser.c globals.h bfcf.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/bfcf -Dbfcf spc.c

../bin/bfch: types_bf.h queue.h parser.c globals.h bfch.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/bfch -Dbfch spc.c

../bin/bfcs: types_bf.h queue.h parser.c globals.h bfcs.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/bfcs -Dbfcs spc.c

../bin/bfct: types_simp.h queue.h parser.c globals.h bfct.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/bfct -Dbfct spc.c

../bin/bfctn: types_simp.h queue.h parser.c globals.h bfctn.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/bfctn -Dbfctn spc.c

../bin/bfcm: types_bfm.h mqueue.h parser.c globals.h bfcm.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/bfcm -Dbfcm spc.c

../bin/simp: types_simp.h queue.h parser.c globals.h simp.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/simp -Dsimp spc.c

../bin/gorc: types_gor.h stack.h parser.c globals.h gorc.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/gorc -Dgorc spc.c

../bin/palt: types_simp.h stack.h parser.c globals.h palt.c spc.c
	$(CCOMP) $(CFLAGS) -o ../bin/palt -Dpalt spc.c

