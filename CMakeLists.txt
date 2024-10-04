cmake_minimum_required(VERSION 3.29)
project(BankingSimulator C)

set(CMAKE_C_STANDARD 11)

add_executable(BankingSimulator main.c
        bank-lib.h
)
