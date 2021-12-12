//
// Created by msaini on 12/10/21.
//

#ifndef DISK_IO_PERFORMANCE_ANALYSIS_UTILS_H
#define DISK_IO_PERFORMANCE_ANALYSIS_UTILS_H

#include <stdlib.h>

unsigned int calculateXor(const unsigned int *buffer, long size);
unsigned int readFile(char *fileName, long blockSize, long blockCount);
void writeFile(char *fileName, long blockSize, long blockCount);
void getTestBlockCounts(long *arr, int n);
long computeReasonableBlockCount(char *fileName, long blockSize);
size_t getFileSize(char *fileName);
long * getTestBlockSizes();
const int MAX_POW2_FOR_BLOCK_SIZE_TESTING;

#endif //DISK_IO_PERFORMANCE_ANALYSIS_UTILS_H
