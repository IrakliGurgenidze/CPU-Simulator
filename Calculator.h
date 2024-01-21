#ifndef __CALCULATOR_H_
#define __CALCULATOR_H_

class Calculator {
    private:
        
        int seed; 
        double lambda;
        int ubound;
        // double tau;
    
    public:
        //constructor
        Calculator(int seed_, double lambda_, int ubound_);
        
        //calculates next exponentially distributed random value
        double next_exp();

        //calculates floor of next exp value (for arrival time)
        int getNewArrival();
        //calculates ceiling of next exp value (for cpu burst time)
        int getNewBurstTime();
        //calculates ceiling of random value from 1-100, normal distribution (for count of cpu bursts)
        int getNewBurstCount();

        // //recalculates tau based on new sample value
        // double reviseTau(double new_sample) {
        //     tau = (alpha * new_sample) + ((1-alpha) * tau);
        //     return tau;
        // }
        
        //resets random number generator for new simulation
        void resetSeed();
        // //resets tau to tau_0 for new simulation
        // void resetTau() {
        //     tau = 1.0/lambda;
        // }
};

#endif