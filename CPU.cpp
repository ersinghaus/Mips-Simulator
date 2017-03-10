/****************************
 * Stephen Ersinghaus
 * CS 3339 Spring 2016
 ****************************/
#include "CPU.h"

Stats s;
CacheStats c;
BranchPred b;

const string CPU::regNames[] = {"$zero","$at","$v0","$v1","$a0","$a1","$a2","$a3",
                                "$t0","$t1","$t2","$t3","$t4","$t5","$t6","$t7",
                                "$s0","$s1","$s2","$s3","$s4","$s5","$s6","$s7",
                                "$t8","$t9","$k0","$k1","$gp","$sp","$fp","$ra"};


CPU::CPU(uint32_t pc, Memory &iMem, Memory &dMem) : pc(pc), iMem(iMem), dMem(dMem) {
  for(int i = 0; i < NREGS; i++) {
    regFile[i] = 0;
  }
  hi = 0;
  lo = 0;
  regFile[28] = 0x10008000; // gp
  regFile[29] = 0x10000000 + dMem.getSize(); // sp

  instructions = 0;
  stop = false;
}

void CPU::run() {
  while(!stop) {
    instructions++;

    fetch();
    decode();
    execute();

    D(printRegFile());
  }
}

void CPU::fetch() {
  instr = iMem.loadWord(pc);
  pc = pc + 4;
}

