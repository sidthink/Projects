#pragma once
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
// Data object.
typedef struct
{
    // 2D floating point array of input.
    float** in;
    // 2D floating point array of target.
    float** tg;
    // Number of inputs to neural network.
    int nips;
    // Number of outputs to neural network.
    int nops;
    // Number of rows in file (number of sets for neural network).
    int rows;
}
Data;

int lns(FILE* const file);
char* readln(FILE* const file);
 float** new2d(const int rows, const int cols);
 Data ndata(const int nips, const int nops, const int rows);
 void parse(const Data data, char* line, const int row);
void dfree(const Data d);
 void shuffle(const Data d);
 Data build(const char* path, const int nips, const int nops);