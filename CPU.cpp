#include "CPU.h"

#include <deque>
#include <set>
#include <stdio.h>
#include <stdlib.h>
#include <algorithm>

#include "Process.h"
#include "Burst.h"
#include "Calculator.h"

CPU::CPU(int algo_index_, int tcs_, int tslice_):
    running{NULL},
    switching{NULL},
    elapsed_time{0},
    stats{simstats()},
    algo_index{algo_index_},
    tcs{tcs_},
    tslice{tslice_},
    cur_slice{0},
    flout{0}
    {};

void CPU::generateProcesses(int n, int seed, int ubound, double lambda, double alpha) {
    Calculator calc = Calculator(seed, lambda, ubound);
    
    //arrival queue should be empty
    if(this->not_arrived.size() != 0){
        fprintf(stderr, "ERORR: Tried generating processes for occupied CPU\n");
        exit(EXIT_FAILURE);
    }

    //generate n processes
    for(int i = 0; i < n; i++) {

        int arrival = calc.getNewArrival();
        int bursts = calc.getNewBurstCount();
        char newid = i+65;
        Process* p = new Process(newid, arrival, bursts, lambda, alpha);
        for(int j = 0; j < bursts - 1; j++) {
            p->newBurst(calc.getNewBurstTime(), false);
            p->newBurst(calc.getNewBurstTime() * 10, true);
        }

        p->newBurst(calc.getNewBurstTime(), false);
        
        not_arrived.insert(p);
    }

}

simstats CPU::simulate() {
    
    while(running != NULL || switching != NULL || not_arrived.size() > 0 || waiting.size() > 0) {

        if(running != NULL)
            run();
        
        switchOut();

        switchFinish();
        if(algo_index == 3 && switching == NULL && running != NULL && cur_slice == tslice) {
            if(ready_queue.size() == 0) {
                if(elapsed_time < 1000) {
                    char* q = this->printQueue();
                    printf("time %dms: Time slice expired; no preemption because ready queue is empty [Q %s]\n", elapsed_time, q);                       
                    free(q);
                }
                cur_slice = 0;
            } else {
                if(elapsed_time < 1000) {
                    char* q = this->printQueue();
                    printf("time %dms: Time slice expired; process %c preempted with %dms to go [Q %s]\n", elapsed_time, running->getID(), running->getFrontBurst().getDuration(), q);   
                    free(q);
                }
                preemptSwitchout();
                stats.preemption_count++;
            }
        }
        wait();
        
        arrivals();

        if(algo_index == 2 && switching == NULL && running != NULL && ready_queue.size() != 0 && ready_queue.front()->getApprox() < running->getApprox()) {
            if(elapsed_time < 1000) {
                char* q = this->printQueue();
                printf("time %dms: Process %c (tau %dms) will preempt %c [Q %s]\n", elapsed_time, ready_queue.front()->getID(), ready_queue.front()->getTau(), running->getID(), q);   
                free(q);
            }
            preemptSwitchout();
            stats.preemption_count++;
        }

        switchIn();

        stats.total_wait+= ready_queue.size();
        elapsed_time++;
    }
    elapsed_time--;
    stats.elapsed_time = elapsed_time;
    return stats;
}

void CPU::run() {
    
    running->run();
    cur_slice++;

    if(running->getFrontBurst().getDuration() == 0) {

        stats.total_burst += running->getFrontBurst().getInitDuration();
        stats.burst_count++;
        
        stats.total_turn += (elapsed_time - running->getTurnaroundTime()) + tcs/2;
        running->setTurnaroundTime(0);

        int temp_tau = running->getTau();
        running->removeBurst();

        char* q = this->printQueue();
        //if running process is to be moved to waiting

        if(running->getRemBursts() > 0) {
            if(elapsed_time < 1000) {
                if(algo_index == 0 || algo_index == 3)
                    printf("time %dms: Process %c completed a CPU burst; %d %s to go [Q %s]\n", elapsed_time, running->getID(), running->getRemBursts(), running->getRemBursts() == 1 ? "burst" : "bursts", q);
                else
                    printf("time %dms: Process %c (tau %dms) completed a CPU burst; %d %s to go [Q %s]\n", elapsed_time, running->getID(), temp_tau, running->getRemBursts(), running->getRemBursts() == 1 ? "burst" : "bursts", q);                
                if(algo_index == 1 || algo_index == 2) 
                    printf("time %dms: Recalculated tau from %dms to %dms for process %c [Q %s]\n", elapsed_time, temp_tau, running->getTau(), running->getID(), q);
            }
            running->setWaitTime(running->getFrontBurst().getDuration()+elapsed_time+(tcs/2));
            if(elapsed_time < 1000) {
                printf("time %dms: Process %c switching out of CPU; will block on I/O until time %dms [Q %s]\n", elapsed_time, running->getID(), running->getWaitTime(), q);
            }
            
        } else  { //if running process is to be terminated
            printf("time %dms: Process %c terminated [Q %s]\n", elapsed_time, running->getID(), q); 
        }  
        free(q);

    } 
}

