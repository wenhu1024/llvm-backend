#ifndef YOURTURN_PASSWITHNEWPM_H
#define YOURTURN_PASSWITHNEWPM_H
#include "llvm/IR/Analysis.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/PassManager.h" // For PassInfoMixin.

namespace llvm {
class Function;
};

class YourTurnConstantPropagationNewPass:public llvm::PassInfoMixin<YourTurnConstantPropagationNewPass> /* TODO: Fill in the blanks*/ {
public:
    llvm::PreservedAnalyses run(llvm::Function &F,llvm::FunctionAnalysisManager &AM);
};

#define YOUR_TURN_IS_READY 1
#endif
