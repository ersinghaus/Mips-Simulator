#include <iostream>
#include <cstdlib>
#include <iomanip>
#include <cmath>
#include "CacheStats.h"
using namespace std;

CacheStats::CacheStats() {
  cout << "Cache Config: ";
  if(!CACHE_EN) {
    cout << "cache disabled" << endl;
  } else {
    cout << (SETS * WAYS * BLOCKSIZE) << " B (";
    cout << BLOCKSIZE << " bytes/block, " << SETS << " sets, " << WAYS << " ways)" << endl;
    cout << "  Latencies: Hit = " << HIT_LATENCY << " cycles, ";
    cout << "Read = " << READ_LATENCY << " cycles, ";
    cout << "Write = " << WRITE_LATENCY << " cycles" << endl;
  }

  reads = 0;
  writes = 0;
  read_misses = 0;
  write_misses = 0;
  writebacks = 0;

  /* TODO: your code here */


    for (int i = 0; i < SETS; i++)
  {
      for (int j = 0; j < WAYS; j++)
      {
          cset[i].eindex[j] = -1;
          cset[i].evalid[j] = false;
          cset[i].edirty[j] = false;
          cset[i].eindex[j] = -1;
      }
  }
}


int CacheStats::access(uint32_t addr, ACCESS_TYPE type) {
  if(!CACHE_EN) { // no cache
    return (type == LOAD) ? READ_LATENCY : WRITE_LATENCY;
  }
  if (type == LOAD)
    reads++;
  else
    writes++;
  setnum = (addr >> 5) % 8;

  index = (addr >> 5) & 0x5;
  tag = (addr >> 10) & 0x16;
  offset = (addr >> 2) & 0x3;
  int entries = cset[setnum].entries;
  bool miss = false;



  for (int i = 0; i < 4; i++)
  {
      if (cset[setnum].eindex[i] == index)
      {
          if (cset[setnum].etag[i] == tag)
          {
              return 0;//hit
          }

          else if (cset[setnum].etag[i] != tag) //miss
          {
              miss = true;
          }
      }
      else if (cset[setnum].eindex[i] != index)//miss
      {
          miss = true;
      }
  }
  if (miss == true)
  {
      if (type == LOAD)
         read_misses++;
      else if(type == STORE)
         write_misses++;
  }
  if (cset[setnum].entries == 0)
  {
      cset[setnum].eindex[0] == index;
      cset[setnum].evalid[0] == true;
      cset[setnum].edirty[0] == true;
      cset[setnum].etag[0] == tag;
      cset[setnum].entries++;
  }
  else if (cset[setnum].entries == 4)
  {
      writebacks++;
      for (int i = 4; i > 1; i--)
      {
          cset[setnum].eindex[i] = cset[setnum].eindex[i-1];
          cset[setnum].evalid[i] = cset[setnum].evalid[i-1];
          cset[setnum].edirty[i] = cset[setnum].edirty[i-1];
          cset[setnum].etag[i] = cset[setnum].etag[i-1];
      }
      cset[setnum].eindex[0] == index;
      cset[setnum].evalid[0] == true;
      cset[setnum].edirty[0] == true;
      cset[setnum].etag[0] == tag;
  }
  else
  {
      cset[setnum].eindex[entries+1] == index;
      cset[setnum].evalid[entries+1] == true;
      cset[setnum].edirty[entries+1] == true;
      cset[setnum].etag[entries+1] == tag;
      cset[setnum].entries++;
  }

  return (read_misses * READ_LATENCY) + (write_misses * WRITE_LATENCY);



  /* TODO: your code here */
}

void CacheStats::printFinalStats() {
  /* TODO: your code here (don't forget to drain the cache of writebacks) */

  int dirtycount;
  for (int i = 0; i < SETS; i++)
  {
      for (int j = 0; j < WAYS; j++)
      {
          if (cset[i].edirty[j] == true)
            dirtycount++;
      }
  }
  writebacks += dirtycount;



  int accesses = reads + writes;
  int misses = read_misses + write_misses;
  cout << "Accesses: " << accesses << endl;
  cout << "  Reads: " << reads << endl;
  cout << "  Writes: " << writes << endl;
  cout << "Misses: " << misses << endl;
  cout << "  Read misses: " << read_misses << endl;
  cout << "  Write misses: " << write_misses << endl;
  cout << "Writebacks: " << writebacks << endl;
  cout << "Hit Ratio: " << fixed << setprecision(1) << 100.0 * (accesses - misses) / accesses;
  cout << "%" << endl;
}
