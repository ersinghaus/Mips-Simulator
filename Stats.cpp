#include "Stats.h"

Stats::Stats() {
  cycles = PIPESTAGES - 1; // pipeline startup cost
  flushes = 0;
  bubbles = 0;

  memops = 0;
  branches = 0;
  taken = 0;

  for(int i = IF1; i < PIPESTAGES; i++) {
    resultReg[i] = -1;
    resultStage[i] = -1;
  }
}

void Stats::clock(PIPESTAGE stage) {
  cycles++;

  // pipeline the register tracking from provided start stage
  // (ops in 'stage' thru end of pipe advance, ops in stages before 'stage'
  // are frozen)
  for(int i = WB; i > stage; i--) {
    resultReg[i] = resultReg[i-1];
    resultStage[i] = resultStage[i-1];
  }
  // inject no-op into 'stage'
  resultReg[stage] = -1;
  resultStage[stage] = -1;
}

void Stats::registerSrc(int r, int needed) {

    if (r == 0)
        return;
    for (int i = EXE1; i < WB; i++)
    {
        if(r == resultReg[i])
        {
            int number = (needed - ID);
            int ready = resultStage[i] - i;
            while (ready > number)
            {
                bubble();
                ready--;
            }
            if (i == EXE1)
                exeone++;
            else if (i == EXE2)
                exetwo++;
            else if (i == MEM1)
                memone++;
            else if (i == MEM2)
                memtwo++;
            break;
        }
     }
}

void Stats::registerDest(int r, int avail) {
    resultReg[ID] = r;
    resultStage[ID] = avail;
    available = avail;

}

void Stats::flush(int count) { // count == how many ops to flush
    for (int i = 0; i < count; i++)
    {
        flushes++;
        clock(IF1);
    }
}

void Stats::bubble() {
    bubbles++;
    clock(EXE1);
}

void Stats::stall(int numstalls)
{
    cycles += numstalls;
}
