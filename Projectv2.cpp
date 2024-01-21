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
#include "CPU.h"

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

    int seed = atoi(*(argv+2));
    double lambda = atof(*(argv+3));
    int ubound = atoi(*(argv+4));
    int tcs = atoi(*(argv+5));
    if(tcs % 2 != 0) {
        fprintf(stderr, "tcs must be an even value");
        exit(EXIT_FAILURE);
    }
    double alpha = atof(*(argv+6));
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

    //for loop that represents going through each scheduling algorithm
    //  0: FCFS
    //  1: SJF
    //  2: SRT
    //  3: RR
    for(int algo_index = 0; algo_index < 4; algo_index++) {

        CPU cpu = CPU(algo_index, tcs, tslice);

        cpu.generateProcesses(n, seed, ubound, lambda, alpha);

        if(algo_index == 0)
            cpu.printProcesses();

        if(algo_index == 0)
            printf("\ntime 0ms: Simulator started for FCFS [Q empty]\n");
        else if(algo_index == 1)
            printf("\ntime 0ms: Simulator started for SJF [Q empty]\n");
        else if(algo_index == 2)
            printf("\ntime 0ms: Simulator started for SRT [Q empty]\n");
        else
            printf("\ntime 0ms: Simulator started for RR with time slice %dms [Q empty]\n", tslice);
        
        simstats stats = cpu.simulate();
        
        //output to file for each algorithm
        fprintf(outfile, "Algorithm ");
        switch(algo_index) {
            case 0:
                fprintf(outfile, "FCFS");
                break;
            case 1:
                fprintf(outfile, "SJF");
                break;
            case 2:
                fprintf(outfile, "SRT");
                break;
            case 3:
                fprintf(outfile, "RR");
                break;
        }

        fprintf(outfile, "\n-- average CPU burst time: %.3f ms\n", (1.0*stats.total_burst)/stats.burst_count);
        fprintf(outfile, "-- average wait time: %.3f ms\n", (1.0*stats.total_wait)/stats.burst_count);
        fprintf(outfile, "-- average turnaround time: %.3f ms\n", (1.0*stats.total_turn)/stats.burst_count);
        fprintf(outfile, "-- total number of context switches: %d\n", stats.context_count);
        fprintf(outfile, "-- total number of preemptions: %d\n", stats.preemption_count);
        fprintf(outfile, "-- CPU utilization: %.3f%%\n", 100.0*(1.0*stats.total_burst)/stats.elapsed_time);


        if(algo_index == 0)
            printf("time %dms: Simulator ended for FCFS [Q empty]\n", stats.elapsed_time);
        else if(algo_index == 1)
            printf("time %dms: Simulator ended for SJF [Q empty]\n", stats.elapsed_time);
        else if(algo_index == 2)
            printf("time %dms: Simulator ended for SRT [Q empty]\n", stats.elapsed_time);
        else
            printf("time %dms: Simulator ended for RR [Q empty]\n", stats.elapsed_time);
    }

    fclose(outfile);
    return EXIT_SUCCESS;
}

