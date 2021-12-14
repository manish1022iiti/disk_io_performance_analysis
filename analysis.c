//
// Created by msaini on 12/12/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "utils.h"

// number of iterations to do while computing avgReadPerformance for a given blockSize (and corresponding
// reasonable blockCount)
const int NUM_ITER_FOR_AVG_READ_PERFORMANCE = 5;

void analyzeReadPerformance(char *fileName){
    long bc, reasonableFileSize;
    time_t begin, end, timeTaken;
    double cachedReadPerformance, uncachedReadPerformance;

    long *testBs = getTestBlockSizes();
    int n = MAX_POW2_FOR_BLOCK_SIZE_TESTING - 1;

    int numIterForAvgPerf = NUM_ITER_FOR_AVG_READ_PERFORMANCE;

    printf("Beginning test on file: %s; total no. of blocksizes to be tested: %d\n",fileName, n);

    // analyzing performance for each blockSize
    for(int i = 0; i < n; i++){
        bc = computeReasonableBlockCount(fileName, testBs[i]);

        // reasonable fileSize (for test blockSize) = blockSize * reasonable blockCount
        reasonableFileSize = bc * testBs[i];

        // running multiple iterations (of reading the file with the test blockSize and corresponding
        // reasonable blockCount) to compute the average reading performance
        cachedReadPerformance = uncachedReadPerformance = 0.0;
        for(int j = 0; j < numIterForAvgPerf; j++){
            //cached measure
            begin = time(NULL);
            readFile(fileName, testBs[i], bc);
            end = time(NULL);
            timeTaken = end - begin;
            cachedReadPerformance += reasonableFileSize / timeTaken;

            //dropping cache
            system("sudo sh -c \"/usr/bin/echo 3 > /proc/sys/vm/drop_caches\"");

            //uncached measure
            begin = time(NULL);
            readFile(fileName, testBs[i], bc);
            end = time(NULL);
            timeTaken = end - begin;
            uncachedReadPerformance += reasonableFileSize / timeTaken;
        }
        cachedReadPerformance /= numIterForAvgPerf;
        uncachedReadPerformance /= numIterForAvgPerf;

        printf("Given: fileName: %s, blockSize: %ld B; Output: reasonableBlockCount: %ld,"
               " reasonableFileSize: %ld B, (cached)ReadPerformance: %f B/s (%f MiB/s),"
               " (UNcached)ReadPerformance: %f B/s (%f MiB/s) \n", fileName, testBs[i], bc,
               reasonableFileSize, cachedReadPerformance, cachedReadPerformance / (1024 * 1024),
               uncachedReadPerformance, uncachedReadPerformance / (1024 * 1024));
    }
}

void analyzeRun2(char* fileName){
    long bc;
    long *testBs = getTestBlockSizes();
    int n = MAX_POW2_FOR_BLOCK_SIZE_TESTING - 1;

    printf("Beginning test on file: %s; total no. of blocksizes to be tested: %d\n",fileName, n);

    // running test on each blockSize
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
    char testName = argv[2][0];

    printf("fileName: %s, testName: %c\n", fileName, testName);

    switch (testName) {
        case '2':
            analyzeRun2(fileName);
            break;
        case '4':
            analyzeReadPerformance(fileName);
            break;
        default:
            printf("testName: %c is INVALID.\n", testName);
            exit(0);
    }

    exit(0);
}