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

// default infinite performance measure = -1 B/s (when time taken is 0 seconds).
// If performance measure for any experiment is this value => it took 0 seconds to perform that
// experiment (for all "numIter" iterations we performed for it).
const double INFINITE_PERFORMANCE = -1;

// part 5
void analyzeOneByteReadPerformance(char *fileName){
    //since we plan to do all tests on blockSize = 1 bytes
    long blockSize = 1;

    int n = MAX_POW2_FOR_TEST_BLOCK_COUNTS;
    long currBc, test[n];
    time_t begin, end, timeTaken;
    double readPerformance, lseekPerformance;

    int ret;
    int numIter = NUM_ITER_FOR_AVG_READ_PERFORMANCE;

    size_t fileSize = getFileSize(fileName);

    // getting block counts to run our tests on
    getTestBlockCounts(test, n);

    // running test for each blockCount
    printf("fileName,blockSize(B),blockCount,rtest1(B/s),ltest1(B/s),rtest1(MiB/s),ltest1(MiB/s),"
           "rtest2(B/s),ltest2(B/s),rtest2(MiB/s),ltest2(MiB/s),"
           "rtest3(B/s),ltest3(B/s),rtest3(MiB/s),ltest3(MiB/s),"
           "rtest4(B/s),ltest4(B/s),rtest4(MiB/s),ltest4(MiB/s),"
           "rtest5(B/s),ltest5(B/s),rtest5(MiB/s),ltest5(MiB/s),\n");
    for(int i = 0; i < n; i++){
//    for(int i = 0; i < 28; i++){
        currBc = test[i];

        // If the file is NOT large enough (i.e >= blockSize * blockCount), no point in continuing
        if (currBc * blockSize > fileSize){
            printf("Bummer! File Size: %ld is < currBc * blockSize: %ld * %ld. Get a larger"
                   " file please.\n", fileSize, currBc, blockSize);
            exit(0);
        }

        printf("%s,%ld,%ld,", fileName, blockSize, currBc);

        readPerformance = lseekPerformance = INFINITE_PERFORMANCE;
        for(int j = 0; j < numIter; j++){
            //dropping cache (since we want to measure "cache-free" read time down below
            ret = system("sudo sh -c \"/usr/bin/echo 3 > /proc/sys/vm/drop_caches\"");

            //=======================================================================================
            // measuring read time
            //=======================================================================================
            begin = time(NULL);
            readFile(fileName, blockSize, currBc);
            end = time(NULL);
            timeTaken = end - begin;

            if (timeTaken > 0){
                // since blockSize = 1 byte, performance is just (blockCount / timeTaken)
                readPerformance = currBc / timeTaken;
            }
            //=======================================================================================

            //dropping cache (since we want to measure "cache-free" lseek time down below
            ret = system("sudo sh -c \"/usr/bin/echo 3 > /proc/sys/vm/drop_caches\"");

            //=======================================================================================
            // measuring lseek time
            //=======================================================================================
            begin = time(NULL);
            lseekFile(fileName, currBc);
            end = time(NULL);
            timeTaken = end - begin;

            // incorporating performance measure for current iteration ONLY IF time taken > 0
            if (timeTaken > 0){
                // since blockCount represents the no. of times "lseek" is done inside lseekFile function
                lseekPerformance = currBc / timeTaken;
            }
            //=======================================================================================

            printf("%f,%f,%f,%f,", readPerformance, lseekPerformance, readPerformance / (1024 * 1024),
                   lseekPerformance / (1024 * 1024));

//            printf("Given: fileName: %s, blockSize: %ld B; blockCount(2^%d): %ld, Iteration: %d,"
//                   " Output: readPerformance: %f B/s (%f MiB/s), lseekPerformance: %f B/s (%f MiB/s) \n",
//                   fileName, blockSize, i, currBc, j + 1, readPerformance, readPerformance / (1024 * 1024),
//                   lseekPerformance, lseekPerformance / (1024 * 1024));
        }
        printf("\n");
    }
}

