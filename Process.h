#ifndef __PROCESS_H_
#define __PROCESS_H_

#include <queue>
#include "Burst.h"

/*
Process creation will be done in CPU shell to avoid creaing new Calculator objects
for each process. Value population will be done through helper methods.
*/

class Process  {
    private:
        int num_bursts; //number of CPU bursts
        int rem_bursts; //number of remaining CPU bursts

        int rem_switch; //remaining time for process to complete context switch

        int init_arrival; //initial process arrival time

        int burst_start;

        int turnaround_time;

        double tau; //estimated CPU burst time
        double alpha; //constant used for exponential averaging

        char id; //process ID (alpha A-Z)

        int done_waiting;

    public:
        std::queue<Burst> burst_queue; //queue of all process burst times

        //constructor
        Process(char _id, int _init_arrival, int _num_bursts, double lambda, double _alpha);

        //helper method to add burst to process
        void newBurst(int burst_time, bool burst_type);

        //helper function ro remove frontmost burst from queue
        void removeBurst();

        //helper method for the running process to be ran for x ms
        void run();

        //helper to retrieve frontmost burst on process queue
        Burst getFrontBurst();

        //get init_arrival
        int getInitArrival();

        //set/get num_bursts
        int getNumBursts();

        //get rem_bursts
        int getRemBursts();

        //get rem_swtich
        void setRemSwitch(int _rem_switch);
        int getRemSwitch();
        void subRemSwitch();

        //set/get id
        char getID();

        //set/get tau
        int getTau();

        //set/get turnaround_time
        int getTurnaroundTime();
        void setTurnaroundTime(int _turnaround_time);

        //get approximate time based on tau and elapsed time
        double getApprox();

        //set/get alpha 
        double getAlpha();
        
        void setWaitTime(int tim);
        int getWaitTime();
};


class CompareProcessAlpha {
    public:
        bool operator() (Process* p1, Process* p2);
};

struct CompareTau
{
    CompareTau();// constructor

    bool operator()(Process* p1, Process* p2);
};

#endif
