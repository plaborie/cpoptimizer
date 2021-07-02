## Platform dependent flags

# crimee
#SEP = "/"
#SYSTEM = sparc_5_5.0
#LIBFORMAT = static_pic_mt
#CCC = /opt/SUNWspro/bin/CC5.0
#OPTIONS =  -pto -PIC -DIL_STD +w +w2
#DEBUG = -O -DNDEBUG
#OTHERLIBS = -lsocket -lnsl -lsunmath
#NFSHOME = /nfs/optim
#EXE=
#OBJ=.o

# HP 32
#SEP = "/"
#SYSTEM = hp32_pa8000_11_3.15
#LIBFORMAT = static_pic
#CCC = /bin/aCC3.15
#OPTIONS =  +DA2.0 +w +W400,495,361,655,392,684 
#DEBUG = -O -DNDEBUG
#OTHERLIBS = 
#NFSHOME = /nfs/optim
#EXE =
#OBJ=.o

# MSVC7.1
SEP = /
SYSTEM = x86_.net2003_7.1
LIBFORMAT = stat_mda
CCC = cl
OPTIONS = /nologo /MD /DIL_STD  /W3 /GX /Zm500 /D_WINDOWS /Op  /Zc:forScope
DEBUG = /O2 /DNDEBUG
#DEBUG = /Od /Z7
OTHERLIBS = 
NFSHOME = //glaciere/optim
EXE = .exe
OBJ=.obj

## ILOG Product path
SOLVERDIR = $(NFSHOME)$(SEP)solver$(SEP)cplus$(SEP)solver61rc1
CONCERTDIR = $(NFSHOME)$(SEP)concert$(SEP)cplus$(SEP)concert21rc1
SCHEDDIR = $(NFSHOME)$(SEP)schedule$(SEP)cplus$(SEP)scheduler61

## *************************************************************
## *************************************************************

## *************************************************************
## *************************************************************

CONCERTINCDIR = $(CONCERTDIR)$(SEP)include
SOLVERINCDIR = $(SOLVERDIR)$(SEP)include
SCHEDINCDIR = $(SCHEDDIR)$(SEP)include

CONCERTLIBDIR = $(CONCERTDIR)$(SEP)lib$(SEP)$(SYSTEM)$(SEP)$(LIBFORMAT)
SOLVERLIBDIR = $(SOLVERDIR)$(SEP)lib$(SEP)$(SYSTEM)$(SEP)$(LIBFORMAT)
SCHEDLIBDIR = $(SCHEDDIR)$(SEP)lib$(SEP)$(SYSTEM)$(SEP)$(LIBFORMAT)

#LDFLAGS = -L$(SCHEDLIBDIR) -lschedule -L$(SOLVERLIBDIR) -lsolver -L$(CONCERTLIBDIR) -lconcert -lm $(OTHERLIBS)

LDFLAGS = /link /LIBPATH:\lib /INCREMENTAL:NO /FIXED:NO wsock32.lib user32.lib advapi32.lib  /LIBPATH:. /LIBPATH:$(SCHEDLIBDIR) /LIBPATH:$(SOLVERLIBDIR) /LIBPATH:$(CONCERTLIBDIR) schedule.lib solver.lib concert.lib

INCFLAGS = -I$(CONCERTINCDIR) -I$(SOLVERINCDIR) -I$(SCHEDINCDIR)

all: exec bench

clean:
	rm -f benchAIJ1 benchAIJ1.o *~

exec: benchAIJ1

benchAIJ1.o: benchAIJ1.cpp
	$(CCC) $(OPTIONS) -c $(DEBUG) $(INCFLAGS) -o benchAIJ1.o benchAIJ1.cpp

benchAIJ1: benchAIJ1.o
	$(CCC) $(OPTIONS) $(INCFLAGS) benchAIJ1$(OBJ) /FebenchAIJ1$(EXE) $(LDFLAGS) 

bench: benchAIJ1
	touch OUTPUT
	rm -f OUTPUT
	touch OUTPUT
	benchAIJ1 data/open/psp10.sch 91   1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp10.sch 92   3 2 0 >> OUTPUT
	benchAIJ1 data/open/psp27.sch 95   1 2 1 >> OUTPUT
	benchAIJ1 data/open/psp27.sch 96   2 1 0 >> OUTPUT
	benchAIJ1 data/open/psp82.sch 454  1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp6.sch  210  3 3 0 >> OUTPUT
	benchAIJ1 data/open/psp6.sch  211  2 1 0 >> OUTPUT
	benchAIJ1 data/open/psp12.sch 196  1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp12.sch 197  2 1 0 >> OUTPUT
	benchAIJ1 data/open/psp20.sch 198  1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp20.sch 199  1 2 0 >> OUTPUT
	benchAIJ1 data/open/psp30.sch 203  3 2 1 >> OUTPUT
	benchAIJ1 data/open/psp30.sch 204  3 3 1 >> OUTPUT
	benchAIJ1 data/open/psp41.sch 336  1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp41.sch 337  3 2 0 >> OUTPUT
	benchAIJ1 data/open/psp43.sch 1002 2 2 2 >> OUTPUT
	benchAIJ1 data/open/psp54.sch 343  1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp54.sch 344  1 2 0 >> OUTPUT
	benchAIJ1 data/open/psp58.sch 316  1 1 0 >> OUTPUT
	benchAIJ1 data/open/psp58.sch 317  2 1 0 >> OUTPUT
	benchAIJ1 data/open/psp69.sch 1283 2 3 2 >> OUTPUT
