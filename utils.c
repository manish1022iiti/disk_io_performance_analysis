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
#include <pthread.h>

// while finding the right blockCount for any experiment, we want to stop when blockCount hits 2^32
// i.e. we will try to find the right blockCount ONLY till <= 2^31 blocks
const int MAX_POW2_FOR_TEST_BLOCK_COUNTS = 32;

// equivalent to 128 MiB
const int MAX_POW2_FOR_BLOCK_SIZE_TESTING = 27;
//const int MAX_POW2_FOR_BLOCK_SIZE_TESTING = 5;

// number of iterations to do while computing average read time
const int NUM_ITER_TO_COMPUTE_AVG_READ_TIME = 5;

// max reasonable time (in seconds)
const int MAX_REASONABLE_TIME = 8;
//const int MAX_REASONABLE_TIME = 5;

struct readBlockStruct {
    int fd;
    long startPos;
    long blockCount;
    long blockSize;
};

long * getTestBlockSizes(){
    // since we want to start testing from block size = 4 bytes
    static long test[26];
//    static long test[4];
    for (int i = 2; i <= MAX_POW2_FOR_BLOCK_SIZE_TESTING; i++){
        test[i - 2] = (long) pow(2, i);
    }
    return test;
}

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
    int n = MAX_POW2_FOR_TEST_BLOCK_COUNTS;
    int numIter = NUM_ITER_TO_COMPUTE_AVG_READ_TIME;
    long currBc, test[n];
    time_t begin, end, timeTaken;
    size_t fileSize = getFileSize(fileName);

    // getting block counts to run our tests on
    getTestBlockCounts(test, n);

    // running tests
    for(int i = 0; i < n; i++){
        currBc = test[i];

        // If the file is NOT large enough (i.e >= blockSize * blockCount), no point in continuing
        if (currBc * blockSize > fileSize){
            printf("Bummer! File Size: %ld is < currBc * blockSize: %ld * %ld. Get a larger"
                   " file please.\n", fileSize, currBc, blockSize);
            exit(0);
        }

        // reading given block counts of the file "numIter" times to compute average time to read
        timeTaken = 0;
        for (int j = 0; j < numIter; j++){
            begin = time(NULL);
            readFile(fileName, blockSize, currBc);
            end = time(NULL);
            timeTaken += end - begin;
        }
        timeTaken = ceil((double) timeTaken / numIter);

//        printf("filename: %s, test_file_size: %ld B, block_size: %ld, block_count(2**%d): %ld, wall time: %ld seconds\n",
//               fileName, fileSize, blockSize, i, currBc, timeTaken);

        // if time taken to read the current no. of block counts is > 15 seconds, returning the
        // previous read block count as answer (since it is reasonable i.e. <= 15 seconds)
        if (timeTaken > MAX_REASONABLE_TIME){
            return test[i - 1];
        }
    }

    // if the code reaches till this point, that means none of the block counts (from 2^0 to 2^31)
    // were read in more than MAX_REASONABLE_TIME; therefore, returning the maximum block count we are
    // testing on i.e. test[n - 1] = 2^31
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

void lseekFile(char *fileName, long numIter) {
    int fd;
    long ret;

    // open file
    if ((fd = open(fileName, O_RDONLY)) < 0) {
        printf("Can not open file: %s\n", fileName);
        exit(0);
    }

    // lseek file
    for(int i = 0; i < numIter; i++){
        // just always move the pointer to the same position (4 bytes left to the EOF)
        ret = lseek(fd, 4, SEEK_END);

//        printf("%d: lseek(%s, 4, SEEK_END) = %ld\n", i, fileName, ret);

        if(ret == -1){
            printf("Bummer! Couldn't 'lseek' 4 bytes (SEEK_END) for file: %s\n", fileName);
            exit(0);
        }
    }

    // close file
    close(fd);
}

long getOptimalBlockSize(long fileSize){
    // x MiB
    long blockSize = 4 * 1024 * 1024;

    // if file size is <= x MiB, we will just read it with 4 byte block size
    // else we will use x MiB block size
    if (fileSize <= blockSize){
        return 4;
    }
    return blockSize;
}

