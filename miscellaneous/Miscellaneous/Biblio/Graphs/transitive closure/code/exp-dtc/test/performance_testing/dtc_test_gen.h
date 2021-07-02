// --------------------------------------------------------------------
//
//  File:        dtc_test_gen.h
//  Date:        02/98
//  Last update: 03/99
//  Description: Generator class for tests (graphs and opseqs)
//
//  (C) 1998, 1999 Tobias Miller & Christos Zaroliagis
//                 {miller,zaro}@mpi-sb.mpg.de
//
// --------------------------------------------------------------------

#ifndef _DTC_TEST_GEN_H
#define _DTC_TEST_GEN_H

#define DTC_ALGORITHM dtc_cfnp
#include <LEP/dynamic_graphs/dtc_cfnp.h>

#include <LEDA/window.h>
#include "dtc_test.h"

/******************** Class DTC_Test_Gen ********************/

enum gen_type { _non_random_, _random_ };

class dtc_test_gen : public dtc_test
  {
    private:
      DTC_ALGORITHM*	DTC;
      random_source	RandomSource;
      int		UpdAnz, InsUpdAnz, RemUpdAnz;
      int		QryAnz, BoolQryAnz, PathQryAnz;
      
      void save();
      bool gen_error_panel();
      bool gen_non_random();
      bool gen_random(int);
      bool gen_random_graph();
      bool gen_random_opseq();
      bool gen_random_source(node&);
      bool gen_random_target(node&, node&);
      void insert_queries(int, int);
      
    public:
      bool generate(graph&, window&);
  };

#endif _DTC_TEST_GEN_H
