#!/usr/bin/env bash

gcc -O3 -fno-stack-protector -pthread utils.c run.c -o run -lm
gcc -O3 -fno-stack-protector -pthread utils.c run2.c -o run2 -lm
gcc -O3 -fno-stack-protector -pthread utils.c fast.c -o fast -lm
gcc -O3 -fno-stack-protector -pthread utils.c analysis.c -o analysis -lm