void * readBlocks(void * args){
    struct readBlockStruct *arguments = (struct readBlockStruct *)args;
    int fd;
    long n, blockSize, blockCount, startPos, totBytesRead, currBlockCount, bufTotLen, bufReadLen;
    long offset;

    fd = arguments->fd;
    blockSize = arguments->blockSize;
    blockCount = arguments->blockCount;
    startPos = arguments->startPos;
    unsigned int xor = 0;

    // allocating buffer space
    bufTotLen = ceil((double) blockSize / sizeof(unsigned int));
    unsigned int *buf = malloc(bufTotLen * sizeof(unsigned int));

    // read file
    totBytesRead = 0;
    currBlockCount = 1;
    offset = startPos;
    while ((n = pread(fd, buf, blockSize, offset)) > 0) {
        totBytesRead += n;
        bufReadLen = ceil((double ) n / sizeof(unsigned int));

//        printf("Start Pos: %ld, Block Count: %ld, Block Size: %ld, Bytes Read (current): %ld, Bytes Read (total):"
//               "%ld, bufTotLen: %ld, bufReadLen: %ld\n", startPos, currBlockCount, blockSize, n,
//               totBytesRead, bufTotLen, bufReadLen);

        xor = xor ^ calculateXor(buf, bufReadLen);

        if (currBlockCount == blockCount)
            break;

        currBlockCount += 1;
        offset += n;
    }

//    printf("startPos: %ld, blockCount: %ld, blockSize: %ld, xor: %08x\n", startPos, blockCount,
//           blockSize, xor);

    pthread_exit((void *)xor);
}

unsigned int readFileFast(char *fileName, size_t fileSize, long blockSize, int threadCount) {
    long blockCount, threadBlockCount, remainingBlockCount, currFileIdx;
    int fd;
    pthread_t *threads;
    unsigned int txor, xor = 0;

    // get the optimal blockSize for the given fileSize (if the user input blockSize is < 0)
    if (blockSize < 0){
        blockSize = getOptimalBlockSize((long) fileSize);
    }

    // blockCount
    blockCount = ceil((double)fileSize / (double)blockSize);

//    printf("fileSize: %ld, blockSize: %ld, blockCount: %ld\n", fileSize, blockSize, blockCount);

    // initializing number of threads (if the user input blockSize is < 0)
    if (threadCount < 0){
        threadCount = 8;
    }

    // if blockCount is NOT >= a certain threshold, better to read the file sequentially
    if (blockCount < 2 * threadCount){
        return readFile(fileName, blockSize, blockCount);
    }

    // initializing threads
    threads = (pthread_t *) malloc(sizeof(pthread_t) * threadCount);

    // open file
    if ((fd = open(fileName, O_RDONLY)) < 0) {
        printf("Can not open file: %s\n", fileName);
        exit(0);
    }

    // read file
    struct readBlockStruct args[threadCount];
    threadBlockCount = floor((double) blockCount / threadCount);
    remainingBlockCount = blockCount;
    currFileIdx = 0;
    for (int i = 0; i < threadCount; i++){
        // setting up args
        args[i].fd = fd;
        if (i == threadCount - 1){
            args[i].blockCount = remainingBlockCount;
            remainingBlockCount = 0;
        }
        else{
            args[i].blockCount = threadBlockCount;
            remainingBlockCount -= threadBlockCount;
        }
        args[i].startPos = currFileIdx;
        args[i].blockSize = blockSize;

//        printf("thread %d: startPos: %ld, blockCount: %ld, blockSize: %ld\n", i, args[i].startPos,
//               args[i].blockCount, args[i].blockSize);

        // launching thread
        pthread_create(&threads[i], NULL, (void *(*)(void *)) readBlocks, (void*)&args[i]);

        // resetting index for next thread
        currFileIdx += args[i].blockCount * blockSize;
    }

    // collecting results of all threads
    for (int i = 0; i < threadCount; i++){
        pthread_join(threads[i], (void *)&txor);
        xor ^= txor;
    }
//    printf("xor: %08x\n", xor);

    // close file
    close(fd);

    return xor;
}

unsigned int readFile(char *fileName, long blockSize, long blockCount) {
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

    return xor;
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
