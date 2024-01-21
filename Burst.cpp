#include "Burst.h"

#include <stdio.h>
#include <stdlib.h>


//constructor
Burst::Burst(): init_duration{0}, duration{0}, ID{'-'}, IO{false} {}
Burst::Burst(int duration_, char ID_, bool isIO_): 
    init_duration{duration_}, duration{duration_}, ID{ID_}, IO{isIO_} {}

//accessors
bool Burst::isIO() { return IO; }
int Burst::getInitDuration() { return init_duration; }
int Burst::getDuration() { return duration; }   
char Burst::getID() { return ID; }

//mutators
void Burst::subDuration() {            
    duration--;
}