void CPU::switchIn() {
    
    //moving top of ready to switching
    if(running == NULL && switching == NULL && ready_queue.size() > 0) {
        flout = 1;
        switching = ready_queue.front();
        ready_queue.pop_front();
        switching->setRemSwitch(tcs/2);
    } 
    if(switching != NULL)
        switching->subRemSwitch();

}

void CPU::switchFinish() {
    //if in process of switching from ready to running
    if(switching != NULL && running == NULL && flout == 1) {

        //if done switching and needs to be moved to running
        if(switching->getRemSwitch() == 0) {
            running = switching;
            switching = NULL;
            stats.context_count++;
            flout = 0;
            if(algo_index == 3)
                cur_slice = 0;
            if(elapsed_time < 1000) {
                char* q = this->printQueue();
                if(algo_index == 3 && running->getFrontBurst().getDuration() != running->getFrontBurst().getInitDuration()) 
                    printf("time %dms: Process %c started using the CPU for remaining %dms of %dms burst [Q %s]\n", elapsed_time, running->getID(), running->getFrontBurst().getDuration(), running->getFrontBurst().getInitDuration(), q);
                else if(algo_index == 0 || algo_index == 3)  
                    printf("time %dms: Process %c started using the CPU for %dms burst [Q %s]\n", elapsed_time, running->getID(), running->getFrontBurst().getDuration(), q);
                else {
                    if(running->getFrontBurst().getDuration() != running->getFrontBurst().getInitDuration())
                        printf("time %dms: Process %c (tau %dms) started using the CPU for remaining %dms of %dms burst [Q %s]\n", elapsed_time, running->getID(), running->getTau(), running->getFrontBurst().getDuration(), running->getFrontBurst().getInitDuration(), q);
                    else
                        printf("time %dms: Process %c (tau %dms) started using the CPU for %dms burst [Q %s]\n", elapsed_time, running->getID(), running->getTau(), running->getFrontBurst().getDuration(), q);
                }
                free(q);
            }
            // running->run();
        }
    }
}

void CPU::switchOut() {

    //if switching buffer is occupied, check whether flushing action is necessary
    if(switching != NULL && switching->getRemSwitch() == 0){

        //process terminates
        if(switching->getRemBursts() == 0) {
            delete(switching);
            switching = NULL;
            flout = 0;
        }
    
        //process moves into waiting queue
        else if(flout == -1){
            if(switching->getFrontBurst().isIO()) {
                waiting.insert(switching);
                switching = NULL;
                flout = 0;
            } else {
                ready_queue.push_back(switching);
                if(algo_index == 1 || algo_index == 2)
                    std::sort(ready_queue.begin(), ready_queue.end(), CompareTau());
                switching = NULL;
                flout =0;
            }
        } 
    }

    //buffer is NOT occupied, check whether running process must leave CPU
    else if(running != NULL){
        if(running->getFrontBurst().isIO() || running->getRemBursts() == 0){
            switching = running;
            running = NULL;
            switching->setRemSwitch(tcs/2);
            flout = -1;
        }
    }
    fflush(stdout);
}

void CPU::preemptSwitchout() {
    switching = running;
    running = NULL;
    switching->setRemSwitch(tcs/2);
    flout = -1;
}

