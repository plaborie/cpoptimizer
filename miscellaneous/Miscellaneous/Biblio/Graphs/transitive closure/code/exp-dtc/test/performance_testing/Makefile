 
UNIXLIB = -L../.. -lLEP_dynamic_graphs -lW -lP -lG -lL -lX11 -lm $(SOL)

WINDOW_APP = 1

include ../Make.test

DTC      =    dtc_test 

DTCOBJS  =    dtc_evaluate.o dtc_test.o dtc_test_gen.o

$(DTC): $(DTCOBJS)
	$(LD) -o $(DTC) $(DTCOBJS) $(XLIB_PATH) $(LFLAGS) $(LDFLAGS) $(LPATH) $(UNIXLIB)

