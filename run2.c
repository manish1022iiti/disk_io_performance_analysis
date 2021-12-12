//
// Created by msaini on 12/9/21.
//
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    char *p;
    long blockSize, blockCount, fileSize;

    //=================================================================================================================
    // Parsing and validating arguments
    //=================================================================================================================
    if (argc < 3) {
        printf("too few arguments. at least <filename> and <block_size> is needed!\n");
        exit(0);
    }

    // file name
    char *fileName = argv[1];

    // block size
    errno = 0;
    blockSize = strtol(argv[2], &p, 10);
    if (errno != 0 || *p != '\0') {
        printf("Could not parse blockSize: %s", argv[2]);
        exit(0);
    }

    printf("fileName: %s, blockSize: %ld\n", fileName, blockSize);

    blockCount = computeReasonableBlockCount(fileName, blockSize);
    fileSize = blockCount * blockSize;

    printf("For block_size: %ld bytes, Reasonable block_count (file_size): %ld (%ld bytes)\n", blockSize,
           blockCount, fileSize);

    exit(0);
}
