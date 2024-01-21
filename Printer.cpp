#include "Printer.h"

#include <stdio.h>
#include <string.h>
#include <set>
#include <string>
#include "Process.h"

typedef std::priority_queue<Process*,std::vector<Process*>,CompareProcessReady> prio_queue;

Printer::Printer(int* time, int algo_, prio_queue* q): elapsed_time{time}, algo{algo_}, queue{q} {};

//special = 1: will stop printing, includes process marker
//special = 2: will stop printing, doesn't include process marker
//special = -1: will not stop printing, includes process marker
//special = -2: will not stop printing, doesn't include process marker


void Printer::print(char* incident, Process* p, bool persistent, bool print_tau) {
    if(*elapsed_time > 999 && !persistent)
        return;
    char* time = this->timeProcess(p, print_tau);
    char* q = this->print_queue();
    printf("%s %s %s\n", time, incident, q);
    fflush(stdout);
    free(time);
    free(q);
}

void Printer::print(char* incident, bool persistent) {
    if(*elapsed_time > 999 && !persistent)
        return;
    char* time = this->time();
    char* q = this->print_queue();
    printf("%s %s %s\n", time, incident, q);
    fflush(stdout);
    free(time);
    free(q);
}



char* Printer::print_queue() {
    if(queue->size() == 0) {
        char* ans = (char*)calloc(15, sizeof(char));
        strcpy(ans, "[Q empty]");
        return ans;
    }
    
    char* ans = (char*)calloc(15 + queue->size(), sizeof(char));
    char* qeqeqe = (char*)calloc(queue->size()+1, sizeof(char));
    char* hehehe = qeqeqe;
    prio_queue temp_q = prio_queue(*queue);
    while(temp_q.size() != 0) {
        *qeqeqe = temp_q.top()->getID();
        qeqeqe++;
        temp_q.pop();
    }
    sprintf(ans, "[Q %s]", hehehe);
    free(hehehe);
    return ans;
}

char* Printer::time() {
    char* buffer = (char*)calloc(100, sizeof(char));
    sprintf(buffer, "time %dms:", *elapsed_time);
    return buffer;
}

char* Printer::timeProcess(Process* p, bool print_tau) {
    char* buffer = (char*)calloc(100, sizeof(char));
    if(algo == 0 || algo == 3 || !print_tau)    
        sprintf(buffer, "time %dms: Process %c", *elapsed_time, p->getID());
    else
        sprintf(buffer, "time %dms: Process %c (tau %dms)", *elapsed_time, p->getID(), p->getTau());
    return buffer;
}

int Printer::getTime() {
    return *elapsed_time;
}