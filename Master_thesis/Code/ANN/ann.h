#pragma once

typedef struct
{
    // All the weights.
    float* w;
    // Hidden to output layer weights.
    float* x;
    // Biases.
    float* b;
    // Hidden layer.
    float* h;
    // Output layer.
    float* o;
    // Number of biases - always two - Tinn only supports a single hidden layer.
    int nb;
    // Number of weights.
    int nw;
    // Number of inputs.
    int nips;
    // Number of hidden neurons.
    int nhid;
    // Number of outputs.
    int nops;
}
Ann;

float* xtpredict(Ann, const float* in);

float xttrain(Ann, const float* in, const float* tg, float rate);

Ann xtbuild(int nips, int nhid, int nops);

void xtsave(Ann, const char* path);

Ann xtload(const char* path);

void xtfree(Ann);

void xtprint(const float* arr, const int size);


/*
#ifndef ANN_H
#define ANN_H

#include<stdio.h>

#ifndef RANDOM
#define RANDOM() (((double)rand())/RAND_MAX)
#endif

struct ann;

typedef double (*ann_actfun)(const struct ann *ann, double a);


// ANN structure of a node 
typedef struct neuralNode{
    int inputs, outputs;

    ann_actfun activation_hidden;
    ann_actfun activation_output;

    double *weights; // array of weights

    double *inputs // array of inputs

    double *outputs // array of outputs

};

//Creates and initialises an ann on a node
ann *ann_init(int inputs, int outputs);

//Sets random weights to each input link
void ann_randomise(ann *ann);

//Frees the memory used by an ann. 
void ann_free(ann *ann);

//Runs the feedforward algorithm to calculate the ann's output. 
double const *ann_run(ann const *ann, double const *inputs);

// Does a single backprop update. 
void ann_train(ann const *ann, double const *inputs, double const *desired_outputs, double learning_rate);
void ann_train_deltas(ann const *ann, double const *inputs, double const *desired_outputs, double learning_rate,double* deltabuf);

// Saves the ann
void ann_write(ann const *ann, FILE *out);

void ann_init_sigmoid_lookup(const ann *ann);
double ann_act_sigmoid(const ann *ann, double a);
double ann_act_sigmoid_cached(const ann *ann, double a);
double ann_act_threshold(const ann *ann, double a);
double ann_act_linear(const ann *ann, double a);

#endif
*/