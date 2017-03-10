#ifndef __BRANCH_PRED_H
#define __BRANCH_PRED_H

#include <cstdint>
#include "Debug.h"
using namespace std;

#ifndef BPRED_SIZE
#define BPRED_SIZE 64
#endif

class BranchPred {
  private:


    struct PredMap
    {
        int satcount;
        int targpc;
    };
    PredMap * pred = new PredMap[BPRED_SIZE];
    int predictions;
    int pred_takens;
    int mispredictions;
    int mispred_direction;
    int mispred_target;


  public:
    int count = 0;
    BranchPred();
    void incmisspred() { mispredictions++; }
    void countpred() { predictions++; }
    void counttaken() { pred_takens++; }
    void countdirmiss() { mispred_direction++; }
    void counttarmiss() { mispred_target++; }
    int predictdirection(uint32_t pc);
    int predicttarget(uint32_t pc);
    void update(uint32_t pc, bool istaken);


    /* Your member functions here */

    void printFinalStats();
};

#endif