// part 4 (subsumes results for part 2 and 3 as well)
void analyzeReadPerformance(char *fileName){
    int ret;
    long bc, reasonableFileSize;
    time_t begin, end, timeTaken;
    double cachedReadPerformance, uncachedReadPerformance;

    long *testBs = getTestBlockSizes();
    int n = MAX_POW2_FOR_BLOCK_SIZE_TESTING - 1;

    int numIterForAvgPerf = NUM_ITER_FOR_AVG_READ_PERFORMANCE;

//    printf("Beginning test on file: %s; total no. of blocksizes to be tested: %d\n",fileName, n);

    // analyzing performance for each blockSize
    printf("fileName,blockSize(B),reasonableBlockCount,reasonableFileSize(B),"
           "ctest1(B/s),utest1(B/s),ctest1(MiB/s),utest1(MiB/s),"
           "ctest2(B/s),utest2(B/s),ctest2(MiB/s),utest2(MiB/s),"
           "ctest3(B/s),utest3(B/s),ctest3(MiB/s),utest3(MiB/s),"
           "ctest4(B/s),utest4(B/s),ctest4(MiB/s),utest4(MiB/s),"
           "ctest5(B/s),utest5(B/s),ctest5(MiB/s),utest5(MiB/s),\n");
    for(int i = 0; i < n; i++){
        bc = computeReasonableBlockCount(fileName, testBs[i]);

        // reasonable fileSize (for test blockSize) = blockSize * reasonable blockCount
        reasonableFileSize = bc * testBs[i];

        // running multiple iterations (of reading the file with the test blockSize and corresponding
        // reasonable blockCount) to compute the average reading performance
        printf("%s,%ld,%ld,%ld,", fileName, testBs[i], bc, reasonableFileSize);

        cachedReadPerformance = uncachedReadPerformance = INFINITE_PERFORMANCE;
        // reading the file once before starting loop to fill the cache
        readFile(fileName, testBs[i], bc);
        for(int j = 0; j < numIterForAvgPerf; j++){
            //cached measure
            begin = time(NULL);
            readFile(fileName, testBs[i], bc);
            end = time(NULL);
            timeTaken = end - begin;
            if (timeTaken > 0){
                cachedReadPerformance = reasonableFileSize / timeTaken;
            }

            //dropping cache
            ret = system("sudo sh -c \"/usr/bin/echo 3 > /proc/sys/vm/drop_caches\"");

            //uncached measure
            begin = time(NULL);
            readFile(fileName, testBs[i], bc);
            end = time(NULL);
            timeTaken = end - begin;
            if (timeTaken > 0){
                uncachedReadPerformance = reasonableFileSize / timeTaken;
            }

            printf("%f,%f,%f,%f,", cachedReadPerformance, uncachedReadPerformance,
                   cachedReadPerformance / (1024 * 1024), uncachedReadPerformance / (1024 * 1024));

//            printf("Given: fileName: %s, blockSize: %ld B; Output: reasonableBlockCount: %ld,"
//                   " reasonableFileSize: %ld B, #%d: (cached)ReadPerformance: %f B/s (%f MiB/s),"
//                   " (UNcached)ReadPerformance: %f B/s (%f MiB/s) \n", fileName, testBs[i], bc,
//                   reasonableFileSize, j + 1, cachedReadPerformance, cachedReadPerformance / (1024 * 1024),
//                   uncachedReadPerformance, uncachedReadPerformance / (1024 * 1024));
        }
        printf("\n");
    }
}

// part 2
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

//    printf("fileName: %s, testName: %c\n", fileName, testName);

    switch (testName) {
        case '2':
            analyzeRun2(fileName);
            break;
        case '4':
            analyzeReadPerformance(fileName);
            break;
        case '5':
            analyzeOneByteReadPerformance(fileName);
            break;
        default:
            printf("testName: %c is INVALID.\n", testName);
            exit(0);
    }

    exit(0);
}