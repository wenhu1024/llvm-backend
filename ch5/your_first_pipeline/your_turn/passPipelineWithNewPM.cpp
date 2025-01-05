#include "llvm/CodeGen/RegisterBank.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/PassInstrumentation.h"
#include "llvm/IR/PassManager.h" // For the new PassManager.
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/StandardInstrumentations.h"
#include "llvm/Transforms/IPO/AlwaysInliner.h"
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Transforms/Utils/Mem2Reg.h"

using namespace llvm;

void runYourTurnPassPipelineForNewPM(Module &MyModule) {
    LLVMContext &Context = MyModule.getContext();

    FunctionAnalysisManager FAM;
    ModuleAnalysisManager MAM;

    PassInstrumentationCallbacks PIC;
    PrintPassOptions PrintPassOpts;
    PrintPassOpts.Verbose=true;
    PrintPassOpts.SkipAnalyses=false;
    PrintPassOpts.Indent=true;

    StandardInstrumentations SI(Context,true,false,PrintPassOpts);
    SI.registerCallbacks(PIC,&MAM);

    MAM.registerPass([&]{return PassInstrumentationAnalysis(&PIC);});
    FAM.registerPass([&]{return PassInstrumentationAnalysis(&PIC);});

    MAM.registerPass([&]{return FunctionAnalysisManagerModuleProxy(FAM);});
    FAM.registerPass([&]{return ModuleAnalysisManagerFunctionProxy(MAM);});

    ModulePassManager NewPM;
    FunctionPassManager FPMgr;

    PassBuilder PB;
    PB.registerFunctionAnalyses(FAM);
    PB.registerModuleAnalyses(MAM);

#ifndef USE_MODULE_MGR
    FPMgr.addPass(PromotePass());
    FPMgr.addPass(InstCombinePass());
#else   
    NewPM.addPass(createModuleToFunctionPassAdaptor(PromotePass()));
    NewPM.addPass(createModuleToFunctionPassAdaptor(InstCombinePass()));
#endif
    NewPM.addPass(createModuleToFunctionPassAdaptor(std::move(FPMgr)));
    NewPM.addPass(AlwaysInlinerPass());
    NewPM.run(MyModule,MAM);
}
