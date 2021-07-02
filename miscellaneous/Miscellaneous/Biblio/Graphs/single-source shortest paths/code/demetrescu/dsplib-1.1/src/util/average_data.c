//=========================================================================
//  average_data.c
//=========================================================================

// Author:          Camil Demetrescu
// Change log:      CD000501
// Description:     read table from stdin and write table with averages
//                  to stdout


#include<stdlib.h>
#include<stdio.h>
#include<string.h>


//#define DEBUG_

struct Fields{
    int     NumNodes;
    int     NumEdges;
    int     MinWeight;
    int     MaxWeight;
    int     Seed;
    int     DOpCount;
    int     IOpCount;
    float   InitTime;
    float   TotDecrTime;
    float   TotIncrTime;
    float   MaxDOpTime;
    float   AvgDOpTime;
    float   MaxIOpTime;
    float   AvgIOpTime;
    float   AvgDN;
    float   AvgDM;
    float   AvgIN;
    float   AvgIM;
};


#define LINE_SIZE_  1024


//=========================================================================
//  main
//=========================================================================

int main(){

    char            theLine[LINE_SIZE_];
    struct Fields   theFields;
    struct Fields   theOldFields    = { 0 };
    struct Fields   theAverage      = { 0 };
    int             theGroupCount   = 0;

    // read first line from stdin and write it to stdout
    if ( fgets(theLine, LINE_SIZE_, stdin) != NULL )
        printf("%-5s %-5s %-5s %-5s %-5s %-5s %-7s %-7s %-7s %-10s %-10s %-10s %-10s %-10s %-10s %-10s %-10s\n",
                "n",        "m",        "MinW",     "MaxW",     "DOpC",  "IOpC",  "InitTm", "DecrTm", "IncrTm",
                "MaxDOpTm", "AvgDOpTm", "MaxIOpTm", "AvgIOpTm", "AvgDN", "AvgDM", "AvgIN",  "AvgIM" );

    // scan lines and average values
    while( fgets(theLine, LINE_SIZE_, stdin) != NULL ) {

        // parse line
        sscanf(theLine, "%d %d %d %d %d %d %d %f %f %f %f %f %f %f %f %f %f %f",
                        &theFields.NumNodes,
                        &theFields.NumEdges,
                        &theFields.MinWeight,
                        &theFields.MaxWeight,
                        &theFields.Seed,
                        &theFields.DOpCount,
                        &theFields.IOpCount,
                        &theFields.InitTime,
                        &theFields.TotDecrTime,
                        &theFields.TotIncrTime,
                        &theFields.MaxDOpTime,
                        &theFields.AvgDOpTime,
                        &theFields.MaxIOpTime,
                        &theFields.AvgIOpTime,
                        &theFields.AvgDN,
                        &theFields.AvgDM,
                        &theFields.AvgIN,
                        &theFields.AvgIM
                        );

        // output average values on values group end
        if ( theGroupCount != 0 &&
             ( theFields.NumNodes  != theOldFields.NumNodes  ||
               theFields.NumEdges  != theOldFields.NumEdges  ||
               theFields.MinWeight != theOldFields.MinWeight ||
               theFields.MaxWeight != theOldFields.MaxWeight ) ) {

            printf("%-5d %-5d %-5d %-5d %-5d %-5d %-7.4f %-7.4f %-7.4f %-10.6f %-10.6f %-10.6f %-10.6f %-10.2f %-10.2f %-10.2f %-10.2f\n",
                    theOldFields.NumNodes,
                    theOldFields.NumEdges,
                    theOldFields.MinWeight,
                    theOldFields.MaxWeight,
                    theAverage.DOpCount    / theGroupCount,
                    theAverage.IOpCount    / theGroupCount,
                    theAverage.InitTime    / theGroupCount,
                    theAverage.TotDecrTime / theGroupCount,
                    theAverage.TotIncrTime / theGroupCount,
                    theAverage.MaxDOpTime  / theGroupCount,
                    theAverage.AvgDOpTime  / theGroupCount,
                    theAverage.MaxIOpTime  / theGroupCount,
                    theAverage.AvgIOpTime  / theGroupCount,
                    theAverage.AvgDN       / theGroupCount,
                    theAverage.AvgDM       / theGroupCount,
                    theAverage.AvgIN       / theGroupCount,
                    theAverage.AvgIM       / theGroupCount
                    );

            theGroupCount = 0;

            theAverage.DOpCount    = 0;
            theAverage.IOpCount    = 0;
            theAverage.InitTime    = 0;
            theAverage.TotDecrTime = 0;
            theAverage.TotIncrTime = 0;
            theAverage.MaxDOpTime  = 0;
            theAverage.AvgDOpTime  = 0;
            theAverage.MaxIOpTime  = 0;
            theAverage.AvgIOpTime  = 0;
            theAverage.AvgDN       = 0;
            theAverage.AvgDM       = 0;
            theAverage.AvgIN       = 0;
            theAverage.AvgIM       = 0;
        }

        // add value to the group of values
        theGroupCount++;
        theOldFields = theFields;

        theAverage.DOpCount     += theFields.DOpCount;
        theAverage.IOpCount     += theFields.IOpCount;
        theAverage.InitTime     += theFields.InitTime;
        theAverage.TotDecrTime  += theFields.TotDecrTime;
        theAverage.TotIncrTime  += theFields.TotIncrTime;
        theAverage.MaxDOpTime   += theFields.MaxDOpTime;
        theAverage.AvgDOpTime   += theFields.AvgDOpTime;
        theAverage.MaxIOpTime   += theFields.MaxIOpTime;
        theAverage.AvgIOpTime   += theFields.AvgIOpTime;
        theAverage.AvgDN        += theFields.AvgDN;
        theAverage.AvgDM        += theFields.AvgDM;
        theAverage.AvgIN        += theFields.AvgIN;
        theAverage.AvgIM        += theFields.AvgIM;
    }

    // skip if no pending groups
    if ( theGroupCount == 0 ) return 0;

    // flush pending group values
    printf("%-5d %-5d %-5d %-5d %-5d %-5d %-7.4f %-7.4f %-7.4f %-10.6f %-10.6f %-10.6f %-10.6f %-10.2f %-10.2f %-10.2f %-10.2f\n",
            theOldFields.NumNodes,
            theOldFields.NumEdges,
            theOldFields.MinWeight,
            theOldFields.MaxWeight,
            theAverage.DOpCount    / theGroupCount,
            theAverage.IOpCount    / theGroupCount,
            theAverage.InitTime    / theGroupCount,
            theAverage.TotDecrTime / theGroupCount,
            theAverage.TotIncrTime / theGroupCount,
            theAverage.MaxDOpTime  / theGroupCount,
            theAverage.AvgDOpTime  / theGroupCount,
            theAverage.MaxIOpTime  / theGroupCount,
            theAverage.AvgIOpTime  / theGroupCount,
            theAverage.AvgDN       / theGroupCount,
            theAverage.AvgDM       / theGroupCount,
            theAverage.AvgIN       / theGroupCount,
            theAverage.AvgIM       / theGroupCount
            );

    return 0;
}
