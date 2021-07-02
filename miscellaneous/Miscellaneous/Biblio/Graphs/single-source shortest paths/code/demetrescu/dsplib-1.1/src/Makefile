#                           -------------------------------------
#                           dsplib-1.1
#                           Dynamic single-source shortest paths
#                           Makefile (c) 2000 Camil Demetrescu
#                           Change log: CD000429
#                           -------------------------------------

#                           -------------------------------------
#                           Macros
#                           -------------------------------------

C   =                       gcc -O4 -I. -Iutil
CC  =                       g++ -O4 -I$(LEDAROOT)/incl -I.
LIB =                       -L$(LEDAROOT) -L/usr/X11/lib -lW -lP -lG -lL -lX11 -lm
BIN =                       ../bin/


#                           -------------------------------------
#                           Rules
#                           -------------------------------------

all:                        obj exe

obj:                        util/utility.o                      \
                            code_bfm/_bellman_ford.o            \
                            code_bfm/_bellman_ford_k.o          \
                            code_bfm/_bellman_ford_p.o          \
                            code_bfm/_bellman_ford_pk.o         \
                            code_dfmn/class_dfmn.o              \
                            code_dfmn/class_cd.o                \
                            code_dfmn/class_cdh.o               \
                            code_rr/class_rr.o

exe:                        $(BIN)test_dfmn                     \
                            $(BIN)check_dfmn                    \
                            $(BIN)run_dfmn                      \
                            $(BIN)test_cd                       \
                            $(BIN)check_cd                      \
                            $(BIN)run_cd                        \
                            $(BIN)test_cdh                      \
                            $(BIN)check_cdh                     \
                            $(BIN)run_cdh                       \
                            $(BIN)test_rr                       \
                            $(BIN)check_rr                      \
                            $(BIN)run_rr                        \
                            $(BIN)run_bfm                       \
                            $(BIN)run_bfmk                      \
                            $(BIN)run_bfmp                      \
                            $(BIN)run_bfmpk                     \
                            $(BIN)gen_graph_rnd_nnc             \
                            $(BIN)gen_graph_rnd_nncsc           \
                            $(BIN)gen_graph_rnd_nncgr           \
                            $(BIN)gen_graph_rnd_acyc            \
                            $(BIN)gen_seq_rnd_nnc_dfmn          \
                            $(BIN)gen_seq_rnd_nnc_cd            \
                            $(BIN)dimacs2leda                   \
                            $(BIN)average_data                  \
                            $(BIN)test_clock

