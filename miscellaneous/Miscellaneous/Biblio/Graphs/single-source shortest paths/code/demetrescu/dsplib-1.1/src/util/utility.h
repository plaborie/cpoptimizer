//=========================================================================
//  utility.h
//=========================================================================

// Author:         Camil Demetrescu
// Change log:     CD000430
// Description:    utilities

#include <sys/resource.h>
#include <sys/time.h>
#include <LEDA/stream.h>
#include <LEDA/string.h>
#include <string.h>

float stop_watch();
float stop_watch(float &t2);

void AppendToTable(char* inFileName,
                   int inNumNodes,
                   int inNumEdges,
                   int inMinWeight,
                   int inMaxWeight,
                   int inSeed,
                   int inDecrOpCount,
                   int inIncrOpCount,
                   float inInitTime,
                   float inTotDecrTime,
                   float inTotIncrTime,
                   float inMaxDecrOpTime,
                   float inAvgDecrOpTime,
                   float inMaxIncrOpTime,
                   float inAvgIncrOpTime,
                   float inAvDecrRefN = 0.0,
                   float inAvDecrRefM = 0.0,
                   float inAvIncrRefN = 0.0,
                   float inAvIncrRefM = 0.0);
