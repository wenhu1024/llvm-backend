#ifndef __YOURTURNREGISTERINFOS_H__
#define __YOURTURNREGISTERINFOS_H__

#include "llvm/ADT/BitVector.h"
#include "llvm/CodeGen/MachineBasicBlock.h"
#include <cstdint>

#define GET_REGINFO_HEADER
#define GET_REGINFO_ENUM // Technically this one belongs to MC.
#include "YourTurnGenRegisterInfo.inc"

namespace llvm {
class MachineFunction;

class YourTurnRegisterInfo : public YourTurnGenRegisterInfo {
public:
  YourTurnRegisterInfo() : YourTurnGenRegisterInfo(Register()) {}

  BitVector getReservedRegs(const MachineFunction &MF) const override {
    return BitVector();
  }

  const MCPhysReg *
  getCalleeSavedRegs(const MachineFunction *MF) const override {
    return nullptr;
  }

  bool eliminateFrameIndex(MachineBasicBlock::iterator II, int SPAdj,
                           unsigned FIOperandNum,
                           RegScavenger *RS = nullptr) const override {
    return false;
  }

  Register getFrameRegister(const MachineFunction &MF) const override {
    return Register();
  }
};

} // end namespace llvm.
#endif
