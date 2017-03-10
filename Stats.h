#ifndef __STATS_H
#define __STATS_H
#include <iostream>
#include "Debug.h"
using namespace std;

enum PIPESTAGE { IF1 = 0, IF2 = 1, ID = 2, EXE1 = 3, EXE2 = 4, MEM1 = 5,
                 MEM2 = 6, WB = 7, PIPESTAGES = 8 };

class Stats {
  private:
    long long cycles;
    int flushes;
    int bubbles;
    int scycles;

    int memops;
    int branches;
    int taken;
    int available;
    int exeone;
    int exetwo;
    int memone;
    int memtwo;

    int resultReg[PIPESTAGES];
    int resultStage[PIPESTAGES];

  public:
    Stats();

    void clock(PIPESTAGE stage);

    void flush(int count);

    void registerSrc(int r, int needed);
    void registerDest(int r, int avail);
    void stall(int stalls);

    void countMemOp() { memops++; }
    void countBranch() { branches++; }
    void countTaken() { taken++; }


    // getters
    long long getCycles() { return cycles; }
    int getFlushes() { return flushes; }
    int getBubbles() { return bubbles; }
    int getMemOps() { return memops; }
    int getBranches() { return branches; }
    int getTaken() { return taken; }
    int getEXE1() { return exeone; }
    int getEXE2() { return exetwo; }
    int getMEM1() { return memone; }
    int getMEM2() { return memtwo; }
    int getTotal() { return (exeone + exetwo + memone + memtwo); }
    int getscycles() {return scycles; }

  private:
    void bubble();
};

#endif
