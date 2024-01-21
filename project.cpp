#include <stdio.h>
#include <stdlib.h>
#include <unordered_map>
#include <string.h>
#include <iostream>
#include <fstream>
#include <deque>
#include <list>
#include <iterator>
#include <set>
#include <string>
#include <algorithm>
#include <utility>

#include "Calculator.h"
#include "Printer.h"

typedef std::priority_queue<Process*,std::vector<Process*>,CompareTau> prio_queue;

/*
THE PLAN
Phase 1 - understand
Phase 2 - come up with a plan (network at pi lambda)
Phase 3 - grind time
Phase 3.5 - begin fermentation process
Phase 3.65 - notice fuckup
Phase 3.66 - major rewrite
Phase 3.75 - nighttime celebration purchases (acquire party materials @beer zone)
Phase 4 - purchasing flasks
Phase 4.45 - life, liberty, and Bang(tm)
Phase 4.5 - finishing touches
Phase 4.75 - balloon time
Phase 5 - hurl on coasters (mission critical)
Phase 5.5 - reconsider future of project (Depression :/)
Phase 6 - write up
Phase 6.5 - purchase steaks
Phase 6.75 - purchase construction helmet and straws for passive hydration
Phase 7 - grillmeisters
Phase 7.5 - enjoy fine fermented beverages within reason (somewhat)
Phase 8 - grillmeisters 2.0 (beach episode)
Phase 9 - stabilize the Bolivar
Phase ? - Take opsys final
Phase ?.5 - recover from opsys final with the aid of fine fermented beverages
*/

//helper function to move process from running state to switching state
void flushCPU(Process** running, Process** switching, int tcs){
    if(*switching != NULL){
        fprintf(stderr, "ERORR: Tried flushing CPU into an occupied buffer");
        exit(EXIT_FAILURE);
    }

    *switching = *running;
    *running = NULL;

    (*switching)->setRemSwitch(tcs / 2); //outgoing process must spend tcs/2 ms in the buffer
}

Process* flushSwitching(Process** switching, Process** running, std::set<Process*, CompareProcessAlpha>* waiting, prio_queue* ready, Printer* printer){
    if((*switching) == NULL){ //no bueno
        fprintf(stderr, "ERROR: Tried to flush empty switching buffer\n");
        exit(EXIT_FAILURE);
    }

    Process* temp = NULL;

    if((*switching)->getRemSwitch() == 0){ //time in switching queue is up, do something
        if((*switching)->getRemBursts() == 0){ //no more bursts, process complete
            temp = *switching;
            (*switching) = NULL; //flush switching

            if(ready->size() != 0) {
                (*switching) = ready->top();
                (*switching)->setRemSwitch((*switching)->getRemSwitch()+1);
                ready->pop();
            }
        } else if(!(*switching)->getFrontBurst().isIO()){ //process is moving into cpu
            *running = *switching;
            (*running)->run();
            *switching = NULL;
        } else{ //process is leaving cpu
            
            waiting->insert((*switching));
            (*switching) = NULL;

            //need to move next process in ready queue into switching
            if(ready->size() != 0) {
                (*switching) = ready->top();
                (*switching)->setRemSwitch((*switching)->getRemSwitch()+1);
                ready->pop();
            }
        }
    }

    else{ //process must remain in queue at least one tick longer
        (*switching)->subRemSwitch();
    }

    return temp;
}

void enterSwitching(Process** switching, prio_queue* ready, int tcs){
    if(*switching != NULL){ //buffer should be empty before attempting to move process
        fprintf(stderr, "ERROR: Tried moving process into switching while buffer full");
        exit(EXIT_FAILURE);
    } 
    
    if(ready->size() == 0) { //there should be a process in the ready queue
        fprintf(stderr, "ERROR: attempted to enter switching with nothing in ready queue");
        exit(EXIT_FAILURE);
    }
    else{ //there IS a process in the ready queue
        (*switching) = ready->top();
        (*switching)->setRemSwitch(tcs/2);
        ready->pop();
    }
}

