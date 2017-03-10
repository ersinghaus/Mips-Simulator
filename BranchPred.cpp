#include <iostream>
#include <iomanip>
#include <cstdint>
#include "BranchPred.h"
using namespace std;

BranchPred::BranchPred() {
  cout << "Branch Predictor Entries: " << BPRED_SIZE << endl;

  for (int i = 0; i < BPRED_SIZE; i++)
  {
      pred[i].satcount = 0;
      pred[i].targpc = -1;
  }


  predictions = 0;
  pred_takens = 0;
  mispredictions = 0;
  mispred_direction = 0;
  mispred_target = 0;
}

int BranchPred::predictdirection(uint32_t pc) //1 for taken, 0 for not taken.
{
    int index = (pc >> 2) % BPRED_SIZE;
    if (pred[index].satcount >= 2) //predict taken
    {
        return 1;
    }
    else                        // predict not taken
    {
        return 0;
    }

}
int BranchPred::predicttarget(uint32_t pc) //returns true if targetpc matches false if it doesnt.
{
    int index = (pc >> 2) % BPRED_SIZE;
    if (pred[index].targpc == pc)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}
void BranchPred::update(uint32_t pc, bool istaken)
{
    int index = (pc >> 2) % BPRED_SIZE;
    if ((istaken) && (pred[index].satcount != 3))
    {
        pred[index].satcount++;
        pred[index].targpc = pc;

    }
    //else if ((!istaken) && (pred[index].satcount != 0))
    if((istaken == false) && pred[index].satcount != 0)
    {
        pred[index].satcount--;
    }

}
/* You should add functions here */

void BranchPred::printFinalStats() {
  int correct = predictions - mispredictions;
  int not_takens = predictions - pred_takens;

  cout << setprecision(1);
  cout << "Branches predicted: " << predictions << endl;
  cout << "  Pred T: " << pred_takens << " ("
       << (100.0 * pred_takens/predictions) << "%)" << endl;
  cout << "  Pred NT: " << not_takens << endl;
  cout << "Mispredictions: " << mispredictions << " ("
       << (100.0 * mispredictions/predictions) << "%)" << endl;
  cout << "  Mispredicted direction: " << mispred_direction << endl;
  cout << "  Mispredicted target: " << mispred_target << endl;
  cout << "Predictor accuracy: " << (100.0 * correct/predictions) << "%" << endl;
}
