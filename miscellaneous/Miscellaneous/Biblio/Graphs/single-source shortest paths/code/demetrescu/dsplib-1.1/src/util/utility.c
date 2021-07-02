//=========================================================================
//  utility.c
//=========================================================================

// Authors:        Camil Demetrescu
// Change log:     CD000501
// Description:    utilities


#include "utility.h"


// FUNCTION DEFINITIONS

//=========================================================================
//  stop_watch
//=========================================================================
// stop_watch uses the UNIX getrusage function, returns current clock

float stop_watch(){
    rusage tm1;
    getrusage(RUSAGE_SELF,&tm1);
    return ((float)tm1.ru_utime.tv_sec+(float)tm1.ru_utime.tv_usec/1E6);
}


//=========================================================================
//  stop_watch
//=========================================================================
// stop_watch uses the UNIX getrusage function, returns elapsed time t2-t1
// in seconds

float stop_watch(float &t2){
    float t1=t2; 
    rusage tm1;
    getrusage(RUSAGE_SELF,&tm1);
    t2=(float)tm1.ru_utime.tv_sec+(float)tm1.ru_utime.tv_usec/1E6;
    return (t2-t1);
}


//=========================================================================
//  AppendToTable
//=========================================================================
// Append to inFileName a table row

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
                   float inAvDecrRefN,
                   float inAvDecrRefM,
                   float inAvIncrRefN,
                   float inAvIncrRefM) {

    FILE* theFp;
    int theExist;

    theFp=fopen(inFileName,"r");
    if (theExist=(theFp!=NULL)) fclose(theFp);

    theFp=fopen(inFileName,"a");

    if (theFp==NULL) exit((printf("Can't open file\n"),1));

    if (!theExist)
        fprintf(theFp,"%-5s %-5s %-5s %-5s %-5s %-5s %-5s %-7s %-7s %-7s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
                    "n",
                    "m",
                    "MinW",
                    "MaxW",
                    "Seed",
                    "DOpC",
                    "IOpC",
                    "InitTm",
                    "DecrTm",
                    "IncrTm",
                    "MaxDOpTm",
                    "AvgDOpTm",
                    "MaxIOpTm",
                    "AvgIOpTm",
                    "AvgDN",
                    "AvgDM",
                    "AvgIN",
                    "AvgIM");

    fprintf(theFp,"%-5d %-5d %-5d %-5d %-5d %-5d %-5d %-7.4f %-7.4f %-7.4f %-10.6f %-10.6f %-10.6f %-10.6f %-10.2f %-10.2f %-10.2f %-10.2f\n",
                  inNumNodes,
                  inNumEdges,
                  inMinWeight,
                  inMaxWeight,
                  inSeed,
                  inDecrOpCount,
                  inIncrOpCount,
                  inInitTime,
                  inTotDecrTime,
                  inTotIncrTime,
                  inMaxDecrOpTime,
                  inAvgDecrOpTime,
                  inMaxIncrOpTime,
                  inAvgIncrOpTime,
                  inAvDecrRefN,
                  inAvDecrRefM,
                  inAvIncrRefN,
                  inAvIncrRefM);

    fclose(theFp);
}



