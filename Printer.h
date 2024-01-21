#ifndef __PRINTER_H_
#define __PRINTER_H_

#include <set>

#include "Process.h"

typedef std::priority_queue<Process*,std::vector<Process*>,CompareProcessReady> prio_queue;

class Printer {
    private:
        int* elapsed_time;
        int algo;
        prio_queue* queue;
            
    public:
        Printer(int* time, int algo_, prio_queue* q);
        
        //special = 1: will stop printing, includes process marker
        //special = 2: will stop printing, doesn't include process marker
        //special = -1: will not stop printing, includes process marker
        //special = -2: will not stop printing, doesn't include process marker
        void print(char* incident, Process* p, bool persistent, bool print_tau);


        void print(char* incident, bool persistent);




        char* print_queue();

        char* time();

        char* timeProcess(Process* p, bool print_tau);

        int getTime();
};
#endif