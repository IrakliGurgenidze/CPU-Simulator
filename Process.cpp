#include "Process.h"

#include <stdio.h>
#include <stdlib.h>
#include <algorithm>
#include <string.h>
#include <queue>
#include <math.h>
#include "Burst.h"
#include "Calculator.h"

/*
Process creation will be done in CPU shell to avoid creaing new Calculator objects
for each process. Value population will be done through helper methods.
*/

//constructor
Process::Process(char _id, int _init_arrival, int _num_bursts, double lambda, double _alpha):  
    num_bursts{_num_bursts}, 
    rem_bursts{_num_bursts},
    rem_switch{0},
    init_arrival{_init_arrival}, 
    burst_start{0},
    turnaround_time{0},
    tau{1.0/lambda},
    alpha{_alpha},
    id{_id}, 
    done_waiting{-1},
    burst_queue()
    {};

//helper method to add burst to process
void Process::newBurst(int burst_time, bool burst_type){
    Burst burst = Burst(burst_time, id, burst_type);
    burst_queue.push(burst);
}

//helper function ro remove frontmost burst from queue
void Process::removeBurst(){
    //if cpu burst, update tau value
    if(!burst_queue.front().isIO()){
        tau = (alpha * burst_queue.front().getInitDuration())
            + ((1-alpha) * tau);
        rem_bursts--;
    }
    burst_queue.pop();
}

//helper method for a program to wait x ms (can either subtract from arrival time or IO burst)
//returns length of wait if the process is ready to be ran
//returns 0 otherwise
// void Process::wait() {
//     queue.front().subDuration();
//     if(!getFrontBurst().isIO()) {
//         fprintf(stderr, "process with no arrival time or IO burst was waited");
//         exit(EXIT_FAILURE);
//     }
// }

//helper method for the running process to be ran for x ms
//returns length of burst completed if burst was run to completion
//returns 0 otherwise
void Process::run() {
    burst_queue.front().subDuration();
    if(getFrontBurst().isIO()) {
        fprintf(stderr, "IO burst was ran or process was not ready");
        exit(EXIT_FAILURE);
    }
}

// void Process::beginBurst() {
//     burst_start = *elapsed_time;
// }

// int Process::finishBurst() {
//     return *elapsed_time - burst_start;
// }

//helper to retrieve frontmost burst on process queue
Burst Process::getFrontBurst(){
    return burst_queue.front();
}

//get init_arrival
int Process::getInitArrival(){return init_arrival;}

//set/get num_bursts
int Process::getNumBursts(){return num_bursts;}

//get rem_bursts
int Process::getRemBursts(){return rem_bursts;}

//get rem_swtich
void Process::setRemSwitch(int _rem_switch){rem_switch = _rem_switch;}
int Process::getRemSwitch(){return rem_switch;}
void Process::subRemSwitch(){
    if(rem_switch != 0) 
        rem_switch--;
    else{
        fprintf(stderr, "think again boyo");
        exit(EXIT_FAILURE);
    }
}

//set/get id
char Process::getID(){return id;}

//set/get tau
int Process::getTau(){
    return ceil(tau);
}

//set/get turnaround_time
int Process::getTurnaroundTime(){return turnaround_time;}
void Process::setTurnaroundTime(int _turnaround_time){turnaround_time = _turnaround_time;}

//get approximate time based on tau and elapsed time
double Process::getApprox(){return getTau() - (getFrontBurst().getInitDuration() - getFrontBurst().getDuration()); }

//set/get alpha 
double Process::getAlpha(){return alpha;}

void Process::setWaitTime(int tim) { done_waiting = tim; }

int Process::getWaitTime() { return done_waiting; };

bool CompareProcessAlpha::operator() (Process* p1, Process* p2){
    return (*p1).getID() < (*p2).getID();
}

CompareTau::CompareTau() {} // constructor

bool CompareTau::operator()(Process* p1, Process* p2) {
    if ((*p1).getApprox() == (*p2).getApprox())
        return (*p1).getID() < (*p2).getID();
    return (*p1).getApprox() < (*p2).getApprox();
}
