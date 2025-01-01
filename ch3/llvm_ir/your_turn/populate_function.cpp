#include "llvm/ADT/ArrayRef.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"    // For ConstantInt.
#include "llvm/IR/DerivedTypes.h" // For PointerType, FunctionType.
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Value.h"
#include "llvm/Support/Debug.h" // For errs().

#include <memory> // For unique_ptr
#include <type_traits>

using namespace llvm;

// The goal of this function is to build a Module that
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
// The IR for this snippet (at O0) is:
// define void @foo(i32 %arg, i32 %arg1) {
// bb:
//   %i = alloca i32
//   %i2 = alloca i32
//   %i3 = alloca i32
//   store i32 %arg, ptr %i
//   store i32 %arg1, ptr %i2
//   %i4 = load i32, ptr %i
//   %i5 = load i32, ptr %i2
//   %i6 = add i32 %i4, %i5
//   store i32 %i6, ptr %i3
//   %i7 = load i32, ptr %i3
//   %i8 = icmp eq i32 %i7, 255
//   br i1 %i8, label %bb9, label %bb12
//
// bb9:
//   %i10 = load i32, ptr %i3
//   call void @bar(i32 %i10)
//   %i11 = call i32 @baz()
//   store i32 %i11, ptr %i3
//   br label %bb12
//
// bb12:
//   %i13 = load i32, ptr %i3
//   call void @bar(i32 %i13)
//   ret void
// }
//
// declare void @bar(i32)
// declare i32 @baz(...)
std::unique_ptr<Module> myBuildModule(LLVMContext &Ctxt) { 
    Type *Int32Ty = Type::getInt32Ty(Ctxt);
    Type *VoidTy = Type::getVoidTy(Ctxt);
    PointerType *PtrTy = PointerType::get(Ctxt,0);

    std::unique_ptr<Module> MyModule=std::make_unique<Module>("my solution module",Ctxt);

    FunctionType *BazTy=FunctionType::get(Int32Ty,false);
    Function *BazFunc =cast<Function>(MyModule->getOrInsertFunction("baz",BazTy).getCallee());

    FunctionType *BarTy=FunctionType::get(VoidTy,Int32Ty,false);
    Function *BarFunc=cast<Function>(MyModule->getOrInsertFunction("bar",BarTy).getCallee());

    FunctionType *FooTy=FunctionType::get(VoidTy,{Int32Ty,Int32Ty},false);
    Function *FooFunc=cast<Function>(MyModule->getOrInsertFunction("foo",FooTy).getCallee());

    BasicBlock *BB=BasicBlock::Create(Ctxt,"bb",FooFunc);
    BasicBlock *BB9=BasicBlock::Create(Ctxt,"bb9",FooFunc);
    BasicBlock *BB12=BasicBlock::Create(Ctxt,"bb12",FooFunc);

    IRBuilder Builder(BB); 
    Value *I=Builder.CreateAlloca(Int32Ty);
    Value *I1=Builder.CreateAlloca(Int32Ty);
    Value *I2=Builder.CreateAlloca(Int32Ty);

    Value *Arg=FooFunc->getArg(0);
    Value *Arg1=FooFunc->getArg(1);
    Builder.CreateStore(Arg,I);
    Builder.CreateStore(Arg1,I1);

    Value *I3=Builder.CreateLoad(Int32Ty,I);
    Value *I4=Builder.CreateLoad(Int32Ty,I1);
    Value *I5=Builder.CreateAdd(I3, I4);

    Builder.CreateStore(I5,I2);
    Value *I6=Builder.CreateLoad(Int32Ty,I2);

    Value *Cst255=ConstantInt::get(Int32Ty,255);
    Value *I7=Builder.CreateICmpEQ(I6,Cst255);

    Builder.CreateCondBr(I7,BB9,BB12);


    Builder.SetInsertPoint(BB9);
    Value *I8=Builder.CreateLoad(Int32Ty,I2);
    Builder.CreateCall(BarFunc->getFunctionType(),BarFunc,ArrayRef(I8));
    Value *I9=Builder.CreateCall(BazFunc->getFunctionType(),BazFunc);
    Builder.CreateStore(I9, I2);
    Builder.CreateBr(BB12);

    Builder.SetInsertPoint(BB12);
    Value *I10=Builder.CreateLoad(Int32Ty,I2);
    Builder.CreateCall(BarFunc->getFunctionType(),BarFunc,ArrayRef(I10));
    Builder.CreateRetVoid();

    return MyModule; 
}
