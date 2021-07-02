## Platform dependent flags
SYSTEM = hp32_pa8000_11_3.15
LIBFORMAT = static_pic

CCC = /bin/aCC3.15
OPTIONS =  +DA2.0 +w +W400,495,361,655,392,684 
DEBUG = -O -DNDEBUG

## ILOG Product path
SOLVERDIR = /nfs/optim/solver/cplus/solver52
CONCERTDIR = /nfs/optim/solver/concert/concert12
SCHEDDIR = /nfs/optim/schedule/cplus/sched52

## *************************************************************
## *************************************************************

CONCERTINCDIR = $(CONCERTDIR)/include
SOLVERINCDIR = $(SOLVERDIR)/include
SCHEDINCDIR = $(SCHEDDIR)/include

CONCERTLIBDIR = $(CONCERTDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
SOLVERLIBDIR = $(SOLVERDIR)/lib/$(SYSTEM)/$(LIBFORMAT)
SCHEDLIBDIR = $(SCHEDDIR)/lib/$(SYSTEM)/$(LIBFORMAT)

LDFLAGS = -L$(SCHEDLIBDIR) -lschedule -L$(SOLVERLIBDIR) -lsolver -L$(CONCERTLIBDIR) -lconcert -lm
INCFLAGS = -I$(CONCERTINCDIR) -I$(SOLVERINCDIR) -I$(SCHEDINCDIR)

all: exec bench

clean:
	rm -f benchAIJ2 benchAIJ2.o *~

exec: benchAIJ2

benchAIJ2.o: benchAIJ2.cpp
	$(CCC) $(OPTIONS) -c $(DEBUG) $(INCFLAGS) -o benchAIJ2.o benchAIJ2.cpp

benchAIJ2: benchAIJ2.o
	$(CCC) $(CFLAGS) benchAIJ2.o -o benchAIJ2 $(LDFLAGS) 	

bench: benchAIJ2
	touch OUTPUT
	rm -f OUTPUT
	touch OUTPUT
	benchAIJ2 data/abz5 >> OUTPUT
	benchAIJ2 data/abz6 >> OUTPUT
	benchAIJ2 data/ft06 >> OUTPUT
	benchAIJ2 data/ft10 >> OUTPUT
	benchAIJ2 data/ft20 >> OUTPUT
	benchAIJ2 data/la01 >> OUTPUT
	benchAIJ2 data/la02 >> OUTPUT
	benchAIJ2 data/la03 >> OUTPUT
	benchAIJ2 data/la04 >> OUTPUT
	benchAIJ2 data/la05 >> OUTPUT
	benchAIJ2 data/la06 >> OUTPUT
	benchAIJ2 data/la07 >> OUTPUT
	benchAIJ2 data/la08 >> OUTPUT
	benchAIJ2 data/la09 >> OUTPUT
	benchAIJ2 data/la10 >> OUTPUT
	benchAIJ2 data/la11 >> OUTPUT
	benchAIJ2 data/la12 >> OUTPUT
	benchAIJ2 data/la13 >> OUTPUT
	benchAIJ2 data/la14 >> OUTPUT
	benchAIJ2 data/la15 >> OUTPUT
	benchAIJ2 data/la16 >> OUTPUT
	benchAIJ2 data/la17 >> OUTPUT
	benchAIJ2 data/la18 >> OUTPUT
	benchAIJ2 data/la19 >> OUTPUT
	benchAIJ2 data/la20 >> OUTPUT
	benchAIJ2 data/la21 >> OUTPUT
	benchAIJ2 data/la22 >> OUTPUT
	benchAIJ2 data/la23 >> OUTPUT
	benchAIJ2 data/la24 >> OUTPUT
	benchAIJ2 data/la25 >> OUTPUT
	benchAIJ2 data/la26 >> OUTPUT
	benchAIJ2 data/la27 >> OUTPUT
	benchAIJ2 data/la28 >> OUTPUT
	benchAIJ2 data/la29 >> OUTPUT
	benchAIJ2 data/la30 >> OUTPUT
	benchAIJ2 data/la31 >> OUTPUT
	benchAIJ2 data/la32 >> OUTPUT
	benchAIJ2 data/la33 >> OUTPUT
	benchAIJ2 data/la34 >> OUTPUT
	benchAIJ2 data/la35 >> OUTPUT
	benchAIJ2 data/la36 >> OUTPUT
	benchAIJ2 data/la37 >> OUTPUT
	benchAIJ2 data/la38 >> OUTPUT
	benchAIJ2 data/la39 >> OUTPUT
	benchAIJ2 data/la40 >> OUTPUT



