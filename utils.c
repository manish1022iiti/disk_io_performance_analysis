//
// Created by msaini on 12/10/21.
//

#include "utils.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>
#include <time.h>

size_t getFileSize(char* fileName){
    int fd;
    size_t sz;

    // open file
    if ((fd = open(fileName, O_RDONLY)) < 0) {
        printf("Can not open file: %s\n", fileName);
        exit(0);
    }

    sz = lseek(fd, 0, SEEK_END) + 1;
    close(fd);
    return sz;
}

void getTestBlockCounts(long *arr, int n){
    for (int i = 0; i < n; i++){
        arr[i] = (long) pow(2, i);
    }
}

long computeReasonableBlockCount(char *fileName, long blockSize){
    int n = 32;
    int numIter = 5;
    long currBc, test[n];
    time_t begin, end, timeTaken;
    size_t fileSize = getFileSize(fileName);

    // getting block counts to run our tests on
    getTestBlockCounts(test, n);

    // running tests
    for(int i = 0; i < n; i++){
        currBc = test[i];

        // If the file is NOT large enough (i.e >= blockSize * blockCount), no point in continuing
        if (currBc * blockSize > fileSize)
            break;

        // reading given block counts of the file "numIter" times to compute average time to read
        timeTaken = 0;
        for (int j = 0; j < numIter; j++){
            begin = time(NULL);
            readFile(fileName, blockSize, currBc);
            end = time(NULL);
            timeTaken += end - begin;
        }
        timeTaken = ceil((double) timeTaken / numIter);

        printf("filename: %s, file_size: %ld, block_size: %ld, block_count(2**%d): %ld, wall time: %ld seconds\n",
               fileName, fileSize, blockSize, i, currBc, timeTaken);

        // if time taken to read the current no. of block counts is >= 10 seconds, returning the
        // previous read block count as answer (since it is reasonable i.e. fewer than 10 seconds)
        if (timeTaken >= 10){
            return test[i - 1];
        }
    }

    // if the code reaches till this point, that means none of the block counts (from 2^0 to 2^31)
    // were read in 10 or more seconds; therefore, returning the maximum block count we are testing on
    // i.e. n - 1 = 2^31
    return test[n - 1];
}

unsigned int calculateXor(const unsigned int *buffer, long len) {
    unsigned int result = 0;
    for (int i = 0; i < len; i++) {
//        printf("size: %ld, i: %d, %u\n", len, i, buffer[i]);
        result ^= buffer[i];
    }
    return result;
}

void readFile(char *fileName, long blockSize, long blockCount) {
    long n, totBytesRead, currBlockCount, bufTotLen, bufReadLen;
    int fd;
    unsigned int xor = 0;

    // allocating buffer space
    bufTotLen = ceil((double) blockSize / sizeof(unsigned int));
    unsigned int *buf = malloc(bufTotLen * sizeof(unsigned int));

    // open file
    if ((fd = open(fileName, O_RDONLY)) < 0) {
        printf("Can not open file: %s\n", fileName);
        exit(0);
    }

    // read file
    totBytesRead = 0;
    currBlockCount = 1;
    while ((n = read(fd, buf, blockSize)) > 0) {
        totBytesRead += n;
        bufReadLen = ceil((double ) n / sizeof(unsigned int));

//        printf("Filename: %s, Block Count: %ld, Block Size: %ld, Bytes Read (current): %ld, Bytes Read (total):"
//               "%ld, bufTotLen: %ld, bufReadLen: %ld\n", fileName, currBlockCount, blockSize, n, totBytesRead,
//               bufTotLen, bufReadLen);

        xor = xor ^ calculateXor(buf, bufReadLen);

        // if blockCount is +ve, we only want to read fixed number of blocks and then exit
        // elif blockCount is -ve, we want to read till EOF
        if ((blockCount > 0) & (currBlockCount == blockCount))
            break;

        currBlockCount += 1;
    }

//    printf("\nfile_name: %s, xor value: %08x\n", fileName, xor);

    // close file
    close(fd);
}

void writeFile(char *fileName, long blockSize, long blockCount) {
    long n, bytesWritten, totBytesWritten;
    int fd;
    char *buf = malloc(blockSize);

    // open file
    if ((fd = open(fileName, O_WRONLY | O_CREAT, 0777)) < 0) {
        printf("unable to create/open file: %s\n", fileName);
        exit(1);
    }

    // write to file
    n = totBytesWritten = 0;
    while (n < blockCount) {
        bytesWritten = write(fd, buf, blockSize);
        totBytesWritten += bytesWritten;

        if (bytesWritten < 0) {
            printf("Error(%ld) while writing to file: %s (current block count: %ld)\n", bytesWritten, fileName, n);
            exit(0);
        }
//        printf("Filename: %s, Block Count: %ld, Bytes Written (current): %ld, Bytes Written (total): %ld\n",
//               fileName, n + 1, bytesWritten, totBytesWritten);
        n++;
    }

    // close file
    close(fd);
}
