//=========================================================================
//  test_clock.c
//=========================================================================

// Authors:        Camil Demetrescu
// Change log:     CD000501
// Description:    utilities


#include <stdio.h>
#include <sys/resource.h>
#include <sys/time.h>


// FUNCTION DEFINITIONS

//=========================================================================
//  main
//=========================================================================
// main

int main() {

    while(1) {
        struct rusage tm;
        getrusage(RUSAGE_SELF,&tm);
        printf("tm.ru_utime.tv_sec+tm.ru_utime.tv_usec/1E6=%f\n",(float)tm.ru_utime.tv_sec+(float)tm.ru_utime.tv_usec/1E6);
    }

    return 0;
}
