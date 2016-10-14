#include "llvm/IR/DerivedTypes.h"   //for types
#include "llvm/IR/IRBuilder.h"      //for Builder
#include "llvm/IR/LLVMContext.h"    //for Context
#include "llvm/IR/Module.h"         //for Module
#include "llvm/IR/Verifier.h"       //for Verify function
#include "llvm/ExecutionEngine/ExecutionEngine.h" //for JIT
#include "llvm/ExecutionEngine/GenericValue.h"    //for GenericValue
#include "llvm/ExecutionEngine/MCJIT.h"

#include <iostream>
#include <vector>

int main() {
    llvm::LLVMContext &Context = llvm::getGlobalContext();
    llvm::Module *OurModule;
    llvm::IRBuilder<> Builder(Context);

    std::unique_ptr<llvm::Module> Owner = llvm::make_unique<llvm::Module>("Our first intermediary code", Context);
    OurModule = Owner.get();

    /***** Main: a function that gets no parameters and returns an integer *****/
    /* http://llvm.org/docs/doxygen/html/classllvm_1_1FunctionType.html
     * static FunctionType * get (Type *Result, bool isVarArg)
     * FunctionType::get - Create a FunctionType taking no parameters. */
    llvm::Type* intType = llvm::Type::getInt64Ty(Context);
    llvm::FunctionType* typeOfMain = llvm::FunctionType::get(intType,false);

    /* http://llvm.org/docs/doxygen/html/classllvm_1_1Function.html
     * static Function * Create (FunctionType *Ty, LinkageTypes Linkage, const Twine &N="", Module *M=nullptr) */
    llvm::Function* ourMain = llvm::Function::Create(typeOfMain,llvm::Function::ExternalLinkage, "main", OurModule);

    /* http://llvm.org/docs/doxygen/html/classllvm_1_1BasicBlock.html
     * static BasicBlock * Create (LLVMContext &Context, const Twine &Name="", Function *Parent=nullptr, BasicBlock *InsertBefore=nullptr)
     * Creates a new BasicBlock.
     */
    llvm::BasicBlock *mainBB = llvm::BasicBlock::Create(Context, "entry", ourMain);

    /* http://llvm.org/docs/doxygen/html/classllvm_1_1IRBuilderBase.html
     * void SetInsertPoint (BasicBlock *TheBB)
     * This specifies that created instructions should be appended to the end of the specified block.
     */
    Builder.SetInsertPoint(mainBB);

    /*** Constants are all unified ***/
    /* http://llvm.org/docs/doxygen/html/classllvm_1_1ConstantInt.html
     * static ConstantInt *	get (LLVMContext &Context, const APInt &V)
     * Return a ConstantInt with the specified value and an implied Type.
     * APInt = Arbitrary Precision Integer */
    llvm::ConstantInt* Ten = llvm::ConstantInt::get(Context, llvm::APInt(64,10));
    auto One = llvm::ConstantInt::get(Context, llvm::APInt(64, 1));
    auto Zero = llvm::ConstantInt::get(Context, llvm::APInt(64, 0));

    /* static ConstantInt *	get (IntegerType *Ty, uint64_t V, bool isSigned=false)
     * Return a ConstantInt with the specified integer value for the specified type. */
    llvm::Constant* NegativeOne = llvm::ConstantInt::get( intType, -1, true);

    /*** A and B are variables in memory, so we can read and write from them ***/

    /* http://llvm.org/docs/doxygen/html/classllvm_1_1IRBuilder.html
     * AllocaInst * 	CreateAlloca (Type *Ty, Value *ArraySize=nullptr, const Twine &Name="") */
    llvm::AllocaInst* A = Builder.CreateAlloca(intType, NULL, "A");
    llvm::AllocaInst* B = Builder.CreateAlloca(intType, NULL, "B");

    /*** lets compute
     * 0. A = 0;
     * 1. x = A;
     * 2. y = 0;
     * 3. x = y + 10 - (-1) + x;
     * 4. x = x + 1 + 10 + 10 + 10
     * 5. A = x;
     * 6. return x;
     ***/
    llvm::Function *TheFunction = Builder.GetInsertBlock()->getParent();

    llvm::BasicBlock *ThenBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "then", TheFunction);
    llvm::BasicBlock *ElseBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "else", TheFunction);
    llvm::BasicBlock *MergeBB = llvm::BasicBlock::Create(llvm::getGlobalContext(), "merge", TheFunction);    

    Builder.CreateStore(Zero, A); //0
    llvm::Value* x = Builder.CreateLoad(A, "x"); //1
    
    llvm::Value* xLessThanOne = Builder.CreateICmpULT(x, One, "tmp");
    Builder.CreateCondBr(xLessThanOne, ThenBB, ElseBB);

    Builder.SetInsertPoint(ThenBB);
    x = Builder.CreateAdd(x, One, "x");
    Builder.CreateBr(MergeBB);

    Builder.SetInsertPoint(ElseBB);
    auto x2 = Builder.CreateSub(x, One, "x");
    Builder.CreateBr(MergeBB);

    Builder.SetInsertPoint(MergeBB);
    llvm::PHINode *Phi = Builder.CreatePHI(llvm::Type::getInt32Ty(llvm::getGlobalContext()), 2, "iftmp");
    Phi->addIncoming(x, ThenBB);
    Phi->addIncoming(x2, ElseBB);

    Builder.CreateStore(Phi, A); //5
    Builder.CreateRet(Phi); //6

    llvm::verifyFunction(*ourMain); //Checks if everything is okay with our function

    /*** Lets print the intermediary representation generated ***/
    OurModule->dump();
    //ourMain->dump();

    /*** Now lets compute it with a just in time (JIT) compiler ***/
    //llvm::ExecutionEngine* OurExecutionEngine;
    //std::string Error;
    //LLVMInitializeNativeAsmPrinter();
    //LLVMInitializeNativeAsmParser();
    //LLVMInitializeNativeAsmPrinter(); //target = generates code for my processor
    //OurExecutionEngine = llvm::EngineBuilder(std::move(Owner)).setErrorStr(&Error).setMCPU(llvm::sys::getHostCPUName()).create();
    //if (!OurExecutionEngine) {
    //    fprintf(stderr, "Could not create OurExecutionEngine: %s\n", Error.c_str());
    //    exit(1);
    //}

    //// JIT our main. It returns a function pointer.
    //void *mainPointer = OurExecutionEngine->getPointerToFunction(ourMain);
    //std::cout << "1\n";
    //// Translate the pointer and run our main to get its results
    //int (*result)() = (int (*)())(intptr_t)mainPointer;
    //std::cout << "Result of our main = " << result() << std::endl;
}
