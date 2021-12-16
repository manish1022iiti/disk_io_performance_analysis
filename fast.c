//
// Created by msaini on 12/15/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <errno.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    char *p;
    unsigned int xor;
    long blockSize;
    int threadCount;
    double performance, timeTaken;
    size_t fileSize;
    struct timeval begin, end;

    //=================================================================================================================
    // Parsing and validating arguments
    //=================================================================================================================
    if (argc < 2) {
        printf("too few arguments. <filename> is needed!\n");
        exit(0);
    }


    // file name
    char *fileName = argv[1];

    // blockSize (by default is -1, since we will let the code determine the optimal blockSize)
    blockSize = -1;
    if (argc >= 3){
        errno = 0;
        blockSize = strtol(argv[2], &p, 10);
        if (errno != 0 || *p != '\0') {
            printf("Could not parse blockSize: %s", argv[2]);
            exit(0);
        }
    }

    // threadCount (by default is -1, since we will let the code determine the optimal threadCount)
    threadCount = -1;
    if (argc >= 4) {
        errno = 0;
        threadCount = strtol(argv[3], &p, 10);
        if (errno != 0 || *p != '\0') {
            printf("Could not parse threadCount: %s", argv[3]);
            exit(0);
        }
    }

    // file size
    fileSize = getFileSize(fileName);

//    printf("reading file (%ld B): %s...\n", fileSize, fileName);

    // read file and compute xor
    // start timer.
    gettimeofday(&begin, NULL);

    xor = readFileFast(fileName, fileSize, blockSize, threadCount);

    // stop timer.
    gettimeofday(&end, NULL);
    timeTaken = (end.tv_sec - begin.tv_sec) * 1e6;
    timeTaken = (timeTaken + (end.tv_usec - begin.tv_usec)) * 1e-6;

    if (timeTaken > 0){
        // performance measure in B/s
        performance = fileSize / timeTaken;
        printf("xor: %08x, timeTaken: %f seconds, performance(B/s): %f, performance(MiB/s): %f\n",
               xor, timeTaken, performance, performance / (1024 * 1024));
    }
    else{
        printf("xor: %08x, timeTaken: %f seconds\n", xor, timeTaken);
    }

    exit(0);
}

