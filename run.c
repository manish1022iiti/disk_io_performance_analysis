#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include "utils.h"

int main(int argc, char *argv[]) {
    char *p;
    unsigned int xor;
    time_t begin, end;

    //=================================================================================================================
    // Parsing and validating arguments
    //=================================================================================================================
    if (argc < 3) {
        printf("too few arguments. at least <filename> and read/write option [-r|-w] is needed!\n");
        exit(0);
    }

    // file name
    char *fileName = argv[1];

    // action (r|w)
    char action = argv[2][1];
    if ((action != 'r') & (action != 'w')) {
        printf("wrong read/write representation. Supported: [-r|-w]\n");
        exit(0);
    }

    // setting up default arguments
    long blockSize = 1024;
    // blockCount -1 is an indicator to read the entire file
    long blockCount = -1;

    // block size
    if (argc > 3) {
        errno = 0;
        blockSize = strtol(argv[3], &p, 10);
        if (errno != 0 || *p != '\0') {
            printf("Could not parse blockSize: %s\n", argv[3]);
            exit(0);
        }
    }
    // block count
    if (argc > 4) {
        errno = 0;
        blockCount = strtol(argv[4], &p, 10);
        if (errno != 0 || *p != '\0') {
            printf("Could not parse blockCount: %s\n", argv[4]);
            exit(0);
        }
    }

//    printf("fileName: %s, action: %c, blockSize: %ld, blockCount: %ld\n", fileName, action, blockSize, blockCount);

    switch (action) {
        case 'r':
            begin = time(NULL);
            xor = readFile(fileName, blockSize, blockCount);
            end = time(NULL);
            printf("xor value: %08x, time taken: %ld seconds\n", xor, (end - begin));
            break;
        case 'w':
            begin = time(NULL);
            writeFile(fileName, blockSize, blockCount);
            end = time(NULL);
            printf("time taken: %ld seconds\n", (end - begin));
            break;
        default:
            printf("action: %c, Code should NEVER reach here; if it did, there is a BUG!\n", action);
    }

    exit(0);
}
