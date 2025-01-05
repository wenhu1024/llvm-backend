#include "llvm/IR/CallingConv.h"
#include "llvm/IR/Function.h"
#include "llvm/Pass.h"          // For FunctionPass & INITIALIZE_PASS.
#include "llvm/PassRegistry.h"
#include "llvm/Support/Debug.h" // For errs().

using namespace llvm;

extern bool solutionConstantPropagation(llvm::Function &);

// The implementation of this function is generated at the end of this file. See
// INITIALIZE_PASS.
namespace llvm {
void initializeYourTurnConstantPropagationPass(PassRegistry &);
};

namespace {
class YourTurnConstantPropagation:public FunctionPass  {
public:
  YourTurnConstantPropagation():FunctionPass(ID) 
  {
    initializeYourTurnConstantPropagationPass(*PassRegistry::getPassRegistry());
  }

  static char ID;

  bool runOnFunction(Function &F) override{
    errs() << "my Solution Legacy called on " << F.getName() << '\n';
    return solutionConstantPropagation(F);
  }
};
} // End anonymous namespace.

INITIALIZE_PASS(/*passImplementationName=*/YourTurnConstantPropagation,
                /*commandLineArgName=*/"legacy-solution",
                /*name=*/"Legacy Solution", /*isCFGOnly=*/false,
                /*isAnalysis=*/false);

char YourTurnConstantPropagation::ID=0;

Pass *createYourTurnPassForLegacyPM() {
  return new YourTurnConstantPropagation();
}