int main(int argc, char** argv) {

    if(argc != 8) {
        fprintf(stderr, "ERROR: Correct argument order: ./*.out <n> <seed> <lambda> <ubound> <tcs> <alpha> <tslice>\n");
        return EXIT_FAILURE;
    }

    //number of processes to simulate, max 26
    int n = atoi(*(argv+1));
    if(n <= 0 || n > 26) {
        fprintf(stderr, "ERROR: There must be at least 1 argument and no more than 26 arguments (0 < n < 27)\n");
        return EXIT_FAILURE;
    }

    //seed for randon number generator
    int seed = atoi(*(argv+2));

    //value to determine curve of exponential distribution calculation
    double lambda = atof(*(argv+3));

    /*
    represents upper bound for valid psuedo random
    numbers for exponential distribution
    */
    int ubound = atoi(*(argv+4));

    /*
    time, in milliseconds, that it takes to perform
    context switch
    */
    int tcs = atoi(*(argv+5));
    if(tcs % 2 != 0) {
        fprintf(stderr, "tcs must be an even value");
        exit(EXIT_FAILURE);
    }

    //used for exponential averaging
    double alpha = atof(*(argv+6));
    
    //time slice value for RR in milliseconds
    int tslice = atoi(*(argv+7));

    if((n == 0 && **(argv+1) != '0') || 
       (seed == 0 && **(argv+2) != '0') ||
       (lambda == 0 && **(argv+3) != '0') || 
       (ubound == 0 && **(argv+4) != '0') || 
       (tcs == 0 && **(argv+5) != '0') || 
       (alpha == 0 && **(argv+6) != '0') || 
       (tslice == 0 && **(argv+7) != '0')) {
        fprintf(stderr, "ERROR: Correct argument order: ./*.out <n> <seed> <lambda> <ubound> <tcs> <alpha> <tslice>\n");
        return EXIT_FAILURE;    
    }

    //opening output file
    FILE* outfile = fopen("simout.txt", "w+");

    //object to generate random numbers
    Calculator calc = Calculator(seed, lambda, ubound);

    //for loop that represents going through each scheduling algorithm
    //  0: FCFS
    //  1: SJF
    //  2: SRT
    //  3: RR
    for(int algo_index = 0; algo_index < 2; algo_index++) {
        //variables to keep track of algorithm statistics
        double total_burst = 0; //total cpu burst time -- FIN?
        int burst_count = 0; //number of cpu bursts -- FIN?
        double total_wait = 0; //total time a process waits in the ready queue -- FIN?
        int wait_count = 0; //number of processes that had to wait -- FIN?
        double total_turn = 0; //
        int context_count = 0; //number of context switches -- FIN?
        int preemption_count = 0; //number of preemptions

        calc.resetSeed();

        int elapsed_time = 0;

        //map of PIDs to the processes
        std::unordered_map<char, Process*> processes;

        //object to represent ready queue
        prio_queue ready_queue = prio_queue(CompareTau());

        //represents the burst currently running, initially set to dummy value
        Process* running = NULL;
        Process* switching = NULL;

        //object to represent waiting processes
        std::set<Process*, CompareProcessAlpha> waiting;
        std::set<Process*, CompareProcessAlpha> not_arrived;

        Printer printer = Printer(&elapsed_time, algo_index, &ready_queue);
        char* temp_print = (char*)calloc(200, sizeof(char));

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

            processes.insert(std::pair<char, Process*>(newid, p));
            if(algo_index == 0) {
                printf("Process %c (arrival time %d ms) %d CPU bursts (tau %dms)\n", p->getID(), arrival, bursts, p->getTau());
            }
        }

        printf("\n");
        if(algo_index == 0)
            strcpy(temp_print, "Simulator started for FCFS");
        else if(algo_index == 1)
            strcpy(temp_print, "Simulator started for SJF");
        else if(algo_index == 2)
            strcpy(temp_print, "Simulator started for SRT");
        else
            strcpy(temp_print, "Simulator started for RR");
        printer.print(temp_print, true);

        //variable for round robin slice
        // int cur_slice = tslice;

        //where simulation officially begins
        while(processes.size() > 0) {

            //preemption check (later implementation)
            // printf("\nstarting %dms", elapsed_time);
            fflush(stdout);
            //cpu burst completion
            if(running != NULL){ //if there is a process running
                if(running->getFrontBurst().getDuration() == 0){ //process has completed
                    //add burst time to counter
                    total_burst += running->getFrontBurst().getInitDuration();
                    burst_count++;
                    
                    total_turn += (elapsed_time - running->getTurnaroundTime()) + tcs/2;
                    running->setTurnaroundTime(0);
                    if(running->getRemBursts() > 1) {
                        if(running->getRemBursts() == 2)
                            sprintf(temp_print, "completed a CPU burst; %d burst to go", running->getRemBursts()-1);
                        else
                            sprintf(temp_print, "completed a CPU burst; %d bursts to go", running->getRemBursts()-1);

                        printer.print(temp_print, running, true, true);
                    } else {
                        sprintf(temp_print, "terminated");
                        printer.print(temp_print, running, true, false);
                    }
                    int pretau = running->getTau();
                    running->removeBurst();
                    
                    if(running->getRemBursts() > 0) {
                        if(algo_index == 2 || algo_index == 1) {
                            sprintf(temp_print, "Recalculated tau from %dms to %dms for process %c", pretau, running->getTau(), running->getID());
                            printer.print(temp_print, true);
                        }
                        running->setWaitTime(running->getFrontBurst().getDuration()+elapsed_time+(tcs/2));
                        sprintf(temp_print, "switching out of CPU; will block on I/O until time %dms", running->getWaitTime());
                        printer.print(temp_print, running, true, false);
                        
                    }
                    //remove CPU burst from process burst queue
                    

                    //move process to switching
                    flushCPU(&running, &switching, tcs); //running now = NULL
                    context_count++;
                }
                else{ //process is still running
                    running->run(); //subtract remaining burst time by one tick
                }
            }
            
            if(switching != NULL && switching->getRemSwitch() == 1 && switching->getRemBursts() != 0 && !switching->getFrontBurst().isIO()) {
                sprintf(temp_print, "started using the CPU for %dms burst", running->getFrontBurst().getInitDuration());
                printer.print(temp_print, running, true, true);
            }

            //io burst completion
            std::set<Process*, CompareProcessAlpha>::iterator witr = waiting.begin(); //iterator to traverse waiting set
            while(witr != waiting.end()){ //for each process waiting on IO
                if(!(*witr)->getFrontBurst().isIO()){ //not an IO burst
                    fprintf(stderr, "non-IO burst found in waiting queue");
                    exit(EXIT_FAILURE);
                }
                else if((*witr)->getWaitTime() == -1) {
                    fprintf(stderr, "waiting process does not have a marked completion time");
                    exit(EXIT_FAILURE);
                }
                else if((*witr)->getWaitTime() == elapsed_time){ //IO burst complete
                    Process* p = *witr;
                    witr = waiting.erase(witr);
                    p->removeBurst();
                    ready_queue.push(p);
                    p->setTurnaroundTime(elapsed_time);
                    strcpy(temp_print, "completed I/O; added to ready queue");
                    printer.print(temp_print, p, true, true);
                    wait_count++;
                } 
                // else{ //detract remaining IO burst duration by one tick
                //     (*witr)->wait();
                // }
                else if(witr != waiting.end())
                    witr++; //move on to next process in queue
            }

            //new arrivals
            std::set<Process*, CompareProcessAlpha>::iterator arrival_itr = not_arrived.begin();
            while(arrival_itr != not_arrived.end()) {
                if((*arrival_itr)->getInitArrival() == elapsed_time){
                    Process* p = *arrival_itr;
                    arrival_itr = not_arrived.erase(arrival_itr);
                    ready_queue.push(p);
                    p->setTurnaroundTime(elapsed_time);
                    strcpy(temp_print, "arrived; added to ready queue");
                    printer.print(temp_print, p, true, true);
                } else {
                    arrival_itr++;
                }
            }

            //check if time to move process from ready queue to running
            if(switching == NULL && running == NULL && ready_queue.size() > 0) {
                enterSwitching(&switching, &ready_queue, tcs); 

                //count context switch
            }

            //handle processes in the context switch buffer
            if(switching != NULL){ //there is a process in buffer
                Process* temp = flushSwitching(&switching, &running, &waiting, &ready_queue, &printer);
                if(temp != NULL) {
                    processes.erase(temp->getID());
                    delete(temp);
                }
            }

            //iterate over ready queue to sum turnaroud time
            

            total_wait += ready_queue.size();

            
            elapsed_time++;            
        }
        elapsed_time--;
        
        //output to file for each algorithm
        fprintf(outfile, "Algorithm ");
        switch(algo_index) {
            case 0:
                fprintf(outfile, "FCFS");
            case 1:
                fprintf(outfile, "SJT");
            case 2:
                fprintf(outfile, "SRT");
            case 3:
                fprintf(outfile, "RR");
        }

        fprintf(outfile, "\n-- average CPU burst time: %.3f\n", (1.0*total_burst)/burst_count);
        fprintf(outfile, "-- average wait time: %.3f\n", (1.0*total_wait)/burst_count);
        fprintf(outfile, "-- average turnaround time: %.3f\n", (1.0*total_turn)/burst_count);
        fprintf(outfile, "-- total number of context switches: %d\n", context_count);
        fprintf(outfile, "-- total number of preemtions: %d\n", preemption_count);
        fprintf(outfile, "-- CPU utilization: %.3f%%\n", 100.0*(1.0*total_burst)/elapsed_time);


        if(algo_index == 0)
            strcpy(temp_print, "Simulator ended for FCFS");
        else if(algo_index == 1)
            strcpy(temp_print, "Simulator ended for SJF");
        else if(algo_index == 2)
            strcpy(temp_print, "Simulator ended for SRT");
        else
            strcpy(temp_print, "Simulator ended for RR");
        printer.print(temp_print, true);
        free(temp_print);
    }

    fclose(outfile);
    return EXIT_SUCCESS;
}

