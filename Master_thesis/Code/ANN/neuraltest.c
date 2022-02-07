#include "ann.h"
#include "dataread.h"
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

// Learns and predicts hand written digits with 98% accuracy.
int main()
{
    // Tinn does not seed the random number generator.
    srand(time(0));
    // Input and output size is harded coded here as machine learning
    // repositories usually don't include the input and output size in the data itself.
    const int nips = 8;
    const int nops = 2;
    // Hyper Parameters.
    // Learning rate is annealed and thus not constant.
    // It can be fine tuned along with the number of hidden layers.
    // Feel free to modify the anneal rate.
    // The number of iterations can be changed for stronger training.
    float rate = 1.0f;
    const int nhid = 6;
    const float anneal = 0.99f;
    const int iterations = 100;
    
    
    // Load the training set.
    const Data data = build("rssitrainingdata.csv", nips, nops);
    // Train, baby, train.
    const Ann ann = xtbuild(nips, nhid, nops);
    for(int i = 0; i < iterations; i++)
    {
        //shuffle(data);
        float error = 0.0f;
        for(int j = 0; j < data.rows; j++)
        {
            const float* const in = data.in[j];
            const float* const tg = data.tg[j];
            error += xttrain(ann, in, tg, rate);
        }
        printf("error %.12f :: learning rate %f\n",
            (double) error,
            (double) rate);
        rate *= anneal;
    }
    // This is how you save the neural network to disk.
    xtsave(ann, "saved.ann");
    xtfree(ann);
    // This is how you load the neural network from disk.
    const Ann loaded = xtload("saved.ann");
    // Now we do a prediction with the neural network we loaded from disk.
    // Ideally, we would also load a testing set to make the prediction with,
    // but for the sake of brevity here we just reuse the training set from earlier.
    // One data set is picked at random (zero index of input and target arrays is enough
    // as they were both shuffled earlier).
    const float* const in = data.in[0];
    const float* const tg = data.tg[0];
    const float* const pd = xtpredict(loaded, in);
    // Prints target.
    xtprint(tg, data.nops);
    // Prints prediction.
    xtprint(pd, data.nops);
    // All done. Let's clean up.
    xtfree(loaded);
    dfree(data);
    
    return 0;
}