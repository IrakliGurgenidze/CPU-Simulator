#ifndef __BURST_H_
#define __BURST_H_

class Burst  {
    private:
        int init_duration; //initial burst duration
        int duration; //duration remaining
        char ID; //ID of the process this burst belongs to
        bool IO; //true if the burst is an IO burst, false if CPU burst
    
    public:
        //constructor
        Burst();
        Burst(int duration_, char ID_, bool isIO_);

        //accessors
        bool isIO();
        int getInitDuration();
        int getDuration();
        char getID();

        //mutators
        void subDuration();
};

#endif