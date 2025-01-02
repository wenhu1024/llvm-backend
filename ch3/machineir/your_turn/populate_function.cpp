#include "llvm/CodeGen/GlobalISel/MachineIRBuilder.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include "llvm/CodeGen/MachineFrameInfo.h" // For CreateStackObject.
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineMemOperand.h" // For MachinePointerInfo.
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/CodeGen/Register.h"
#include "llvm/CodeGen/TargetOpcodes.h"     // For INLINEASM.
#include "llvm/CodeGenTypes/LowLevelType.h" // For LLT.
#include "llvm/IR/Function.h"
#include "llvm/IR/InstrTypes.h" // For ICMP_EQ.
#include "llvm/IR/Instructions.h"
#include "llvm/Support/Alignment.h"

using namespace llvm;

// The goal of this function is to build a MachineFunction that
// represents the lowering of the following foo, a C function:
// extern int baz();
// extern void bar(int);
// void foo(int a, int b) {
//   int var = a + b;
//   if (var == 0xFF) {
//     bar(var);
//     var = baz();
//   }
//   bar(var);
// }
//
// The proposed ABI is:
// - 32-bit arguments are passed through registers: w0, w1
// - 32-bit returned values are passed through registers: w0, w1
// w0 and w1 are given as argument of this Function.
//
// The local variable named var is expected to live on the stack.
MachineFunction *populateMachineIR(MachineModuleInfo &MMI, Function &Foo,
                                   Register W0, Register W1) {
  MachineFunction &MF = MMI.getOrCreateMachineFunction(Foo);

  // The type for bool.
  LLT I1 = LLT::scalar(1);
  // The type of var.
  LLT I32 = LLT::scalar(32);

  // To use to create load and store for var.
  MachinePointerInfo PtrInfo;
  Align VarStackAlign(4);

  // The type for the address of var.
  LLT VarAddrLLT = LLT::pointer(/*AddressSpace=*/0, /*SizeInBits=*/64);

  // The stack slot for var.
  int FrameIndex = MF.getFrameInfo().CreateStackObject(32, VarStackAlign,
                                                       /*IsSpillSlot=*/false);

  // TODO: Populate MF.
  MachineBasicBlock *EntryBB=MF.CreateMachineBasicBlock();
  MF.push_back(EntryBB);
  MachineBasicBlock *ThenBB=MF.CreateMachineBasicBlock();
  MF.push_back(ThenBB);
  MachineBasicBlock *ExitBB=MF.CreateMachineBasicBlock();
  MF.push_back(ExitBB);

  EntryBB->addSuccessor(ThenBB);
  EntryBB->addSuccessor(ExitBB);
  ThenBB->addSuccessor(ExitBB);

  MachineIRBuilder MIBuilder(*EntryBB,EntryBB->end());
  Register A=MIBuilder.buildCopy(I32,W0).getReg(0);
  Register B=MIBuilder.buildCopy(I32, W1).getReg(0);
  Register VarStackAddr=MIBuilder.buildFrameIndex(VarAddrLLT,FrameIndex).getReg(0);

  Register ResAdd=MIBuilder.buildAdd(I32,A,B).getReg(0);
  MIBuilder.buildStore(ResAdd,VarStackAddr,PtrInfo, VarStackAlign);

  Register Cst0xFF=MIBuilder.buildConstant(I32,0XFF).getReg(0);
  Register ReloadedVar0=MIBuilder.buildLoad(I32,VarStackAddr,PtrInfo,VarStackAlign).getReg(0);
  Register Cmp=MIBuilder.buildICmp(CmpInst::ICMP_EQ, I1,ReloadedVar0,Cst0xFF).getReg(0);
  MIBuilder.buildBrCond(Cmp, *ThenBB);
  MIBuilder.buildBr(*ExitBB);

  MIBuilder.setInsertPt(*ThenBB, ThenBB->end());
  Register ReloadedVar1 =MIBuilder.buildLoad(I32,VarStackAddr,PtrInfo,VarStackAlign).getReg(0);
  MIBuilder.buildCopy(W0,ReloadedVar1);

  MIBuilder.buildInstr(TargetOpcode::INLINEASM,{},{})
      .addExternalSymbol("bl @bar")
      .addImm(0)
      .addReg(W0, RegState::Implicit);
  MIBuilder.buildInstr(TargetOpcode::INLINEASM,{},{})
      .addExternalSymbol("bl @baz")
      .addImm(0)
      .addReg(W0,RegState::Implicit | RegState::Define);
  Register ResOfBaz=MIBuilder.buildCopy(I32,W0).getReg(0);
  MIBuilder.buildStore(ResOfBaz,VarStackAddr,PtrInfo,VarStackAlign).getReg(0);
  
  MIBuilder.setInsertPt(*ExitBB, ExitBB->end());
  Register ReloadedVar2=MIBuilder.buildLoad(I32,VarStackAddr,PtrInfo,VarStackAlign).getReg(0);
  MIBuilder.buildCopy(W0, ReloadedVar2);

  MIBuilder.buildInstr(TargetOpcode::INLINEASM,{},{})
      .addExternalSymbol("bl @bar")
      .addImm(0)
      .addReg(W0, RegState::Implicit);
  MIBuilder.buildInstr(TargetOpcode::INLINEASM,{},{})
      .addExternalSymbol("ret")
      .addImm(0);
  
  return &MF;
}
