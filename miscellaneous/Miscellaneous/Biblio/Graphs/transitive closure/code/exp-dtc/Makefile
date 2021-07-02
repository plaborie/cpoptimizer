#------------------------------------------------------------------------------
# Library
#------------------------------------------------------------------------------

SHELL   = /bin/sh

all: .configured lib demos tests

lib: .configured
	cd src;  $(MAKE) FLAGS=-O;

lib_g: .configured
	cd src;  $(MAKE) FLAGS=-g;

lib_Wall: .configured
	cd src;  $(MAKE) FLAGS=-Wall;


#------------------------------------------------------------------------------
# Configuration
#------------------------------------------------------------------------------

.configured: Makefile
	@ $(MAKE) -i configure

configure:
	@ ./lconfig                  # configure
	@ $(MAKE) clean

#------------------------------------------------------------------------------
# Demos
#------------------------------------------------------------------------------

demos:
	cd demo; $(MAKE) -i FLAGS=-O;

demos_g:
	cd demo; $(MAKE) -i FLAGS=-g;

#------------------------------------------------------------------------------
# Tests
#------------------------------------------------------------------------------

tests:
	cd test; $(MAKE) -i FLAGS=-O;

tests_g:
	cd test; $(MAKE) -i FLAGS=-g;


#------------------------------------------------------------------------------
# Cleaning up
#------------------------------------------------------------------------------

del_lib:
	@ echo "deleting libraries..."
	@ $(RM) lib*.a lib*.so

del_obj:
	@ echo "deleting object files..."
	@ cd src;  $(MAKE) -i clean

del_prog:
	@ echo "deleting executables..."
	@ cd demo; $(MAKE) -i clean
	@ cd test; $(MAKE) -i clean

del_conf:
	@ echo "deleting configuration files..."
	@ cd src; $(RM) Make.src
	@ cd demo; $(RM) Make.demo
	@ cd test; $(RM) Make.test
	@ $(RM) .configured

clean: .configured del_lib del_obj del_prog

dist_clean: clean del_conf

