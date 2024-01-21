#include "Calculator.h"

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>


//constructor
Calculator::Calculator(int seed_, double lambda_, int ubound_):
    seed{seed_}, lambda{lambda_}, ubound{ubound_}
    { srand48(seed); }

//calculates next exponentially distributed random value
double Calculator::next_exp() {
    double r;
    double x = ubound + 1;
    while(x > ubound) {
        r = drand48();
        x = -log(r)/lambda;
    }
    return x;
}

//calculates floor of next exp value (for arrival time)
int Calculator::getNewArrival() {
    return floor(next_exp());
}
//calculates ceiling of next exp value (for cpu burst time)
int Calculator::getNewBurstTime() {
    return ceil(next_exp());
}
//calculates ceiling of random value from 1-100, normal distribution (for count of cpu bursts)
int Calculator::getNewBurstCount() {
    return ceil(drand48()*100.0);
}

// //recalculates tau based on new sample value
// double reviseTau(double new_sample) {
//     tau = (alpha * new_sample) + ((1-alpha) * tau);
//     return tau;
// }

//resets random number generator for new simulation
void Calculator::resetSeed() {
    srand48(seed);
}
// //resets tau to tau_0 for new simulation
// void resetTau() {
//     tau = 1.0/lambda;
// }