/////////////////////////////////////////
// ALL YOUR CHANGES GO IN THIS FUNCTION
/////////////////////////////////////////
void CPU::decode() {
  uint32_t opcode = instr >> 26;
  uint32_t rs = (instr >> 21) & 0x1f;
  uint32_t rt = (instr >> 16) & 0x1f;
  uint32_t rd = (instr >> 11) & 0x1f;
  uint32_t shamt = (instr >> 6) & 0x1f;
  uint32_t funct = instr & 0x3f;
  uint32_t uimm = instr & 0xffff;
  int32_t  simm = ((signed)uimm << 16) >> 16;
  uint32_t addr = instr & 0x3ffffff;

  int x;

  opIsLoad=false;
  opIsStore=false;
  opIsMultDiv=false;
  bool istaken = false;
  int predictiondir;
  int predictiontar;
  bool mispredict = false;
  uint32_t predpc;
  D(cout << "  " << hex << setw(8) << pc - 4 << ": ");
  switch(opcode) {
    case 0x00:
      switch(funct) {
        case 0x00: D(cout << "sll " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   aluOp = SHF_L;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = shamt;
                   writeDest = true;
                   destReg = rd;
                   s.registerDest(rd, MEM1);
                   s.registerSrc(rs, EXE1);
                   break;
        case 0x03: D(cout << "sra " << regNames[rd] << ", " << regNames[rs] << ", " << dec << shamt);
                   aluOp = SHF_R;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = shamt;
                   writeDest  = true;
                   destReg = rd;
                   s.registerDest(rd, MEM1);
                   s.registerSrc(rs, EXE1);
                   break;
        case 0x08: D(cout << "jr " << regNames[rs]);
                   aluOp = OUT_S1;
                   pc = regFile[rs];
                   writeDest = false;
                   s.registerSrc(rs, ID);
                   s.flush(ID - IF1);
                   break;
        case 0x10: D(cout << "mfhi " << regNames[rd]);
                   aluOp = OUT_S1;
                   regFile[rd] = hi;
                   writeDest = false;
                   s.registerSrc(REG_HILO, EXE1);
                   s.registerDest(rd, MEM1);
                   break;
        case 0x12: D(cout << "mflo " << regNames[rd]);
                   aluOp = OUT_S1;
                   regFile[rd] = lo;
                   writeDest = false;
                   s.registerSrc(REG_HILO, EXE1);
                   s.registerDest(rd, MEM1);
                   break;
        case 0x18: D(cout << "mult " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true;
                   aluOp = MUL;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   s.registerSrc(rs, EXE1);
                   s.registerSrc(rt, EXE1);
                   writeDest = false;
                   s.registerDest(REG_HILO, WB);
                   break;
        case 0x1a: D(cout << "div " << regNames[rs] << ", " << regNames[rt]);
                   opIsMultDiv = true;
                   aluOp = DIV;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   s.registerSrc(rs, EXE1);
                   s.registerSrc(rt, EXE1);
                   writeDest = false;
                   s.registerDest(REG_HILO, WB);
                   break;
        case 0x21: D(cout << "addu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   s.registerSrc(rs, EXE1);
                   s.registerSrc(rt, EXE1);
                   writeDest  = true;
                   destReg = rd;
                   s.registerDest(rd, MEM1);
                   break;
        case 0x23: D(cout << "subu " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = ADD;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = -regFile[rt];
                   s.registerSrc(rs, EXE1);
                   s.registerSrc(rt, EXE1);
                   writeDest  = true;
                   destReg = rd;
                   s.registerDest(rd, MEM1);
                   break;
        case 0x2a: D(cout << "slt " << regNames[rd] << ", " << regNames[rs] << ", " << regNames[rt]);
                   aluOp = CMP_LT;
                   aluSrc1 = regFile[rs];
                   aluSrc2 = regFile[rt];
                   s.registerSrc(rs, EXE1);
                   s.registerSrc(rt, EXE1);
                   writeDest  = true;
                   destReg = rd;
                   s.registerDest(rd, MEM1);
                   break;
        default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
      }
      break;
    case 0x02: D(cout << "j " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               aluOp = OUT_S1;
               writeDest = false;
               pc = ((pc & 0xf0000000) | addr << 2);
               s.flush(ID - IF1);
               break;
    case 0x03: D(cout << "jal " << hex << ((pc & 0xf0000000) | addr << 2)); // P1: pc + 4
               aluOp = OUT_S1; // pass src1 straight through ALU
               writeDest = true; destReg = REG_RA; // writes PC+4 to $ra
               aluSrc1 = pc;
               pc = (pc & 0xf0000000) | addr << 2;
               s.flush(ID - IF1);
               s.registerDest(REG_RA, EXE1);
               break;
    case 0x04: D(cout << "beq " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               aluOp = OUT_S1;
               s.registerSrc(rs, ID);
               s.registerSrc(rt, ID);
	           predpc = pc +(simm << 2);
               predictiondir = b.predictdirection(predpc);
               predictiontar = b.predicttarget(predpc);
               if (predictiondir == 1)
               {
                   b.counttaken();
               }
               if (regFile[rs] == regFile[rt]) //branch taken
               {
                   pc = (pc + (simm << 2));
                   if ((predictiondir == 1) and (predictiontar == 0)) //if predict taken but target miss
                   {
                       b.counttarmiss(); //cout target miss
                       mispredict = true;
;
                   }
                   else if (predictiondir == 0) //if predict not taken
                   {
                       b.countdirmiss(); //count direction miss
                       mispredict = true;
                   }
                   istaken = true;
               }
               else //branch not taken
               {
                   istaken = false;
                   if (predictiondir == 1)
                   {
                       mispredict = true;
                       b.countdirmiss();
                   }
               }
               if (mispredict)
               {
                   b.incmisspred();
                   s.flush(ID - IF1);
               }
               writeDest = false;
               b.update(pc, istaken);
               b.countpred();
               break;
    case 0x05: D(cout << "bne " << regNames[rs] << ", " << regNames[rt] << ", " << pc + (simm << 2));
               aluOp = OUT_S1;
               s.registerSrc(rs, ID);
               s.registerSrc(rt, ID);
	           predpc = pc +(simm << 2);
               predictiondir = b.predictdirection(predpc);
               predictiontar = b.predicttarget(predpc);
               if (predictiondir == true)
               {
                   b.counttaken();
               }
               if (regFile[rs] != regFile[rt])
               {
                   pc = (pc + (simm << 2));
                   if ((predictiondir == 1) and (predictiontar == 0)) //if predict not taken and predicted target != target
                   {
                       b.counttarmiss(); //count target miss
                       mispredict = true;
;
                   }
                   else if (predictiondir == 0) //if predict not taken
                   {
                       b.countdirmiss(); //count direction miss
                       mispredict = true;
                   }
                   istaken = true;
               }
               else
               {
                   istaken = false;
                   if (predictiondir == 1) // if branch not taken, but predicted taken
                   {
                       mispredict = true;
                       b.countdirmiss();
                   }
               }
               if (mispredict) //if mispredicted, flush
               {
                   b.incmisspred();
                   s.flush(ID - IF1);
               }
               writeDest = false;
               b.update(pc, istaken);
               b.countpred();
               break;
    case 0x08: D(cout << "addi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               aluOp = ADD;
               aluSrc1 = regFile[rs];
               s.registerSrc(rs, EXE1);
               aluSrc2 = simm;
               writeDest = true;
               destReg = rt;
               s.registerDest(rt, MEM1);
               break;
    case 0x09: D(cout << "addiu " << regNames[rt] << ", " << regNames[rs] << ", " << dec << simm);
               aluOp = ADD;
               aluSrc1 = regFile[rs];
               s.registerSrc(rs, EXE1);
               aluSrc2 = simm;
               writeDest = true;
               destReg = rt;
               s.registerDest(rt, MEM1);
               break;
    case 0x0c: D(cout << "andi " << regNames[rt] << ", " << regNames[rs] << ", " << dec << uimm);
               aluOp = AND;
               aluSrc1 = regFile[rs];
               s.registerSrc(rs, EXE1);
               aluSrc2 = uimm;
               writeDest = true;
               destReg = rt;
               s.registerDest(rt, MEM1);
               break;
    case 0x0f: D(cout << "lui " << regNames[rt] << ", " << dec << simm);
               aluOp = ADD;
               aluSrc1 = simm << 16;
               aluSrc2 = 0;
               writeDest = true;
               destReg = rt;
               s.registerDest(rt, MEM1);
               break;
    case 0x1a: D(cout << "trap " << hex << addr);
               aluOp = OUT_S1; // don't need the ALU
               writeDest = false;
               switch(addr & 0xf) {
                 case 0x0: cout << endl; break;
                 case 0x1: cout << " " << (signed)regFile[rs]; s.registerSrc(rs, EXE1); break;
                 case 0x5: cout << endl << "? "; cin >> regFile[rt]; s.registerDest(rt, MEM1); break;
                 case 0xa: stop = true; break;
                 default: cerr << "unimplemented trap: pc = 0x" << hex << pc - 4 << endl;
                          stop = true;
               }
               break;
    case 0x23: D(cout << "lw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
               aluOp = ADD;
               aluSrc1 = regFile[rs];
               s.registerSrc(rs, EXE1);
               aluSrc2 = simm;
               opIsLoad = true;
               writeDest = true;
               destReg = rt;
               s.registerDest(rt, WB);
               s.countMemOp();
               x = c.access(addr, LOAD);
               break;
    case 0x2b: D(cout << "sw " << regNames[rt] << ", " << dec << simm << "(" << regNames[rs] << ")");
                 aluSrc1 = regFile[rs];
                 s.registerSrc(rs, EXE1);
                 aluSrc2 = simm;
                 aluOp = ADD;
                 opIsStore = true;
                 writeDest = false;
                 storeData = regFile[rt];
                 s.registerSrc(rt, MEM1);
                 s.countMemOp();
                 x = c.access(addr, STORE);
               break;
    default: cerr << "unimplemented instruction: pc = 0x" << hex << pc - 4 << endl;
  }
  D(cout << endl);
  s.clock(IF1);
  s.stall(x);

}

void CPU::execute() {
  uint32_t aluOut = alu.op(aluOp, aluSrc1, aluSrc2);

  if(opIsLoad)
    aluOut = dMem.loadWord(aluOut);
  else if(opIsStore)
    dMem.storeWord(storeData, aluOut);
  else if(opIsMultDiv) {
    hi = alu.getUpper();
    lo = alu.getLower();
  }

  // Regfile update (but never write to register 0)
  if(writeDest && destReg > 0)
    regFile[destReg] = aluOut;
}

void CPU::printRegFile() {
  cout << hex;
  for(int i = 0; i < NREGS; i++) {
    cout << "    " << regNames[i];
    if(i > 0) cout << "  ";
    cout << ": " << setfill('0') << setw(8) << regFile[i];
    if( i == (NREGS - 1) || (i + 1) % 4 == 0 )
      cout << endl;
  }
  cout << "    hi   : " << setfill('0') << setw(8) << hi;
  cout << "    lo   : " << setfill('0') << setw(8) << lo;
  cout << dec << endl;
}

void CPU::printFinalStats() {
  cout << "Program finished at pc = 0x" << hex << pc << "  ("
       << dec << instructions << " instructions executed)" << endl;

  cout << "Cycles: " << s.getCycles() << endl << "CPI: " << fixed << setprecision(1) << ((float)s.getCycles() / instructions)
       << endl << endl << "Bubbles: " << s.getBubbles() << endl << "Flushes: " << s.getFlushes() << "\nStalls: " << /*c.getStalls()*/0 << "\n\n";
  b.printFinalStats();
  //c.printFinalStats();
//  cout << "Mem ops: ";
//  cout << fixed << setprecision(1) << 100.0 * s.getMemOps() / instructions << " of instructions" << endl;
//
//  cout << "Branches: ";
//  cout << fixed << setprecision(1) << 100.0 * s.getBranches() / instructions << " of instructions" << endl;
//
//  cout << "% taken: ";
//  cout << fixed << setprecision(1) << 100.0 * s.getTaken() / s.getBranches() << " of instructions" << endl;
//
//  cout << "RAW Hazards: " << s.getTotal() << " (1 per every " << fixed << setprecision(2) << (instructions / (float)s.getTotal())
//       << " instructions)" << endl;
//
//  cout << "   On EXE1 op: ";
//  cout << s.getEXE1() << " (" << fixed << setprecision(0) << 100 * (float)s.getEXE1() / s.getTotal() << "%)" << endl;
//
//  cout << "   On EXE2 op: ";
//  cout << s.getEXE2() << " (" << fixed << setprecision(0) << 100 * (float)s.getEXE2() / s.getTotal() << "%)" << endl;
//
//  cout << "   On MEM1 op: ";
//  cout << s.getMEM1() << " (" << fixed << setprecision(0) << 100 * (float)s.getMEM1() / s.getTotal() << "%)" << endl;
//
//  cout << "   On MEM2 op: ";
//  cout << s.getMEM2() << " (" << fixed << setprecision(0) << 100 * (float)s.getMEM2() / s.getTotal() << "%)" << endl;

}