void CPU::wait() {
    std::set<Process*>::iterator witr = waiting.begin();
    while(witr != waiting.end()) {
        if((*witr)->getWaitTime() == elapsed_time) {

            Process* p = *witr;

            p->setWaitTime(-1);

            ready_queue.push_back(p);
            if(algo_index == 1 || algo_index == 2)
                std::sort(ready_queue.begin(), ready_queue.end(), CompareTau());
            witr = waiting.erase(witr);
            p->removeBurst();

            p->setTurnaroundTime(elapsed_time);
            stats.wait_count++;

            char* q = this->printQueue();
            if((algo_index == 0 || algo_index == 3) && elapsed_time < 1000)  
                printf("time %dms: Process %c completed I/O; added to ready queue [Q %s]\n", elapsed_time, p->getID(), q);
            else if(algo_index == 2 && switching == NULL && running != NULL && p->getApprox() < running->getApprox()) {
                if(elapsed_time < 1000)
                    printf("time %dms: Process %c (tau %dms) completed I/O; preempting %c [Q %s]\n", elapsed_time, p->getID(), p->getTau(), running->getID(), q);   
                preemptSwitchout();        
                stats.preemption_count++;
            } else if((algo_index == 1 || algo_index == 2) && elapsed_time < 1000) {
                printf("time %dms: Process %c (tau %dms) completed I/O; added to ready queue [Q %s]\n", elapsed_time, p->getID(), p->getTau(), q);            
            } 
            free(q);
        } else {
            witr++;
        }
    }
    fflush(stdout);
}

void CPU::arrivals() {
    std::set<Process*>::iterator aitr = not_arrived.begin();
    while(aitr != not_arrived.end()) {
        if((*aitr)->getInitArrival() == elapsed_time) {
            Process* p = *aitr;
            ready_queue.push_back(p);
            p->setTurnaroundTime(elapsed_time);

            if(algo_index == 1 || algo_index == 2)
                std::sort(ready_queue.begin(), ready_queue.end(), CompareTau());
            aitr = not_arrived.erase(aitr);

            char* q = this->printQueue();
            if((algo_index == 0 || algo_index == 3) && elapsed_time < 1000)  
                printf("time %dms: Process %c arrived; added to ready queue [Q %s]\n", elapsed_time, p->getID(), q);
            else if(algo_index == 2 && switching == NULL && running != NULL && p->getApprox() < running->getApprox()) {
                if(elapsed_time < 1000)
                    printf("time %dms: Process %c (tau %dms) arrived; preempting %c [Q %s]\n", elapsed_time, p->getID(), p->getTau(), running->getID(), q);   
                preemptSwitchout();   
                stats.preemption_count++;
            } else if((algo_index == 1 || algo_index == 2) && elapsed_time < 1000)
                printf("time %dms: Process %c (tau %dms) arrived; added to ready queue [Q %s]\n", elapsed_time, p->getID(), p->getTau(), q);
            
            free(q);
        } else {
            aitr++;
        }
    }
    fflush(stdout);
}

void CPU::printProcesses() {
    std::set<Process*>::iterator aitr = not_arrived.begin();
    while(aitr != not_arrived.end()) {
        if((*aitr)->getNumBursts() == 1)
            printf("Process %c (arrival time %d ms) %d CPU burst (tau %dms)\n", (*aitr)->getID(), (*aitr)->getInitArrival(), (*aitr)->getNumBursts(), (*aitr)->getTau());
        else
            printf("Process %c (arrival time %d ms) %d CPU bursts (tau %dms)\n", (*aitr)->getID(), (*aitr)->getInitArrival(), (*aitr)->getNumBursts(), (*aitr)->getTau());
        aitr++;
    }
}

char* CPU::printQueue() {
    if(ready_queue.size() == 0) {
        char* ans = (char*)calloc(6, sizeof(char));
        sprintf(ans, "empty");
        return ans;
    }
    char* ans = (char*)calloc(ready_queue.size() + 2, sizeof(char));
    char* sans = ans;
    std::deque<Process*>::iterator ritr = ready_queue.begin();
    while(ritr != ready_queue.end()) {
        *ans = (*ritr)->getID();
        ans++;
        ritr++;
    }
    return sans;
}