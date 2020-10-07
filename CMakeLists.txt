cmake_minimum_required(VERSION 3.14)
project(GA_Knapsack)

set(CMAKE_CXX_STANDARD 14)

add_executable(GA_Knapsack main.cpp GA.cpp GA.h Statistic.cpp Statistic.h)