clean:
	                        rm -f $(BIN)* util/*.o code_fmn/*.o code_dfmn/*.o code_bfm/*.o code_rr/*.o gen_graph/*.o gen_seq/*.o


#                           -------------------------------------
#                           Objects
#                           -------------------------------------

util/utility.o:             util/utility.c util/utility.h
	                        $(CC) util/utility.c -c -o util/utility.o

code_bfm/_bellman_ford.o:   code_bfm/_bellman_ford.c
	                        $(CC) code_bfm/_bellman_ford.c -c -o code_bfm/_bellman_ford.o

code_bfm/_bellman_ford_k.o: code_bfm/_bellman_ford.c
	                        $(CC) code_bfm/_bellman_ford.c -c -DKEEP_DECR_DIST_ -o code_bfm/_bellman_ford_k.o

code_bfm/_bellman_ford_p.o: code_bfm/_bellman_ford.c
	                        $(CC) code_bfm/_bellman_ford.c -c -DPARENT_CHECKING_ -o code_bfm/_bellman_ford_p.o

code_bfm/_bellman_ford_pk.o: code_bfm/_bellman_ford.c
	                        $(CC) code_bfm/_bellman_ford.c -c -DPARENT_CHECKING_ -DKEEP_DECR_DIST_ -o code_bfm/_bellman_ford_pk.o

code_dfmn/class_dfmn.o:     code_dfmn/class_dfmn.cpp code_dfmn/class_dfmn.h
	                        $(CC) code_dfmn/class_dfmn.cpp -c -o code_dfmn/class_dfmn.o

code_dfmn/class_cd.o:       code_dfmn/class_dfmn.cpp code_dfmn/class_dfmn.h
	                        $(CC) code_dfmn/class_dfmn.cpp -c -o code_dfmn/class_cd.o -DINCREASE_CD_VERSION_

code_dfmn/class_cdh.o:      code_dfmn/class_dfmn.cpp code_dfmn/class_dfmn.h
	                        $(CC) code_dfmn/class_dfmn.cpp -c -o code_dfmn/class_cdh.o -DINCREASE_CD_VERSION_ -DINCREASE_DELTA_HEUR_

code_rr/class_rr.o:         code_rr/class_rr.cpp code_rr/class_rr.h
	                        $(CC) code_rr/class_rr.cpp -c -o code_rr/class_rr.o
	
#                           -------------------------------------
#                           Executables
#                           -------------------------------------

$(BIN)test_dfmn:            code_dfmn/class_dfmn.o code_dfmn/test_dfmn.cpp
	                        $(CC) $(LIB) code_dfmn/class_dfmn.o code_dfmn/test_dfmn.cpp -o $(BIN)test_dfmn

$(BIN)check_dfmn:           code_dfmn/class_dfmn.o code_dfmn/check_dfmn.cpp
	                        $(CC) $(LIB) code_dfmn/class_dfmn.o code_dfmn/check_dfmn.cpp -o $(BIN)check_dfmn

$(BIN)run_dfmn:             util/utility.o code_dfmn/class_dfmn.o code_dfmn/run_dfmn.cpp
	                        $(CC) $(LIB) util/utility.o code_dfmn/class_dfmn.o code_dfmn/run_dfmn.cpp -o $(BIN)run_dfmn

$(BIN)test_cd:              code_dfmn/class_cd.o code_dfmn/test_dfmn.cpp
	                        $(CC) $(LIB) code_dfmn/class_cd.o code_dfmn/test_dfmn.cpp -o $(BIN)test_cd

$(BIN)check_cd:             code_dfmn/class_cd.o code_dfmn/check_dfmn.cpp
	                        $(CC) $(LIB) code_dfmn/class_cd.o code_dfmn/check_dfmn.cpp -o $(BIN)check_cd

$(BIN)run_cd:               util/utility.o code_dfmn/class_cd.o code_dfmn/run_dfmn.cpp
	                        $(CC) $(LIB) util/utility.o code_dfmn/class_cd.o code_dfmn/run_dfmn.cpp -o $(BIN)run_cd

$(BIN)test_cdh:             code_dfmn/class_cdh.o code_dfmn/test_dfmn.cpp
	                        $(CC) $(LIB) code_dfmn/class_cdh.o code_dfmn/test_dfmn.cpp -o $(BIN)test_cdh

$(BIN)check_cdh:            code_dfmn/class_cdh.o code_dfmn/check_dfmn.cpp
	                        $(CC) $(LIB) code_dfmn/class_cdh.o code_dfmn/check_dfmn.cpp -o $(BIN)check_cdh

$(BIN)run_cdh:              util/utility.o code_dfmn/class_cdh.o code_dfmn/run_dfmn.cpp
	                        $(CC) $(LIB) util/utility.o code_dfmn/class_cdh.o code_dfmn/run_dfmn.cpp -o $(BIN)run_cdh

$(BIN)test_rr:              code_rr/class_rr.o code_rr/test_rr.cpp
	                        $(CC) $(LIB) code_rr/class_rr.o code_rr/test_rr.cpp -o $(BIN)test_rr

$(BIN)check_rr:             code_rr/class_rr.o code_rr/check_rr.cpp
	                        $(CC) $(LIB) code_rr/class_rr.o code_rr/check_rr.cpp -o $(BIN)check_rr

$(BIN)run_rr:               util/utility.o code_rr/class_rr.o code_rr/run_rr.cpp
	                        $(CC) $(LIB) util/utility.o code_rr/class_rr.o code_rr/run_rr.cpp -o $(BIN)run_rr

$(BIN)run_bfm:              util/utility.o code_bfm/_bellman_ford.o code_bfm/run_bfm.c
	                        $(CC) $(LIB) util/utility.o code_bfm/_bellman_ford.o code_bfm/run_bfm.c -o $(BIN)run_bfm

$(BIN)run_bfmk:             util/utility.o code_bfm/_bellman_ford_k.o code_bfm/run_bfm.c
	                        $(CC) $(LIB) util/utility.o code_bfm/_bellman_ford_k.o code_bfm/run_bfm.c -DKEEP_DECR_DIST_ -o $(BIN)run_bfmk

$(BIN)run_bfmp:             util/utility.o code_bfm/_bellman_ford_p.o code_bfm/run_bfm.c
	                        $(CC) $(LIB) util/utility.o code_bfm/_bellman_ford_p.o code_bfm/run_bfm.c -o $(BIN)run_bfmp

$(BIN)run_bfmpk:            util/utility.o code_bfm/_bellman_ford_pk.o code_bfm/run_bfm.c
	                        $(CC) $(LIB) util/utility.o code_bfm/_bellman_ford_pk.o code_bfm/run_bfm.c -DKEEP_DECR_DIST_ -o $(BIN)run_bfmpk

$(BIN)gen_graph_rnd_nnc:    gen_graph/gen_graph_rnd_nnc.c
	                        $(CC) $(LIB) gen_graph/gen_graph_rnd_nnc.c -o $(BIN)gen_graph_rnd_nnc

$(BIN)gen_graph_rnd_nncsc:  gen_graph/gen_graph_rnd_nncsc.c util/random.c
	                        $(C) gen_graph/gen_graph_rnd_nncsc.c -o $(BIN)gen_graph_rnd_nncsc

$(BIN)gen_graph_rnd_nncgr:  gen_graph/gen_graph_rnd_nncgr.c util/random.c
	                        $(C) gen_graph/gen_graph_rnd_nncgr.c -o $(BIN)gen_graph_rnd_nncgr

$(BIN)gen_graph_rnd_acyc:   gen_graph/gen_graph_rnd_acyc.c util/random.c
	                        $(C) gen_graph/gen_graph_rnd_acyc.c -o $(BIN)gen_graph_rnd_acyc

$(BIN)gen_seq_rnd_nnc_dfmn: code_dfmn/class_dfmn.o gen_seq/gen_seq_rnd_nnc_dfmn.c
	                        $(CC) $(LIB) code_dfmn/class_dfmn.o gen_seq/gen_seq_rnd_nnc_dfmn.c -o $(BIN)gen_seq_rnd_nnc_dfmn

$(BIN)gen_seq_rnd_nnc_cd:   code_dfmn/class_cd.o gen_seq/gen_seq_rnd_nnc_dfmn.c
	                        $(CC) $(LIB) code_dfmn/class_cd.o gen_seq/gen_seq_rnd_nnc_dfmn.c -o $(BIN)gen_seq_rnd_nnc_cd

$(BIN)dimacs2leda:          util/dimacs2leda.cpp
	                        $(CC) $(LIB) util/dimacs2leda.cpp -o $(BIN)dimacs2leda

$(BIN)average_data:         util/average_data.c
	                        $(C) util/average_data.c -o $(BIN)average_data

$(BIN)test_clock:           util/test_clock.c
	                        $(C) util/test_clock.c -o $(BIN)test_clock
