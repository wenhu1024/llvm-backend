#include "llvm/IR/Analysis.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Support/Debug.h" // For errs().

#include "passWithNewPM.h"

using namespace llvm;

extern bool solutionConstantPropagation(llvm::Function &);

PreservedAnalyses YourTurnConstantPropagationNewPass::run(Function &F,FunctionAnalysisManager& AM){
    errs() << "my Solution New PM on " << F.getName() << "\n";
    bool MadeChanges=solutionConstantPropagation(F);
    if(!MadeChanges){
        return PreservedAnalyses::all();
    }

    PreservedAnalyses PA;
    PA.preserveSet<CFGAnalyses>();
    return PA;
}
