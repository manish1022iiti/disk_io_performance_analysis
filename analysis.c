//
// Created by msaini on 12/12/21.
//
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

void analyzeRun2(char* fileName){
    long bc;
    long *testBs = getTestBlockSizes();
    int n = MAX_POW2_FOR_BLOCK_SIZE_TESTING - 1;

    printf("Beginning test on file: %s; total no. of blocksizes to be tested: %d\n",fileName, n);

    for(int i = 0; i < n; i++){
        bc = computeReasonableBlockCount(fileName, testBs[i]);
        printf("Given: testFileName: %s, testBlockSize: %ld; Output: reasonableBlockCount (fileSize): %ld (%ld bytes)\n",
               fileName, testBs[i], bc, testBs[i]*bc);
    }
}

int main(int argc, char *argv[]) {
    //=================================================================================================================
    // Parsing and validating arguments
    //=================================================================================================================
    if (argc < 3) {
        printf("too few arguments. at least fileName and what 'analysis' to run is needed!\n");
        exit(0);
    }

    // file name
    char *fileName = argv[1];

    // test name
    char testName = argv[2][1];

    printf("fileName: %s, testName: %c\n", fileName, testName);

    switch (testName) {
        case 'm':
            analyzeRun2(fileName);
            break;
        default:
            printf("testName: %c is INVALID.\n", testName);
            exit(0);
    }

    exit(0);
}