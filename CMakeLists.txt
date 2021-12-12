cmake_minimum_required(VERSION 3.21)
project(disk_io_performance_analysis C)

set(CMAKE_C_STANDARD 99)

add_executable(disk_io_performance_analysis run.c run2.c utils.c utils.h temp/temp.c)
